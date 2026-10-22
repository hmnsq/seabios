// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // memalign_low
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SIZE
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-xhci.h" // struct ehci_qh
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc
#ଅନ୍ତର୍ଭାଵ "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_ବସ୍ତୁଗୁଡ଼ିକ          16
#define XHCI_RING_SIZE           (XHCI_RING_ବସ୍ତୁଗୁଡ଼ିକ*ଆକାର(ସଂରଚନା xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((ସଂରଚନା xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

// --------------------------------------------------------------
// bit definitions

#define XHCI_CMD_RS              (1<<0)
#define XHCI_CMD_HCRST           (1<<1)
#define XHCI_CMD_INTE            (1<<2)
#define XHCI_CMD_HSEE            (1<<3)
#define XHCI_CMD_LHCRST          (1<<7)
#define XHCI_CMD_CSS             (1<<8)
#define XHCI_CMD_CRS             (1<<9)
#define XHCI_CMD_EWE             (1<<10)
#define XHCI_CMD_EU3S            (1<<11)

#define XHCI_STS_HCH             (1<<0)
#define XHCI_STS_HSE             (1<<2)
#define XHCI_STS_EINT            (1<<3)
#define XHCI_STS_PCD             (1<<4)
#define XHCI_STS_SSS             (1<<8)
#define XHCI_STS_RSS             (1<<9)
#define XHCI_STS_SRE             (1<<10)
#define XHCI_STS_CNR             (1<<11)
#define XHCI_STS_HCE             (1<<12)

#define XHCI_PORTSC_CCS          (1<<0)
#define XHCI_PORTSC_PED          (1<<1)
#define XHCI_PORTSC_OCA          (1<<3)
#define XHCI_PORTSC_PR           (1<<4)
#define XHCI_PORTSC_PLS_SHIFT        5
#define XHCI_PORTSC_PLS_MASK     0xf
#define XHCI_PORTSC_PP           (1<<9)
#define XHCI_PORTSC_SPEED_SHIFT      10
#define XHCI_PORTSC_SPEED_MASK   0xf
#define XHCI_PORTSC_SPEED_FULL   (1<<10)
#define XHCI_PORTSC_SPEED_ତଳ    (2<<10)
#define XHCI_PORTSC_SPEED_ଉପର   (3<<10)
#define XHCI_PORTSC_SPEED_SUPER  (4<<10)
#define XHCI_PORTSC_PIC_SHIFT        14
#define XHCI_PORTSC_PIC_MASK     0x3
#define XHCI_PORTSC_LWS          (1<<16)
#define XHCI_PORTSC_CSC          (1<<17)
#define XHCI_PORTSC_PEC          (1<<18)
#define XHCI_PORTSC_WRC          (1<<19)
#define XHCI_PORTSC_OCC          (1<<20)
#define XHCI_PORTSC_PRC          (1<<21)
#define XHCI_PORTSC_PLC          (1<<22)
#define XHCI_PORTSC_CEC          (1<<23)
#define XHCI_PORTSC_CAS          (1<<24)
#define XHCI_PORTSC_WCE          (1<<25)
#define XHCI_PORTSC_WDE          (1<<26)
#define XHCI_PORTSC_WOE          (1<<27)
#define XHCI_PORTSC_DR           (1<<30)
#define XHCI_PORTSC_WPR          (1<<31)

#define TRB_C               (1<<0)
#define TRB_TYPE_SHIFT          10
#define TRB_TYPE_MASK       0x3f
#define TRB_TYPE(t)         (((t) >> TRB_TYPE_SHIFT) & TRB_TYPE_MASK)

#define TRB_EV_ED           (1<<2)

#define TRB_TR_ENT          (1<<1)
#define TRB_TR_ISP          (1<<2)
#define TRB_TR_NS           (1<<3)
#define TRB_TR_CH           (1<<4)
#define TRB_TR_IOC          (1<<5)
#define TRB_TR_IDT          (1<<6)
#define TRB_TR_TBC_SHIFT        7
#define TRB_TR_TBC_MASK     0x3
#define TRB_TR_BEI          (1<<9)
#define TRB_TR_TLBPC_SHIFT      16
#define TRB_TR_TLBPC_MASK   0xf
#define TRB_TR_FRAMEID_SHIFT    20
#define TRB_TR_FRAMEID_MASK 0x7ff
#define TRB_TR_SIA          (1<<31)

#define TRB_TR_DIR          (1<<16)

#define TRB_CR_SLOTID_SHIFT     24
#define TRB_CR_SLOTID_MASK  0xff
#define TRB_CR_EPID_SHIFT       16
#define TRB_CR_EPID_MASK    0x1f

#define TRB_CR_BSR          (1<<9)
#define TRB_CR_DC           (1<<9)

#define TRB_LK_TC           (1<<1)

#define TRB_INTR_SHIFT          22
#define TRB_INTR_MASK       0x3ff
#define TRB_INTR(t)         (((t).status >> TRB_INTR_SHIFT) & TRB_INTR_MASK)

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା TRBType {
    TRB_RESERVED = 0,
    TR_NORMAL,
    TR_SETUP,
    TR_DATA,
    TR_STATUS,
    TR_ISOCH,
    TR_LINK,
    TR_EVDATA,
    TR_NOOP,
    CR_ENABLE_SLOT,
    CR_DISABLE_SLOT,
    CR_ADDRESS_DEVICE,
    CR_CONFIGURE_ENDPOINT,
    CR_EVALUATE_CONTEXT,
    CR_RESET_ENDPOINT,
    CR_STOP_ENDPOINT,
    CR_SET_TR_DEQUEUE,
    CR_RESET_DEVICE,
    CR_FORCE_EVENT,
    CR_NEGOTIATE_BW,
    CR_SET_LATENCY_TOLERANCE,
    CR_GET_PORT_BANDWIDTH,
    CR_FORCE_HEADER,
    CR_NOOP,
    ER_TRANSFER = 32,
    ER_COMMAND_COMPLETE,
    ER_PORT_STATUS_CHANGE,
    ER_BANDWIDTH_REQUEST,
    ER_DOORBELL,
    ER_HOST_CONTROLLER,
    ER_DEVICE_NOTIFICATION,
    ER_MFINDEX_WRAP,
} TRBType;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_DATA_BUFFER_ERROR,
    CC_BABBLE_DETECTED,
    CC_USB_TRANSACTION_ERROR,
    CC_TRB_ERROR,
    CC_STALL_ERROR,
    CC_RESOURCE_ERROR,
    CC_BANDWIDTH_ERROR,
    CC_NO_SLOTS_ERROR,
    CC_INVALID_STREAM_TYPE_ERROR,
    CC_SLOT_NOT_ENABLED_ERROR,
    CC_EP_NOT_ENABLED_ERROR,
    CC_SHORT_PACKET,
    CC_RING_UNDERRUN,
    CC_RING_OVERRUN,
    CC_VF_ER_FULL,
    CC_PARAMETER_ERROR,
    CC_BANDWIDTH_OVERRUN,
    CC_CONTEXT_STATE_ERROR,
    CC_NO_PING_RESPONSE_ERROR,
    CC_EVENT_RING_FULL_ERROR,
    CC_INCOMPATIBLE_DEVICE_ERROR,
    CC_MISSED_SERVICE_ERROR,
    CC_COMMAND_RING_STOPPED,
    CC_COMMAND_ABORTED,
    CC_STOPPED,
    CC_STOPPED_LENGTH_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_BUFFER_OVERRUN = 31,
    CC_EVENT_LOST_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_STREAM_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_SPLIT_TRANSACTION_ERROR
} TRBCCode;

ଗଣନା {
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
    PLS_COMPILANCE_MODE = 10,
    PLS_TEST_MODE       = 11,
    PLS_RESUME          = 15,
};

#define xhci_get_field(data, field)             \
    (((data) >> field##_SHIFT) & field##_MASK)

// --------------------------------------------------------------
// state structs

ସଂରଚନା xhci_ring {
    ସଂରଚନା xhci_trb      ring[XHCI_RING_ବସ୍ତୁଗୁଡ଼ିକ];
    ସଂରଚନା xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    ସଂରଚନା mutex_s       lock;
};

ସଂରଚନା xhci_portmap {
    u8 start;
    u8 ଗଣନା_2;
};

ସଂରଚନା usb_xhci_s {
    ସଂରଚନା usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    ସଂରଚନା xhci_portmap  usb2;
    ସଂରଚନା xhci_portmap  usb3;

    /* xhci registers */
    ସଂରଚନା xhci_caps     *caps;
    ସଂରଚନା xhci_op       *op;
    ସଂରଚନା xhci_pr       *pr;
    ସଂରଚନା xhci_ir       *ir;
    ସଂରଚନା xhci_db       *db;

    /* xhci data structures */
    ସଂରଚନା xhci_devlist  *devs;
    ସଂରଚନା xhci_ring     *cmds;
    ସଂରଚନା xhci_ring     *evts;
    ସଂରଚନା xhci_er_seg   *eseg;
};

ସଂରଚନା xhci_pipe {
    ସଂରଚନା xhci_ring     reqs;

    ସଂରଚନା usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    ଶୂନ୍ୟ                 *buf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା                  bufused;
};

// --------------------------------------------------------------
// tables

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wait_bit(u32 *reg, u32 mask, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ, u32 timeout)
{
    u32 end = timer_calc(timeout);

    ଯେତେବେଳେ ((readl(reg) & mask) != ମୂଲ୍ୟ) {
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
    ଫେରନ୍ତୁ 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
ସ୍ଥିର ଶୂନ୍ୟ
xhci_print_port_state(ପୂର୍ଣ୍ଣସଂଖ୍ୟା loglevel, ଅଚଳ ଅକ୍ଷର *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xhci_hub_detect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_xhci_s *xhci = container_of(hub->cntl, ସଂରଚନା usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ଫେରନ୍ତୁ (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xhci_hub_reset(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_xhci_s *xhci = container_of(hub->cntl, ସଂରଚନା usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ଯଦି (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        ଫେରନ୍ତୁ -1;

    ବାଛ (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    ସ୍ଥିତି PLS_U0:
        // A USB3 port - controller automatically performs reset
        ଭାଙ୍ଗ;
    ସ୍ଥିତି PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    ପାଇଁ (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        ଯଦି (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            ଫେରନ୍ତୁ -1;
        ଯଦି (portsc & XHCI_PORTSC_PED)
            // Reset complete
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xhci_hub_portmap(ସଂରଚନା usbhub_s *hub, u32 vport)
{
    ସଂରଚନା usb_xhci_s *xhci = container_of(hub->cntl, ସଂରଚନା usb_xhci_s, usb);
    u32 pport = vport + 1;

    ଯଦି (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.ଗଣନା_2)
        pport = vport + 2 - xhci->usb3.start;

    ଯଦି (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.ଗଣନା_2)
        pport = vport + 2 - xhci->usb2.start;

    ଫେରନ୍ତୁ pport;
}

ସ୍ଥିର ଶୂନ୍ୟ
xhci_hub_disconnect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

ସ୍ଥିର ସଂରଚନା usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xhci_check_ports(ସଂରଚନା usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    ସଂରଚନା usbhub_s hub;
    memset(&hub, 0, ଆକାର(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    ଫେରନ୍ତୁ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

ସ୍ଥିର ଶୂନ୍ୟ
xhci_free_pipes(ସଂରଚନା usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

ସ୍ଥିର ଶୂନ୍ୟ
configure_xhci(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_ଉପର(64, ଆକାର(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ଉପର(64, ଆକାର(*xhci->eseg));
    xhci->cmds = memalign_ଉପର(XHCI_RING_SIZE, ଆକାର(*xhci->cmds));
    xhci->evts = memalign_ଉପର(XHCI_RING_SIZE, ଆକାର(*xhci->evts));
    ଯଦି (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        ଯାଅ fail;
    }
    memset(xhci->devs, 0, ଆକାର(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, ଆକାର(*xhci->cmds));
    memset(xhci->evts, 0, ଆକାର(*xhci->evts));
    memset(xhci->eseg, 0, ଆକାର(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    ଯଦି (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        ଯଦି (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            ଯାଅ fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    ଯଦି (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        ଯାଅ fail;
    ଯଦି (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        ଯାଅ fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_ତଳ, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ଉପର, 0);
    writel(&xhci->op->crcr_ତଳ, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ଉପର, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_ତଳ = (u32)xhci->evts;
    xhci->eseg->ptr_ଉପର = 0;
    xhci->eseg->size = XHCI_RING_ବସ୍ତୁଗୁଡ଼ିକ;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_ତଳ, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ଉପର, 0);
    writel(&xhci->ir->erstba_ତଳ, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ଉପର, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    ଯଦି (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ଉପର(64, ଆକାର(*spba) * spb);
        ଶୂନ୍ୟ *pad = memalign_ଉପର(PAGE_SIZE, PAGE_SIZE * spb);
        ଯଦି (!spba || !pad) {
            warn_noalloc();
            ମୁକ୍ତ(spba);
            ମୁକ୍ତ(pad);
            ଯାଅ fail;
        }
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_ତଳ = (u32)spba;
        xhci->devs[0].ptr_ଉପର = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    ଯଦି (ଗଣନା_2)
        // Success
        ଫେରନ୍ତୁ;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    ମୁକ୍ତ(xhci->eseg);
    ମୁକ୍ତ(xhci->evts);
    ମୁକ୍ତ(xhci->cmds);
    ମୁକ୍ତ(xhci->devs);
    ମୁକ୍ତ(xhci);
}

ସ୍ଥିର ସଂରଚନା usb_xhci_s*
xhci_controller_setup(ଶୂନ୍ୟ *baseaddr)
{
    ସଂରଚନା usb_xhci_s *xhci = malloc_ଉପର(ଆକାର(*xhci));
    ଯଦି (!xhci) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(xhci, 0, ଆକାର(*xhci));
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

    ଯଦି (xhci->xcap) {
        u32 off;
        ଶୂନ୍ୟ *addr = baseaddr + xhci->xcap;
        କର {
            ସଂରଚନା xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            ବାଛ (cap & 0xff) {
            ସ୍ଥିତି 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 ଗଣନା_2 = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , ଗଣନା_2, start
                        , ports >> 16);
                ଯଦି (name == 0x20425355 /* "USB " */) {
                    ଯଦି (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.ଗଣନା_2 = ଗଣନା_2;
                    }
                    ଯଦି (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.ଗଣନା_2 = ଗଣନା_2;
                    }
                }
                ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                ଭାଙ୍ଗ;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } ଯେତେବେଳେ (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    ଯଦି (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        ମୁକ୍ତ(xhci);
        ଫେରନ୍ତୁ ଖାଲି;
    }

    ଫେରନ୍ତୁ xhci;
}

ସ୍ଥିର ଶୂନ୍ୟ
xhci_controller_setup_pci(ସଂରଚନା pci_device *pci)
{
    ସଂରଚନା usb_xhci_s *xhci;
    ଶୂନ୍ୟ *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ଯଦି (!baseaddr)
        ଫେରନ୍ତୁ;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    ଯଦି (!xhci)
        ଫେରନ୍ତୁ;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

ସ୍ଥିର ଶୂନ୍ୟ
xhci_controller_setup_acpi(ସଂରଚନା acpi_device *dev)
{
    ସଂରଚନା usb_xhci_s *xhci;
    u64 mem, unused;
    ଶୂନ୍ୟ *baseaddr;

    ଯଦି (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        ଫେରନ୍ତୁ;
    ଯଦି (mem >= 0x100000000ll)
        ଫେରନ୍ତୁ;

    baseaddr = (ଶୂନ୍ୟ*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    ଯଦି (!xhci)
        ଫେରନ୍ତୁ;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

ଶୂନ୍ୟ
xhci_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_USB_XHCI)
        ଫେରନ୍ତୁ;

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    ସଂରଚନା acpi_device *dev;
    ପାଇଁ (dev = acpi_dsdt_find_eisaid(ଖାଲି, xhci_eisaid);
         dev != ଖାଲି;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ସ୍ଥିର ଶୂନ୍ୟ xhci_doorbell(ସଂରଚନା usb_xhci_s *xhci, u32 slotid, u32 ମୂଲ୍ୟ)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, ମୂଲ୍ୟ);
    ସଂରଚନା xhci_db *db = xhci->db;
    ଶୂନ୍ୟ *addr = &db[slotid].doorbell;
    writel(addr, ମୂଲ୍ୟ);
}

// Dequeue events on the XHCI command ring generated by the hardware
ସ୍ଥିର ଶୂନ୍ୟ xhci_process_events(ସଂରଚନା usb_xhci_s *xhci)
{
    ସଂରଚନା xhci_ring *evts = xhci->evts;

    ପାଇଁ (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        ସଂରଚନା xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        ଯଦି ((control & TRB_C) != (cs ? 1 : 0))
            ଫେରନ୍ତୁ;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        ବାଛ (evt_type) {
        ସ୍ଥିତି ER_TRANSFER:
        ସ୍ଥିତି ER_COMMAND_COMPLETE:
        {
            ସଂରଚନା xhci_trb  *rtrb = (ଶୂନ୍ୟ*)etrb->ptr_ତଳ;
            ସଂରଚନା xhci_ring *ring = XHCI_RING(rtrb);
            ସଂରଚନା xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, ଆକାର(*etrb));
            ring->eidx = eidx;
            ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_ତଳ >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            ଭାଙ୍ଗ;
        }
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            ଭାଙ୍ଗ;
        }

        /* move ring index, notify xhci */
        nidx++;
        ଯଦି (nidx == XHCI_RING_ବସ୍ତୁଗୁଡ଼ିକ) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        ସଂରଚନା xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_ତଳ, erdp);
        writel(&ir->erdp_ଉପର, 0);
    }
}

// Check if a ring has any pending TRBs
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_ring_busy(ସଂରଚନା xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    ଫେରନ୍ତୁ (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_event_wait(ସଂରଚନା usb_xhci_s *xhci,
                           ସଂରଚନା xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    ପାଇଁ (;;) {
        xhci_process_events(xhci);
        ଯଦି (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            ଫେରନ୍ତୁ (status >> 24) & 0xff;
        }
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ସ୍ଥିର ଶୂନ୍ୟ xhci_trb_fill(ସଂରଚନା xhci_ring *ring
                          , ଶୂନ୍ୟ *data, u32 xferlen, u32 flags)
{
    ସଂରଚନା xhci_trb *dst = &ring->ring[ring->nidx];
    ଯଦି (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_ତଳ, data, xferlen);
    } ଅନ୍ୟଥା {
        dst->ptr_ତଳ = (u32)data;
        dst->ptr_ଉପର = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ସ୍ଥିର ଶୂନ୍ୟ xhci_trb_queue(ସଂରଚନା xhci_ring *ring,
                           ଶୂନ୍ୟ *data, u32 xferlen, u32 flags)
{
    ଯଦି (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, data, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_cmd_submit(ସଂରଚନା usb_xhci_s *xhci, ସଂରଚନା xhci_inctx *inctx
                           , u32 flags)
{
    ଯଦି (inctx) {
        ସଂରଚନା xhci_slotctx *slot = (ଶୂନ୍ୟ*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        ଯଦି (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            ଫେରନ୍ତୁ -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_cmd_enable_slot(ସଂରଚନା usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cc = xhci_cmd_submit(xhci, ଖାଲି, CR_ENABLE_SLOT << 10);
    ଯଦି (cc != CC_SUCCESS)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ (xhci->cmds->evt.control >> 24) & 0xff;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_cmd_disable_slot(ସଂରଚନା usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ଫେରନ୍ତୁ xhci_cmd_submit(xhci, ଖାଲି, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_cmd_address_device(ସଂରଚନା usb_xhci_s *xhci, u32 slotid
                                   , ସଂରଚନା xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ଫେରନ୍ତୁ xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_cmd_configure_endpoint(ସଂରଚନା usb_xhci_s *xhci, u32 slotid
                                       , ସଂରଚନା xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ଫେରନ୍ତୁ xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_cmd_evaluate_context(ସଂରଚନା usb_xhci_s *xhci, u32 slotid
                                     , ସଂରଚନା xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ଫେରନ୍ତୁ xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ସ୍ଥିର ସଂରଚନା xhci_inctx *
xhci_alloc_inctx(ସଂରଚନା usbdevice_s *usbdev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxepid)
{
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_xhci_s, usb);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = (ଆକାର(ସଂରଚନା xhci_inctx) * 33) << xhci->context64;
    ସଂରଚନା xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    ଯଦି (!in) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(in, 0, size);

    ସଂରଚନା xhci_slotctx *slot = (ଶୂନ୍ୟ*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    ସଂରଚନା usbdevice_s *hubdev = usbdev->hub->usbdev;
    ଯଦି (hubdev) {
        ଯଦି (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            ସଂରଚନା xhci_pipe *hpipe = container_of(
                hubdev->defpipe, ସଂରଚନା xhci_pipe, pipe);
            ଯଦି (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } ଅନ୍ୟଥା {
                ସଂରଚନା xhci_slotctx *hslot = (ଶୂନ୍ୟ*)xhci->devs[hpipe->slotid].ptr_ତଳ;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        ଯେତେବେଳେ (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    ଫେରନ୍ତୁ in;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xhci_config_hub(ସଂରଚନା usbhub_s *hub)
{
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        hub->cntl, ସଂରଚନା usb_xhci_s, usb);
    ସଂରଚନା xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, ସଂରଚନା xhci_pipe, pipe);
    ସଂରଚନା xhci_slotctx *hdslot = (ଶୂନ୍ୟ*)xhci->devs[pipe->slotid].ptr_ତଳ;
    ଯଦି ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        ଫେରନ୍ତୁ 0;
    ସଂରଚନା xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    ଯଦି (!in)
        ଫେରନ୍ତୁ -1;
    in->add = 0x01;
    ସଂରଚନା xhci_slotctx *slot = (ଶୂନ୍ୟ*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    ମୁକ୍ତ(in);
    ଯଦି (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ସଂରଚନା usb_pipe *
xhci_alloc_pipe(ସଂରଚନା usbdevice_s *usbdev
                , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_xhci_s, usb);
    ସଂରଚନା xhci_pipe *pipe;
    u32 epid;

    ଯଦି (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } ଅନ୍ୟଥା {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    ଯଦି (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ଉପର(XHCI_RING_SIZE, ଆକାର(*pipe));
    ଅନ୍ୟଥା
        pipe = memalign_ତଳ(XHCI_RING_SIZE, ଆକାର(*pipe));
    ଯଦି (!pipe) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(pipe, 0, ଆକାର(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    ଯଦି (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ଉପର(pipe->pipe.maxpacket);
        ଯଦି (!pipe->buf) {
            warn_noalloc();
            ମୁକ୍ତ(pipe);
            ଫେରନ୍ତୁ ଖାଲି;
        }
    }

    // Allocate input context and initialize endpoint info.
    ସଂରଚନା xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    ଯଦି (!in)
        ଯାଅ fail;
    in->add = 0x01 | (1 << epid);
    ସଂରଚନା xhci_epctx *ep = (ଶୂନ୍ୟ*)&in[(pipe->epid+1) << xhci->context64];
    ଯଦି (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    ଯଦି (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_ତଳ  = (u32)&pipe->reqs.ring[0];
    ep->deq_ତଳ  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ଯଦି (pipe->epid == 1) {
        ଯଦି (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = xhci_config_hub(usbdev->hub);
            ଯଦି (ret)
                ଯାଅ fail;
        }
        // Enable slot.
        u32 size = (ଆକାର(ସଂରଚନା xhci_slotctx) * 32) << xhci->context64;
        ସଂରଚନା xhci_slotctx *dev = memalign_ଉପର(1024 << xhci->context64, size);
        ଯଦି (!dev) {
            warn_noalloc();
            ଯାଅ fail;
        }
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା slotid = xhci_cmd_enable_slot(xhci);
        ଯଦି (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            ମୁକ୍ତ(dev);
            ଯାଅ fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_ତଳ = (u32)dev;
        xhci->devs[slotid].ptr_ଉପର = 0;

        // Send set_address command.
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା cc = xhci_cmd_address_device(xhci, slotid, in);
        ଯଦି (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            ଯଦି (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                ଯାଅ fail;
            }
            xhci->devs[slotid].ptr_ତଳ = 0;
            ମୁକ୍ତ(dev);
            ଯାଅ fail;
        }
        pipe->slotid = slotid;
    } ଅନ୍ୟଥା {
        ସଂରଚନା xhci_pipe *defpipe = container_of(
            usbdev->defpipe, ସଂରଚନା xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        ଯଦି (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            ଯାଅ fail;
        }
    }
    ମୁକ୍ତ(in);
    ଫେରନ୍ତୁ &pipe->pipe;

fail:
    ମୁକ୍ତ(pipe->buf);
    ମୁକ୍ତ(pipe);
    ମୁକ୍ତ(in);
    ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା usb_pipe *
xhci_realloc_pipe(ସଂରଚନା usbdevice_s *usbdev, ସଂରଚନା usb_pipe *upipe
                  , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ଯଦି (!CONFIG_USB_XHCI)
        ଫେରନ୍ତୁ ଖାଲି;
    ଯଦି (!epdesc) {
        usb_add_freelist(upipe);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ଯଦି (!upipe)
        ଫେରନ୍ତୁ xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    ସଂରଚନା xhci_pipe *pipe = container_of(upipe, ସଂରଚନା xhci_pipe, pipe);
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ସଂରଚନା usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ଯଦି (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        ଫେରନ୍ତୁ upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    ସଂରଚନା xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    ଯଦି (!in)
        ଫେରନ୍ତୁ upipe;
    in->add = (1 << 1);
    ସଂରଚନା xhci_epctx *ep = (ଶୂନ୍ୟ*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    ଯଦି (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    ମୁକ୍ତ(in);

    ଫେରନ୍ତୁ upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ସ୍ଥିର ଶୂନ୍ୟ xhci_xfer_setup(ସଂରଚନା xhci_pipe *pipe, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଶୂନ୍ୟ *cmd
                            , ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datalen)
{
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ସଂରଚନା usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    ଯଦି (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, ଖାଲି, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ସ୍ଥିର ଶୂନ୍ୟ xhci_xfer_normal(ସଂରଚନା xhci_pipe *pipe,
                             ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datalen)
{
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ସଂରଚନା usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xhci_send_pipe(ସଂରଚନା usb_pipe *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଅଚଳ ଶୂନ୍ୟ *cmd
               , ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datalen)
{
    ଯଦି (!CONFIG_USB_XHCI)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା xhci_pipe *pipe = container_of(p, ସଂରଚନା xhci_pipe, pipe);
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ସଂରଚନା usb_xhci_s, usb);

    ଯଦି (cmd) {
        ଅଚଳ ସଂରଚନା usb_ctrlrequest *req = cmd;
        ଯଦି (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            ଫେରନ୍ତୁ 0;
        xhci_xfer_setup(pipe, dir, (ଶୂନ୍ୟ*)req, data, datalen);
    } ଅନ୍ୟଥା {
        xhci_xfer_normal(pipe, data, datalen);
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    ଯଦି (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        ଫେରନ୍ତୁ -1;
    }

    ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା VISIBLE32FLAT
xhci_poll_intr(ସଂରଚନା usb_pipe *p, ଶୂନ୍ୟ *data)
{
    ଯଦି (!CONFIG_USB_XHCI)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା xhci_pipe *pipe = container_of(p, ସଂରଚନା xhci_pipe, pipe);
    ସଂରଚନା usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ସଂରଚନା usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    ଶୂନ୍ୟ *buf = pipe->buf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bufused = pipe->bufused;

    ଯଦି (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        ଫେରନ୍ତୁ -1;
    }

    xhci_process_events(xhci);
    ଯଦି (xhci_ring_busy(&pipe->reqs))
        ଫେରନ୍ତୁ -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    ଫେରନ୍ତୁ 0;
}
