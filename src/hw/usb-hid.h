#اگر_غیر_تعریف_شدہ __USB_HID_H
#تعریف_کرو __USB_HID_H

// usb-hid.c
ساخت usbdevice_s;
صحیح usb_hid_تیاری(ساخت usbdevice_s *usbdev);
صحیح usb_kbd_فعال(خالی);
صحیح usb_kbd_command(صحیح command, u8 *param);
صحیح usb_mouse_فعال(خالی);
صحیح usb_mouse_command(صحیح command, u8 *param);
خالی usb_check_event(خالی);


/****************************************************************
 * hid flags
 ****************************************************************/

#تعریف_کرو USB_INTERFACE_SUBCLASS_BOOT     1
#تعریف_کرو USB_INTERFACE_PROTOCOL_KEYBOARD 1
#تعریف_کرو USB_INTERFACE_PROTOCOL_MOUSE    2

#تعریف_کرو HID_REQ_GET_REPORT              0x01
#تعریف_کرو HID_REQ_GET_IDLE                0x02
#تعریف_کرو HID_REQ_GET_PROTOCOL            0x03
#تعریف_کرو HID_REQ_سیٹ_REPORT              0x09
#تعریف_کرو HID_REQ_سیٹ_IDLE                0x0A
#تعریف_کرو HID_REQ_سیٹ_PROTOCOL            0x0B

#اگر_ختم // ush-hid.h
