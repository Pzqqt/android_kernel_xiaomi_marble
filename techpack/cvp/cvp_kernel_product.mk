ifneq ($(call is-board-platform-in-list,parrot),true)
PRODUCT_PACKAGES += msm-cvp.ko
endif