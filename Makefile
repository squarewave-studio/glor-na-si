# Project Name
TARGET = GlorNaSi

# DEBUG = 1

USE_DAISYSP_LGPL = 1

# Sources
CPP_SOURCES = GlorNaSi.cpp $(wildcard touch/*.cpp) $(wildcard mound/*.cpp) $(wildcard ui/*.cpp)
C_SOURCES = $(wildcard mound/*.c)
C_INCLUDES = -I$(TOUCHSTRING_DIR)/lib/ -Icommon/

# Library Locations
TOUCHSTRING_DIR ?= $(HOME)/Development/Daisy/TouchString
LIBDAISY_DIR ?= $(TOUCHSTRING_DIR)/lib/libDaisy/
DAISYSP_DIR ?= $(TOUCHSTRING_DIR)/lib/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

CPP_STANDARD = -std=gnu++17

libs:
	cd $(LIBDAISY_DIR) && $(MAKE)
	cd $(DAISYSP_DIR) && $(MAKE)

clean-libs:
	cd $(LIBDAISY_DIR) && $(MAKE) clean
	cd $(DAISYSP_DIR) && $(MAKE) clean
