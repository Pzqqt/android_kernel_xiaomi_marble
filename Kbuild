# SPDX-License-Identifier: GPL-2.0-only

ifneq ($(CONFIG_ARCH_MONACO), y)
KBUILD_CPPFLAGS += -DCONFIG_MSM_MMRM=1
endif

ifeq ($(CONFIG_ARCH_WAIPIO), y)
include $(VIDEO_ROOT)/config/waipio_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/waipio_video.h \
                   -I$(VIDEO_ROOT)/driver/platform/waipio/inc
endif

ifeq ($(CONFIG_ARCH_DIWALI), y)
include $(VIDEO_ROOT)/config/diwali_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/diwali_video.h \
                   -I$(VIDEO_ROOT)/driver/platform/diwali/inc
endif

ifeq ($(CONFIG_ARCH_NEO), y)
include $(VIDEO_ROOT)/config/neo_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/neo_video.h \
                   -I$(VIDEO_ROOT)/driver/platform/neo/inc
endif

ifeq ($(CONFIG_ARCH_PARROT), y)
include $(VIDEO_ROOT)/config/parrot_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/parrot_video.h \
                   -I$(VIDEO_ROOT)/driver/platform/parrot/inc
endif

ifeq ($(CONFIG_ARCH_KHAJE), y)
include $(VIDEO_ROOT)/config/khaje_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/khaje_video.h \
                   -I$(VIDEO_ROOT)/driver/platform/khaje/inc
endif

ifeq ($(CONFIG_ARCH_RAVELIN), y)
include $(VIDEO_ROOT)/config/ravelin_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/ravelin_video.h \
                   -I$(VIDEO_ROOT)/driver/platform/ravelin/inc
endif

ifeq ($(CONFIG_ARCH_MONACO), y)
include $(VIDEO_ROOT)/config/monaco_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/monaco_video.h \
                   -I$(VIDEO_ROOT)/driver/platform/monaco/inc
endif

LINUXINCLUDE    += -I$(VIDEO_ROOT)/driver/vidc/inc \
                   -I$(VIDEO_ROOT)/driver/platform/common/inc \
                   -I$(VIDEO_ROOT)/include/uapi/vidc

USERINCLUDE     += -I$(VIDEO_ROOT)/include/uapi/vidc/media \
                   -I$(VIDEO_ROOT)/include/uapi/vidc

obj-m += msm_video.o

ifeq ($(CONFIG_MSM_VIDC_WAIPIO), y)
msm_video-objs += driver/platform/waipio/src/msm_vidc_waipio.o
endif

ifeq ($(CONFIG_MSM_VIDC_DIWALI), y)
msm_video-objs += driver/platform/diwali/src/msm_vidc_diwali.o
endif

ifeq ($(CONFIG_MSM_VIDC_NEO), y)
msm_video-objs += driver/platform/neo/src/msm_vidc_neo.o
endif

ifeq ($(CONFIG_MSM_VIDC_PARROT), y)
msm_video-objs += driver/platform/parrot/src/msm_vidc_parrot.o
endif

ifeq ($(CONFIG_MSM_VIDC_KHAJE), y)
msm_video-objs += driver/platform/khaje/src/msm_vidc_khaje.o
endif

ifeq ($(CONFIG_MSM_VIDC_RAVELIN), y)
msm_video-objs += driver/platform/ravelin/src/msm_vidc_ravelin.o
endif

ifeq ($(CONFIG_MSM_VIDC_MONACO), y)
msm_video-objs += driver/platform/monaco/src/msm_vidc_monaco.o
endif

ifeq ($(CONFIG_MSM_VIDC_IRIS2), y)
LINUXINCLUDE   += -I$(VIDEO_ROOT)/driver/variant/iris2/inc
msm_video-objs += driver/variant/iris2/src/msm_vidc_buffer_iris2.o \
                  driver/variant/iris2/src/msm_vidc_power_iris2.o \
                  driver/variant/iris2/src/msm_vidc_iris2.o
endif

# As of now for NEO, IRIS3 & IRIS2 have no changes.
# If any changes are seen, add new iris3 based files later
ifeq ($(CONFIG_MSM_VIDC_IRIS3), y)
LINUXINCLUDE   += -I$(VIDEO_ROOT)/driver/variant/iris2/inc
msm_video-objs += driver/variant/iris2/src/msm_vidc_buffer_iris2.o \
                  driver/variant/iris2/src/msm_vidc_power_iris2.o \
                  driver/variant/iris2/src/msm_vidc_iris2.o
endif

ifeq ($(CONFIG_MSM_VIDC_AR50LT), y)
LINUXINCLUDE    += -I$(VIDEO_ROOT)/driver/variant/ar50lt/inc
msm_video-objs += driver/variant/ar50lt/src/msm_vidc_buffer_ar50lt.o \
                  driver/variant/ar50lt/src/msm_vidc_power_ar50lt.o \
                  driver/variant/ar50lt/src/msm_vidc_ar50lt.o
endif

msm_video-objs += driver/vidc/src/msm_vidc_v4l2.o \
                  driver/vidc/src/msm_vidc_vb2.o \
                  driver/vidc/src/msm_vidc.o \
                  driver/vidc/src/msm_vdec.o \
                  driver/vidc/src/msm_venc.o \
                  driver/vidc/src/msm_vidc_driver.o \
                  driver/vidc/src/msm_vidc_control.o \
                  driver/vidc/src/msm_vidc_buffer.o \
                  driver/vidc/src/msm_vidc_power.o \
                  driver/vidc/src/msm_vidc_probe.o \
                  driver/vidc/src/msm_vidc_dt.o \
                  driver/vidc/src/msm_vidc_debug.o \
                  driver/vidc/src/msm_vidc_memory.o \
                  driver/vidc/src/venus_hfi.o \
                  driver/vidc/src/hfi_packet.o \
                  driver/vidc/src/venus_hfi_response.o \
                  driver/platform/common/src/msm_vidc_platform.o
