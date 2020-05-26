#By default build for CLD
RMNET_OFFLOAD_SELECT := CONFIG_RMNET_OFFLOAD=m
KBUILD_OPTIONS += $(RMNET_OFFLOAD_SELECT)
KBUILD_OPTIONS += $(KBUILD_EXTRA) # Extra config if any

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
