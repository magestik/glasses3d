#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <linux/usb.h>
#include <linux/interrupt.h>

#include "nv3dvision.h"

/* cpu clock */
#define NVSTUSB_CLOCK 48000000LL

/* T0 runs at 4 MHz */
#define NVSTUSB_T0_CLOCK (NVSTUSB_CLOCK/12LL)
#define NVSTUSB_T0_COUNT(us) (-(us)*(NVSTUSB_T0_CLOCK/1000000)+1)
#define NVSTUSB_T0_US(count) (-(count-1)/(NVSTUSB_T0_CLOCK/1000000))

/* T2 runs at 12 MHz */
#define NVSTUSB_T2_CLOCK (NVSTUSB_CLOCK/ 4LL)
#define NVSTUSB_T2_COUNT(us) (-(us)*(NVSTUSB_T2_CLOCK/1000000)+1)
#define NVSTUSB_T2_US(count) (-(count-1)/(NVSTUSB_T2_CLOCK/1000000))

#define NVSTUSB_CMD_WRITE (0x01) /* write data */
#define NVSTUSB_CMD_READ (0x02) /* read data */
#define NVSTUSB_CMD_CLEAR (0x40) /* set data to 0 */

#define NVSTUSB_CMD_SET_EYE (0xAA) /* set current eye */
#define NVSTUSB_CMD_CALL_X0199 (0xBE) /* call routine at 0x0199 */

struct nv3d_usbir *nv3d;

int nv3d_init(struct usb_interface *interface, const struct usb_device_id *id)
{
	
	struct usb_device *udev = interface_to_usbdev(interface);
	
	printk(KERN_INFO "Nvidia 3D Vision : starting init\n");
	
	if (!(nv3d = kmalloc (sizeof (struct nv3d_usbir), GFP_KERNEL))) {
		err("%s - Out of memory.", __FUNCTION__);
		return -ENOMEM;
	}
	
	memset(nv3d, 0, sizeof(*nv3d));
	
	nv3d->udev = udev;
	
	if(interface->cur_altsetting->desc.bNumEndpoints == 0) {
		printk(KERN_NOTICE "0 Endpoint: must upload nvidia emitter firmware\n");
		// Upload the firmware to the USB device
	} else {
		printk(KERN_NOTICE "%d Endpoints\n", interface->cur_altsetting->desc.bNumEndpoints);
	}
	
	//nv3d->rate = nv3d_set_rate(udev, 120);
	nv3d->eye = 1;

	printk(KERN_INFO "Nvidia 3D Vision : init complete\n");
	return 0;
}

void nv3d_del(struct usb_interface *interface)
{
	usb_set_intfdata(interface, NULL);
	
	if(nv3d) {
		kfree(nv3d);
	}
	
	printk(KERN_INFO "Nvidia 3D Vision : emitter now disconnected\n");
}

int nv3d_set_rate(struct usb_device *udev, int rate)
{
	//struct nv3d_usbir *nv3d;
	int actual_length;
	int n;
	
	int32_t w = NVSTUSB_T2_COUNT(4568.50); /* 4.56800 ms */
	int32_t x = NVSTUSB_T0_COUNT(4774.25); /* 4.77425 ms */
	int32_t y = NVSTUSB_T0_COUNT(2080); /* 2.08000 ms time each eye is on*/
	int32_t z = NVSTUSB_T2_COUNT(1000000/rate); /* 8.33333 ms if 120 Hz */
	
	// uint_8 = unsigned char (8 bits)
	uint8_t cmdTimings[] = {
		NVSTUSB_CMD_WRITE, /* write data */
		0x00, /* to address 0x2007 (0x2007+0x00) = ?? */
		0x18, 0x00, /* 24 bytes follow */
		
		/* original: e1 29 ff ff (-54815; -55835) */
		w, w>>8, w>>16, w>>24,
		/* 2007: ?? some timer 2 counter, 1020 is subtracted from this
		* loaded at startup with:
		* 0x44 0xEC 0xFE 0xFF (-70588(-1020)) */
		
		/* original: 68 b5 ff ff (-19096), 4.774 ms */
		x, x>>8, x>>16, x>>24,
		/* 200b: ?? counter saved at long at address 0x4f
		 * increased at timer 0 interrupt if bit 20h.1
		 * is cleared, on overflow
		 * to 0 the code at 0x03c8 is executed.
		 * timer 0 will be started with this value
		 * by timer2 */
		
		/* original: 81 df ff ff (-8319), 2.08 ms */
		y, y>>8, y>>16, y>>24, 
		/* 200f: ?? counter saved at long at address 0x4f, 784 is added to this
		 * if PD1 is set, delay until turning eye off? */
		
		/* wave forms to send via IR: */
		0x30, /* 2013: 110000 PD1=0, PD2=0: left eye off */
		0x28, /* 2014: 101000 PD1=1, PD2=0: left eye on */
		0x24, /* 2015: 100100 PD1=0, PD2=1: right eye off */
		0x22, /* 2016: 100010 PD1=1, PD2=1: right eye on */
		
		/* ?? used when frameState is != 2, for toggling bits in Port B,
		 * values seem to have no influence on the glasses or infrared signals */
		0x0a, /* 2017: 1010 */
		0x08, /* 2018: 1000 */
		0x05, /* 2019: 0101 */
		0x04, /* 201a: 0100 */
		
		z, z>>8, z>>16, z>>24 /* 201b: timer 2 reload value */
	};
	
	uint8_t cmd0x1c[] = {
		NVSTUSB_CMD_WRITE, /* write data */
		0x1c, /* to address 0x2023 (0x2007+0x1c) = ?? */
		0x02, 0x00, /* 2 bytes follow */
		0x02, 0x00
		/* ?? seems to be the start value of some
		* counter. runs up to 6, some things happen
		* when it is lower, that will stop if when
		* it reaches 6. could be the index to 6 byte values
		* at 0x17ce that are loaded into TH0*/
	};
	
	uint16_t timeout = rate << 2; 
	
	uint8_t cmdTimeout[] = {
		NVSTUSB_CMD_WRITE, /* write data */
		0x1e, /* to address 0x2025 (0x2007+0x1e) = timeout */
		0x02, 0x00, /* 2 bytes follow */
		timeout, timeout>>8 /* idle timeout (number of frames) */
	};
	
	uint8_t cmd0x1b[] = {
		NVSTUSB_CMD_WRITE, /* write data */
		0x1b, /* to address 0x2022 (0x2007+0x1b) = ?? */
		0x01, 0x00, /* 1 byte follows */
		0x07
		/* ?? compared with byte at 0x29 in TD_Poll()
		 * bit 0-1: index to a table of 4 bytes at 0x17d4 (0x00,0x08,0x04,0x0C),
		 * PB1 is set in TD_Poll() if this index is 0, cleared otherwise
		 * bit 2: set bool21_4, start timer 1, enable ext. int. 5
		 * bit 3: PC1 is set to the inverted value of this bit in TD_Poll()
		 * bit 4-5: index to a table of 4 bytes at 0x2a
		 * bit 6: restart t0 on some conditions in TD_Poll()
		 */
	};
	
	printk(KERN_INFO "Nvidia 3D Vision : starting set rate\n");		
	
	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmdTimings, sizeof(cmdTimings), &actual_length, 0);	

	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmd0x1c, sizeof(cmd0x1c), &actual_length, 0);	

	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmdTimeout, sizeof(cmdTimeout), &actual_length, 0);	
	
	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmd0x1b, sizeof(cmd0x1b), &actual_length, 0);
	
	printk(KERN_INFO "Nvidia 3D Vision : rate set to %d Hz\n", rate);
	
	return rate;
}

void nv3d_swap_callback(struct urb *urb)
{
	/* sync/async unlink faults aren't errors */
	 if (urb->status && 
		!(urb->status == -ENOENT || 
			urb->status == -ECONNRESET ||
			urb->status == -ESHUTDOWN)) {
				dbg("%s - nonzero write bulk status received: %d",__FUNCTION__, urb->status);
	}
	
	printk(KERN_INFO "Nvidia 3D Vision : eyes swapped %d\n", nv3d->eye);
	
	/* free up our allocated buffer */
	//usb_buffer_free(urb->dev, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma);
}

void nv3d_swap(void)
{
	
	int retval = 0;
	struct urb *urb = NULL;
	
	uint32_t r = NVSTUSB_T2_COUNT((int)((1000000/120)/1.8));
	r = 0;
	
	uint8_t buf[] = {
		NVSTUSB_CMD_SET_EYE, /* set shutter state */
		(nv3d->eye == 0) ? 0xFE : 0xFF, /* eye selection */
		0x00, 0x00, /* unused */
		r>>8&0xFF, r>>16&0xFF, 0xFF,0xFF
		/*r, r>>8, r>>16, r>>24*/
	}; 
	
	/* create a urb, and a buffer for it, and copy the data to the urb */
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		retval = -ENOMEM;
		goto error;
	}

	/* initialize the urb properly */
	usb_fill_bulk_urb(urb, nv3d->udev, usb_sndbulkpipe(nv3d->udev, 1), buf, sizeof(buf), nv3d_swap_callback, nv3d);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	
	
	/* send the data out the bulk port */
	retval = usb_submit_urb(urb, GFP_KERNEL);

	if (retval) {
		err("%s - failed submitting write urb, error %d", __FUNCTION__, retval);
		goto error;
	}
	
	/* release our reference to this urb, the USB core will eventually free it entirely */
	usb_free_urb(urb);
	
	nv3d->eye = !nv3d->eye;

	return;

error:
	//usb_buffer_free(nv3d->udev, sizeof(buf), buf, urb->transfer_dma);
	usb_free_urb(urb);
	return;
}
