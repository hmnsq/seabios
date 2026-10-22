#ifndef __USB_XHCI_H
#define __USB_XHCI_H

ساختار usbdevice_s;
ساختار usb_endpoint_descriptor;
ساختار usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
تهی xhci_setup(تهی);
ساختار usb_pipe *xhci_realloc_pipe(ساختار usbdevice_s *usbdev
                                   , ساختار usb_pipe *upipe
                                   , ساختار usb_endpoint_descriptor *epdesc);
صحیح xhci_send_pipe(ساختار usb_pipe *p, صحیح dir, ثابت تهی *cmd
                   , تهی *data, صحیح datasize);
صحیح xhci_poll_intr(ساختار usb_pipe *p, تهی *data);

// --------------------------------------------------------------
// register interface

// capabilities
ساختار xhci_caps {
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
ساختار xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
ساختار xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_کم;
    u32 crcr_بالا;
    u32 reserved_02[4];
    u32 dcbaap_کم;
    u32 dcbaap_بالا;
    u32 config;
} PACKED;

// port registers
ساختار xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
ساختار xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
ساختار xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
ساختار xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_کم;
    u32 erstba_بالا;
    u32 erdp_کم;
    u32 erdp_بالا;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
ساختار xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
ساختار xhci_epctx {
    u32 ctx[2];
    u32 deq_کم;
    u32 deq_بالا;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
ساختار xhci_devlist {
    u32 ptr_کم;
    u32 ptr_بالا;
} PACKED;

// input context
ساختار xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
ساختار xhci_trb {
    u32 ptr_کم;
    u32 ptr_بالا;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
ساختار xhci_er_seg {
    u32 ptr_کم;
    u32 ptr_بالا;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
