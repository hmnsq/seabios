#ifndef __USB_XHCI_H
#define __USB_XHCI_H

ಸಂರಚನಾ usbdevice_s;
ಸಂರಚನಾ usb_endpoint_descriptor;
ಸಂರಚನಾ usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
ಶೂನ್ಯ xhci_setup(ಶೂನ್ಯ);
ಸಂರಚನಾ usb_pipe *xhci_realloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev
                                   , ಸಂರಚನಾ usb_pipe *upipe
                                   , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc);
ಪೂರ್ಣಾಙ್ಕ xhci_send_pipe(ಸಂರಚನಾ usb_pipe *p, ಪೂರ್ಣಾಙ್ಕ dir, ಧ್ರುವ ಶೂನ್ಯ *cmd
                   , ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datasize);
ಪೂರ್ಣಾಙ್ಕ xhci_poll_intr(ಸಂರಚನಾ usb_pipe *p, ಶೂನ್ಯ *data);

// --------------------------------------------------------------
// register interface

// capabilities
ಸಂರಚನಾ xhci_caps {
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
ಸಂರಚನಾ xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
ಸಂರಚನಾ xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_ಕೆಳಗಿನ;
    u32 crcr_ಮೇಲಿನ;
    u32 reserved_02[4];
    u32 dcbaap_ಕೆಳಗಿನ;
    u32 dcbaap_ಮೇಲಿನ;
    u32 config;
} PACKED;

// port registers
ಸಂರಚನಾ xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
ಸಂರಚನಾ xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
ಸಂರಚನಾ xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
ಸಂರಚನಾ xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_ಕೆಳಗಿನ;
    u32 erstba_ಮೇಲಿನ;
    u32 erdp_ಕೆಳಗಿನ;
    u32 erdp_ಮೇಲಿನ;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
ಸಂರಚನಾ xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
ಸಂರಚನಾ xhci_epctx {
    u32 ctx[2];
    u32 deq_ಕೆಳಗಿನ;
    u32 deq_ಮೇಲಿನ;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
ಸಂರಚನಾ xhci_devlist {
    u32 ptr_ಕೆಳಗಿನ;
    u32 ptr_ಮೇಲಿನ;
} PACKED;

// input context
ಸಂರಚನಾ xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
ಸಂರಚನಾ xhci_trb {
    u32 ptr_ಕೆಳಗಿನ;
    u32 ptr_ಮೇಲಿನ;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
ಸಂರಚನಾ xhci_er_seg {
    u32 ptr_ಕೆಳಗಿನ;
    u32 ptr_ಮೇಲಿನ;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
