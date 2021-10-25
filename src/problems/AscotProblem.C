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

bool
AscotProblem::converged()
{
  return true;
}

void
AscotProblem::externalSolve()
{
  // TODO call ASCOT5
  return;
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

  // Get solution from ASCOT5 run
  if (direction == Direction::FROM_EXTERNAL_APP)
  {
    // Open ASCOT5 file and relevant group
    H5File ascot5_file(_ascot5_file_name, H5F_ACC_RDONLY);
    Group ascot5_active_endstate = getActiveEndstate(ascot5_file);

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
      _console << "Heat flux mapping from ASCOT5 HDF5" << std::endl;
      _console << "==================================" << std::endl;
      _console << "el_dof: " << dof_i << ", el_id: " << el->id()
               << ", flux: " << heat_fluxes[el->id()] << std::endl;
      sync_to_var.sys().solution().set(dof_i, heat_fluxes[el->id()]);
    }

    sync_to_var.sys().solution().close();
    sync_to_var.sys().update();
  }
}

Group
AscotProblem::getActiveEndstate(const H5File & hdf5_file)
{
  // Open the results group
  Group results_group = hdf5_file.openGroup("results");

  // Check if the attribute 'active' is present
  if (results_group.attrExists("active"))
  {
    // Open the attribute
    H5::Attribute results_active_attr = results_group.openAttribute("active");
    // Get its string type
    StrType stype = results_active_attr.getStrType();
    // Read the active run number into a string buffer
    std::string active_result_num;
    results_active_attr.read(stype, active_result_num);
    // Open the active run group
    std::string endstate_name = "run_" + active_result_num + "/endstate";
    Group endstate_group = results_group.openGroup(endstate_name);
    return endstate_group;
  }
  else
  {
    throw MooseException("ASCOT5 HDF5 File missing 'active' attribute.");
  }
}

std::vector<int64_t>
AscotProblem::getWallTileHits(Group & endstate_group)
{
  // Open the walltile dataset
  DataSet walltile_dataset = endstate_group.openDataSet("walltile");
  // Get the walltile dataset's data space
  DataSpace walltile_dataspace = walltile_dataset.getSpace();
  // check we only have 1 dim
  if (walltile_dataspace.getSimpleExtentNdims() == 1)
  {
    hssize_t n_markers = walltile_dataspace.getSimpleExtentNpoints();
    std::vector<int64_t> walltile(n_markers);
    walltile_dataset.read(walltile.data(), PredType::NATIVE_INT64);
    return walltile;
  }
  else
  {
    throw MooseException("ASCOT5 HDF5 File walltile dataset of incorrect dim.");
  }
}

std::vector<double_t>
AscotProblem::getMarkerWeights(Group & endstate_group)
{

  // Open the datasets to check the number of ions/markers
  DataSet weight_dataset = endstate_group.openDataSet("weight");
  DataSpace weight_dataspace = weight_dataset.getSpace();
  // Check we only have 1 dim
  if (weight_dataspace.getSimpleExtentNdims() == 1)
  {
    const hsize_t n_markers = weight_dataspace.getSimpleExtentNpoints();
    std::vector<double_t> marker_weights(n_markers);
    weight_dataset.read(marker_weights.data(), PredType::NATIVE_DOUBLE);
    return marker_weights;
  }
  else
  {
    throw MooseException("ASCOT5 HDF5 File weight dataset is of incorrect dim.");
  }
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