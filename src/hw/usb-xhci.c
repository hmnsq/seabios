// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // memalign_low
#అన్తర్భావ "memmap.h" // PAGE_SIZE
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#అన్తర్భావ "pci_regs.h" // PCI_BASE_ADDRESS_0
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-xhci.h" // struct ehci_qh
#అన్తర్భావ "util.h" // timer_calc
#అన్తర్భావ "x86.h" // readl

// --------------------------------------------------------------
// configuration

#నిర్వచించు XHCI_RING_వస్తువులు          16
#నిర్వచించు XHCI_RING_కొలత           (XHCI_RING_వస్తువులు*పరిమాణం(నిర్మాణం xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#నిర్వచించు XHCI_RING(_trb)          \
    ((నిర్మాణం xhci_ring*)((u32)(_trb) & ~(XHCI_RING_కొలత-1)))

// --------------------------------------------------------------
// bit definitions

#నిర్వచించు XHCI_CMD_RS              (1<<0)
#నిర్వచించు XHCI_CMD_HCRST           (1<<1)
#నిర్వచించు XHCI_CMD_INTE            (1<<2)
#నిర్వచించు XHCI_CMD_HSEE            (1<<3)
#నిర్వచించు XHCI_CMD_LHCRST          (1<<7)
#నిర్వచించు XHCI_CMD_CSS             (1<<8)
#నిర్వచించు XHCI_CMD_CRS             (1<<9)
#నిర్వచించు XHCI_CMD_EWE             (1<<10)
#నిర్వచించు XHCI_CMD_EU3S            (1<<11)

#నిర్వచించు XHCI_STS_HCH             (1<<0)
#నిర్వచించు XHCI_STS_HSE             (1<<2)
#నిర్వచించు XHCI_STS_EINT            (1<<3)
#నిర్వచించు XHCI_STS_PCD             (1<<4)
#నిర్వచించు XHCI_STS_SSS             (1<<8)
#నిర్వచించు XHCI_STS_RSS             (1<<9)
#నిర్వచించు XHCI_STS_SRE             (1<<10)
#నిర్వచించు XHCI_STS_CNR             (1<<11)
#నిర్వచించు XHCI_STS_HCE             (1<<12)

#నిర్వచించు XHCI_PORTSC_CCS          (1<<0)
#నిర్వచించు XHCI_PORTSC_PED          (1<<1)
#నిర్వచించు XHCI_PORTSC_OCA          (1<<3)
#నిర్వచించు XHCI_PORTSC_PR           (1<<4)
#నిర్వచించు XHCI_PORTSC_PLS_జరుపు        5
#నిర్వచించు XHCI_PORTSC_PLS_ముసుగు     0xf
#నిర్వచించు XHCI_PORTSC_PP           (1<<9)
#నిర్వచించు XHCI_PORTSC_వేగం_జరుపు      10
#నిర్వచించు XHCI_PORTSC_వేగం_ముసుగు   0xf
#నిర్వచించు XHCI_PORTSC_వేగం_నిండిన   (1<<10)
#నిర్వచించు XHCI_PORTSC_వేగం_దిగువ    (2<<10)
#నిర్వచించు XHCI_PORTSC_వేగం_ఎగువ   (3<<10)
#నిర్వచించు XHCI_PORTSC_వేగం_SUPER  (4<<10)
#నిర్వచించు XHCI_PORTSC_PIC_జరుపు        14
#నిర్వచించు XHCI_PORTSC_PIC_ముసుగు     0x3
#నిర్వచించు XHCI_PORTSC_LWS          (1<<16)
#నిర్వచించు XHCI_PORTSC_CSC          (1<<17)
#నిర్వచించు XHCI_PORTSC_PEC          (1<<18)
#నిర్వచించు XHCI_PORTSC_WRC          (1<<19)
#నిర్వచించు XHCI_PORTSC_OCC          (1<<20)
#నిర్వచించు XHCI_PORTSC_PRC          (1<<21)
#నిర్వచించు XHCI_PORTSC_PLC          (1<<22)
#నిర్వచించు XHCI_PORTSC_CEC          (1<<23)
#నిర్వచించు XHCI_PORTSC_CAS          (1<<24)
#నిర్వచించు XHCI_PORTSC_WCE          (1<<25)
#నిర్వచించు XHCI_PORTSC_WDE          (1<<26)
#నిర్వచించు XHCI_PORTSC_WOE          (1<<27)
#నిర్వచించు XHCI_PORTSC_DR           (1<<30)
#నిర్వచించు XHCI_PORTSC_WPR          (1<<31)

#నిర్వచించు TRB_C               (1<<0)
#నిర్వచించు TRB_TYPE_జరుపు          10
#నిర్వచించు TRB_TYPE_ముసుగు       0x3f
#నిర్వచించు TRB_TYPE(t)         (((t) >> TRB_TYPE_జరుపు) & TRB_TYPE_ముసుగు)

#నిర్వచించు TRB_EV_ED           (1<<2)

#నిర్వచించు TRB_TR_ENT          (1<<1)
#నిర్వచించు TRB_TR_ISP          (1<<2)
#నిర్వచించు TRB_TR_NS           (1<<3)
#నిర్వచించు TRB_TR_CH           (1<<4)
#నిర్వచించు TRB_TR_IOC          (1<<5)
#నిర్వచించు TRB_TR_IDT          (1<<6)
#నిర్వచించు TRB_TR_TBC_జరుపు        7
#నిర్వచించు TRB_TR_TBC_ముసుగు     0x3
#నిర్వచించు TRB_TR_BEI          (1<<9)
#నిర్వచించు TRB_TR_TLBPC_జరుపు      16
#నిర్వచించు TRB_TR_TLBPC_ముసుగు   0xf
#నిర్వచించు TRB_TR_FRAMEID_జరుపు    20
#నిర్వచించు TRB_TR_FRAMEID_ముసుగు 0x7ff
#నిర్వచించు TRB_TR_SIA          (1<<31)

#నిర్వచించు TRB_TR_DIR          (1<<16)

#నిర్వచించు TRB_CR_SLOTID_జరుపు     24
#నిర్వచించు TRB_CR_SLOTID_ముసుగు  0xff
#నిర్వచించు TRB_CR_EPID_జరుపు       16
#నిర్వచించు TRB_CR_EPID_ముసుగు    0x1f

#నిర్వచించు TRB_CR_BSR          (1<<9)
#నిర్వచించు TRB_CR_DC           (1<<9)

#నిర్వచించు TRB_LK_TC           (1<<1)

#నిర్వచించు TRB_INTR_జరుపు          22
#నిర్వచించు TRB_INTR_ముసుగు       0x3ff
#నిర్వచించు TRB_INTR(t)         (((t).స్థితి_2 >> TRB_INTR_జరుపు) & TRB_INTR_ముసుగు)

రకనిర్వచనం గణన TRBType {
    TRB_RESERVED = 0,
    TR_సాధారణ,
    TR_అమరిక,
    TR_దత్తాంశం,
    TR_స్థితి,
    TR_ISOCH,
    TR_LINK,
    TR_EVDATA,
    TR_NOOP,
    CR_ENABLE_స్థానం,
    CR_DISABLE_స్థానం,
    CR_చిరునామా_DEVICE,
    CR_CONFIGURE_ENDPOINT,
    CR_EVALUATE_CONTEXT,
    CR_RESET_ENDPOINT,
    CR_STOP_ENDPOINT,
    CR_సమితి_TR_వరుస_నుండి_తీయి,
    CR_RESET_DEVICE,
    CR_FORCE_EVENT,
    CR_NEGOTIATE_BW,
    CR_సమితి_LATENCY_TOLERANCE,
    CR_GET_పోర్టు_BANDWIDTH,
    CR_FORCE_శీర్షిక,
    CR_NOOP,
    ER_TRANSFER = 32,
    ER_COMMAND_COMPLETE,
    ER_పోర్టు_స్థితి_CHANGE,
    ER_BANDWIDTH_REQUEST,
    ER_DOORBELL,
    ER_HOST_CONTROLLER,
    ER_DEVICE_NOTIFICATION,
    ER_MFINDEX_WRAP,
} TRBType;

రకనిర్వచనం గణన TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_దత్తాంశం_బఫరు_ERROR,
    CC_BABBLE_DETECTED,
    CC_USB_TRANSACTION_ERROR,
    CC_TRB_ERROR,
    CC_STALL_ERROR,
    CC_RESOURCE_ERROR,
    CC_BANDWIDTH_ERROR,
    CC_NO_SLOTS_ERROR,
    CC_INVALID_ప్రవాహం_TYPE_ERROR,
    CC_స్థానం_NOT_ENABLED_ERROR,
    CC_EP_NOT_ENABLED_ERROR,
    CC_SHORT_PACKET,
    CC_RING_UNDERRUN,
    CC_RING_OVERRUN,
    CC_VF_ER_నిండిన,
    CC_PARAMETER_ERROR,
    CC_BANDWIDTH_OVERRUN,
    CC_CONTEXT_STATE_ERROR,
    CC_NO_PING_RESPONSE_ERROR,
    CC_EVENT_RING_నిండిన_ERROR,
    CC_INCOMPATIBLE_DEVICE_ERROR,
    CC_MISSED_SERVICE_ERROR,
    CC_COMMAND_RING_STOPPED,
    CC_COMMAND_ABORTED,
    CC_STOPPED,
    CC_STOPPED_నిడివి_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_బఫరు_OVERRUN = 31,
    CC_EVENT_కోల్పోయిన_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_ప్రవాహం_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_విభజించు_TRANSACTION_ERROR
} TRBCCode;

గణన {
    PLS_U0              =  0,
    PLS_U1              =  1,
    PLS_U2              =  2,
    PLS_U3              =  3,
    PLS_DISABLED        =  4,
    PLS_RX_DETECT       =  5,
    PLS_INACTIVE        =  6,
    PLS_POLLING         =  7,
    PLS_RECOVERY        =  8,
    PLS_HOT_RESET       =  9,
    PLS_COMPILANCE_బహుళకం = 10,
    PLS_TEST_బహుళకం       = 11,
    PLS_RESUME          = 15,
};

#నిర్వచించు xhci_get_క్షేత్రం(దత్తాంశం, క్షేత్రం)             \
    (((దత్తాంశం) >> క్షేత్రం##_SHIFT) & క్షేత్రం##_MASK)

// --------------------------------------------------------------
// state structs

నిర్మాణం xhci_ring {
    నిర్మాణం xhci_trb      ring[XHCI_RING_వస్తువులు];
    నిర్మాణం xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    నిర్మాణం mutex_s       తాళం_2;
};

నిర్మాణం xhci_portmap {
    u8 ప్రారంభం;
    u8 లెక్క;
};

నిర్మాణం usb_xhci_s {
    నిర్మాణం usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    నిర్మాణం xhci_portmap  usb2;
    నిర్మాణం xhci_portmap  usb3;

    /* xhci registers */
    నిర్మాణం xhci_caps     *caps;
    నిర్మాణం xhci_op       *op;
    నిర్మాణం xhci_pr       *pr;
    నిర్మాణం xhci_ir       *ir;
    నిర్మాణం xhci_db       *db;

    /* xhci data structures */
    నిర్మాణం xhci_devlist  *devs;
    నిర్మాణం xhci_ring     *cmds;
    నిర్మాణం xhci_ring     *evts;
    నిర్మాణం xhci_er_seg   *eseg;
};

నిర్మాణం xhci_pipe {
    నిర్మాణం xhci_ring     reqs;

    నిర్మాణం usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    శూన్య                 *buf;
    పూర్ణాంకం                  bufused;
};

// --------------------------------------------------------------
// tables

స్థిర స్థిరరాశి అక్షరం *వేగం_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

స్థిర స్థిరరాశి పూర్ణాంకం వేగం_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

స్థిర స్థిరరాశి పూర్ణాంకం వేగం_గమ్యం_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

స్థిర పూర్ణాంకం wait_బిట్టు(u32 *reg, u32 ముసుగు, పూర్ణాంకం విలువ, u32 timeout)
{
    u32 చివర = timer_calc(timeout);

    అయితే ((readl(reg) & ముసుగు) != విలువ) {
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }
    తిరిగి 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#నిర్వచించు XHCI_సమయం_POSTPOWER 20

// Check if device attached to port
స్థిర శూన్య
xhci_print_పోర్టు_state(పూర్ణాంకం loglevel, స్థిరరాశి అక్షరం *ఉపసర్గ, u32 పోర్టు, u32 portsc)
{
    u32 pls = xhci_get_క్షేత్రం(portsc, XHCI_PORTSC_PLS);
    u32 వేగం = xhci_get_క్షేత్రం(portsc, XHCI_PORTSC_వేగం);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            ఉపసర్గ, పోర్టు + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, వేగం, వేగం_name[వేగం]);
}

స్థిర పూర్ణాంకం
xhci_hub_detect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(hub->cntl, నిర్మాణం usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[పోర్టు].portsc);
    తిరిగి (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
స్థిర పూర్ణాంకం
xhci_hub_reset(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(hub->cntl, నిర్మాణం usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[పోర్టు].portsc);
    ఒకవేళ (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        తిరిగి -1;

    ఎంపిక (xhci_get_క్షేత్రం(portsc, XHCI_PORTSC_PLS)) {
    స్థితి PLS_U0:
        // A USB3 port - controller automatically performs reset
        ఆపు;
    స్థితి PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_పోర్టు_state(3, __func__, పోర్టు, portsc);
        writel(&xhci->pr[పోర్టు].portsc, portsc | XHCI_PORTSC_PR);
        ఆపు;
    అప్రమేయ:
        తిరిగి -1;
    }

    // Wait for device to complete reset and be enabled
    u32 చివర = timer_calc(100);
    కొరకు (;;) {
        portsc = readl(&xhci->pr[పోర్టు].portsc);
        ఒకవేళ (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            తిరిగి -1;
        ఒకవేళ (portsc & XHCI_PORTSC_PED)
            // Reset complete
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }

    పూర్ణాంకం rc = వేగం_from_xhci[xhci_get_క్షేత్రం(portsc, XHCI_PORTSC_వేగం)];
    xhci_print_పోర్టు_state(1, "XHCI", పోర్టు, portsc);
    తిరిగి rc;
}

స్థిర పూర్ణాంకం
xhci_hub_portmap(నిర్మాణం usbhub_s *hub, u32 vport)
{
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(hub->cntl, నిర్మాణం usb_xhci_s, usb);
    u32 pport = vport + 1;

    ఒకవేళ (vport + 1 >= xhci->usb3.ప్రారంభం &&
        vport + 1 < xhci->usb3.ప్రారంభం + xhci->usb3.లెక్క)
        pport = vport + 2 - xhci->usb3.ప్రారంభం;

    ఒకవేళ (vport + 1 >= xhci->usb2.ప్రారంభం &&
        vport + 1 < xhci->usb2.ప్రారంభం + xhci->usb2.లెక్క)
        pport = vport + 2 - xhci->usb2.ప్రారంభం;

    తిరిగి pport;
}

స్థిర శూన్య
xhci_hub_disconnect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    // XXX - should turn the port power off.
}

స్థిర నిర్మాణం usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
స్థిర పూర్ణాంకం
xhci_check_ports(నిర్మాణం usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_సమయం_POSTPOWER);

    నిర్మాణం usbhub_s hub;
    memset(&hub, 0, పరిమాణం(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    తిరిగి hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

స్థిర శూన్య
xhci_free_pipes(నిర్మాణం usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

స్థిర శూన్య
configure_xhci(శూన్య *దత్తాంశం)
{
    నిర్మాణం usb_xhci_s *xhci = దత్తాంశం;
    u32 reg;

    xhci->devs = memalign_ఎగువ(64, పరిమాణం(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ఎగువ(64, పరిమాణం(*xhci->eseg));
    xhci->cmds = memalign_ఎగువ(XHCI_RING_కొలత, పరిమాణం(*xhci->cmds));
    xhci->evts = memalign_ఎగువ(XHCI_RING_కొలత, పరిమాణం(*xhci->evts));
    ఒకవేళ (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        వెళ్ళు fail;
    }
    memset(xhci->devs, 0, పరిమాణం(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, పరిమాణం(*xhci->cmds));
    memset(xhci->evts, 0, పరిమాణం(*xhci->evts));
    memset(xhci->eseg, 0, పరిమాణం(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    ఒకవేళ (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        ఒకవేళ (wait_బిట్టు(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            వెళ్ళు fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    ఒకవేళ (wait_బిట్టు(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        వెళ్ళు fail;
    ఒకవేళ (wait_బిట్టు(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        వెళ్ళు fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_దిగువ, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ఎగువ, 0);
    writel(&xhci->op->crcr_దిగువ, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ఎగువ, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_దిగువ = (u32)xhci->evts;
    xhci->eseg->ptr_ఎగువ = 0;
    xhci->eseg->కొలత = XHCI_RING_వస్తువులు;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_దిగువ, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ఎగువ, 0);
    writel(&xhci->ir->erstba_దిగువ, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ఎగువ, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    ఒకవేళ (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ఎగువ(64, పరిమాణం(*spba) * spb);
        శూన్య *pad = memalign_ఎగువ(పుట_కొలత, పుట_కొలత * spb);
        ఒకవేళ (!spba || !pad) {
            warn_noalloc();
            విడుదల(spba);
            విడుదల(pad);
            వెళ్ళు fail;
        }
        పూర్ణాంకం i;
        కొరకు (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * పుట_కొలత);
        xhci->devs[0].ptr_దిగువ = (u32)spba;
        xhci->devs[0].ptr_ఎగువ = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    పూర్ణాంకం లెక్క = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    ఒకవేళ (లెక్క)
        // Success
        తిరిగి;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_బిట్టు(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    విడుదల(xhci->eseg);
    విడుదల(xhci->evts);
    విడుదల(xhci->cmds);
    విడుదల(xhci->devs);
    విడుదల(xhci);
}

స్థిర నిర్మాణం usb_xhci_s*
xhci_controller_అమరిక(శూన్య *baseaddr)
{
    నిర్మాణం usb_xhci_s *xhci = malloc_ఎగువ(పరిమాణం(*xhci));
    ఒకవేళ (!xhci) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(xhci, 0, పరిమాణం(*xhci));
    xhci->caps  = baseaddr;
    xhci->op    = baseaddr + readb(&xhci->caps->caplength);
    xhci->pr    = baseaddr + readb(&xhci->caps->caplength) + 0x400;
    xhci->db    = baseaddr + readl(&xhci->caps->dboff);
    xhci->ir    = baseaddr + readl(&xhci->caps->rtsoff) + 0x20;

    u32 hcs1 = readl(&xhci->caps->hcsparams1);
    u32 hcc  = readl(&xhci->caps->hccparams);
    xhci->ports = (hcs1 >> 24) & 0xff;
    xhci->slots = hcs1         & 0xff;
    xhci->xcap  = ((hcc >> 16) & 0xffff) << 2;
    xhci->context64 = (hcc & 0x04) ? 1 : 0;
    xhci->usb.type = USB_TYPE_XHCI;

    dprintf(1, "XHCI init: regs @ %p, %d ports, %d slots"
            ", %d byte contexts\n"
            , xhci->caps, xhci->ports, xhci->slots
            , xhci->context64 ? 64 : 32);

    ఒకవేళ (xhci->xcap) {
        u32 off;
        శూన్య *addr = baseaddr + xhci->xcap;
        చేయి {
            నిర్మాణం xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            ఎంపిక (cap & 0xff) {
            స్థితి 0x02:
                name  = readl(&xcap->దత్తాంశం[0]);
                ports = readl(&xcap->దత్తాంశం[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 లెక్క = (ports >> 8) & 0xff;
                u8 ప్రారంభం = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , లెక్క, ప్రారంభం
                        , ports >> 16);
                ఒకవేళ (name == 0x20425355 /* "USB " */) {
                    ఒకవేళ (major == 2) {
                        xhci->usb2.ప్రారంభం = ప్రారంభం;
                        xhci->usb2.లెక్క = లెక్క;
                    }
                    ఒకవేళ (major == 3) {
                        xhci->usb3.ప్రారంభం = ప్రారంభం;
                        xhci->usb3.లెక్క = లెక్క;
                    }
                }
                ఆపు;
            అప్రమేయ:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                ఆపు;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } అయితే (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    ఒకవేళ (పుట_కొలత != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        విడుదల(xhci);
        తిరిగి ఖాళీ;
    }

    తిరిగి xhci;
}

స్థిర శూన్య
xhci_controller_అమరిక_pci(నిర్మాణం pci_device *pci)
{
    నిర్మాణం usb_xhci_s *xhci;
    శూన్య *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!baseaddr)
        తిరిగి;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_అమరిక(baseaddr);
    ఒకవేళ (!xhci)
        తిరిగి;

    xhci->usb.pci = pci;
    నిరంతరం_thread(configure_xhci, xhci);
}

స్థిర శూన్య
xhci_controller_అమరిక_acpi(నిర్మాణం acpi_device *dev)
{
    నిర్మాణం usb_xhci_s *xhci;
    u64 mem, వాడని;
    శూన్య *baseaddr;

    ఒకవేళ (acpi_dsdt_కనుగొను_mem(dev, &mem, &వాడని) < 0)
        తిరిగి;
    ఒకవేళ (mem >= 0x100000000ll)
        తిరిగి;

    baseaddr = (శూన్య*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_అమరిక(baseaddr);
    ఒకవేళ (!xhci)
        తిరిగి;

    xhci->usb.mmio = baseaddr;
    నిరంతరం_thread(configure_xhci, xhci);
}

శూన్య
xhci_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_XHCI)
        తిరిగి;

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_అమరిక_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    నిర్మాణం acpi_device *dev;
    కొరకు (dev = acpi_dsdt_కనుగొను_eisaid(ఖాళీ, xhci_eisaid);
         dev != ఖాళీ;
         dev = acpi_dsdt_కనుగొను_eisaid(dev, xhci_eisaid)) {
        xhci_controller_అమరిక_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
స్థిర శూన్య xhci_doorbell(నిర్మాణం usb_xhci_s *xhci, u32 slotid, u32 విలువ)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, విలువ);
    నిర్మాణం xhci_db *db = xhci->db;
    శూన్య *addr = &db[slotid].doorbell;
    writel(addr, విలువ);
}

// Dequeue events on the XHCI command ring generated by the hardware
స్థిర శూన్య xhci_process_events(నిర్మాణం usb_xhci_s *xhci)
{
    నిర్మాణం xhci_ring *evts = xhci->evts;

    కొరకు (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        నిర్మాణం xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        ఒకవేళ ((control & TRB_C) != (cs ? 1 : 0))
            తిరిగి;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->స్థితి_2 >> 24) & 0xff;
        ఎంపిక (evt_type) {
        స్థితి ER_TRANSFER:
        స్థితి ER_COMMAND_COMPLETE:
        {
            నిర్మాణం xhci_trb  *rtrb = (శూన్య*)etrb->ptr_దిగువ;
            నిర్మాణం xhci_ring *ring = XHCI_RING(rtrb);
            నిర్మాణం xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            జ్ఞాపకం_నకలు(evt, etrb, పరిమాణం(*etrb));
            ring->eidx = eidx;
            ఆపు;
        }
        స్థితి ER_పోర్టు_స్థితి_CHANGE:
        {
            u32 పోర్టు = ((etrb->ptr_దిగువ >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[పోర్టు].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_ముసుగు<<XHCI_PORTSC_PLS_జరుపు))
                          | (1<<XHCI_PORTSC_PLS_జరుపు));
            writel(&xhci->pr[పోర్టు].portsc, pclear);

            xhci_print_పోర్టు_state(3, __func__, పోర్టు, portsc);
            ఆపు;
        }
        అప్రమేయ:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            ఆపు;
        }

        /* move ring index, notify xhci */
        nidx++;
        ఒకవేళ (nidx == XHCI_RING_వస్తువులు) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        నిర్మాణం xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_దిగువ, erdp);
        writel(&ir->erdp_ఎగువ, 0);
    }
}

// Check if a ring has any pending TRBs
స్థిర పూర్ణాంకం xhci_ring_busy(నిర్మాణం xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    తిరిగి (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
స్థిర పూర్ణాంకం xhci_event_wait(నిర్మాణం usb_xhci_s *xhci,
                           నిర్మాణం xhci_ring *ring,
                           u32 timeout)
{
    u32 చివర = timer_calc(timeout);

    కొరకు (;;) {
        xhci_process_events(xhci);
        ఒకవేళ (!xhci_ring_busy(ring)) {
            u32 స్థితి_2 = ring->evt.స్థితి_2;
            తిరిగి (స్థితి_2 >> 24) & 0xff;
        }
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
స్థిర శూన్య xhci_trb_fill(నిర్మాణం xhci_ring *ring
                          , శూన్య *దత్తాంశం, u32 xferlen, u32 flags)
{
    నిర్మాణం xhci_trb *dst = &ring->ring[ring->nidx];
    ఒకవేళ (flags & TRB_TR_IDT) {
        జ్ఞాపకం_నకలు(&dst->ptr_దిగువ, దత్తాంశం, xferlen);
    } లేకపోతే {
        dst->ptr_దిగువ = (u32)దత్తాంశం;
        dst->ptr_ఎగువ = 0;
    }
    dst->స్థితి_2 = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
స్థిర శూన్య xhci_trb_వరుస(నిర్మాణం xhci_ring *ring,
                           శూన్య *దత్తాంశం, u32 xferlen, u32 flags)
{
    ఒకవేళ (ring->nidx >= ARRAY_కొలత(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, దత్తాంశం, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
స్థిర పూర్ణాంకం xhci_cmd_submit(నిర్మాణం usb_xhci_s *xhci, నిర్మాణం xhci_inctx *inctx
                           , u32 flags)
{
    ఒకవేళ (inctx) {
        నిర్మాణం xhci_slotctx *స్థానం = (శూన్య*)&inctx[1 << xhci->context64];
        u32 పోర్టు = ((స్థానం->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[పోర్టు].portsc);
        ఒకవేళ (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_పోర్టు_state(1, __func__, పోర్టు, portsc);
            తిరిగి -1;
        }
    }

    mutex_తాళం(&xhci->cmds->తాళం_2);
    xhci_trb_వరుస(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    పూర్ణాంకం rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->తాళం_2);
    తిరిగి rc;
}

స్థిర పూర్ణాంకం xhci_cmd_enable_స్థానం(నిర్మాణం usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    పూర్ణాంకం cc = xhci_cmd_submit(xhci, ఖాళీ, CR_ENABLE_స్థానం << 10);
    ఒకవేళ (cc != CC_SUCCESS)
        తిరిగి -1;
    తిరిగి (xhci->cmds->evt.control >> 24) & 0xff;
}

స్థిర పూర్ణాంకం xhci_cmd_disable_స్థానం(నిర్మాణం usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    తిరిగి xhci_cmd_submit(xhci, ఖాళీ, (CR_DISABLE_స్థానం << 10) | (slotid << 24));
}

స్థిర పూర్ణాంకం xhci_cmd_చిరునామా_device(నిర్మాణం usb_xhci_s *xhci, u32 slotid
                                   , నిర్మాణం xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    తిరిగి xhci_cmd_submit(xhci, inctx
                           , (CR_చిరునామా_DEVICE << 10) | (slotid << 24));
}

స్థిర పూర్ణాంకం xhci_cmd_configure_endpoint(నిర్మాణం usb_xhci_s *xhci, u32 slotid
                                       , నిర్మాణం xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->కూడు, inctx->del);
    తిరిగి xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

స్థిర పూర్ణాంకం xhci_cmd_evaluate_context(నిర్మాణం usb_xhci_s *xhci, u32 slotid
                                     , నిర్మాణం xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->కూడు, inctx->del);
    తిరిగి xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

స్థిర నిర్మాణం xhci_inctx *
xhci_alloc_inctx(నిర్మాణం usbdevice_s *usbdev, పూర్ణాంకం maxepid)
{
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_xhci_s, usb);
    పూర్ణాంకం కొలత = (పరిమాణం(నిర్మాణం xhci_inctx) * 33) << xhci->context64;
    నిర్మాణం xhci_inctx *లోపల = memalign_tmphigh(2048 << xhci->context64, కొలత);
    ఒకవేళ (!లోపల) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(లోపల, 0, కొలత);

    నిర్మాణం xhci_slotctx *స్థానం = (శూన్య*)&లోపల[1 << xhci->context64];
    స్థానం->ctx[0]    |= maxepid << 27; // context entries
    స్థానం->ctx[0]    |= వేగం_గమ్యం_xhci[usbdev->వేగం] << 20;

    // Set high-speed hub flags.
    నిర్మాణం usbdevice_s *hubdev = usbdev->hub->usbdev;
    ఒకవేళ (hubdev) {
        ఒకవేళ (usbdev->వేగం == USB_LOWSPEED || usbdev->వేగం == USB_FULLSPEED) {
            నిర్మాణం xhci_pipe *hpipe = container_యొక్క(
                hubdev->defpipe, నిర్మాణం xhci_pipe, pipe);
            ఒకవేళ (hubdev->వేగం == USB_HIGHSPEED) {
                స్థానం->ctx[2] |= hpipe->slotid;
                స్థానం->ctx[2] |= (usbdev->పోర్టు+1) << 8;
            } లేకపోతే {
                నిర్మాణం xhci_slotctx *hslot = (శూన్య*)xhci->devs[hpipe->slotid].ptr_దిగువ;
                స్థానం->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        అయితే (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->పోర్టు+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        స్థానం->ctx[0]    |= route;
    }

    స్థానం->ctx[1]    |= (usbdev->పోర్టు+1) << 16;

    తిరిగి లోపల;
}

స్థిర పూర్ణాంకం xhci_config_hub(నిర్మాణం usbhub_s *hub)
{
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        hub->cntl, నిర్మాణం usb_xhci_s, usb);
    నిర్మాణం xhci_pipe *pipe = container_యొక్క(
        hub->usbdev->defpipe, నిర్మాణం xhci_pipe, pipe);
    నిర్మాణం xhci_slotctx *hdslot = (శూన్య*)xhci->devs[pipe->slotid].ptr_దిగువ;
    ఒకవేళ ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        తిరిగి 0;
    నిర్మాణం xhci_inctx *లోపల = xhci_alloc_inctx(hub->usbdev, 1);
    ఒకవేళ (!లోపల)
        తిరిగి -1;
    లోపల->కూడు = 0x01;
    నిర్మాణం xhci_slotctx *స్థానం = (శూన్య*)&లోపల[1 << xhci->context64];
    స్థానం->ctx[0] |= 1 << 26;
    స్థానం->ctx[1] |= hub->portcount << 24;

    పూర్ణాంకం cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, లోపల);
    విడుదల(లోపల);
    ఒకవేళ (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        తిరిగి -1;
    }
    తిరిగి 0;
}

స్థిర నిర్మాణం usb_pipe *
xhci_alloc_pipe(నిర్మాణం usbdevice_s *usbdev
                , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు;
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_xhci_s, usb);
    నిర్మాణం xhci_pipe *pipe;
    u32 epid;

    ఒకవేళ (epdesc->b_Endpoint_చిరునామా == 0) {
        epid = 1;
    } లేకపోతే {
        epid = (epdesc->b_Endpoint_చిరునామా & 0x0f) * 2;
        epid += (epdesc->b_Endpoint_చిరునామా & USB_DIR_లోపల) ? 1 : 0;
    }

    ఒకవేళ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ఎగువ(XHCI_RING_కొలత, పరిమాణం(*pipe));
    లేకపోతే
        pipe = memalign_దిగువ(XHCI_RING_కొలత, పరిమాణం(*pipe));
    ఒకవేళ (!pipe) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(pipe, 0, పరిమాణం(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ఎగువ(pipe->pipe.maxpacket);
        ఒకవేళ (!pipe->buf) {
            warn_noalloc();
            విడుదల(pipe);
            తిరిగి ఖాళీ;
        }
    }

    // Allocate input context and initialize endpoint info.
    నిర్మాణం xhci_inctx *లోపల = xhci_alloc_inctx(usbdev, epid);
    ఒకవేళ (!లోపల)
        వెళ్ళు fail;
    లోపల->కూడు = 0x01 | (1 << epid);
    నిర్మాణం xhci_epctx *ep = (శూన్య*)&లోపల[(pipe->epid+1) << xhci->context64];
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    ఒకవేళ (epdesc->b_Endpoint_చిరునామా & USB_DIR_లోపల
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_దిగువ  = (u32)&pipe->reqs.ring[0];
    ep->deq_దిగువ  |= 1;         // dcs
    ep->నిడివి   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ఒకవేళ (pipe->epid == 1) {
        ఒకవేళ (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            పూర్ణాంకం ret = xhci_config_hub(usbdev->hub);
            ఒకవేళ (ret)
                వెళ్ళు fail;
        }
        // Enable slot.
        u32 కొలత = (పరిమాణం(నిర్మాణం xhci_slotctx) * 32) << xhci->context64;
        నిర్మాణం xhci_slotctx *dev = memalign_ఎగువ(1024 << xhci->context64, కొలత);
        ఒకవేళ (!dev) {
            warn_noalloc();
            వెళ్ళు fail;
        }
        పూర్ణాంకం slotid = xhci_cmd_enable_స్థానం(xhci);
        ఒకవేళ (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            విడుదల(dev);
            వెళ్ళు fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, కొలత);
        xhci->devs[slotid].ptr_దిగువ = (u32)dev;
        xhci->devs[slotid].ptr_ఎగువ = 0;

        // Send set_address command.
        పూర్ణాంకం cc = xhci_cmd_చిరునామా_device(xhci, slotid, లోపల);
        ఒకవేళ (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_స్థానం(xhci, slotid);
            ఒకవేళ (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                వెళ్ళు fail;
            }
            xhci->devs[slotid].ptr_దిగువ = 0;
            విడుదల(dev);
            వెళ్ళు fail;
        }
        pipe->slotid = slotid;
    } లేకపోతే {
        నిర్మాణం xhci_pipe *defpipe = container_యొక్క(
            usbdev->defpipe, నిర్మాణం xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        పూర్ణాంకం cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, లోపల);
        ఒకవేళ (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            వెళ్ళు fail;
        }
    }
    విడుదల(లోపల);
    తిరిగి &pipe->pipe;

fail:
    విడుదల(pipe->buf);
    విడుదల(pipe);
    విడుదల(లోపల);
    తిరిగి ఖాళీ;
}

నిర్మాణం usb_pipe *
xhci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev, నిర్మాణం usb_pipe *upipe
                  , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    ఒకవేళ (!CONFIG_USB_XHCI)
        తిరిగి ఖాళీ;
    ఒకవేళ (!epdesc) {
        usb_కూడు_freelist(upipe);
        తిరిగి ఖాళీ;
    }
    ఒకవేళ (!upipe)
        తిరిగి xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు;
    పూర్ణాంకం oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    నిర్మాణం xhci_pipe *pipe = container_యొక్క(upipe, నిర్మాణం xhci_pipe, pipe);
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        pipe->pipe.cntl, నిర్మాణం usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ఒకవేళ (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        తిరిగి upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    నిర్మాణం xhci_inctx *లోపల = xhci_alloc_inctx(usbdev, 1);
    ఒకవేళ (!లోపల)
        తిరిగి upipe;
    లోపల->కూడు = (1 << 1);
    నిర్మాణం xhci_epctx *ep = (శూన్య*)&లోపల[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    పూర్ణాంకం cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, లోపల);
    ఒకవేళ (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    విడుదల(లోపల);

    తిరిగి upipe;
}

// Submit a USB "setup" message request to the pipe's ring
స్థిర శూన్య xhci_xfer_అమరిక(నిర్మాణం xhci_pipe *pipe, పూర్ణాంకం dir, శూన్య *cmd
                            , శూన్య *దత్తాంశం, పూర్ణాంకం datalen)
{
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        pipe->pipe.cntl, నిర్మాణం usb_xhci_s, usb);
    xhci_trb_వరుస(&pipe->reqs, cmd, USB_CONTROL_అమరిక_కొలత
                   , (TR_అమరిక << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    ఒకవేళ (datalen)
        xhci_trb_వరుస(&pipe->reqs, దత్తాంశం, datalen, (TR_దత్తాంశం << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_వరుస(&pipe->reqs, ఖాళీ, 0, (TR_స్థితి << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
స్థిర శూన్య xhci_xfer_సాధారణ(నిర్మాణం xhci_pipe *pipe,
                             శూన్య *దత్తాంశం, పూర్ణాంకం datalen)
{
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        pipe->pipe.cntl, నిర్మాణం usb_xhci_s, usb);
    xhci_trb_వరుస(&pipe->reqs, దత్తాంశం, datalen, (TR_సాధారణ << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

పూర్ణాంకం
xhci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
               , శూన్య *దత్తాంశం, పూర్ణాంకం datalen)
{
    ఒకవేళ (!CONFIG_USB_XHCI)
        తిరిగి -1;
    నిర్మాణం xhci_pipe *pipe = container_యొక్క(p, నిర్మాణం xhci_pipe, pipe);
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        pipe->pipe.cntl, నిర్మాణం usb_xhci_s, usb);

    ఒకవేళ (cmd) {
        స్థిరరాశి నిర్మాణం usb_ctrlrequest *req = cmd;
        ఒకవేళ (req->bRequest == USB_REQ_సమితి_చిరునామా)
            // Set address command sent during xhci_alloc_pipe.
            తిరిగి 0;
        xhci_xfer_అమరిక(pipe, dir, (శూన్య*)req, దత్తాంశం, datalen);
    } లేకపోతే {
        xhci_xfer_సాధారణ(pipe, దత్తాంశం, datalen);
    }

    పూర్ణాంకం cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_సమయం(p, datalen));
    ఒకవేళ (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        తిరిగి -1;
    }

    తిరిగి 0;
}

పూర్ణాంకం VISIBLE32FLAT
xhci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం)
{
    ఒకవేళ (!CONFIG_USB_XHCI)
        తిరిగి -1;

    నిర్మాణం xhci_pipe *pipe = container_యొక్క(p, నిర్మాణం xhci_pipe, pipe);
    నిర్మాణం usb_xhci_s *xhci = container_యొక్క(
        pipe->pipe.cntl, నిర్మాణం usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    శూన్య *buf = pipe->buf;
    పూర్ణాంకం bufused = pipe->bufused;

    ఒకవేళ (!bufused) {
        xhci_xfer_సాధారణ(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        తిరిగి -1;
    }

    xhci_process_events(xhci);
    ఒకవేళ (xhci_ring_busy(&pipe->reqs))
        తిరిగి -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.స్థితి_2,
            pipe->reqs.evt.control,
            దత్తాంశం, buf, len);
    జ్ఞాపకం_నకలు(దత్తాంశం, buf, len);
    xhci_xfer_సాధారణ(pipe, buf, len);
    తిరిగి 0;
}
