snd-soc-wcd934x-objs := wcd934x.o
snd-soc-wcd9xxx-v2-objs := wcd9xxx-common-v2.o wcd9xxx-resmgr-v2.o wcdcal-hwdep.o wcd9xxx-soc-init.o
snd-soc-wcd-cpe-objs := wcd_cpe_services.o wcd_cpe_core.o
snd-soc-wsa881x-objs := wsa881x.o wsa881x-tables.o wsa881x-regmap.o wsa881x-temp-sensor.o
snd-soc-wcd-mbhc-objs := wcd-mbhc-v2.o wcd-mbhc-adc.o
snd-soc-wcd-dsp-utils-objs := wcd-dsp-utils.o
snd-soc-wcd-dsp-mgr-objs := wcd-dsp-mgr.o
snd-soc-wcd-spi-objs := wcd-spi.o

obj-$(CONFIG_SND_SOC_WCD934X)	+= wcd934x/
ifeq ($(CONFIG_COMMON_CLK_MSM), y)
	obj-$(CONFIG_AUDIO_EXT_CLK)	+= audio-ext-clk.o
endif
ifeq ($(CONFIG_COMMON_CLK_QCOM), y)
	obj-$(CONFIG_AUDIO_EXT_CLK)     += audio-ext-clk-up.o
endif

obj-$(CONFIG_SND_SOC_WCD9XXX_V2) += snd-soc-wcd9xxx-v2.o
obj-$(CONFIG_SND_SOC_WCD_CPE)   += snd-soc-wcd-cpe.o
obj-$(CONFIG_SND_SOC_WCD_MBHC)  += snd-soc-wcd-mbhc.o
obj-$(CONFIG_SND_SOC_WSA881X)	+= snd-soc-wsa881x.o
obj-$(CONFIG_SND_SOC_WCD_DSP_MGR)	+= snd-soc-wcd-dsp-mgr.o snd-soc-wcd-dsp-utils.o
obj-$(CONFIG_SND_SOC_WCD_SPI)  += snd-soc-wcd-spi.o

snd-soc-msm-stub-objs := msm_stub.o
obj-$(CONFIG_SND_SOC_MSM_STUB)  += snd-soc-msm-stub.o
