/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <linux/workqueue.h>
#include "ep92.h"

#define EP92_POLL_INTERVAL_OFF_MSEC 2000
#define EP92_POLL_INTERVAL_ON_MSEC  100


#define EP92_RATES (SNDRV_PCM_RATE_32000 |\
	SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |\
	SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000 |\
	SNDRV_PCM_RATE_176400 | SNDRV_PCM_RATE_192000)

#define EP92_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

#define EP92_UEVENT_CTRL_NUM_KEYS  8
#define EP92_UEVENT_AUDIO_NUM_KEYS 9

static const unsigned int ep92_samp_freq_table[8] = {
	32000, 44100, 48000, 88200, 96000, 176400, 192000, 768000
};

static const char hex_to_char[] = {'0', '1', '2', '3', '4', '5', '6', '7',
				   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

struct ep92_uevent_data {
	struct kobject kobj;
	struct kobj_type ktype;
};

static struct kset *ep92_uevent_kset;
static struct ep92_uevent_data *ep92_uevent_ctrl;
static struct ep92_uevent_data *ep92_uevent_audio;

static void ep92_release_uevent_data(struct kobject *kobj)
{
	struct ep92_uevent_data *data = container_of(kobj,
		struct ep92_uevent_data, kobj);

	kfree(data);
}

static int ep92_init_uevent_data(struct ep92_uevent_data *uevent_data,
	char *name)
{
	int ret = -EINVAL;

	if (!uevent_data || !name)
		return ret;

	/* Set kset for kobject before initializing the kobject */
	uevent_data->kobj.kset = ep92_uevent_kset;

	/* Initialize kobject and add it to kernel */
	ret = kobject_init_and_add(&uevent_data->kobj, &uevent_data->ktype,
		NULL, "%s", name);
	if (ret) {
		pr_err("%s: error initializing uevent kernel object: %d",
			__func__, ret);
		kobject_put(&uevent_data->kobj);
		return ret;
	}

	/* Send kobject add event to the system */
	kobject_uevent(&uevent_data->kobj, KOBJ_ADD);

	return ret;
}

/**
 * ep92_destroy_uevent_data - destroy kernel object.
 *
 * @uevent_data: uevent data.
 */
static void ep92_destroy_uevent_data(struct ep92_uevent_data *uevent_data)
{
	if (uevent_data)
		kobject_put(&uevent_data->kobj);
}

static bool ep92_volatile_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case EP92_BI_GENERAL_INFO_0:
	case EP92_BI_GENERAL_INFO_1:
	case EP92_BI_GENERAL_INFO_2:
	case EP92_BI_GENERAL_INFO_3:
	case EP92_BI_GENERAL_INFO_4:
	case EP92_BI_GENERAL_INFO_5:
	case EP92_BI_GENERAL_INFO_6:
	case EP92_GENERAL_CONTROL_0:
	case EP92_GENERAL_CONTROL_1:
	case EP92_GENERAL_CONTROL_2:
	case EP92_GENERAL_CONTROL_3:
	case EP92_GENERAL_CONTROL_4:
	case EP92_AUDIO_INFO_SYSTEM_STATUS_0:
	case EP92_AUDIO_INFO_SYSTEM_STATUS_1:
	case EP92_AUDIO_INFO_AUDIO_STATUS:
	case EP92_AUDIO_INFO_CHANNEL_STATUS_0:
	case EP92_AUDIO_INFO_CHANNEL_STATUS_1:
	case EP92_AUDIO_INFO_CHANNEL_STATUS_2:
	case EP92_AUDIO_INFO_CHANNEL_STATUS_3:
	case EP92_AUDIO_INFO_CHANNEL_STATUS_4:
	case EP92_AUDIO_INFO_ADO_INFO_FRAME_0:
	case EP92_AUDIO_INFO_ADO_INFO_FRAME_1:
	case EP92_AUDIO_INFO_ADO_INFO_FRAME_2:
	case EP92_AUDIO_INFO_ADO_INFO_FRAME_3:
	case EP92_AUDIO_INFO_ADO_INFO_FRAME_4:
	case EP92_AUDIO_INFO_ADO_INFO_FRAME_5:
		return true;
	default:
		return false;
	}
}

static bool ep92_writeable_registers(struct device *dev, unsigned int reg)
{
	if (reg >= EP92_ISP_MODE_ENTER_ISP && reg <= EP92_GENERAL_CONTROL_4)
		return true;

	return false;
}

static bool ep92_readable_registers(struct device *dev, unsigned int reg)
{
	if (reg >= EP92_BI_VENDOR_ID_0 && reg <= EP92_MAX_REGISTER_ADDR)
		return true;

	return false;
}

/* codec private data */
struct ep92_pdata {
	struct regmap        *regmap;
	struct snd_soc_codec *codec;
	struct timer_list    timer;
	struct work_struct   read_status_worker;
	int                  irq;

	struct {
		u8 ctl;
		u8 rx_sel;
		u8 cec_volume;
	} gc; /* General Control block */

	struct {
		u8 system_status_0;
		u8 system_status_1;
		u8 audio_status;
		u8 cs[5];
		u8 cc;
		u8 ca;
	} ai; /* Audio Info block */

	u8 old_mode;
};

/*
 * EP92 Controls
 */

/* enumerated controls */
static const char * const ep92_off_on_text[] = {"Off", "On"};
static const char * const ep92_aud_path_text[] = {"TV", "Speaker"};
static const char * const ep92_rx_sel_text[] = {"Port 0", "Port 1", "Port 2",
	"Res 3", "Res 4", "Res 5", "None", "Res 7"};
static const char * const ep92_cec_mute_text[] = {"Normal", "Muted"};

static const char * const ep92_state_text[] = {"Inactive", "Active"};
static const char * const ep92_avmute_text[] = {"Normal", "Muted"};
static const char * const ep92_layout_text[] = {"Layout 0", "Layout 1"};
static const char * const ep92_mode_text[] = {"LPCM", "Compr"};

SOC_ENUM_SINGLE_DECL(ep92_power_enum, EP92_GENERAL_CONTROL_0,
	EP92_GC_POWER_SHIFT, ep92_off_on_text);
SOC_ENUM_SINGLE_DECL(ep92_audio_path_enum, EP92_GENERAL_CONTROL_0,
	EP92_GC_AUDIO_PATH_SHIFT, ep92_aud_path_text);
SOC_ENUM_SINGLE_DECL(ep92_rx_sel_enum, EP92_GENERAL_CONTROL_1,
	EP92_GC_RX_SEL_SHIFT, ep92_rx_sel_text);
SOC_ENUM_SINGLE_DECL(ep92_arc_en_enum, EP92_GENERAL_CONTROL_0,
	EP92_GC_ARC_EN_SHIFT, ep92_off_on_text);
SOC_ENUM_SINGLE_DECL(ep92_cec_mute_enum, EP92_GENERAL_CONTROL_0,
	EP92_GC_CEC_MUTE_SHIFT, ep92_cec_mute_text);

SOC_ENUM_SINGLE_DECL(ep92_state_enum, EP92_AUDIO_INFO_SYSTEM_STATUS_0,
	EP92_AI_MCLK_ON_SHIFT, ep92_state_text);
SOC_ENUM_SINGLE_DECL(ep92_avmute_enum, EP92_AUDIO_INFO_SYSTEM_STATUS_0,
	EP92_AI_AVMUTE_SHIFT, ep92_avmute_text);
SOC_ENUM_SINGLE_DECL(ep92_layout_enum, EP92_AUDIO_INFO_SYSTEM_STATUS_0,
	EP92_AI_LAYOUT_SHIFT, ep92_layout_text);
SOC_ENUM_SINGLE_DECL(ep92_mode_enum, EP92_AUDIO_INFO_AUDIO_STATUS,
	EP92_AI_STD_ADO_SHIFT, ep92_mode_text);

/* get/set functions */
static int ep92_power_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] =
			(val >> e->shift_l) & EP92_2CHOICE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_off_on_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_power_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	val &= ~EP92_GC_POWER_MASK;
	val |= (ucontrol->value.enumerated.item[0] & EP92_2CHOICE_MASK)
			<< e->shift_l;
	snd_soc_write(codec, e->reg, val);
	ep92->gc.ctl &= ~EP92_GC_POWER_MASK;
	ep92->gc.ctl |= val & EP92_GC_POWER_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_off_on_text[ucontrol->value.enumerated.item[0] &
				 EP92_2CHOICE_MASK]);
	return 0;
}

static int ep92_audio_path_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] = (val >> e->shift_l) &
			EP92_2CHOICE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_aud_path_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_audio_path_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	val &= ~EP92_GC_AUDIO_PATH_MASK;
	val |= (ucontrol->value.enumerated.item[0] & EP92_2CHOICE_MASK)
			<< e->shift_l;
	snd_soc_write(codec, e->reg, val);
	ep92->gc.ctl &= ~EP92_GC_AUDIO_PATH_MASK;
	ep92->gc.ctl |= val & EP92_GC_AUDIO_PATH_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_aud_path_text[ucontrol->value.enumerated.item[0] &
				   EP92_2CHOICE_MASK]);
	return 0;
}

static int ep92_cec_mute_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] = (val >> e->shift_l) &
			EP92_2CHOICE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_cec_mute_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_cec_mute_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	val &= ~EP92_GC_CEC_MUTE_MASK;
	val |= (ucontrol->value.enumerated.item[0] & EP92_2CHOICE_MASK)
			<< e->shift_l;
	snd_soc_write(codec, e->reg, val);
	ep92->gc.ctl &= ~EP92_GC_CEC_MUTE_MASK;
	ep92->gc.ctl |= val & EP92_GC_CEC_MUTE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_cec_mute_text[ucontrol->value.enumerated.item[0] &
				   EP92_2CHOICE_MASK]);
	return 0;
}

static int ep92_arc_en_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] = (val >> e->shift_l) &
			EP92_2CHOICE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_off_on_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_arc_en_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	val &= ~EP92_GC_ARC_EN_MASK;
	val |= (ucontrol->value.enumerated.item[0] & EP92_2CHOICE_MASK)
			<< e->shift_l;
	snd_soc_write(codec, e->reg, val);
	ep92->gc.ctl &= ~EP92_GC_ARC_EN_MASK;
	ep92->gc.ctl |= val & EP92_GC_ARC_EN_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_off_on_text[ucontrol->value.enumerated.item[0] & 0x01]);
	return 0;
}

static int ep92_rx_sel_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] = (val >> e->shift_l) &
			EP92_GC_RX_SEL_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_rx_sel_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_rx_sel_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	val &= ~EP92_GC_RX_SEL_MASK;
	val |= (ucontrol->value.enumerated.item[0] & EP92_GC_RX_SEL_MASK)
			<< e->shift_l;
	snd_soc_write(codec, e->reg, val);
	ep92->gc.rx_sel &= ~EP92_GC_RX_SEL_MASK;
	ep92->gc.rx_sel |= val & EP92_GC_RX_SEL_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_rx_sel_text[ucontrol->value.enumerated.item[0] &
				 EP92_GC_RX_SEL_MASK]);
	return 0;
}

static int ep92_cec_volume_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, mc->reg);
	ucontrol->value.integer.value[0] = (val >> mc->shift) &
			EP92_GC_CEC_VOLUME_MASK;

	pr_debug("%s: volume = %ld\n", __func__,
		ucontrol->value.integer.value[0]);
	return 0;
}

static int ep92_cec_volume_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;

	val = ucontrol->value.integer.value[0] & EP92_GC_CEC_VOLUME_MASK;
	if (val > EP92_GC_CEC_VOLUME_MAX)
		val = EP92_GC_CEC_VOLUME_MAX;
	snd_soc_write(codec, mc->reg, val);
	ep92->gc.cec_volume = val;

	pr_debug("%s: volume = %ld\n", __func__,
		ucontrol->value.integer.value[0]);
	return 0;
}

static int ep92_state_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] = (val >> e->shift_l) &
			EP92_2CHOICE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_state_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_avmute_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] = (val >> e->shift_l) &
			EP92_2CHOICE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_avmute_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_layout_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	ucontrol->value.enumerated.item[0] = (val >> e->shift_l) &
			EP92_2CHOICE_MASK;

	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_layout_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_mode_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);
	unsigned int val;

	val = snd_soc_read(codec, e->reg);
	if (val & EP92_AI_STD_ADO_MASK) {
		val = snd_soc_read(codec, EP92_AUDIO_INFO_CHANNEL_STATUS_0);
		if (val & EP92_AI_NPCM_MASK)
			ucontrol->value.enumerated.item[0] = 1; /* Compr */
		else
			ucontrol->value.enumerated.item[0] = 0; /* LPCM */
	} else if (val & EP92_AI_HBR_ADO_MASK) {
		ucontrol->value.enumerated.item[0] = 1; /* Compr */
	} else {
		ucontrol->value.enumerated.item[0] = ep92->old_mode;
	}
	pr_debug("%s: item = %d (%s)\n", __func__,
		ucontrol->value.enumerated.item[0],
		ep92_mode_text[ucontrol->value.enumerated.item[0]]);
	return 0;
}

static int ep92_rate_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, mc->reg);
	val &= EP92_AI_RATE_MASK;
	val = ep92_samp_freq_table[val];
	ucontrol->value.integer.value[0] = val;

	pr_debug("%s: rate = %ld\n", __func__,
		ucontrol->value.integer.value[0]);
	return 0;
}

static int ep92_ch_count_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, mc->reg) & EP92_AI_CH_COUNT_MASK;
	/* mapping is ch_count = reg_val + 1, with exception: 0 = unknown */
	if (val > 0)
		val += 1;

	ucontrol->value.integer.value[0] = val;

	pr_debug("%s: ch_count = %ld\n", __func__,
		ucontrol->value.integer.value[0]);
	return 0;
}

static int ep92_ch_alloc_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	unsigned int val;

	val = snd_soc_read(codec, mc->reg);
	ucontrol->value.integer.value[0] = (val >> mc->shift) &
			EP92_AI_CH_ALLOC_MASK;

	pr_debug("%s: ch_alloc = 0x%02lx\n", __func__,
		ucontrol->value.integer.value[0]);
	return 0;
}

static const struct snd_kcontrol_new ep92_snd_controls[] = {

	SOC_ENUM_EXT("HDMI_IN POWER", ep92_power_enum,
		ep92_power_get, ep92_power_put),
	SOC_ENUM_EXT("HDMI_IN AUDIO_PATH", ep92_audio_path_enum,
		ep92_audio_path_get, ep92_audio_path_put),
	SOC_ENUM_EXT("HDMI_IN RX_SEL", ep92_rx_sel_enum,
		ep92_rx_sel_get, ep92_rx_sel_put),
	SOC_ENUM_EXT("HDMI_IN ARC_EN", ep92_arc_en_enum,
		ep92_arc_en_get, ep92_arc_en_put),
	SOC_ENUM_EXT("HDMI_IN CEC_MUTE", ep92_cec_mute_enum,
		ep92_cec_mute_get, ep92_cec_mute_put),
	SOC_SINGLE_EXT("HDMI_IN CEC_VOLUME", EP92_GENERAL_CONTROL_3,
		EP92_GC_CEC_VOLUME_MIN, EP92_GC_CEC_VOLUME_MAX,
		0, ep92_cec_volume_get, ep92_cec_volume_put),

	SOC_ENUM_EXT("HDMI_IN STATE", ep92_state_enum, ep92_state_get, NULL),
	SOC_ENUM_EXT("HDMI_IN AVMUTE", ep92_avmute_enum, ep92_avmute_get, NULL),
	SOC_ENUM_EXT("HDMI_IN LAYOUT", ep92_layout_enum, ep92_layout_get, NULL),
	SOC_ENUM_EXT("HDMI_IN MODE", ep92_mode_enum, ep92_mode_get, NULL),
	SOC_SINGLE_EXT("HDMI_IN RATE", EP92_AUDIO_INFO_AUDIO_STATUS,
		EP92_AI_RATE_MIN, EP92_AI_RATE_MAX, 0, ep92_rate_get, NULL),
	SOC_SINGLE_EXT("HDMI_IN CH_COUNT", EP92_AUDIO_INFO_ADO_INFO_FRAME_1,
		EP92_AI_CH_COUNT_MIN, EP92_AI_CH_COUNT_MAX,
		0, ep92_ch_count_get, NULL),
	SOC_SINGLE_EXT("HDMI_IN CH_ALLOC", EP92_AUDIO_INFO_ADO_INFO_FRAME_4,
		EP92_AI_CH_ALLOC_MIN, EP92_AI_CH_ALLOC_MAX, 0,
		ep92_ch_alloc_get, NULL),
};

static int ep92_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static void ep92_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
}

static int ep92_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

static struct snd_soc_dai_ops ep92_dai_ops = {
	.startup = ep92_startup,
	.shutdown = ep92_shutdown,
	.hw_params = ep92_hw_params,
};

static struct snd_soc_dai_driver ep92_dai[] = {
	{
		.name = "ep92-hdmi",
		.id = 1,
		.capture = {
			.stream_name = "HDMI Capture",
			.rate_max = 192000,
			.rate_min = 32000,
			.channels_min = 1,
			.channels_max = 8,
			.rates = EP92_RATES,
			.formats = EP92_FORMATS,
		},
		.ops = &ep92_dai_ops, /* callbacks */
	},
	{
		.name = "ep92-arc",
		.id = 2,
		.capture = {
			.stream_name = "ARC Capture",
			.rate_max = 192000,
			.rate_min = 32000,
			.channels_min = 1,
			.channels_max = 2,
			.rates = EP92_RATES,
			.formats = EP92_FORMATS,
		},
		.ops = &ep92_dai_ops, /* callbacks */
	},
};

static const char * const ep92_event_power_text[] = {
	"POWER=Off",
	"POWER=On",
};

static const char * const ep92_event_arc_en_text[] = {
	"ARC_EN=Off",
	"ARC_EN=On",
};

static const char * const ep92_event_audio_path_text[] = {
	"AUDIO_PATH=TV",
	"AUDIO_PATH=Speaker",
};

static const char *const ep92_event_rx_sel_text[] = {
	"RX_SEL=Port0",
	"RX_SEL=Port1",
	"RX_SEL=Port2",
	"RX_SEL=Res3",
	"RX_SEL=Res4",
	"RX_SEL=Res5",
	"RX_SEL=None",
	"RX_SEL=Res7",
};

static const char *const ep92_event_cec_mute_text[] = {
	"CEC_MUTE=Normal",
	"CEC_MUTE=Muted",
};

static int ep92_send_uevent_ctrl(struct ep92_pdata *ep92)
{
	char *env[EP92_UEVENT_CTRL_NUM_KEYS];
	u8 idx = 0;
	u8 cec_volume;
	char cec_volume_text[] = "CEC_VOLUME=0x00";
	char *ptr;

	env[idx++] = "HDMI_CONTROL=TRUE";

	if ((ep92->gc.ctl >> EP92_GC_POWER_SHIFT) &
		EP92_2CHOICE_MASK)
		env[idx++] = (char *)ep92_event_power_text[1];
	else
		env[idx++] = (char *)ep92_event_power_text[0];

	if (ep92->gc.ctl & EP92_2CHOICE_MASK)
		env[idx++] = (char *)ep92_event_arc_en_text[1];
	else
		env[idx++] = (char *)ep92_event_arc_en_text[0];

	if ((ep92->gc.ctl >> EP92_GC_AUDIO_PATH_SHIFT) &
		EP92_2CHOICE_MASK)
		env[idx++] = (char *)ep92_event_audio_path_text[1];
	else
		env[idx++] = (char *)ep92_event_audio_path_text[0];

	switch (ep92->gc.rx_sel & EP92_GC_RX_SEL_MASK) {
	case 0:
		env[idx++] = (char *)ep92_event_rx_sel_text[0];
		break;
	case 1:
		env[idx++] = (char *)ep92_event_rx_sel_text[1];
		break;
	case 2:
		env[idx++] = (char *)ep92_event_rx_sel_text[2];
		break;
	case 3:
		env[idx++] = (char *)ep92_event_rx_sel_text[3];
		break;
	case 4:
		env[idx++] = (char *)ep92_event_rx_sel_text[4];
		break;
	case 5:
		env[idx++] = (char *)ep92_event_rx_sel_text[5];
		break;
	case 6:
		env[idx++] = (char *)ep92_event_rx_sel_text[6];
		break;
	case 7:
		env[idx++] = (char *)ep92_event_rx_sel_text[7];
		break;
	default:
		env[idx++] = (char *)ep92_event_rx_sel_text[0];
	}

	if ((ep92->gc.ctl >> EP92_GC_CEC_MUTE_SHIFT) &
		EP92_2CHOICE_MASK)
		env[idx++] = (char *)ep92_event_cec_mute_text[1];
	else
		env[idx++] = (char *)ep92_event_cec_mute_text[0];

	ptr = &cec_volume_text[strlen(cec_volume_text)-2];
	cec_volume = (ep92->ai.ca) & EP92_GC_CEC_VOLUME_MASK;
	*ptr++ = hex_to_char[(cec_volume >> 4) & 0x0f];
	*ptr++ = hex_to_char[cec_volume & 0x0f];
	env[idx++] = (char *)cec_volume_text;

	env[idx++] = NULL;

	if (idx != EP92_UEVENT_CTRL_NUM_KEYS) {
		pr_err("ep92 wrong number of audio uevent keys (%d).\n",
			idx);
		return -EINVAL;
	}

	return kobject_uevent_env(&ep92_uevent_ctrl->kobj, KOBJ_CHANGE, env);
}

static const char * const ep92_event_state_text[] = {
	"STATE=Inactive",
	"STATE=Active",
};

static const char *const ep92_event_rate_text[] = {
	"RATE=32000",
	"RATE=44100",
	"RATE=48000",
	"RATE=88200",
	"RATE=96000",
	"RATE=176400",
	"RATE=192000",
	"RATE=768000",
};

static const char *const ep92_event_format_text[] = {
	"FORMAT=LPCM",
	"FORMAT=Compr",
};

static const char *const ep92_event_layout_text[] = {
	"LAYOUT=2ch",
	"LAYOUT=8ch",
};

static const char *const ep92_event_avmute_text[] = {
	"AVMUTE=Normal",
	"AVMUTE=Muted",
};

static const char *const ep92_event_ch_count_text[] = {
	"CH_COUNT=One",
	"CH_COUNT=Two",
	"CH_COUNT=Three",
	"CH_COUNT=Four",
	"CH_COUNT=Five",
	"CH_COUNT=Six",
	"CH_COUNT=Seven",
	"CH_COUNT=Eight",
};

static int ep92_send_uevent_audio(struct ep92_pdata *ep92)
{
	char *env[EP92_UEVENT_AUDIO_NUM_KEYS];
	u8 idx = 0;
	u8 ch_alloc;
	char ch_alloc_text[] = "CH_ALLOC=0x00";
	char *ptr;

	env[idx++] = "HDMI_FMT_UPDATE=TRUE";

	if (((ep92->ai.system_status_0 >> EP92_AI_MCLK_ON_SHIFT) &
		EP92_2CHOICE_MASK) == EP92_STATUS_AUDIO_ACTIVE)
		env[idx++] = (char *)ep92_event_state_text[1];
	else
		env[idx++] = (char *)ep92_event_state_text[0];

	switch (ep92->ai.audio_status & EP92_AI_RATE_MASK) {
	case 0:
		env[idx++] = (char *)ep92_event_rate_text[0];
		break;
	case 1:
		env[idx++] = (char *)ep92_event_rate_text[1];
		break;
	case 2:
		env[idx++] = (char *)ep92_event_rate_text[2];
		break;
	case 3:
		env[idx++] = (char *)ep92_event_rate_text[3];
		break;
	case 4:
		env[idx++] = (char *)ep92_event_rate_text[4];
		break;
	case 5:
		env[idx++] = (char *)ep92_event_rate_text[5];
		break;
	case 6:
		env[idx++] = (char *)ep92_event_rate_text[6];
		break;
	case 7:
		env[idx++] = (char *)ep92_event_rate_text[7];
		break;
	default:
		env[idx++] = (char *)ep92_event_rate_text[2];
	}

	if (ep92->old_mode)
		env[idx++] = (char *)ep92_event_format_text[1];
	else
		env[idx++] = (char *)ep92_event_format_text[0];

	if (ep92->ai.system_status_0 & EP92_2CHOICE_MASK)
		env[idx++] = (char *)ep92_event_layout_text[1];
	else
		env[idx++] = (char *)ep92_event_layout_text[0];

	if ((ep92->ai.system_status_0 >> EP92_AI_AVMUTE_SHIFT) &
		EP92_2CHOICE_MASK)
		env[idx++] = (char *)ep92_event_avmute_text[1];
	else
		env[idx++] = (char *)ep92_event_avmute_text[0];

	/* cc==0 signals n/a and is treated as stereo */
	switch (ep92->ai.cc & EP92_AI_CH_COUNT_MASK) {
	case 0:
		env[idx++] = (char *)ep92_event_ch_count_text[1];
		break;
	case 1:
		env[idx++] = (char *)ep92_event_ch_count_text[1];
		break;
	case 2:
		env[idx++] = (char *)ep92_event_ch_count_text[2];
		break;
	case 3:
		env[idx++] = (char *)ep92_event_ch_count_text[3];
		break;
	case 4:
		env[idx++] = (char *)ep92_event_ch_count_text[4];
		break;
	case 5:
		env[idx++] = (char *)ep92_event_ch_count_text[5];
		break;
	case 6:
		env[idx++] = (char *)ep92_event_ch_count_text[6];
		break;
	case 7:
		env[idx++] = (char *)ep92_event_ch_count_text[7];
		break;
	default:
		env[idx++] = (char *)ep92_event_ch_count_text[1];
	}

	ptr = &ch_alloc_text[strlen(ch_alloc_text)-2];
	ch_alloc = (ep92->ai.ca) & EP92_AI_CH_ALLOC_MASK;
	*ptr++ = hex_to_char[(ch_alloc >> 4) & 0x0f];
	*ptr++ = hex_to_char[ch_alloc & 0x0f];
	env[idx++] = (char *)ch_alloc_text;

	env[idx++] = NULL;

	if (idx != EP92_UEVENT_AUDIO_NUM_KEYS) {
		pr_err("ep92 wrong number of audio uevent keys (%d).\n",
			idx);
		return -EINVAL;
	}

	return kobject_uevent_env(&ep92_uevent_audio->kobj, KOBJ_CHANGE, env);
}

static void ep92_read_general_control(struct snd_soc_codec *codec,
	struct ep92_pdata *ep92)
{
	u8 old, change;
	bool send_uevent = false;

	old = ep92->gc.ctl;
	ep92->gc.ctl = snd_soc_read(codec, EP92_GENERAL_CONTROL_0);
	change = ep92->gc.ctl ^ old;
	if (change & EP92_GC_POWER_MASK) {
		pr_debug("ep92 power changed to %d (%s)\n",
			(ep92->gc.ctl >> EP92_GC_POWER_SHIFT) &
			EP92_2CHOICE_MASK,
			ep92_off_on_text[(ep92->gc.ctl
			>> EP92_GC_POWER_SHIFT) & EP92_2CHOICE_MASK]);
		send_uevent = true;
	}
	if (change & EP92_GC_AUDIO_PATH_MASK) {
		pr_debug("ep92 audio_path changed to %d (%s)\n",
			(ep92->gc.ctl >> EP92_GC_AUDIO_PATH_SHIFT) &
			EP92_2CHOICE_MASK,
			ep92_aud_path_text[(ep92->gc.ctl
			>> EP92_GC_AUDIO_PATH_SHIFT) & EP92_2CHOICE_MASK]);
		send_uevent = true;
	}
	if (change & EP92_GC_CEC_MUTE_MASK) {
		pr_debug("ep92 cec_mute changed to %d (%s)\n",
			(ep92->gc.ctl >> EP92_GC_CEC_MUTE_SHIFT) &
			EP92_2CHOICE_MASK,
			ep92_cec_mute_text[(ep92->gc.ctl
			>> EP92_GC_CEC_MUTE_SHIFT) & EP92_2CHOICE_MASK]);
		send_uevent = true;
	}
	if (change & EP92_GC_ARC_EN_MASK) {
		pr_debug("ep92 arc_en changed to %d (%s)\n",
			ep92->gc.ctl & EP92_2CHOICE_MASK,
			ep92_off_on_text[ep92->gc.ctl & EP92_2CHOICE_MASK]);
		send_uevent = true;
	}

	old = ep92->gc.rx_sel;
	ep92->gc.rx_sel = snd_soc_read(codec, EP92_GENERAL_CONTROL_1);
	change = ep92->gc.rx_sel ^ old;
	if (change & EP92_GC_RX_SEL_MASK) {
		pr_debug("ep92 rx_sel changed to %d (%s)\n",
			ep92->gc.rx_sel & EP92_GC_RX_SEL_MASK,
			ep92_rx_sel_text[ep92->gc.rx_sel &
			EP92_GC_RX_SEL_MASK]);
		send_uevent = true;
	}

	old = ep92->gc.cec_volume;
	ep92->gc.cec_volume = snd_soc_read(codec, EP92_GENERAL_CONTROL_3);
	change = ep92->gc.cec_volume ^ old;
	if (change & EP92_GC_CEC_VOLUME_MASK) {
		pr_debug("ep92 cec_volume changed to %d\n",
			ep92->gc.cec_volume & EP92_GC_CEC_VOLUME_MASK);
		send_uevent = true;
	}

	if (send_uevent)
		ep92_send_uevent_ctrl(ep92);
}

static void ep92_read_audio_info(struct snd_soc_codec *codec,
	struct ep92_pdata *ep92)
{
	u8 old, change;
	u8 new_mode;
	bool send_uevent = false;

	old = ep92->ai.system_status_0;
	ep92->ai.system_status_0 = snd_soc_read(codec,
		EP92_AUDIO_INFO_SYSTEM_STATUS_0);
	change = ep92->ai.system_status_0 ^ old;
	if (change & EP92_AI_MCLK_ON_MASK) {
		pr_debug("ep92 status changed to %d (%s)\n",
			(ep92->ai.system_status_0 >> EP92_AI_MCLK_ON_SHIFT) &
			EP92_2CHOICE_MASK,
			ep92_state_text[(ep92->ai.system_status_0
			>> EP92_AI_MCLK_ON_SHIFT) & EP92_2CHOICE_MASK]);
		send_uevent = true;
	}
	if (change & EP92_AI_AVMUTE_MASK) {
		pr_debug("ep92 avmute changed to %d (%s)\n",
			(ep92->ai.system_status_0 >> EP92_AI_AVMUTE_SHIFT) &
			EP92_2CHOICE_MASK,
			ep92_avmute_text[(ep92->ai.system_status_0
			>> EP92_AI_AVMUTE_SHIFT) & EP92_2CHOICE_MASK]);
		send_uevent = true;
	}
	if (change & EP92_AI_LAYOUT_MASK) {
		pr_debug("ep92 layout changed to %d (%s)\n",
			(ep92->ai.system_status_0) & EP92_2CHOICE_MASK,
			ep92_layout_text[(ep92->ai.system_status_0) &
			EP92_2CHOICE_MASK]);
		send_uevent = true;
	}

	old = ep92->ai.audio_status;
	ep92->ai.audio_status = snd_soc_read(codec,
		EP92_AUDIO_INFO_AUDIO_STATUS);
	change = ep92->ai.audio_status ^ old;
	if (change & EP92_AI_RATE_MASK) {
		pr_debug("ep92 rate changed to %d\n",
			ep92_samp_freq_table[(ep92->ai.audio_status) &
			EP92_AI_RATE_MASK]);
		send_uevent = true;
	}

	new_mode = ep92->old_mode;
	if (ep92->ai.audio_status & EP92_AI_STD_ADO_MASK) {
		ep92->ai.cs[0] = snd_soc_read(codec,
			EP92_AUDIO_INFO_CHANNEL_STATUS_0);
		if (ep92->ai.cs[0] & EP92_AI_NPCM_MASK)
			new_mode = 1; /* Compr */
		else
			new_mode = 0; /* LPCM */
	} else if (ep92->ai.audio_status & EP92_AI_HBR_ADO_MASK)
		new_mode = 1; /* Compr */

	if (ep92->old_mode != new_mode) {
		pr_debug("ep92 mode changed to %d (%s)\n", new_mode,
			ep92_mode_text[new_mode]);
		send_uevent = true;
	}
	ep92->old_mode = new_mode;

	old = ep92->ai.cc;
	ep92->ai.cc = snd_soc_read(codec, EP92_AUDIO_INFO_ADO_INFO_FRAME_1);
	change = ep92->ai.cc ^ old;
	if (change & EP92_AI_CH_COUNT_MASK) {
		pr_debug("ep92 ch_count changed to %d (%d)\n",
			ep92->ai.cc & EP92_AI_CH_COUNT_MASK,
			(ep92->ai.cc & EP92_AI_CH_COUNT_MASK) == 0 ? 0 :
			(ep92->ai.cc & EP92_AI_CH_COUNT_MASK) + 1);
		send_uevent = true;
	}

	old = ep92->ai.ca;
	ep92->ai.ca = snd_soc_read(codec, EP92_AUDIO_INFO_ADO_INFO_FRAME_4);
	change = ep92->ai.ca ^ old;
	if (change & EP92_AI_CH_ALLOC_MASK) {
		pr_debug("ep92 ch_alloc changed to 0x%02x\n",
			(ep92->ai.ca) & EP92_AI_CH_ALLOC_MASK);
		send_uevent = true;
	}

	if (send_uevent)
		ep92_send_uevent_audio(ep92);
}

static void ep92_init(struct snd_soc_codec *codec, struct ep92_pdata *ep92)
{
	/* update the format information in mixer controls */
	ep92_read_general_control(codec, ep92);
	ep92_read_audio_info(codec, ep92);
}

static int ep92_probe(struct snd_soc_codec *codec)
{
	struct ep92_pdata *ep92 = snd_soc_codec_get_drvdata(codec);

	ep92->codec = codec;
	ep92_init(codec, ep92);

	return 0;
}

static int ep92_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static struct regmap *ep92_get_regmap(struct device *dev)
{
	struct ep92_pdata *ep92_ctrl = dev_get_drvdata(dev);

	if (!ep92_ctrl)
		return NULL;

	return ep92_ctrl->regmap;
}

static struct snd_soc_codec_driver soc_codec_drv_ep92 = {
	.probe  = ep92_probe,
	.remove = ep92_remove,
	.get_regmap = ep92_get_regmap,
	.component_driver = {
		.controls = ep92_snd_controls,
		.num_controls = ARRAY_SIZE(ep92_snd_controls),
	},
};

static struct regmap_config ep92_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.reg_defaults = ep92_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(ep92_reg_defaults),
	.max_register = EP92_MAX_REGISTER_ADDR,
	.volatile_reg = ep92_volatile_register,
	.writeable_reg = ep92_writeable_registers,
	.readable_reg = ep92_readable_registers,
};

void ep92_read_status(struct work_struct *work)
{
	struct ep92_pdata *ep92 = container_of(work, struct ep92_pdata,
		read_status_worker);
	struct snd_soc_codec *codec = ep92->codec;
	u8 val;

	/* No polling before codec is initialized */
	if (codec == NULL)
		return;

	/* check ADO_CHF that is set when audio format has changed */
	val = snd_soc_read(codec, EP92_BI_GENERAL_INFO_1);
	if (val == 0xff) {
		/* workaround for Nak'ed first read */
		val = snd_soc_read(codec, EP92_BI_GENERAL_INFO_1);
		if (val == 0xff)
			return;	/* assume device not present */
	}

	if (val & EP92_GI_ADO_CHF_MASK)
		pr_debug("ep92 audio mode change trigger.\n");

	if (val & EP92_GI_CEC_ECF_MASK)
		pr_debug("ep92 CEC change trigger.\n");

	/* check for general control changes */
	ep92_read_general_control(codec, ep92);

	/* update the format information in mixer controls */
	ep92_read_audio_info(codec, ep92);
}

static irqreturn_t ep92_irq(int irq, void *data)
{
	struct ep92_pdata *ep92 = data;
	struct snd_soc_codec *codec = ep92->codec;

	/* Treat interrupt before codec is initialized as spurious */
	if (codec == NULL)
		return IRQ_NONE;

	dev_dbg(codec->dev, "ep92_interrupt\n");

	schedule_work(&ep92->read_status_worker);

	return IRQ_HANDLED;
};

void ep92_poll_status(unsigned long data)
{
	struct ep92_pdata *ep92 = (struct ep92_pdata *)data;
	u32 poll_msec;

	if ((ep92->gc.ctl & EP92_GC_POWER_MASK) == 0)
		poll_msec = EP92_POLL_INTERVAL_OFF_MSEC;
	else
		poll_msec = EP92_POLL_INTERVAL_ON_MSEC;

	mod_timer(&ep92->timer, jiffies + msecs_to_jiffies(poll_msec));

	schedule_work(&ep92->read_status_worker);
}

static const struct of_device_id ep92_of_match[] = {
	{ .compatible = "explore,ep92a6", },
	{ }
};
MODULE_DEVICE_TABLE(of, ep92_of_match);

static int ep92_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct ep92_pdata *ep92;
	int ret;

	ep92 = devm_kzalloc(&client->dev, sizeof(struct ep92_pdata),
		GFP_KERNEL);
	if (ep92 == NULL)
		return -ENOMEM;

	ep92->regmap = devm_regmap_init_i2c(client, &ep92_regmap_config);
	if (IS_ERR(ep92->regmap)) {
		ret = PTR_ERR(ep92->regmap);
		dev_err(&client->dev,
			"%s %d: Failed to allocate regmap for I2C device: %d\n",
			__func__,  __LINE__, ret);
		return ret;
	}

	i2c_set_clientdata(client, ep92);

	/* register interrupt handler */
	INIT_WORK(&ep92->read_status_worker, ep92_read_status);
	ep92->irq = client->irq;
	if (ep92->irq) {
		ret = devm_request_threaded_irq(&client->dev, ep92->irq,
			NULL, ep92_irq, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
			"ep92_irq", ep92);
		if (ret) {
			dev_err(&client->dev,
				"%s: Failed to request IRQ %d: %d\n",
				__func__, ep92->irq, ret);
			ep92->irq = 0;
		}
	}
	/* poll status if IRQ is not configured */
	if (ep92->irq == 0) {
		setup_timer(&ep92->timer, ep92_poll_status,
			(unsigned long)ep92);
		mod_timer(&ep92->timer, jiffies +
			msecs_to_jiffies(EP92_POLL_INTERVAL_OFF_MSEC));
	}

	/* register codec */
	ret = snd_soc_register_codec(&client->dev, &soc_codec_drv_ep92,
		ep92_dai, ARRAY_SIZE(ep92_dai));
	if (ret) {
		dev_err(&client->dev,
			"%s %d: Failed to register CODEC: %d\n",
			__func__,  __LINE__, ret);
		goto err_reg;
	}

	/* Create a kset under /sys/kernel/ */
	ep92_uevent_kset = kset_create_and_add("ep92-hdmi", NULL, kernel_kobj);
	if (!ep92_uevent_kset) {
		pr_err("%s: error creating uevent kernel set", __func__);
		ret = -EINVAL;
		goto err_kset;
	}

	/* uevent to signal control changes */
	ep92_uevent_ctrl = devm_kzalloc(&client->dev,
		sizeof(*ep92_uevent_ctrl), GFP_KERNEL);
	if (!ep92_uevent_ctrl) {
		ret = -ENOMEM;
		goto err_ue_ctrl;
	}

	ep92_uevent_ctrl->ktype.release = ep92_release_uevent_data;
	ret = ep92_init_uevent_data(ep92_uevent_ctrl, "ctrl-uevent");
	if (ret) {
		dev_err(&client->dev,
			"%s: Failed to init ctrl-uevent: %d\n",
			__func__, ret);
		goto err_ue_init_ctrl;
	}

	/* uevent to signal audio format changes */
	ep92_uevent_audio = devm_kzalloc(&client->dev,
		sizeof(*ep92_uevent_audio), GFP_KERNEL);
	if (!ep92_uevent_audio) {
		ret = -ENOMEM;
		goto err_ue_audio;
	}

	ep92_uevent_audio->ktype.release = ep92_release_uevent_data;
	ret = ep92_init_uevent_data(ep92_uevent_audio, "audio-uevent");
	if (ret) {
		dev_err(&client->dev,
			"%s: Failed to init ctrl-uevent: %d\n",
			__func__, ret);
		goto err_ue_init_audio;
	}
	return 0;

err_ue_init_audio:
	devm_kfree(&client->dev, ep92_uevent_audio);
err_ue_audio:
	ep92_destroy_uevent_data(ep92_uevent_ctrl);
err_ue_init_ctrl:
	devm_kfree(&client->dev, ep92_uevent_ctrl);
err_ue_ctrl:
	kset_unregister(ep92_uevent_kset);
err_kset:
	snd_soc_unregister_codec(&client->dev);
err_reg:
	if (ep92->irq == 0)
		del_timer(&ep92->timer);

	return ret;
}

static int ep92_i2c_remove(struct i2c_client *client)
{
	struct ep92_pdata *ep92;

	ep92 = i2c_get_clientdata(client);
	if ((ep92 != NULL) && (ep92->irq == 0))
		del_timer(&ep92->timer);

	snd_soc_unregister_codec(&client->dev);

	ep92_destroy_uevent_data(ep92_uevent_ctrl);
	devm_kfree(&client->dev, ep92_uevent_ctrl);

	ep92_destroy_uevent_data(ep92_uevent_audio);
	devm_kfree(&client->dev, ep92_uevent_audio);

	if (ep92_uevent_kset) {
		kset_unregister(ep92_uevent_kset);
		ep92_uevent_kset = NULL;
	}

	return 0;
}

static const struct i2c_device_id ep92_i2c_id[] = {
	{ "ep92-dev", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, ep92_i2c_id);

static struct i2c_driver ep92_i2c_driver = {
	.probe =    ep92_i2c_probe,
	.remove =   ep92_i2c_remove,
	.id_table = ep92_i2c_id,
	.driver = {
		.name = "ep92",
		.owner = THIS_MODULE,
		.of_match_table = ep92_of_match
	},
};

static int __init ep92_codec_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&ep92_i2c_driver);
	if (ret)
		pr_err("Failed to register EP92 I2C driver: %d\n", ret);

	return ret;
}
module_init(ep92_codec_init);

static void __exit ep92_codec_exit(void)
{
	i2c_del_driver(&ep92_i2c_driver);
}
module_exit(ep92_codec_exit);

MODULE_DESCRIPTION("EP92 HDMI repeater/switch driver");
MODULE_LICENSE("GPL v2");
