/*
 * KTZ Semiconductor KTZ8866 LED Driver
 *
 * Copyright (C) 2013 Ideas on board SPRL
 *
 * Contact: Zhang Teng <zhangteng3@xiaomi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include "mi_backlight_ktz8866.h"
#define u8	unsigned char

enum {
    KTZ8866_A = 0,
    KTZ8866_B,
};

struct i2c_client *g_client;
struct i2c_client *g_clientb;

static struct ktz8866_led g_ktz8866_led;

int ktz8866_read(struct i2c_client *client,u8 reg, u8 *data)
{
	int ret;
	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "ktz8866 i2c failed reading at 0x%02x\n", reg);
		return ret;
	}
	*data = (uint8_t)ret;
	return 0;
}
int ktz8866_write(struct i2c_client *client,u8 reg, u8 data)
{
	return i2c_smbus_write_byte_data(client, reg, data);
}

int dualktz8866_write(u8 reg, u8 data)
{
	int ret;
	ret = ktz8866_write(g_client ,reg, data);
	ret = ktz8866_write(g_clientb,reg, data);
	return ret;
}

int ktz8866_backlight_update_status(unsigned int level)
{
	int exponential_bl = level;
	int brightness = 0;
	u8 v[2];

	if(g_ktz8866_led.HBM_enable)
	{
		int normal_brightness = 1737;
		if(exponential_bl <= BL_LEVEL_MAX) {
			exponential_bl = (exponential_bl * normal_brightness) / 2047;
		}
		else if(exponential_bl <=BL_LEVEL_MAX_HBM) {
			exponential_bl = ((exponential_bl - 2048) * (2047 - normal_brightness)) / 2047 + normal_brightness;
		}
		else {
			dev_warn(&g_client->dev, "ktz8866 backlight out of 4095 too large!!!\n");
			return 0;
		}
	}

	brightness = mi_bl_level_remap[exponential_bl];
	if (brightness < 0 || brightness > BL_LEVEL_MAX || brightness == g_ktz8866_led.level)
		return 0;
	mutex_lock(&g_ktz8866_led.lock);
	dev_warn(&g_client->dev, "ktz8866 backlight 0x%02x ,exponential brightness %d \n", brightness, exponential_bl);
	if (!g_ktz8866_led.ktz8866_status && brightness > 0) {
		dualktz8866_write(KTZ8866_DISP_BL_ENABLE, 0x7f);
		g_ktz8866_led.ktz8866_status = 1;
		dev_warn(&g_client->dev, "ktz8866 backlight enable,dimming close");
	} else if (brightness == 0) {
		dualktz8866_write(KTZ8866_DISP_BL_ENABLE, 0x3f);
		g_ktz8866_led.ktz8866_status = 0;
		//usleep_range((10 * 1000),(10 * 1000) + 10);
		dev_warn(&g_client->dev, "ktz8866 backlight disable,dimming close");
	}

	v[0] = (brightness >> 3) & 0xff;
	v[1] = (brightness - (brightness >> 3) * 8) & 0x7;
	dualktz8866_write(KTZ8866_DISP_BB_LSB, v[1]);
	dualktz8866_write(KTZ8866_DISP_BB_MSB, v[0]);

	g_ktz8866_led.level = brightness;
	mutex_unlock(&g_ktz8866_led.lock);

	return 0;
}
static int ktz8866_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	u8 read;
	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_warn(&client->dev, "ktz8866 I2C adapter doesn't support I2C_FUNC_SMBUS_BYTE\n");
		return -EIO;
	}

	if(id!=NULL) {
		if(id->driver_data == KTZ8866_A)
		{
			g_client = client;
			ktz8866_read(client,KTZ8866_DISP_FLAGS, &read);
			dev_err(&client->dev, "ktz8866 A reading 0x%02x is 0x%02x\n", KTZ8866_DISP_FLAGS, read);

			g_ktz8866_led.HBM_enable = false;
			g_ktz8866_led.HBM_enable = of_property_read_bool((&g_client->dev)->of_node,"ktz8866,backlight-HBM-enable");
			if(g_ktz8866_led.HBM_enable)
				dev_err(&client->dev, "ktz8866 HBM is enabled ! \n");
			else
				dev_err(&client->dev, "ktz8866 HBM is disenabled! \n");
		}else{
			g_clientb = client;
			ktz8866_read(client,KTZ8866_DISP_FLAGS, &read);
			dev_err(&client->dev, "ktz8866 B reading 0x%02x is 0x%02x\n", KTZ8866_DISP_FLAGS, read);
		}
	} else{
		dev_warn(&client->dev, "ktz8866 device_id is NULL !!!! \n");
	}

	mutex_init(&g_ktz8866_led.lock);
	dev_warn(&client->dev, "ktz8866 init success\n");
	return 0;
}
static int ktz8866_remove(struct i2c_client *client)
{
	int backlight = 0;
	ktz8866_backlight_update_status(backlight);
	return 0;
}

static const struct i2c_device_id ktz8866_ids[] = {
    { "ktz8866", 0 },
    { "ktz8866b", 1 },
    {}
};
MODULE_DEVICE_TABLE(i2c, ktz8866_ids);

static struct of_device_id ktz8866_match_table[] = {
	{ .compatible = "ktz,ktz8866",},
	{ .compatible = "ktz,ktz8866b",},
	{ },
};
static struct i2c_driver ktz8866_driver = {
	.driver = {
		.name = "ktz8866",
		.of_match_table = ktz8866_match_table,
	},
	.probe = ktz8866_probe,
	.remove = ktz8866_remove,
	.id_table = ktz8866_ids,
};
int mi_backlight_ktz8866_init(void)
{
	return i2c_add_driver(&ktz8866_driver);
}
