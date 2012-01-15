int dev3d_probe(struct usb_interface *interface, const struct usb_device_id *id);
void dev3d_disconnect(struct usb_interface *interface);
int __init glasses3d_init(void);
void __exit glasses3d_exit(void);
