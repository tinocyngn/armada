/*
 * MDIO I2C bridge
 *
 * Copyright (C) 2015 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/i2c.h>
#include <linux/phy.h>

#include "mdio-i2c.h"

static int i2c_mii_read(struct mii_bus *bus, int phy_id, int reg)
{
	struct i2c_adapter *i2c = bus->priv;
	struct i2c_msg msgs[2];
	u8 data[2], dev_addr = reg;
	int bus_addr, ret;

	bus_addr = 0x40 + phy_id;
	if (bus_addr == 0x50 || bus_addr == 0x51)
		return 0xffff;

	msgs[0].addr = bus_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &dev_addr;
	msgs[1].addr = bus_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = sizeof(data);
	msgs[1].buf = data;

	ret = i2c_transfer(i2c, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs))
		return 0xffff;

	return data[0] << 8 | data[1];
}

static int i2c_mii_write(struct mii_bus *bus, int phy_id, int reg, u16 val)
{
	struct i2c_adapter *i2c = bus->priv;
	struct i2c_msg msg;
	int bus_addr, ret;
	u8 data[3];

	bus_addr = 0x40 + phy_id;
	if (bus_addr == 0x50 || bus_addr == 0x51)
		return 0;

	data[0] = reg;
	data[1] = val >> 8;
	data[2] = val;

	msg.addr = bus_addr;
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;

	ret = i2c_transfer(i2c, &msg, 1);

	return ret < 0 ? ret : 0;
}

struct mii_bus *mdio_i2c_alloc(struct device *parent, struct i2c_adapter *i2c)
{
	struct mii_bus *mii;

	if (!i2c_check_functionality(i2c, I2C_FUNC_I2C))
		return ERR_PTR(-EINVAL);

	mii = mdiobus_alloc();
	if (!mii)
		return ERR_PTR(-ENOMEM);

	snprintf(mii->id, MII_BUS_ID_SIZE, "i2c:%s", dev_name(parent));
	mii->parent = parent;
	mii->read = i2c_mii_read;
	mii->write = i2c_mii_write;
	mii->priv = i2c;

	return mii;
}
EXPORT_SYMBOL_GPL(mdio_i2c_alloc);

MODULE_AUTHOR("Russell King");
MODULE_DESCRIPTION("MDIO I2C bridge library");
MODULE_LICENSE("GPL v2");
