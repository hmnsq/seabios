#اگر_تعریف_نشده __USB_HID_H
#تعریف_کن __USB_HID_H

// usb-hid.c
بنية usbdevice_s;
صحيح usb_hid_راهاندازی(بنية usbdevice_s *usbdev);
صحيح usb_kbd_فعال(فراغ);
صحيح usb_kbd_command(صحيح command, u8 *param);
صحيح usb_mouse_فعال(فراغ);
صحيح usb_mouse_command(صحيح command, u8 *param);
فراغ usb_check_event(فراغ);


/****************************************************************
 * hid flags
 ****************************************************************/

#تعریف_کن USB_INTERFACE_SUBCLASS_BOOT     1
#تعریف_کن USB_INTERFACE_PROTOCOL_KEYBOARD 1
#تعریف_کن USB_INTERFACE_PROTOCOL_MOUSE    2

#تعریف_کن HID_REQ_GET_REPORT              0x01
#تعریف_کن HID_REQ_GET_IDLE                0x02
#تعریف_کن HID_REQ_GET_PROTOCOL            0x03
#تعریف_کن HID_REQ_مجموعه_REPORT              0x09
#تعریف_کن HID_REQ_مجموعه_IDLE                0x0A
#تعریف_کن HID_REQ_مجموعه_PROTOCOL            0x0B

#پایان_اگر // ush-hid.h
