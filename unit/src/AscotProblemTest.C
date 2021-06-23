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

  ASSERT_EQ(problemPtr->getWallTileHits(), simple_run_walltile);
}
