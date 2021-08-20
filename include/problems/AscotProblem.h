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

namespace constants
{
const double_t c = 299792458.0;
const double_t amu = 1.6605390666e-27;
}

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

  /**
   * @brief Get the Active Endstate Group object
   *
   * @param hdf5_file the ASCOT5 HDF5 file with output data.
   * @return Group the HDF5 group object for the active endstate.
   */
  static H5::Group getActiveEndstate(const H5::H5File & hdf5_file);

  /**
   * @brief Get the indices of wall tiles that each particle has collided with
   *
   * @param endstate_group the HDF5 group object for the active endstate.
   * @return std::vector<int64_t> the wall mesh elements that each particle has
   * hit in this timestep. Indexed by particles.
   */
  static std::vector<int64_t> getWallTileHits(H5::Group & endstate_group);

  /**
   * @brief Get the Marker Weights
   *
   * The "weight" of a marker represents the rate of particles tracked for this
   * time step
   *
   * @param endstate_group the HDF5 group object for the active endstate.
   * @return std::vector<double_t> the marker weights in units of markers/s.
   */
  static std::vector<double_t> getMarkerWeights(H5::Group & endstate_group);

  /**
   * @brief Get the Particle Energies
   *
   * @param endstate_group the HDF5 group object for the active endstate.
   * @return std::vector<double_t> the particles energies in Joules.
   */
  static std::vector<double_t> getParticleEnergies(H5::Group & endstate_group);

  /**
   * @brief Calculate the relativistic energy for a particle from velocities
   *
   * @param mass the particle's mass in kg.
   * @param velocity the particle's velocity vector in m/s.
   * @return double_t the particle's energy in Joules
   */
  static double_t calculateRelativisticEnergy(double_t mass, std::vector<double_t> velocity);

private:
  /// The name of the AuxVariable to transfer to
  const VariableName & _sync_to_var_name;
  /// The Auxiliary system in which the heat flux values will be stored
  System & _problem_system;
  /// The HDF5 file that is both the ASCOT5 input and output
  const H5std_string _ascot5_file_name;
  H5::H5File _ascot5_file;
};
