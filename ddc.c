#include "ddc.h"

#include "glasses3d.h"
#include "sync.h"

// DDC is specific I2C

// TODO: implement http://code.google.com/p/a3dcontrol/source/browse/src/a3dcontrol.c

/* KERNEL MODULE */
static struct i2c_device_id id_table [] = {
	{ "foo", my_id_for_foo },
	{ }
};

MODULE_DEVICE_TABLE(i2c, id_table);

/* I2C */
static struct i2c_driver i2c_driver = {
	.driver = {
		.name	= DRIVER_NAME,
	},
	
	.probe		= ddc_probe,
	.remove		= ddc_remove,		
	.id_table	= id_table,
	.class		= I2C_CLASS_DDC
}

/* On Module Load */
int ddc_init() {
	return i2c_add_driver(&i2c_driver);
}

/* On Module Unload */
void ddc_stop() {
	i2c_del_driver(&i2c_driver);
}

/* DDC plug */
int ddc_probe(struct i2c_client *client, const struct i2c_device_id *idp){
	
}

/* DDC unplug */
void ddc_remove() {
	
}
