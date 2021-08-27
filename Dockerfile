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
# This is a completely serial version with no MPI or OpenMP offloading to GPUs
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

# TODO put a requirements.txt file in repo instead of listing these here
RUN pip install meshio[all] click jupyter

##################################
# Phaethon Development Environment
##################################
FROM phaethon-deps as phaethon-dev

ENV METHOD=devel
WORKDIR /home/moose
# Update libMesh and PETSc
RUN ./scripts/update_and_rebuild_petsc.sh --prefix=/home/petsc && \
    ./scripts/update_and_rebuild_libmesh.sh --with-mpi
# This is needed or it mpiexec complains because docker runs as root
# Discussion on this issue https://github.com/open-mpi/ompi/issues/4451
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
# Re-build MOOSE framework in devel mode
RUN cd test && make -j4 && ./run_tests -j4
RUN cd modules && make -j4 && ./run_tests -j4

RUN apt-get -y install clang-format curl hdf5-tools
RUN curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash && \
    apt-get -y install git-lfs && \
    git lfs install

ENV OMPI_ALLOW_RUN_AS_ROOT=
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=

# Python dev deps
RUN python -m pip install -U flake8 --user
ENV PATH="/root/.local/bin:${PATH}"

# Add a non-root user so git works inside the dev container
ARG USERNAME=vscode
ARG USER_UID=1002
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

##############################
# Phaethon Release Build Stage
##############################
FROM phaethon-deps as phaethon

WORKDIR /home/phaethon
COPY ./ ./
RUN git clean -dfx && make -j4 && make -C unit/ -j4 
RUN cd unit && ./run_tests
