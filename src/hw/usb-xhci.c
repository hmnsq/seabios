// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // memalign_low
#شامل "memmap.h" // PAGE_SIZE
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#شامل "pci_regs.h" // PCI_BASE_ADDRESS_0
#شامل "string.h" // memcpy
#شامل "usb.h" // struct usb_s
#شامل "usb-xhci.h" // struct ehci_qh
#شامل "util.h" // timer_calc
#شامل "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_اقلام          16
#define XHCI_RING_SIZE           (XHCI_RING_اقلام*اندازه(ساختار xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((ساختار xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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
#define XHCI_PORTSC_SPEED_کم    (2<<10)
#define XHCI_PORTSC_SPEED_بالا   (3<<10)
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

تعریفنوع شمارش TRBType {
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

تعریفنوع شمارش TRBCCode {
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

شمارش {
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

ساختار xhci_ring {
    ساختار xhci_trb      ring[XHCI_RING_اقلام];
    ساختار xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    ساختار mutex_s       lock;
};

ساختار xhci_portmap {
    u8 start;
    u8 شمار;
};

ساختار usb_xhci_s {
    ساختار usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    ساختار xhci_portmap  usb2;
    ساختار xhci_portmap  usb3;

    /* xhci registers */
    ساختار xhci_caps     *caps;
    ساختار xhci_op       *op;
    ساختار xhci_pr       *pr;
    ساختار xhci_ir       *ir;
    ساختار xhci_db       *db;

    /* xhci data structures */
    ساختار xhci_devlist  *devs;
    ساختار xhci_ring     *cmds;
    ساختار xhci_ring     *evts;
    ساختار xhci_er_seg   *eseg;
};

ساختار xhci_pipe {
    ساختار xhci_ring     reqs;

    ساختار usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    تهی                 *buf;
    صحیح                  bufused;
};

// --------------------------------------------------------------
// tables

ایستا ثابت نویسه *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ایستا ثابت صحیح speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ایستا ثابت صحیح speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ایستا صحیح wait_bit(u32 *reg, u32 mask, صحیح مقدار, u32 timeout)
{
    u32 end = timer_calc(timeout);

    وقتی ((readl(reg) & mask) != مقدار) {
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت -1;
        }
        yield();
    }
    بازگشت 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
ایستا تهی
xhci_print_port_state(صحیح loglevel, ثابت نویسه *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

ایستا صحیح
xhci_hub_detect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_xhci_s *xhci = container_of(hub->cntl, ساختار usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    بازگشت (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ایستا صحیح
xhci_hub_reset(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_xhci_s *xhci = container_of(hub->cntl, ساختار usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    اگر (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        بازگشت -1;

    انتخاب (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    حالت PLS_U0:
        // A USB3 port - controller automatically performs reset
        بشکن;
    حالت PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        بشکن;
    پیشفرض:
        بازگشت -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    برای (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        اگر (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            بازگشت -1;
        اگر (portsc & XHCI_PORTSC_PED)
            // Reset complete
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت -1;
        }
        yield();
    }

    صحیح rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    بازگشت rc;
}

ایستا صحیح
xhci_hub_portmap(ساختار usbhub_s *hub, u32 vport)
{
    ساختار usb_xhci_s *xhci = container_of(hub->cntl, ساختار usb_xhci_s, usb);
    u32 pport = vport + 1;

    اگر (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.شمار)
        pport = vport + 2 - xhci->usb3.start;

    اگر (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.شمار)
        pport = vport + 2 - xhci->usb2.start;

    بازگشت pport;
}

ایستا تهی
xhci_hub_disconnect(ساختار usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

ایستا ساختار usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ایستا صحیح
xhci_check_ports(ساختار usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    ساختار usbhub_s hub;
    memset(&hub, 0, اندازه(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    بازگشت hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

ایستا تهی
xhci_free_pipes(ساختار usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

ایستا تهی
configure_xhci(تهی *data)
{
    ساختار usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_بالا(64, اندازه(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_بالا(64, اندازه(*xhci->eseg));
    xhci->cmds = memalign_بالا(XHCI_RING_SIZE, اندازه(*xhci->cmds));
    xhci->evts = memalign_بالا(XHCI_RING_SIZE, اندازه(*xhci->evts));
    اگر (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        برو fail;
    }
    memset(xhci->devs, 0, اندازه(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, اندازه(*xhci->cmds));
    memset(xhci->evts, 0, اندازه(*xhci->evts));
    memset(xhci->eseg, 0, اندازه(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    اگر (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        اگر (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            برو fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    اگر (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        برو fail;
    اگر (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        برو fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_کم, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_بالا, 0);
    writel(&xhci->op->crcr_کم, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_بالا, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_کم = (u32)xhci->evts;
    xhci->eseg->ptr_بالا = 0;
    xhci->eseg->size = XHCI_RING_اقلام;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_کم, (u32)xhci->evts);
    writel(&xhci->ir->erdp_بالا, 0);
    writel(&xhci->ir->erstba_کم, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_بالا, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    اگر (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_بالا(64, اندازه(*spba) * spb);
        تهی *pad = memalign_بالا(PAGE_SIZE, PAGE_SIZE * spb);
        اگر (!spba || !pad) {
            warn_noalloc();
            آزاد(spba);
            آزاد(pad);
            برو fail;
        }
        صحیح i;
        برای (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_کم = (u32)spba;
        xhci->devs[0].ptr_بالا = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    صحیح شمار = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    اگر (شمار)
        // Success
        بازگشت;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    آزاد(xhci->eseg);
    آزاد(xhci->evts);
    آزاد(xhci->cmds);
    آزاد(xhci->devs);
    آزاد(xhci);
}

ایستا ساختار usb_xhci_s*
xhci_controller_setup(تهی *baseaddr)
{
    ساختار usb_xhci_s *xhci = malloc_بالا(اندازه(*xhci));
    اگر (!xhci) {
        warn_noalloc();
        بازگشت پوچ;
    }
    memset(xhci, 0, اندازه(*xhci));
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

    اگر (xhci->xcap) {
        u32 off;
        تهی *addr = baseaddr + xhci->xcap;
        انجامبده {
            ساختار xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            انتخاب (cap & 0xff) {
            حالت 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 شمار = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , شمار, start
                        , ports >> 16);
                اگر (name == 0x20425355 /* "USB " */) {
                    اگر (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.شمار = شمار;
                    }
                    اگر (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.شمار = شمار;
                    }
                }
                بشکن;
            پیشفرض:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                بشکن;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } وقتی (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    اگر (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        آزاد(xhci);
        بازگشت پوچ;
    }

    بازگشت xhci;
}

ایستا تهی
xhci_controller_setup_pci(ساختار pci_device *pci)
{
    ساختار usb_xhci_s *xhci;
    تهی *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    اگر (!baseaddr)
        بازگشت;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    اگر (!xhci)
        بازگشت;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

ایستا تهی
xhci_controller_setup_acpi(ساختار acpi_device *dev)
{
    ساختار usb_xhci_s *xhci;
    u64 mem, unused;
    تهی *baseaddr;

    اگر (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        بازگشت;
    اگر (mem >= 0x100000000ll)
        بازگشت;

    baseaddr = (تهی*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    اگر (!xhci)
        بازگشت;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

تهی
xhci_setup(تهی)
{
    اگر (! CONFIG_USB_XHCI)
        بازگشت;

    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    ساختار acpi_device *dev;
    برای (dev = acpi_dsdt_find_eisaid(پوچ, xhci_eisaid);
         dev != پوچ;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ایستا تهی xhci_doorbell(ساختار usb_xhci_s *xhci, u32 slotid, u32 مقدار)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, مقدار);
    ساختار xhci_db *db = xhci->db;
    تهی *addr = &db[slotid].doorbell;
    writel(addr, مقدار);
}

// Dequeue events on the XHCI command ring generated by the hardware
ایستا تهی xhci_process_events(ساختار usb_xhci_s *xhci)
{
    ساختار xhci_ring *evts = xhci->evts;

    برای (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        ساختار xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        اگر ((control & TRB_C) != (cs ? 1 : 0))
            بازگشت;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        انتخاب (evt_type) {
        حالت ER_TRANSFER:
        حالت ER_COMMAND_COMPLETE:
        {
            ساختار xhci_trb  *rtrb = (تهی*)etrb->ptr_کم;
            ساختار xhci_ring *ring = XHCI_RING(rtrb);
            ساختار xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, اندازه(*etrb));
            ring->eidx = eidx;
            بشکن;
        }
        حالت ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_کم >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            بشکن;
        }
        پیشفرض:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            بشکن;
        }

        /* move ring index, notify xhci */
        nidx++;
        اگر (nidx == XHCI_RING_اقلام) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        ساختار xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_کم, erdp);
        writel(&ir->erdp_بالا, 0);
    }
}

// Check if a ring has any pending TRBs
ایستا صحیح xhci_ring_busy(ساختار xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    بازگشت (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ایستا صحیح xhci_event_wait(ساختار usb_xhci_s *xhci,
                           ساختار xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    برای (;;) {
        xhci_process_events(xhci);
        اگر (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            بازگشت (status >> 24) & 0xff;
        }
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ایستا تهی xhci_trb_fill(ساختار xhci_ring *ring
                          , تهی *data, u32 xferlen, u32 flags)
{
    ساختار xhci_trb *dst = &ring->ring[ring->nidx];
    اگر (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_کم, data, xferlen);
    } وگرنه {
        dst->ptr_کم = (u32)data;
        dst->ptr_بالا = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ایستا تهی xhci_trb_queue(ساختار xhci_ring *ring,
                           تهی *data, u32 xferlen, u32 flags)
{
    اگر (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
ایستا صحیح xhci_cmd_submit(ساختار usb_xhci_s *xhci, ساختار xhci_inctx *inctx
                           , u32 flags)
{
    اگر (inctx) {
        ساختار xhci_slotctx *slot = (تهی*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        اگر (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            بازگشت -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    صحیح rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    بازگشت rc;
}

ایستا صحیح xhci_cmd_enable_slot(ساختار usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    صحیح cc = xhci_cmd_submit(xhci, پوچ, CR_ENABLE_SLOT << 10);
    اگر (cc != CC_SUCCESS)
        بازگشت -1;
    بازگشت (xhci->cmds->evt.control >> 24) & 0xff;
}

ایستا صحیح xhci_cmd_disable_slot(ساختار usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    بازگشت xhci_cmd_submit(xhci, پوچ, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

ایستا صحیح xhci_cmd_address_device(ساختار usb_xhci_s *xhci, u32 slotid
                                   , ساختار xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    بازگشت xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

ایستا صحیح xhci_cmd_configure_endpoint(ساختار usb_xhci_s *xhci, u32 slotid
                                       , ساختار xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    بازگشت xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ایستا صحیح xhci_cmd_evaluate_context(ساختار usb_xhci_s *xhci, u32 slotid
                                     , ساختار xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    بازگشت xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ایستا ساختار xhci_inctx *
xhci_alloc_inctx(ساختار usbdevice_s *usbdev, صحیح maxepid)
{
    ساختار usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ساختار usb_xhci_s, usb);
    صحیح size = (اندازه(ساختار xhci_inctx) * 33) << xhci->context64;
    ساختار xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    اگر (!in) {
        warn_noalloc();
        بازگشت پوچ;
    }
    memset(in, 0, size);

    ساختار xhci_slotctx *slot = (تهی*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    ساختار usbdevice_s *hubdev = usbdev->hub->usbdev;
    اگر (hubdev) {
        اگر (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            ساختار xhci_pipe *hpipe = container_of(
                hubdev->defpipe, ساختار xhci_pipe, pipe);
            اگر (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } وگرنه {
                ساختار xhci_slotctx *hslot = (تهی*)xhci->devs[hpipe->slotid].ptr_کم;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        وقتی (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    بازگشت in;
}

ایستا صحیح xhci_config_hub(ساختار usbhub_s *hub)
{
    ساختار usb_xhci_s *xhci = container_of(
        hub->cntl, ساختار usb_xhci_s, usb);
    ساختار xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, ساختار xhci_pipe, pipe);
    ساختار xhci_slotctx *hdslot = (تهی*)xhci->devs[pipe->slotid].ptr_کم;
    اگر ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        بازگشت 0;
    ساختار xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    اگر (!in)
        بازگشت -1;
    in->add = 0x01;
    ساختار xhci_slotctx *slot = (تهی*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    صحیح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    آزاد(in);
    اگر (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        بازگشت -1;
    }
    بازگشت 0;
}

ایستا ساختار usb_pipe *
xhci_alloc_pipe(ساختار usbdevice_s *usbdev
                , ساختار usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ساختار usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ساختار usb_xhci_s, usb);
    ساختار xhci_pipe *pipe;
    u32 epid;

    اگر (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } وگرنه {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_بالا(XHCI_RING_SIZE, اندازه(*pipe));
    وگرنه
        pipe = memalign_کم(XHCI_RING_SIZE, اندازه(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        بازگشت پوچ;
    }
    memset(pipe, 0, اندازه(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    اگر (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_بالا(pipe->pipe.maxpacket);
        اگر (!pipe->buf) {
            warn_noalloc();
            آزاد(pipe);
            بازگشت پوچ;
        }
    }

    // Allocate input context and initialize endpoint info.
    ساختار xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    اگر (!in)
        برو fail;
    in->add = 0x01 | (1 << epid);
    ساختار xhci_epctx *ep = (تهی*)&in[(pipe->epid+1) << xhci->context64];
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    اگر (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_کم  = (u32)&pipe->reqs.ring[0];
    ep->deq_کم  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    اگر (pipe->epid == 1) {
        اگر (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            صحیح ret = xhci_config_hub(usbdev->hub);
            اگر (ret)
                برو fail;
        }
        // Enable slot.
        u32 size = (اندازه(ساختار xhci_slotctx) * 32) << xhci->context64;
        ساختار xhci_slotctx *dev = memalign_بالا(1024 << xhci->context64, size);
        اگر (!dev) {
            warn_noalloc();
            برو fail;
        }
        صحیح slotid = xhci_cmd_enable_slot(xhci);
        اگر (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            آزاد(dev);
            برو fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_کم = (u32)dev;
        xhci->devs[slotid].ptr_بالا = 0;

        // Send set_address command.
        صحیح cc = xhci_cmd_address_device(xhci, slotid, in);
        اگر (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            اگر (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                برو fail;
            }
            xhci->devs[slotid].ptr_کم = 0;
            آزاد(dev);
            برو fail;
        }
        pipe->slotid = slotid;
    } وگرنه {
        ساختار xhci_pipe *defpipe = container_of(
            usbdev->defpipe, ساختار xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        صحیح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        اگر (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            برو fail;
        }
    }
    آزاد(in);
    بازگشت &pipe->pipe;

fail:
    آزاد(pipe->buf);
    آزاد(pipe);
    آزاد(in);
    بازگشت پوچ;
}

ساختار usb_pipe *
xhci_realloc_pipe(ساختار usbdevice_s *usbdev, ساختار usb_pipe *upipe
                  , ساختار usb_endpoint_descriptor *epdesc)
{
    اگر (!CONFIG_USB_XHCI)
        بازگشت پوچ;
    اگر (!epdesc) {
        usb_add_freelist(upipe);
        بازگشت پوچ;
    }
    اگر (!upipe)
        بازگشت xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    صحیح oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    ساختار xhci_pipe *pipe = container_of(upipe, ساختار xhci_pipe, pipe);
    ساختار usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ساختار usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    اگر (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        بازگشت upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    ساختار xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    اگر (!in)
        بازگشت upipe;
    in->add = (1 << 1);
    ساختار xhci_epctx *ep = (تهی*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    صحیح cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    اگر (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    آزاد(in);

    بازگشت upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ایستا تهی xhci_xfer_setup(ساختار xhci_pipe *pipe, صحیح dir, تهی *cmd
                            , تهی *data, صحیح datalen)
{
    ساختار usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ساختار usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    اگر (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, پوچ, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ایستا تهی xhci_xfer_normal(ساختار xhci_pipe *pipe,
                             تهی *data, صحیح datalen)
{
    ساختار usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ساختار usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

صحیح
xhci_send_pipe(ساختار usb_pipe *p, صحیح dir, ثابت تهی *cmd
               , تهی *data, صحیح datalen)
{
    اگر (!CONFIG_USB_XHCI)
        بازگشت -1;
    ساختار xhci_pipe *pipe = container_of(p, ساختار xhci_pipe, pipe);
    ساختار usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ساختار usb_xhci_s, usb);

    اگر (cmd) {
        ثابت ساختار usb_ctrlrequest *req = cmd;
        اگر (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            بازگشت 0;
        xhci_xfer_setup(pipe, dir, (تهی*)req, data, datalen);
    } وگرنه {
        xhci_xfer_normal(pipe, data, datalen);
    }

    صحیح cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    اگر (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        بازگشت -1;
    }

    بازگشت 0;
}

صحیح VISIBLE32FLAT
xhci_poll_intr(ساختار usb_pipe *p, تهی *data)
{
    اگر (!CONFIG_USB_XHCI)
        بازگشت -1;

    ساختار xhci_pipe *pipe = container_of(p, ساختار xhci_pipe, pipe);
    ساختار usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ساختار usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    تهی *buf = pipe->buf;
    صحیح bufused = pipe->bufused;

    اگر (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        بازگشت -1;
    }

    xhci_process_events(xhci);
    اگر (xhci_ring_busy(&pipe->reqs))
        بازگشت -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    بازگشت 0;
}
