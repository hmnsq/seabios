#جيڪڏهن_وصف_نه_ٿيل __USB_EHCI_H
#وصف_ڪريو __USB_EHCI_H

// usb-ehci.c
فراغ ehci_تياري(فراغ);
فراغ ehci_wait_controllers(فراغ);
بنية usbdevice_s;
بنية usb_endpoint_descriptor;
بنية usb_pipe;
بنية usb_pipe *ehci_realloc_pipe(بنية usbdevice_s *usbdev
                                   , بنية usb_pipe *upipe
                                   , بنية usb_endpoint_descriptor *epdesc);
صحيح ehci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
                   , فراغ *معلومات, صحيح datasize);
صحيح ehci_poll_intr(بنية usb_pipe *p, فراغ *معلومات);


/****************************************************************
 * ehci structs and flags
 ****************************************************************/

بنية ehci_caps {
    u8 caplength;
    u8 reserved_01;
    u16 hciversion;
    u32 hcsparams;
    u32 hccparams;
    u64 portroute;
} PACKED;

#وصف_ڪريو HCC_64_ٻٽو_انگ_ADDR 1

#وصف_ڪريو HCS_N_PORTS_نقاب 0xf

بنية ehci_regs {
    u32 usbcmd;
    u32 usbsts;
    u32 usbintr;
    u32 frindex;
    u32 ctrldssegment;
    u32 periodiclistbase;
    u32 asynclistbase;
    u32 reserved[9];
    u32 configflag;
    u32 portsc[0];
} PACKED;

#وصف_ڪريو CMD_PARK        (1<<11)
#وصف_ڪريو CMD_PARK_CNT(c) (((c)>>8)&3)
#وصف_ڪريو CMD_LRESET      (1<<7)
#وصف_ڪريو CMD_IAAD        (1<<6)
#وصف_ڪريو CMD_ASE         (1<<5)
#وصف_ڪريو CMD_PSE         (1<<4)
#وصف_ڪريو CMD_HCRESET     (1<<1)
#وصف_ڪريو CMD_لاڳيتو         (1<<0)

#وصف_ڪريو STS_ASS         (1<<15)
#وصف_ڪريو STS_PSS         (1<<14)
#وصف_ڪريو STS_RECL        (1<<13)
#وصف_ڪريو STS_HALT        (1<<12)
#وصف_ڪريو STS_IAA         (1<<5)
#وصف_ڪريو STS_FATAL       (1<<4)
#وصف_ڪريو STS_FLR         (1<<3)
#وصف_ڪريو STS_PCD         (1<<2)
#وصف_ڪريو STS_ERR         (1<<1)
#وصف_ڪريو STS_INT         (1<<0)

#وصف_ڪريو FLAG_CF         (1<<0)

#وصف_ڪريو پورٽ_WKOC_E     (1<<22)
#وصف_ڪريو پورٽ_WKDISC_E   (1<<21)
#وصف_ڪريو پورٽ_WKCONN_E   (1<<20)
#وصف_ڪريو پورٽ_TEST_PKT   (0x4<<16)
#وصف_ڪريو پورٽ_LED_OFF    (0<<14)
#وصف_ڪريو پورٽ_LED_AMBER  (1<<14)
#وصف_ڪريو پورٽ_LED_GREEN  (2<<14)
#وصف_ڪريو پورٽ_LED_نقاب   (3<<14)
#وصف_ڪريو پورٽ_OWNER      (1<<13)
#وصف_ڪريو پورٽ_POWER      (1<<12)
#وصف_ڪريو پورٽ_LINESTATUS_نقاب   (3<<10)
#وصف_ڪريو پورٽ_LINESTATUS_KSTATE (1<<10)
#وصف_ڪريو پورٽ_RESET      (1<<8)
#وصف_ڪريو پورٽ_SUSPEND    (1<<7)
#وصف_ڪريو پورٽ_RESUME     (1<<6)
#وصف_ڪريو پورٽ_OCC        (1<<5)
#وصف_ڪريو پورٽ_OC         (1<<4)
#وصف_ڪريو پورٽ_PEC        (1<<3)
#وصف_ڪريو پورٽ_PE         (1<<2)
#وصف_ڪريو پورٽ_CSC        (1<<1)
#وصف_ڪريو پورٽ_CONNECT    (1<<0)
#وصف_ڪريو پورٽ_RWC_بٽ   (پورٽ_CSC | پورٽ_PEC | پورٽ_OCC)


#وصف_ڪريو EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

بنية ehci_qh {
    u32 ايندڙ;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_ايندڙ;
    u32 alt_ايندڙ;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#وصف_ڪريو QH_CONTROL       (1 << 27)
#وصف_ڪريو QH_MAXPACKET_سرڪايو 16
#وصف_ڪريو QH_MAXPACKET_نقاب  (0x7ff << QH_MAXPACKET_سرڪايو)
#وصف_ڪريو QH_سر          (1 << 15)
#وصف_ڪريو QH_TOGGLECONTROL (1 << 14)
#وصف_ڪريو QH_رفتار_سرڪايو   12
#وصف_ڪريو QH_رفتار_نقاب    (0x3 << QH_رفتار_سرڪايو)
#وصف_ڪريو QH_EP_سرڪايو      8
#وصف_ڪريو QH_EP_نقاب       (0xf << QH_EP_سرڪايو)
#وصف_ڪريو QH_DEVADDR_سرڪايو 0
#وصف_ڪريو QH_DEVADDR_نقاب  (0x7f << QH_DEVADDR_سرڪايو)

#وصف_ڪريو QH_SMASK_سرڪايو   0
#وصف_ڪريو QH_SMASK_نقاب    (0xff << QH_SMASK_سرڪايو)
#وصف_ڪريو QH_CMASK_سرڪايو   8
#وصف_ڪريو QH_CMASK_نقاب    (0xff << QH_CMASK_سرڪايو)
#وصف_ڪريو QH_HUBADDR_سرڪايو 16
#وصف_ڪريو QH_HUBADDR_نقاب  (0x7f << QH_HUBADDR_سرڪايو)
#وصف_ڪريو QH_HUBPORT_سرڪايو 23
#وصف_ڪريو QH_HUBPORT_نقاب  (0x7f << QH_HUBPORT_سرڪايو)
#وصف_ڪريو QH_MULT_سرڪايو    30
#وصف_ڪريو QH_MULT_نقاب     (0x3 << QH_MULT_سرڪايو)

#وصف_ڪريو EHCI_PTR_بٽ           0x001F
#وصف_ڪريو EHCI_PTR_رڪن           0x0001
#وصف_ڪريو EHCI_PTR_QH             0x0002


#وصف_ڪريو EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

بنية ehci_qtd {
    u32 qtd_ايندڙ;
    u32 alt_ايندڙ;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#وصف_ڪريو QTD_TOGGLE      (1 << 31)
#وصف_ڪريو QTD_ڊيگهائي_سرڪايو 16
#وصف_ڪريو QTD_ڊيگهائي_نقاب (0x7fff << QTD_ڊيگهائي_سرڪايو)
#وصف_ڪريو QTD_CERR_سرڪايو  10
#وصف_ڪريو QTD_CERR_نقاب   (0x3 << QTD_CERR_سرڪايو)
#وصف_ڪريو QTD_IOC         (1 << 15)
#وصف_ڪريو QTD_PID_ٻاهريون     (0x0 << 8)
#وصف_ڪريو QTD_PID_اندريون      (0x1 << 8)
#وصف_ڪريو QTD_PID_تياري   (0x2 << 8)
#وصف_ڪريو QTD_STS_سرگرم  (1 << 7)
#وصف_ڪريو QTD_STS_HALT    (1 << 6)
#وصف_ڪريو QTD_STS_DBE     (1 << 5)
#وصف_ڪريو QTD_STS_BABBLE  (1 << 4)
#وصف_ڪريو QTD_STS_XACT    (1 << 3)
#وصف_ڪريو QTD_STS_MMF     (1 << 2)
#وصف_ڪريو QTD_STS_STS     (1 << 1)
#وصف_ڪريو QTD_STS_PING    (1 << 0)

#وصف_ڪريو ehci_explen(len) (((len) << QTD_ڊيگهائي_سرڪايو) & QTD_ڊيگهائي_نقاب)

#وصف_ڪريو ehci_maxerr(err) (((err) << QTD_CERR_سرڪايو) & QTD_CERR_نقاب)


بنية ehci_framelist {
    u32 links[1024];
} PACKED;

#جيڪڏهن_پورو // usb-ehci.h
