#నిర్వచితం_కాకపోతే __USB_XHCI_H
#నిర్వచించు __USB_XHCI_H

నిర్మాణం usbdevice_s;
నిర్మాణం usb_endpoint_descriptor;
నిర్మాణం usb_pipe;

// --------------------------------------------------------------

// usb-xhci.c
శూన్య xhci_అమరిక(శూన్య);
నిర్మాణం usb_pipe *xhci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev
                                   , నిర్మాణం usb_pipe *upipe
                                   , నిర్మాణం usb_endpoint_descriptor *epdesc);
పూర్ణాంకం xhci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
                   , శూన్య *దత్తాంశం, పూర్ణాంకం datasize);
పూర్ణాంకం xhci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం);

// --------------------------------------------------------------
// register interface

// capabilities
నిర్మాణం xhci_caps {
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
నిర్మాణం xhci_xcap {
    u32 cap;
    u32 దత్తాంశం[];
} PACKED;

// operational registers
నిర్మాణం xhci_op {
    u32 usbcmd;
    u32 usbsts;
    u32 pagesize;
    u32 reserved_01[2];
    u32 dnctl;
    u32 crcr_దిగువ;
    u32 crcr_ఎగువ;
    u32 reserved_02[4];
    u32 dcbaap_దిగువ;
    u32 dcbaap_ఎగువ;
    u32 config;
} PACKED;

// port registers
నిర్మాణం xhci_pr {
    u32 portsc;
    u32 portpmsc;
    u32 portli;
    u32 reserved_01;
} PACKED;

// doorbell registers
నిర్మాణం xhci_db {
    u32 doorbell;
} PACKED;

// runtime registers
నిర్మాణం xhci_rts {
    u32 mfindex;
} PACKED;

// interrupter registers
నిర్మాణం xhci_ir {
    u32 iman;
    u32 imod;
    u32 erstsz;
    u32 reserved_01;
    u32 erstba_దిగువ;
    u32 erstba_ఎగువ;
    u32 erdp_దిగువ;
    u32 erdp_ఎగువ;
} PACKED;

// --------------------------------------------------------------
// memory data structs

// slot context
నిర్మాణం xhci_slotctx {
    u32 ctx[4];
    u32 reserved_01[4];
} PACKED;

// endpoint context
నిర్మాణం xhci_epctx {
    u32 ctx[2];
    u32 deq_దిగువ;
    u32 deq_ఎగువ;
    u32 నిడివి;
    u32 reserved_01[3];
} PACKED;

// device context array element
నిర్మాణం xhci_devlist {
    u32 ptr_దిగువ;
    u32 ptr_ఎగువ;
} PACKED;

// input context
నిర్మాణం xhci_inctx {
    u32 del;
    u32 కూడు;
    u32 reserved_01[6];
} PACKED;

// transfer block (ring element)
నిర్మాణం xhci_trb {
    u32 ptr_దిగువ;
    u32 ptr_ఎగువ;
    u32 స్థితి_2;
    u32 control;
} PACKED;

// event ring segment
నిర్మాణం xhci_er_seg {
    u32 ptr_దిగువ;
    u32 ptr_ఎగువ;
    u32 కొలత;
    u32 reserved_01;
} PACKED;

#అయితే_ముగింపు // usb-xhci.h
