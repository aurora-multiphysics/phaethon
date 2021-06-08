//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include <memory>

#include "PhaethonAppTest.h"
#include "Executioner.h"

TEST_F(PhaethonAppBasicTest, registryTest)
{

  // Check we can find heat conduction
  bool foundHeatConduction = Registry::isRegisteredObj("ADHeatConduction");
  EXPECT_TRUE(foundHeatConduction);

  // Check we can find the objects we need
  std::vector<std::string> knownObjNames;
  knownObjNames.push_back("AscotProblem");
  checkKnownObjects(knownObjNames);

  // TODO might need separate checks here when using MultiApp, like below
  // knownObjNames.clear();
  // knownObjNames.push_back("AscotProblem");
  // checkKnownObjects(knownObjNames,"AscotApp");
}

class MinimalInputTest : public PhaethonAppInputTest
{
protected:
  MinimalInputTest() : PhaethonAppInputTest("minimal.i"){};
};

TEST_F(MinimalInputTest, readInput)
{
  ASSERT_FALSE(appIsNull);

  ASSERT_NO_THROW(app->setupOptions());
  ASSERT_NO_THROW(app->runInputFile());
}

// TODO also possible to do a full regression test here, but should probably
// use the "gold" system instead
