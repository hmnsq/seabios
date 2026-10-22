#ifndef __USB_UAS_H
#define __USB_UAS_H

संरचना disk_op_s;
पूर्णाङ्क uas_process_op(संरचना disk_op_s *op);
संरचना usbdevice_s;
पूर्णाङ्क usb_uas_setup(संरचना usbdevice_s *usbdev);

#endif /* __USB_UAS_H */
