// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // memalign_low
#અન્તર્ભાવ "memmap.h" // PAGE_SIZE
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#અન્તર્ભાવ "pci_regs.h" // PCI_BASE_ADDRESS_0
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-xhci.h" // struct ehci_qh
#અન્તર્ભાવ "util.h" // timer_calc
#અન્તર્ભાવ "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_વસ્તુઓ          16
#define XHCI_RING_SIZE           (XHCI_RING_વસ્તુઓ*કદ(સંરચના xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((સંરચના xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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
#define XHCI_PORTSC_SPEED_નીચલું    (2<<10)
#define XHCI_PORTSC_SPEED_ઉપલું   (3<<10)
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

પ્રકારવ્યાખ્યા ગણના TRBType {
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

પ્રકારવ્યાખ્યા ગણના TRBCCode {
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

ગણના {
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

સંરચના xhci_ring {
    સંરચના xhci_trb      ring[XHCI_RING_વસ્તુઓ];
    સંરચના xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    સંરચના mutex_s       lock;
};

સંરચના xhci_portmap {
    u8 start;
    u8 ગણતરી;
};

સંરચના usb_xhci_s {
    સંરચના usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    સંરચના xhci_portmap  usb2;
    સંરચના xhci_portmap  usb3;

    /* xhci registers */
    સંરચના xhci_caps     *caps;
    સંરચના xhci_op       *op;
    સંરચના xhci_pr       *pr;
    સંરચના xhci_ir       *ir;
    સંરચના xhci_db       *db;

    /* xhci data structures */
    સંરચના xhci_devlist  *devs;
    સંરચના xhci_ring     *cmds;
    સંરચના xhci_ring     *evts;
    સંરચના xhci_er_seg   *eseg;
};

સંરચના xhci_pipe {
    સંરચના xhci_ring     reqs;

    સંરચના usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    શૂન્ય                 *buf;
    પૂર્ણાંક                  bufused;
};

// --------------------------------------------------------------
// tables

સ્થિર અચલ અક્ષર *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

સ્થિર અચલ પૂર્ણાંક speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

સ્થિર અચલ પૂર્ણાંક speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

સ્થિર પૂર્ણાંક wait_bit(u32 *reg, u32 mask, પૂર્ણાંક કિંમત, u32 timeout)
{
    u32 end = timer_calc(timeout);

    જ્યારે ((readl(reg) & mask) != કિંમત) {
        જો (timer_check(end)) {
            warn_timeout();
            પાછા -1;
        }
        yield();
    }
    પાછા 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
સ્થિર શૂન્ય
xhci_print_port_state(પૂર્ણાંક loglevel, અચલ અક્ષર *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

સ્થિર પૂર્ણાંક
xhci_hub_detect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_xhci_s *xhci = container_of(hub->cntl, સંરચના usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    પાછા (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
સ્થિર પૂર્ણાંક
xhci_hub_reset(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_xhci_s *xhci = container_of(hub->cntl, સંરચના usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    જો (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        પાછા -1;

    પસંદગી (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    સ્થિતિ PLS_U0:
        // A USB3 port - controller automatically performs reset
        તોડો;
    સ્થિતિ PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        તોડો;
    મૂળભૂત:
        પાછા -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    માટે (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        જો (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            પાછા -1;
        જો (portsc & XHCI_PORTSC_PED)
            // Reset complete
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            પાછા -1;
        }
        yield();
    }

    પૂર્ણાંક rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    પાછા rc;
}

સ્થિર પૂર્ણાંક
xhci_hub_portmap(સંરચના usbhub_s *hub, u32 vport)
{
    સંરચના usb_xhci_s *xhci = container_of(hub->cntl, સંરચના usb_xhci_s, usb);
    u32 pport = vport + 1;

    જો (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.ગણતરી)
        pport = vport + 2 - xhci->usb3.start;

    જો (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.ગણતરી)
        pport = vport + 2 - xhci->usb2.start;

    પાછા pport;
}

સ્થિર શૂન્ય
xhci_hub_disconnect(સંરચના usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

સ્થિર સંરચના usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
સ્થિર પૂર્ણાંક
xhci_check_ports(સંરચના usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    સંરચના usbhub_s hub;
    memset(&hub, 0, કદ(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    પાછા hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

સ્થિર શૂન્ય
xhci_free_pipes(સંરચના usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

સ્થિર શૂન્ય
configure_xhci(શૂન્ય *data)
{
    સંરચના usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_ઉપલું(64, કદ(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ઉપલું(64, કદ(*xhci->eseg));
    xhci->cmds = memalign_ઉપલું(XHCI_RING_SIZE, કદ(*xhci->cmds));
    xhci->evts = memalign_ઉપલું(XHCI_RING_SIZE, કદ(*xhci->evts));
    જો (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        જાઓ fail;
    }
    memset(xhci->devs, 0, કદ(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, કદ(*xhci->cmds));
    memset(xhci->evts, 0, કદ(*xhci->evts));
    memset(xhci->eseg, 0, કદ(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    જો (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        જો (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            જાઓ fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    જો (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        જાઓ fail;
    જો (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        જાઓ fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_નીચલું, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ઉપલું, 0);
    writel(&xhci->op->crcr_નીચલું, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ઉપલું, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_નીચલું = (u32)xhci->evts;
    xhci->eseg->ptr_ઉપલું = 0;
    xhci->eseg->size = XHCI_RING_વસ્તુઓ;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_નીચલું, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ઉપલું, 0);
    writel(&xhci->ir->erstba_નીચલું, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ઉપલું, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    જો (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ઉપલું(64, કદ(*spba) * spb);
        શૂન્ય *pad = memalign_ઉપલું(PAGE_SIZE, PAGE_SIZE * spb);
        જો (!spba || !pad) {
            warn_noalloc();
            મુક્ત(spba);
            મુક્ત(pad);
            જાઓ fail;
        }
        પૂર્ણાંક i;
        માટે (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_નીચલું = (u32)spba;
        xhci->devs[0].ptr_ઉપલું = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    પૂર્ણાંક ગણતરી = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    જો (ગણતરી)
        // Success
        પાછા;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    મુક્ત(xhci->eseg);
    મુક્ત(xhci->evts);
    મુક્ત(xhci->cmds);
    મુક્ત(xhci->devs);
    મુક્ત(xhci);
}

સ્થિર સંરચના usb_xhci_s*
xhci_controller_setup(શૂન્ય *baseaddr)
{
    સંરચના usb_xhci_s *xhci = malloc_ઉપલું(કદ(*xhci));
    જો (!xhci) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(xhci, 0, કદ(*xhci));
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

    જો (xhci->xcap) {
        u32 off;
        શૂન્ય *addr = baseaddr + xhci->xcap;
        કરો {
            સંરચના xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            પસંદગી (cap & 0xff) {
            સ્થિતિ 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 ગણતરી = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , ગણતરી, start
                        , ports >> 16);
                જો (name == 0x20425355 /* "USB " */) {
                    જો (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.ગણતરી = ગણતરી;
                    }
                    જો (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.ગણતરી = ગણતરી;
                    }
                }
                તોડો;
            મૂળભૂત:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                તોડો;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } જ્યારે (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    જો (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        મુક્ત(xhci);
        પાછા ખાલી;
    }

    પાછા xhci;
}

સ્થિર શૂન્ય
xhci_controller_setup_pci(સંરચના pci_device *pci)
{
    સંરચના usb_xhci_s *xhci;
    શૂન્ય *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    જો (!baseaddr)
        પાછા;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    જો (!xhci)
        પાછા;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

સ્થિર શૂન્ય
xhci_controller_setup_acpi(સંરચના acpi_device *dev)
{
    સંરચના usb_xhci_s *xhci;
    u64 mem, unused;
    શૂન્ય *baseaddr;

    જો (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        પાછા;
    જો (mem >= 0x100000000ll)
        પાછા;

    baseaddr = (શૂન્ય*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    જો (!xhci)
        પાછા;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

શૂન્ય
xhci_setup(શૂન્ય)
{
    જો (! CONFIG_USB_XHCI)
        પાછા;

    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    સંરચના acpi_device *dev;
    માટે (dev = acpi_dsdt_find_eisaid(ખાલી, xhci_eisaid);
         dev != ખાલી;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
સ્થિર શૂન્ય xhci_doorbell(સંરચના usb_xhci_s *xhci, u32 slotid, u32 કિંમત)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, કિંમત);
    સંરચના xhci_db *db = xhci->db;
    શૂન્ય *addr = &db[slotid].doorbell;
    writel(addr, કિંમત);
}

// Dequeue events on the XHCI command ring generated by the hardware
સ્થિર શૂન્ય xhci_process_events(સંરચના usb_xhci_s *xhci)
{
    સંરચના xhci_ring *evts = xhci->evts;

    માટે (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        સંરચના xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        જો ((control & TRB_C) != (cs ? 1 : 0))
            પાછા;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        પસંદગી (evt_type) {
        સ્થિતિ ER_TRANSFER:
        સ્થિતિ ER_COMMAND_COMPLETE:
        {
            સંરચના xhci_trb  *rtrb = (શૂન્ય*)etrb->ptr_નીચલું;
            સંરચના xhci_ring *ring = XHCI_RING(rtrb);
            સંરચના xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, કદ(*etrb));
            ring->eidx = eidx;
            તોડો;
        }
        સ્થિતિ ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_નીચલું >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            તોડો;
        }
        મૂળભૂત:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            તોડો;
        }

        /* move ring index, notify xhci */
        nidx++;
        જો (nidx == XHCI_RING_વસ્તુઓ) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        સંરચના xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_નીચલું, erdp);
        writel(&ir->erdp_ઉપલું, 0);
    }
}

// Check if a ring has any pending TRBs
સ્થિર પૂર્ણાંક xhci_ring_busy(સંરચના xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    પાછા (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
સ્થિર પૂર્ણાંક xhci_event_wait(સંરચના usb_xhci_s *xhci,
                           સંરચના xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    માટે (;;) {
        xhci_process_events(xhci);
        જો (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            પાછા (status >> 24) & 0xff;
        }
        જો (timer_check(end)) {
            warn_timeout();
            પાછા -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
સ્થિર શૂન્ય xhci_trb_fill(સંરચના xhci_ring *ring
                          , શૂન્ય *data, u32 xferlen, u32 flags)
{
    સંરચના xhci_trb *dst = &ring->ring[ring->nidx];
    જો (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_નીચલું, data, xferlen);
    } નહીંતર {
        dst->ptr_નીચલું = (u32)data;
        dst->ptr_ઉપલું = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
સ્થિર શૂન્ય xhci_trb_queue(સંરચના xhci_ring *ring,
                           શૂન્ય *data, u32 xferlen, u32 flags)
{
    જો (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
સ્થિર પૂર્ણાંક xhci_cmd_submit(સંરચના usb_xhci_s *xhci, સંરચના xhci_inctx *inctx
                           , u32 flags)
{
    જો (inctx) {
        સંરચના xhci_slotctx *slot = (શૂન્ય*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        જો (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            પાછા -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    પૂર્ણાંક rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    પાછા rc;
}

સ્થિર પૂર્ણાંક xhci_cmd_enable_slot(સંરચના usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    પૂર્ણાંક cc = xhci_cmd_submit(xhci, ખાલી, CR_ENABLE_SLOT << 10);
    જો (cc != CC_SUCCESS)
        પાછા -1;
    પાછા (xhci->cmds->evt.control >> 24) & 0xff;
}

સ્થિર પૂર્ણાંક xhci_cmd_disable_slot(સંરચના usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    પાછા xhci_cmd_submit(xhci, ખાલી, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

સ્થિર પૂર્ણાંક xhci_cmd_address_device(સંરચના usb_xhci_s *xhci, u32 slotid
                                   , સંરચના xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    પાછા xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

સ્થિર પૂર્ણાંક xhci_cmd_configure_endpoint(સંરચના usb_xhci_s *xhci, u32 slotid
                                       , સંરચના xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    પાછા xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

સ્થિર પૂર્ણાંક xhci_cmd_evaluate_context(સંરચના usb_xhci_s *xhci, u32 slotid
                                     , સંરચના xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    પાછા xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

સ્થિર સંરચના xhci_inctx *
xhci_alloc_inctx(સંરચના usbdevice_s *usbdev, પૂર્ણાંક maxepid)
{
    સંરચના usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, સંરચના usb_xhci_s, usb);
    પૂર્ણાંક size = (કદ(સંરચના xhci_inctx) * 33) << xhci->context64;
    સંરચના xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    જો (!in) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(in, 0, size);

    સંરચના xhci_slotctx *slot = (શૂન્ય*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    સંરચના usbdevice_s *hubdev = usbdev->hub->usbdev;
    જો (hubdev) {
        જો (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            સંરચના xhci_pipe *hpipe = container_of(
                hubdev->defpipe, સંરચના xhci_pipe, pipe);
            જો (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } નહીંતર {
                સંરચના xhci_slotctx *hslot = (શૂન્ય*)xhci->devs[hpipe->slotid].ptr_નીચલું;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        જ્યારે (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    પાછા in;
}

સ્થિર પૂર્ણાંક xhci_config_hub(સંરચના usbhub_s *hub)
{
    સંરચના usb_xhci_s *xhci = container_of(
        hub->cntl, સંરચના usb_xhci_s, usb);
    સંરચના xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, સંરચના xhci_pipe, pipe);
    સંરચના xhci_slotctx *hdslot = (શૂન્ય*)xhci->devs[pipe->slotid].ptr_નીચલું;
    જો ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        પાછા 0;
    સંરચના xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    જો (!in)
        પાછા -1;
    in->add = 0x01;
    સંરચના xhci_slotctx *slot = (શૂન્ય*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    પૂર્ણાંક cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    મુક્ત(in);
    જો (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        પાછા -1;
    }
    પાછા 0;
}

સ્થિર સંરચના usb_pipe *
xhci_alloc_pipe(સંરચના usbdevice_s *usbdev
                , સંરચના usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    સંરચના usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, સંરચના usb_xhci_s, usb);
    સંરચના xhci_pipe *pipe;
    u32 epid;

    જો (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } નહીંતર {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    જો (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ઉપલું(XHCI_RING_SIZE, કદ(*pipe));
    નહીંતર
        pipe = memalign_નીચલું(XHCI_RING_SIZE, કદ(*pipe));
    જો (!pipe) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(pipe, 0, કદ(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    જો (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ઉપલું(pipe->pipe.maxpacket);
        જો (!pipe->buf) {
            warn_noalloc();
            મુક્ત(pipe);
            પાછા ખાલી;
        }
    }

    // Allocate input context and initialize endpoint info.
    સંરચના xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    જો (!in)
        જાઓ fail;
    in->add = 0x01 | (1 << epid);
    સંરચના xhci_epctx *ep = (શૂન્ય*)&in[(pipe->epid+1) << xhci->context64];
    જો (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    જો (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_નીચલું  = (u32)&pipe->reqs.ring[0];
    ep->deq_નીચલું  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    જો (pipe->epid == 1) {
        જો (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            પૂર્ણાંક ret = xhci_config_hub(usbdev->hub);
            જો (ret)
                જાઓ fail;
        }
        // Enable slot.
        u32 size = (કદ(સંરચના xhci_slotctx) * 32) << xhci->context64;
        સંરચના xhci_slotctx *dev = memalign_ઉપલું(1024 << xhci->context64, size);
        જો (!dev) {
            warn_noalloc();
            જાઓ fail;
        }
        પૂર્ણાંક slotid = xhci_cmd_enable_slot(xhci);
        જો (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            મુક્ત(dev);
            જાઓ fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_નીચલું = (u32)dev;
        xhci->devs[slotid].ptr_ઉપલું = 0;

        // Send set_address command.
        પૂર્ણાંક cc = xhci_cmd_address_device(xhci, slotid, in);
        જો (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            જો (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                જાઓ fail;
            }
            xhci->devs[slotid].ptr_નીચલું = 0;
            મુક્ત(dev);
            જાઓ fail;
        }
        pipe->slotid = slotid;
    } નહીંતર {
        સંરચના xhci_pipe *defpipe = container_of(
            usbdev->defpipe, સંરચના xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        પૂર્ણાંક cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        જો (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            જાઓ fail;
        }
    }
    મુક્ત(in);
    પાછા &pipe->pipe;

fail:
    મુક્ત(pipe->buf);
    મુક્ત(pipe);
    મુક્ત(in);
    પાછા ખાલી;
}

સંરચના usb_pipe *
xhci_realloc_pipe(સંરચના usbdevice_s *usbdev, સંરચના usb_pipe *upipe
                  , સંરચના usb_endpoint_descriptor *epdesc)
{
    જો (!CONFIG_USB_XHCI)
        પાછા ખાલી;
    જો (!epdesc) {
        usb_add_freelist(upipe);
        પાછા ખાલી;
    }
    જો (!upipe)
        પાછા xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    પૂર્ણાંક oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    સંરચના xhci_pipe *pipe = container_of(upipe, સંરચના xhci_pipe, pipe);
    સંરચના usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, સંરચના usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    જો (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        પાછા upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    સંરચના xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    જો (!in)
        પાછા upipe;
    in->add = (1 << 1);
    સંરચના xhci_epctx *ep = (શૂન્ય*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    પૂર્ણાંક cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    જો (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    મુક્ત(in);

    પાછા upipe;
}

// Submit a USB "setup" message request to the pipe's ring
સ્થિર શૂન્ય xhci_xfer_setup(સંરચના xhci_pipe *pipe, પૂર્ણાંક dir, શૂન્ય *cmd
                            , શૂન્ય *data, પૂર્ણાંક datalen)
{
    સંરચના usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, સંરચના usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    જો (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, ખાલી, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
સ્થિર શૂન્ય xhci_xfer_normal(સંરચના xhci_pipe *pipe,
                             શૂન્ય *data, પૂર્ણાંક datalen)
{
    સંરચના usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, સંરચના usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

પૂર્ણાંક
xhci_send_pipe(સંરચના usb_pipe *p, પૂર્ણાંક dir, અચલ શૂન્ય *cmd
               , શૂન્ય *data, પૂર્ણાંક datalen)
{
    જો (!CONFIG_USB_XHCI)
        પાછા -1;
    સંરચના xhci_pipe *pipe = container_of(p, સંરચના xhci_pipe, pipe);
    સંરચના usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, સંરચના usb_xhci_s, usb);

    જો (cmd) {
        અચલ સંરચના usb_ctrlrequest *req = cmd;
        જો (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            પાછા 0;
        xhci_xfer_setup(pipe, dir, (શૂન્ય*)req, data, datalen);
    } નહીંતર {
        xhci_xfer_normal(pipe, data, datalen);
    }

    પૂર્ણાંક cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    જો (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        પાછા -1;
    }

    પાછા 0;
}

પૂર્ણાંક VISIBLE32FLAT
xhci_poll_intr(સંરચના usb_pipe *p, શૂન્ય *data)
{
    જો (!CONFIG_USB_XHCI)
        પાછા -1;

    સંરચના xhci_pipe *pipe = container_of(p, સંરચના xhci_pipe, pipe);
    સંરચના usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, સંરચના usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    શૂન્ય *buf = pipe->buf;
    પૂર્ણાંક bufused = pipe->bufused;

    જો (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        પાછા -1;
    }

    xhci_process_events(xhci);
    જો (xhci_ring_busy(&pipe->reqs))
        પાછા -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    પાછા 0;
}
