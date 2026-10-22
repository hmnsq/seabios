#నిర్వచితం_కాకపోతే __USB_EHCI_H
#నిర్వచించు __USB_EHCI_H

// usb-ehci.c
శూన్య ehci_అమరిక(శూన్య);
శూన్య ehci_wait_controllers(శూన్య);
నిర్మాణం usbdevice_s;
నిర్మాణం usb_endpoint_descriptor;
నిర్మాణం usb_pipe;
నిర్మాణం usb_pipe *ehci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev
                                   , నిర్మాణం usb_pipe *upipe
                                   , నిర్మాణం usb_endpoint_descriptor *epdesc);
పూర్ణాంకం ehci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
                   , శూన్య *దత్తాంశం, పూర్ణాంకం datasize);
పూర్ణాంకం ehci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం);


/****************************************************************
 * ehci structs and flags
 ****************************************************************/

నిర్మాణం ehci_caps {
    u8 caplength;
    u8 reserved_01;
    u16 hciversion;
    u32 hcsparams;
    u32 hccparams;
    u64 portroute;
} PACKED;

#నిర్వచించు HCC_64_బిట్టు_ADDR 1

#నిర్వచించు HCS_N_PORTS_ముసుగు 0xf

నిర్మాణం ehci_regs {
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

#నిర్వచించు CMD_PARK        (1<<11)
#నిర్వచించు CMD_PARK_CNT(c) (((c)>>8)&3)
#నిర్వచించు CMD_LRESET      (1<<7)
#నిర్వచించు CMD_IAAD        (1<<6)
#నిర్వచించు CMD_ASE         (1<<5)
#నిర్వచించు CMD_PSE         (1<<4)
#నిర్వచించు CMD_HCRESET     (1<<1)
#నిర్వచించు CMD_నిరంతరం         (1<<0)

#నిర్వచించు STS_ASS         (1<<15)
#నిర్వచించు STS_PSS         (1<<14)
#నిర్వచించు STS_RECL        (1<<13)
#నిర్వచించు STS_HALT        (1<<12)
#నిర్వచించు STS_IAA         (1<<5)
#నిర్వచించు STS_FATAL       (1<<4)
#నిర్వచించు STS_FLR         (1<<3)
#నిర్వచించు STS_PCD         (1<<2)
#నిర్వచించు STS_ERR         (1<<1)
#నిర్వచించు STS_INT         (1<<0)

#నిర్వచించు FLAG_CF         (1<<0)

#నిర్వచించు పోర్టు_WKOC_E     (1<<22)
#నిర్వచించు పోర్టు_WKDISC_E   (1<<21)
#నిర్వచించు పోర్టు_WKCONN_E   (1<<20)
#నిర్వచించు పోర్టు_TEST_PKT   (0x4<<16)
#నిర్వచించు పోర్టు_LED_OFF    (0<<14)
#నిర్వచించు పోర్టు_LED_AMBER  (1<<14)
#నిర్వచించు పోర్టు_LED_GREEN  (2<<14)
#నిర్వచించు పోర్టు_LED_ముసుగు   (3<<14)
#నిర్వచించు పోర్టు_OWNER      (1<<13)
#నిర్వచించు పోర్టు_POWER      (1<<12)
#నిర్వచించు పోర్టు_LINESTATUS_ముసుగు   (3<<10)
#నిర్వచించు పోర్టు_LINESTATUS_KSTATE (1<<10)
#నిర్వచించు పోర్టు_RESET      (1<<8)
#నిర్వచించు పోర్టు_SUSPEND    (1<<7)
#నిర్వచించు పోర్టు_RESUME     (1<<6)
#నిర్వచించు పోర్టు_OCC        (1<<5)
#నిర్వచించు పోర్టు_OC         (1<<4)
#నిర్వచించు పోర్టు_PEC        (1<<3)
#నిర్వచించు పోర్టు_PE         (1<<2)
#నిర్వచించు పోర్టు_CSC        (1<<1)
#నిర్వచించు పోర్టు_CONNECT    (1<<0)
#నిర్వచించు పోర్టు_RWC_బిట్లు   (పోర్టు_CSC | పోర్టు_PEC | పోర్టు_OCC)


#నిర్వచించు EHCI_QH_ALIGN 128 // Can't span a 4K boundary, so increase from 32

నిర్మాణం ehci_qh {
    u32 తదుపరి;
    u32 info1;
    u32 info2;
    u32 current;

    u32 qtd_తదుపరి;
    u32 alt_తదుపరి;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
} PACKED;

#నిర్వచించు QH_CONTROL       (1 << 27)
#నిర్వచించు QH_MAXPACKET_జరుపు 16
#నిర్వచించు QH_MAXPACKET_ముసుగు  (0x7ff << QH_MAXPACKET_జరుపు)
#నిర్వచించు QH_తల          (1 << 15)
#నిర్వచించు QH_TOGGLECONTROL (1 << 14)
#నిర్వచించు QH_వేగం_జరుపు   12
#నిర్వచించు QH_వేగం_ముసుగు    (0x3 << QH_వేగం_జరుపు)
#నిర్వచించు QH_EP_జరుపు      8
#నిర్వచించు QH_EP_ముసుగు       (0xf << QH_EP_జరుపు)
#నిర్వచించు QH_DEVADDR_జరుపు 0
#నిర్వచించు QH_DEVADDR_ముసుగు  (0x7f << QH_DEVADDR_జరుపు)

#నిర్వచించు QH_SMASK_జరుపు   0
#నిర్వచించు QH_SMASK_ముసుగు    (0xff << QH_SMASK_జరుపు)
#నిర్వచించు QH_CMASK_జరుపు   8
#నిర్వచించు QH_CMASK_ముసుగు    (0xff << QH_CMASK_జరుపు)
#నిర్వచించు QH_HUBADDR_జరుపు 16
#నిర్వచించు QH_HUBADDR_ముసుగు  (0x7f << QH_HUBADDR_జరుపు)
#నిర్వచించు QH_HUBPORT_జరుపు 23
#నిర్వచించు QH_HUBPORT_ముసుగు  (0x7f << QH_HUBPORT_జరుపు)
#నిర్వచించు QH_MULT_జరుపు    30
#నిర్వచించు QH_MULT_ముసుగు     (0x3 << QH_MULT_జరుపు)

#నిర్వచించు EHCI_PTR_బిట్లు           0x001F
#నిర్వచించు EHCI_PTR_పదం           0x0001
#నిర్వచించు EHCI_PTR_QH             0x0002


#నిర్వచించు EHCI_QTD_ALIGN 64 // Can't span a 4K boundary, so increase from 32

నిర్మాణం ehci_qtd {
    u32 qtd_తదుపరి;
    u32 alt_తదుపరి;
    u32 token;
    u32 buf[5];
    u32 buf_hi[5];
    /* keep struct size a multiple of 64 bytes, as we're allocating
       arrays. Without this padding, the second qtd could have the
       wrong alignment. */
} PACKED __aligned(EHCI_QTD_ALIGN);

#నిర్వచించు QTD_TOGGLE      (1 << 31)
#నిర్వచించు QTD_నిడివి_జరుపు 16
#నిర్వచించు QTD_నిడివి_ముసుగు (0x7fff << QTD_నిడివి_జరుపు)
#నిర్వచించు QTD_CERR_జరుపు  10
#నిర్వచించు QTD_CERR_ముసుగు   (0x3 << QTD_CERR_జరుపు)
#నిర్వచించు QTD_IOC         (1 << 15)
#నిర్వచించు QTD_PID_బహిర్గతం     (0x0 << 8)
#నిర్వచించు QTD_PID_లోపల      (0x1 << 8)
#నిర్వచించు QTD_PID_అమరిక   (0x2 << 8)
#నిర్వచించు QTD_STS_చురుకైన  (1 << 7)
#నిర్వచించు QTD_STS_HALT    (1 << 6)
#నిర్వచించు QTD_STS_DBE     (1 << 5)
#నిర్వచించు QTD_STS_BABBLE  (1 << 4)
#నిర్వచించు QTD_STS_XACT    (1 << 3)
#నిర్వచించు QTD_STS_MMF     (1 << 2)
#నిర్వచించు QTD_STS_STS     (1 << 1)
#నిర్వచించు QTD_STS_PING    (1 << 0)

#నిర్వచించు ehci_explen(len) (((len) << QTD_నిడివి_జరుపు) & QTD_నిడివి_ముసుగు)

#నిర్వచించు ehci_maxerr(err) (((err) << QTD_CERR_జరుపు) & QTD_CERR_ముసుగు)


నిర్మాణం ehci_framelist {
    u32 links[1024];
} PACKED;

#అయితే_ముగింపు // usb-ehci.h
