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
  std::vector<int> simple_run_walltile{
      16,  376, 33,  0,   74,  0,   271, 16,  13,  256, 173, 53,  133, 311, 133, 133, 0,
      0,   56,  0,   0,   0,   0,   213, 93,  0,   213, 154, 0,   334, 76,  193, 0,   0,
      0,   0,   0,   213, 0,   151, 93,  0,   391, 296, 373, 274, 374, 0,   93,  0,   31,
      274, 0,   73,  396, 0,   213, 0,   71,  94,  214, 53,  233, 194, 336, 0,   0,   53,
      196, 0,   373, 0,   0,   133, 293, 293, 0,   53,  0,   294, 391, 253, 0,   216, 356,
      236, 333, 116, 193, 253, 116, 293, 111, 334, 74,  356, 0,   236, 0,   0};

  ASSERT_EQ(problemPtr->getWallTileHits(hdf5_file), simple_run_walltile);
}
