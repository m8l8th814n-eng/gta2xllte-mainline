// SPDX-License-Identifier: GPL-2.0-only
/*
 * Intersil/Renesas ISL98608 DSI panel bias power IC.
 *
 * Supplies the positive (VSP) and negative (VSN) rails (~5.8 V) that a
 * MIPI-DSI panel needs for its source drivers. The chip is brought up by
 * raising an enable GPIO and writing the VBST/VN/VP output levels over I2C.
 *
 * Modelled as a single enable-only regulator: the panel's vsp-supply and
 * vsn-supply both point at this device, so enabling either rail runs the
 * full power sequence (the regulator core ref-counts the two consumers).
 *
 * Power sequence taken from the downstream Samsung isl98608-panel_power.c
 * (drivers/video/msm/mdss/samsung/HX8279_TV101WUM/) as shipped on the
 * Galaxy Tab A 10.5 (gta2xllte).
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>

#define ISL98608_REG_VBST	0x06
#define ISL98608_REG_VN		0x08
#define ISL98608_REG_VP		0x09

/* 0x08 selects 5.8 V on VBST/VN/VP (downstream values). */
#define ISL98608_LEVEL_5_8V	0x08
#define ISL98608_FIXED_UV	5800000

struct isl98608 {
	struct i2c_client *client;
	struct gpio_desc *enable_gpio;
};

static int isl98608_enable(struct regulator_dev *rdev)
{
	struct isl98608 *priv = rdev_get_drvdata(rdev);
	struct i2c_client *client = priv->client;
	int ret;

	/* Let the input rail settle before talking to the chip. */
	usleep_range(5000, 6000);

	ret = i2c_smbus_write_byte_data(client, ISL98608_REG_VBST,
					ISL98608_LEVEL_5_8V);
	if (ret)
		return ret;
	ret = i2c_smbus_write_byte_data(client, ISL98608_REG_VN,
					ISL98608_LEVEL_5_8V);
	if (ret)
		return ret;
	ret = i2c_smbus_write_byte_data(client, ISL98608_REG_VP,
					ISL98608_LEVEL_5_8V);
	if (ret)
		return ret;

	gpiod_set_value_cansleep(priv->enable_gpio, 1);
	usleep_range(3000, 4000);

	return 0;
}

static int isl98608_disable(struct regulator_dev *rdev)
{
	struct isl98608 *priv = rdev_get_drvdata(rdev);

	usleep_range(3000, 4000);
	gpiod_set_value_cansleep(priv->enable_gpio, 0);
	usleep_range(5000, 6000);

	return 0;
}

static int isl98608_is_enabled(struct regulator_dev *rdev)
{
	struct isl98608 *priv = rdev_get_drvdata(rdev);

	return gpiod_get_value_cansleep(priv->enable_gpio);
}

static const struct regulator_ops isl98608_ops = {
	.enable		= isl98608_enable,
	.disable	= isl98608_disable,
	.is_enabled	= isl98608_is_enabled,
};

static const struct regulator_desc isl98608_desc = {
	.name		= "isl98608",
	.type		= REGULATOR_VOLTAGE,
	.owner		= THIS_MODULE,
	.ops		= &isl98608_ops,
	.n_voltages	= 1,
	.fixed_uV	= ISL98608_FIXED_UV,
	.enable_time	= 8000,
};

static int isl98608_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct regulator_config config = { };
	struct regulator_dev *rdev;
	struct isl98608 *priv;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->client = client;

	/* Start disabled; the regulator core enables on first consumer use. */
	priv->enable_gpio = devm_gpiod_get(dev, "enable", GPIOD_OUT_LOW);
	if (IS_ERR(priv->enable_gpio))
		return dev_err_probe(dev, PTR_ERR(priv->enable_gpio),
				     "failed to get enable GPIO\n");

	config.dev = dev;
	config.of_node = dev->of_node;
	config.init_data = of_get_regulator_init_data(dev, dev->of_node,
						      &isl98608_desc);
	config.driver_data = priv;

	rdev = devm_regulator_register(dev, &isl98608_desc, &config);
	if (IS_ERR(rdev))
		return dev_err_probe(dev, PTR_ERR(rdev),
				     "failed to register regulator\n");

	return 0;
}

static const struct of_device_id isl98608_of_match[] = {
	{ .compatible = "isil,isl98608" },
	{ }
};
MODULE_DEVICE_TABLE(of, isl98608_of_match);

static const struct i2c_device_id isl98608_i2c_id[] = {
	{ "isl98608" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, isl98608_i2c_id);

static struct i2c_driver isl98608_driver = {
	.driver = {
		.name = "isl98608",
		.of_match_table = isl98608_of_match,
	},
	.probe = isl98608_probe,
	.id_table = isl98608_i2c_id,
};
module_i2c_driver(isl98608_driver);

MODULE_DESCRIPTION("Intersil ISL98608 DSI panel bias regulator");
MODULE_LICENSE("GPL");
