#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USB_XHCI_H
#ವ್ಯಾಖ್ಯಾನಿಸು __USB_XHCI_H

ರಚನೆ usbdevice_s;
ರಚನೆ usb_endpoint_descriptor;
ರಚನೆ usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
ಶೂನ್ಯ xhci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ರಚನೆ usb_pipe *xhci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev
                                   , ರಚನೆ usb_pipe *upipe
                                   , ರಚನೆ usb_endpoint_descriptor *epdesc);
ಪೂರ್ಣಾಂಕ xhci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
                   , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize);
ಪೂರ್ಣಾಂಕ xhci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ);

// --------------------------------------------------------------
// register interface

// capabilities
ರಚನೆ xhci_caps {
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
ರಚನೆ xhci_xcap {
    u32 cap;
    u32 ದತ್ತಾಂಶ[];
} PACKED;

// operational registers
ರಚನೆ xhci_op {
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
ರಚನೆ xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
ರಚನೆ xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
ರಚನೆ xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
ರಚನೆ xhci_ir {
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
ರಚನೆ xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
ರಚನೆ xhci_epctx {
    u32 ctx[2];
    u32 deq_ಕೆಳಗಿನ;
    u32 deq_ಮೇಲಿನ;
    u32 ಉದ್ದಳತೆ;
    u32 reserved_01[3];
} PACKED;

// device context array element
ರಚನೆ xhci_devlist {
    u32 ptr_ಕೆಳಗಿನ;
    u32 ptr_ಮೇಲಿನ;
} PACKED;

// input context
ರಚನೆ xhci_inctx {
    u32 del;
    u32 ಕೂಡಿಸು;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
ರಚನೆ xhci_trb {
    u32 ptr_ಕೆಳಗಿನ;
    u32 ptr_ಮೇಲಿನ;
    u32 ಸ್ಥಿತಿ_2;
    u32 control;
} PACKED;

// event ring segment
ರಚನೆ xhci_er_seg {
    u32 ptr_ಕೆಳಗಿನ;
    u32 ptr_ಮೇಲಿನ;
    u32 ಗಾತ್ರ_2;
    u32 reserved_01;
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // usb-xhci.h
