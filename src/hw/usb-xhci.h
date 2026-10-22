#ifndef __USB_XHCI_H
#define __USB_XHCI_H

ഘടന usbdevice_s;
ഘടന usb_endpoint_descriptor;
ഘടന usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
ശൂന്യം xhci_setup(ശൂന്യം);
ഘടന usb_pipe *xhci_realloc_pipe(ഘടന usbdevice_s *usbdev
                                   , ഘടന usb_pipe *upipe
                                   , ഘടന usb_endpoint_descriptor *epdesc);
പൂർണ്ണസംഖ്യ xhci_send_pipe(ഘടന usb_pipe *p, പൂർണ്ണസംഖ്യ dir, സ്ഥിരം ശൂന്യം *cmd
                   , ശൂന്യം *data, പൂർണ്ണസംഖ്യ datasize);
പൂർണ്ണസംഖ്യ xhci_poll_intr(ഘടന usb_pipe *p, ശൂന്യം *data);

// --------------------------------------------------------------
// register interface

// capabilities
ഘടന xhci_caps {
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
ഘടന xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
ഘടന xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_താഴെയുള്ള;
    u32 crcr_മുകളിലെ;
    u32 reserved_02[4];
    u32 dcbaap_താഴെയുള്ള;
    u32 dcbaap_മുകളിലെ;
    u32 config;
} PACKED;

// port registers
ഘടന xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
ഘടന xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
ഘടന xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
ഘടന xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_താഴെയുള്ള;
    u32 erstba_മുകളിലെ;
    u32 erdp_താഴെയുള്ള;
    u32 erdp_മുകളിലെ;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
ഘടന xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
ഘടന xhci_epctx {
    u32 ctx[2];
    u32 deq_താഴെയുള്ള;
    u32 deq_മുകളിലെ;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
ഘടന xhci_devlist {
    u32 ptr_താഴെയുള്ള;
    u32 ptr_മുകളിലെ;
} PACKED;

// input context
ഘടന xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
ഘടന xhci_trb {
    u32 ptr_താഴെയുള്ള;
    u32 ptr_മുകളിലെ;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
ഘടന xhci_er_seg {
    u32 ptr_താഴെയുള്ള;
    u32 ptr_മുകളിലെ;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
