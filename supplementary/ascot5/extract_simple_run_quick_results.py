#!/usr/bin/env python
# coding: utf-8

import a5py.ascot5io.ascot5 as ascot5
import numpy as np

# Get the endstate results from the HDF5 file
hdf5 = ascot5.Ascot('simple_run_quick.h5')
endstate = hdf5.active.endstate

# Save a variety of the arrays to csv files for validating unit test runs
np.savetxt('weight_quick.csv', endstate.get('weight'), newline=', ')
np.savetxt('walltile_quick.csv', endstate.get('walltile'), newline=', ', fmt='%1d')
for vel in ('vr', 'vphi', 'vz'):
    np.savetxt(vel + '_quick.csv', endstate.get(vel), newline=', ')
