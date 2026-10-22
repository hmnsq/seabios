#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
ഘടന disk_op_s;
പൂർണ്ണസംഖ്യ usb_process_op(ഘടന disk_op_s *op);
ഘടന usbdevice_s;
പൂർണ്ണസംഖ്യ usb_msc_setup(ഘടന usbdevice_s *usbdev);

#endif // ush-msc.h
