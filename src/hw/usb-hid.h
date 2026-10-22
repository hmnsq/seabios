#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
بنية usbdevice_s;
صحيح usb_hid_setup(بنية usbdevice_s *usbdev);
صحيح usb_kbd_active(فراغ);
صحيح usb_kbd_command(صحيح command, u8 *param);
صحيح usb_mouse_active(فراغ);
صحيح usb_mouse_command(صحيح command, u8 *param);
فراغ usb_check_event(فراغ);


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
