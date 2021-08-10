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
  AscotProblemHDF5Test()
    : AscotProblemTest("ascot_hdf5.i"),
      hdf5_file(H5::H5File("inputs/simple_run.h5", H5F_ACC_RDONLY)){};

  virtual void SetUp() override
  {
    // Call the base class method
    EXPECT_NO_THROW(AscotProblemTest::SetUp());

    // Get the file and briefly check it
    EXPECT_GE(hdf5_file.getId(), 0);
  }

  H5::H5File hdf5_file;
};
