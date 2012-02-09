#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>

#include "glasses3d.h"
#include "sync.h"

#include "usb.h"

/* GLOBALS */
int current_eye = 0;
int inversed = 0;

dev3d_t devices_list[MAX_DEVICES];
int devices_count = 0;

/* On Load */
int __init glasses3d_init(void)
{
	
	sync_init();
	
	// init_usb_control()

	printk(KERN_INFO "%s succefully loaded\n", DRIVER_NAME);
	return 0;
}

/* On Unload */
void __exit glasses3d_exit(void)
{
	
	sync_stop();
	
	// stop_usb_control()
	
	printk(KERN_INFO "%s succefully unloaded\n", DRIVER_NAME);	
}

module_init(glasses3d_init);
module_exit(glasses3d_exit);


MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
