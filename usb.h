#include <linux/usb.h>

/* NVIDIA 3D VISION */
#define NV3D_VENDOR_ID	0x0955
#define NV3D_PRODUCT_ID	0x0007

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
