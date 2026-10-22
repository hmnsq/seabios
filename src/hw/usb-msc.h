#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
কাঠামো disk_op_s;
পূর্ণসংখ্যা usb_process_op(কাঠামো disk_op_s *op);
কাঠামো usbdevice_s;
পূর্ণসংখ্যা usb_msc_setup(কাঠামো usbdevice_s *usbdev);

#endif // ush-msc.h
