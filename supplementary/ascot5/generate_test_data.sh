#!/bin/bash
# coding: utf-8

# Generate test data from ASCOT5 'simplerun'

python ../../ascot5-python/a5py/a5py/preprocessing/simpleruns.py
mv helloworld.h5 simple_run.h5

# Run ascot5 on input data

ascot5_main --in simple_run

# Generate dummy mesh for use in MOOSE app

./a5wall_convert.py simple_run.h5 

# Create text files from which unit test data was copied

h5ls --data simple_run.h5/results/run_0069271660/endstate/weight > weight.txt