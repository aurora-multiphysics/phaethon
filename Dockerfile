####################
# ASCOT5 Build Stage
####################
# TODO should probably move away from using submodules. It means the version
# information of dependencies is scattered, and it would be better to centralise
# it here.
FROM helenbrooks/moose-ubuntu AS ascot5-moose-ubuntu

# Basic environment
ARG DEBIAN_FRONTEND=noninteractive
WORKDIR /home/ascot5
# Set path to shared library for Python interface to use and other environment
ENV LD_LIBRARY_PATH=/home/ascot5 EDITOR=vim LC_ALL=C.UTF-8 LANG=C.UTF-8

# ASCOT5 main C program and library
RUN apt-get update && apt-get -y install make libhdf5-dev
COPY ascot5 ./
ENV MAKE_OPT='NOGIT=true CC=h5cc MPI=0 FLAGS=-foffload=disable'
RUN make clean ${MAKE_OPT}} && make ascot5_main ${MAKE_OPT} && \
    make libascot ${MAKE_OPT} && \
    ln -s -t /usr/local/bin/ $(readlink -f ascot5_main)

# ASCOT5 Python
RUN apt-get -y install python3-pip 
COPY ascot5-python ./ascot5-python
RUN pip install ascot5-python/a5py

###################################
# Phaethon Dependencies Build Stage
###################################
FROM ascot5-moose-ubuntu AS phaethon-deps

RUN pip install meshio[all] click
