# $Id: helper.mk 14052 2005-03-14 10:32:28Z gvg $
#
# Helper makefile for ReactOS modules
# Variables this makefile accepts:
#   $TARGET_TYPE       = Type of target:
#                        program = User mode program
#                        proglib = Executable program that have exported functions
#                        dynlink = Dynamic Link Library (DLL)
#                        library = Library that will be linked with other code
#                        driver = Kernel mode driver
#                        export_driver = Kernel mode driver that have exported functions
#                        driver_library = Import library for a driver
#                        kmlibrary = Static kernel-mode library
#                        host_library = Static library for use in the build env
#                        hal = Hardware Abstraction Layer
#                        bootpgm = Boot program
#                        miniport = Kernel mode driver that does not link with ntoskrnl.exe or hal.dll
#                        gdi_driver = Kernel mode graphics driver that link with win32k.sys
#                        subsystem = Kernel subsystem
#                        kmdll = Kernel mode DLL
#                        winedll = DLL imported from wine
#                        kernel = ReactOS kernel
#                        test = ReactOS test
#   $TARGET_APPTYPE    = Application type (windows,native,console).
#                        Required only for TARGET_TYPEs program and proglib
#   $TARGET_NAME       = Base name of output file and .rc, .def, and .edf files
#   $TARGET_OBJECTS    = Object files that compose the module
#   $TARGET_CPPAPP     = C++ application (no,yes) (optional)
#   $TARGET_HEADERS    = Header files that the object files depend on (optional)
#   $TARGET_DEFNAME    = Base name of .def and .edf files (optional)
#   $TARGET_BASENAME   = Base name of output file (overrides $TARGET_NAME if it exists) (optional)
#   $TARGET_EXTENSION  = Extension of the output file (optional)
#   $TARGET_DDKLIBS    = DDK libraries that are to be imported by the module (optional)
#   $TARGET_SDKLIBS    = SDK libraries that are to be imported by the module (optional)
#   $TARGET_LIBS       = Other libraries that are to be imported by the module (optional)
#   $TARGET_GCCLIBS    = GCC libraries imported with -l (optional)
#   $TARGET_LFLAGS     = GCC flags when linking (optional)
#   $TARGET_CFLAGS     = GCC flags (optional)
#   $TARGET_CPPFLAGS   = G++ flags (optional)
#   $TARGET_ASFLAGS    = GCC assembler flags (optional)
#   $TARGET_NFLAGS     = NASM flags (optional)
#   $TARGET_RCFLAGS    = Windres flags (optional)
#   $TARGET_CLEAN      = Files that are part of the clean rule (optional)
#   $TARGET_PATH       = Relative path for *.def, *.edf, and *.rc (optional)
#   $TARGET_BASE       = Default base address (optional)
#   $TARGET_ENTRY      = Entry point (optional)
#   $TARGET_NORC       = Do not include standard resource file (no,yes) (optional)
#   $TARGET_LIBPATH    = Destination path for static libraries (optional)
#   $TARGET_IMPLIBPATH = Destination path for import libraries (optional)
#   $TARGET_INSTALLDIR = Destination path when installed (optional)
#   $TARGET_PCH        = Filename of header to use to generate a PCH if supported by the compiler (optional)
#   $TARGET_BOOTSTRAP  = Whether this file is needed to bootstrap the installation (no,yes) (optional)
#   $TARGET_BOOTSTRAP_NAME = Name on the installation medium (optional)
#   $TARGET_REGTESTS   = This module has regression tests (no,yes) (optional)
#   $TARGET_WINETESTS  = This module Wine regression tests (no,yes) (optional)
#   $TARGET_INSTALL    = Install the file (no,yes) (optional)
#   $SUBDIRS           = Subdirs in which to run make (optional)

include $(PATH_TO_TOP)/config
include $(PATH_TO_TOP)/baseaddress.cfg

ifeq ($(TARGET_PATH),)
TARGET_PATH := .
endif

ifeq ($(ARCH),i386)
 MK_ARCH_ID := _M_IX86
endif

ifeq ($(ARCH),alpha)
 MK_ARCH_ID := _M_ALPHA
endif

ifeq ($(ARCH),mips)
 MK_ARCH_ID := _M_MIPS
endif

ifeq ($(ARCH),powerpc)
 MK_ARCH_ID := _M_PPC
endif

# unknown architecture
ifeq ($(MK_ARCH_ID),)
 MK_ARCH_ID := _M_UNKNOWN
endif

#
# VARIABLES IN USE BY VARIOUS TARGETS
#
# MK_BOOTCDDIR     = Directory on the ReactOS ISO CD in which to place the file (subdir of reactos/)
# MK_CFLAGS        = C compiler command-line flags for this target
# MK_CPPFLAGS      = C++ compiler command-line flags for this target
# MK_DDKLIBS       = Import libraries from the ReactOS DDK to link with
# MK_DEFENTRY      = Module entry point:
#                    _WinMain@16 for windows EXE files that are export libraries
#                    _DriverEntry@8 for .SYS files
#                    _DllMain@12 for .DLL files
#                    _DrvEnableDriver@12 for GDI drivers
#                    _WinMainCRTStartup for Win32 EXE files
#                    _NtProcessStartup@4 for Native EXE files
#                    _mainCRTStartup for Console EXE files
# MK_DEFEXT        = Extension to give compiled modules (.EXE, .DLL, .SYS, .a)
# MK_DISTDIR       = (unused?)
# MK_EXETYPE       = Compiler option packages based on type of PE file (exe, dll)
# MK_IMPLIB        = Whether or not to generate a DLL import stub library (yes, no)
# MK_IMPLIB_EXT    = Extension to give import libraries (.a always)
# MK_IMPLIBDEFPATH = Default place to put the import stub library when built
# MK_IMPLIBONLY    = Whether the target is only an import library (yes, no; used only by generic hal)
# MK_INSTALLDIR    = Where "make install" should put the target, relative to reactos/
# MK_MODE          = Mode the target's code is intended to run in
#                    user - User-mode compiler settings
#                    kernel - Kernel-mode compiler settings
#                    static - Static library compiler settings
# MK_RCFLAGS       = Flags to add to resource compiler command line
# MK_RES_BASE      = Base name of resource files
# MK_SDKLIBS       = Default SDK libriaries to link with
#

ifeq ($(TARGET_TYPE),program)
  MK_MODE := user
  MK_EXETYPE := exe
  MK_DEFEXT := .exe
  MK_DEFENTRY := _DEFINE_TARGET_APPTYPE
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -I.
  MK_CPPFLAGS := -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH :=
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := bin
  MK_BOOTCDDIR := system32
  MK_DISTDIR := apps
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),proglib)
  MK_MODE := user
  MK_EXETYPE := dll
  MK_DEFEXT := .exe
  MK_DEFENTRY := _WinMain@16
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -I.
  MK_CPPFLAGS := -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(SDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := bin
  MK_BOOTCDDIR := system32
  MK_DISTDIR := apps
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),dynlink)
  MK_MODE := user
  MK_EXETYPE := dll
  MK_DEFEXT := .dll
  MK_DEFENTRY := _DllMain@12
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -I.
  MK_CPPFLAGS := -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(SDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_BOOTCDDIR := system32
  MK_DISTDIR := dlls
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),library)
  TARGET_NORC := yes
  MK_MODE := static
  MK_EXETYPE :=
  MK_DEFEXT := .a
  MK_DEFENTRY :=
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -I.
  MK_CPPFLAGS := -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH :=
  MK_IMPLIB_EXT :=
  MK_INSTALLDIR := # none
  MK_BOOTCDDIR := system32
  MK_DISTDIR := # none
  MK_RES_BASE :=
endif

ifeq ($(TARGET_TYPE),kmlibrary)
  TARGET_NORC := yes
  MK_MODE := static
  MK_DEFEXT := .a
  MK_CFLAGS := -I.
  MK_CPPFLAGS := -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  #MK_IMPLIB_EXT :=
endif

ifeq ($(TARGET_TYPE),driver_library)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .dll
  MK_DEFENTRY :=
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -I.
  MK_CPPFLAGS := -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := yes
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := $(DDK_PATH_INC)
  MK_BOOTCDDIR := .
  MK_DISTDIR := # FIXME
  MK_RES_BASE :=
endif

ifeq ($(TARGET_TYPE),host_library)
  TARGET_NORC := yes
  MK_MODE := static
  MK_DEFEXT := .a
  MK_CFLAGS := 
  MK_CPPFLAGS := 
  MK_LIBPATH := .
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH :=
  MK_CC := $(HOST_CC)
  MK_AR := $(HOST_AR)
endif

ifeq ($(TARGET_TYPE),driver)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .sys
  MK_DEFENTRY := _DriverEntry@8
  MK_DDKLIBS := ntoskrnl.a hal.a
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTDRIVER__ -I.
  MK_CPPFLAGS := -D__NTDRIVER__ -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH :=
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32/drivers
  MK_BOOTCDDIR := .
  MK_DISTDIR := drivers
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),export_driver)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .sys
  MK_DEFENTRY := _DriverEntry@8
  MK_DDKLIBS := ntoskrnl.a hal.a
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTDRIVER__ -I.
  MK_CPPFLAGS := -D__NTDRIVER__ -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32/drivers
  MK_BOOTCDDIR := .
  MK_DISTDIR := drivers
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),hal)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .dll
  MK_DEFENTRY := _DriverEntry@8
  MK_DDKLIBS := ntoskrnl.a
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTHAL__ -I.
  MK_CPPFLAGS := -D__NTHAL__ -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_BOOTCDDIR := .
  MK_DISTDIR := dlls
  MK_RES_BASE := $(TARGET_NAME)
  MK_INSTALL_BASENAME := hal
  MK_INSTALL_FULLNAME := hal.dll
  ifeq ($(TARGET_BOOTSTRAP),yes)
    TARGET_BOOTSTRAP_NAME := hal.dll
  else
    TARGET_BOOTSTRAP_NAME := $(TARGET_NAME)$(MK_DEFEXT)
  endif
  TARGET_BOOTSTRAP := yes
endif

ifeq ($(TARGET_TYPE),bootpgm)
  MK_MODE := kernel
  MK_EXETYPE := exe
  MK_DEFEXT := .exe
  MK_DEFENTRY := _DriverEntry@8
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTDRIVER__ -I.
  MK_CPPFLAGS := -D__NTDRIVER__ -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH :=
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_BOOTCDDIR := system32
  MK_DISTDIR := # FIXME
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),miniport)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .sys
  MK_DEFENTRY := _DriverEntry@8
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTDRIVER__ -I.
  MK_CPPFLAGS := -D__NTDRIVER__ -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH :=
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32/drivers
  MK_BOOTCDDIR := .
  MK_DISTDIR := drivers
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),gdi_driver)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .dll
  MK_DEFENTRY := _DrvEnableDriver@12
  MK_DDKLIBS := ntoskrnl.a hal.a win32k.a
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTDRIVER__ -I.
  MK_CPPFLAGS := -D__NTDRIVER__ -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_BOOTCDDIR := .
  MK_DISTDIR := dlls
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),kernel)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .exe
  MK_DEFENTRY := _NtProcessStartup
  MK_DDKLIBS := hal.a
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTOSKRNL__ -I.
  MK_CPPFLAGS := -D__NTOSKRNL__ -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_BOOTCDDIR := .
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),test)
  TARGET_NORC := yes
  MK_MODE := static
  MK_EXETYPE :=
  MK_DEFEXT := .a
  MK_DEFENTRY :=
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -I.
  MK_CPPFLAGS := -I.
  MK_IMPLIB := no
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH :=
  MK_IMPLIB_EXT :=
  MK_INSTALLDIR := # none
  MK_BOOTCDDIR := system32
  MK_DISTDIR := # none
  MK_RES_BASE :=
  TARGET_OBJECTS := _rtstub.o _regtests.o $(TARGET_OBJECTS)
endif

ifeq ($(MK_CC),)
  MK_CC := $(CC)
endif

ifeq ($(MK_AR),)
  MK_AR := $(AR)
endif

# can be overidden with $(CXX) for linkage of c++ executables
LD_CC = $(MK_CC)

ifeq ($(RM_AT_FROM_SYMBOLS),no)
  MK_KILLAT :=
else
  MK_KILLAT := --kill-at
endif

ifeq ($(TARGET_TYPE),program)
  ifeq ($(TARGET_APPTYPE),windows)
    MK_DEFENTRY := _WinMainCRTStartup
    MK_SDKLIBS := ntdll.a kernel32.a gdi32.a user32.a
    TARGET_LFLAGS += -Wl,--subsystem,windows
  endif

  ifeq ($(TARGET_APPTYPE),native)
    MK_DEFENTRY := _NtProcessStartup@4
    MK_SDKLIBS := ntdll.a
    TARGET_LFLAGS += -Wl,--subsystem,native -nostartfiles
  endif

  ifeq ($(TARGET_APPTYPE),console)
    MK_DEFENTRY := _mainCRTStartup
    MK_SDKLIBS :=
    TARGET_LFLAGS += -Wl,--subsystem,console
  endif
endif

ifeq ($(TARGET_TYPE),proglib)
  ifeq ($(TARGET_APPTYPE),windows)
    MK_DEFENTRY := _WinMainCRTStartup
    MK_SDKLIBS := ntdll.a kernel32.a gdi32.a user32.a
    TARGET_LFLAGS += -Wl,--subsystem,windows
  endif

  ifeq ($(TARGET_APPTYPE),native)
    MK_DEFENTRY := _NtProcessStartup@4
    MK_SDKLIBS := ntdll.a
    TARGET_LFLAGS += -Wl,--subsystem,native -nostartfiles
  endif

  ifeq ($(TARGET_APPTYPE),console)
    MK_DEFENTRY := _mainCRTStartup
    MK_SDKLIBS :=
    TARGET_LFLAGS += -Wl,--subsystem,console
  endif
endif

ifeq ($(TARGET_TYPE),subsystem)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .sys
  MK_DEFENTRY := _DriverEntry@8
  MK_DDKLIBS := ntoskrnl.a hal.a
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTDRIVER__ -I.
  MK_CPPFLAGS := -D__NTDRIVER__ -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_DISTDIR := drivers
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),kmdll)
  MK_MODE := kernel
  MK_EXETYPE := dll
  MK_DEFEXT := .dll
  MK_DEFENTRY := 0x0
  MK_DDKLIBS := ntoskrnl.a hal.a
  MK_SDKLIBS :=
  MK_CFLAGS := -D__NTDRIVER__ -I.
  MK_CPPFLAGS := -D__NTDRIVER__ -I.
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(DDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_DISTDIR := drivers
  MK_RES_BASE := $(TARGET_NAME)
endif

ifeq ($(TARGET_TYPE),winedll)
-include Makefile.ros
  MK_GENERATED_MAKEFILE = Makefile.ros
  MK_MODE := user
  MK_EXETYPE := dll
  MK_DEFEXT := .dll
  MK_DEFENTRY := _DllMain@12
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -D__USE_W32API -D_WIN32_IE=0x600 -D_WIN32_WINNT=0x501 -DWINVER=0x501 -D_STDDEF_H -I$(PATH_TO_TOP)/include/wine
  MK_CPPFLAGS := -D__USE_W32API -D_WIN32_IE=0x600 -D_WIN32_WINNT=0x501 -DWINVER=0x501 -D__need_offsetof -I$(PATH_TO_TOP)/include -I$(PATH_TO_TOP)/include/wine
  MK_PREPROC_FOR_RC_FLAGS := -xc -E -DRC_INVOKED -D__USE_W32API -I$(PATH_TO_TOP)/include/wine -I$(PATH_TO_TOP)/include -I$(PATH_TO_TOP)/w32api/include
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(SDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_BOOTCDDIR := system32
  MK_DISTDIR := dlls
  MK_RES_BASE := $(TARGET_NAME)
ifeq ($(TARGET_DEFNAME),)
  MK_DEFBASENAME := $(TARGET_NAME).spec
  MK_SPECDEF := $(MK_DEFBASENAME).def
else
  MK_DEFBASENAME := $(TARGET_DEFNAME)
endif
ifneq ($(TARGET_STUBS),no)
  MK_STUBS_SRC := $(TARGET_NAME).stubs.c
  MK_STUBS_OBJ := $(TARGET_NAME).stubs.o
endif
  MK_RC_BINARIES = $(TARGET_RC_BINARIES)
endif

ifeq ($(TARGET_TYPE),winedrv)
-include Makefile.ros
  MK_GENERATED_MAKEFILE = Makefile.ros
  MK_MODE := user
  MK_EXETYPE := drv
  MK_DEFEXT := .drv
# does this need changing?:
  MK_DEFENTRY := _DllMain@12
  MK_DDKLIBS :=
  MK_SDKLIBS :=
  MK_CFLAGS := -D__USE_W32API -D_WIN32_IE=0x600 -D_WIN32_WINNT=0x501 -DWINVER=0x501 -D__need_offsetof -I$(PATH_TO_TOP)/include/wine
  MK_CPPFLAGS := -D__USE_W32API -D_WIN32_IE=0x600 -D_WIN32_WINNT=0x501 -DWINVER=0x501 -D__need_offsetof -I$(PATH_TO_TOP)/include -I$(PATH_TO_TOP)/include/wine
  MK_RCFLAGS := --define __USE_W32API --include-dir $(PATH_TO_TOP)/include/wine
  MK_IMPLIB := yes
  MK_IMPLIBONLY := no
  MK_IMPLIBDEFPATH := $(SDK_PATH_LIB)
  MK_IMPLIB_EXT := .a
  MK_INSTALLDIR := system32
  MK_BOOTCDDIR := system32
  MK_DISTDIR := dlls
  MK_RES_BASE := $(TARGET_NAME)
ifeq ($(TARGET_DEFNAME),)
  MK_DEFBASENAME := $(TARGET_NAME).spec
  MK_SPECDEF := $(MK_DEFBASENAME).def
else
  MK_DEFBASENAME := $(TARGET_DEFNAME)
endif
  MK_RC_BINARIES = $(TARGET_RC_BINARIES)
endif

ifeq ($(TARGET_RC_SRCS),)
  MK_RES_SRC := $(TARGET_PATH)/$(MK_RES_BASE).rc
  MK_RESOURCE := $(TARGET_PATH)/$(MK_RES_BASE).coff
else
  MK_RES_SRC := $(TARGET_RC_SRCS)
  MK_RESOURCE := $(TARGET_RC_SRCS:.rc=.coff)
endif

ifneq ($(TARGET_INSTALLDIR),)
  MK_INSTALLDIR := $(TARGET_INSTALLDIR)
endif


ifneq ($(BOOTCD_INSTALL),)
  MK_INSTALLDIR := .
endif


ifeq ($(TARGET_LIBPATH),)
  ifeq ($(MK_LIBPATH),)
    MK_LIBPATH := $(SDK_PATH_LIB)
  endif
else
  MK_LIBPATH := $(TARGET_LIBPATH)
endif

ifeq ($(TARGET_IMPLIBPATH),)
  MK_IMPLIBPATH := $(MK_IMPLIBDEFPATH)
else
  MK_IMPLIBPATH := $(TARGET_IMPLIBPATH)
endif


ifeq ($(TARGET_BASENAME),)
  MK_BASENAME := $(TARGET_NAME)
else
  MK_BASENAME := $(TARGET_BASENAME)
endif


ifeq ($(TARGET_EXTENSION),)
  MK_EXT := $(MK_DEFEXT)
else
  MK_EXT := $(TARGET_EXTENSION)
endif


ifeq ($(TARGET_NORC),yes)
  MK_FULLRES :=
else
  MK_FULLRES := $(MK_RESOURCE)
endif

ifneq ($(TARGET_TYPE),winedll)
ifeq ($(TARGET_DEFNAME),)
  MK_DEFBASENAME := $(TARGET_NAME)
else
  MK_DEFBASENAME := $(TARGET_DEFNAME)
endif
endif

MK_DEFNAME := $(TARGET_PATH)/$(MK_DEFBASENAME).def

ifeq ($(MK_MODE),user)
  ifeq ($(MK_EXETYPE),dll)
    MK_DEFBASE := 0x10000000
  else
    MK_DEFBASE := 0x400000
  endif
  ifneq ($(TARGET_SDKLIBS),)
    MK_LIBS := $(addprefix $(SDK_PATH_LIB)/lib, $(TARGET_SDKLIBS))
  else
    MK_LIBS := $(addprefix $(SDK_PATH_LIB)/lib, $(MK_SDKLIBS))
  endif
endif


ifeq ($(MK_MODE),kernel)
  MK_DEFBASE := 0x10000
  MK_LIBS := $(addprefix $(DDK_PATH_LIB)/lib, $(TARGET_DDKLIBS) $(MK_DDKLIBS))
  MK_CFLAGS += -D_SEH_NO_NATIVE_NLG
  MK_CPPFLAGS += -D_SEH_NO_NATIVE_NLG
  MK_LFLAGS += -nostartfiles
ifneq ($(TARGET_TYPE),kernel)
  MK_LFLAGS += -nostdlib
endif
endif

#
# Enable Tree-Wide Optimization.
# Protect uncompatible files here with an ifneq
# if needed, until their problems can be found
#
ifeq ($(DBG), 0)
  MK_CFLAGS += -O2 -Wno-strict-aliasing
  MK_CPPFLAGS += -O2 -Wno-strict-aliasing
endif

ifneq ($(TARGET_LIBS),)
  MK_LIBS := $(TARGET_LIBS) $(MK_LIBS)
endif


ifeq ($(TARGET_BASE),)
  TARGET_BASE := $(MK_DEFBASE)
endif


ifeq ($(TARGET_ENTRY),)
  TARGET_ENTRY := $(MK_DEFENTRY)
endif

#
# Include details of the OS configuration
#
include $(PATH_TO_TOP)/config


TARGET_CFLAGS += $(MK_CFLAGS) $(STD_CFLAGS) -g

TARGET_CPPFLAGS += $(MK_CPPFLAGS) $(STD_CPPFLAGS) -g

TARGET_RCFLAGS += $(MK_RCFLAGS) $(STD_RCFLAGS)

TARGET_ASFLAGS += $(MK_ASFLAGS) $(STD_ASFLAGS) -g

TARGET_NFLAGS += $(MK_NFLAGS) $(STD_NFLAGS)

TARGET_LFLAGS += $(MK_LFLAGS) $(STD_LFLAGS) -g


MK_GCCLIBS := $(addprefix -l, $(TARGET_GCCLIBS))

ifeq ($(MK_MODE),static)
  MK_FULLNAME := $(MK_LIBPATH)/lib$(MK_BASENAME)$(MK_EXT)
else
  MK_FULLNAME := $(MK_BASENAME)$(MK_EXT)
endif

ifeq ($(TARGET_TYPE), kmlibrary)
  MK_FULLNAME := $(DDK_PATH_LIB)/lib$(MK_BASENAME)$(MK_EXT)
endif

MK_IMPLIB_FULLNAME := lib$(MK_BASENAME)$(MK_IMPLIB_EXT)

MK_NOSTRIPNAME := $(MK_BASENAME).nostrip$(MK_EXT)
MK_DEBUGNAME := $(MK_BASENAME).dbg

MK_EXTRADEP := $(filter %.h,$(TARGET_OBJECTS))

ifeq ($(TARGET_TYPE),test)
MK_EXTRADEP += _stubs.o _hooks.o
endif

# We don't want to link header files
MK_OBJECTS := $(filter-out %.h,$(TARGET_OBJECTS)) $(MK_STUBS_OBJ)

# There is problems with C++ applications and ld -r. Ld can cause errors like:
#   reloc refers to symbol `.text$_ZN9CCABCodecC2Ev' which is not being output
ifeq ($(TARGET_CPPAPP),yes)
  MK_STRIPPED_OBJECT := $(MK_OBJECTS)
else
  MK_STRIPPED_OBJECT := $(MK_BASENAME).stripped.o
endif

ifeq ($(TARGET_REGTESTS),yes)
  REGTEST_TARGETS := tests/_hooks.c tests/_regtests.c tests/_stubs.S tests/Makefile.tests tests/_rtstub.c
  MK_REGTESTS_CLEAN := clean_regtests
else
  REGTEST_TARGETS :=
  MK_REGTESTS_CLEAN :=
endif

ifeq ($(TARGET_WINETESTS),yes)
all:
	- $(MAKE) -C winetests
  MK_REGTESTS_CLEAN := clean_winetests
endif

ifeq ($(TARGET_INSTALL),)
 MK_INSTALL := yes
else
 MK_INSTALL := $(TARGET_INSTALL)
endif

ifeq ($(MK_INSTALL_BASENAME),)
  MK_INSTALL_BASENAME := $(MK_BASENAME)
endif
ifeq ($(MK_INSTALL_FULLNAME),)
  MK_INSTALL_FULLNAME := $(MK_FULLNAME)
endif

ifeq ($(MK_IMPLIBONLY),yes)

TARGET_CLEAN += $(MK_IMPLIBPATH)/$(MK_IMPLIB_FULLNAME)

all: $(WINETEST_TARGETS) $(REGTEST_TARGETS) $(MK_IMPLIBPATH)/$(MK_IMPLIB_FULLNAME)

$(MK_IMPLIBPATH)/$(MK_IMPLIB_FULLNAME): $(MK_OBJECTS) $(MK_DEFNAME)
	$(HALFVERBOSEECHO) [DLLTOOL] $(MK_IMPLIB_FULLNAME)
	$(DLLTOOL) \
		--dllname $(MK_FULLNAME) \
		--def $(MK_DEFNAME) \
		--output-lib $(MK_IMPLIBPATH)/$(MK_IMPLIB_FULLNAME) \
		$(MK_KILLAT)

else # MK_IMPLIBONLY

all: $(REGTEST_TARGETS) $(MK_FULLNAME) $(MK_NOSTRIPNAME) $(SUBDIRS:%=%_all)


ifeq ($(MK_IMPLIB),yes)
  MK_EXTRACMD := --def $(MK_DEFNAME)
else
  MK_EXTRACMD :=
endif


# User mode targets
ifeq ($(MK_MODE),user)

ifeq ($(MK_EXETYPE),dll)
  TARGET_LFLAGS += -mdll -Wl,--image-base,$(TARGET_BASE)
  MK_EXTRADEP += $(MK_DEFNAME)
  MK_EXTRACMD2 := -Wl,temp.exp
else
  MK_EXTRACMD2 :=
endif

$(MK_BASENAME).a: $(MK_OBJECTS)
	$(HALFVERBOSEECHO) [AR]      $(MK_BASENAME).a
	$(MK_AR) -rc $(MK_BASENAME).a $(MK_OBJECTS)

$(MK_NOSTRIPNAME): $(MK_EXTRADEP) $(MK_FULLRES) $(MK_BASENAME).a $(MK_LIBS) $(MK_STUBS_SRC) $(MK_STUBS_OBJ)
	$(HALFVERBOSEECHO) [LD]      $(MK_NOSTRIPNAME)
ifeq ($(MK_EXETYPE),dll)
	$(LD_CC) -Wl,--base-file,base.tmp \
		-Wl,--entry,$(TARGET_ENTRY) \
		$(TARGET_LFLAGS) \
		-o junk.tmp \
		$(MK_FULLRES) $(MK_OBJECTS) $(MK_LIBS) $(MK_GCCLIBS)
	- $(RM) junk.tmp
	$(DLLTOOL) --dllname $(MK_FULLNAME) \
		--base-file base.tmp \
		--output-exp temp.exp $(MK_EXTRACMD)
	- $(RM) base.tmp
	$(LD_CC) -Wl,--base-file,base.tmp \
		-Wl,--entry,$(TARGET_ENTRY) \
		$(TARGET_LFLAGS) \
		temp.exp \
		-o junk.tmp \
		$(MK_FULLRES) $(MK_OBJECTS) $(MK_LIBS) $(MK_GCCLIBS)
	- $(RM) junk.tmp
	$(DLLTOOL) --dllname $(MK_FULLNAME) \
		--base-file base.tmp \
		--output-exp temp.exp $(MK_KILLAT) $(MK_EXTRACMD)
	- $(RM) base.tmp
endif
	$(LD_CC) $(TARGET_LFLAGS) \
		-Wl,--entry,$(TARGET_ENTRY) $(MK_EXTRACMD2) \
	  	-o $(MK_NOSTRIPNAME) \
	  	$(MK_FULLRES) $(MK_OBJECTS) $(MK_LIBS) $(MK_GCCLIBS)
	- $(RM) temp.exp
ifeq ($(BUILD_MAP),full)
	$(HALFVERBOSEECHO) [OBJDUMP] $(MK_BASENAME).map
	$(OBJDUMP) -d -S $(MK_NOSTRIPNAME) > $(MK_BASENAME).map
else
ifeq ($(BUILD_MAP),yes)
	$(HALFVERBOSEECHO) [NM]      $(MK_BASENAME).map
	$(NM) --numeric-sort $(MK_NOSTRIPNAME) > $(MK_BASENAME).map
endif
endif

$(MK_FULLNAME): $(MK_NOSTRIPNAME) $(MK_EXTRADEP)
	$(HALFVERBOSEECHO) [RSYM]    $(MK_FULLNAME)
	$(RSYM) $(MK_NOSTRIPNAME) $(MK_FULLNAME)

endif # KM_MODE

# Kernel mode targets
ifeq ($(MK_MODE),kernel)

ifeq ($(MK_IMPLIB),yes)
  MK_EXTRACMD := --def $(MK_DEFNAME)
else
  MK_EXTRACMD :=
endif

$(MK_BASENAME).a: $(MK_OBJECTS)
	$(HALFVERBOSEECHO) [AR]      $(MK_BASENAME).a
	$(MK_AR) -rc $(MK_BASENAME).a $(MK_OBJECTS)

$(MK_NOSTRIPNAME): $(MK_EXTRADEP) $(MK_FULLRES) $(MK_BASENAME).a $(MK_LIBS)
	$(HALFVERBOSEECHO) [LD]      $(MK_NOSTRIPNAME)
	$(LD_CC) -Wl,--base-file,base.tmp \
		-Wl,--entry,$(TARGET_ENTRY) \
		$(TARGET_LFLAGS) \
		-o junk.tmp \
		$(MK_FULLRES) $(MK_OBJECTS) $(MK_LIBS) $(MK_GCCLIBS)
	- $(RM) junk.tmp
	$(DLLTOOL) --dllname $(MK_FULLNAME) \
		--base-file base.tmp \
		--output-exp temp.exp $(MK_EXTRACMD) $(MK_KILLAT)
	- $(RM) base.tmp
	$(LD_CC) $(TARGET_LFLAGS) \
		-Wl,--subsystem,native \
		-Wl,--image-base,$(TARGET_BASE) \
		-Wl,--file-alignment,0x1000 \
		-Wl,--section-alignment,0x1000 \
		-Wl,--entry,$(TARGET_ENTRY) \
		-Wl,temp.exp -mdll \
		-o $(MK_NOSTRIPNAME) \
	  	$(MK_FULLRES) $(MK_OBJECTS) $(MK_LIBS) $(MK_GCCLIBS)
	- $(RM) temp.exp
ifeq ($(BUILD_MAP),full)
	$(HALFVERBOSEECHO) [OBJDUMP] $(MK_BASENAME).map
	$(OBJDUMP) -d -S $(MK_NOSTRIPNAME) > $(MK_BASENAME).map
else
ifeq ($(BUILD_MAP),yes)
	$(HALFVERBOSEECHO) [NM]      $(MK_BASENAME).map
	$(NM) --numeric-sort $(MK_NOSTRIPNAME) > $(MK_BASENAME).map
endif
endif

$(MK_FULLNAME): $(MK_NOSTRIPNAME)
	$(HALFVERBOSEECHO) [RSYM]    $(MK_FULLNAME)
	$(RSYM) $(MK_NOSTRIPNAME) $(MK_FULLNAME)

endif # MK_MODE

# Static library target
ifeq ($(MK_MODE),static)

$(MK_FULLNAME): $(MK_EXTRADEP) $(MK_OBJECTS)
	$(HALFVERBOSEECHO) [AR]      $(MK_FULLNAME)
	$(MK_AR) -rc $(MK_FULLNAME) $(MK_OBJECTS)

# Static libraries dont have a nostrip version
$(MK_NOSTRIPNAME):
	-

.PHONY: $(MK_NOSTRIPNAME)

endif # MK_MODE

endif # MK_IMPLIBONLY


$(MK_FULLRES): $(PATH_TO_TOP)/include/reactos/buildno.h $(MK_RES_SRC)

ifeq ($(MK_DEPENDS),yes)
depends:
else
depends:
endif

ifeq ($(MK_IMPLIB),yes)
$(MK_IMPLIBPATH)/$(MK_IMPLIB_FULLNAME): $(MK_DEFNAME)
	$(HALFVERBOSEECHO) [DLLTOOL] $(MK_IMPLIB_FULLNAME)
	$(DLLTOOL) --dllname $(MK_FULLNAME) \
		--def $(MK_DEFNAME) \
		--output-lib $(MK_IMPLIBPATH)/$(MK_IMPLIB_FULLNAME) \
		$(MK_KILLAT)

implib: $(MK_IMPLIBPATH)/$(MK_IMPLIB_FULLNAME)
else
implib: $(SUBDIRS:%=%_implib)
endif

# Precompiled header support
# When using PCHs, use dependency tracking to keep the .gch files up-to-date.
# When TARGET_PCH is defined, we always want to clean the .gch file, even if
# ROS_USE_PCH is not active. This prevents problems when switching between
# PCH and non-PCH builds.

ifneq ($(TARGET_PCH),)
MK_PCHCLEAN = $(TARGET_PCH).gch
ifeq ($(ROS_USE_PCH),yes)
MK_PCHNAME = $(TARGET_PCH).gch
ifeq ($(TARGET_CPPAPP),yes)
PCH_CC := $(CXX)
else # TARGET_CPPAPP
PCH_CC := $(MK_CC)
endif # TARGET_CPPAPP
else # ROS_USE_PCH
MK_PCHNAME =
endif # ROS_USE_PCH
else # TARGET_PCH
MK_PCHCLEAN =
MK_PCHNAME =
endif # TARGET_PCH

# Be carefull not to clean non-object files
MK_CLEANFILES := $(filter %.o,$(MK_OBJECTS))
MK_CLEANFILTERED := $(MK_OBJECTS:.o=.d) $(TARGET_PCH:.h=.d)
MK_CLEANDEPS := $(join $(dir $(MK_CLEANFILTERED)), $(addprefix ., $(notdir $(MK_CLEANFILTERED))))

clean: $(MK_REGTESTS_CLEAN) $(SUBDIRS:%=%_clean)
	$(HALFVERBOSEECHO) [CLEAN]
	- $(RM) *.o $(MK_PCHCLEAN) $(MK_BASENAME).a $(MK_RESOURCE) \
	  $(MK_FULLNAME) $(MK_NOSTRIPNAME) $(MK_CLEANFILES) $(MK_CLEANDEPS) $(MK_BASENAME).map \
	  junk.tmp base.tmp temp.exp $(MK_RC_BINARIES) $(MK_SPECDEF) $(MK_STUBS_SRC) \
	  $(MK_RES_TEMPS) $(MK_GENERATED_MAKEFILE) $(TARGET_CLEAN)

ifneq ($(TARGET_HEADERS),)
$(TARGET_OBJECTS): $(TARGET_HEADERS)
endif

# install and bootcd rules

ifeq ($(MK_IMPLIBONLY),yes)

# Don't install import libraries

forceinstall:

install:

bootcd:

else # MK_IMPLIBONLY


# Don't install static libraries
ifeq ($(MK_MODE),static)

forceinstall:

install:

bootcd:

else # MK_MODE

ifneq ($(MK_INSTALL),no)

install: forceinstall

else # MK_INSTALL

install:

endif # MK_INSTALL

forceinstall: $(SUBDIRS:%=%_install) $(MK_FULLNAME)
	$(HALFVERBOSEECHO) [INSTALL] $(MK_FULLNAME) to $(MK_INSTALLDIR)/$(MK_INSTALL_FULLNAME)
	-$(CP) $(MK_FULLNAME) $(INSTALL_DIR)/$(MK_INSTALLDIR)/$(MK_INSTALL_FULLNAME)

# Bootstrap files for the bootable CD
ifeq ($(TARGET_BOOTSTRAP),yes)

ifneq ($(TARGET_BOOTSTRAP_NAME),)
MK_BOOTSTRAP_NAME := $(TARGET_BOOTSTRAP_NAME)
else # TARGET_BOOTSTRAP_NAME
MK_BOOTSTRAP_NAME := $(MK_INSTALL_FULLNAME)
endif # TARGET_BOOTSTRAP_NAME

bootcd: $(SUBDIRS:%=%_bootcd)
	- $(CP) $(MK_FULLNAME) $(BOOTCD_DIR)/reactos/$(MK_BOOTCDDIR)/$(MK_BOOTSTRAP_NAME)

else # TARGET_BOOTSTRAP

bootcd:

endif # TARGET_BOOTSTRAP

endif # MK_MODE

endif # MK_IMPLIBONLY

ifeq ($(TARGET_TYPE),winedll)
Makefile.ros: Makefile.in Makefile.ros-template
	$(WINE2ROS) Makefile.in Makefile.ros-template Makefile.ros

$(MK_RC_BINARIES): $(TARGET_RC_BINSRC)
	$(BIN2RES) -f -o $@ $(TARGET_RC_BINSRC)

$(MK_RESOURCE): $(MK_RC_BINARIES)

MK_RES_TEMPS = $(MK_RESOURCE:.coff=.rci) $(MK_RESOURCE:.coff=.res)
endif

REGTEST_TESTS = $(wildcard tests/tests/*.c)

$(REGTEST_TARGETS): $(REGTEST_TESTS) ./tests/stubs.tst
	$(REGTESTS) ./tests/tests ./tests/_regtests.c ./tests/Makefile.tests -e ./tests/_rtstub.c
	$(REGTESTS) -s ./tests/stubs.tst ./tests/_stubs.S ./tests/_hooks.c

clean_regtests:
	- $(MAKE) -C tests TARGET_REGTESTS=no clean
	- $(RM) ./tests/_rtstub.c ./tests/_hooks.c ./tests/_regtests.c ./tests/_stubs.S ./tests/Makefile.tests

clean_winetests:
	- $(MAKE) -C winetests clean

.PHONY: all depends implib clean install dist bootcd depends gen_regtests clean_regtests clean_winetests

ifneq ($(SUBDIRS),)
$(SUBDIRS:%=%_all): %_all:
	$(MAKE) -C $* SUBDIRS= all

$(SUBDIRS:%=%_implib): %_implib:
	$(MAKE) -C $* SUBDIRS= implib

$(SUBDIRS:%=%_test): %_test:
	$(MAKE) -C $* SUBDIRS= test

$(SUBDIRS:%=%_clean): %_clean:
	$(MAKE) -C $* SUBDIRS= clean

$(SUBDIRS:%=%_install): %_install:
	$(MAKE) -C $* SUBDIRS= install

$(SUBDIRS:%=%_dist): %_dist:
	$(MAKE) -C $* SUBDIRS= dist

$(SUBDIRS:%=%_bootcd): %_bootcd:
	$(MAKE) -C $* SUBDIRS= bootcd

.PHONY: $(SUBDIRS:%=%_all) $(SUBDIRS:%=%_implib) $(SUBDIRS:%=%_test) \
        $(SUBDIRS:%=%_clean) $(SUBDIRS:%=%_install) $(SUBDIRS:%=%_dist) \
        $(SUBDIRS:%=%_bootcd)
endif

ifeq ($(TARGET_REGTESTS),yes)
test: all
	$(MAKE) -C tests run
else
test:
	-
endif

ifeq ($(TARGET_TYPE),test)
run: all
	@$(MK_CC) -nostdlib -o _runtest.exe regtests.a $(TARGET_LIBS) _stubs.o \
	$(SDK_PATH_LIB)/librtshared.a $(SDK_PATH_LIB)/libregtests.a $(SDK_PATH_LIB)/libpseh.a \
	_hooks.o -lgcc -lmsvcrt -lntdll
	@$(CP) $(REGTESTS_PATH)/regtests/regtests.dll regtests.dll
	@_runtest.exe
	@$(RM) regtests.dll
	@$(RM) _runtest.exe
endif

%.o: %.c $(MK_PCHNAME)
	$(HALFVERBOSEECHO) [CC]      $<
	$(MK_CC) $(TARGET_CFLAGS) -c $< -o $@
%.o: %.cc $(MK_PCHNAME)
	$(HALFVERBOSEECHO) [CXX]     $<
	$(CXX) $(TARGET_CPPFLAGS) -c $< -o $@
%.o: %.cxx $(MK_PCHNAME)
	$(HALFVERBOSEECHO) [CXX]     $<
	$(CXX) $(TARGET_CPPFLAGS) -c $< -o $@
%.o: %.cpp $(MK_PCHNANE)
	$(HALFVERBOSEECHO) [CXX]     $<
	$(CXX) $(TARGET_CPPFLAGS) -c $< -o $@
%.o: %.S
	$(HALFVERBOSEECHO) [AS]      $<
	$(AS) $(TARGET_ASFLAGS) -c $< -o $@
%.o: %.s
	$(HALFVERBOSEECHO) [AS]      $<
	$(AS) $(TARGET_ASFLAGS) -c $< -o $@
%.o: %.asm
	$(HALFVERBOSEECHO) [NASM]    $<
	$(NASM_CMD) $(TARGET_NFLAGS) $< -o $@
ifeq ($(TARGET_TYPE),winedll)
%.coff: %.rc
	$(HALFVERBOSEECHO) [RC]      $<
	$(MK_CC) $(MK_PREPROC_FOR_RC_FLAGS) $< > $(<:.rc=.rci)
	$(WRC) $(<:.rc=.rci) $(<:.rc=.res)
	$(RM) $(<:.rc=.rci)
	$(RC) $(<:.rc=.res) -o $@
	$(RM) $(<:.rc=.res)
else
%.coff: %.rc
	$(HALFVERBOSEECHO) [RC]      $<
	$(RC) $(TARGET_RCFLAGS) $< -o $@
endif
%.spec.def: %.spec
	$(HALFVERBOSEECHO) [DEF]     $<
	$(WINEBUILD) $(DEFS) -o $@ --def -E $<
%.drv.spec.def: %.spec
	$(HALFVERBOSEECHO) [DEF]     $<
	$(WINEBUILD) $(DEFS) -o $@ --def $<
%.stubs.c: %.spec
	$(HALFVERBOSEECHO) [STUBS]   $<
	$(WINEBUILD) $(DEFS) -o $@ --pedll $<
%.i: %.c
	$(HALFVERBOSEECHO) [CPP]     $<
	$(MK_CC) $(TARGET_CFLAGS) -E $< > $@
%.h.gch: %.h
	$(HALFVERBOSEECHO) [PCH]     $<
	$(PCH_CC) $(CFLAGS) $<
# rule for msvc conversion
%.c: %_msvc.c
	$(HALFVERBOSEECHO) [MS2PS]   $<
	$(subst /,$(SEP),$(MS2PS)) -try try -except except -finally finally < $< > $@

# Kill implicit rule
.o:;

# Compatibility
CFLAGS := $(TARGET_CFLAGS)
NFLAGS := $(TARGET_NFLAGS)
