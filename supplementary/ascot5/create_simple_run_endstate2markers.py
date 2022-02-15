#!/usr/bin/env python
# coding: utf-8

import shutil
import sys
import subprocess
import numpy as np
import h5py
import scipy.constants as constants
import a5py.ascot5io.ascot5 as ascot5
import a5py.ascot5io.mrk_prt as mrk_prt
import a5py.ascot5io.mrk_gc as mrk_gc
import a5py.ascot5io.mrk_fl as mrk_fl


def main(base_file='simple_run.h5', h5file='simple_run_endstate2markers.h5'):
    # Copy the base HDF5 file
    shutil.copy(base_file, h5file)

    # Copy the endstate to markers group
    #   We only want the markers that have reached the end of the valid
    #   simulation time, which corresponds to endcond=1. All other end
    #   conditions correspond to particles that should no longer be simulated.
    #   You can find the definition of the endconditions in the C source:
    #   `ascot5/endcond.h`
    new_marker = state2markers(fn=h5file, markertype='fo', state='endstate',
                               endcond=1)

    # Trim down h5file so that it takes less space, and copy the group created
    # above into the existing marker group so that we can use `h5diff` for
    # tests
    with h5py.File(h5file, mode='r+') as file:
        del file['results']
        marker_group = file['marker']
        active_marker = 'prt_' + marker_group.attrs['active'].decode('UTF-8')
        for key in marker_group[new_marker].keys():
            del marker_group[active_marker][key]
            marker_group[active_marker][key] = marker_group[new_marker][key]

    # Now repack it so that it actually takes less space
    tempfile = 'temp'
    subprocess.call(args=['h5repack', h5file, tempfile])
    shutil.move(tempfile, h5file)


# Redefining this from `a5py/postprocessing/state2markers.py` because that has
# an error where fn is not used to open the HDF5 file. In the future, once this
# is fixed one should use that routine instead of this one.
def state2markers(fn, markertype, state, endcond, ids=np.array([0]),
                  idx=np.array([0])):
    """
    Convert ASCOT5 state to markers for rerun.

    The new markers are written in the same HDF5 file as from
    which the state is read so make a copy first!

    Parameters
    ----------

    h5fn : str
        Full path to HDF5 file.

    markertype : str
        Type of markers to be generated:
        - 'fo' Particles
        - 'gc' Guiding centers
        - 'ml' Magnetic field lines

    state : str
        State from where particles are picked

    endcond : int
        Initializes new markers from the old markers that have
        this endstate.

    ids : str, optional
        Array of marker ids to be rerun. Overrides endcond.

    idx : str, optional
        Array of marker indexes to be rerun. Overrides ids.

    Notes
    -----

    If both "endcond" and "ids" are given, then the latter overrides former.
    """

    AMU2KG = 1.66053904e-27
    J2EV = 1.0/constants.elementary_charge

    # Read endstate and find indexes of valid markers
    if state == "endstate":
        a5 = ascot5.Ascot(fn).active.endstate.read()
    elif state == "inistate":
        a5 = ascot5.Ascot(fn).active.inistate.read()
    else:
        raise ValueError('Unknown state type.')

    if (idx == 0).all() and (ids == 0).all():
        idx = a5["endcond"] == endcond
    elif (idx == 0).all():
        print("TODO Not implemted")

    N = np.sum(idx)
    if(N == 0):
        print("Error: No valid markers.")
        return

    # Convert endstate to...

    # ..particles
    if markertype == 'fo':
        ids    = a5["id"][idx]
        mass   = a5["mass"][idx]
        charge = a5["charge"][idx]
        R      = a5["rprt"][idx]
        phi    = a5["phiprt"][idx]
        z      = a5["zprt"][idx]
        vR     = a5["vr"][idx]
        vphi   = a5["vphi"][idx]
        vz     = a5["vz"][idx]
        anum   = a5["anum"][idx]
        znum   = a5["znum"][idx]
        weight = a5["weight"][idx]
        time   = a5["time"][idx]
        return mrk_prt.write_hdf5(fn, N, ids, mass, charge,
                           R, phi, z, vR, vphi, vz,
                           anum, znum, weight, time)

    # ..guiding centers
    if markertype == 'gc':
        ids    = a5["id"][idx]
        mass   = a5["mass"][idx]
        charge = a5["charge"][idx]
        R      = a5["r"][idx]
        phi    = a5["phi"][idx]
        z      = a5["z"][idx]
        mu     = a5["mu"][idx]
        vpar   = a5["vpar"][idx]
        theta  = a5["theta"][idx]
        anum   = a5["anum"][idx]
        znum   = a5["znum"][idx]
        weight = a5["weight"][idx]
        time   = a5["time"][idx]
        B      = np.sqrt( np.power(a5["br"][idx],2)
                          + np.power(a5["bphi"][idx],2)
                          + np.power(a5["bz"][idx],2) )

        energy = mu * B + (0.5 * mass * vpar * vpar) * (AMU2KG * J2EV)
        v = np.sqrt( 2 * ( energy / J2EV ) / (mass * AMU2KG) )
        pitch  = vpar / v
        return mrk_gc.write_hdf5(fn, N, ids, mass, charge,
                          R, phi, z, energy, pitch, theta,
                          anum, znum, weight, time)

    # ..magnetic field lines
    if markertype == 'ml':
        ids    = a5["id"][idx]
        R      = a5["r"][idx]
        phi    = a5["phi"][idx]
        z      = a5["z"][idx]
        weight = a5["weight"][idx]
        time   = a5["time"][idx]
        vpar   = a5["vpar"][idx]
        v      = np.sqrt( np.power(a5["vr"][idx],2)
                          + np.power(a5["vphi"][idx],2)
                          + np.power(a5["vz"][idx],2) )
        pitch  = vpar / v
        return mrk_fl.write_hdf5(fn, N, ids, R, phi, z, pitch,
                          weight, time)


if __name__ == '__main__':
    if len(sys.argv[1:]) == 0:
        main()
    else:
        main(sys.argv[1:])
