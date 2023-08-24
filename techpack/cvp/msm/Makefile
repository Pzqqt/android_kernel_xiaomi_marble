# SPDX-License-Identifier: GPL-2.0-only
ccflags-y += -I$(srctree)/techpack/eva/msm/eva/ \
    -I$(srctree)/drivers/media/platform/msm/synx/

msm-cvp-objs := eva/cvp.o \
                eva/msm_cvp_ioctl.o \
                eva/msm_cvp_platform.o \
                eva/msm_cvp_common.o \
                eva/msm_cvp_core.o \
                eva/msm_cvp.o \
                eva/msm_smem.o \
                eva/msm_cvp_debug.o \
                eva/msm_cvp_res_parse.o \
                eva/cvp_dump.o \
                eva/cvp_hfi.o \
                eva/hfi_response_handler.o \
                eva/hfi_packetization.o \
                eva/cvp_core_hfi.o \
                eva/msm_cvp_clocks.o\
                eva/msm_cvp_dsp.o \
                eva/msm_cvp_buf.o \
                eva/msm_cvp_synx.o \
		eva/cvp_fw_load.o

obj-$(CONFIG_MSM_CVP) := msm-cvp.o

