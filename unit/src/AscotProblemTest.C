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

using namespace H5;

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
  std::vector<int64_t> simple_run_walltile{
      16,  376, 33,  0,   74,  0,   271, 16,  13,  256, 173, 53,  133, 311, 133, 133, 0,
      0,   56,  0,   0,   0,   0,   213, 93,  0,   213, 154, 0,   334, 76,  193, 0,   0,
      0,   0,   0,   213, 0,   151, 93,  0,   391, 296, 373, 274, 374, 0,   93,  0,   31,
      274, 0,   73,  396, 0,   213, 0,   71,  94,  214, 53,  233, 194, 336, 0,   0,   53,
      196, 0,   373, 0,   0,   133, 293, 293, 0,   53,  0,   294, 391, 253, 0,   216, 356,
      236, 333, 116, 193, 253, 116, 293, 111, 334, 74,  356, 0,   236, 0,   0};

  ASSERT_EQ(problemPtr->getWallTileHits(endstate_group), simple_run_walltile);
}

TEST_F(AscotProblemHDF5Test, read_weights)
{

  ASSERT_FALSE(appIsNull);
  std::vector<double_t> simple_run_weight{
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

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
  std::vector<double_t> simple_run_energy{
      5.605926430766929e-13,  5.601051900561567e-13, 5.605507494388142e-13,  5.605521060424595e-13,
      5.605507494389468e-13,  5.605419474202633e-13, 5.605507494389468e-13,  5.605230803343478e-13,
      4.8187003883044004e-14, 5.600793767077879e-13, 5.493419161077282e-14,  5.60065615647442e-13,
      5.607982721999194e-13,  5.605507494389468e-13, 5.597399589610087e-13,  5.609889157350513e-13,
      5.605467255517832e-13,  5.601083012514452e-13, 5.605917132098798e-13,  5.605465781923524e-13,
      5.605558284969342e-13,  5.605618763590173e-13, 2.0217818919896866e-15, 5.601996318234811e-13,
      5.605121299265648e-13,  5.605573776524855e-13, 5.601718010601258e-13,  5.605507494388142e-13,
      5.60437135749313e-13,   4.583489379583762e-14, 5.605507494389468e-13,  5.605507494392119e-13,
      5.601922633496998e-13,  5.597604996911506e-13, 5.605483815852361e-13,  5.605498044238039e-13,
      5.605974841660187e-13,  5.602017727087353e-13, 5.605508751150348e-13,  5.605507494389468e-13,
      5.60736391874695e-13,   5.606891917619445e-13, 5.605507494389468e-13,  5.604382320411032e-13,
      5.610998403998983e-13,  5.605507494388142e-13, 2.0498838052658058e-14, 5.605554641374569e-13,
      5.601533005143227e-13,  5.605501683576524e-13, 5.605507494390794e-13,  5.605507494388142e-13,
      5.604745474953177e-13,  5.605507494388142e-13, 5.605507494389468e-13,  5.605477399784898e-13,
      3.1963590684255424e-13, 5.605469077076623e-13, 5.605507494389468e-13,  5.601656161475514e-13,
      5.602356950913613e-13,  5.601689312043307e-13, 5.605507494390794e-13,  5.605507494388142e-13,
      5.601159061748266e-13,  5.605457615743528e-13, 5.60546590906605e-13,   5.603182193530339e-13,
      5.605507494389468e-13,  5.599371462290527e-13, 5.597735375312527e-13,  5.060830674399048e-15,
      3.2042099743819206e-16, 5.60927426253862e-13,  1.1805070617155672e-14, 5.59996737743059e-13,
      5.605564375481793e-13,  3.499971908884817e-14, 5.601468719299787e-13,  5.602773784667478e-13,
      5.605507494389468e-13,  5.608335956994626e-13, 5.605506143298319e-13,  5.598687451474493e-13,
      5.605507494389468e-13,  5.605507494390794e-13, 5.338941835714529e-14,  5.605507494389468e-13,
      5.605507494390794e-13,  5.603962540140598e-13, 5.605507494389468e-13,  5.606533306652346e-13,
      5.605507494390794e-13,  5.602775486684352e-13, 5.605507494388142e-13,  5.605507494389468e-13,
      5.605442437704533e-13,  5.605507494393444e-13, 5.600653006044399e-13,  5.605565120199672e-13};

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