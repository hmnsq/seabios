#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USB_EHCI_H
#ವ್ಯಾಖ್ಯಾನಿಸು __USB_EHCI_H

// usb-ehci.c
ಶೂನ್ಯ ehci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ ehci_wait_controllers(ಶೂನ್ಯ);
ರಚನೆ usbdevice_s;
ರಚನೆ usb_endpoint_descriptor;
ರಚನೆ usb_pipe;
ರಚನೆ usb_pipe *ehci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev
                                   , ರಚನೆ usb_pipe *upipe
                                   , ರಚನೆ usb_endpoint_descriptor *epdesc);
ಪೂರ್ಣಾಂಕ ehci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
                   , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize);
ಪೂರ್ಣಾಂಕ ehci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ);


/****************************************************************
 * ehci structs and flags
 ****************************************************************/

ರಚನೆ ehci_caps {
    u8 caplength;
    u8 reserved_01;
    u16 hciversion;
    u32 hcsparams;
    u32 hccparams;
    u64 portroute;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು HCC_64_ಬಿಟ್_ADDR 1

#ವ್ಯಾಖ್ಯಾನಿಸು HCS_N_PORTS_ಮುಖವಾಡ 0xf

ರಚನೆ ehci_regs {
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

#ವ್ಯಾಖ್ಯಾನಿಸು CMD_PARK        (1<<11)
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_PARK_CNT(c) (((c)>>8)&3)
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_LRESET      (1<<7)
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_IAAD        (1<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_ASE         (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_PSE         (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_HCRESET     (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_ಸತತ         (1<<0)

#ವ್ಯಾಖ್ಯಾನಿಸು STS_ASS         (1<<15)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_PSS         (1<<14)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_RECL        (1<<13)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_HALT        (1<<12)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_IAA         (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_FATAL       (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_FLR         (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_PCD         (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_ERR         (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು STS_INT         (1<<0)

#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_CF         (1<<0)

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_WKOC_E     (1<<22)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_WKDISC_E   (1<<21)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_WKCONN_E   (1<<20)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_TEST_PKT   (0x4<<16)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LED_OFF    (0<<14)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LED_AMBER  (1<<14)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LED_GREEN  (2<<14)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LED_ಮುಖವಾಡ   (3<<14)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_OWNER      (1<<13)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_POWER      (1<<12)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LINESTATUS_ಮುಖವಾಡ   (3<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LINESTATUS_KSTATE (1<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_RESET      (1<<8)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SUSPEND    (1<<7)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_RESUME     (1<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_OCC        (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_OC         (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PEC        (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PE         (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CSC        (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CONNECT    (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_RWC_ಬಿಟ್ಟುಗಳು   (ಪೋರ್ಟ್_CSC | ಪೋರ್ಟ್_PEC | ಪೋರ್ಟ್_OCC)


#ವ್ಯಾಖ್ಯಾನಿಸು EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

ರಚನೆ ehci_qh {
    u32 ಮುಂದಿನ;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_ಮುಂದಿನ;
    u32 alt_ಮುಂದಿನ;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು QH_CONTROL       (1 << 27)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_MAXPACKET_ಸರಿಸು 16
#ವ್ಯಾಖ್ಯಾನಿಸು QH_MAXPACKET_ಮುಖವಾಡ  (0x7ff << QH_MAXPACKET_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_ತಲೆ          (1 << 15)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_TOGGLECONTROL (1 << 14)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_ವೇಗ_ಸರಿಸು   12
#ವ್ಯಾಖ್ಯಾನಿಸು QH_ವೇಗ_ಮುಖವಾಡ    (0x3 << QH_ವೇಗ_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_EP_ಸರಿಸು      8
#ವ್ಯಾಖ್ಯಾನಿಸು QH_EP_ಮುಖವಾಡ       (0xf << QH_EP_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_DEVADDR_ಸರಿಸು 0
#ವ್ಯಾಖ್ಯಾನಿಸು QH_DEVADDR_ಮುಖವಾಡ  (0x7f << QH_DEVADDR_ಸರಿಸು)

#ವ್ಯಾಖ್ಯಾನಿಸು QH_SMASK_ಸರಿಸು   0
#ವ್ಯಾಖ್ಯಾನಿಸು QH_SMASK_ಮುಖವಾಡ    (0xff << QH_SMASK_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_CMASK_ಸರಿಸು   8
#ವ್ಯಾಖ್ಯಾನಿಸು QH_CMASK_ಮುಖವಾಡ    (0xff << QH_CMASK_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_HUBADDR_ಸರಿಸು 16
#ವ್ಯಾಖ್ಯಾನಿಸು QH_HUBADDR_ಮುಖವಾಡ  (0x7f << QH_HUBADDR_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_HUBPORT_ಸರಿಸು 23
#ವ್ಯಾಖ್ಯಾನಿಸು QH_HUBPORT_ಮುಖವಾಡ  (0x7f << QH_HUBPORT_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QH_MULT_ಸರಿಸು    30
#ವ್ಯಾಖ್ಯಾನಿಸು QH_MULT_ಮುಖವಾಡ     (0x3 << QH_MULT_ಸರಿಸು)

#ವ್ಯಾಖ್ಯಾನಿಸು EHCI_PTR_ಬಿಟ್ಟುಗಳು           0x001F
#ವ್ಯಾಖ್ಯಾನಿಸು EHCI_PTR_ಪದ           0x0001
#ವ್ಯಾಖ್ಯಾನಿಸು EHCI_PTR_QH             0x0002


#ವ್ಯಾಖ್ಯಾನಿಸು EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

ರಚನೆ ehci_qtd {
    u32 qtd_ಮುಂದಿನ;
    u32 alt_ಮುಂದಿನ;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#ವ್ಯಾಖ್ಯಾನಿಸು QTD_TOGGLE      (1 << 31)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_ಉದ್ದಳತೆ_ಸರಿಸು 16
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_ಉದ್ದಳತೆ_ಮುಖವಾಡ (0x7fff << QTD_ಉದ್ದಳತೆ_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_CERR_ಸರಿಸು  10
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_CERR_ಮುಖವಾಡ   (0x3 << QTD_CERR_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_IOC         (1 << 15)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_PID_ಹೊರಹೋಗು     (0x0 << 8)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_PID_ಒಳ      (0x1 << 8)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_PID_ಸಿದ್ಧತೆ   (0x2 << 8)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_ಸಕ್ರಿಯ  (1 << 7)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_HALT    (1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_DBE     (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_BABBLE  (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_XACT    (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_MMF     (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_STS     (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು QTD_STS_PING    (1 << 0)

#ವ್ಯಾಖ್ಯಾನಿಸು ehci_explen(len) (((len) << QTD_ಉದ್ದಳತೆ_ಸರಿಸು) & QTD_ಉದ್ದಳತೆ_ಮುಖವಾಡ)

#ವ್ಯಾಖ್ಯಾನಿಸು ehci_maxerr(err) (((err) << QTD_CERR_ಸರಿಸು) & QTD_CERR_ಮುಖವಾಡ)


ರಚನೆ ehci_framelist {
    u32 links[1024];
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // usb-ehci.h
