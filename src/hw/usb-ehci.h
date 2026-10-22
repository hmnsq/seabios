#اگر_غیر_تعریف_شدہ __USB_EHCI_H
#تعریف_کرو __USB_EHCI_H

// usb-ehci.c
خالی ehci_تیاری(خالی);
خالی ehci_wait_controllers(خالی);
ساخت usbdevice_s;
ساخت usb_endpoint_descriptor;
ساخت usb_pipe;
ساخت usb_pipe *ehci_realloc_pipe(ساخت usbdevice_s *usbdev
                                   , ساخت usb_pipe *upipe
                                   , ساخت usb_endpoint_descriptor *epdesc);
صحیح ehci_send_pipe(ساخت usb_pipe *p, صحیح dir, ثابت خالی *cmd
                   , خالی *ڈیٹا, صحیح datasize);
صحیح ehci_poll_intr(ساخت usb_pipe *p, خالی *ڈیٹا);


/****************************************************************
 * ehci structs and flags
 ****************************************************************/

ساخت ehci_caps {
    u8 caplength;
    u8 reserved_01;
    u16 hciversion;
    u32 hcsparams;
    u32 hccparams;
    u64 portroute;
} PACKED;

#تعریف_کرو HCC_64_ثنائی_ہندسہ_ADDR 1

#تعریف_کرو HCS_N_PORTS_نقاب 0xf

ساخت ehci_regs {
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

#تعریف_کرو CMD_PARK        (1<<11)
#تعریف_کرو CMD_PARK_CNT(c) (((c)>>8)&3)
#تعریف_کرو CMD_LRESET      (1<<7)
#تعریف_کرو CMD_IAAD        (1<<6)
#تعریف_کرو CMD_ASE         (1<<5)
#تعریف_کرو CMD_PSE         (1<<4)
#تعریف_کرو CMD_HCRESET     (1<<1)
#تعریف_کرو CMD_تسلسل         (1<<0)

#تعریف_کرو STS_ASS         (1<<15)
#تعریف_کرو STS_PSS         (1<<14)
#تعریف_کرو STS_RECL        (1<<13)
#تعریف_کرو STS_HALT        (1<<12)
#تعریف_کرو STS_IAA         (1<<5)
#تعریف_کرو STS_FATAL       (1<<4)
#تعریف_کرو STS_FLR         (1<<3)
#تعریف_کرو STS_PCD         (1<<2)
#تعریف_کرو STS_ERR         (1<<1)
#تعریف_کرو STS_INT         (1<<0)

#تعریف_کرو FLAG_CF         (1<<0)

#تعریف_کرو پورٹ_WKOC_E     (1<<22)
#تعریف_کرو پورٹ_WKDISC_E   (1<<21)
#تعریف_کرو پورٹ_WKCONN_E   (1<<20)
#تعریف_کرو پورٹ_TEST_PKT   (0x4<<16)
#تعریف_کرو پورٹ_LED_OFF    (0<<14)
#تعریف_کرو پورٹ_LED_AMBER  (1<<14)
#تعریف_کرو پورٹ_LED_GREEN  (2<<14)
#تعریف_کرو پورٹ_LED_نقاب   (3<<14)
#تعریف_کرو پورٹ_OWNER      (1<<13)
#تعریف_کرو پورٹ_POWER      (1<<12)
#تعریف_کرو پورٹ_LINESTATUS_نقاب   (3<<10)
#تعریف_کرو پورٹ_LINESTATUS_KSTATE (1<<10)
#تعریف_کرو پورٹ_RESET      (1<<8)
#تعریف_کرو پورٹ_SUSPEND    (1<<7)
#تعریف_کرو پورٹ_RESUME     (1<<6)
#تعریف_کرو پورٹ_OCC        (1<<5)
#تعریف_کرو پورٹ_OC         (1<<4)
#تعریف_کرو پورٹ_PEC        (1<<3)
#تعریف_کرو پورٹ_PE         (1<<2)
#تعریف_کرو پورٹ_CSC        (1<<1)
#تعریف_کرو پورٹ_CONNECT    (1<<0)
#تعریف_کرو پورٹ_RWC_بٹ   (پورٹ_CSC | پورٹ_PEC | پورٹ_OCC)


#تعریف_کرو EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

ساخت ehci_qh {
    u32 اگلا;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_اگلا;
    u32 alt_اگلا;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#تعریف_کرو QH_CONTROL       (1 << 27)
#تعریف_کرو QH_MAXPACKET_سرکاؤ 16
#تعریف_کرو QH_MAXPACKET_نقاب  (0x7ff << QH_MAXPACKET_سرکاؤ)
#تعریف_کرو QH_سر          (1 << 15)
#تعریف_کرو QH_TOGGLECONTROL (1 << 14)
#تعریف_کرو QH_رفتار_سرکاؤ   12
#تعریف_کرو QH_رفتار_نقاب    (0x3 << QH_رفتار_سرکاؤ)
#تعریف_کرو QH_EP_سرکاؤ      8
#تعریف_کرو QH_EP_نقاب       (0xf << QH_EP_سرکاؤ)
#تعریف_کرو QH_DEVADDR_سرکاؤ 0
#تعریف_کرو QH_DEVADDR_نقاب  (0x7f << QH_DEVADDR_سرکاؤ)

#تعریف_کرو QH_SMASK_سرکاؤ   0
#تعریف_کرو QH_SMASK_نقاب    (0xff << QH_SMASK_سرکاؤ)
#تعریف_کرو QH_CMASK_سرکاؤ   8
#تعریف_کرو QH_CMASK_نقاب    (0xff << QH_CMASK_سرکاؤ)
#تعریف_کرو QH_HUBADDR_سرکاؤ 16
#تعریف_کرو QH_HUBADDR_نقاب  (0x7f << QH_HUBADDR_سرکاؤ)
#تعریف_کرو QH_HUBPORT_سرکاؤ 23
#تعریف_کرو QH_HUBPORT_نقاب  (0x7f << QH_HUBPORT_سرکاؤ)
#تعریف_کرو QH_MULT_سرکاؤ    30
#تعریف_کرو QH_MULT_نقاب     (0x3 << QH_MULT_سرکاؤ)

#تعریف_کرو EHCI_PTR_بٹ           0x001F
#تعریف_کرو EHCI_PTR_رکن           0x0001
#تعریف_کرو EHCI_PTR_QH             0x0002


#تعریف_کرو EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

ساخت ehci_qtd {
    u32 qtd_اگلا;
    u32 alt_اگلا;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#تعریف_کرو QTD_TOGGLE      (1 << 31)
#تعریف_کرو QTD_لمبائی_سرکاؤ 16
#تعریف_کرو QTD_لمبائی_نقاب (0x7fff << QTD_لمبائی_سرکاؤ)
#تعریف_کرو QTD_CERR_سرکاؤ  10
#تعریف_کرو QTD_CERR_نقاب   (0x3 << QTD_CERR_سرکاؤ)
#تعریف_کرو QTD_IOC         (1 << 15)
#تعریف_کرو QTD_PID_برآمد     (0x0 << 8)
#تعریف_کرو QTD_PID_اندر      (0x1 << 8)
#تعریف_کرو QTD_PID_تیاری   (0x2 << 8)
#تعریف_کرو QTD_STS_فعال  (1 << 7)
#تعریف_کرو QTD_STS_HALT    (1 << 6)
#تعریف_کرو QTD_STS_DBE     (1 << 5)
#تعریف_کرو QTD_STS_BABBLE  (1 << 4)
#تعریف_کرو QTD_STS_XACT    (1 << 3)
#تعریف_کرو QTD_STS_MMF     (1 << 2)
#تعریف_کرو QTD_STS_STS     (1 << 1)
#تعریف_کرو QTD_STS_PING    (1 << 0)

#تعریف_کرو ehci_explen(len) (((len) << QTD_لمبائی_سرکاؤ) & QTD_لمبائی_نقاب)

#تعریف_کرو ehci_maxerr(err) (((err) << QTD_CERR_سرکاؤ) & QTD_CERR_نقاب)


ساخت ehci_framelist {
    u32 links[1024];
} PACKED;

#اگر_ختم // usb-ehci.h
