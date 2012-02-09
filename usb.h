#include <linux/usb.h>

/* NVIDIA 3D VISION */
#define NV3D_VENDOR_ID	0x0955
#define NV3D_PRODUCT_ID	0x0007

extern struct usb_driver usb_driver;

int usb_init(void);
void usb_stop(void);

int usb_probe(struct usb_interface *interface, const struct usb_device_id *id);
void usb_disconnect(struct usb_interface *interface);
