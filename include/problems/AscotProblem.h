//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExternalProblem.h"
#include "PhaethonApp.h"
#include "H5Cpp.h"
using namespace H5;

class AscotProblem : public ExternalProblem
{
public:
  static InputParameters validParams();

  AscotProblem(const InputParameters & parameters);

  ~AscotProblem();

  /**
   * External problems should provide an override for converged, not inherit the default from
   * FEProblem that provides a reasonable implementation.
   */
  virtual bool converged() override;

  /**
   * New interface for solving an External problem. "solve()" is finalized here to provide
   * callbacks for solution syncing.
   */
  virtual void externalSolve() override;

  /**
   * Method to transfer data to/from the external application to the associated transfer mesh.
   */
  virtual void syncSolutions(Direction direction) override;

  /**
   * Method called to add AuxVariables to the simulation. These variables would be the fields
   * that should either be saved out with the MOOSE-formatted solutions or available for
   * transfer to variables in Multiapp simulations.
   */
  virtual void addExternalVariables() override {}

  std::vector<int> getWallTileHits();

  H5File & getHDF5File(H5std_string file_name);
  H5File & getHDF5File();

private:
  /// The name of the AuxVariable to transfer to
  const VariableName & _sync_to_var_name;
  /// The Auxiliary system in which the heat flux values will be stored
  System & _problem_system;
  /// The HDF5 file that is both the ASCOT5 input and output
  const H5std_string _ascot5_file_name;
  H5File _ascot5_file;
};
