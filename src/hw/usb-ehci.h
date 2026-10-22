#यदि_अपरिभाषितम् __USB_EHCI_H
#परिभाषय __USB_EHCI_H

// usb-ehci.c
शून्य ehci_सज्जता(शून्य);
शून्य ehci_wait_controllers(शून्य);
संरचना usbdevice_s;
संरचना usb_endpoint_descriptor;
संरचना usb_pipe;
संरचना usb_pipe *ehci_realloc_pipe(संरचना usbdevice_s *usbdev
                                   , संरचना usb_pipe *upipe
                                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णाङ्क ehci_send_pipe(संरचना usb_pipe *p, पूर्णाङ्क dir, ध्रुव शून्य *cmd
                   , शून्य *दत्त, पूर्णाङ्क datasize);
पूर्णाङ्क ehci_poll_intr(संरचना usb_pipe *p, शून्य *दत्त);


/****************************************************************
 * ehci structs and flags
 ****************************************************************/

संरचना ehci_caps {
    u8 caplength;
    u8 reserved_01;
    u16 hciversion;
    u32 hcsparams;
    u32 hccparams;
    u64 portroute;
} PACKED;

#परिभाषय HCC_64BIT_ADDR 1

#परिभाषय HCS_N_PORTS_आवरणम् 0xf

संरचना ehci_regs {
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

#परिभाषय CMD_PARK        (1<<11)
#परिभाषय CMD_PARK_CNT(c) (((c)>>8)&3)
#परिभाषय CMD_LRESET      (1<<7)
#परिभाषय CMD_IAAD        (1<<6)
#परिभाषय CMD_ASE         (1<<5)
#परिभाषय CMD_PSE         (1<<4)
#परिभाषय CMD_HCRESET     (1<<1)
#परिभाषय CMD_सततम्         (1<<0)

#परिभाषय STS_ASS         (1<<15)
#परिभाषय STS_PSS         (1<<14)
#परिभाषय STS_RECL        (1<<13)
#परिभाषय STS_HALT        (1<<12)
#परिभाषय STS_IAA         (1<<5)
#परिभाषय STS_FATAL       (1<<4)
#परिभाषय STS_FLR         (1<<3)
#परिभाषय STS_PCD         (1<<2)
#परिभाषय STS_ERR         (1<<1)
#परिभाषय STS_INT         (1<<0)

#परिभाषय FLAG_CF         (1<<0)

#परिभाषय पत्तनम्_WKOC_E     (1<<22)
#परिभाषय पत्तनम्_WKDISC_E   (1<<21)
#परिभाषय पत्तनम्_WKCONN_E   (1<<20)
#परिभाषय पत्तनम्_TEST_PKT   (0x4<<16)
#परिभाषय पत्तनम्_LED_OFF    (0<<14)
#परिभाषय पत्तनम्_LED_AMBER  (1<<14)
#परिभाषय पत्तनम्_LED_GREEN  (2<<14)
#परिभाषय पत्तनम्_LED_आवरणम्   (3<<14)
#परिभाषय पत्तनम्_OWNER      (1<<13)
#परिभाषय पत्तनम्_POWER      (1<<12)
#परिभाषय पत्तनम्_LINESTATUS_आवरणम्   (3<<10)
#परिभाषय पत्तनम्_LINESTATUS_KSTATE (1<<10)
#परिभाषय पत्तनम्_RESET      (1<<8)
#परिभाषय पत्तनम्_SUSPEND    (1<<7)
#परिभाषय पत्तनम्_RESUME     (1<<6)
#परिभाषय पत्तनम्_OCC        (1<<5)
#परिभाषय पत्तनम्_OC         (1<<4)
#परिभाषय पत्तनम्_PEC        (1<<3)
#परिभाषय पत्तनम्_PE         (1<<2)
#परिभाषय पत्तनम्_CSC        (1<<1)
#परिभाषय पत्तनम्_CONNECT    (1<<0)
#परिभाषय पत्तनम्_RWC_द्वयङ्काः   (पत्तनम्_CSC | पत्तनम्_PEC | पत्तनम्_OCC)


#परिभाषय EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

संरचना ehci_qh {
    u32 अग्र;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_अग्र;
    u32 alt_अग्र;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#परिभाषय QH_CONTROL       (1 << 27)
#परिभाषय QH_MAXPACKET_सरणम् 16
#परिभाषय QH_MAXPACKET_आवरणम्  (0x7ff << QH_MAXPACKET_सरणम्)
#परिभाषय QH_शीर्ष          (1 << 15)
#परिभाषय QH_TOGGLECONTROL (1 << 14)
#परिभाषय QH_जवः_सरणम्   12
#परिभाषय QH_जवः_आवरणम्    (0x3 << QH_जवः_सरणम्)
#परिभाषय QH_EP_सरणम्      8
#परिभाषय QH_EP_आवरणम्       (0xf << QH_EP_सरणम्)
#परिभाषय QH_DEVADDR_सरणम् 0
#परिभाषय QH_DEVADDR_आवरणम्  (0x7f << QH_DEVADDR_सरणम्)

#परिभाषय QH_SMASK_सरणम्   0
#परिभाषय QH_SMASK_आवरणम्    (0xff << QH_SMASK_सरणम्)
#परिभाषय QH_CMASK_सरणम्   8
#परिभाषय QH_CMASK_आवरणम्    (0xff << QH_CMASK_सरणम्)
#परिभाषय QH_HUBADDR_सरणम् 16
#परिभाषय QH_HUBADDR_आवरणम्  (0x7f << QH_HUBADDR_सरणम्)
#परिभाषय QH_HUBPORT_सरणम् 23
#परिभाषय QH_HUBPORT_आवरणम्  (0x7f << QH_HUBPORT_सरणम्)
#परिभाषय QH_MULT_सरणम्    30
#परिभाषय QH_MULT_आवरणम्     (0x3 << QH_MULT_सरणम्)

#परिभाषय EHCI_PTR_द्वयङ्काः           0x001F
#परिभाषय EHCI_PTR_पदांशः           0x0001
#परिभाषय EHCI_PTR_QH             0x0002


#परिभाषय EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

संरचना ehci_qtd {
    u32 qtd_अग्र;
    u32 alt_अग्र;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#परिभाषय QTD_TOGGLE      (1 << 31)
#परिभाषय QTD_दीर्घता_सरणम् 16
#परिभाषय QTD_दीर्घता_आवरणम् (0x7fff << QTD_दीर्घता_सरणम्)
#परिभाषय QTD_CERR_सरणम्  10
#परिभाषय QTD_CERR_आवरणम्   (0x3 << QTD_CERR_सरणम्)
#परिभाषय QTD_IOC         (1 << 15)
#परिभाषय QTD_PID_बहिर्गमनम्     (0x0 << 8)
#परिभाषय QTD_PID_अन्तर्      (0x1 << 8)
#परिभाषय QTD_PID_सज्जता   (0x2 << 8)
#परिभाषय QTD_STS_सक्रिय  (1 << 7)
#परिभाषय QTD_STS_HALT    (1 << 6)
#परिभाषय QTD_STS_DBE     (1 << 5)
#परिभाषय QTD_STS_BABBLE  (1 << 4)
#परिभाषय QTD_STS_XACT    (1 << 3)
#परिभाषय QTD_STS_MMF     (1 << 2)
#परिभाषय QTD_STS_STS     (1 << 1)
#परिभाषय QTD_STS_PING    (1 << 0)

#परिभाषय ehci_explen(len) (((len) << QTD_दीर्घता_सरणम्) & QTD_दीर्घता_आवरणम्)

#परिभाषय ehci_maxerr(err) (((err) << QTD_CERR_सरणम्) & QTD_CERR_आवरणम्)


संरचना ehci_framelist {
    u32 links[1024];
} PACKED;

#यदि_अन्तः // usb-ehci.h
