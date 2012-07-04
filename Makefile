obj-m := module3d.o
module3d-objs := glasses3d.o sync.o usb.o nv3d.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	mv module3d.ko glasses3d.ko

install:
	cp glasses3d.ko /lib/modules/glasses3d.ko
	#wget NVIDIA_stereo_controller.exe
	#cabextract
	#firmware_extract
	#cp nvstusb.fw /lib/firmware/nvstusb.fw
	
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
