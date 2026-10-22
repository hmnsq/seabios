#ifndef __USB_XHCI_H
#define __USB_XHCI_H

સંરચના usbdevice_s;
સંરચના usb_endpoint_descriptor;
સંરચના usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
શૂન્ય xhci_setup(શૂન્ય);
સંરચના usb_pipe *xhci_realloc_pipe(સંરચના usbdevice_s *usbdev
                                   , સંરચના usb_pipe *upipe
                                   , સંરચના usb_endpoint_descriptor *epdesc);
પૂર્ણાંક xhci_send_pipe(સંરચના usb_pipe *p, પૂર્ણાંક dir, અચલ શૂન્ય *cmd
                   , શૂન્ય *data, પૂર્ણાંક datasize);
પૂર્ણાંક xhci_poll_intr(સંરચના usb_pipe *p, શૂન્ય *data);

// --------------------------------------------------------------
// register interface

// capabilities
સંરચના xhci_caps {
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
સંરચના xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
સંરચના xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_નીચલું;
    u32 crcr_ઉપલું;
    u32 reserved_02[4];
    u32 dcbaap_નીચલું;
    u32 dcbaap_ઉપલું;
    u32 config;
} PACKED;

// port registers
સંરચના xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
સંરચના xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
સંરચના xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
સંરચના xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_નીચલું;
    u32 erstba_ઉપલું;
    u32 erdp_નીચલું;
    u32 erdp_ઉપલું;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
સંરચના xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
સંરચના xhci_epctx {
    u32 ctx[2];
    u32 deq_નીચલું;
    u32 deq_ઉપલું;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
સંરચના xhci_devlist {
    u32 ptr_નીચલું;
    u32 ptr_ઉપલું;
} PACKED;

// input context
સંરચના xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
સંરચના xhci_trb {
    u32 ptr_નીચલું;
    u32 ptr_ઉપલું;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
સંરચના xhci_er_seg {
    u32 ptr_નીચલું;
    u32 ptr_ઉપલું;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
