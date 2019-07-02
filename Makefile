# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the MSM specific device drivers.
#

ifeq ($(CONFIG_ARCH_KONA), y)
include $(srctree)/techpack/dataipa/config/konadataipa.conf
endif

ifeq ($(CONFIG_ARCH_KONA), y)
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/konadataipaconf.h
endif

ifeq ($(CONFIG_ARCH_LITO), y)
include $(srctree)/techpack/dataipa/config/litodataipa.conf
endif

ifeq ($(CONFIG_ARCH_LITO), y)
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/litodataipaconf.h
endif

obj-$(CONFIG_GSI) += gsi/
obj-$(CONFIG_IPA3) += ipa/
