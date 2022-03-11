//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AscotProblem.h"
#include "AuxiliarySystem.h"
#include <algorithm>
#include <filesystem>
namespace ascot5
{
#include "ascot5_main.h"
}
using namespace H5;

registerMooseObject("PhaethonApp", AscotProblem);

InputParameters
AscotProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();

  params.addClassDescription("Retrieve fast ion heat fluxes from an external program");
  params.addRequiredParam<VariableName>(
      "sync_variable", "The variable that the external solution will be synced to");
  params.addParam<FileName, FileName>(
      "ascot5_file", "ascot5.h5", "The HDF5 input and output file for ASCOT5");
  return params;
}

AscotProblem::AscotProblem(const InputParameters & parameters)
  : ExternalProblem(parameters),
    _sync_to_var_name(getParam<VariableName>("sync_variable")),
    _problem_system(getAuxiliarySystem()),
    _ascot5_file_name(getParam<FileName>("ascot5_file"))
{
}

AscotProblem::~AscotProblem() {}

const std::unordered_map<std::string, std::string> AscotProblem::hdf5_group_prefix = {
    {"marker", "prt"}, {"options", "opt"}, {"results", "run"}};

const std::vector<std::string> AscotProblem::endstate_fields_double = {
    "mass", "rprt", "phiprt", "zprt", "vr", "vphi", "vz", "weight", "time"};

const std::vector<std::string> AscotProblem::endstate_fields_int = {
    "id", "charge", "anum", "znum", "endcond"};

bool
AscotProblem::converged()
{
  return true;
}

void
AscotProblem::externalSolve()
{
  // Compose the input arguments to ASCOT5
  int argc = 2;
  size_t lastindex = _ascot5_file_name.find(".");
  std::string ascot5_input = "--in=" + _ascot5_file_name.substr(0, lastindex);
  const char * argv[2] = {"ascot5", ascot5_input.c_str()};
  try
  {
    ascot5::ascot5_main(argc, (char **)argv);
  }
  catch (const std::exception & e)
  {
    std::cerr << e.what() << '\n';
    throw MooseException(e.what());
  }
}

void
AscotProblem::syncSolutions(Direction direction)
{
  // TODO this works, but I'm not sure if it is the best place to check that the
  // AuxVariable being passed is present in the system
  if (!_problem_system.hasVariable(_sync_to_var_name))
  {
    throw MooseException("AuxVariable " + _sync_to_var_name +
                         " from input file is not present in the AuxiliarySystem. Make sure you "
                         "have declared the AuxVariables block.");
  }

  // ExternalProblem's are not threaded, so pass thread 0
  auto & sync_to_var = _problem_system.getVariable(0, _sync_to_var_name);

  if (sync_to_var.isNodal() || sync_to_var.feType().order != 0)
  {
    throw MooseException(
        "MooseVariable passed to AscotProblem is nodal or of order > 0. It must be "
        "elemental and order 0 (i.e. CONSTANT).");
  }

  // Send input for current time step to ASCOT5
  if (direction == Direction::TO_EXTERNAL_APP)
  {
    // Open ASCOT5 file and relevant groups for writing
    H5File ascot5_file(_ascot5_file_name, H5F_ACC_RDWR);
    Group ascot5_options = getAscotH5Group(ascot5_file, "options");

    // Catch any exceptions related to writing to HDF5 file
    try
    {
      // Write the end time condition to the options group
      DataSet endcond_max_simtime = ascot5_options.openDataSet("ENDCOND_MAX_SIMTIME");
      double_t data[1] = {time() + dt()};
      endcond_max_simtime.write(data, PredType::NATIVE_DOUBLE);
      // Copy the endstate to the marker group
      if (_t_step > 1)
      {
        copyEndstate2MarkerGroup(ascot5_file);
      }
    }
    catch (DataSetIException error)
    {
      error.printErrorStack();
    }
  }

  // Get solution from ASCOT5 run
  if (direction == Direction::FROM_EXTERNAL_APP)
  {
    // Open ASCOT5 file and relevant group
    H5File ascot5_file(_ascot5_file_name, H5F_ACC_RDONLY);
    Group ascot5_active_endstate = getActiveEndstate(ascot5_file);

    // Read relevant endstate variables for restarting ASCOT5
    endstate_data_double = getAscotH5EndstateDouble(ascot5_active_endstate);
    endstate_data_int = getAscotH5EndstateInt(ascot5_active_endstate);

    // Get particle information
    std::vector<int64_t> walltile = getWallTileHits(ascot5_active_endstate);
    std::vector<double_t> energies = getParticleEnergies(ascot5_active_endstate);
    std::vector<double_t> weights = getMarkerWeights(ascot5_active_endstate);

    // Calculate the heat fluxes
    std::vector<double_t> heat_fluxes = calculateHeatFluxes(walltile, energies, weights);

    // Get the mesh
    MeshBase & to_mesh = mesh().getMesh();

    dof_id_type dof_i;
    // Although there is a fairly trivial 1:1 mapping here between mesh element
    // id and dof index, it is safer to explicitly get the dof indicies from the
    // active mesh elements
    for (const auto & el : to_mesh.active_local_element_ptr_range())
    {
      dof_i = el->dof_number(sync_to_var.sys().number(), sync_to_var.number(), 0);
      // TODO make this heading coloured
      //_console << "Heat flux mapping from ASCOT5 HDF5" << std::endl;
      //_console << "==================================" << std::endl;
      //_console << "el_dof: " << dof_i << ", el_id: " << el->id()
      //         << ", flux: " << heat_fluxes[el->id()] << std::endl;
      sync_to_var.sys().solution().set(dof_i, heat_fluxes[el->id()]);
    }

    sync_to_var.sys().solution().close();
    sync_to_var.sys().update();
  }
}

Group
AscotProblem::getActiveEndstate(const H5File & hdf5_file)
{
  return getAscotH5Group(hdf5_file, "results");
}

Group
AscotProblem::getAscotH5Group(const H5File & hdf5_file, const std::string & group_name)
{
  // Open the top-level group
  Group top_group = hdf5_file.openGroup(group_name);

  // Check if the attribute 'active' is present
  if (top_group.attrExists("active"))
  {
    // Open the attribute
    H5::Attribute active_attr = top_group.openAttribute("active");
    // Get its string type
    StrType stype = active_attr.getStrType();
    // Read the active run number into a string buffer
    std::string active_num;
    active_attr.read(stype, active_num);
    // Get the name for the group
    std::string subgroup_name = AscotProblem::hdf5_group_prefix.at(group_name) + "_" + active_num;
    if (group_name == "results")
    {
      subgroup_name.append("/endstate");
    }
    // Open the group
    Group active_group = top_group.openGroup(subgroup_name);
    return active_group;
  }
  else
  {
    throw MooseException("ASCOT5 HDF5 File missing 'active' attribute.");
  }
}

template <class T>
std::vector<T>
AscotProblem::getAscotH5DataField(H5::Group & endstate_group, const std::string & field_name)
{

  // Open the datasets to check the number of ions/markers
  DataSet dataset = endstate_group.openDataSet(field_name);
  DataSpace dataspace = dataset.getSpace();
  // Check we only have 1 dim
  if (dataspace.getSimpleExtentNdims() == 1)
  {
    const hsize_t n_markers = dataspace.getSimpleExtentNpoints();
    std::vector<T> marker_data(n_markers);
    // Map the C++ type to what is expected by HDF5 routines
    if (typeid(T) == typeid(double_t))
    {
      dataset.read(marker_data.data(), PredType::NATIVE_DOUBLE);
    }
    else if (typeid(T) == typeid(int64_t))
    {
      dataset.read(marker_data.data(), PredType::NATIVE_INT64);
    }
    else
    {
      throw MooseException("Unrecognised data type requested from HDF5 file");
    }
    return marker_data;
  }
  else
  {
    throw MooseException("ASCOT5 HDF5 File weight dataset is of incorrect dim.");
  }
}

std::unordered_map<std::string, std::vector<double_t>>
AscotProblem::getAscotH5EndstateDouble(H5::Group & endstate_group)
{
  std::unordered_map<std::string, std::vector<double_t>> endstate_data;
  for (auto && field : endstate_fields_double)
  {
    endstate_data[field] = getAscotH5DataField<double_t>(endstate_group, field);
  }
  return endstate_data;
}

std::unordered_map<std::string, std::vector<int64_t>>
AscotProblem::getAscotH5EndstateInt(H5::Group & endstate_group)
{
  std::unordered_map<std::string, std::vector<int64_t>> endstate_data;
  for (auto && field : endstate_fields_int)
  {
    endstate_data[field] = getAscotH5DataField<int64_t>(endstate_group, field);
  }
  return endstate_data;
}

std::vector<int64_t>
AscotProblem::getWallTileHits(Group & endstate_group)
{
  return getAscotH5DataField<int64_t>(endstate_group, "walltile");
}

std::vector<double_t>
AscotProblem::getMarkerWeights(Group & endstate_group)
{
  return getAscotH5DataField<double_t>(endstate_group, "weight");
}

std::vector<double_t>
AscotProblem::getParticleEnergies(Group & endstate_group)
{

  std::vector<double_t> particle_energies;
  // Open the one of the datasets to check the number of ions/markers
  DataSet mass_dataset = endstate_group.openDataSet("mass");
  DataSpace mass_dataspace = mass_dataset.getSpace();
  // Check we only have 1 dim
  if (mass_dataspace.getSimpleExtentNdims() != 1)
  {
    throw MooseException("ASCOT5 HDF5 File walltile dataset of incorrect dim.");
  }
  const hsize_t n_markers = mass_dataspace.getSimpleExtentNpoints();

  // Read in mass
  std::vector<double_t> mass(n_markers);
  mass_dataset.read(mass.data(), PredType::NATIVE_DOUBLE);
  // Read in velocities
  std::unordered_map<std::string, std::vector<double_t>> velocities;
  std::vector<std::string> velocity_names{"vr", "vphi", "vz"};
  for (auto && name : velocity_names)
  {
    velocities[name].resize(n_markers);
    DataSet v_dataset = endstate_group.openDataSet(name);
    v_dataset.read(velocities[name].data(), PredType::NATIVE_DOUBLE);
  }

  std::vector<double_t> velocity;
  for (hsize_t i = 0; i < n_markers; i++)
  {
    for (auto && name : velocity_names)
    {
      velocity.push_back(velocities[name][i]);
    }
    particle_energies.push_back(calculateRelativisticEnergy(mass[i], velocity));
    velocity.clear();
  }

  return particle_energies;
}

double_t
AscotProblem::calculateRelativisticEnergy(double_t mass, std::vector<double_t> velocity)
{

  std::transform(velocity.begin(), velocity.end(), velocity.begin(), [](double_t & v) {
    return v / constants::c;
  });
  double_t magnitude = std::inner_product(velocity.begin(), velocity.end(), velocity.begin(), 0.0);
  double_t gamma = 1.0 / sqrt(1.0 - magnitude);
  return (gamma - 1.0) * mass * constants::amu * constants::c * constants::c;
}

std::vector<double_t>
AscotProblem::calculateHeatFluxes(std::vector<int64_t> walltile,
                                  std::vector<double_t> energies,
                                  std::vector<double_t> weights)
{

  // allocate heat flux storage based on number of elements in current mesh
  std::vector<double_t> heat_fluxes(_mesh.nElem());

  // sum the energies incident on each walltile
  for (size_t i = 0; i < walltile.size(); i++)
  {
    if (walltile[i] > 0)
    {
      heat_fluxes[walltile[i] - 1] += energies[i] * weights[i];
    }
  }

  // divide by tile area to get flux, done separately to reduce numerical errors
  for (size_t i = 0; i < heat_fluxes.size(); i++)
  {
    heat_fluxes[i] /= _mesh.elemPtr(i)->volume();
  }

  return heat_fluxes;
}

void
AscotProblem::copyEndstate2MarkerGroup(const H5File & hdf5_file)
{
  // create a new marker group for the next time step
  std::string step_num = std::to_string(_t_step);
  if (step_num.length() < 10)
    step_num.insert(step_num.front() == '-' ? 1 : 0, 10 - step_num.length(), '0');
  Group new_marker = hdf5_file.createGroup("marker/prt_" + step_num);
  // adjust the 'active' attribute on the top level marker group
  Group marker = hdf5_file.openGroup("marker");
  H5::Attribute active = marker.openAttribute("active");
  StrType stype = active.getStrType();
  active.write(stype, step_num);
  // get the number of markers still active
  std::vector<size_t> valid_indices;
  // TODO this could undoubtedly be optimised. Performance will be quite poor if
  // endcond vector is large.
  for (size_t i = 0; i != endstate_data_int.at("endcond").size(); i++)
  {
    // An endcondition of 1 indicates the marker reached the end of the
    // simulation time. All other endconditions indicate that the marker has
    // terminated and should no longer be simulated.
    if (endstate_data_int.at("endcond")[i] == 1)
    {
      valid_indices.push_back(i);
    }
  }
  std::vector<int64_t> nmarkers = {(int64_t)valid_indices.size()};
  // write the number of markers to the new group
  const int64_t rank = 2;
  hsize_t dims[rank] = {1, 1};
  DataSpace data_space(rank, dims);
  createAndWriteDataset<int64_t>(nmarkers, "n", data_space, new_marker);
  // set the DataSpace for all other arrays based on the number of markers
  dims[0] = (hsize_t)nmarkers[0];
  data_space = DataSpace(rank, dims);
  // Write double data
  for (auto && field : endstate_fields_double)
  {
    std::vector<double_t> data;
    // filter the data
    for (auto && j : valid_indices)
    {
      data.push_back(endstate_data_double.at(field)[j]);
    }
    createAndWriteDataset<double_t>(data, field, data_space, new_marker);
  }
  // Write integer data
  for (auto && field : endstate_fields_int)
  {
    std::vector<int64_t> data;
    if (field == "endcond")
    {
      continue;
    }
    // filter the data
    for (auto && j : valid_indices)
    {
      data.push_back(endstate_data_int.at(field)[j]);
    }
    createAndWriteDataset<int64_t>(data, field, data_space, new_marker);
  }
}

template <class T>
void
AscotProblem::createAndWriteDataset(const std::vector<T> & data,
                                    const std::string & name,
                                    const DataSpace & data_space,
                                    const Group & group)
{
  const PredType * type = nullptr;
  if (typeid(T) == typeid(double_t))
  {
    type = &PredType::NATIVE_DOUBLE;
  }
  else if (typeid(T) == typeid(int64_t))
  {
    type = &PredType::NATIVE_INT64;
  }
  else
  {
    throw MooseException("Unrecognised data type requested from HDF5 file");
  }
  // remove the 'prt' substring from some of the field names
  std::string name_local(name);
  size_t start = name_local.find("prt");
  if (start != std::string::npos)
  {
    name_local.erase(start, 3);
  }
  DataSet dataset = group.createDataSet(name_local, *type, data_space);
  dataset.write(data.data(), *type);
}