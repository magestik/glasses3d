#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <linux/usb.h>
#include <linux/interrupt.h>

#include "usb_module.h"
#include "nv3dvision.h"

#define DRIVER_AUTHOR "Magestik"
#define DRIVER_DESC "3D Glasses driver"
#define DRIVER_NAME "glasses3d"

/* GRAPHIC CARD IRQ */
#define NVCARD_IRQ 19 /* cat /proc/driver/nvidia/cards/0 */

/* NVIDIA 3D VISION */
#define NV3D_VENDOR_ID	0x0955
#define NV3D_PRODUCT_ID	0x0007

/* Hardware supported */
struct usb_device_id id_table [] = {
	{ USB_DEVICE(NV3D_VENDOR_ID, NV3D_PRODUCT_ID) }, // NVIDIA 3D VISION
	{ },
};

/* USB driver */
struct usb_driver dev3d_driver = {
	.name		= DRIVER_NAME,
	.probe		= dev3d_probe,
	.disconnect = dev3d_disconnect,
	.id_table 	= id_table,
};

MODULE_DEVICE_TABLE (usb, id_table);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

irqreturn_t sync_interrupt(int irq, void *dev_id){

	// TODO :: check if interrupt signal a vblank
	printk(KERN_INFO "IRQ\n");	
	nv3d_swap();
	
	return IRQ_HANDLED;
}

/* USB plug */
int dev3d_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(interface);
	int r = 0;
	int i;
	
	if ((dev->descriptor.idVendor == NV3D_VENDOR_ID) && (dev->descriptor.idProduct == NV3D_PRODUCT_ID)) {
		r = nv3d_init(interface, id);
		
		return r;
	} else {
		return -ENODEV;
	}
}

/* USB unplug */
void dev3d_disconnect(struct usb_interface *interface)
{	
	
	disable_irq(NVCARD_IRQ);
	nv3d_del(interface);
	
}

/* Module loading */
int __init glasses3d_init(void)
{
	int retval = 0;

	/* Register as USB driver */
	retval = 0;
	retval = usb_register(&dev3d_driver);
	if (retval) {
		printk(KERN_ALERT "Error: usb_register failed (errno=%d)", retval);
		return retval;
	}
		
	/* Request the video card IRQ */
	retval = 0;
	retval = request_irq(NVCARD_IRQ, &sync_interrupt, IRQF_SHARED, DRIVER_NAME, &sync_interrupt); // On appelle la fonction nv_interrupt à chaque interruption materielle
	if (retval) {
		printk(KERN_ALERT "Error: unable to get Graphic Card IRQ %d (errno=%d).\n", NVCARD_IRQ, retval);
		return retval;
	};
	enable_irq(NVCARD_IRQ);
	/* Everything is OK */
	printk(KERN_INFO "%s succefully loaded\n", DRIVER_NAME);	
	return retval;
}

/* Module exit */
void __exit glasses3d_exit(void)
{
	usb_deregister(&dev3d_driver);
	free_irq(NVCARD_IRQ, &sync_interrupt);
	printk(KERN_INFO "%s unloaded\n", DRIVER_NAME);	
}

module_init(glasses3d_init);
module_exit(glasses3d_exit);
