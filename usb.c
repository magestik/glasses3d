#include "usb.h"

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>

#include "glasses3d.h"
#include "sync.h"

#include "nv3d.h"

const struct usb_device_id id_table [] = {
	{ USB_DEVICE(NV3D_VENDOR_ID, NV3D_PRODUCT_ID) }, // NVIDIA 3D VISION (v1)
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

usb_list attached_usb_devices;

//TODO use <linux/list.h>

/* Add an element to the list */
void usb_add_element(struct usb_interface *interface) {
	struct usb_list_element *newElement = kmalloc(sizeof(struct usb_list_element), GFP_KERNEL);
	newElement->interface = interface;
	newElement->next = attached_usb_devices;
	attached_usb_devices = newElement;
	//printk(KERN_INFO "Add to list");
}

/* Remove an element from the list */
void usb_remove_element(struct usb_glasses *del) {
	struct usb_glasses *dev;
	struct usb_list_element *next;
	struct usb_list_element *currentElement = attached_usb_devices;
	
	dev = usb_get_intfdata (currentElement->interface);
	if( dev == del ) {
		attached_usb_devices = currentElement->next;
		//printk(KERN_INFO "Remove from list");
	}
	
	while(currentElement->next != NULL) {
		dev = usb_get_intfdata (currentElement->next->interface);
		if( dev == del ) { // pointeurs comparison
			next = currentElement->next->next;
			currentElement->next = currentElement->next->next;
			kfree(next);
			//printk(KERN_INFO "Remove from list");
		}
		
		currentElement = currentElement->next;
	}
}

/* Swap all elements in list */
void usb_swap(void) {
	struct usb_glasses *dev;
	
	struct usb_list_element *currentElement = attached_usb_devices;
	
	while(currentElement != NULL) {
		dev = usb_get_intfdata (currentElement->interface);
		if( dev->active ) {
			dev->swap_eyes(dev->udev, current_eye^dev->inversed);
		}
		
		currentElement = currentElement->next;
	}
}

/* On Module Load */
int usb_init() {
	int err = 0;
	attached_usb_devices = NULL;
	err = usb_register(&usb_driver);
	if (err) {
		printk(KERN_NOTICE "%s: usb_register failed (returned %d)\n", DRIVER_NAME, err);
	}

	return err;
}

/* On Module Unload */
void usb_stop() {
	usb_deregister(&usb_driver);
}

/* USB plug */
int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
	struct usb_device *udev = interface_to_usbdev(interface);
	struct usb_glasses *dev = NULL;
	
	dev = kmalloc (sizeof (struct usb_glasses), GFP_KERNEL);
	if(dev == NULL){
		dev_err(&interface->dev, "Out of memory\n");
		return -ENOMEM;
	}
	
	memset (dev, 0x00, sizeof (*dev));
	
	dev->udev = usb_get_dev(udev);
	usb_set_intfdata (interface, dev);

	if (udev->descriptor.idVendor == NV3D_VENDOR_ID && udev->descriptor.idProduct == NV3D_PRODUCT_ID) {
		nv3d_init(interface);
	} else {
		return -ENODEV;
	}

	usb_add_element(interface); // liste chainée
	
	dev_info(&interface->dev, "USB glasses now attached\n");
	return 0;
}

/* USB unplug */
void usb_disconnect(struct usb_interface *interface) {
	struct usb_glasses *dev;
	
	dev = usb_get_intfdata (interface);
	usb_remove_element(dev); // liste chainée
	usb_set_intfdata (interface, NULL);

	usb_put_dev(dev->udev);
	
	kfree(dev);

	dev_info(&interface->dev, "USB glasses now disconnected\n");
}
