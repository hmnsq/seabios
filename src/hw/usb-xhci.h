#ifndef __USB_XHCI_H
#define __USB_XHCI_H

কাঠামো usbdevice_s;
কাঠামো usb_endpoint_descriptor;
কাঠামো usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
শূন্য xhci_setup(শূন্য);
কাঠামো usb_pipe *xhci_realloc_pipe(কাঠামো usbdevice_s *usbdev
                                   , কাঠামো usb_pipe *upipe
                                   , কাঠামো usb_endpoint_descriptor *epdesc);
পূর্ণসংখ্যা xhci_send_pipe(কাঠামো usb_pipe *p, পূর্ণসংখ্যা dir, ধ্রুবক শূন্য *cmd
                   , শূন্য *তথ্য, পূর্ণসংখ্যা datasize);
পূর্ণসংখ্যা xhci_poll_intr(কাঠামো usb_pipe *p, শূন্য *তথ্য);

// --------------------------------------------------------------
// register interface

// capabilities
কাঠামো xhci_caps {
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
কাঠামো xhci_xcap {
    u32 cap;
    u32 তথ্য[];
} PACKED;

// operational registers
কাঠামো xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_নিচের;
    u32 crcr_উপরের;
    u32 reserved_02[4];
    u32 dcbaap_নিচের;
    u32 dcbaap_উপরের;
    u32 config;
} PACKED;

// port registers
কাঠামো xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
কাঠামো xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
কাঠামো xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
কাঠামো xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_নিচের;
    u32 erstba_উপরের;
    u32 erdp_নিচের;
    u32 erdp_উপরের;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
কাঠামো xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
কাঠামো xhci_epctx {
    u32 ctx[2];
    u32 deq_নিচের;
    u32 deq_উপরের;
    u32 দৈর্ঘ্য;
    u32 reserved_01[3];
} PACKED;

// device context array element
কাঠামো xhci_devlist {
    u32 ptr_নিচের;
    u32 ptr_উপরের;
} PACKED;

// input context
কাঠামো xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
কাঠামো xhci_trb {
    u32 ptr_নিচের;
    u32 ptr_উপরের;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
কাঠামো xhci_er_seg {
    u32 ptr_নিচের;
    u32 ptr_উপরের;
    u32 আকার_2;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
