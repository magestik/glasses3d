#include "nv3d.h"

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/usb.h>
#include <linux/interrupt.h>

#include "glasses3d.h"
#include "usb.h"

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

int nv3d_init(struct usb_interface *interface) {
	struct usb_glasses *dev;
	dev = usb_get_intfdata (interface);
	
	if(interface->cur_altsetting->desc.bNumEndpoints == 0) {
		printk(KERN_NOTICE "0 Endpoint: must upload nvidia emitter firmware\n");
		nv3d_download_firmware(dev->udev); // Upload the firmware to the USB device
		dev->active = 0; // glasses are not active
	} else {
		printk(KERN_NOTICE "%d Endpoints\n", interface->cur_altsetting->desc.bNumEndpoints);
		dev->active = 1; // glasses are active
		nv3d_set_rate(dev->udev, refresh_rate);
	}

	dev->swap_eyes = nv3d_swap; // swap function
	dev->inversed = 0; // eyes are not inversed
	
	return 0;
}

int nv3d_set_rate(struct usb_device *udev, int rate) {

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
	
	uint16_t timeout = rate * 4; 
	
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
	
	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmdTimings, sizeof(cmdTimings), &actual_length, 0);
	
	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmd0x1c, sizeof(cmd0x1c), &actual_length, 0);

	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmdTimeout, sizeof(cmdTimeout), &actual_length, 0);	
	
	n = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 2), cmd0x1b, sizeof(cmd0x1b), &actual_length, 0);
	
	printk(KERN_INFO "Nvidia 3D Vision : rate set to %d Hz\n", rate);
	
	return rate;
}

void nv3d_swap(struct usb_device *udev, int eye) {
	int e, actual_length;
	
	uint32_t r = NVSTUSB_T2_COUNT((1000000/refresh_rate)); // divisé par 1.8 ?
	
	uint8_t buf[8] = {
		NVSTUSB_CMD_SET_EYE, /* set shutter state */
		(eye) ? 0xFE : 0xFF, /* eye selection */
		0x00, 0x00, /* unused */
		r, r>>8, r>>16, r>>24
	};
	
	e = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 1), buf, sizeof(buf), &actual_length, 0); // TODO: make this asynchronous
}

/*
 * load the firmware to the device
 */
 
int nv3d_usb_hexline(const struct firmware *fw, struct hexline *hx, int *pos) {
	u8 *b;
	int data_offs = 4;
	
	if (*pos+data_offs >= fw->size) {
		printk("glasses3d fin firmware");
		return 0;
	}
	
	b = (u8 *) &fw->data[*pos];
	
	memset(hx, 0, sizeof(struct hexline));
	
	hx->len = (b[0]<<8) | b[1];	
	hx->addr = (b[2]<<8) | b[3];
	printk("len=%d , addr=%d", hx->len, hx->addr );

	memcpy(hx->data, &b[data_offs], hx->len);
	hx->chk = b[data_offs + hx->len];
	
	*pos += 4 + hx->len;
	return 4 + hx->len;
}

int usb_nv3d_loadfw(struct usb_device *udev, const struct firmware *fw) {
	int err; // retour de l'usb
	int count = 0;
	int ret;
	struct hexline hx;
	
	while ((ret = nv3d_usb_hexline(fw, &hx, &count)) > 0) {
		printk(KERN_INFO "Writing length: %d at pos: %d", hx.len, hx.addr);
	
		err = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0xA0, USB_TYPE_VENDOR, hx.addr, 0x00, hx.data, hx.len, 0);
	
		if (err < 0) {
			return err;
		}
	}
	
	return 0;
}

int nv3d_download_firmware(struct usb_device *udev) {
	int ret;
	char *filename = "nvstusb.fw";
	const struct firmware *fw = NULL;
	

	if ((ret = request_firmware(&fw, filename, &udev->dev)) != 0) {
		printk(KERN_ALERT "Error: Did not find the firmware file %s (errno=%d).", filename, ret);
		return ret;
	}
	
	ret = usb_nv3d_loadfw(udev, fw);
	
	release_firmware(fw);
	
	return ret;
}
