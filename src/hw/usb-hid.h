#که_نه_تعریف_شوی __USB_HID_H
#تعریف_کړه __USB_HID_H

// usb-hid.c
بنية usbdevice_s;
صحيح usb_hid_تیاری(بنية usbdevice_s *usbdev);
صحيح usb_kbd_فعال(فراغ);
صحيح usb_kbd_command(صحيح command, u8 *param);
صحيح usb_mouse_فعال(فراغ);
صحيح usb_mouse_command(صحيح command, u8 *param);
فراغ usb_check_event(فراغ);


/****************************************************************
 * hid flags
 ****************************************************************/

#تعریف_کړه USB_INTERFACE_SUBCLASS_BOOT     1
#تعریف_کړه USB_INTERFACE_PROTOCOL_KEYBOARD 1
#تعریف_کړه USB_INTERFACE_PROTOCOL_MOUSE    2

#تعریف_کړه HID_REQ_GET_REPORT              0x01
#تعریف_کړه HID_REQ_GET_IDLE                0x02
#تعریف_کړه HID_REQ_GET_PROTOCOL            0x03
#تعریف_کړه HID_REQ_سېټ_REPORT              0x09
#تعریف_کړه HID_REQ_سېټ_IDLE                0x0A
#تعریف_کړه HID_REQ_سېټ_PROTOCOL            0x0B

#د_که_پای // ush-hid.h
