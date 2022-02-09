#!/usr/bin/env python
# coding: utf-8

import a5py.ascot5io.ascot5 as ascot5

# Get the endstate results from the HDF5 file
hdf5 = ascot5.Ascot('simple_run_quick.h5')
endstate = hdf5.active.endstate

# Double/floating-point data for endstate
fp_data = ('mass', 'rprt', 'phiprt', 'zprt', 'vr', 'vphi', 'vz', 'weight',
           'time')

# Integer data for endstate
integer_data = ('id', 'charge', 'anum', 'znum')

# Write the fp data to file as a C++ unordered map
with open('simple_run_quick_endstate_fp.txt', 'w') as fp_file:
    for field in fp_data:
        data_string = ', '.join([f'{x:e}' for x in endstate.get(field)])
        dataline = f'{{"{field}", {{{data_string}}}}},\n'
        fp_file.write(dataline)

# Write the fp data to file as a C++ unordered map
with open('simple_run_quick_endstate_int.txt', 'w') as int_file:
    for field in integer_data:
        data_string = ', '.join([f'{int(x)}' for x in endstate.get(field)])
        dataline = f'{{"{field}", {{{data_string}}}}},\n'
        int_file.write(dataline)
