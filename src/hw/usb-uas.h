#ifndef __USB_UAS_H
#define __USB_UAS_H

ഘടന disk_op_s;
പൂർണ്ണസംഖ്യ uas_process_op(ഘടന disk_op_s *op);
ഘടന usbdevice_s;
പൂർണ്ണസംഖ്യ usb_uas_setup(ഘടന usbdevice_s *usbdev);

#endif /* __USB_UAS_H */
