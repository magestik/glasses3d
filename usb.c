#include "glasses3d.h"
#include "sync.h"

#include "usb.h"

/* KERNEL MODULE */
const struct usb_device_id id_table [] = {
	{ USB_DEVICE(NV3D_VENDOR_ID, NV3D_PRODUCT_ID) }, // NVIDIA 3D VISION
	{ },
};

MODULE_DEVICE_TABLE (usb, id_table);

/* USB */
struct usb_driver usb_driver = {
	.name		= DRIVER_NAME,
	.probe		= usb_probe,
	.disconnect = usb_disconnect,
	.id_table 	= id_table,
};

/* On Module Load */
int usb_init() {
	int retval = 0;
	retval = usb_register(&usb_driver);
	
	if (retval) {
		printk(KERN_ALERT "Error: usb_register failed (errno=%d)", retval);
	}

	return retval;
}

/* On Module Unload */
void usb_stop() {
	usb_deregister(&usb_driver);
}

/* USB plug */
int usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(interface);
	int r = 0;
	
	if ((dev->descriptor.idVendor == NV3D_VENDOR_ID) && (dev->descriptor.idProduct == NV3D_PRODUCT_ID)) {
		//r = nv3d_init(interface, id);
		return r;
	} else {
		return -ENODEV;
	}
}

/* USB unplug */
void usb_disconnect(struct usb_interface *interface)
{
	//nv3d_del(interface);
}

