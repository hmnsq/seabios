#ifndef __USB_XHCI_H
#define __USB_XHCI_H

संरचना usbdevice_s;
संरचना usb_endpoint_descriptor;
संरचना usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
शून्य xhci_setup(शून्य);
संरचना usb_pipe *xhci_realloc_pipe(संरचना usbdevice_s *usbdev
                                   , संरचना usb_pipe *upipe
                                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णांक xhci_send_pipe(संरचना usb_pipe *p, पूर्णांक dir, स्थिरांक शून्य *cmd
                   , शून्य *data, पूर्णांक datasize);
पूर्णांक xhci_poll_intr(संरचना usb_pipe *p, शून्य *data);

// --------------------------------------------------------------
// register interface

// capabilities
संरचना xhci_caps {
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
संरचना xhci_xcap {
    u32 cap;
    u32 data[];
} PACKED;

// operational registers
संरचना xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_खालचा;
    u32 crcr_वरचा;
    u32 reserved_02[4];
    u32 dcbaap_खालचा;
    u32 dcbaap_वरचा;
    u32 config;
} PACKED;

// port registers
संरचना xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
संरचना xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
संरचना xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
संरचना xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_खालचा;
    u32 erstba_वरचा;
    u32 erdp_खालचा;
    u32 erdp_वरचा;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
संरचना xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
संरचना xhci_epctx {
    u32 ctx[2];
    u32 deq_खालचा;
    u32 deq_वरचा;
    u32 length;
    u32 reserved_01[3];
} PACKED;

// device context array element
संरचना xhci_devlist {
    u32 ptr_खालचा;
    u32 ptr_वरचा;
} PACKED;

// input context
संरचना xhci_inctx {
    u32 del;
    u32 add;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
संरचना xhci_trb {
    u32 ptr_खालचा;
    u32 ptr_वरचा;
    u32 status;
    u32 control;
} PACKED;

// event ring segment
संरचना xhci_er_seg {
    u32 ptr_खालचा;
    u32 ptr_वरचा;
    u32 size;
    u32 reserved_01;
} PACKED;

#endif // usb-xhci.h
