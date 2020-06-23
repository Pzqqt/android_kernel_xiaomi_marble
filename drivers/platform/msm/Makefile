# SPDX-License-Identifier: GPL-2.0-only

ifeq ($(CONFIG_ARCH_LAHAINA), y)
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/dataipa.h
ifeq ($(CONFIG_QGKI),y)
include $(srctree)/techpack/dataipa/config/dataipa_QGKI.conf
ifeq ($(CONFIG_LOCALVERSION), "-qgki-debug")
include $(srctree)/techpack/dataipa/config/dataipa_debug.conf
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/dataipa_debug.h
endif
else
include $(srctree)/techpack/dataipa/config/dataipa_GKI.conf
endif
endif

ifeq ($(CONFIG_ARCH_HOLI), y)
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/dataipa.h
ifeq ($(CONFIG_QGKI),y)
include $(srctree)/techpack/dataipa/config/dataipa_QGKI.conf
ifeq ($(CONFIG_LOCALVERSION), "-qgki-debug")
include $(srctree)/techpack/dataipa/config/dataipa_debug.conf
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/dataipa_debug.h
endif
else
include $(srctree)/techpack/dataipa/config/dataipa_GKI.conf
endif
endif

obj-$(CONFIG_GSI) += gsi/
obj-$(CONFIG_IPA3) += ipa/
