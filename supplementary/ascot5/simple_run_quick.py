"""
Module for generating simple runs from analytical inputs.

File: simpleruns.py
"""
import numpy as np

import a5py.ascot5io.B_GS      as B_GS
import a5py.ascot5io.E_TC      as E_TC
import a5py.ascot5io.plasma_1D as plasma_1D
import a5py.ascot5io.wall_2D   as wall_2D
import a5py.ascot5io.N0_3D     as N0_3D
import a5py.ascot5io.options   as options
import a5py.ascot5io.mrk_prt   as mrk_prt
import a5py.ascot5io.mrk_gc    as mrk_gc

from numpy.random import random as rand
from scipy.constants import physical_constants as const

def make_ascot5_hello_world():
    settings = {}

    # Magnetic field input, choose whether you want
    # - Analytical or spline interpolated
    # - Axisymmetric or 3D
    settings["bfield_use_splines"] = False
    settings["bfield_make_3D"] = False

    # How many markers you want
    settings["markers"] = 100

    # Do you want 2D or 3D wall model
    settings["wall_make_3D"] = True

    # Simulate guiding centers (GC) (if false, simulate gyro-orbits (GO))
    settings["sim_gc_mode"] = False

    # If this is a guiding center simulation, do we use adaptive step?
    settings["sim_use_adaptivestep"] = False

    # Try out the hybrid model for wall collision checks
    settings["sim_use_hybrid"] = False

    # Record GC position even if the simulation is GO simulation
    settings["sim_recordGCasGO"] = False

    make_ascot5_slowingdownrun("simple_run_quick.h5", settings)

def make_ascot5_regression_test():
    print("implement me")

def make_ascot5_slowingdownrun(fn, settings):
    """
    Create test case base with uniform EM fields and plasma.
    """
    # ITER like but circular magnetic field paramters
    psi_coeff = np.array([2.21808016e-02,  -1.28841781e-01,  -4.17718173e-02,
                          -6.22680280e-02,   6.20083978e-03,  -1.20524711e-03,
                          -3.70147050e-05,   0.00000000e+00,   0.00000000e+00,
                          0.00000000e+00,   0.00000000e+00,   0.00000000e+00,
                          -0.155])
    R0 = 6.2
    z0 = 0
    B_phi0 = 5.3
    psi_mult = 200
    N_TFcoils = 18
    minor_radius = 2
    ripple_penetration = 0.2
    ripple_strength = 0.5

    # Magnetic field grid if splines are used
    Rmin =  4; Rmax = 8.5; nR  = 120
    pmin =  0; pmax = 360; nph = 360
    zmin = -4; zmax =   4; nz  = 200

    # Generate input of type that was requested
    if not settings["bfield_use_splines"]:
        if not settings["bfield_make_3D"]:
            # Analytic 2D equilibrium
            B_GS.write_hdf5(fn, R0, z0, B_phi0, psi_mult, psi_coeff)
        else :
            # Analytic 2D equilibrium and analytic ripple
            B_GS.write_hdf5(fn, R0, z0, B_phi0, psi_mult, psi_coeff,
                            nripple=N_TFcoils, a0=minor_radius,
                            alpha0=ripple_penetration, delta0=ripple_strength)
    else:
        if not settings["bfield_make_3D"]:
            # Analytic equilibrium represented with 2D splines
            B_GS.write_hdf5_B_2DS(fn, R0, z0, B_phi0, psi_mult, psi_coeff,
                                  Rmin, Rmax, nR, zmin, zmax, nz)
        else :
            # Analytic equilibrium represented with 3D splines
            B_GS.write_hdf5_B_3DS(fn, R0, z0, B_phi0, psi_mult, psi_coeff,
                                  N_TFcoils, minor_radius,
                                  ripple_penetration, ripple_strength,
                                  Rmin, Rmax, nR, zmin,
                                  zmax, nz, pmin, pmax, nph)

    # Electric field that is zero everywhere
    Exyz = np.array([0, 0, 0])
    E_TC.write_hdf5(fn, Exyz)

    # Plasma data: uniform density through 0 to 1 and exponential decay after
    dens   = 1e20
    temp   = 1e3
    Nrho   = 100
    Nion   = 1
    anum   = np.array([1])
    znum   = np.array([1])
    mass   = np.array([1])
    charge = np.array([1])
    rho = np.transpose(np.linspace(0,2,num=Nrho))
    plasmaprofile = np.ones((len(rho),1))
    edens = dens*plasmaprofile
    etemp = temp*plasmaprofile
    idens = dens*plasmaprofile
    itemp = temp*plasmaprofile

    edens[rho>1] = 1
    idens[rho>1] = 1
    plasma_1D.write_hdf5(fn, Nrho, Nion, anum, znum, mass, charge, rho, edens,
                         etemp, idens, itemp)

    # Square wall around the plasma
    wall_nseg = 20
    wall_R    = np.concatenate( (np.linspace(4.1, 8.4, 6)[1:],
                                 np.linspace(8.4, 8.4, 6)[1:],
                                 np.linspace(8.4, 4.1, 6)[1:],
                                 np.linspace(4.1, 4.1, 6)[1:]) )
    wall_z    = np.concatenate( (np.linspace(-3.9, -3.9, 6)[1:],
                                 np.linspace(-3.9, 3.9, 6)[1:],
                                 np.linspace(3.9, 3.9, 6)[1:],
                                 np.linspace(3.9, -3.9, 6)[1:]) )
    wall_nphi = 10

    # If requested, make it 3D by wrapping 2D wall around torus
    if not settings["wall_make_3D"]:
        wall_2D.write_hdf5(fn, wall_nseg, wall_R, wall_z)
    else :
        wall_2D.write_hdf5_3D(fn, wall_nseg, wall_R, wall_z, wall_nphi)

    # Dummy neutral data
    N0_3D.write_hdf5_dummy(fn)

    # Particle input
    Nmrk   = settings["markers"]
    ids    = np.linspace(1,Nmrk,Nmrk)
    mass   = 4*np.ones(ids.shape)
    charge = 2*np.ones(ids.shape)
    anum   = 4*np.ones(ids.shape)
    znum   = 2*np.ones(ids.shape)
    R      = 3.5 + 2*rand(ids.shape)
    phi    = 360*rand(ids.shape)
    z      = 0*np.ones(ids.shape)
    weight = 1*np.ones(ids.shape)
    time   = 0*np.ones(ids.shape)
    energy = 3.5e6*np.ones(ids.shape)
    pitch  = 0.999-1.999*rand(ids.shape)
    theta  = 2*np.pi*rand(ids.shape)

    gamma = 1 + energy * const["elementary charge"][0] \
            / ( const["alpha particle mass"][0] \
                * np.power(const["speed of light in vacuum"][0],2) )
    v = np.sqrt(1-1/(gamma*gamma))*const["speed of light in vacuum"][0]
    vR = np.sqrt(1-pitch*pitch)*v
    vphi = pitch*v
    vz = 0*v
    mrk_prt.write_hdf5(fn, Nmrk, ids, mass, charge, R, phi, z, vR, vphi, vz,
                       anum, znum, weight, time)

    # Set options to null state
    o = options.generateopt()
    o = flagsToZero(o,"ENABLE")
    o = flagsToZero(o,"ENDCOND")
    o["RECORD_MODE"] = np.array([0],dtype='i4')

    # Set slowing-down simulation options

    o["SIM_MODE"]               = 1
    o["FIXEDSTEP_USERDEFINED"]  = 1.0e-9
    if settings["sim_gc_mode"]:
        o["SIM_MODE"]              = 2
        o["FIXEDSTEP_USERDEFINED"] = 5.0e-8

    if settings["sim_use_hybrid"]:
        o["SIM_MODE"]               = 3
        o["FIXEDSTEP_USERDEFINED"]  = 5.0e-8

    if settings["sim_use_adaptivestep"]:
        o["ENABLE_ADAPTIVE"] = 1

    if settings["sim_recordGCasGO"]:
        o["RECORD_MODE"] = 1

    o["FIXEDSTEP_USE_USERDEFINED"] = 1
    o["ADAPTIVE_TOL_ORBIT"]        = 1.0e-8
    o["ADAPTIVE_TOL_CCOL"]         = 1.0e-2
    o["ADAPTIVE_MAX_DRHO"]         = 1.0
    o["ADAPTIVE_MAX_DPHI"]         = 1.0

    o["ENDCOND_SIMTIMELIM"] = 1
    o["ENDCOND_CPUTIMELIM"] = 1
    o["ENDCOND_RHOLIM"]     = 1
    o["ENDCOND_ENERGYLIM"]  = 1
    o["ENDCOND_WALLHIT"]    = 1

    o["ENDCOND_MAX_SIMTIME"]  = 1.0e-4
    o["ENDCOND_MAX_CPUTIME"]  = 1.0e3
    o["ENDCOND_MAX_RHO"]      = 10.0
    o["ENDCOND_MIN_RHO"]      = 0.7
    o["ENDCOND_MIN_ENERGY"]   = 1.99e3
    o["ENDCOND_MIN_THERMAL"]  = 2.0

    o["ENABLE_ORBIT_FOLLOWING"]    = 1
    o["ENABLE_COULOMB_COLLISIONS"] = 1

    # All distributions on
    o["ENABLE_DIST_5D"]    = 1
    o["ENABLE_DIST_6D"]    = 1
    o["ENABLE_DIST_RHO5D"] = 1
    o["ENABLE_DIST_RHO6D"] = 1

    o["DIST_MIN_R"]    = 3.5
    o["DIST_MAX_R"]    = 8.5
    o["DIST_NBIN_R"]   = 12

    o["DIST_MIN_PHI"]  = 0
    o["DIST_MAX_PHI"]  = 360
    o["DIST_NBIN_PHI"] = 20

    o["DIST_MIN_Z"]    = -2.45
    o["DIST_MAX_Z"]    = 2.45
    o["DIST_NBIN_Z"]   = 24

    o["DIST_MIN_RHO"]  = 0
    o["DIST_MAX_RHO"]  = 1
    o["DIST_NBIN_RHO"] = 11

    o["DIST_MIN_THETA"]  = 0
    o["DIST_MAX_THETA"]  = 360
    o["DIST_NBIN_THETA"] = 13

    o["DIST_MIN_VPA"]  = -1.5e7
    o["DIST_MAX_VPA"]  = 1.5e7
    o["DIST_NBIN_VPA"] = 36

    o["DIST_MIN_VPE"]  = 0
    o["DIST_MAX_VPE"]  = 1.5e7
    o["DIST_NBIN_VPE"] = 18

    o["DIST_MIN_VR"]    = -1.5e7
    o["DIST_MAX_VR"]    = 1.5e7
    o["DIST_NBIN_VR"]   = 14

    o["DIST_MIN_VPHI"]  = -1.5e7
    o["DIST_MAX_VPHI"]  = 1.5e7
    o["DIST_NBIN_VPHI"] = 15

    o["DIST_MIN_VZ"]    = -1.5e7
    o["DIST_MAX_VZ"]    = 1.5e7
    o["DIST_NBIN_VZ"]   = 16

    o["DIST_MIN_TIME"]    = 0
    o["DIST_MAX_TIME"]    = 3e-2
    o["DIST_NBIN_TIME"]   = 2

    o["DIST_MIN_CHARGE"]    = -100
    o["DIST_MAX_CHARGE"]    = 100
    o["DIST_NBIN_CHARGE"]   = 1

    o["ENABLE_ORBITWRITE"]    = 1
    o["ORBITWRITE_MODE"]      = 1
    o["ORBITWRITE_NPOINT"]    = 100
    o["ORBITWRITE_INTERVAL"]  = 0

    options.write_hdf5(fn, o)

def flagsToZero(options,flags):
    for i in options:
        if i.startswith(flags):
            options[i] = np.array([0],dtype='i4')
    return options

if __name__ == '__main__':
    make_ascot5_hello_world()
