#جيڪڏهن_وصف_نه_ٿيل __USB_HID_H
#وصف_ڪريو __USB_HID_H

// usb-hid.c
بنية usbdevice_s;
صحيح usb_hid_تياري(بنية usbdevice_s *usbdev);
صحيح usb_kbd_سرگرم(فراغ);
صحيح usb_kbd_command(صحيح command, u8 *param);
صحيح usb_mouse_سرگرم(فراغ);
صحيح usb_mouse_command(صحيح command, u8 *param);
فراغ usb_check_event(فراغ);


/****************************************************************
 * hid flags
 ****************************************************************/

#وصف_ڪريو USB_INTERFACE_SUBCLASS_BOOT     1
#وصف_ڪريو USB_INTERFACE_PROTOCOL_KEYBOARD 1
#وصف_ڪريو USB_INTERFACE_PROTOCOL_MOUSE    2

#وصف_ڪريو HID_REQ_GET_REPORT              0x01
#وصف_ڪريو HID_REQ_GET_IDLE                0x02
#وصف_ڪريو HID_REQ_GET_PROTOCOL            0x03
#وصف_ڪريو HID_REQ_سيٽ_REPORT              0x09
#وصف_ڪريو HID_REQ_سيٽ_IDLE                0x0A
#وصف_ڪريو HID_REQ_سيٽ_PROTOCOL            0x0B

#جيڪڏهن_پورو // ush-hid.h
