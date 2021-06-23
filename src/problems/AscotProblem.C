//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AscotProblem.h"
#include "AuxiliarySystem.h"

registerMooseObject("PhaethonApp", AscotProblem);

InputParameters
AscotProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();

  params.addClassDescription("Retrieve fast ion heat fluxes from an external program");
  params.addRequiredParam<VariableName>(
      "sync_variable", "The variable that the external solution will be synced to");
  return params;
}

AscotProblem::AscotProblem(const InputParameters & parameters)
  : ExternalProblem(parameters),
    _sync_to_var_name(getParam<VariableName>("sync_variable")),
    _problem_system(getAuxiliarySystem().system()),
    _ascot5_file_name("ascot5.h5")
{
}

AscotProblem::~AscotProblem() {}

bool
AscotProblem::converged()
{
  return true;
}

void
AscotProblem::externalSolve()
{
  // TODO call ASCOT5
  return;
}

void
AscotProblem::syncSolutions(Direction direction)
{
  if (direction == Direction::FROM_EXTERNAL_APP)
  {
    return;
  }
  return;
}

std::vector<int>
AscotProblem::getWallTileHits(H5File & hdf5_file)
{
  std::vector<int> walltile{0};

  // Open the results group
  Group results_group = hdf5_file.openGroup("results");

  // Check if the attribute 'active' is present
  if (results_group.attrExists("active"))
  {
    // Open the attribute
    H5::Attribute results_active_attr = results_group.openAttribute("active");
    // Get its string type
    StrType stype = results_active_attr.getStrType();
    // Read the active run number into a string buffer
    std::string active_result_num;
    results_active_attr.read(stype, active_result_num);
    // Open the active run group
    std::string endstate_name = "run_" + active_result_num + "/endstate";
    Group endstate_group = results_group.openGroup(endstate_name);
    // Open the walltile dataset
    DataSet walltile_ds = endstate_group.openDataSet("walltile");
    // TODO current position: need to extract 1D array of "walltile" values
  }
  else
  {
    throw MooseException("ASCOT5 HDF5 File missing 'active' attribute.");
  }
  return walltile;
}

std::vector<int>
AscotProblem::getWallTileHits()
{
  return getWallTileHits(_ascot5_file);
}

H5File &
AscotProblem::getHDF5File(H5std_string file_name)
{
  // Try to open the ASCOT5 HDF5 file
  try
  {
    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    // Exception::dontPrint();

    // Open an existing file in read only mode
    _ascot5_file = H5File(file_name, H5F_ACC_RDONLY);

    return _ascot5_file;

  } // end of try block

  // catch failure caused by the H5File operations
  catch (FileIException error)
  {
    error.printErrorStack();
    // TODO should pass the FileIException to MooseException somehow
    throw MooseException("Failed to open ASCOT5 HDF5 file.");
  }
}

H5File &
AscotProblem::getHDF5File()
{
  return getHDF5File(_ascot5_file_name);
}
