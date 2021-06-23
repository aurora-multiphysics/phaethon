#pragma once

#include "PhaethonAppTest.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "AscotProblem.h"

// Fixture to test the Ascot External Problem
class AscotProblemTest : public PhaethonAppInputTest
{
protected:
  AscotProblemTest(std::string inputfile) : PhaethonAppInputTest(inputfile){};

  virtual void SetUp() override
  {

    // Call the base class method
    ASSERT_NO_THROW(PhaethonAppInputTest::SetUp());

    ASSERT_FALSE(appIsNull);
    ASSERT_NO_THROW(app->setupOptions());
    ASSERT_NO_THROW(app->runInputFile());

    // Get the executioner
    executionerPtr = app->getExecutioner();
    ASSERT_NE(executionerPtr, nullptr);

    // Get the FE problem
    problemPtr = dynamic_cast<AscotProblem *>(&(executionerPtr->feProblem()));
    ASSERT_NE(problemPtr, nullptr);

    // Check type (i.e. make sure this is an AscotProblem)
    ASSERT_EQ(problemPtr->type(), "AscotProblem");
  }

  Executioner * executionerPtr = nullptr;
  AscotProblem * problemPtr = nullptr;
};

class AscotProblemHDF5Test : public AscotProblemTest
{
protected:
  AscotProblemHDF5Test() : AscotProblemTest("ascot_hdf5.i"){};

  virtual void SetUp() override
  {

    // Call the base class method
    EXPECT_NO_THROW(AscotProblemTest::SetUp());

    // File name for HDF5 file with test data
    H5std_string ascot5_file_name("inputs/simple_run.h5");

    // Get the file
    EXPECT_TRUE(H5File::isHdf5(ascot5_file_name));
    hdf5_file = problemPtr->getHDF5File(ascot5_file_name);
    EXPECT_GE(hdf5_file.getId(), 0);
  }

  H5File hdf5_file;
};
