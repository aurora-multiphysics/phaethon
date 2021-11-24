#!/bin/bash
# coding: utf-8

# Generate test data from ASCOT5 'simplerun'

python ../../ascot5-python/a5py/a5py/preprocessing/simpleruns.py
mv helloworld.h5 simple_run.h5
python simple_run_quick.py
cp simple_run_quick.h5 simple_run_quick_input.h5

# Run ascot5 on input data

ascot5_main --in simple_run
ascot5_main --in simple_run_quick

# Generate dummy mesh for use in MOOSE app

./a5wall_convert.py simple_run.h5 

# Create other unit test data

python calculate_simple_run_particle_energies.py
python calculate_simple_run_heat_fluxes.py
python extract_simple_run_quick_results.py

# Tidy up
rm simple_run_quick.h5
