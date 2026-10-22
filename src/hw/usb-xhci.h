#ifndef __USB_XHCI_H
#define __USB_XHCI_H

সংরচনা usbdevice_s;
সংরচনা usb_endpoint_descriptor;
সংরচনা usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
শূন্য xhci_setup(শূন্য);
সংরচনা usb_pipe *xhci_realloc_pipe(সংরচনা usbdevice_s *usbdev
                                   , সংরচনা usb_pipe *upipe
                                   , সংরচনা usb_endpoint_descriptor *epdesc);
পূর্ণাঙ্ক xhci_send_pipe(সংরচনা usb_pipe *p, পূর্ণাঙ্ক dir, ধ্রুৱ শূন্য *cmd
                   , শূন্য *data, পূর্ণাঙ্ক datasize);
পূর্ণাঙ্ক xhci_poll_intr(সংরচনা usb_pipe *p, শূন্য *data);

// --------------------------------------------------------------
// register interface

// capabilities
সংরচনা xhci_caps {
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
সংরচনা xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
সংরচনা xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_নিচেৰ;
    u32 crcr_উপৰেৰ;
    u32 reserved_02[4];
    u32 dcbaap_নিচেৰ;
    u32 dcbaap_উপৰেৰ;
    u32 config;
} PACKED;

// port registers
সংরচনা xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
সংরচনা xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
সংরচনা xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
সংরচনা xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_নিচেৰ;
    u32 erstba_উপৰেৰ;
    u32 erdp_নিচেৰ;
    u32 erdp_উপৰেৰ;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
সংরচনা xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
সংরচনা xhci_epctx {
    u32 ctx[2];
    u32 deq_নিচেৰ;
    u32 deq_উপৰেৰ;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
সংরচনা xhci_devlist {
    u32 ptr_নিচেৰ;
    u32 ptr_উপৰেৰ;
} PACKED;

// input context
সংরচনা xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
সংরচনা xhci_trb {
    u32 ptr_নিচেৰ;
    u32 ptr_উপৰেৰ;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
সংরচনা xhci_er_seg {
    u32 ptr_নিচেৰ;
    u32 ptr_উপৰেৰ;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
