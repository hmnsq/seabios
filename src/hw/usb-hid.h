#यदि_अपरिभाषितम् __USB_HID_H
#परिभाषय __USB_HID_H

// usb-hid.c
संरचना usbdevice_s;
पूर्णाङ्क usb_hid_सज्जता(संरचना usbdevice_s *usbdev);
पूर्णाङ्क usb_kbd_सक्रिय(शून्य);
पूर्णाङ्क usb_kbd_command(पूर्णाङ्क command, u8 *param);
पूर्णाङ्क usb_mouse_सक्रिय(शून्य);
पूर्णाङ्क usb_mouse_command(पूर्णाङ्क command, u8 *param);
शून्य usb_check_event(शून्य);


/****************************************************************
 * hid flags
 ****************************************************************/

#परिभाषय USB_INTERFACE_SUBCLASS_BOOT     1
#परिभाषय USB_INTERFACE_PROTOCOL_KEYBOARD 1
#परिभाषय USB_INTERFACE_PROTOCOL_MOUSE    2

#परिभाषय HID_REQ_GET_REPORT              0x01
#परिभाषय HID_REQ_GET_IDLE                0x02
#परिभाषय HID_REQ_GET_PROTOCOL            0x03
#परिभाषय HID_REQ_समुच्चयः_REPORT              0x09
#परिभाषय HID_REQ_समुच्चयः_IDLE                0x0A
#परिभाषय HID_REQ_समुच्चयः_PROTOCOL            0x0B

#यदि_अन्तः // ush-hid.h
