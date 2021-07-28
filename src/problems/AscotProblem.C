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

registerMooseObject("PhaethonApp", AscotProblem);

InputParameters
AscotProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();

  params.addClassDescription("Retrieve fast ion heat fluxes from an external program");
  params.addRequiredParam<VariableName>(
      "sync_variable", "The variable that the external solution will be synced to");
  return params;
}

AscotProblem::AscotProblem(const InputParameters & parameters)
  : ExternalProblem(parameters),
    _sync_to_var_name(getParam<VariableName>("sync_variable")),
    _problem_system(getAuxiliarySystem().system()),
    _ascot5_file_name("ascot5.h5")
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
  if (direction == Direction::FROM_EXTERNAL_APP)
  {
    // Open ASCOT5 file and relevant group
    H5File ascot5_file(_ascot5_file_name, H5F_ACC_RDONLY);
    Group ascot5_active_endstate = getActiveEndstate(ascot5_file);

    // Get particle information
    std::vector<int64_t> walltile = getWallTileHits(ascot5_active_endstate);
    std::vector<double_t> energies = getParticleEnergies(ascot5_active_endstate);

    /*
    TODO
      - Calculate heat flux values for each mesh element using walltile and energies
      - Map these onto the AuxVariable in _problem_system
    */
  }
  return;
}

Group
AscotProblem::getActiveEndstate(H5File & hdf5_file)
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
    particle_energies.push_back(calculateRelativisticEnergy(mass[i], velocity.data()));
    velocity.clear();
  }

  return particle_energies;
}

double_t
AscotProblem::calculateRelativisticEnergy(double_t mass, double_t velocity[3])
{
  double_t magnitude = std::inner_product(velocity, velocity + 3, velocity, 0.0);
  double_t gamma = 1.0 / sqrt(1.0 - magnitude / pow(constants::c, 2.0));
  return (gamma - 1.0) * mass * constants::amu * pow(constants::c, 2.0);
}