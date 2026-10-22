#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
સંરચના usbdevice_s;
પૂર્ણાંક usb_hid_setup(સંરચના usbdevice_s *usbdev);
પૂર્ણાંક usb_kbd_active(શૂન્ય);
પૂર્ણાંક usb_kbd_command(પૂર્ણાંક command, u8 *param);
પૂર્ણાંક usb_mouse_active(શૂન્ય);
પૂર્ણાંક usb_mouse_command(પૂર્ણાંક command, u8 *param);
શૂન્ય usb_check_event(શૂન્ય);


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
