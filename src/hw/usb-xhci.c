// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // memalign_low
#অন্তর্ভাব "memmap.h" // PAGE_SIZE
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#অন্তর্ভাব "pci_regs.h" // PCI_BASE_ADDRESS_0
#অন্তর্ভাব "string.h" // memcpy
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-xhci.h" // struct ehci_qh
#অন্তর্ভাব "util.h" // timer_calc
#অন্তর্ভাব "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_বস্তুসমূহ          16
#define XHCI_RING_আকার           (XHCI_RING_বস্তুসমূহ*আকার(কাঠামো xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((কাঠামো xhci_ring*)((u32)(_trb) & ~(XHCI_RING_আকার-1)))

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
#define XHCI_PORTSC_SPEED_নিচের    (2<<10)
#define XHCI_PORTSC_SPEED_উপরের   (3<<10)
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

প্রকারলেখ গণনা TRBType {
    TRB_RESERVED = 0,
    TR_NORMAL,
    TR_SETUP,
    TR_তথ্য,
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

প্রকারলেখ গণনা TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_তথ্য_বাফার_ERROR,
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
    CC_STOPPED_দৈর্ঘ্য_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_বাফার_OVERRUN = 31,
    CC_EVENT_LOST_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_STREAM_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_SPLIT_TRANSACTION_ERROR
} TRBCCode;

গণনা {
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

#define xhci_get_field(তথ্য, field)             \
    (((তথ্য) >> field##_SHIFT) & field##_MASK)

// --------------------------------------------------------------
// state structs

কাঠামো xhci_ring {
    কাঠামো xhci_trb      ring[XHCI_RING_বস্তুসমূহ];
    কাঠামো xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    কাঠামো mutex_s       lock;
};

কাঠামো xhci_portmap {
    u8 start;
    u8 গুনতি;
};

কাঠামো usb_xhci_s {
    কাঠামো usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    কাঠামো xhci_portmap  usb2;
    কাঠামো xhci_portmap  usb3;

    /* xhci registers */
    কাঠামো xhci_caps     *caps;
    কাঠামো xhci_op       *op;
    কাঠামো xhci_pr       *pr;
    কাঠামো xhci_ir       *ir;
    কাঠামো xhci_db       *db;

    /* xhci data structures */
    কাঠামো xhci_devlist  *devs;
    কাঠামো xhci_ring     *cmds;
    কাঠামো xhci_ring     *evts;
    কাঠামো xhci_er_seg   *eseg;
};

কাঠামো xhci_pipe {
    কাঠামো xhci_ring     reqs;

    কাঠামো usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    শূন্য                 *buf;
    পূর্ণসংখ্যা                  bufused;
};

// --------------------------------------------------------------
// tables

স্থির ধ্রুবক অক্ষর *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

স্থির ধ্রুবক পূর্ণসংখ্যা speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

স্থির ধ্রুবক পূর্ণসংখ্যা speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

স্থির পূর্ণসংখ্যা wait_bit(u32 *reg, u32 mask, পূর্ণসংখ্যা মান, u32 timeout)
{
    u32 end = timer_calc(timeout);

    যখন ((readl(reg) & mask) != মান) {
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত -1;
        }
        yield();
    }
    ফেরত 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
স্থির শূন্য
xhci_print_port_state(পূর্ণসংখ্যা loglevel, ধ্রুবক অক্ষর *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

স্থির পূর্ণসংখ্যা
xhci_hub_detect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_xhci_s *xhci = container_of(hub->cntl, কাঠামো usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ফেরত (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
স্থির পূর্ণসংখ্যা
xhci_hub_reset(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_xhci_s *xhci = container_of(hub->cntl, কাঠামো usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    যদি (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        ফেরত -1;

    নির্বাচন (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    ক্ষেত্র PLS_U0:
        // A USB3 port - controller automatically performs reset
        বিরতি;
    ক্ষেত্র PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        বিরতি;
    পূর্বনির্ধারিত:
        ফেরত -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    জন্য (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        যদি (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            ফেরত -1;
        যদি (portsc & XHCI_PORTSC_PED)
            // Reset complete
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত -1;
        }
        yield();
    }

    পূর্ণসংখ্যা rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    ফেরত rc;
}

স্থির পূর্ণসংখ্যা
xhci_hub_portmap(কাঠামো usbhub_s *hub, u32 vport)
{
    কাঠামো usb_xhci_s *xhci = container_of(hub->cntl, কাঠামো usb_xhci_s, usb);
    u32 pport = vport + 1;

    যদি (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.গুনতি)
        pport = vport + 2 - xhci->usb3.start;

    যদি (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.গুনতি)
        pport = vport + 2 - xhci->usb2.start;

    ফেরত pport;
}

স্থির শূন্য
xhci_hub_disconnect(কাঠামো usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

স্থির কাঠামো usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
স্থির পূর্ণসংখ্যা
xhci_check_ports(কাঠামো usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    কাঠামো usbhub_s hub;
    memset(&hub, 0, আকার(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    ফেরত hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

স্থির শূন্য
xhci_free_pipes(কাঠামো usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

স্থির শূন্য
configure_xhci(শূন্য *তথ্য)
{
    কাঠামো usb_xhci_s *xhci = তথ্য;
    u32 reg;

    xhci->devs = memalign_উপরের(64, আকার(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_উপরের(64, আকার(*xhci->eseg));
    xhci->cmds = memalign_উপরের(XHCI_RING_আকার, আকার(*xhci->cmds));
    xhci->evts = memalign_উপরের(XHCI_RING_আকার, আকার(*xhci->evts));
    যদি (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        যাও fail;
    }
    memset(xhci->devs, 0, আকার(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, আকার(*xhci->cmds));
    memset(xhci->evts, 0, আকার(*xhci->evts));
    memset(xhci->eseg, 0, আকার(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    যদি (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        যদি (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            যাও fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    যদি (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        যাও fail;
    যদি (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        যাও fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_নিচের, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_উপরের, 0);
    writel(&xhci->op->crcr_নিচের, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_উপরের, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_নিচের = (u32)xhci->evts;
    xhci->eseg->ptr_উপরের = 0;
    xhci->eseg->আকার_2 = XHCI_RING_বস্তুসমূহ;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_নিচের, (u32)xhci->evts);
    writel(&xhci->ir->erdp_উপরের, 0);
    writel(&xhci->ir->erstba_নিচের, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_উপরের, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    যদি (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_উপরের(64, আকার(*spba) * spb);
        শূন্য *pad = memalign_উপরের(PAGE_আকার, PAGE_আকার * spb);
        যদি (!spba || !pad) {
            warn_noalloc();
            মুক্ত(spba);
            মুক্ত(pad);
            যাও fail;
        }
        পূর্ণসংখ্যা i;
        জন্য (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_আকার);
        xhci->devs[0].ptr_নিচের = (u32)spba;
        xhci->devs[0].ptr_উপরের = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    পূর্ণসংখ্যা গুনতি = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    যদি (গুনতি)
        // Success
        ফেরত;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    মুক্ত(xhci->eseg);
    মুক্ত(xhci->evts);
    মুক্ত(xhci->cmds);
    মুক্ত(xhci->devs);
    মুক্ত(xhci);
}

স্থির কাঠামো usb_xhci_s*
xhci_controller_setup(শূন্য *baseaddr)
{
    কাঠামো usb_xhci_s *xhci = malloc_উপরের(আকার(*xhci));
    যদি (!xhci) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(xhci, 0, আকার(*xhci));
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

    যদি (xhci->xcap) {
        u32 off;
        শূন্য *addr = baseaddr + xhci->xcap;
        করো {
            কাঠামো xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            নির্বাচন (cap & 0xff) {
            ক্ষেত্র 0x02:
                name  = readl(&xcap->তথ্য[0]);
                ports = readl(&xcap->তথ্য[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 গুনতি = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , গুনতি, start
                        , ports >> 16);
                যদি (name == 0x20425355 /* "USB " */) {
                    যদি (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.গুনতি = গুনতি;
                    }
                    যদি (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.গুনতি = গুনতি;
                    }
                }
                বিরতি;
            পূর্বনির্ধারিত:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                বিরতি;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } যখন (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    যদি (PAGE_আকার != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        মুক্ত(xhci);
        ফেরত ফাঁকা;
    }

    ফেরত xhci;
}

স্থির শূন্য
xhci_controller_setup_pci(কাঠামো pci_device *pci)
{
    কাঠামো usb_xhci_s *xhci;
    শূন্য *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!baseaddr)
        ফেরত;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    যদি (!xhci)
        ফেরত;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

স্থির শূন্য
xhci_controller_setup_acpi(কাঠামো acpi_device *dev)
{
    কাঠামো usb_xhci_s *xhci;
    u64 mem, unused;
    শূন্য *baseaddr;

    যদি (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        ফেরত;
    যদি (mem >= 0x100000000ll)
        ফেরত;

    baseaddr = (শূন্য*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    যদি (!xhci)
        ফেরত;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

শূন্য
xhci_setup(শূন্য)
{
    যদি (! CONFIG_USB_XHCI)
        ফেরত;

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    কাঠামো acpi_device *dev;
    জন্য (dev = acpi_dsdt_find_eisaid(ফাঁকা, xhci_eisaid);
         dev != ফাঁকা;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
স্থির শূন্য xhci_doorbell(কাঠামো usb_xhci_s *xhci, u32 slotid, u32 মান)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, মান);
    কাঠামো xhci_db *db = xhci->db;
    শূন্য *addr = &db[slotid].doorbell;
    writel(addr, মান);
}

// Dequeue events on the XHCI command ring generated by the hardware
স্থির শূন্য xhci_process_events(কাঠামো usb_xhci_s *xhci)
{
    কাঠামো xhci_ring *evts = xhci->evts;

    জন্য (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        কাঠামো xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        যদি ((control & TRB_C) != (cs ? 1 : 0))
            ফেরত;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        নির্বাচন (evt_type) {
        ক্ষেত্র ER_TRANSFER:
        ক্ষেত্র ER_COMMAND_COMPLETE:
        {
            কাঠামো xhci_trb  *rtrb = (শূন্য*)etrb->ptr_নিচের;
            কাঠামো xhci_ring *ring = XHCI_RING(rtrb);
            কাঠামো xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, আকার(*etrb));
            ring->eidx = eidx;
            বিরতি;
        }
        ক্ষেত্র ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_নিচের >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            বিরতি;
        }
        পূর্বনির্ধারিত:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            বিরতি;
        }

        /* move ring index, notify xhci */
        nidx++;
        যদি (nidx == XHCI_RING_বস্তুসমূহ) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        কাঠামো xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_নিচের, erdp);
        writel(&ir->erdp_উপরের, 0);
    }
}

// Check if a ring has any pending TRBs
স্থির পূর্ণসংখ্যা xhci_ring_busy(কাঠামো xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    ফেরত (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
স্থির পূর্ণসংখ্যা xhci_event_wait(কাঠামো usb_xhci_s *xhci,
                           কাঠামো xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    জন্য (;;) {
        xhci_process_events(xhci);
        যদি (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            ফেরত (status >> 24) & 0xff;
        }
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
স্থির শূন্য xhci_trb_fill(কাঠামো xhci_ring *ring
                          , শূন্য *তথ্য, u32 xferlen, u32 flags)
{
    কাঠামো xhci_trb *dst = &ring->ring[ring->nidx];
    যদি (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_নিচের, তথ্য, xferlen);
    } অন্যথা {
        dst->ptr_নিচের = (u32)তথ্য;
        dst->ptr_উপরের = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
স্থির শূন্য xhci_trb_queue(কাঠামো xhci_ring *ring,
                           শূন্য *তথ্য, u32 xferlen, u32 flags)
{
    যদি (ring->nidx >= ARRAY_আকার(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, তথ্য, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
স্থির পূর্ণসংখ্যা xhci_cmd_submit(কাঠামো usb_xhci_s *xhci, কাঠামো xhci_inctx *inctx
                           , u32 flags)
{
    যদি (inctx) {
        কাঠামো xhci_slotctx *slot = (শূন্য*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        যদি (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            ফেরত -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    পূর্ণসংখ্যা rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    ফেরত rc;
}

স্থির পূর্ণসংখ্যা xhci_cmd_enable_slot(কাঠামো usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    পূর্ণসংখ্যা cc = xhci_cmd_submit(xhci, ফাঁকা, CR_ENABLE_SLOT << 10);
    যদি (cc != CC_SUCCESS)
        ফেরত -1;
    ফেরত (xhci->cmds->evt.control >> 24) & 0xff;
}

স্থির পূর্ণসংখ্যা xhci_cmd_disable_slot(কাঠামো usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ফেরত xhci_cmd_submit(xhci, ফাঁকা, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

স্থির পূর্ণসংখ্যা xhci_cmd_address_device(কাঠামো usb_xhci_s *xhci, u32 slotid
                                   , কাঠামো xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ফেরত xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

স্থির পূর্ণসংখ্যা xhci_cmd_configure_endpoint(কাঠামো usb_xhci_s *xhci, u32 slotid
                                       , কাঠামো xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ফেরত xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

স্থির পূর্ণসংখ্যা xhci_cmd_evaluate_context(কাঠামো usb_xhci_s *xhci, u32 slotid
                                     , কাঠামো xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ফেরত xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

স্থির কাঠামো xhci_inctx *
xhci_alloc_inctx(কাঠামো usbdevice_s *usbdev, পূর্ণসংখ্যা maxepid)
{
    কাঠামো usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, কাঠামো usb_xhci_s, usb);
    পূর্ণসংখ্যা আকার_2 = (আকার(কাঠামো xhci_inctx) * 33) << xhci->context64;
    কাঠামো xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, আকার_2);
    যদি (!in) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(in, 0, আকার_2);

    কাঠামো xhci_slotctx *slot = (শূন্য*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    কাঠামো usbdevice_s *hubdev = usbdev->hub->usbdev;
    যদি (hubdev) {
        যদি (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            কাঠামো xhci_pipe *hpipe = container_of(
                hubdev->defpipe, কাঠামো xhci_pipe, pipe);
            যদি (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } অন্যথা {
                কাঠামো xhci_slotctx *hslot = (শূন্য*)xhci->devs[hpipe->slotid].ptr_নিচের;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        যখন (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    ফেরত in;
}

স্থির পূর্ণসংখ্যা xhci_config_hub(কাঠামো usbhub_s *hub)
{
    কাঠামো usb_xhci_s *xhci = container_of(
        hub->cntl, কাঠামো usb_xhci_s, usb);
    কাঠামো xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, কাঠামো xhci_pipe, pipe);
    কাঠামো xhci_slotctx *hdslot = (শূন্য*)xhci->devs[pipe->slotid].ptr_নিচের;
    যদি ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        ফেরত 0;
    কাঠামো xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    যদি (!in)
        ফেরত -1;
    in->add = 0x01;
    কাঠামো xhci_slotctx *slot = (শূন্য*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    পূর্ণসংখ্যা cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    মুক্ত(in);
    যদি (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        ফেরত -1;
    }
    ফেরত 0;
}

স্থির কাঠামো usb_pipe *
xhci_alloc_pipe(কাঠামো usbdevice_s *usbdev
                , কাঠামো usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    কাঠামো usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, কাঠামো usb_xhci_s, usb);
    কাঠামো xhci_pipe *pipe;
    u32 epid;

    যদি (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } অন্যথা {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_উপরের(XHCI_RING_আকার, আকার(*pipe));
    অন্যথা
        pipe = memalign_নিচের(XHCI_RING_আকার, আকার(*pipe));
    যদি (!pipe) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(pipe, 0, আকার(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    যদি (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_উপরের(pipe->pipe.maxpacket);
        যদি (!pipe->buf) {
            warn_noalloc();
            মুক্ত(pipe);
            ফেরত ফাঁকা;
        }
    }

    // Allocate input context and initialize endpoint info.
    কাঠামো xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    যদি (!in)
        যাও fail;
    in->add = 0x01 | (1 << epid);
    কাঠামো xhci_epctx *ep = (শূন্য*)&in[(pipe->epid+1) << xhci->context64];
    যদি (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    যদি (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_নিচের  = (u32)&pipe->reqs.ring[0];
    ep->deq_নিচের  |= 1;         // dcs
    ep->দৈর্ঘ্য   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    যদি (pipe->epid == 1) {
        যদি (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            পূর্ণসংখ্যা ret = xhci_config_hub(usbdev->hub);
            যদি (ret)
                যাও fail;
        }
        // Enable slot.
        u32 আকার_2 = (আকার(কাঠামো xhci_slotctx) * 32) << xhci->context64;
        কাঠামো xhci_slotctx *dev = memalign_উপরের(1024 << xhci->context64, আকার_2);
        যদি (!dev) {
            warn_noalloc();
            যাও fail;
        }
        পূর্ণসংখ্যা slotid = xhci_cmd_enable_slot(xhci);
        যদি (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            মুক্ত(dev);
            যাও fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, আকার_2);
        xhci->devs[slotid].ptr_নিচের = (u32)dev;
        xhci->devs[slotid].ptr_উপরের = 0;

        // Send set_address command.
        পূর্ণসংখ্যা cc = xhci_cmd_address_device(xhci, slotid, in);
        যদি (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            যদি (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                যাও fail;
            }
            xhci->devs[slotid].ptr_নিচের = 0;
            মুক্ত(dev);
            যাও fail;
        }
        pipe->slotid = slotid;
    } অন্যথা {
        কাঠামো xhci_pipe *defpipe = container_of(
            usbdev->defpipe, কাঠামো xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        পূর্ণসংখ্যা cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        যদি (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            যাও fail;
        }
    }
    মুক্ত(in);
    ফেরত &pipe->pipe;

fail:
    মুক্ত(pipe->buf);
    মুক্ত(pipe);
    মুক্ত(in);
    ফেরত ফাঁকা;
}

কাঠামো usb_pipe *
xhci_realloc_pipe(কাঠামো usbdevice_s *usbdev, কাঠামো usb_pipe *upipe
                  , কাঠামো usb_endpoint_descriptor *epdesc)
{
    যদি (!CONFIG_USB_XHCI)
        ফেরত ফাঁকা;
    যদি (!epdesc) {
        usb_add_freelist(upipe);
        ফেরত ফাঁকা;
    }
    যদি (!upipe)
        ফেরত xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    পূর্ণসংখ্যা oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    কাঠামো xhci_pipe *pipe = container_of(upipe, কাঠামো xhci_pipe, pipe);
    কাঠামো usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, কাঠামো usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    যদি (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        ফেরত upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    কাঠামো xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    যদি (!in)
        ফেরত upipe;
    in->add = (1 << 1);
    কাঠামো xhci_epctx *ep = (শূন্য*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    পূর্ণসংখ্যা cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    যদি (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    মুক্ত(in);

    ফেরত upipe;
}

// Submit a USB "setup" message request to the pipe's ring
স্থির শূন্য xhci_xfer_setup(কাঠামো xhci_pipe *pipe, পূর্ণসংখ্যা dir, শূন্য *cmd
                            , শূন্য *তথ্য, পূর্ণসংখ্যা datalen)
{
    কাঠামো usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, কাঠামো usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_আকার
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    যদি (datalen)
        xhci_trb_queue(&pipe->reqs, তথ্য, datalen, (TR_তথ্য << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, ফাঁকা, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
স্থির শূন্য xhci_xfer_normal(কাঠামো xhci_pipe *pipe,
                             শূন্য *তথ্য, পূর্ণসংখ্যা datalen)
{
    কাঠামো usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, কাঠামো usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, তথ্য, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

পূর্ণসংখ্যা
xhci_send_pipe(কাঠামো usb_pipe *p, পূর্ণসংখ্যা dir, ধ্রুবক শূন্য *cmd
               , শূন্য *তথ্য, পূর্ণসংখ্যা datalen)
{
    যদি (!CONFIG_USB_XHCI)
        ফেরত -1;
    কাঠামো xhci_pipe *pipe = container_of(p, কাঠামো xhci_pipe, pipe);
    কাঠামো usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, কাঠামো usb_xhci_s, usb);

    যদি (cmd) {
        ধ্রুবক কাঠামো usb_ctrlrequest *req = cmd;
        যদি (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            ফেরত 0;
        xhci_xfer_setup(pipe, dir, (শূন্য*)req, তথ্য, datalen);
    } অন্যথা {
        xhci_xfer_normal(pipe, তথ্য, datalen);
    }

    পূর্ণসংখ্যা cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    যদি (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        ফেরত -1;
    }

    ফেরত 0;
}

পূর্ণসংখ্যা VISIBLE32FLAT
xhci_poll_intr(কাঠামো usb_pipe *p, শূন্য *তথ্য)
{
    যদি (!CONFIG_USB_XHCI)
        ফেরত -1;

    কাঠামো xhci_pipe *pipe = container_of(p, কাঠামো xhci_pipe, pipe);
    কাঠামো usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, কাঠামো usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    শূন্য *buf = pipe->buf;
    পূর্ণসংখ্যা bufused = pipe->bufused;

    যদি (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        ফেরত -1;
    }

    xhci_process_events(xhci);
    যদি (xhci_ring_busy(&pipe->reqs))
        ফেরত -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            তথ্য, buf, len);
    memcpy(তথ্য, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    ফেরত 0;
}
