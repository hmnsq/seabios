#ifndef __USB_UAS_H
#define __USB_UAS_H

sa_nracanaa disk_op_s;
จำนวนเต็ม uas_process_op(sa_nracanaa disk_op_s *op);
sa_nracanaa usbdevice_s;
จำนวนเต็ม usb_uas_setup(sa_nracanaa usbdevice_s *usbdev);

#endif /* __USB_UAS_H */
