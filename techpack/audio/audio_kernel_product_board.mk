# Build audio kernel driver
PRODUCT_PACKAGES += $(KERNEL_MODULES_OUT)/q6_notifier_dlkm.ko\
	$(KERNEL_MODULES_OUT)/spf_core_dlkm.ko \
	$(KERNEL_MODULES_OUT)/audpkt_ion_dlkm.ko \
	$(KERNEL_MODULES_OUT)/gpr_dlkm.ko \
	$(KERNEL_MODULES_OUT)/audio_pkt_dlkm.ko \
	$(KERNEL_MODULES_OUT)/q6_dlkm.ko \
	$(KERNEL_MODULES_OUT)/adsp_loader_dlkm.ko \
	$(KERNEL_MODULES_OUT)/audio_prm_dlkm.ko \
	$(KERNEL_MODULES_OUT)/q6_pdr_dlkm.ko \
	$(KERNEL_MODULES_OUT)/pinctrl_lpi_dlkm.ko \
	$(KERNEL_MODULES_OUT)/swr_dlkm.ko \
	$(KERNEL_MODULES_OUT)/swr_ctrl_dlkm.ko \
	$(KERNEL_MODULES_OUT)/snd_event_dlkm.ko \
	$(KERNEL_MODULES_OUT)/wcd_core_dlkm.ko \
	$(KERNEL_MODULES_OUT)/mbhc_dlkm.ko \
	$(KERNEL_MODULES_OUT)/swr_dmic_dlkm.ko \
	$(KERNEL_MODULES_OUT)/wcd9xxx_dlkm.ko \
	$(KERNEL_MODULES_OUT)/swr_haptics_dlkm.ko \
	$(KERNEL_MODULES_OUT)/stub_dlkm.ko \
	$(KERNEL_MODULES_OUT)/machine_dlkm.ko \
	$(KERNEL_MODULES_OUT)/lpass_cdc_wsa2_macro_dlkm.ko \
	$(KERNEL_MODULES_OUT)/lpass_cdc_wsa_macro_dlkm.ko \
	$(KERNEL_MODULES_OUT)/lpass_cdc_va_macro_dlkm.ko \
	$(KERNEL_MODULES_OUT)/lpass_cdc_rx_macro_dlkm.ko \
	$(KERNEL_MODULES_OUT)/lpass_cdc_tx_macro_dlkm.ko \
	$(KERNEL_MODULES_OUT)/lpass_cdc_dlkm.ko \
	$(KERNEL_MODULES_OUT)/wsa883x_dlkm.ko \
	$(KERNEL_MODULES_OUT)/wcd938x_dlkm.ko \
	$(KERNEL_MODULES_OUT)/wcd938x_slave_dlkm.ko \
	$(KERNEL_MODULES_OUT)/wcd937x_dlkm.ko \
	$(KERNEL_MODULES_OUT)/wcd937x_slave_dlkm.ko \
	$(KERNEL_MODULES_OUT)/hdmi_dlkm.ko
