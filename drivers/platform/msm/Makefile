# SPDX-License-Identifier: GPL-2.0-only

#MSMs - GKI
ifeq ($(filter $(CONFIG_ARCH_LAHAINA) $(CONFIG_ARCH_HOLI), y),y)
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


#MDMs
ifeq ($(CONFIG_ARCH_SDXLEMUR), y)
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/dataipa.h
include $(srctree)/techpack/dataipa/config/dataipa_QGKI.conf
ifneq ($(CONFIG_LOCALVERSION), "-perf")
include $(srctree)/techpack/dataipa/config/dataipa_debug.conf
LINUXINCLUDE    += -include $(srctree)/techpack/dataipa/config/dataipa_debug.h
endif
endif

DATAIPADRVTOP = $(srctree)/techpack/dataipa/drivers/platform/msm

ifneq (,$(filter $(CONFIG_IPA3) $(CONFIG_GSI),y m))
LINUXINCLUDE += -I$(DATAIPADRVTOP)/gsi
LINUXINCLUDE += -I$(DATAIPADRVTOP)/gsi/gsihal
LINUXINCLUDE += -I$(DATAIPADRVTOP)/ipa
LINUXINCLUDE += -I$(DATAIPADRVTOP)/ipa/ipa_v3
LINUXINCLUDE += -I$(DATAIPADRVTOP)/ipa/ipa_v3/ipahal
LINUXINCLUDE += -I$(DATAIPADRVTOP)/ipa/ipa_clients
endif

ifneq (,$(filter $(CONFIG_IPA3_REGDUMP),y m))
LINUXINCLUDE += -I$(DATAIPADRVTOP)/ipa/ipa_v3/dump
endif

ifneq (,$(filter $(CONFIG_IPA3_REGDUMP_IPA_4_5),y m))
LINUXINCLUDE += -I$(DATAIPADRVTOP)/ipa/ipa_v3/dump/ipa4.5
endif


obj-$(CONFIG_GSI) += gsi/
obj-$(CONFIG_IPA3) += ipa/
