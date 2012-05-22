#include "glasses3d.h"

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>

#include "sync.h"
#include "usb.h"

/* GLOBALS */
int current_eye = 0;
int refresh_rate = 120;

void glasses3d_swap() {
	usb_swap();
	
	// TODO quickly
	//ddc_swap();
	
	// TODO ???
	//bluetooth_swap();
}

/* On Load */
int __init glasses3d_init(void) {
	
	sync_init();
	
	usb_init();
	//ddc_init();
	
	printk(KERN_INFO "%s succefully loaded\n", DRIVER_NAME);
	return 0;
}

/* On Unload */
void __exit glasses3d_exit(void) {
	
	sync_stop();
	
	usb_stop();
	//ddc_stop();
	
	printk(KERN_INFO "%s succefully unloaded\n", DRIVER_NAME);	
}

module_init(glasses3d_init);
module_exit(glasses3d_exit);


MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
