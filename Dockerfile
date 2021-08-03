###################
# MOOSE Build Stage
###################
# TODO for next rebuild:
# 1. replace `apt` with `apt-get` since its UI is more stable
# 2. move the more generic environment variables in ASCOT5 section to the beginning 
FROM ubuntu:18.04 AS moose-ubuntu
# Basic environment
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/London
RUN apt update && apt -y install \
    cmake \
    gcc \
    g++ \
    gfortran \
    git \
    python3 \
    python3-dev \
    python3-distutils \
    wget
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.6 10

# PETSC dependencies
RUN apt -y install \
    bison \
    mpich \
    flex \
    libblas-dev \
    liblapack-dev

# MPI environment variables
ENV CC=mpicc CXX=mpicxx F90=mpif90 F77=mpif77 FC=mpif90

# Get MOOSE source code
RUN cd /home/ && \
    git clone https://github.com/idaholab/moose.git && \
    cd moose && \
    git checkout 0c6666e7dd
WORKDIR /home/moose
ENV MOOSE_JOBS=4

# Make PETSC
RUN ./scripts/update_and_rebuild_petsc.sh --prefix=/home/petsc
ENV PETSC_DIR=/home/petsc

# Make libMesh
RUN ./scripts/update_and_rebuild_libmesh.sh --with-mpi

# Make MOOSE framework
ARG METHOD=opt
RUN ./configure --with-derivative-size=200 --with-ad-indexing-type=global && \
    cd test && \
    make -j4
# This is needed or it mpiexec complains because docker runs as root
# Discussion on this issue https://github.com/open-mpi/ompi/issues/4451
ENV OMPI_ALLOW_RUN_AS_ROOT=1 OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
RUN cd test && ./run_tests -j 4
ENV MOOSE_DIR=/home/moose
# Make MOOSE modules
RUN cd modules && \
    make -j4  && \
    ./run_tests -j 4
# Unset these variables we set before
ENV OMPI_ALLOW_RUN_AS_ROOT=
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=

###################################
# Phaethon Dependencies Build Stage
###################################
# TODO should probably move away from using submodules. It means the version
# information of dependencies is scattered, and it would be better to centralise
# it here.
FROM moose-ubuntu AS phaethon-deps

ARG DEBIAN_FRONTEND=noninteractive
# ASCOT5
WORKDIR /home/ascot5
RUN apt -y install make libhdf5-mpich-dev
COPY ascot5 ./
RUN make NOGIT=true clean && make NOGIT=true ascot5_main MPI=1 && \
    make NOGIT=true libascot && \
    ln -s -t /usr/local/bin/ $(readlink -f ascot5_main)
# Set path to shared library for Python interface to use and other environment variables
# TODO move the more generic ones to the beginning once I need to do a full rebuild of the container
ENV LD_LIBRARY_PATH=/home/ascot5 EDITOR=vim LC_ALL=C.UTF-8 LANG=C.UTF-8

# ASCOT5 Python
RUN apt -y install python3-pip  && \
    update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10
COPY ascot5-python ./ascot5-python
RUN pip install ascot5-python/a5py

# Phaethon dependencies
RUN apt -y install libhdf5-dev
RUN pip install ascot5-python/a5py meshio[all] click
