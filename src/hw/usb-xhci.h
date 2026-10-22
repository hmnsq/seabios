#अगर_अघोषित __USB_XHCI_H
#घोषणा __USB_XHCI_H

काष्ठा usbdevice_s;
काष्ठा usb_endpoint_descriptor;
काष्ठा usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
व्योम xhci_setup(व्योम);
काष्ठा usb_pipe *xhci_realloc_pipe(काष्ठा usbdevice_s *usbdev
                                   , काष्ठा usb_pipe *upipe
                                   , काष्ठा usb_endpoint_descriptor *epdesc);
पूर्णांक xhci_send_pipe(काष्ठा usb_pipe *p, पूर्णांक dir, स्थिर व्योम *cmd
                   , व्योम *तथ्य, पूर्णांक datasize);
पूर्णांक xhci_poll_intr(काष्ठा usb_pipe *p, व्योम *तथ्य);

// --------------------------------------------------------------
// register interface

// capabilities
काष्ठा xhci_caps {
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
काष्ठा xhci_xcap {
    u32 cap;
    u32 तथ्य[];
} PACKED;

// operational registers
काष्ठा xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_निचला;
    u32 crcr_ऊपरी;
    u32 reserved_02[4];
    u32 dcbaap_निचला;
    u32 dcbaap_ऊपरी;
    u32 config;
} PACKED;

// port registers
काष्ठा xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
काष्ठा xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
काष्ठा xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
काष्ठा xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_निचला;
    u32 erstba_ऊपरी;
    u32 erdp_निचला;
    u32 erdp_ऊपरी;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
काष्ठा xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
काष्ठा xhci_epctx {
    u32 ctx[2];
    u32 deq_निचला;
    u32 deq_ऊपरी;
    u32 लंबाई;
    u32 reserved_01[3];
} PACKED;

// device context array element
काष्ठा xhci_devlist {
    u32 ptr_निचला;
    u32 ptr_ऊपरी;
} PACKED;

// input context
काष्ठा xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
काष्ठा xhci_trb {
    u32 ptr_निचला;
    u32 ptr_ऊपरी;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
काष्ठा xhci_er_seg {
    u32 ptr_निचला;
    u32 ptr_ऊपरी;
    u32 आकार;
    u32 reserved_01;
} PACKED;

#पूर्ण_अगर // usb-xhci.h
