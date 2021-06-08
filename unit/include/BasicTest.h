#pragma once

#include "gtest/gtest.h"

#include <memory>

#include "AppFactory.h"
#include "MooseObject.h"

class BasicTest : public ::testing::Test
{

protected:
  BasicTest(std::string appNameIn) : args(""), appName(appNameIn), appIsNull(true){};

  virtual void SetUp() override{};

  virtual void TearDown() override { app = nullptr; };

  void createApp()
  {

    // Convert string arguments to char array
    char * cstr = new char[args.length() + 1];
    std::strcpy(cstr, args.c_str());

    // Split string by whitespace delimiter
    std::vector<char *> arg_list;
    char * next;
    next = strtok(cstr, " ");
    while (next != NULL)
    {
      arg_list.push_back(next);
      next = strtok(NULL, " ");
    }

    // Create array of char*
    size_t argc = arg_list.size();
    char ** argv = new char *[argc];
    for (size_t i = 0; i < argc; i++)
    {
      argv[i] = arg_list.at(i);
    }

    try
    {
      app = AppFactory::createAppShared(appName, argc, argv);
      appIsNull = (app == nullptr);
    }
    catch (std::exception & e)
    {
      std::cout << e.what() << std::endl;
      appIsNull = true;
    }

    // Deallocate memory for C arrays created with new
    delete argv;
    delete cstr;
  };

  void checkKnownObjects(const std::vector<std::string> & knownObjNames)
  {
    checkKnownObjects(knownObjNames, appName);
  }

  void checkKnownObjects(const std::vector<std::string> & knownObjNames, std::string appNameTest)
  {
    ASSERT_FALSE(appIsNull);

    // Fetch a reference to all objects that been registered (in the global singleton)
    const auto & allRegistered = Registry::allObjects();

    // Check the app exists in the registry
    bool foundApp = allRegistered.find(appNameTest) != allRegistered.end();
    ASSERT_TRUE(foundApp);

    // Get all objects registered to the app
    const auto & appObjs = allRegistered.at(appNameTest);

    // Create a list of names of all objects we expect to see registered
    std::map<std::string, bool> knownObjs;
    for (const auto objName : knownObjNames)
    {
      knownObjs[objName] = false;
    }

    // Check the objects match up
    for (const auto & obj : appObjs)
    {
      std::string objName = obj._classname;
      bool isKnownObj = knownObjs.find(objName) != knownObjs.end();
      EXPECT_TRUE(isKnownObj) << "Unknown object was registered";
      if (isKnownObj)
      {
        // Shouldn't already have marked found
        EXPECT_FALSE(knownObjs[objName]) << "Duplicate enties of object " << objName;

        // Mark that we found this object
        knownObjs[objName] = true;
      }
    }

    // Now should have found all known objects
    for (const auto & knownObj : knownObjs)
    {
      EXPECT_TRUE(knownObj.second)
          << "Object " << knownObj.first << " was not registered to " << appNameTest;
    }
  }

  // Arguments for our app
  std::string args;

  // Pointer to our Moose App;
  std::shared_ptr<MooseApp> app = nullptr;

  std::string appName;

  bool appIsNull;
};

class InputFileTest : public BasicTest
{

protected:
  InputFileTest(std::string appName, std::string inputfile) : BasicTest(appName), tol(1.e-9)
  {
    setInputFile(inputfile);
  };

  void setInputFile(std::string inputfile)
  {
    if (inputfile == "")
    {
      throw std::logic_error("Input file string is empty");
    }
    args += " -i inputs/" + inputfile;
  };

  // Some file utility methods
  bool fileExists(std::string filename)
  {
    std::ifstream f(filename.c_str());
    return f.good();
  }

  void fetchInputFile(std::string file, std::string newName = "")
  {

    std::string fileNow = "inputs/" + file;
    ASSERT_TRUE(fileExists(fileNow));

    if (newName == "")
      newName = file;

    // This is ugly: system() can result in undefined behaviour. Fix me.
    std::string command = "cp " + fileNow + " " + newName;
    int retval = system(command.c_str());
    ASSERT_EQ(retval, 0);
    ASSERT_TRUE(fileExists(newName));
  }

  void fetchInput(const std::vector<std::string> & inputFiles)
  {
    for (const auto file : inputFiles)
    {
      fetchInputFile(file);
    }
  }

  void deleteFile(std::string filename)
  {
    std::string err = "Failed to remove " + filename;
    EXPECT_EQ(remove(filename.c_str()), 0) << err;
  }

  void deleteIfFileExists(const std::string & file)
  {
    if (fileExists(file))
    {
      deleteFile(file);
    }
  }

  void deleteAll(const std::vector<std::string> & fileList)
  {
    for (const auto file : fileList)
    {
      deleteIfFileExists(file);
    }
  }

  void checkFilesExist(const std::vector<std::string> & fileList)
  {
    for (const auto file : fileList)
    {
      EXPECT_TRUE(fileExists(file)) << file << " was not found";
    }
  }

  // Define a tolerance for double comparisons
  double tol;
};
