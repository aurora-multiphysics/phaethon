#! /bin/bash

conda env export > conda_env_freeze.yml
sed -i "s/a5py==.*/.\/ascot5-python\/a5py/" conda_env_freeze.yml
