#ifndef __USB_UAS_H
#define __USB_UAS_H

ساختار disk_op_s;
صحیح uas_process_op(ساختار disk_op_s *op);
ساختار usbdevice_s;
صحیح usb_uas_setup(ساختار usbdevice_s *usbdev);

#endif /* __USB_UAS_H */
