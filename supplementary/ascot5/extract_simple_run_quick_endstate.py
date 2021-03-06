#!/usr/bin/env python
# coding: utf-8

import a5py.ascot5io.ascot5 as ascot5
import sys


def main(h5file='simple_run.h5'):
    # Get the endstate results from the HDF5 file
    hdf5 = ascot5.Ascot(h5file)
    endstate = hdf5.active.endstate

    # Double/floating-point data for endstate
    fp_data = ('mass', 'rprt', 'phiprt', 'zprt', 'vr', 'vphi', 'vz', 'weight',
               'time')

    # Integer data for endstate
    integer_data = ('id', 'charge', 'anum', 'znum', 'endcond')

    basename = h5file.split('.')[0]
    # Write the fp data to file as a C++ unordered map
    with open(basename + '_endstate_fp.txt', 'w') as fp_file:
        for field in fp_data:
            data_string = ', '.join([f'{x:.18f}' for x in
                                    endstate.get(field, SI=False)])
            dataline = f'{{"{field}", {{{data_string}}}}},\n'
            fp_file.write(dataline)

    # Write the fp data to file as a C++ unordered map
    with open(basename + '_endstate_int.txt', 'w') as int_file:
        for field in integer_data:
            endstate_data = endstate.get(field, SI=False)
            # for some reason, endcond is bit shifted by 2 leftwards in the
            # python reading routines ¯\_(ツ)_/¯
            if field == 'endcond':
                endstate_data = endstate_data >> 2
            data_string = ', '.join([f'{int(x)}' for x in endstate_data])
            dataline = f'{{"{field}", {{{data_string}}}}},\n'
            int_file.write(dataline)


if __name__ == '__main__':
    main(sys.argv[1])
