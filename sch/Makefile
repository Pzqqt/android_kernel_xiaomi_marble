#By default build for CLD
RMNET_SCH_SELECT := CONFIG_RMNET_SCH=m
KBUILD_OPTIONS += $(RMNET_SCH_SELECT)
KBUILD_OPTIONS += $(KBUILD_EXTRA) # Extra config if any

M ?= $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 -C $(KERNEL_SRC) M=$(M) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
