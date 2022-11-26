PATH_TO_TOP = .

include $(PATH_TO_TOP)/rules.mak
include $(PATH_TO_TOP)/config

# Required to run the system
COMPONENTS = ntoskrnl

all: bootstrap $(COMPONENTS)

bootstrap: dk implib iface_native

depends:

implib: $(COMPONENTS:%=%_implib)

clean: $(COMPONENTS:%=%_clean)

.PHONY: all bootstrap depends implib test clean

#
# Tools
#
tools:


tools_implib:
	

tools_test:
	

tools_clean:


tools_install:

.PHONY: tools tools_implib tools_test tools_clean tools_install


#
# Developer Kits
#
dk:

dk_implib:

dk_clean:


dk_install:

.PHONY: dk dk_implib dk_clean dk_install


#
# Interfaces
#
iface_native:


iface_native_implib:
	
iface_native_test:
	
iface_native_clean:


iface_native_install:

iface_native_bootcd:

.PHONY: iface_native iface_native_implib iface_native_test iface_native_clean \
        iface_native_install iface_native_bootcd


#
# Required system components
#
ntoskrnl: bootstrap
	$(MAKE) -C ntoskrnl

ntoskrnl_implib: dk
	$(MAKE) --silent -C ntoskrnl implib

ntoskrnl_test:
	$(MAKE) -C ntoskrnl test

ntoskrnl_clean:
	$(MAKE) -C ntoskrnl clean

ntoskrnl_install:
	$(MAKE) -C ntoskrnl install

ntoskrnl_bootcd:
	$(MAKE) -C ntoskrnl bootcd

.PHONY: ntoskrnl ntoskrnl_implib ntoskrnl_test \
        ntoskrnl_clean ntoskrnl_install ntoskrnl_bootcd


include $(TOOLS_PATH)/config.mk
