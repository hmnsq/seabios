#اگر_غیر_تعریف_شدہ __USB_XHCI_H
#تعریف_کرو __USB_XHCI_H

ساخت usbdevice_s;
ساخت usb_endpoint_descriptor;
ساخت usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
خالی xhci_تیاری(خالی);
ساخت usb_pipe *xhci_realloc_pipe(ساخت usbdevice_s *usbdev
                                   , ساخت usb_pipe *upipe
                                   , ساخت usb_endpoint_descriptor *epdesc);
صحیح xhci_send_pipe(ساخت usb_pipe *p, صحیح dir, ثابت خالی *cmd
                   , خالی *ڈیٹا, صحیح datasize);
صحیح xhci_poll_intr(ساخت usb_pipe *p, خالی *ڈیٹا);

// --------------------------------------------------------------
// register interface

// capabilities
ساخت xhci_caps {
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
ساخت xhci_xcap {
    u32 cap;
    u32 ڈیٹا[];
} PACKED;

// operational registers
ساخت xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_کم;
    u32 crcr_بلند;
    u32 reserved_02[4];
    u32 dcbaap_کم;
    u32 dcbaap_بلند;
    u32 config;
} PACKED;

// port registers
ساخت xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
ساخت xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
ساخت xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
ساخت xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_کم;
    u32 erstba_بلند;
    u32 erdp_کم;
    u32 erdp_بلند;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
ساخت xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
ساخت xhci_epctx {
    u32 ctx[2];
    u32 deq_کم;
    u32 deq_بلند;
    u32 لمبائی;
    u32 reserved_01[3];
} PACKED;

// device context array element
ساخت xhci_devlist {
    u32 ptr_کم;
    u32 ptr_بلند;
} PACKED;

// input context
ساخت xhci_inctx {
    u32 del;
    u32 جوڑو;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
ساخت xhci_trb {
    u32 ptr_کم;
    u32 ptr_بلند;
    u32 حالت_2;
    u32 control;
} PACKED;

// event ring segment
ساخت xhci_er_seg {
    u32 ptr_کم;
    u32 ptr_بلند;
    u32 سائز;
    u32 reserved_01;
} PACKED;

#اگر_ختم // usb-xhci.h
