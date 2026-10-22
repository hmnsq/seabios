#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USB_HID_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USB_HID_H

// usb-hid.c
ਬਣਤਰ usbdevice_s;
ਪੂਰਨ usb_hid_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev);
ਪੂਰਨ usb_kbd_ਸਰਗਰਮ(ਖਾਲੀ);
ਪੂਰਨ usb_kbd_command(ਪੂਰਨ command, u8 *param);
ਪੂਰਨ usb_mouse_ਸਰਗਰਮ(ਖਾਲੀ);
ਪੂਰਨ usb_mouse_command(ਪੂਰਨ command, u8 *param);
ਖਾਲੀ usb_check_event(ਖਾਲੀ);


/****************************************************************
 * hid flags
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_INTERFACE_SUBCLASS_BOOT     1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_INTERFACE_PROTOCOL_KEYBOARD 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_INTERFACE_PROTOCOL_MOUSE    2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HID_REQ_GET_REPORT              0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HID_REQ_GET_IDLE                0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HID_REQ_GET_PROTOCOL            0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HID_REQ_ਸੈੱਟ_REPORT              0x09
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HID_REQ_ਸੈੱਟ_IDLE                0x0A
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HID_REQ_ਸੈੱਟ_PROTOCOL            0x0B

#ਜੇ_ਅੰਤ // ush-hid.h
