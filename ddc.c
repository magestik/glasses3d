#include "ddc.h"
#include <linux/i2c-dev.h>
#include "glasses3d.h"
#include "sync.h"

static struct class *i2c_dev_class;

static int i2cdev_attach_adapter(struct device *dev, void *dummy) {
	struct i2c_adapter *adap;
	//struct i2c_dev *i2c_dev;
	//int res;
	
	if (dev->type != &i2c_adapter_type)
		return 0;
	
	adap = to_i2c_adapter(dev);	
	
	printk(KERN_INFO "ATTACH I2C %s", adap->name);
	return 0;
}

static int i2cdev_detach_adapter(struct device *dev, void *dummy) {
	struct i2c_adapter *adap;
	//struct i2c_dev *i2c_dev;
	
	if (dev->type != &i2c_adapter_type)
		return 0;
	
	adap = to_i2c_adapter(dev);
	
	printk(KERN_INFO "DETACH I2C");
	return 0;
}

static int i2cdev_notifier_call(struct notifier_block *nb, unsigned long action, void *data) {
	struct device *dev = data;
	
	switch (action) {
		case BUS_NOTIFY_ADD_DEVICE:
			return i2cdev_attach_adapter(dev, NULL);
			
		case BUS_NOTIFY_DEL_DEVICE:
			return i2cdev_detach_adapter(dev, NULL);
	}
	
	return 0;
}

static struct notifier_block i2cdev_notifier = {
	.notifier_call = i2cdev_notifier_call,
};

/* ------------------------------------------------------------------------- */

/*
 * module load/unload record keeping
 */

void i2c_dev_init(void) {
	int res;
	/*
	printk(KERN_INFO "i2c /dev entries driver\n");
	res = register_chrdev(I2C_MAJOR, "i2c", &i2cdev_fops);
	if (res)
		goto out;
	*/
	i2c_dev_class = class_create(THIS_MODULE, "i2c-dev");
	if (IS_ERR(i2c_dev_class)) {
		res = PTR_ERR(i2c_dev_class);
		return;
		//goto out_unreg_chrdev;
	}
	
	/* Keep track of adapters which will be added or removed later */
	res = bus_register_notifier(&i2c_bus_type, &i2cdev_notifier);
	if (res)
		return;
		
	/* Bind to already existing adapters right away */
	i2c_for_each_dev(NULL, i2cdev_attach_adapter);
}

void i2c_dev_exit(void) {
	bus_unregister_notifier(&i2c_bus_type, &i2cdev_notifier);
	i2c_for_each_dev(NULL, i2cdev_detach_adapter);
	class_destroy(i2c_dev_class);
	//unregister_chrdev(I2C_MAJOR, "i2c");
}
