#اگر_تعریف_نشده __USB_EHCI_H
#تعریف_کن __USB_EHCI_H

// usb-ehci.c
فراغ ehci_راهاندازی(فراغ);
فراغ ehci_wait_controllers(فراغ);
بنية usbdevice_s;
بنية usb_endpoint_descriptor;
بنية usb_pipe;
بنية usb_pipe *ehci_realloc_pipe(بنية usbdevice_s *usbdev
                                   , بنية usb_pipe *upipe
                                   , بنية usb_endpoint_descriptor *epdesc);
صحيح ehci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
                   , فراغ *داده, صحيح datasize);
صحيح ehci_poll_intr(بنية usb_pipe *p, فراغ *داده);


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

#تعریف_کن HCC_64_بیت_ADDR 1

#تعریف_کن HCS_N_PORTS_ماسک 0xf

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

#تعریف_کن CMD_PARK        (1<<11)
#تعریف_کن CMD_PARK_CNT(c) (((c)>>8)&3)
#تعریف_کن CMD_LRESET      (1<<7)
#تعریف_کن CMD_IAAD        (1<<6)
#تعریف_کن CMD_ASE         (1<<5)
#تعریف_کن CMD_PSE         (1<<4)
#تعریف_کن CMD_HCRESET     (1<<1)
#تعریف_کن CMD_پیاپی         (1<<0)

#تعریف_کن STS_ASS         (1<<15)
#تعریف_کن STS_PSS         (1<<14)
#تعریف_کن STS_RECL        (1<<13)
#تعریف_کن STS_HALT        (1<<12)
#تعریف_کن STS_IAA         (1<<5)
#تعریف_کن STS_FATAL       (1<<4)
#تعریف_کن STS_FLR         (1<<3)
#تعریف_کن STS_PCD         (1<<2)
#تعریف_کن STS_ERR         (1<<1)
#تعریف_کن STS_INT         (1<<0)

#تعریف_کن FLAG_CF         (1<<0)

#تعریف_کن پورت_WKOC_E     (1<<22)
#تعریف_کن پورت_WKDISC_E   (1<<21)
#تعریف_کن پورت_WKCONN_E   (1<<20)
#تعریف_کن پورت_TEST_PKT   (0x4<<16)
#تعریف_کن پورت_LED_OFF    (0<<14)
#تعریف_کن پورت_LED_AMBER  (1<<14)
#تعریف_کن پورت_LED_GREEN  (2<<14)
#تعریف_کن پورت_LED_ماسک   (3<<14)
#تعریف_کن پورت_OWNER      (1<<13)
#تعریف_کن پورت_POWER      (1<<12)
#تعریف_کن پورت_LINESTATUS_ماسک   (3<<10)
#تعریف_کن پورت_LINESTATUS_KSTATE (1<<10)
#تعریف_کن پورت_RESET      (1<<8)
#تعریف_کن پورت_SUSPEND    (1<<7)
#تعریف_کن پورت_RESUME     (1<<6)
#تعریف_کن پورت_OCC        (1<<5)
#تعریف_کن پورت_OC         (1<<4)
#تعریف_کن پورت_PEC        (1<<3)
#تعریف_کن پورت_PE         (1<<2)
#تعریف_کن پورت_CSC        (1<<1)
#تعریف_کن پورت_CONNECT    (1<<0)
#تعریف_کن پورت_RWC_بیتها   (پورت_CSC | پورت_PEC | پورت_OCC)


#تعریف_کن EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

بنية ehci_qh {
    u32 بعدی;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_بعدی;
    u32 alt_بعدی;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#تعریف_کن QH_CONTROL       (1 << 27)
#تعریف_کن QH_MAXPACKET_لغزش 16
#تعریف_کن QH_MAXPACKET_ماسک  (0x7ff << QH_MAXPACKET_لغزش)
#تعریف_کن QH_سر          (1 << 15)
#تعریف_کن QH_TOGGLECONTROL (1 << 14)
#تعریف_کن QH_سرعت_لغزش   12
#تعریف_کن QH_سرعت_ماسک    (0x3 << QH_سرعت_لغزش)
#تعریف_کن QH_EP_لغزش      8
#تعریف_کن QH_EP_ماسک       (0xf << QH_EP_لغزش)
#تعریف_کن QH_DEVADDR_لغزش 0
#تعریف_کن QH_DEVADDR_ماسک  (0x7f << QH_DEVADDR_لغزش)

#تعریف_کن QH_SMASK_لغزش   0
#تعریف_کن QH_SMASK_ماسک    (0xff << QH_SMASK_لغزش)
#تعریف_کن QH_CMASK_لغزش   8
#تعریف_کن QH_CMASK_ماسک    (0xff << QH_CMASK_لغزش)
#تعریف_کن QH_HUBADDR_لغزش 16
#تعریف_کن QH_HUBADDR_ماسک  (0x7f << QH_HUBADDR_لغزش)
#تعریف_کن QH_HUBPORT_لغزش 23
#تعریف_کن QH_HUBPORT_ماسک  (0x7f << QH_HUBPORT_لغزش)
#تعریف_کن QH_MULT_لغزش    30
#تعریف_کن QH_MULT_ماسک     (0x3 << QH_MULT_لغزش)

#تعریف_کن EHCI_PTR_بیتها           0x001F
#تعریف_کن EHCI_PTR_جمله           0x0001
#تعریف_کن EHCI_PTR_QH             0x0002


#تعریف_کن EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

بنية ehci_qtd {
    u32 qtd_بعدی;
    u32 alt_بعدی;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#تعریف_کن QTD_TOGGLE      (1 << 31)
#تعریف_کن QTD_درازی_لغزش 16
#تعریف_کن QTD_درازی_ماسک (0x7fff << QTD_درازی_لغزش)
#تعریف_کن QTD_CERR_لغزش  10
#تعریف_کن QTD_CERR_ماسک   (0x3 << QTD_CERR_لغزش)
#تعریف_کن QTD_IOC         (1 << 15)
#تعریف_کن QTD_PID_بیرون     (0x0 << 8)
#تعریف_کن QTD_PID_درون      (0x1 << 8)
#تعریف_کن QTD_PID_راهاندازی   (0x2 << 8)
#تعریف_کن QTD_STS_فعال  (1 << 7)
#تعریف_کن QTD_STS_HALT    (1 << 6)
#تعریف_کن QTD_STS_DBE     (1 << 5)
#تعریف_کن QTD_STS_BABBLE  (1 << 4)
#تعریف_کن QTD_STS_XACT    (1 << 3)
#تعریف_کن QTD_STS_MMF     (1 << 2)
#تعریف_کن QTD_STS_STS     (1 << 1)
#تعریف_کن QTD_STS_PING    (1 << 0)

#تعریف_کن ehci_explen(len) (((len) << QTD_درازی_لغزش) & QTD_درازی_ماسک)

#تعریف_کن ehci_maxerr(err) (((err) << QTD_CERR_لغزش) & QTD_CERR_ماسک)


بنية ehci_framelist {
    u32 links[1024];
} PACKED;

#پایان_اگر // usb-ehci.h
