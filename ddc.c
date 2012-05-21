#include "ddc.h"

#include "glasses3d.h"
#include "sync.h"

// DDC is specific I2C

// TODO: implement http://code.google.com/p/a3dcontrol/source/browse/src/a3dcontrol.c

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
	.class		= I2C_CLASS_DDC
}

/* On Module Load */
int ddc_init() {
	
	return 0;
}

/* On Module Unload */
void ddc_stop() {
	
}

/* DDC plug */
int i2c_probe() {
	
}

/* DDC unplug */
void i2c_disconnect() {
	
}
