ifeq ($(call is-board-platform-in-list,waipio),true)
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/msm-mmrm.ko
endif
