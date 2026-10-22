#ifndef __USB_UAS_H
#define __USB_UAS_H

ସଂରଚନା disk_op_s;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା uas_process_op(ସଂରଚନା disk_op_s *op);
ସଂରଚନା usbdevice_s;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_uas_setup(ସଂରଚନା usbdevice_s *usbdev);

#endif /* __USB_UAS_H */
