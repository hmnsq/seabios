#ifndef __USB_XHCI_H
#define __USB_XHCI_H

ସଂରଚନା usbdevice_s;
ସଂରଚନା usb_endpoint_descriptor;
ସଂରଚନା usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
ଶୂନ୍ୟ xhci_setup(ଶୂନ୍ୟ);
ସଂରଚନା usb_pipe *xhci_realloc_pipe(ସଂରଚନା usbdevice_s *usbdev
                                   , ସଂରଚନା usb_pipe *upipe
                                   , ସଂରଚନା usb_endpoint_descriptor *epdesc);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_send_pipe(ସଂରଚନା usb_pipe *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଅଚଳ ଶୂନ୍ୟ *cmd
                   , ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datasize);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_poll_intr(ସଂରଚନା usb_pipe *p, ଶୂନ୍ୟ *data);

// --------------------------------------------------------------
// register interface

// capabilities
ସଂରଚନା xhci_caps {
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
ସଂରଚନା xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
ସଂରଚନା xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_ତଳ;
    u32 crcr_ଉପର;
    u32 reserved_02[4];
    u32 dcbaap_ତଳ;
    u32 dcbaap_ଉପର;
    u32 config;
} PACKED;

// port registers
ସଂରଚନା xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
ସଂରଚନା xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
ସଂରଚନା xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
ସଂରଚନା xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_ତଳ;
    u32 erstba_ଉପର;
    u32 erdp_ତଳ;
    u32 erdp_ଉପର;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
ସଂରଚନା xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
ସଂରଚନା xhci_epctx {
    u32 ctx[2];
    u32 deq_ତଳ;
    u32 deq_ଉପର;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
ସଂରଚନା xhci_devlist {
    u32 ptr_ତଳ;
    u32 ptr_ଉପର;
} PACKED;

// input context
ସଂରଚନା xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
ସଂରଚନା xhci_trb {
    u32 ptr_ତଳ;
    u32 ptr_ଉପର;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
ସଂରଚନା xhci_er_seg {
    u32 ptr_ତଳ;
    u32 ptr_ଉପର;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
