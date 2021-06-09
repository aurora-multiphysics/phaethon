//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AscotProblemTest.h"

class AscotProblemHDF5Test : public AscotProblemTest
{
protected:
  AscotProblemHDF5Test() : AscotProblemTest("ascot_hdf5.i"){};
};

TEST_F(AscotProblemHDF5Test, setup) { ASSERT_FALSE(appIsNull); }
