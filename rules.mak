# Default to half-verbose mode
ifeq ($(VERBOSE),no)
  Q = @
  HALFVERBOSEECHO = @:
  # Do not print "Entering directory ..."
  export MAKEFLAGS += --no-print-directory
  # Be silent
  export MAKEFLAGS += --silent
else
ifeq ($(VERBOSE),yes)
  Q =
  HALFVERBOSEECHO = @:
else
  Q = @
  # the following is a hack to get the target name for wine dlls
  # it's disabled because it produces warnings about overriden rules for author.c
  #ifeq ($(TARGET_TYPE),winedll)
  #  export TOOLS_PATH = $(PATH_TO_TOP)/tools
  #  -include Makefile.ros
  #endif
  ifeq ($(TARGET_NAME),)
    HALFVERBOSEECHO = @echo
  else
    HALFVERBOSEECHO = @echo $(TARGET_NAME):
  endif
  # Do not print "Entering directory ..."
  export MAKEFLAGS += --no-print-directory
  # Be silent
  export MAKEFLAGS += --silent
endif
endif

export MAKE := @$(MAKE)

ifeq ($(VERBOSE),no)
endif

# detect Windows host environment
ifeq ($(HOST),)
ifeq ($(word 1,$(shell gcc -dumpmachine)),mingw32)
export HOST=mingw32-windows
else
export HOST=mingw32-linux
endif
endif

# Default to building map files which includes source and asm code
# Other options are: yes
ifeq ($(BUILD_MAP),)
export BUILD_MAP = full
endif

# Default to dumping .sym files out of .nostrip files
ifeq ($(BUILD_SYM),)
export BUILD_SYM = yes
endif

# Default to minimal dependencies, making components not
# depend on all import libraries
ifeq ($(MINIMALDEPENDENCIES),)
export MINIMALDEPENDENCIES = yes
endif

# Default to no PCH support
ifeq ($(ROS_USE_PCH),)
export ROS_USE_PCH = no
endif

# uncomment if you use bochs and it displays only 30 rows
# BOCHS_30ROWS = yes

#
# Choose various options
#
ifeq ($(HOST),mingw32-linux)
export NASM_FORMAT = win32
export PREFIX = mingw32-
export EXE_POSTFIX :=
export EXE_PREFIX := ./
export DLLTOOL = $(PREFIX)dlltool --as=$(PREFIX)as
#
# Do not change NASM_CMD to NASM because older versions of 
# nasm doesn't like an environment variable NASM
#
export NASM_CMD = nasm
export DOSCLI =
export FLOPPY_DIR = /mnt/floppy
export SEP := /
export PIPE :=
endif

ifeq ($(HOST),mingw32-windows)
export NASM_FORMAT = win32
export PREFIX =
export EXE_PREFIX :=
export EXE_POSTFIX := .exe
export DLLTOOL = $(Q)$(PREFIX)dlltool --as=$(PREFIX)as
#
# Do not change NASM_CMD to NASM because older versions of 
# nasm doesn't like an environment variable NASM
#
export NASM_CMD = $(Q)nasmw
export DOSCLI = yes
export FLOPPY_DIR = A:
export SEP := \$(EMPTY_VAR)
export PIPE := -pipe
endif

# TOPDIR is used by make bootcd but not defined anywhere.  Usurp pointed out
# that it has the same meaning as PATH_TO_TOP.
export TOPDIR = $(PATH_TO_TOP)

# Directory to build a bootable CD image in
export BOOTCD_DIR=$(TOPDIR)/../bootcd/disk
export LIVECD_DIR=$(TOPDIR)/../livecd/disk

ifeq ($(LIVECD_INSTALL),yes)
export INSTALL_DIR=$(LIVECD_DIR)/reactos
else
# Use environment var ROS_INSTALL to override default install dir
ifeq ($(ROS_INSTALL),)
ifeq ($(HOST),mingw32-windows)
export INSTALL_DIR = C:/reactos
else
export INSTALL_DIR = $(PATH_TO_TOP)/reactos
endif
else
export INSTALL_DIR = $(ROS_INSTALL)
endif
endif


export CC = $(Q)$(PREFIX)gcc
export CXX = $(Q)$(PREFIX)g++
export HOST_CC = $(Q)gcc
export HOST_CXX = $(Q)g++
export HOST_AR = $(Q)ar
export HOST_NM = $(Q)nm
export LD = $(Q)$(PREFIX)ld
export NM = $(Q)$(PREFIX)nm
export OBJCOPY = $(Q)$(PREFIX)objcopy
export STRIP = $(Q)$(PREFIX)strip
export AS = $(Q)$(PREFIX)gcc -c -x assembler-with-cpp
export CPP = $(Q)$(PREFIX)cpp
export AR = $(Q)$(PREFIX)ar
export RC = $(Q)$(PREFIX)windres
export WRC = $(Q)$(WINE_TOP)/tools/wrc/wrc
export OBJCOPY = $(Q)$(PREFIX)objcopy
export OBJDUMP =$(Q)$(PREFIX)objdump
export TOOLS_PATH = $(PATH_TO_TOP)/tools
export W32API_PATH = $(PATH_TO_TOP)/w32api
export CP = $(Q)rcopy
export RM = $(Q)rdel
export RLINE = $(Q)rline
export RMDIR = $(Q)rrmdir
export RMKDIR = $(Q)rmkdir
export RSYM = $(Q)rsym
export RTOUCH = $(Q)rtouch
export REGTESTS = $(Q)regtests
export MC = $(Q)wmc
export CABMAN = $(Q)cabman
export WINEBUILD = $(Q)winebuild
export WINE2ROS = $(Q)wine2ros
export MKHIVE = $(Q)mkhive
export CDMAKE = $(Q)cdmake
export BIN2RES = $(Q)bin2res
export XSLTPROC = $(Q)xsltproc
export MS2PS = $(Q)ms2ps
export WRC = $(Q)wrc
export WIDL = $(Q)widl

export PREBUILT_PATH = $(PATH_TO_TOP)/PREBUILT

export STD_CFLAGS = -I$(PATH_TO_TOP)/include -I$(W32API_PATH)/include -pipe -march=$(OARCH) -D_M_IX86
export STD_CPPFLAGS = $(STD_CFLAGS)
# Check for 3GB 
ifeq ($(3GB), 1)
export STD_ASFLAGS = -I$(PATH_TO_TOP)/include -I$(W32API_PATH)/include -D__ASM__ -D_M_IX86 -D__3GB__
else
export STD_ASFLAGS = -I$(PATH_TO_TOP)/include -I$(W32API_PATH)/include -D__ASM__ -D_M_IX86
endif
export STD_RCFLAGS = --include-dir $(PATH_TO_TOP)/include --include-dir $(W32API_PATH)/include
export STD_NFLAGS = -f win32


export DDK_PATH=$(PREBUILT_PATH)
export DDK_PATH_LIB=$(PREBUILT_PATH)

