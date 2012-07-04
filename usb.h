#include <linux/usb.h>

/* NVIDIA 3D VISION */
#define NVIDIA_VENDOR_ID	0x0955
#define NV3D_PRODUCT_ID_1	0x0007 // version 1
#define NV3D_PRODUCT_ID_2	0x7002 // laptop internal

extern struct usb_driver usb_driver;

struct usb_glasses {
	struct usb_device *	udev;
	unsigned int		active;
	unsigned int		inversed;
	void (*swap_eyes)(struct usb_device *udev, int eye);
};

struct usb_list_element {
    struct usb_interface *interface;
    struct usb_list_element *next;
};

typedef struct usb_list_element *usb_list;

int usb_init(void);
void usb_stop(void);
void usb_swap(void);
int usb_probe(struct usb_interface *interface, const struct usb_device_id *id);
void usb_disconnect(struct usb_interface *interface);
