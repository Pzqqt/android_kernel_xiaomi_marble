# SPDX-License-Identifier: GPL-2.0-only

# auto-detect subdirs
ifneq ($(CONFIG_ARCH_QTI_VM), y)
ifeq ($(CONFIG_ARCH_LAHAINA), y)
include $(srctree)/techpack/eva/config/waipioeva.conf
LINUXINCLUDE    += -include $(srctree)/techpack/eva/config/waipioevaconf.h
endif
endif

obj-y +=msm/
