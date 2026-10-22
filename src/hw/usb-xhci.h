#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USB_XHCI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USB_XHCI_H

ਬਣਤਰ usbdevice_s;
ਬਣਤਰ usb_endpoint_descriptor;
ਬਣਤਰ usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
ਖਾਲੀ xhci_ਤਿਆਰੀ(ਖਾਲੀ);
ਬਣਤਰ usb_pipe *xhci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev
                                   , ਬਣਤਰ usb_pipe *upipe
                                   , ਬਣਤਰ usb_endpoint_descriptor *epdesc);
ਪੂਰਨ xhci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
                   , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize);
ਪੂਰਨ xhci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ);

// --------------------------------------------------------------
// register interface

// capabilities
ਬਣਤਰ xhci_caps {
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
ਬਣਤਰ xhci_xcap {
    u32 cap;
    u32 ਅੰਕੜੇ[];
} PACKED;

// operational registers
ਬਣਤਰ xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_ਨੀਵਾਂ;
    u32 crcr_ਉੱਚਾ;
    u32 reserved_02[4];
    u32 dcbaap_ਨੀਵਾਂ;
    u32 dcbaap_ਉੱਚਾ;
    u32 config;
} PACKED;

// port registers
ਬਣਤਰ xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
ਬਣਤਰ xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
ਬਣਤਰ xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
ਬਣਤਰ xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_ਨੀਵਾਂ;
    u32 erstba_ਉੱਚਾ;
    u32 erdp_ਨੀਵਾਂ;
    u32 erdp_ਉੱਚਾ;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
ਬਣਤਰ xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
ਬਣਤਰ xhci_epctx {
    u32 ctx[2];
    u32 deq_ਨੀਵਾਂ;
    u32 deq_ਉੱਚਾ;
    u32 ਲੰਮਾਈ;
    u32 reserved_01[3];
} PACKED;

// device context array element
ਬਣਤਰ xhci_devlist {
    u32 ptr_ਨੀਵਾਂ;
    u32 ptr_ਉੱਚਾ;
} PACKED;

// input context
ਬਣਤਰ xhci_inctx {
    u32 del;
    u32 ਜੋੜੋ;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
ਬਣਤਰ xhci_trb {
    u32 ptr_ਨੀਵਾਂ;
    u32 ptr_ਉੱਚਾ;
    u32 ਸਥਿਤੀ;
    u32 control;
} PACKED;

// event ring segment
ਬਣਤਰ xhci_er_seg {
    u32 ptr_ਨੀਵਾਂ;
    u32 ptr_ਉੱਚਾ;
    u32 ਮਾਪ;
    u32 reserved_01;
} PACKED;

#ਜੇ_ਅੰਤ // usb-xhci.h
