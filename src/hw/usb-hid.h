#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
ಸಂರಚನಾ usbdevice_s;
ಪೂರ್ಣಾಙ್ಕ usb_hid_setup(ಸಂರಚನಾ usbdevice_s *usbdev);
ಪೂರ್ಣಾಙ್ಕ usb_kbd_active(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ usb_kbd_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param);
ಪೂರ್ಣಾಙ್ಕ usb_mouse_active(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ usb_mouse_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param);
ಶೂನ್ಯ usb_check_event(ಶೂನ್ಯ);


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
