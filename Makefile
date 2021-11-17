###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
#
###############################################################################
# Use the MOOSE submodule if it exists and MOOSE_DIR is not set
MOOSE_SUBMODULE    := $(CURDIR)/moose
ifneq ($(wildcard $(MOOSE_SUBMODULE)/framework/Makefile),)
  MOOSE_DIR        ?= $(MOOSE_SUBMODULE)
else
  MOOSE_DIR        ?= $(shell dirname `pwd`)/moose
endif

# framework
FRAMEWORK_DIR      := $(MOOSE_DIR)/framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).

ALL_MODULES                 := no

CHEMICAL_REACTIONS          := no
CONTACT                     := no
EXTERNAL_PETSC_SOLVER       := no
FLUID_PROPERTIES            := no
FSI                         := no
FUNCTIONAL_EXPANSION_TOOLS  := no
GEOCHEMISTRY                := no
HEAT_CONDUCTION             := yes
LEVEL_SET                   := no
MISC                        := no
NAVIER_STOKES               := no
PHASE_FIELD                 := no
POROUS_FLOW                 := no
RAY_TRACING                 := no
RDG                         := no
RICHARDS                    := no
STOCHASTIC_TOOLS            := no
TENSOR_MECHANICS            := no
XFEM                        := no

include $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# Additional libraries for HDF5 support
ifdef CONDA_DEFAULT_ENV
ADDITIONAL_LIBS     := -lhdf5_hl_cpp -lhdf5_cpp -lhdf5_hl -lhdf5
else
ADDITIONAL_INCLUDES := -I/usr/include/hdf5/serial
ADDITIONAL_LIBS     := -lhdf5_hl_cpp -lhdf5_cpp -lhdf5_serial_hl -lhdf5_serial
endif

# ASCOT5
ADDITIONAL_DEPEND_LIBS += libascot_main
ADDITIONAL_LIBS += -L$(CURDIR)/ascot5 -lascot_main
ADDITIONAL_INCLUDES += -I$(CURDIR)/ascot5/include
ASCOT5_OPT = "NOGIT=true CC=h5cc MPI=0 FLAGS=-foffload=disable"

libascot_main:
	$(MAKE) $(ASCOT5_OPT) -C ascot5 libascot_main

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := phaethon
BUILD_EXEC         := yes
GEN_REVISION       := no
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here
