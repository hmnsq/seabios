#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
கட்டமைப்பு usbdevice_s;
முழு usb_hid_setup(கட்டமைப்பு usbdevice_s *usbdev);
முழு usb_kbd_active(வெறுமை);
முழு usb_kbd_command(முழு command, u8 *param);
முழு usb_mouse_active(வெறுமை);
முழு usb_mouse_command(முழு command, u8 *param);
வெறுமை usb_check_event(வெறுமை);


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
