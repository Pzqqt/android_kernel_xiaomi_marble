ifeq ($(RELEASE_PACKAGE),1)
EXTRA_CFLAGS+=-DRELEASE_PACKAGE
endif
LBITS ?= $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
CCFLAGS += -m64
EXTRA_CFLAGS+=-DSYSTEM_IS_64
else
CCFLAGS += -m32
endif

M ?= $(shell pwd)
#obj-m := rmnet_core.o rmnet_ctl.o

rmnet_core-y += 	rmnet_config.o \
			rmnet_descriptor.o \
			rmnet_genl.o \
			rmnet_handlers.o \
			rmnet_map_command.o \
			rmnet_map_data.o \
			rmnet_module.o \
			rmnet_vnd.o \
			dfc_qmap.c \
			dfc_qmi.c \
			qmi_rmnet.0 \
			wda_qmi.0 \
			rmnet_ll.o \
			rmnet_ll_ipa.o \
			rmnet_qmap.o \
			rmnet_ll_qmap.o

ifneq (, $(filter y, $(CONFIG_ARCH_LAHAINA) $(CONFIG_ARCH_WAIPIO)))
rmnet_ctl-y += 		rmnet_ctl_client.o \
			rmnet_ctl_ipa.o \
			rmnet_ctl_mhi.o
endif

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

KBUILD_OPTIONS := RMNET_CORE_ROOT=$(PWD)
KBUILD_OPTIONS += MODNAME?=rmnet_core

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
