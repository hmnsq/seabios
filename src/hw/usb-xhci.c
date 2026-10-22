// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // CONFIG_*
#include "malloc.h" // memalign_low
#include "memmap.h" // PAGE_SIZE
#include "output.h" // dprintf
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#include "pci_regs.h" // PCI_BASE_ADDRESS_0
#include "string.h" // memcpy
#include "usb.h" // struct usb_s
#include "usb-xhci.h" // struct ehci_qh
#include "util.h" // timer_calc
#include "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_ITEMS          16
#define XHCI_RING_SIZE           (XHCI_RING_ITEMS*parimaa_na(sa_nracanaa xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((sa_nracanaa xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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

typedef ga_nanaa TRBType {
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

typedef ga_nanaa TRBCCode {
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

ga_nanaa {
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

sa_nracanaa xhci_ring {
    sa_nracanaa xhci_trb      ring[XHCI_RING_ITEMS];
    sa_nracanaa xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    sa_nracanaa mutex_s       lock;
};

sa_nracanaa xhci_portmap {
    u8 start;
    u8 count;
};

sa_nracanaa usb_xhci_s {
    sa_nracanaa usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    sa_nracanaa xhci_portmap  usb2;
    sa_nracanaa xhci_portmap  usb3;

    /* xhci registers */
    sa_nracanaa xhci_caps     *caps;
    sa_nracanaa xhci_op       *op;
    sa_nracanaa xhci_pr       *pr;
    sa_nracanaa xhci_ir       *ir;
    sa_nracanaa xhci_db       *db;

    /* xhci data structures */
    sa_nracanaa xhci_devlist  *devs;
    sa_nracanaa xhci_ring     *cmds;
    sa_nracanaa xhci_ring     *evts;
    sa_nracanaa xhci_er_seg   *eseg;
};

sa_nracanaa xhci_pipe {
    sa_nracanaa xhci_ring     reqs;

    sa_nracanaa usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    void                 *buf;
    puur_naa_nka                  bufused;
};

// --------------------------------------------------------------
// tables

static const var_na *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

static const puur_naa_nka speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

static const puur_naa_nka speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

static puur_naa_nka wait_bit(u32 *reg, u32 mask, puur_naa_nka value, u32 timeout)
{
    u32 end = timer_calc(timeout);

    ᰀᰬ ((readl(reg) & mask) != value) {
        ᰀᰧ (timer_check(end)) {
            warn_timeout();
            ᰀᰧᰌ -1;
        }
        yield();
    }
    ᰀᰧᰌ 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
static void
xhci_print_port_state(puur_naa_nka loglevel, const var_na *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

static puur_naa_nka
xhci_hub_detect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_xhci_s *xhci = container_of(hub->cntl, sa_nracanaa usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ᰀᰧᰌ (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
static puur_naa_nka
xhci_hub_reset(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_xhci_s *xhci = container_of(hub->cntl, sa_nracanaa usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ᰀᰧ (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        ᰀᰧᰌ -1;

    switch (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    pak_sa PLS_U0:
        // A USB3 port - controller automatically performs reset
        bha_nga;
    pak_sa PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        bha_nga;
    default:
        ᰀᰧᰌ -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    for (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        ᰀᰧ (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            ᰀᰧᰌ -1;
        ᰀᰧ (portsc & XHCI_PORTSC_PED)
            // Reset complete
            bha_nga;
        ᰀᰧ (timer_check(end)) {
            warn_timeout();
            ᰀᰧᰌ -1;
        }
        yield();
    }

    puur_naa_nka rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    ᰀᰧᰌ rc;
}

static puur_naa_nka
xhci_hub_portmap(sa_nracanaa usbhub_s *hub, u32 vport)
{
    sa_nracanaa usb_xhci_s *xhci = container_of(hub->cntl, sa_nracanaa usb_xhci_s, usb);
    u32 pport = vport + 1;

    ᰀᰧ (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.count)
        pport = vport + 2 - xhci->usb3.start;

    ᰀᰧ (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.count)
        pport = vport + 2 - xhci->usb2.start;

    ᰀᰧᰌ pport;
}

static void
xhci_hub_disconnect(sa_nracanaa usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

static sa_nracanaa usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
static puur_naa_nka
xhci_check_ports(sa_nracanaa usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    sa_nracanaa usbhub_s hub;
    memset(&hub, 0, parimaa_na(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    ᰀᰧᰌ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

static void
xhci_free_pipes(sa_nracanaa usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

static void
configure_xhci(void *data)
{
    sa_nracanaa usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_high(64, parimaa_na(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_high(64, parimaa_na(*xhci->eseg));
    xhci->cmds = memalign_high(XHCI_RING_SIZE, parimaa_na(*xhci->cmds));
    xhci->evts = memalign_high(XHCI_RING_SIZE, parimaa_na(*xhci->evts));
    ᰀᰧ (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        goto fail;
    }
    memset(xhci->devs, 0, parimaa_na(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, parimaa_na(*xhci->cmds));
    memset(xhci->evts, 0, parimaa_na(*xhci->evts));
    memset(xhci->eseg, 0, parimaa_na(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    ᰀᰧ (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        ᰀᰧ (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            goto fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    ᰀᰧ (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        goto fail;
    ᰀᰧ (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        goto fail;

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
    ᰀᰧ (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_high(64, parimaa_na(*spba) * spb);
        void *pad = memalign_high(PAGE_SIZE, PAGE_SIZE * spb);
        ᰀᰧ (!spba || !pad) {
            warn_noalloc();
            free(spba);
            free(pad);
            goto fail;
        }
        puur_naa_nka i;
        for (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_low = (u32)spba;
        xhci->devs[0].ptr_high = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    puur_naa_nka count = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    ᰀᰧ (count)
        // Success
        ᰀᰧᰌ;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    free(xhci->eseg);
    free(xhci->evts);
    free(xhci->cmds);
    free(xhci->devs);
    free(xhci);
}

static sa_nracanaa usb_xhci_s*
xhci_controller_setup(void *baseaddr)
{
    sa_nracanaa usb_xhci_s *xhci = malloc_high(parimaa_na(*xhci));
    ᰀᰧ (!xhci) {
        warn_noalloc();
        ᰀᰧᰌ NULL;
    }
    memset(xhci, 0, parimaa_na(*xhci));
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

    ᰀᰧ (xhci->xcap) {
        u32 off;
        void *addr = baseaddr + xhci->xcap;
        do {
            sa_nracanaa xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            switch (cap & 0xff) {
            pak_sa 0x02:
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
                ᰀᰧ (name == 0x20425355 /* "USB " */) {
                    ᰀᰧ (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.count = count;
                    }
                    ᰀᰧ (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.count = count;
                    }
                }
                bha_nga;
            default:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                bha_nga;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } ᰀᰬ (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    ᰀᰧ (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        free(xhci);
        ᰀᰧᰌ NULL;
    }

    ᰀᰧᰌ xhci;
}

static void
xhci_controller_setup_pci(sa_nracanaa pci_device *pci)
{
    sa_nracanaa usb_xhci_s *xhci;
    void *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ᰀᰧ (!baseaddr)
        ᰀᰧᰌ;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    ᰀᰧ (!xhci)
        ᰀᰧᰌ;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

static void
xhci_controller_setup_acpi(sa_nracanaa acpi_device *dev)
{
    sa_nracanaa usb_xhci_s *xhci;
    u64 mem, unused;
    void *baseaddr;

    ᰀᰧ (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        ᰀᰧᰌ;
    ᰀᰧ (mem >= 0x100000000ll)
        ᰀᰧᰌ;

    baseaddr = (void*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    ᰀᰧ (!xhci)
        ᰀᰧᰌ;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

void
xhci_setup(void)
{
    ᰀᰧ (! CONFIG_USB_XHCI)
        ᰀᰧᰌ;

    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ᰀᰧ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    sa_nracanaa acpi_device *dev;
    for (dev = acpi_dsdt_find_eisaid(NULL, xhci_eisaid);
         dev != NULL;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
static void xhci_doorbell(sa_nracanaa usb_xhci_s *xhci, u32 slotid, u32 value)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, value);
    sa_nracanaa xhci_db *db = xhci->db;
    void *addr = &db[slotid].doorbell;
    writel(addr, value);
}

// Dequeue events on the XHCI command ring generated by the hardware
static void xhci_process_events(sa_nracanaa usb_xhci_s *xhci)
{
    sa_nracanaa xhci_ring *evts = xhci->evts;

    for (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        sa_nracanaa xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        ᰀᰧ ((control & TRB_C) != (cs ? 1 : 0))
            ᰀᰧᰌ;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        switch (evt_type) {
        pak_sa ER_TRANSFER:
        pak_sa ER_COMMAND_COMPLETE:
        {
            sa_nracanaa xhci_trb  *rtrb = (void*)etrb->ptr_low;
            sa_nracanaa xhci_ring *ring = XHCI_RING(rtrb);
            sa_nracanaa xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, parimaa_na(*etrb));
            ring->eidx = eidx;
            bha_nga;
        }
        pak_sa ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_low >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            bha_nga;
        }
        default:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            bha_nga;
        }

        /* move ring index, notify xhci */
        nidx++;
        ᰀᰧ (nidx == XHCI_RING_ITEMS) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        sa_nracanaa xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_low, erdp);
        writel(&ir->erdp_high, 0);
    }
}

// Check if a ring has any pending TRBs
static puur_naa_nka xhci_ring_busy(sa_nracanaa xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    ᰀᰧᰌ (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
static puur_naa_nka xhci_event_wait(sa_nracanaa usb_xhci_s *xhci,
                           sa_nracanaa xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    for (;;) {
        xhci_process_events(xhci);
        ᰀᰧ (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            ᰀᰧᰌ (status >> 24) & 0xff;
        }
        ᰀᰧ (timer_check(end)) {
            warn_timeout();
            ᰀᰧᰌ -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
static void xhci_trb_fill(sa_nracanaa xhci_ring *ring
                          , void *data, u32 xferlen, u32 flags)
{
    sa_nracanaa xhci_trb *dst = &ring->ring[ring->nidx];
    ᰀᰧ (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_low, data, xferlen);
    } ᰀᰧᰀ {
        dst->ptr_low = (u32)data;
        dst->ptr_high = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
static void xhci_trb_queue(sa_nracanaa xhci_ring *ring,
                           void *data, u32 xferlen, u32 flags)
{
    ᰀᰧ (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
static puur_naa_nka xhci_cmd_submit(sa_nracanaa usb_xhci_s *xhci, sa_nracanaa xhci_inctx *inctx
                           , u32 flags)
{
    ᰀᰧ (inctx) {
        sa_nracanaa xhci_slotctx *slot = (void*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        ᰀᰧ (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            ᰀᰧᰌ -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    puur_naa_nka rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    ᰀᰧᰌ rc;
}

static puur_naa_nka xhci_cmd_enable_slot(sa_nracanaa usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    puur_naa_nka cc = xhci_cmd_submit(xhci, NULL, CR_ENABLE_SLOT << 10);
    ᰀᰧ (cc != CC_SUCCESS)
        ᰀᰧᰌ -1;
    ᰀᰧᰌ (xhci->cmds->evt.control >> 24) & 0xff;
}

static puur_naa_nka xhci_cmd_disable_slot(sa_nracanaa usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ᰀᰧᰌ xhci_cmd_submit(xhci, NULL, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

static puur_naa_nka xhci_cmd_address_device(sa_nracanaa usb_xhci_s *xhci, u32 slotid
                                   , sa_nracanaa xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ᰀᰧᰌ xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

static puur_naa_nka xhci_cmd_configure_endpoint(sa_nracanaa usb_xhci_s *xhci, u32 slotid
                                       , sa_nracanaa xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ᰀᰧᰌ xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

static puur_naa_nka xhci_cmd_evaluate_context(sa_nracanaa usb_xhci_s *xhci, u32 slotid
                                     , sa_nracanaa xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ᰀᰧᰌ xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

static sa_nracanaa xhci_inctx *
xhci_alloc_inctx(sa_nracanaa usbdevice_s *usbdev, puur_naa_nka maxepid)
{
    sa_nracanaa usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_xhci_s, usb);
    puur_naa_nka size = (parimaa_na(sa_nracanaa xhci_inctx) * 33) << xhci->context64;
    sa_nracanaa xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    ᰀᰧ (!in) {
        warn_noalloc();
        ᰀᰧᰌ NULL;
    }
    memset(in, 0, size);

    sa_nracanaa xhci_slotctx *slot = (void*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    sa_nracanaa usbdevice_s *hubdev = usbdev->hub->usbdev;
    ᰀᰧ (hubdev) {
        ᰀᰧ (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            sa_nracanaa xhci_pipe *hpipe = container_of(
                hubdev->defpipe, sa_nracanaa xhci_pipe, pipe);
            ᰀᰧ (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } ᰀᰧᰀ {
                sa_nracanaa xhci_slotctx *hslot = (void*)xhci->devs[hpipe->slotid].ptr_low;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        ᰀᰬ (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    ᰀᰧᰌ in;
}

static puur_naa_nka xhci_config_hub(sa_nracanaa usbhub_s *hub)
{
    sa_nracanaa usb_xhci_s *xhci = container_of(
        hub->cntl, sa_nracanaa usb_xhci_s, usb);
    sa_nracanaa xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, sa_nracanaa xhci_pipe, pipe);
    sa_nracanaa xhci_slotctx *hdslot = (void*)xhci->devs[pipe->slotid].ptr_low;
    ᰀᰧ ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        ᰀᰧᰌ 0;
    sa_nracanaa xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    ᰀᰧ (!in)
        ᰀᰧᰌ -1;
    in->add = 0x01;
    sa_nracanaa xhci_slotctx *slot = (void*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    puur_naa_nka cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    free(in);
    ᰀᰧ (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        ᰀᰧᰌ -1;
    }
    ᰀᰧᰌ 0;
}

static sa_nracanaa usb_pipe *
xhci_alloc_pipe(sa_nracanaa usbdevice_s *usbdev
                , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    sa_nracanaa usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_xhci_s, usb);
    sa_nracanaa xhci_pipe *pipe;
    u32 epid;

    ᰀᰧ (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } ᰀᰧᰀ {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    ᰀᰧ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_high(XHCI_RING_SIZE, parimaa_na(*pipe));
    ᰀᰧᰀ
        pipe = memalign_low(XHCI_RING_SIZE, parimaa_na(*pipe));
    ᰀᰧ (!pipe) {
        warn_noalloc();
        ᰀᰧᰌ NULL;
    }
    memset(pipe, 0, parimaa_na(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    ᰀᰧ (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_high(pipe->pipe.maxpacket);
        ᰀᰧ (!pipe->buf) {
            warn_noalloc();
            free(pipe);
            ᰀᰧᰌ NULL;
        }
    }

    // Allocate input context and initialize endpoint info.
    sa_nracanaa xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    ᰀᰧ (!in)
        goto fail;
    in->add = 0x01 | (1 << epid);
    sa_nracanaa xhci_epctx *ep = (void*)&in[(pipe->epid+1) << xhci->context64];
    ᰀᰧ (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    ᰀᰧ (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_low  = (u32)&pipe->reqs.ring[0];
    ep->deq_low  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ᰀᰧ (pipe->epid == 1) {
        ᰀᰧ (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            puur_naa_nka ret = xhci_config_hub(usbdev->hub);
            ᰀᰧ (ret)
                goto fail;
        }
        // Enable slot.
        u32 size = (parimaa_na(sa_nracanaa xhci_slotctx) * 32) << xhci->context64;
        sa_nracanaa xhci_slotctx *dev = memalign_high(1024 << xhci->context64, size);
        ᰀᰧ (!dev) {
            warn_noalloc();
            goto fail;
        }
        puur_naa_nka slotid = xhci_cmd_enable_slot(xhci);
        ᰀᰧ (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            free(dev);
            goto fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_low = (u32)dev;
        xhci->devs[slotid].ptr_high = 0;

        // Send set_address command.
        puur_naa_nka cc = xhci_cmd_address_device(xhci, slotid, in);
        ᰀᰧ (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            ᰀᰧ (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                goto fail;
            }
            xhci->devs[slotid].ptr_low = 0;
            free(dev);
            goto fail;
        }
        pipe->slotid = slotid;
    } ᰀᰧᰀ {
        sa_nracanaa xhci_pipe *defpipe = container_of(
            usbdev->defpipe, sa_nracanaa xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        puur_naa_nka cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        ᰀᰧ (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            goto fail;
        }
    }
    free(in);
    ᰀᰧᰌ &pipe->pipe;

fail:
    free(pipe->buf);
    free(pipe);
    free(in);
    ᰀᰧᰌ NULL;
}

sa_nracanaa usb_pipe *
xhci_realloc_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *upipe
                  , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ᰀᰧ (!CONFIG_USB_XHCI)
        ᰀᰧᰌ NULL;
    ᰀᰧ (!epdesc) {
        usb_add_freelist(upipe);
        ᰀᰧᰌ NULL;
    }
    ᰀᰧ (!upipe)
        ᰀᰧᰌ xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    puur_naa_nka oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    sa_nracanaa xhci_pipe *pipe = container_of(upipe, sa_nracanaa xhci_pipe, pipe);
    sa_nracanaa usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, sa_nracanaa usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ᰀᰧ (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        ᰀᰧᰌ upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    sa_nracanaa xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    ᰀᰧ (!in)
        ᰀᰧᰌ upipe;
    in->add = (1 << 1);
    sa_nracanaa xhci_epctx *ep = (void*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    puur_naa_nka cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    ᰀᰧ (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    free(in);

    ᰀᰧᰌ upipe;
}

// Submit a USB "setup" message request to the pipe's ring
static void xhci_xfer_setup(sa_nracanaa xhci_pipe *pipe, puur_naa_nka dir, void *cmd
                            , void *data, puur_naa_nka datalen)
{
    sa_nracanaa usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, sa_nracanaa usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    ᰀᰧ (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, NULL, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
static void xhci_xfer_normal(sa_nracanaa xhci_pipe *pipe,
                             void *data, puur_naa_nka datalen)
{
    sa_nracanaa usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, sa_nracanaa usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

puur_naa_nka
xhci_send_pipe(sa_nracanaa usb_pipe *p, puur_naa_nka dir, const void *cmd
               , void *data, puur_naa_nka datalen)
{
    ᰀᰧ (!CONFIG_USB_XHCI)
        ᰀᰧᰌ -1;
    sa_nracanaa xhci_pipe *pipe = container_of(p, sa_nracanaa xhci_pipe, pipe);
    sa_nracanaa usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, sa_nracanaa usb_xhci_s, usb);

    ᰀᰧ (cmd) {
        const sa_nracanaa usb_ctrlrequest *req = cmd;
        ᰀᰧ (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            ᰀᰧᰌ 0;
        xhci_xfer_setup(pipe, dir, (void*)req, data, datalen);
    } ᰀᰧᰀ {
        xhci_xfer_normal(pipe, data, datalen);
    }

    puur_naa_nka cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    ᰀᰧ (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        ᰀᰧᰌ -1;
    }

    ᰀᰧᰌ 0;
}

puur_naa_nka VISIBLE32FLAT
xhci_poll_intr(sa_nracanaa usb_pipe *p, void *data)
{
    ᰀᰧ (!CONFIG_USB_XHCI)
        ᰀᰧᰌ -1;

    sa_nracanaa xhci_pipe *pipe = container_of(p, sa_nracanaa xhci_pipe, pipe);
    sa_nracanaa usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, sa_nracanaa usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    void *buf = pipe->buf;
    puur_naa_nka bufused = pipe->bufused;

    ᰀᰧ (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        ᰀᰧᰌ -1;
    }

    xhci_process_events(xhci);
    ᰀᰧ (xhci_ring_busy(&pipe->reqs))
        ᰀᰧᰌ -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    ᰀᰧᰌ 0;
}
