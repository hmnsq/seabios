#నిర్వచితం_కాకపోతే __USB_HID_H
#నిర్వచించు __USB_HID_H

// usb-hid.c
నిర్మాణం usbdevice_s;
పూర్ణాంకం usb_hid_అమరిక(నిర్మాణం usbdevice_s *usbdev);
పూర్ణాంకం usb_kbd_చురుకైన(శూన్య);
పూర్ణాంకం usb_kbd_command(పూర్ణాంకం command, u8 *param);
పూర్ణాంకం usb_mouse_చురుకైన(శూన్య);
పూర్ణాంకం usb_mouse_command(పూర్ణాంకం command, u8 *param);
శూన్య usb_check_event(శూన్య);


/****************************************************************
 * hid flags
 ****************************************************************/

#నిర్వచించు USB_INTERFACE_SUBCLASS_BOOT     1
#నిర్వచించు USB_INTERFACE_PROTOCOL_KEYBOARD 1
#నిర్వచించు USB_INTERFACE_PROTOCOL_MOUSE    2

#నిర్వచించు HID_REQ_GET_REPORT              0x01
#నిర్వచించు HID_REQ_GET_IDLE                0x02
#నిర్వచించు HID_REQ_GET_PROTOCOL            0x03
#నిర్వచించు HID_REQ_సమితి_REPORT              0x09
#నిర్వచించు HID_REQ_సమితి_IDLE                0x0A
#నిర్వచించు HID_REQ_సమితి_PROTOCOL            0x0B

#అయితే_ముగింపు // ush-hid.h
