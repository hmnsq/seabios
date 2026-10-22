#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USB_EHCI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USB_EHCI_H

// usb-ehci.c
ਖਾਲੀ ehci_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ ehci_wait_controllers(ਖਾਲੀ);
ਬਣਤਰ usbdevice_s;
ਬਣਤਰ usb_endpoint_descriptor;
ਬਣਤਰ usb_pipe;
ਬਣਤਰ usb_pipe *ehci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev
                                   , ਬਣਤਰ usb_pipe *upipe
                                   , ਬਣਤਰ usb_endpoint_descriptor *epdesc);
ਪੂਰਨ ehci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
                   , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize);
ਪੂਰਨ ehci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ);


/****************************************************************
 * ehci structs and flags
 ****************************************************************/

ਬਣਤਰ ehci_caps {
    u8 caplength;
    u8 reserved_01;
    u16 hciversion;
    u32 hcsparams;
    u32 hccparams;
    u64 portroute;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HCC_64_ਦੋਆਧਾਰੀ_ਅੰਕ_ADDR 1

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HCS_N_PORTS_ਨਕਾਬ 0xf

ਬਣਤਰ ehci_regs {
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_PARK        (1<<11)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_PARK_CNT(c) (((c)>>8)&3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_LRESET      (1<<7)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_IAAD        (1<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_ASE         (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_PSE         (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_HCRESET     (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_ਲੜੀਵਾਰ         (1<<0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_ASS         (1<<15)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_PSS         (1<<14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_RECL        (1<<13)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_HALT        (1<<12)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_IAA         (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_FATAL       (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_FLR         (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_PCD         (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_ERR         (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STS_INT         (1<<0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_CF         (1<<0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_WKOC_E     (1<<22)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_WKDISC_E   (1<<21)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_WKCONN_E   (1<<20)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_TEST_PKT   (0x4<<16)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LED_OFF    (0<<14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LED_AMBER  (1<<14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LED_GREEN  (2<<14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LED_ਨਕਾਬ   (3<<14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_OWNER      (1<<13)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_POWER      (1<<12)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LINESTATUS_ਨਕਾਬ   (3<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LINESTATUS_KSTATE (1<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_RESET      (1<<8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SUSPEND    (1<<7)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_RESUME     (1<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_OCC        (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_OC         (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PEC        (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PE         (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CSC        (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CONNECT    (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_RWC_ਬਿੱਟ   (ਪੋਰਟ_CSC | ਪੋਰਟ_PEC | ਪੋਰਟ_OCC)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

ਬਣਤਰ ehci_qh {
    u32 ਅਗਲਾ;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_ਅਗਲਾ;
    u32 alt_ਅਗਲਾ;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_CONTROL       (1 << 27)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_MAXPACKET_ਖਿਸਕਾਓ 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_MAXPACKET_ਨਕਾਬ  (0x7ff << QH_MAXPACKET_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_ਸਿਰ          (1 << 15)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_TOGGLECONTROL (1 << 14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_ਰਫਤਾਰ_ਖਿਸਕਾਓ   12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_ਰਫਤਾਰ_ਨਕਾਬ    (0x3 << QH_ਰਫਤਾਰ_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_EP_ਖਿਸਕਾਓ      8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_EP_ਨਕਾਬ       (0xf << QH_EP_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_DEVADDR_ਖਿਸਕਾਓ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_DEVADDR_ਨਕਾਬ  (0x7f << QH_DEVADDR_ਖਿਸਕਾਓ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_SMASK_ਖਿਸਕਾਓ   0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_SMASK_ਨਕਾਬ    (0xff << QH_SMASK_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_CMASK_ਖਿਸਕਾਓ   8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_CMASK_ਨਕਾਬ    (0xff << QH_CMASK_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_HUBADDR_ਖਿਸਕਾਓ 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_HUBADDR_ਨਕਾਬ  (0x7f << QH_HUBADDR_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_HUBPORT_ਖਿਸਕਾਓ 23
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_HUBPORT_ਨਕਾਬ  (0x7f << QH_HUBPORT_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_MULT_ਖਿਸਕਾਓ    30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QH_MULT_ਨਕਾਬ     (0x3 << QH_MULT_ਖਿਸਕਾਓ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EHCI_PTR_ਬਿੱਟ           0x001F
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EHCI_PTR_ਪਦ           0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EHCI_PTR_QH             0x0002


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

ਬਣਤਰ ehci_qtd {
    u32 qtd_ਅਗਲਾ;
    u32 alt_ਅਗਲਾ;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_TOGGLE      (1 << 31)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_ਲੰਮਾਈ_ਖਿਸਕਾਓ 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_ਲੰਮਾਈ_ਨਕਾਬ (0x7fff << QTD_ਲੰਮਾਈ_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_CERR_ਖਿਸਕਾਓ  10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_CERR_ਨਕਾਬ   (0x3 << QTD_CERR_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_IOC         (1 << 15)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_PID_ਬਾਹਰਲਾ     (0x0 << 8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_PID_ਵਿੱਚ      (0x1 << 8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_PID_ਤਿਆਰੀ   (0x2 << 8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_ਸਰਗਰਮ  (1 << 7)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_HALT    (1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_DBE     (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_BABBLE  (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_XACT    (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_MMF     (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_STS     (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QTD_STS_PING    (1 << 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ehci_explen(len) (((len) << QTD_ਲੰਮਾਈ_ਖਿਸਕਾਓ) & QTD_ਲੰਮਾਈ_ਨਕਾਬ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ehci_maxerr(err) (((err) << QTD_CERR_ਖਿਸਕਾਓ) & QTD_CERR_ਨਕਾਬ)


ਬਣਤਰ ehci_framelist {
    u32 links[1024];
} PACKED;

#ਜੇ_ਅੰਤ // usb-ehci.h
