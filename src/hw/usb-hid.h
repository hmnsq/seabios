#جے_بنا_تعریف __USB_HID_H
#تعریف_کرو __USB_HID_H

// usb-hid.c
بنية usbdevice_s;
صحيح usb_hid_تیاری(بنية usbdevice_s *usbdev);
صحيح usb_kbd_سرگرم(فراغ);
صحيح usb_kbd_command(صحيح command, u8 *param);
صحيح usb_mouse_سرگرم(فراغ);
صحيح usb_mouse_command(صحيح command, u8 *param);
فراغ usb_check_event(فراغ);


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

#جے_انت // ush-hid.h
