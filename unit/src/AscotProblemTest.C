//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AscotProblemTest.h"
#include <vector>
#include "SystemBase.h"

using namespace H5;

// TODO this is ugly. Need to change!
// Test data
std::vector<int64_t> simple_run_walltile{
#include "../../../supplementary/ascot5/walltile.csv"
};
std::vector<double_t> simple_run_energy{
#include "../../../supplementary/ascot5/simple_run_energies.csv"
};
std::vector<double_t> simple_run_weight{
#include "../../../supplementary/ascot5/weight.csv"
};
std::vector<double_t> simple_run_hfluxes{
#include "../../../supplementary/ascot5/simple_run_heat_fluxes.csv"
};

// Tests
TEST_F(AscotProblemHDF5Test, check_hdf5)
{

  ASSERT_FALSE(appIsNull);
  // Check that the results group is present, then open it
  ASSERT_TRUE(hdf5_file.nameExists("results"));
  Group results_group = hdf5_file.openGroup("results");

  // Check that the attribute 'active' is present, and then open
  ASSERT_TRUE(results_group.attrExists("active"));
  H5::Attribute results_active_attr = results_group.openAttribute("active");

  // Get the string type from the attribute; this seems to be the only way to
  // read a string in without declaring its size in advance
  StrType stype = results_active_attr.getStrType();
  // String buffer where attribute value will be stored
  std::string active_result_num;
  // Read the attribute value
  results_active_attr.read(stype, active_result_num);
  // Check that the active run number is as expected
  ASSERT_EQ(active_result_num, "0069271660");
}

TEST_F(AscotProblemHDF5Test, read_walltile)
{

  ASSERT_FALSE(appIsNull);
  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);

  ASSERT_EQ(problemPtr->getWallTileHits(endstate_group), simple_run_walltile);
}

TEST_F(AscotProblemHDF5Test, read_weights)
{

  ASSERT_FALSE(appIsNull);

  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::vector<double_t> marker_weights = problemPtr->getMarkerWeights(endstate_group);

  for (size_t i = 0; i < simple_run_weight.size(); i++)
  {
    ASSERT_DOUBLE_EQ(marker_weights[i], simple_run_weight[i]);
  }
}

TEST_F(AscotProblemHDF5Test, read_energy)
{

  ASSERT_FALSE(appIsNull);

  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::vector<double_t> particle_energies = problemPtr->getParticleEnergies(endstate_group);

  for (size_t i = 0; i < simple_run_energy.size(); i++)
  {
    ASSERT_DOUBLE_EQ(particle_energies[i], simple_run_energy[i]);
  }
}

TEST_F(AscotProblemHDF5Test, calculate_relativistic_energy)
{
  // velocity components (r, phi, z) in m/s
  std::vector<double_t> velocity{6726950.87616652, -9727805.12233284, 5355264.46022884};
  // mass in amu
  double_t mass = 4.0;
  // relativistic energies compared in SI units (i.e. Joules)
  ASSERT_DOUBLE_EQ(problemPtr->calculateRelativisticEnergy(mass, velocity), 5.605926430766929e-13);
}

TEST_F(AscotProblemHDF5Test, calculate_heat_fluxes)
{

  ASSERT_FALSE(appIsNull);

  // check that the mesh is the expected size
  ASSERT_EQ(problemPtr->mesh().nElem(), (unsigned long int)400);

  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::vector<double_t> heat_fluxes =
      problemPtr->calculateHeatFluxes(simple_run_walltile, simple_run_energy, simple_run_weight);

  double_t tol;
  for (size_t i = 0; i < simple_run_hfluxes.size(); i++)
  {
    // set the relative tolerance to 0.1%
    tol = simple_run_hfluxes[i] * 0.001;
    ASSERT_NEAR(heat_fluxes[i], simple_run_hfluxes[i], tol);
  }
}

/*
TEST_F(AscotProblemHDF5Test, check_auxvariable)
{
  // It looks like I need to think more about the distributed nature of the
  // AuxVariable and mesh. I need to sum over all of the threads and probably
  // processes?
  ASSERT_EQ(problemPtr->getAuxiliarySystem()., (unsigned int)400);
}
*/

// TODO need to get this test passing
TEST_F(AscotProblemHDF5Test, check_solution_sync)
{
  ASSERT_FALSE(appIsNull);
  problemPtr->syncSolutions(ExternalProblem::Direction::FROM_EXTERNAL_APP);
  auto & auxvar_heat_fluxes = problemPtr->getVariable(0, "fi_heat_flux");

  double_t tol;
  for (size_t i = 0; i < simple_run_hfluxes.size(); i++)
  {
    // set the relative tolerance to 0.1%
    tol = simple_run_hfluxes[i] * 0.001;
    ASSERT_NEAR(auxvar_heat_fluxes.sys().solution().el(i), simple_run_hfluxes[i], tol);
  }
}