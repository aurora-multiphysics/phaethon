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
std::vector<int64_t> simple_run_quick_walltile{
#include "../../../supplementary/ascot5/walltile_quick.csv"
};
std::vector<double_t> simple_run_energy{
#include "../../../supplementary/ascot5/simple_run_energies.csv"
};
std::vector<double_t> simple_run_weight{
#include "../../../supplementary/ascot5/weight.csv"
};
std::vector<double_t> simple_run_quick_weight{
#include "../../../supplementary/ascot5/weight_quick.csv"
};
std::vector<double_t> simple_run_hfluxes{
#include "../../../supplementary/ascot5/simple_run_heat_fluxes.csv"
};
std::vector<double_t> simple_run_quick_vr{
#include "../../../supplementary/ascot5/vr_quick.csv"
};
std::vector<double_t> simple_run_quick_vphi{
#include "../../../supplementary/ascot5/vphi_quick.csv"
};
std::vector<double_t> simple_run_quick_vz{
#include "../../../supplementary/ascot5/vz_quick.csv"
};
std::unordered_map<std::string, std::vector<double_t>> simple_run_endstate_fp = {
#include "../../../supplementary/ascot5/simple_run_endstate_fp.txt"
};
std::unordered_map<std::string, std::vector<int64_t>> simple_run_endstate_int = {
#include "../../../supplementary/ascot5/simple_run_endstate_int.txt"
};

// Tests
TEST(CheckMap, CheckMap)
{
  ASSERT_FLOAT_EQ(simple_run_endstate_fp["mass"][0], 4.0);
  ASSERT_EQ(simple_run_endstate_int["anum"][3], 4);
}

TEST_F(AscotProblemHDF5Test, CheckHDF5)
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

TEST_F(AscotProblemHDF5Test, getAscotH5Group)
{

  ASSERT_FALSE(appIsNull);
  const std::string group_name("options");
  // Check that the options group is present, then open it
  ASSERT_TRUE(hdf5_file.nameExists(group_name));
  Group options_group = problemPtr->getAscotH5Group(hdf5_file, group_name);

  // Check that the group location is as expected
  ASSERT_EQ(options_group.getObjName(), "/options/opt_3012694174");
  ASSERT_EQ(options_group.getNumObjs(), (long long unsigned int)79);
}

TEST_F(AscotProblemHDF5Test, ReadEndstateInt)
{
  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::unordered_map<std::string, std::vector<int64_t>> endstate_data;
  endstate_data = problemPtr->getAscotH5EndstateInt(endstate_group);

  for (auto && field : simple_run_endstate_int)
  {
    ASSERT_EQ(endstate_data[field.first], field.second);
  }
}

TEST_F(AscotProblemHDF5Test, ReadEndstateDouble)
{
  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::unordered_map<std::string, std::vector<double_t>> endstate_data;
  endstate_data = problemPtr->getAscotH5EndstateDouble(endstate_group);

  for (auto && field : simple_run_endstate_fp)
  {
    for (size_t i = 0; i < field.second.size(); i++)
    {
      ASSERT_FLOAT_EQ(endstate_data[field.first][i], field.second[i]);
    }
  }
}

TEST_F(AscotProblemHDF5Test, ReadWalltile)
{

  ASSERT_FALSE(appIsNull);
  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);

  ASSERT_EQ(problemPtr->getWallTileHits(endstate_group), simple_run_walltile);
}

TEST_F(AscotProblemHDF5Test, ReadWeights)
{

  ASSERT_FALSE(appIsNull);

  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::vector<double_t> marker_weights = problemPtr->getMarkerWeights(endstate_group);

  for (size_t i = 0; i < simple_run_weight.size(); i++)
  {
    ASSERT_DOUBLE_EQ(marker_weights[i], simple_run_weight[i]);
  }
}

TEST_F(AscotProblemHDF5Test, ReadEnergy)
{

  ASSERT_FALSE(appIsNull);

  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::vector<double_t> particle_energies = problemPtr->getParticleEnergies(endstate_group);

  for (size_t i = 0; i < simple_run_energy.size(); i++)
  {
    ASSERT_DOUBLE_EQ(particle_energies[i], simple_run_energy[i]);
  }
}

TEST_F(AscotProblemHDF5Test, CalculateRelativisticEnergy)
{
  // velocity components (r, phi, z) in m/s
  std::vector<double_t> velocity{6726950.87616652, -9727805.12233284, 5355264.46022884};
  // mass in amu
  double_t mass = 4.0;
  // relativistic energies compared in SI units (i.e. Joules)
  ASSERT_DOUBLE_EQ(problemPtr->calculateRelativisticEnergy(mass, velocity), 5.605926430766929e-13);
}

TEST_F(AscotProblemHDF5Test, CalculateHeatFluxes)
{

  ASSERT_FALSE(appIsNull);

  // check that the mesh is the expected size
  ASSERT_EQ(problemPtr->mesh().nElem(), (unsigned long int)400);

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

TEST_F(AscotProblemHDF5Test, CheckSolutionSync)
{
  ASSERT_FALSE(appIsNull);
  problemPtr->syncSolutions(ExternalProblem::Direction::FROM_EXTERNAL_APP);
  auto & auxvar_heat_fluxes = problemPtr->getVariable(0, "fi_heat_flux");
  MeshBase & to_mesh = auxvar_heat_fluxes.sys().mesh().getMesh();
  double_t tol;
  int64_t id;
  dof_id_type dof_i;
  for (const auto & el : to_mesh.element_ptr_range())
  {
    id = el->id();
    // set the relative tolerance to 0.1%
    tol = simple_run_hfluxes[id] * 0.001;
    dof_i = el->dof_number(auxvar_heat_fluxes.sys().number(), auxvar_heat_fluxes.number(), 0);
    ASSERT_NEAR(auxvar_heat_fluxes.sys().solution().el(dof_i), simple_run_hfluxes[id], tol);
  }
}

TEST_F(AscotProblemHDF5WriteTest, Endstate2Marker)
{

  H5::H5File hdf5_file(hdf5_file_name, H5F_ACC_RDWR);
  // set the test data in the class
  problemPtr->endstate_data_double = simple_run_endstate_fp;
  problemPtr->endstate_data_int = simple_run_endstate_int;

  // write the test data to the marker group
  problemPtr->copyEndstate2MarkerGroup(hdf5_file);
  hdf5_file.close();

  // check newly written marker group against reference with `h5diff`
  // TODO update h5diff arguments
  int h5diff_result =
      std::system(("h5diff -cvv " + hdf5_file_name +
                   " inputs/simple_run_endstate2markers.h5 marker/prt_1 marker/prt_0033994144")
                      .c_str());
  ASSERT_TRUE(h5diff_result);
}

TEST_F(AscotProblemSimpleRunTest, ExectuteSimpleRun)
{
  ASSERT_NO_THROW(problemPtr->externalSolve());
}

TEST_F(AscotProblemSimpleRunTest, VerifySimpleRun)
{
  ASSERT_NO_THROW(problemPtr->externalSolve());

  // Open open output file and relevant group
  H5File ascot5_file(hdf5_file_name, H5F_ACC_RDONLY);
  Group ascot5_active_endstate = problemPtr->getActiveEndstate(ascot5_file);

  // Get relevant output data for verification
  std::vector<int64_t> walltile = problemPtr->getWallTileHits(ascot5_active_endstate);
  std::vector<double_t> weights = problemPtr->getMarkerWeights(ascot5_active_endstate);

  for (size_t i = 0; i < walltile.size(); i++)
  {
    ASSERT_EQ(walltile[i], simple_run_quick_walltile[i]);
    ASSERT_DOUBLE_EQ(weights[i], simple_run_quick_weight[i]);
  }

  // The velocities don't have their own read routine, so do some repetition of code here
  // TODO this should be put into a class method of AscotProblem or a separate HDF5 wrapper class
  std::unordered_map<std::string, std::vector<double_t>> velocities;
  std::unordered_map<std::string, std::vector<double_t>> simple_quick_vel = {
      {"vr", simple_run_quick_vr}, {"vphi", simple_run_quick_vphi}, {"vz", simple_run_quick_vz}};
  std::vector<std::string> velocity_names{"vr", "vphi", "vz"};
  double_t tol;

  for (auto && name : velocity_names)
  {
    velocities[name].resize(100);
    DataSet v_dataset = ascot5_active_endstate.openDataSet(name);
    v_dataset.read(velocities[name].data(), PredType::NATIVE_DOUBLE);

    for (size_t i = 0; i < velocities[name].size(); i++)
    {
      tol = abs(simple_quick_vel[name][i] * 0.001);
      ASSERT_NEAR(velocities[name][i], simple_quick_vel[name][i], tol);
    }
  }
}

TEST_F(AscotProblemSimpleRunTest, CheckTimeStep) { ASSERT_FLOAT_EQ(1e-5, problemPtr->dt()); }

TEST_F(AscotProblemSimpleRunTest, CheckTimeOptionSet)
{
  problemPtr->syncSolutions(ExternalProblem::Direction::TO_EXTERNAL_APP);

  H5File ascot5_file(hdf5_file_name, H5F_ACC_RDONLY);
  Group options = problemPtr->getAscotH5Group(ascot5_file, "options");
  DataSet endcond_max_simtime = options.openDataSet("ENDCOND_MAX_SIMTIME");
  double_t set_time[1];
  endcond_max_simtime.read(set_time, PredType::NATIVE_DOUBLE);
  ASSERT_FLOAT_EQ(set_time[0], 1e-5);
}
