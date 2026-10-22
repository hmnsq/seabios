#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
संरचना usbdevice_s;
पूर्णांक usb_hid_setup(संरचना usbdevice_s *usbdev);
पूर्णांक usb_kbd_active(शून्य);
पूर्णांक usb_kbd_command(पूर्णांक command, u8 *param);
पूर्णांक usb_mouse_active(शून्य);
पूर्णांक usb_mouse_command(पूर्णांक command, u8 *param);
शून्य usb_check_event(शून्य);


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
