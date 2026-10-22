#جے_بنا_تعریف __USB_EHCI_H
#تعریف_کرو __USB_EHCI_H

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
                   , فراغ *انکڑے, صحيح datasize);
صحيح ehci_poll_intr(بنية usb_pipe *p, فراغ *انکڑے);


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

#تعریف_کرو HCC_64_ثنائی_ہندسہ_ADDR 1

#تعریف_کرو HCS_N_PORTS_نقاب 0xf

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

#تعریف_کرو CMD_PARK        (1<<11)
#تعریف_کرو CMD_PARK_CNT(c) (((c)>>8)&3)
#تعریف_کرو CMD_LRESET      (1<<7)
#تعریف_کرو CMD_IAAD        (1<<6)
#تعریف_کرو CMD_ASE         (1<<5)
#تعریف_کرو CMD_PSE         (1<<4)
#تعریف_کرو CMD_HCRESET     (1<<1)
#تعریف_کرو CMD_لگاتار         (1<<0)

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

بنية ehci_qh {
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
#تعریف_کرو QH_MAXPACKET_کھسکاؤ 16
#تعریف_کرو QH_MAXPACKET_نقاب  (0x7ff << QH_MAXPACKET_کھسکاؤ)
#تعریف_کرو QH_سر          (1 << 15)
#تعریف_کرو QH_TOGGLECONTROL (1 << 14)
#تعریف_کرو QH_رفتار_کھسکاؤ   12
#تعریف_کرو QH_رفتار_نقاب    (0x3 << QH_رفتار_کھسکاؤ)
#تعریف_کرو QH_EP_کھسکاؤ      8
#تعریف_کرو QH_EP_نقاب       (0xf << QH_EP_کھسکاؤ)
#تعریف_کرو QH_DEVADDR_کھسکاؤ 0
#تعریف_کرو QH_DEVADDR_نقاب  (0x7f << QH_DEVADDR_کھسکاؤ)

#تعریف_کرو QH_SMASK_کھسکاؤ   0
#تعریف_کرو QH_SMASK_نقاب    (0xff << QH_SMASK_کھسکاؤ)
#تعریف_کرو QH_CMASK_کھسکاؤ   8
#تعریف_کرو QH_CMASK_نقاب    (0xff << QH_CMASK_کھسکاؤ)
#تعریف_کرو QH_HUBADDR_کھسکاؤ 16
#تعریف_کرو QH_HUBADDR_نقاب  (0x7f << QH_HUBADDR_کھسکاؤ)
#تعریف_کرو QH_HUBPORT_کھسکاؤ 23
#تعریف_کرو QH_HUBPORT_نقاب  (0x7f << QH_HUBPORT_کھسکاؤ)
#تعریف_کرو QH_MULT_کھسکاؤ    30
#تعریف_کرو QH_MULT_نقاب     (0x3 << QH_MULT_کھسکاؤ)

#تعریف_کرو EHCI_PTR_بٹ           0x001F
#تعریف_کرو EHCI_PTR_رکن           0x0001
#تعریف_کرو EHCI_PTR_QH             0x0002


#تعریف_کرو EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

بنية ehci_qtd {
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
#تعریف_کرو QTD_لمائی_کھسکاؤ 16
#تعریف_کرو QTD_لمائی_نقاب (0x7fff << QTD_لمائی_کھسکاؤ)
#تعریف_کرو QTD_CERR_کھسکاؤ  10
#تعریف_کرو QTD_CERR_نقاب   (0x3 << QTD_CERR_کھسکاؤ)
#تعریف_کرو QTD_IOC         (1 << 15)
#تعریف_کرو QTD_PID_باہرلا     (0x0 << 8)
#تعریف_کرو QTD_PID_وچ      (0x1 << 8)
#تعریف_کرو QTD_PID_تیاری   (0x2 << 8)
#تعریف_کرو QTD_STS_سرگرم  (1 << 7)
#تعریف_کرو QTD_STS_HALT    (1 << 6)
#تعریف_کرو QTD_STS_DBE     (1 << 5)
#تعریف_کرو QTD_STS_BABBLE  (1 << 4)
#تعریف_کرو QTD_STS_XACT    (1 << 3)
#تعریف_کرو QTD_STS_MMF     (1 << 2)
#تعریف_کرو QTD_STS_STS     (1 << 1)
#تعریف_کرو QTD_STS_PING    (1 << 0)

#تعریف_کرو ehci_explen(len) (((len) << QTD_لمائی_کھسکاؤ) & QTD_لمائی_نقاب)

#تعریف_کرو ehci_maxerr(err) (((err) << QTD_CERR_کھسکاؤ) & QTD_CERR_نقاب)


بنية ehci_framelist {
    u32 links[1024];
} PACKED;

#جے_انت // usb-ehci.h
