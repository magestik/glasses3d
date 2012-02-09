#include "glasses3d.h"
#include "sync.h"

#include "ddc.h"

// DDC is specific I2C

/* KERNEL MODULE */
static struct i2c_device_id id_table [] = {
	{ "foo", my_id_for_foo },
	{ "bar", my_id_for_bar },
	{ }
};

MODULE_DEVICE_TABLE(i2c, foo_idtable);

/* I2C */
static struct i2c_driver i2c_driver = {
	.driver = {
		.name	= DRIVER_NAME,
	},
	
	.probe		= i2c_probe,
	.remove		= i2c_remove,		
	.id_table	= id_table,

	/* if device autodetection is needed: */
	.class		= I2C_CLASS_SOMETHING,
	.detect		= foo_detect,
	.address_list	= normal_i2c
}

int ddc_init() {
}

void ddc_stop() {
}


int i2c_probe()
{
}

void i2c_remove()
{
}
