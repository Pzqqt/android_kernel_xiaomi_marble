# SPDX-License-Identifier: GPL-2.0-only

ifeq ($(CONFIG_ARCH_LAHAINA), y)
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/lahainadataipa.h
ifeq ($(CONFIG_QGKI),y)
include $(srctree)/techpack/dataipa/config/lahainadataipa_QGKI.conf
ifeq ($(CONFIG_DEBUG_KERNEL), y)
include $(srctree)/techpack/dataipa/config/lahainadataipa_debug.conf
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/lahainadataipa_debug.h
endif
else
include $(srctree)/techpack/dataipa/config/lahainadataipa_GKI.conf
endif
endif

obj-$(CONFIG_GSI) += gsi/
obj-$(CONFIG_IPA3) += ipa/
