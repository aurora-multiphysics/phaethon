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
   * @brief Get the Marker Group object
   *
   * @param hdf5_file the ASCOT5 HDF5 file with input and output data.
   * @return H5::Group the HDF5 file group object for the markers
   */
  static H5::Group getAscotH5Group(const H5::H5File & hdf5_file, const std::string & group_name);

  /**
   * @brief Get an Ascot HDF5 Data object
   *
   * @tparam T type of the data object
   * @param endstate_group the group in which the data object sits
   * @param field_name the name of the data object in the HDF5 file
   * @return std::vector<T> the data object values
   */
  template <class T>
  static std::vector<T> getAscotH5DataField(H5::Group & endstate_group,
                                            const std::string & field_name);

  /**
   * @brief Set an Ascot HDF5 Data object
   *
   * @tparam T type of the data object
   * @param endstate_group the group to which the data object will be written
   * @param field_name the name of the data object in the HDF5 file
   */
  template <class T>
  static void setAscotH5DataField(H5::Group & endstate_group, const std::string & field_name);

  /**
   * @brief Create and write a dataset to an HDF5 group
   *
   * @tparam T the type of data
   * @param data the data to write to the HDF5 DataSet
   * @param name the name of the data field in the HDF5 file
   * @param data_space the HDF5 DataSpace for the DataSet
   * @param group the HDF5 Group in which the DataSet will be created
   */
  template <class T>
  static void createAndWriteDataset(const std::vector<T> & data,
                                    const std::string & name,
                                    const H5::DataSpace & data_space,
                                    const H5::Group & group);

  /**
   * @brief Copy the ASCOT5 endstate data members to the marker group in the HDF5 file
   *
   * @param marker_group the marker group in the HDF5 file to which the endstate
   *        is to be copied
   */
  void copyEndstate2MarkerGroup(const H5::H5File & hdf5_file);

  /**
   * @brief Get the Double type Ascot Endstate Variables from HDF5 File
   *
   * @param endstate_group the HDF5 file group where the endstate lives
   * @return std::unordered_map<std::string, std::vector<double_t>> the variables of type double_t
   * in the endstate
   */
  static std::unordered_map<std::string, std::vector<double_t>>
  getAscotH5EndstateDouble(H5::Group & endstate_group);

  /**
   * @brief Get the Integer type Ascot Endstate Variables from HDF5 File
   *
   * @param endstate_group the HDF5 file group where the endstate lives
   * @return std::unordered_map<std::string, std::vector<double_t>> the variables of type int64_t in
   * the endstate
   */
  static std::unordered_map<std::string, std::vector<int64_t>>
  getAscotH5EndstateInt(H5::Group & endstate_group);

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

  /**
   * @brief Calculate the heat flux values on each element of the mesh
   *
   * @param walltile the mesh element index that each marker has hit
   * @param energies the energies of the markers in Joules
   * @param weights the marker weights in units of markers/s
   * @return std::vector<double_t> the heat flux incident on each element of the wall mesh in units
   * of W/m^2
   */
  std::vector<double_t> calculateHeatFluxes(std::vector<int64_t> walltile,
                                            std::vector<double_t> energies,
                                            std::vector<double_t> weights);

  // Endstate variables required for restarting ASCOT5
  std::unordered_map<std::string, std::vector<double_t>> endstate_data_double;
  std::unordered_map<std::string, std::vector<int64_t>> endstate_data_int;

private:
  /// The name of the AuxVariable to transfer to
  const VariableName & _sync_to_var_name;
  /// The Auxiliary system in which the heat flux values will be stored
  AuxiliarySystem & _problem_system;
  /// The HDF5 file that is both the ASCOT5 input and output
  const FileName & _ascot5_file_name;
  H5::H5File _ascot5_file;
  /// Mapping for top-level group name to sub-group prefix for ASCOT5 HDF5 file
  static const std::unordered_map<std::string, std::string> hdf5_group_prefix;
  // The HDF5 dataset names that constitute the required endstate variables to restart ASCOT5
  static const std::vector<std::string> endstate_fields_double;
  static const std::vector<std::string> endstate_fields_int;
};
