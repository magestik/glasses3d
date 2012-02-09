#include <linux/usb.h>
#include <linux/firmware.h>

struct hexline {
	uint16_t len;
	uint16_t addr;
	u8 type;
	u8 data[1024];
	u8 chk;
};

struct nv3d_usbir {
	struct usb_device *	udev;
	struct urb 		  * irq;
	unsigned int		active;
	unsigned int		eye;
	unsigned int 		rate;
	unsigned char 	  * data;
};

void nv3d_key_irq(struct urb *urb, struct pt_regs *regs);
int nv3d_init(struct usb_interface *interface, const struct usb_device_id *id);
void nv3d_del(struct usb_interface *interface);

int nv3d_set_rate(struct usb_device *udev, int rate);
void nv3d_swap(void);

int nv3d_download_firmware(struct usb_device *udev);
int nv3d_usb_hexline(const struct firmware *fw, struct hexline *hx, int *pos);
