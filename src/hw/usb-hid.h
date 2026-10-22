#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
কাঠামো usbdevice_s;
পূর্ণসংখ্যা usb_hid_setup(কাঠামো usbdevice_s *usbdev);
পূর্ণসংখ্যা usb_kbd_active(শূন্য);
পূর্ণসংখ্যা usb_kbd_command(পূর্ণসংখ্যা command, u8 *param);
পূর্ণসংখ্যা usb_mouse_active(শূন্য);
পূর্ণসংখ্যা usb_mouse_command(পূর্ণসংখ্যা command, u8 *param);
শূন্য usb_check_event(শূন্য);


/****************************************************************
 * hid flags
 ****************************************************************/

#define USB_INTERFACE_SUBCLASS_BOOT     1
#define USB_INTERFACE_PROTOCOL_KEYBOARD 1
#define USB_INTERFACE_PROTOCOL_MOUSE    2

#define HID_REQ_GET_REPORT              0x01
#define HID_REQ_GET_IDLE                0x02
#define HID_REQ_GET_PROTOCOL            0x03
#define HID_REQ_SET_REPORT              0x09
#define HID_REQ_SET_IDLE                0x0A
#define HID_REQ_SET_PROTOCOL            0x0B

#endif // ush-hid.h
