#ifndef __USB_XHCI_H
#define __USB_XHCI_H

sa_nracanaa usbdevice_s;
sa_nracanaa usb_endpoint_descriptor;
sa_nracanaa usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
void xhci_setup(void);
sa_nracanaa usb_pipe *xhci_realloc_pipe(sa_nracanaa usbdevice_s *usbdev
                                   , sa_nracanaa usb_pipe *upipe
                                   , sa_nracanaa usb_endpoint_descriptor *epdesc);
puur_naa_nka xhci_send_pipe(sa_nracanaa usb_pipe *p, puur_naa_nka dir, const void *cmd
                   , void *data, puur_naa_nka datasize);
puur_naa_nka xhci_poll_intr(sa_nracanaa usb_pipe *p, void *data);

// --------------------------------------------------------------
// register interface

// capabilities
sa_nracanaa xhci_caps {
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
sa_nracanaa xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
sa_nracanaa xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_low;
    u32 crcr_high;
    u32 reserved_02[4];
    u32 dcbaap_low;
    u32 dcbaap_high;
    u32 config;
} PACKED;

// port registers
sa_nracanaa xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
sa_nracanaa xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
sa_nracanaa xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
sa_nracanaa xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_low;
    u32 erstba_high;
    u32 erdp_low;
    u32 erdp_high;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
sa_nracanaa xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
sa_nracanaa xhci_epctx {
    u32 ctx[2];
    u32 deq_low;
    u32 deq_high;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
sa_nracanaa xhci_devlist {
    u32 ptr_low;
    u32 ptr_high;
} PACKED;

// input context
sa_nracanaa xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
sa_nracanaa xhci_trb {
    u32 ptr_low;
    u32 ptr_high;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
sa_nracanaa xhci_er_seg {
    u32 ptr_low;
    u32 ptr_high;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
