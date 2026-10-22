#ifndef __USB_XHCI_H
#define __USB_XHCI_H

கட்டமைப்பு usbdevice_s;
கட்டமைப்பு usb_endpoint_descriptor;
கட்டமைப்பு usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
வெறுமை xhci_setup(வெறுமை);
கட்டமைப்பு usb_pipe *xhci_realloc_pipe(கட்டமைப்பு usbdevice_s *usbdev
                                   , கட்டமைப்பு usb_pipe *upipe
                                   , கட்டமைப்பு usb_endpoint_descriptor *epdesc);
முழு xhci_send_pipe(கட்டமைப்பு usb_pipe *p, முழு dir, மாறிலி வெறுமை *cmd
                   , வெறுமை *data, முழு datasize);
முழு xhci_poll_intr(கட்டமைப்பு usb_pipe *p, வெறுமை *data);

// --------------------------------------------------------------
// register interface

// capabilities
கட்டமைப்பு xhci_caps {
    u8  caplength;
    u8  reserved_01;
    u16 hciversion;
    u32 hcsparams1;
    u32 hcsparams2;
    u32 hcsparams3;
    u32 hccparams;
    u32 dboff;
    u32 rtsoff;
} PACKED;

// extended capabilities
கட்டமைப்பு xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
கட்டமைப்பு xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_கீழ்;
    u32 crcr_உயர்வு;
    u32 reserved_02[4];
    u32 dcbaap_கீழ்;
    u32 dcbaap_உயர்வு;
    u32 config;
} PACKED;

// port registers
கட்டமைப்பு xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
கட்டமைப்பு xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
கட்டமைப்பு xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
கட்டமைப்பு xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_கீழ்;
    u32 erstba_உயர்வு;
    u32 erdp_கீழ்;
    u32 erdp_உயர்வு;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
கட்டமைப்பு xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
கட்டமைப்பு xhci_epctx {
    u32 ctx[2];
    u32 deq_கீழ்;
    u32 deq_உயர்வு;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
கட்டமைப்பு xhci_devlist {
    u32 ptr_கீழ்;
    u32 ptr_உயர்வு;
} PACKED;

// input context
கட்டமைப்பு xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
கட்டமைப்பு xhci_trb {
    u32 ptr_கீழ்;
    u32 ptr_உயர்வு;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
கட்டமைப்பு xhci_er_seg {
    u32 ptr_கீழ்;
    u32 ptr_உயர்வு;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
