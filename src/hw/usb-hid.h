#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
ഘടന usbdevice_s;
പൂർണ്ണസംഖ്യ usb_hid_setup(ഘടന usbdevice_s *usbdev);
പൂർണ്ണസംഖ്യ usb_kbd_active(ശൂന്യം);
പൂർണ്ണസംഖ്യ usb_kbd_command(പൂർണ്ണസംഖ്യ command, u8 *param);
പൂർണ്ണസംഖ്യ usb_mouse_active(ശൂന്യം);
പൂർണ്ണസംഖ്യ usb_mouse_command(പൂർണ്ണസംഖ്യ command, u8 *param);
ശൂന്യം usb_check_event(ശൂന്യം);


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
