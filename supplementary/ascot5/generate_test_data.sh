#!/bin/bash
# coding: utf-8

# # Generate test data from ASCOT5 'simplerun'
# 
# Build docker image (if not already in existence) 

cd ../../ascot5
make docker

# Generate the input file using a preprocessing script

python3 ../ascot5-python/a5py/a5py/preprocessing/simpleruns.py

# Run ascot5 on input data

./run_docker helloworld

# Move output to this directory

mv helloworld.h5 ../supplementary/ascot5/simple_run.h5

# # Generate dummy mesh for use in MOOSE app

cd -
./a5wall_to_gmsh.py simple_run.h5
