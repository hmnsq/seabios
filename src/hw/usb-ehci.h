#که_نه_تعریف_شوی __USB_EHCI_H
#تعریف_کړه __USB_EHCI_H

// usb-ehci.c
فراغ ehci_تیاری(فراغ);
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

#تعریف_کړه HCC_64_بټ_ADDR 1

#تعریف_کړه HCS_N_PORTS_ماسک 0xf

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

#تعریف_کړه CMD_PARK        (1<<11)
#تعریف_کړه CMD_PARK_CNT(c) (((c)>>8)&3)
#تعریف_کړه CMD_LRESET      (1<<7)
#تعریف_کړه CMD_IAAD        (1<<6)
#تعریف_کړه CMD_ASE         (1<<5)
#تعریف_کړه CMD_PSE         (1<<4)
#تعریف_کړه CMD_HCRESET     (1<<1)
#تعریف_کړه CMD_پرلپسې         (1<<0)

#تعریف_کړه STS_ASS         (1<<15)
#تعریف_کړه STS_PSS         (1<<14)
#تعریف_کړه STS_RECL        (1<<13)
#تعریف_کړه STS_HALT        (1<<12)
#تعریف_کړه STS_IAA         (1<<5)
#تعریف_کړه STS_FATAL       (1<<4)
#تعریف_کړه STS_FLR         (1<<3)
#تعریف_کړه STS_PCD         (1<<2)
#تعریف_کړه STS_ERR         (1<<1)
#تعریف_کړه STS_INT         (1<<0)

#تعریف_کړه FLAG_CF         (1<<0)

#تعریف_کړه پورټ_WKOC_E     (1<<22)
#تعریف_کړه پورټ_WKDISC_E   (1<<21)
#تعریف_کړه پورټ_WKCONN_E   (1<<20)
#تعریف_کړه پورټ_TEST_PKT   (0x4<<16)
#تعریف_کړه پورټ_LED_OFF    (0<<14)
#تعریف_کړه پورټ_LED_AMBER  (1<<14)
#تعریف_کړه پورټ_LED_GREEN  (2<<14)
#تعریف_کړه پورټ_LED_ماسک   (3<<14)
#تعریف_کړه پورټ_OWNER      (1<<13)
#تعریف_کړه پورټ_POWER      (1<<12)
#تعریف_کړه پورټ_LINESTATUS_ماسک   (3<<10)
#تعریف_کړه پورټ_LINESTATUS_KSTATE (1<<10)
#تعریف_کړه پورټ_RESET      (1<<8)
#تعریف_کړه پورټ_SUSPEND    (1<<7)
#تعریف_کړه پورټ_RESUME     (1<<6)
#تعریف_کړه پورټ_OCC        (1<<5)
#تعریف_کړه پورټ_OC         (1<<4)
#تعریف_کړه پورټ_PEC        (1<<3)
#تعریف_کړه پورټ_PE         (1<<2)
#تعریف_کړه پورټ_CSC        (1<<1)
#تعریف_کړه پورټ_CONNECT    (1<<0)
#تعریف_کړه پورټ_RWC_بټونه   (پورټ_CSC | پورټ_PEC | پورټ_OCC)


#تعریف_کړه EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

بنية ehci_qh {
    u32 بل;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_بل;
    u32 alt_بل;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#تعریف_کړه QH_CONTROL       (1 << 27)
#تعریف_کړه QH_MAXPACKET_ښويدنه 16
#تعریف_کړه QH_MAXPACKET_ماسک  (0x7ff << QH_MAXPACKET_ښويدنه)
#تعریف_کړه QH_سر          (1 << 15)
#تعریف_کړه QH_TOGGLECONTROL (1 << 14)
#تعریف_کړه QH_چټکتیا_ښويدنه   12
#تعریف_کړه QH_چټکتیا_ماسک    (0x3 << QH_چټکتیا_ښويدنه)
#تعریف_کړه QH_EP_ښويدنه      8
#تعریف_کړه QH_EP_ماسک       (0xf << QH_EP_ښويدنه)
#تعریف_کړه QH_DEVADDR_ښويدنه 0
#تعریف_کړه QH_DEVADDR_ماسک  (0x7f << QH_DEVADDR_ښويدنه)

#تعریف_کړه QH_SMASK_ښويدنه   0
#تعریف_کړه QH_SMASK_ماسک    (0xff << QH_SMASK_ښويدنه)
#تعریف_کړه QH_CMASK_ښويدنه   8
#تعریف_کړه QH_CMASK_ماسک    (0xff << QH_CMASK_ښويدنه)
#تعریف_کړه QH_HUBADDR_ښويدنه 16
#تعریف_کړه QH_HUBADDR_ماسک  (0x7f << QH_HUBADDR_ښويدنه)
#تعریف_کړه QH_HUBPORT_ښويدنه 23
#تعریف_کړه QH_HUBPORT_ماسک  (0x7f << QH_HUBPORT_ښويدنه)
#تعریف_کړه QH_MULT_ښويدنه    30
#تعریف_کړه QH_MULT_ماسک     (0x3 << QH_MULT_ښويدنه)

#تعریف_کړه EHCI_PTR_بټونه           0x001F
#تعریف_کړه EHCI_PTR_غړی           0x0001
#تعریف_کړه EHCI_PTR_QH             0x0002


#تعریف_کړه EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

بنية ehci_qtd {
    u32 qtd_بل;
    u32 alt_بل;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#تعریف_کړه QTD_TOGGLE      (1 << 31)
#تعریف_کړه QTD_اوږدتیا_ښويدنه 16
#تعریف_کړه QTD_اوږدتیا_ماسک (0x7fff << QTD_اوږدتیا_ښويدنه)
#تعریف_کړه QTD_CERR_ښويدنه  10
#تعریف_کړه QTD_CERR_ماسک   (0x3 << QTD_CERR_ښويدنه)
#تعریف_کړه QTD_IOC         (1 << 15)
#تعریف_کړه QTD_PID_بهر     (0x0 << 8)
#تعریف_کړه QTD_PID_دننه      (0x1 << 8)
#تعریف_کړه QTD_PID_تیاری   (0x2 << 8)
#تعریف_کړه QTD_STS_فعال  (1 << 7)
#تعریف_کړه QTD_STS_HALT    (1 << 6)
#تعریف_کړه QTD_STS_DBE     (1 << 5)
#تعریف_کړه QTD_STS_BABBLE  (1 << 4)
#تعریف_کړه QTD_STS_XACT    (1 << 3)
#تعریف_کړه QTD_STS_MMF     (1 << 2)
#تعریف_کړه QTD_STS_STS     (1 << 1)
#تعریف_کړه QTD_STS_PING    (1 << 0)

#تعریف_کړه ehci_explen(len) (((len) << QTD_اوږدتیا_ښويدنه) & QTD_اوږدتیا_ماسک)

#تعریف_کړه ehci_maxerr(err) (((err) << QTD_CERR_ښويدنه) & QTD_CERR_ماسک)


بنية ehci_framelist {
    u32 links[1024];
} PACKED;

#د_که_پای // usb-ehci.h
