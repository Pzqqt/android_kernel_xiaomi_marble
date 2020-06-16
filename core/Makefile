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

#obj-m := rmnet_core.o rmnet_ctl.o

rmnet_core-y += 	rmnet_config.o \
			rmnet_descriptor.o \
			rmnet_genl.o \
			rmnet_handlers.o \
			rmnet_map_command.o \
			rmnet_map_data.o \
			rmnet_vnd.o \
			dfc_qmap.c \
			dfc_qmi.c \
			qmi_rmnet.0 \
			wda_qmi.0

rmnet_ctl-y += 		rmnet_ctl_client.o \
			rmnet_ctl_ipa.o \
			rmnet_ctl_mhi.o

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean