# SPDX-License-Identifier: GPL-2.0-only
# Build video kernel driver
ifneq ($(TARGET_KERNEL_DLKM_DISABLE),true)
ifneq ($(TARGET_BOARD_AUTO),true)
ifeq ($(call is-board-platform-in-list,$(TARGET_BOARD_PLATFORM)),true)
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/msm_video.ko

BUILD_VIDEO_TECHPACK_SOURCE := true
endif
endif
endif
