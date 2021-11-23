## Some build configurations specific to Phaethon
# This file is automatically included by $(FRAMEWORK_DIR)/app.mk so it doesn't
# need to be included in this project's Makefile

# Additional libraries for HDF5 support
ifdef CONDA_DEFAULT_ENV
ADDITIONAL_LIBS     += -lhdf5_hl_cpp -lhdf5_cpp -lhdf5_hl -lhdf5
else
ADDITIONAL_INCLUDES += -I/usr/include/hdf5/serial
ADDITIONAL_LIBS     += -lhdf5_hl_cpp -lhdf5_cpp -lhdf5_serial_hl -lhdf5_serial
endif

# ASCOT5
ASCOT5_DIR := $(APPLICATION_DIR)/ascot5
ADDITIONAL_DEPEND_LIBS += libascot_main
ADDITIONAL_LIBS += -Wl,-rpath=$(ASCOT5_DIR) -L$(ASCOT5_DIR) -lascot_main
ADDITIONAL_INCLUDES += -I$(ASCOT5_DIR)/include
ASCOT5_OPT := NOGIT=true CC=h5cc MPI=0 FLAGS=-foffload=disable

libascot_main:
	$(MAKE) $(ASCOT5_OPT) -C $(ASCOT5_DIR) libascot_main
