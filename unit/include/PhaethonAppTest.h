#pragma once

#include "BasicTest.h"

class PhaethonAppBasicTest : public BasicTest
{
protected:
  PhaethonAppBasicTest() : BasicTest("PhaethonApp"){};

  virtual void SetUp() override { createApp(); };
};

class PhaethonAppInputTest : public InputFileTest
{
protected:
  PhaethonAppInputTest(std::string inputfile) : InputFileTest("PhaethonApp", inputfile){};

  virtual void SetUp() override { createApp(); };
};
