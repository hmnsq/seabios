// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // memalign_low
#ضمن "memmap.h" // PAGE_SIZE
#ضمن "output.h" // dprintf
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_0
#ضمن "string.h" // memcpy
#ضمن "usb.h" // struct usb_s
#ضمن "usb-xhci.h" // struct ehci_qh
#ضمن "util.h" // timer_calc
#ضمن "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_ITEMS          16
#define XHCI_RING_SIZE           (XHCI_RING_ITEMS*حجم(بنية xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((بنية xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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
#define XHCI_PORTSC_SPEED_LOW    (2<<10)
#define XHCI_PORTSC_SPEED_HIGH   (3<<10)
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

تعريفنوع تعداد TRBType {
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

تعريفنوع تعداد TRBCCode {
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

تعداد {
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

بنية xhci_ring {
    بنية xhci_trb      ring[XHCI_RING_ITEMS];
    بنية xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    بنية mutex_s       lock;
};

بنية xhci_portmap {
    u8 start;
    u8 count;
};

بنية usb_xhci_s {
    بنية usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    بنية xhci_portmap  usb2;
    بنية xhci_portmap  usb3;

    /* xhci registers */
    بنية xhci_caps     *caps;
    بنية xhci_op       *op;
    بنية xhci_pr       *pr;
    بنية xhci_ir       *ir;
    بنية xhci_db       *db;

    /* xhci data structures */
    بنية xhci_devlist  *devs;
    بنية xhci_ring     *cmds;
    بنية xhci_ring     *evts;
    بنية xhci_er_seg   *eseg;
};

بنية xhci_pipe {
    بنية xhci_ring     reqs;

    بنية usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    فراغ                 *buf;
    صحيح                  bufused;
};

// --------------------------------------------------------------
// tables

ساكن ثابت حرف *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ساكن ثابت صحيح speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ساكن ثابت صحيح speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ساكن صحيح wait_bit(u32 *reg, u32 mask, صحيح value, u32 timeout)
{
    u32 end = timer_calc(timeout);

    ܟܡܐ ((readl(reg) & mask) != value) {
        ܐܢ (timer_check(end)) {
            warn_timeout();
            ܦܢܐ -1;
        }
        yield();
    }
    ܦܢܐ 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
ساكن فراغ
xhci_print_port_state(صحيح loglevel, ثابت حرف *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

ساكن صحيح
xhci_hub_detect(بنية usbhub_s *hub, u32 port)
{
    بنية usb_xhci_s *xhci = container_of(hub->cntl, بنية usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ܦܢܐ (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ساكن صحيح
xhci_hub_reset(بنية usbhub_s *hub, u32 port)
{
    بنية usb_xhci_s *xhci = container_of(hub->cntl, بنية usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ܐܢ (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        ܦܢܐ -1;

    بدل (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    حالة PLS_U0:
        // A USB3 port - controller automatically performs reset
        اكسر;
    حالة PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        اكسر;
    افتراضي:
        ܦܢܐ -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    لكل (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        ܐܢ (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            ܦܢܐ -1;
        ܐܢ (portsc & XHCI_PORTSC_PED)
            // Reset complete
            اكسر;
        ܐܢ (timer_check(end)) {
            warn_timeout();
            ܦܢܐ -1;
        }
        yield();
    }

    صحيح rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    ܦܢܐ rc;
}

ساكن صحيح
xhci_hub_portmap(بنية usbhub_s *hub, u32 vport)
{
    بنية usb_xhci_s *xhci = container_of(hub->cntl, بنية usb_xhci_s, usb);
    u32 pport = vport + 1;

    ܐܢ (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.count)
        pport = vport + 2 - xhci->usb3.start;

    ܐܢ (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.count)
        pport = vport + 2 - xhci->usb2.start;

    ܦܢܐ pport;
}

ساكن فراغ
xhci_hub_disconnect(بنية usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

ساكن بنية usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساكن صحيح
xhci_check_ports(بنية usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    ܦܢܐ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

ساكن فراغ
xhci_free_pipes(بنية usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

ساكن فراغ
configure_xhci(فراغ *data)
{
    بنية usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_high(64, حجم(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_high(64, حجم(*xhci->eseg));
    xhci->cmds = memalign_high(XHCI_RING_SIZE, حجم(*xhci->cmds));
    xhci->evts = memalign_high(XHCI_RING_SIZE, حجم(*xhci->evts));
    ܐܢ (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        اذهب fail;
    }
    memset(xhci->devs, 0, حجم(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, حجم(*xhci->cmds));
    memset(xhci->evts, 0, حجم(*xhci->evts));
    memset(xhci->eseg, 0, حجم(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    ܐܢ (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        ܐܢ (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            اذهب fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    ܐܢ (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        اذهب fail;
    ܐܢ (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        اذهب fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_low, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_high, 0);
    writel(&xhci->op->crcr_low, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_high, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_low = (u32)xhci->evts;
    xhci->eseg->ptr_high = 0;
    xhci->eseg->size = XHCI_RING_ITEMS;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_low, (u32)xhci->evts);
    writel(&xhci->ir->erdp_high, 0);
    writel(&xhci->ir->erstba_low, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_high, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    ܐܢ (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_high(64, حجم(*spba) * spb);
        فراغ *pad = memalign_high(PAGE_SIZE, PAGE_SIZE * spb);
        ܐܢ (!spba || !pad) {
            warn_noalloc();
            حرر(spba);
            حرر(pad);
            اذهب fail;
        }
        صحيح i;
        لكل (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_low = (u32)spba;
        xhci->devs[0].ptr_high = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    صحيح count = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    ܐܢ (count)
        // Success
        ܦܢܐ;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    حرر(xhci->eseg);
    حرر(xhci->evts);
    حرر(xhci->cmds);
    حرر(xhci->devs);
    حرر(xhci);
}

ساكن بنية usb_xhci_s*
xhci_controller_setup(فراغ *baseaddr)
{
    بنية usb_xhci_s *xhci = malloc_high(حجم(*xhci));
    ܐܢ (!xhci) {
        warn_noalloc();
        ܦܢܐ عدم;
    }
    memset(xhci, 0, حجم(*xhci));
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

    ܐܢ (xhci->xcap) {
        u32 off;
        فراغ *addr = baseaddr + xhci->xcap;
        ܥܒܕ {
            بنية xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            بدل (cap & 0xff) {
            حالة 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 count = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , count, start
                        , ports >> 16);
                ܐܢ (name == 0x20425355 /* "USB " */) {
                    ܐܢ (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.count = count;
                    }
                    ܐܢ (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.count = count;
                    }
                }
                اكسر;
            افتراضي:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                اكسر;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } ܟܡܐ (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    ܐܢ (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        حرر(xhci);
        ܦܢܐ عدم;
    }

    ܦܢܐ xhci;
}

ساكن فراغ
xhci_controller_setup_pci(بنية pci_device *pci)
{
    بنية usb_xhci_s *xhci;
    فراغ *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ܐܢ (!baseaddr)
        ܦܢܐ;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    ܐܢ (!xhci)
        ܦܢܐ;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

ساكن فراغ
xhci_controller_setup_acpi(بنية acpi_device *dev)
{
    بنية usb_xhci_s *xhci;
    u64 mem, unused;
    فراغ *baseaddr;

    ܐܢ (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        ܦܢܐ;
    ܐܢ (mem >= 0x100000000ll)
        ܦܢܐ;

    baseaddr = (فراغ*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    ܐܢ (!xhci)
        ܦܢܐ;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

فراغ
xhci_setup(فراغ)
{
    ܐܢ (! CONFIG_USB_XHCI)
        ܦܢܐ;

    بنية pci_device *pci;
    foreachpci(pci) {
        ܐܢ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    بنية acpi_device *dev;
    لكل (dev = acpi_dsdt_find_eisaid(عدم, xhci_eisaid);
         dev != عدم;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ساكن فراغ xhci_doorbell(بنية usb_xhci_s *xhci, u32 slotid, u32 value)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, value);
    بنية xhci_db *db = xhci->db;
    فراغ *addr = &db[slotid].doorbell;
    writel(addr, value);
}

// Dequeue events on the XHCI command ring generated by the hardware
ساكن فراغ xhci_process_events(بنية usb_xhci_s *xhci)
{
    بنية xhci_ring *evts = xhci->evts;

    لكل (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        بنية xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        ܐܢ ((control & TRB_C) != (cs ? 1 : 0))
            ܦܢܐ;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        بدل (evt_type) {
        حالة ER_TRANSFER:
        حالة ER_COMMAND_COMPLETE:
        {
            بنية xhci_trb  *rtrb = (فراغ*)etrb->ptr_low;
            بنية xhci_ring *ring = XHCI_RING(rtrb);
            بنية xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, حجم(*etrb));
            ring->eidx = eidx;
            اكسر;
        }
        حالة ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_low >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            اكسر;
        }
        افتراضي:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            اكسر;
        }

        /* move ring index, notify xhci */
        nidx++;
        ܐܢ (nidx == XHCI_RING_ITEMS) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        بنية xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_low, erdp);
        writel(&ir->erdp_high, 0);
    }
}

// Check if a ring has any pending TRBs
ساكن صحيح xhci_ring_busy(بنية xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    ܦܢܐ (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ساكن صحيح xhci_event_wait(بنية usb_xhci_s *xhci,
                           بنية xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    لكل (;;) {
        xhci_process_events(xhci);
        ܐܢ (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            ܦܢܐ (status >> 24) & 0xff;
        }
        ܐܢ (timer_check(end)) {
            warn_timeout();
            ܦܢܐ -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ساكن فراغ xhci_trb_fill(بنية xhci_ring *ring
                          , فراغ *data, u32 xferlen, u32 flags)
{
    بنية xhci_trb *dst = &ring->ring[ring->nidx];
    ܐܢ (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_low, data, xferlen);
    } ܐܚܪܢܐ {
        dst->ptr_low = (u32)data;
        dst->ptr_high = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ساكن فراغ xhci_trb_queue(بنية xhci_ring *ring,
                           فراغ *data, u32 xferlen, u32 flags)
{
    ܐܢ (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
ساكن صحيح xhci_cmd_submit(بنية usb_xhci_s *xhci, بنية xhci_inctx *inctx
                           , u32 flags)
{
    ܐܢ (inctx) {
        بنية xhci_slotctx *slot = (فراغ*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        ܐܢ (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            ܦܢܐ -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    صحيح rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    ܦܢܐ rc;
}

ساكن صحيح xhci_cmd_enable_slot(بنية usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    صحيح cc = xhci_cmd_submit(xhci, عدم, CR_ENABLE_SLOT << 10);
    ܐܢ (cc != CC_SUCCESS)
        ܦܢܐ -1;
    ܦܢܐ (xhci->cmds->evt.control >> 24) & 0xff;
}

ساكن صحيح xhci_cmd_disable_slot(بنية usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ܦܢܐ xhci_cmd_submit(xhci, عدم, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

ساكن صحيح xhci_cmd_address_device(بنية usb_xhci_s *xhci, u32 slotid
                                   , بنية xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ܦܢܐ xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

ساكن صحيح xhci_cmd_configure_endpoint(بنية usb_xhci_s *xhci, u32 slotid
                                       , بنية xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ܦܢܐ xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ساكن صحيح xhci_cmd_evaluate_context(بنية usb_xhci_s *xhci, u32 slotid
                                     , بنية xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ܦܢܐ xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ساكن بنية xhci_inctx *
xhci_alloc_inctx(بنية usbdevice_s *usbdev, صحيح maxepid)
{
    بنية usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, بنية usb_xhci_s, usb);
    صحيح size = (حجم(بنية xhci_inctx) * 33) << xhci->context64;
    بنية xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    ܐܢ (!in) {
        warn_noalloc();
        ܦܢܐ عدم;
    }
    memset(in, 0, size);

    بنية xhci_slotctx *slot = (فراغ*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    بنية usbdevice_s *hubdev = usbdev->hub->usbdev;
    ܐܢ (hubdev) {
        ܐܢ (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            بنية xhci_pipe *hpipe = container_of(
                hubdev->defpipe, بنية xhci_pipe, pipe);
            ܐܢ (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } ܐܚܪܢܐ {
                بنية xhci_slotctx *hslot = (فراغ*)xhci->devs[hpipe->slotid].ptr_low;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        ܟܡܐ (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    ܦܢܐ in;
}

ساكن صحيح xhci_config_hub(بنية usbhub_s *hub)
{
    بنية usb_xhci_s *xhci = container_of(
        hub->cntl, بنية usb_xhci_s, usb);
    بنية xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, بنية xhci_pipe, pipe);
    بنية xhci_slotctx *hdslot = (فراغ*)xhci->devs[pipe->slotid].ptr_low;
    ܐܢ ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        ܦܢܐ 0;
    بنية xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    ܐܢ (!in)
        ܦܢܐ -1;
    in->add = 0x01;
    بنية xhci_slotctx *slot = (فراغ*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    صحيح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    حرر(in);
    ܐܢ (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        ܦܢܐ -1;
    }
    ܦܢܐ 0;
}

ساكن بنية usb_pipe *
xhci_alloc_pipe(بنية usbdevice_s *usbdev
                , بنية usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    بنية usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, بنية usb_xhci_s, usb);
    بنية xhci_pipe *pipe;
    u32 epid;

    ܐܢ (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } ܐܚܪܢܐ {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    ܐܢ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_high(XHCI_RING_SIZE, حجم(*pipe));
    ܐܚܪܢܐ
        pipe = memalign_low(XHCI_RING_SIZE, حجم(*pipe));
    ܐܢ (!pipe) {
        warn_noalloc();
        ܦܢܐ عدم;
    }
    memset(pipe, 0, حجم(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    ܐܢ (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_high(pipe->pipe.maxpacket);
        ܐܢ (!pipe->buf) {
            warn_noalloc();
            حرر(pipe);
            ܦܢܐ عدم;
        }
    }

    // Allocate input context and initialize endpoint info.
    بنية xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    ܐܢ (!in)
        اذهب fail;
    in->add = 0x01 | (1 << epid);
    بنية xhci_epctx *ep = (فراغ*)&in[(pipe->epid+1) << xhci->context64];
    ܐܢ (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    ܐܢ (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_low  = (u32)&pipe->reqs.ring[0];
    ep->deq_low  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ܐܢ (pipe->epid == 1) {
        ܐܢ (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            صحيح ret = xhci_config_hub(usbdev->hub);
            ܐܢ (ret)
                اذهب fail;
        }
        // Enable slot.
        u32 size = (حجم(بنية xhci_slotctx) * 32) << xhci->context64;
        بنية xhci_slotctx *dev = memalign_high(1024 << xhci->context64, size);
        ܐܢ (!dev) {
            warn_noalloc();
            اذهب fail;
        }
        صحيح slotid = xhci_cmd_enable_slot(xhci);
        ܐܢ (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            حرر(dev);
            اذهب fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_low = (u32)dev;
        xhci->devs[slotid].ptr_high = 0;

        // Send set_address command.
        صحيح cc = xhci_cmd_address_device(xhci, slotid, in);
        ܐܢ (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            ܐܢ (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                اذهب fail;
            }
            xhci->devs[slotid].ptr_low = 0;
            حرر(dev);
            اذهب fail;
        }
        pipe->slotid = slotid;
    } ܐܚܪܢܐ {
        بنية xhci_pipe *defpipe = container_of(
            usbdev->defpipe, بنية xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        صحيح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        ܐܢ (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            اذهب fail;
        }
    }
    حرر(in);
    ܦܢܐ &pipe->pipe;

fail:
    حرر(pipe->buf);
    حرر(pipe);
    حرر(in);
    ܦܢܐ عدم;
}

بنية usb_pipe *
xhci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    ܐܢ (!CONFIG_USB_XHCI)
        ܦܢܐ عدم;
    ܐܢ (!epdesc) {
        usb_add_freelist(upipe);
        ܦܢܐ عدم;
    }
    ܐܢ (!upipe)
        ܦܢܐ xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    صحيح oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    بنية xhci_pipe *pipe = container_of(upipe, بنية xhci_pipe, pipe);
    بنية usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ܐܢ (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        ܦܢܐ upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    بنية xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    ܐܢ (!in)
        ܦܢܐ upipe;
    in->add = (1 << 1);
    بنية xhci_epctx *ep = (فراغ*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    صحيح cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    ܐܢ (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    حرر(in);

    ܦܢܐ upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ساكن فراغ xhci_xfer_setup(بنية xhci_pipe *pipe, صحيح dir, فراغ *cmd
                            , فراغ *data, صحيح datalen)
{
    بنية usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    ܐܢ (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, عدم, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ساكن فراغ xhci_xfer_normal(بنية xhci_pipe *pipe,
                             فراغ *data, صحيح datalen)
{
    بنية usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

صحيح
xhci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *data, صحيح datalen)
{
    ܐܢ (!CONFIG_USB_XHCI)
        ܦܢܐ -1;
    بنية xhci_pipe *pipe = container_of(p, بنية xhci_pipe, pipe);
    بنية usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);

    ܐܢ (cmd) {
        ثابت بنية usb_ctrlrequest *req = cmd;
        ܐܢ (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            ܦܢܐ 0;
        xhci_xfer_setup(pipe, dir, (فراغ*)req, data, datalen);
    } ܐܚܪܢܐ {
        xhci_xfer_normal(pipe, data, datalen);
    }

    صحيح cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    ܐܢ (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        ܦܢܐ -1;
    }

    ܦܢܐ 0;
}

صحيح VISIBLE32FLAT
xhci_poll_intr(بنية usb_pipe *p, فراغ *data)
{
    ܐܢ (!CONFIG_USB_XHCI)
        ܦܢܐ -1;

    بنية xhci_pipe *pipe = container_of(p, بنية xhci_pipe, pipe);
    بنية usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    فراغ *buf = pipe->buf;
    صحيح bufused = pipe->bufused;

    ܐܢ (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        ܦܢܐ -1;
    }

    xhci_process_events(xhci);
    ܐܢ (xhci_ring_busy(&pipe->reqs))
        ܦܢܐ -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    ܦܢܐ 0;
}
