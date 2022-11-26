# Global configuration

#
# Include details of the OS configuration
#
include $(PATH_TO_TOP)/config

CONFIG :=

ifeq ($(DBG), 1)
CONFIG += DBG
endif

ifeq ($(KDBG), 1)
CONFIG += KDBG
endif

ifeq ($(CONFIG_SMP), 1)
CONFIG += CONFIG_SMP
endif
