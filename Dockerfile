###################
# MOOSE Build Stage
###################
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

####################################
## Phaethon Dependencies Build Stage
####################################
#FROM moose-ubuntu AS phaethon-deps
## ASCOT5
#WORKDIR /home/ascot5
#RUN apt -y install make libhdf5-mpich-dev python3-pip
#COPY ascot5 ./
## TODO it should be possible to make ascot5 with MPI given it was done with
## MOOSE above
#RUN make clean && make ascot5_main MPI=0 && make libascot
## ASCOT5 Python
#RUN apt -y install python3-pip
#COPY ascot5-python ./
#RUN pip install ascot5-python/a5py
#