# auto-detect subdirs
ifeq ($(CONFIG_ARCH_WAIPIO), y)
include $(srctree)/techpack/mmrm/config/waipiommrm.conf
endif

ifeq ($(CONFIG_ARCH_WAIPIO), y)
LINUXINCLUDE    += -include $(srctree)/techpack/mmrm/config/waipiommrmconf.h
endif

obj-y +=driver/
