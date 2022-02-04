#pragma once

#include "PhaethonAppTest.h"
#include "Executioner.h"
#include "Transient.h"
#include "FEProblemBase.h"
#include "AscotProblem.h"
#include <filesystem>

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

// Fixture to test the time stepping of the Ascot External Problem
// TODO unfortunately, it isn't possible to inherit from AscotProblemTest because downcasting of the
// . So, some duplicate code here.
class AscotProblemTimeTest : public PhaethonAppInputTest
{
protected:
  AscotProblemTimeTest(std::string inputfile) : PhaethonAppInputTest(inputfile){};

  virtual void SetUp() override
  {

    // Call the base class method
    ASSERT_NO_THROW(PhaethonAppInputTest::SetUp());

    ASSERT_FALSE(appIsNull);
    ASSERT_NO_THROW(app->setupOptions());
    ASSERT_NO_THROW(app->runInputFile());

    // Get the executioner
    executionerPtr = dynamic_cast<Transient *>(app->getExecutioner());
    ASSERT_NE(executionerPtr, nullptr);
    // This is the bit that throws a seg fault in subclasses.
    ASSERT_NO_THROW(executionerPtr->init());
    // Simulate taking a time step
    ASSERT_NO_THROW(executionerPtr->preExecute());
    ASSERT_NO_THROW(executionerPtr->incrementStepOrReject());
    ASSERT_NO_THROW(executionerPtr->preStep());
    ASSERT_NO_THROW(executionerPtr->computeDT());

    // Get the FE problem
    problemPtr = dynamic_cast<AscotProblem *>(&(executionerPtr->feProblem()));
    ASSERT_NE(problemPtr, nullptr);

    // Check type (i.e. make sure this is an AscotProblem)
    ASSERT_EQ(problemPtr->type(), "AscotProblem");
  }

  Transient * executionerPtr = nullptr;
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

class AscotProblemSimpleRunTest : public AscotProblemTimeTest
{
protected:
  AscotProblemSimpleRunTest() : AscotProblemTimeTest("ascot_simple_run.i")
  {
    // Copy the simple_run HDF5 file that will be used a temp file for this run
    std::filesystem::copy_file("inputs/simple_run_quick_input.h5", hdf5_file_name);
    setenv("OMP_NUM_THREADS", "1", 1);
  };

  ~AscotProblemSimpleRunTest()
  {
    std::filesystem::remove(hdf5_file_name);
    unsetenv("OMP_NUM_THREADS");
  };

  const std::string hdf5_file_name = "inputs/simple_run_quick_test.h5";
};
