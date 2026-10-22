#ifndef __USB_HID_H
#define __USB_HID_H

// usb-hid.c
struct usbdevice_s;
số_nguyên usb_hid_setup(struct usbdevice_s *usbdev);
số_nguyên usb_kbd_active(void);
số_nguyên usb_kbd_command(số_nguyên command, u8 *param);
số_nguyên usb_mouse_active(void);
số_nguyên usb_mouse_command(số_nguyên command, u8 *param);
void usb_check_event(void);


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
