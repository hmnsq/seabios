#جے_بنا_تعریف __USB_XHCI_H
#تعریف_کرو __USB_XHCI_H

بنية usbdevice_s;
بنية usb_endpoint_descriptor;
بنية usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
فراغ xhci_تیاری(فراغ);
بنية usb_pipe *xhci_realloc_pipe(بنية usbdevice_s *usbdev
                                   , بنية usb_pipe *upipe
                                   , بنية usb_endpoint_descriptor *epdesc);
صحيح xhci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
                   , فراغ *انکڑے, صحيح datasize);
صحيح xhci_poll_intr(بنية usb_pipe *p, فراغ *انکڑے);

// --------------------------------------------------------------
// register interface

// capabilities
بنية xhci_caps {
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
بنية xhci_xcap {
    u32 cap;
    u32 انکڑے[];
} PACKED;

// operational registers
بنية xhci_op {
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
بنية xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
بنية xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
بنية xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
بنية xhci_ir {
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
بنية xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
بنية xhci_epctx {
    u32 ctx[2];
    u32 deq_کم;
    u32 deq_بلند;
    u32 لمائی;
    u32 reserved_01[3];
} PACKED;

// device context array element
بنية xhci_devlist {
    u32 ptr_کم;
    u32 ptr_بلند;
} PACKED;

// input context
بنية xhci_inctx {
    u32 del;
    u32 جوڑو;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
بنية xhci_trb {
    u32 ptr_کم;
    u32 ptr_بلند;
    u32 حالت;
    u32 control;
} PACKED;

// event ring segment
بنية xhci_er_seg {
    u32 ptr_کم;
    u32 ptr_بلند;
    u32 ناپ;
    u32 reserved_01;
} PACKED;

#جے_انت // usb-xhci.h
