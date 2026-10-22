#ifndef __USB_UAS_H
#define __USB_UAS_H

ಸಂರಚನಾ disk_op_s;
ಪೂರ್ಣಾಙ್ಕ uas_process_op(ಸಂರಚನಾ disk_op_s *op);
ಸಂರಚನಾ usbdevice_s;
ಪೂರ್ಣಾಙ್ಕ usb_uas_setup(ಸಂರಚನಾ usbdevice_s *usbdev);

#endif /* __USB_UAS_H */
