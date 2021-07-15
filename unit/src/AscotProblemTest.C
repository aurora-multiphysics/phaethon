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

TEST_F(AscotProblemHDF5Test, read_energy)
{

  ASSERT_FALSE(appIsNull);
  Group endstate_group = problemPtr->getActiveEndstate(hdf5_file);
  std::vector<double_t> simple_run_energy{
      5.60972557e-13, 5.60061066e-13, 5.60617964e-13, 5.60794317e-13, 5.60486953e-13,
      5.60540727e-13, 5.60562110e-13, 5.60898525e-13, 4.81870660e-14, 5.60212366e-13,
      5.49336901e-14, 5.60323894e-13, 5.60775365e-13, 5.60469577e-13, 5.59425721e-13,
      5.60795003e-13, 5.60835655e-13, 5.60124337e-13, 5.60789672e-13, 5.60245026e-13,
      5.60571077e-13, 5.60577938e-13, 2.02177460e-15, 5.60273457e-13, 5.60459751e-13,
      5.60628167e-13, 5.60482076e-13, 5.60442923e-13, 5.60436203e-13, 4.58348643e-14,
      5.60421410e-13, 5.60742667e-13, 5.60220578e-13, 5.59749415e-13, 5.60517881e-13,
      5.60642576e-13, 5.60585056e-13, 5.60266689e-13, 5.60561809e-13, 5.60682267e-13,
      5.60729191e-13, 5.60669035e-13, 5.60502965e-13, 5.60228730e-13, 5.61126162e-13,
      5.60603590e-13, 2.04987510e-14, 5.60493884e-13, 5.60063996e-13, 5.60618897e-13,
      5.60632573e-13, 5.60512952e-13, 5.60457165e-13, 5.60471642e-13, 5.60551243e-13,
      5.60696269e-13, 3.19798774e-13, 5.60546502e-13, 5.60588046e-13, 5.60196347e-13,
      5.60294295e-13, 5.60370442e-13, 5.60513224e-13, 5.60337995e-13, 5.59977905e-13,
      5.60505794e-13, 5.60808035e-13, 5.60323934e-13, 5.60570404e-13, 5.59963425e-13,
      5.59656716e-13, 5.06083187e-15, 3.20424196e-16, 5.60826639e-13, 1.18049988e-14,
      5.59994787e-13, 5.60559895e-13, 3.50031844e-14, 5.60138462e-13, 5.60343431e-13,
      5.60550935e-13, 5.60926403e-13, 5.60270764e-13, 5.60260031e-13, 5.60550710e-13,
      5.60495099e-13, 5.33825399e-14, 5.60527391e-13, 5.60640397e-13, 5.60498642e-13,
      5.60548715e-13, 5.60560665e-13, 5.60546715e-13, 5.60279440e-13, 5.60518951e-13,
      5.60665319e-13, 5.60504749e-13, 5.60472722e-13, 5.60104738e-13, 5.60696985e-13};

  ASSERT_EQ(problemPtr->getParticleEnergies(endstate_group), simple_run_energy);
}

TEST_F(AscotProblemHDF5Test, calculate_relativistic_energy)
{
  // TODO write this unit test to check AscotProblem::calculateRelativisticEnergy() method
}