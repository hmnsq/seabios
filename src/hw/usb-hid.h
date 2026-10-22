#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USB_HID_H
#ವ್ಯಾಖ್ಯಾನಿಸು __USB_HID_H

// usb-hid.c
ರಚನೆ usbdevice_s;
ಪೂರ್ಣಾಂಕ usb_hid_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev);
ಪೂರ್ಣಾಂಕ usb_kbd_ಸಕ್ರಿಯ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ usb_kbd_command(ಪೂರ್ಣಾಂಕ command, u8 *param);
ಪೂರ್ಣಾಂಕ usb_mouse_ಸಕ್ರಿಯ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ usb_mouse_command(ಪೂರ್ಣಾಂಕ command, u8 *param);
ಶೂನ್ಯ usb_check_event(ಶೂನ್ಯ);


/****************************************************************
 * hid flags
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು USB_INTERFACE_SUBCLASS_BOOT     1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_INTERFACE_PROTOCOL_KEYBOARD 1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_INTERFACE_PROTOCOL_MOUSE    2

#ವ್ಯಾಖ್ಯಾನಿಸು HID_REQ_GET_REPORT              0x01
#ವ್ಯಾಖ್ಯಾನಿಸು HID_REQ_GET_IDLE                0x02
#ವ್ಯಾಖ್ಯಾನಿಸು HID_REQ_GET_PROTOCOL            0x03
#ವ್ಯಾಖ್ಯಾನಿಸು HID_REQ_ಗಣ_REPORT              0x09
#ವ್ಯಾಖ್ಯಾನಿಸು HID_REQ_ಗಣ_IDLE                0x0A
#ವ್ಯಾಖ್ಯಾನಿಸು HID_REQ_ಗಣ_PROTOCOL            0x0B

#ಆದರೆ_ಅಂತ್ಯ // ush-hid.h
