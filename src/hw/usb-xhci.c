// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // memalign_low
#அந்தர்பாவ "memmap.h" // PAGE_SIZE
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#அந்தர்பாவ "pci_regs.h" // PCI_BASE_ADDRESS_0
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-xhci.h" // struct ehci_qh
#அந்தர்பாவ "util.h" // timer_calc
#அந்தர்பாவ "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_பொருட்கள்          16
#define XHCI_RING_SIZE           (XHCI_RING_பொருட்கள்*அளவு(கட்டமைப்பு xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((கட்டமைப்பு xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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
#define XHCI_PORTSC_SPEED_கீழ்    (2<<10)
#define XHCI_PORTSC_SPEED_உயர்வு   (3<<10)
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

வகைவரை எண்ணிக்கை TRBType {
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

வகைவரை எண்ணிக்கை TRBCCode {
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

எண்ணிக்கை {
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

கட்டமைப்பு xhci_ring {
    கட்டமைப்பு xhci_trb      ring[XHCI_RING_பொருட்கள்];
    கட்டமைப்பு xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    கட்டமைப்பு mutex_s       lock;
};

கட்டமைப்பு xhci_portmap {
    u8 start;
    u8 மொத்தம்;
};

கட்டமைப்பு usb_xhci_s {
    கட்டமைப்பு usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    கட்டமைப்பு xhci_portmap  usb2;
    கட்டமைப்பு xhci_portmap  usb3;

    /* xhci registers */
    கட்டமைப்பு xhci_caps     *caps;
    கட்டமைப்பு xhci_op       *op;
    கட்டமைப்பு xhci_pr       *pr;
    கட்டமைப்பு xhci_ir       *ir;
    கட்டமைப்பு xhci_db       *db;

    /* xhci data structures */
    கட்டமைப்பு xhci_devlist  *devs;
    கட்டமைப்பு xhci_ring     *cmds;
    கட்டமைப்பு xhci_ring     *evts;
    கட்டமைப்பு xhci_er_seg   *eseg;
};

கட்டமைப்பு xhci_pipe {
    கட்டமைப்பு xhci_ring     reqs;

    கட்டமைப்பு usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    வெறுமை                 *buf;
    முழு                  bufused;
};

// --------------------------------------------------------------
// tables

நிலையான மாறிலி எழுத்து *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

நிலையான மாறிலி முழு speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

நிலையான மாறிலி முழு speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

நிலையான முழு wait_bit(u32 *reg, u32 mask, முழு மதிப்பு, u32 timeout)
{
    u32 end = timer_calc(timeout);

    போது ((readl(reg) & mask) != மதிப்பு) {
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }
    திரும்பு 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
நிலையான வெறுமை
xhci_print_port_state(முழு loglevel, மாறிலி எழுத்து *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

நிலையான முழு
xhci_hub_detect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_xhci_s *xhci = container_of(hub->cntl, கட்டமைப்பு usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    திரும்பு (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
நிலையான முழு
xhci_hub_reset(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_xhci_s *xhci = container_of(hub->cntl, கட்டமைப்பு usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    எனில் (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        திரும்பு -1;

    தேர்வு (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    நிலை PLS_U0:
        // A USB3 port - controller automatically performs reset
        நிறுத்து;
    நிலை PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        நிறுத்து;
    இயல்பு:
        திரும்பு -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    க்கு (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        எனில் (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            திரும்பு -1;
        எனில் (portsc & XHCI_PORTSC_PED)
            // Reset complete
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }

    முழு rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    திரும்பு rc;
}

நிலையான முழு
xhci_hub_portmap(கட்டமைப்பு usbhub_s *hub, u32 vport)
{
    கட்டமைப்பு usb_xhci_s *xhci = container_of(hub->cntl, கட்டமைப்பு usb_xhci_s, usb);
    u32 pport = vport + 1;

    எனில் (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.மொத்தம்)
        pport = vport + 2 - xhci->usb3.start;

    எனில் (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.மொத்தம்)
        pport = vport + 2 - xhci->usb2.start;

    திரும்பு pport;
}

நிலையான வெறுமை
xhci_hub_disconnect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

நிலையான கட்டமைப்பு usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
நிலையான முழு
xhci_check_ports(கட்டமைப்பு usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    கட்டமைப்பு usbhub_s hub;
    memset(&hub, 0, அளவு(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    திரும்பு hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

நிலையான வெறுமை
xhci_free_pipes(கட்டமைப்பு usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

நிலையான வெறுமை
configure_xhci(வெறுமை *data)
{
    கட்டமைப்பு usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_உயர்வு(64, அளவு(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_உயர்வு(64, அளவு(*xhci->eseg));
    xhci->cmds = memalign_உயர்வு(XHCI_RING_SIZE, அளவு(*xhci->cmds));
    xhci->evts = memalign_உயர்வு(XHCI_RING_SIZE, அளவு(*xhci->evts));
    எனில் (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        செல் fail;
    }
    memset(xhci->devs, 0, அளவு(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, அளவு(*xhci->cmds));
    memset(xhci->evts, 0, அளவு(*xhci->evts));
    memset(xhci->eseg, 0, அளவு(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    எனில் (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        எனில் (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            செல் fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    எனில் (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        செல் fail;
    எனில் (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        செல் fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_கீழ், (u32)xhci->devs);
    writel(&xhci->op->dcbaap_உயர்வு, 0);
    writel(&xhci->op->crcr_கீழ், (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_உயர்வு, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_கீழ் = (u32)xhci->evts;
    xhci->eseg->ptr_உயர்வு = 0;
    xhci->eseg->size = XHCI_RING_பொருட்கள்;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_கீழ், (u32)xhci->evts);
    writel(&xhci->ir->erdp_உயர்வு, 0);
    writel(&xhci->ir->erstba_கீழ், (u32)xhci->eseg);
    writel(&xhci->ir->erstba_உயர்வு, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    எனில் (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_உயர்வு(64, அளவு(*spba) * spb);
        வெறுமை *pad = memalign_உயர்வு(PAGE_SIZE, PAGE_SIZE * spb);
        எனில் (!spba || !pad) {
            warn_noalloc();
            விடுவி(spba);
            விடுவி(pad);
            செல் fail;
        }
        முழு i;
        க்கு (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_கீழ் = (u32)spba;
        xhci->devs[0].ptr_உயர்வு = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    முழு மொத்தம் = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    எனில் (மொத்தம்)
        // Success
        திரும்பு;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    விடுவி(xhci->eseg);
    விடுவி(xhci->evts);
    விடுவி(xhci->cmds);
    விடுவி(xhci->devs);
    விடுவி(xhci);
}

நிலையான கட்டமைப்பு usb_xhci_s*
xhci_controller_setup(வெறுமை *baseaddr)
{
    கட்டமைப்பு usb_xhci_s *xhci = malloc_உயர்வு(அளவு(*xhci));
    எனில் (!xhci) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(xhci, 0, அளவு(*xhci));
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

    எனில் (xhci->xcap) {
        u32 off;
        வெறுமை *addr = baseaddr + xhci->xcap;
        செய் {
            கட்டமைப்பு xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            தேர்வு (cap & 0xff) {
            நிலை 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 மொத்தம் = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , மொத்தம், start
                        , ports >> 16);
                எனில் (name == 0x20425355 /* "USB " */) {
                    எனில் (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.மொத்தம் = மொத்தம்;
                    }
                    எனில் (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.மொத்தம் = மொத்தம்;
                    }
                }
                நிறுத்து;
            இயல்பு:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                நிறுத்து;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } போது (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    எனில் (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        விடுவி(xhci);
        திரும்பு வெற்று;
    }

    திரும்பு xhci;
}

நிலையான வெறுமை
xhci_controller_setup_pci(கட்டமைப்பு pci_device *pci)
{
    கட்டமைப்பு usb_xhci_s *xhci;
    வெறுமை *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    எனில் (!baseaddr)
        திரும்பு;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    எனில் (!xhci)
        திரும்பு;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

நிலையான வெறுமை
xhci_controller_setup_acpi(கட்டமைப்பு acpi_device *dev)
{
    கட்டமைப்பு usb_xhci_s *xhci;
    u64 mem, unused;
    வெறுமை *baseaddr;

    எனில் (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        திரும்பு;
    எனில் (mem >= 0x100000000ll)
        திரும்பு;

    baseaddr = (வெறுமை*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    எனில் (!xhci)
        திரும்பு;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

வெறுமை
xhci_setup(வெறுமை)
{
    எனில் (! CONFIG_USB_XHCI)
        திரும்பு;

    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    கட்டமைப்பு acpi_device *dev;
    க்கு (dev = acpi_dsdt_find_eisaid(வெற்று, xhci_eisaid);
         dev != வெற்று;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
நிலையான வெறுமை xhci_doorbell(கட்டமைப்பு usb_xhci_s *xhci, u32 slotid, u32 மதிப்பு)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, மதிப்பு);
    கட்டமைப்பு xhci_db *db = xhci->db;
    வெறுமை *addr = &db[slotid].doorbell;
    writel(addr, மதிப்பு);
}

// Dequeue events on the XHCI command ring generated by the hardware
நிலையான வெறுமை xhci_process_events(கட்டமைப்பு usb_xhci_s *xhci)
{
    கட்டமைப்பு xhci_ring *evts = xhci->evts;

    க்கு (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        கட்டமைப்பு xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        எனில் ((control & TRB_C) != (cs ? 1 : 0))
            திரும்பு;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        தேர்வு (evt_type) {
        நிலை ER_TRANSFER:
        நிலை ER_COMMAND_COMPLETE:
        {
            கட்டமைப்பு xhci_trb  *rtrb = (வெறுமை*)etrb->ptr_கீழ்;
            கட்டமைப்பு xhci_ring *ring = XHCI_RING(rtrb);
            கட்டமைப்பு xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, அளவு(*etrb));
            ring->eidx = eidx;
            நிறுத்து;
        }
        நிலை ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_கீழ் >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            நிறுத்து;
        }
        இயல்பு:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            நிறுத்து;
        }

        /* move ring index, notify xhci */
        nidx++;
        எனில் (nidx == XHCI_RING_பொருட்கள்) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        கட்டமைப்பு xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_கீழ், erdp);
        writel(&ir->erdp_உயர்வு, 0);
    }
}

// Check if a ring has any pending TRBs
நிலையான முழு xhci_ring_busy(கட்டமைப்பு xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    திரும்பு (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
நிலையான முழு xhci_event_wait(கட்டமைப்பு usb_xhci_s *xhci,
                           கட்டமைப்பு xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    க்கு (;;) {
        xhci_process_events(xhci);
        எனில் (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            திரும்பு (status >> 24) & 0xff;
        }
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
நிலையான வெறுமை xhci_trb_fill(கட்டமைப்பு xhci_ring *ring
                          , வெறுமை *data, u32 xferlen, u32 flags)
{
    கட்டமைப்பு xhci_trb *dst = &ring->ring[ring->nidx];
    எனில் (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_கீழ், data, xferlen);
    } இல்லையெனில் {
        dst->ptr_கீழ் = (u32)data;
        dst->ptr_உயர்வு = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
நிலையான வெறுமை xhci_trb_queue(கட்டமைப்பு xhci_ring *ring,
                           வெறுமை *data, u32 xferlen, u32 flags)
{
    எனில் (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
நிலையான முழு xhci_cmd_submit(கட்டமைப்பு usb_xhci_s *xhci, கட்டமைப்பு xhci_inctx *inctx
                           , u32 flags)
{
    எனில் (inctx) {
        கட்டமைப்பு xhci_slotctx *slot = (வெறுமை*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        எனில் (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            திரும்பு -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    முழு rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    திரும்பு rc;
}

நிலையான முழு xhci_cmd_enable_slot(கட்டமைப்பு usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    முழு cc = xhci_cmd_submit(xhci, வெற்று, CR_ENABLE_SLOT << 10);
    எனில் (cc != CC_SUCCESS)
        திரும்பு -1;
    திரும்பு (xhci->cmds->evt.control >> 24) & 0xff;
}

நிலையான முழு xhci_cmd_disable_slot(கட்டமைப்பு usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    திரும்பு xhci_cmd_submit(xhci, வெற்று, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

நிலையான முழு xhci_cmd_address_device(கட்டமைப்பு usb_xhci_s *xhci, u32 slotid
                                   , கட்டமைப்பு xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    திரும்பு xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

நிலையான முழு xhci_cmd_configure_endpoint(கட்டமைப்பு usb_xhci_s *xhci, u32 slotid
                                       , கட்டமைப்பு xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    திரும்பு xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

நிலையான முழு xhci_cmd_evaluate_context(கட்டமைப்பு usb_xhci_s *xhci, u32 slotid
                                     , கட்டமைப்பு xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    திரும்பு xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

நிலையான கட்டமைப்பு xhci_inctx *
xhci_alloc_inctx(கட்டமைப்பு usbdevice_s *usbdev, முழு maxepid)
{
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_xhci_s, usb);
    முழு size = (அளவு(கட்டமைப்பு xhci_inctx) * 33) << xhci->context64;
    கட்டமைப்பு xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    எனில் (!in) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(in, 0, size);

    கட்டமைப்பு xhci_slotctx *slot = (வெறுமை*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    கட்டமைப்பு usbdevice_s *hubdev = usbdev->hub->usbdev;
    எனில் (hubdev) {
        எனில் (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            கட்டமைப்பு xhci_pipe *hpipe = container_of(
                hubdev->defpipe, கட்டமைப்பு xhci_pipe, pipe);
            எனில் (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } இல்லையெனில் {
                கட்டமைப்பு xhci_slotctx *hslot = (வெறுமை*)xhci->devs[hpipe->slotid].ptr_கீழ்;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        போது (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    திரும்பு in;
}

நிலையான முழு xhci_config_hub(கட்டமைப்பு usbhub_s *hub)
{
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        hub->cntl, கட்டமைப்பு usb_xhci_s, usb);
    கட்டமைப்பு xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, கட்டமைப்பு xhci_pipe, pipe);
    கட்டமைப்பு xhci_slotctx *hdslot = (வெறுமை*)xhci->devs[pipe->slotid].ptr_கீழ்;
    எனில் ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        திரும்பு 0;
    கட்டமைப்பு xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    எனில் (!in)
        திரும்பு -1;
    in->add = 0x01;
    கட்டமைப்பு xhci_slotctx *slot = (வெறுமை*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    முழு cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    விடுவி(in);
    எனில் (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        திரும்பு -1;
    }
    திரும்பு 0;
}

நிலையான கட்டமைப்பு usb_pipe *
xhci_alloc_pipe(கட்டமைப்பு usbdevice_s *usbdev
                , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_xhci_s, usb);
    கட்டமைப்பு xhci_pipe *pipe;
    u32 epid;

    எனில் (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } இல்லையெனில் {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    எனில் (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_உயர்வு(XHCI_RING_SIZE, அளவு(*pipe));
    இல்லையெனில்
        pipe = memalign_கீழ்(XHCI_RING_SIZE, அளவு(*pipe));
    எனில் (!pipe) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(pipe, 0, அளவு(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    எனில் (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_உயர்வு(pipe->pipe.maxpacket);
        எனில் (!pipe->buf) {
            warn_noalloc();
            விடுவி(pipe);
            திரும்பு வெற்று;
        }
    }

    // Allocate input context and initialize endpoint info.
    கட்டமைப்பு xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    எனில் (!in)
        செல் fail;
    in->add = 0x01 | (1 << epid);
    கட்டமைப்பு xhci_epctx *ep = (வெறுமை*)&in[(pipe->epid+1) << xhci->context64];
    எனில் (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    எனில் (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_கீழ்  = (u32)&pipe->reqs.ring[0];
    ep->deq_கீழ்  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    எனில் (pipe->epid == 1) {
        எனில் (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            முழு ret = xhci_config_hub(usbdev->hub);
            எனில் (ret)
                செல் fail;
        }
        // Enable slot.
        u32 size = (அளவு(கட்டமைப்பு xhci_slotctx) * 32) << xhci->context64;
        கட்டமைப்பு xhci_slotctx *dev = memalign_உயர்வு(1024 << xhci->context64, size);
        எனில் (!dev) {
            warn_noalloc();
            செல் fail;
        }
        முழு slotid = xhci_cmd_enable_slot(xhci);
        எனில் (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            விடுவி(dev);
            செல் fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_கீழ் = (u32)dev;
        xhci->devs[slotid].ptr_உயர்வு = 0;

        // Send set_address command.
        முழு cc = xhci_cmd_address_device(xhci, slotid, in);
        எனில் (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            எனில் (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                செல் fail;
            }
            xhci->devs[slotid].ptr_கீழ் = 0;
            விடுவி(dev);
            செல் fail;
        }
        pipe->slotid = slotid;
    } இல்லையெனில் {
        கட்டமைப்பு xhci_pipe *defpipe = container_of(
            usbdev->defpipe, கட்டமைப்பு xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        முழு cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        எனில் (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            செல் fail;
        }
    }
    விடுவி(in);
    திரும்பு &pipe->pipe;

fail:
    விடுவி(pipe->buf);
    விடுவி(pipe);
    விடுவி(in);
    திரும்பு வெற்று;
}

கட்டமைப்பு usb_pipe *
xhci_realloc_pipe(கட்டமைப்பு usbdevice_s *usbdev, கட்டமைப்பு usb_pipe *upipe
                  , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    எனில் (!CONFIG_USB_XHCI)
        திரும்பு வெற்று;
    எனில் (!epdesc) {
        usb_add_freelist(upipe);
        திரும்பு வெற்று;
    }
    எனில் (!upipe)
        திரும்பு xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    முழு oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    கட்டமைப்பு xhci_pipe *pipe = container_of(upipe, கட்டமைப்பு xhci_pipe, pipe);
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, கட்டமைப்பு usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    எனில் (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        திரும்பு upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    கட்டமைப்பு xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    எனில் (!in)
        திரும்பு upipe;
    in->add = (1 << 1);
    கட்டமைப்பு xhci_epctx *ep = (வெறுமை*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    முழு cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    எனில் (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    விடுவி(in);

    திரும்பு upipe;
}

// Submit a USB "setup" message request to the pipe's ring
நிலையான வெறுமை xhci_xfer_setup(கட்டமைப்பு xhci_pipe *pipe, முழு dir, வெறுமை *cmd
                            , வெறுமை *data, முழு datalen)
{
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, கட்டமைப்பு usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    எனில் (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, வெற்று, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
நிலையான வெறுமை xhci_xfer_normal(கட்டமைப்பு xhci_pipe *pipe,
                             வெறுமை *data, முழு datalen)
{
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, கட்டமைப்பு usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

முழு
xhci_send_pipe(கட்டமைப்பு usb_pipe *p, முழு dir, மாறிலி வெறுமை *cmd
               , வெறுமை *data, முழு datalen)
{
    எனில் (!CONFIG_USB_XHCI)
        திரும்பு -1;
    கட்டமைப்பு xhci_pipe *pipe = container_of(p, கட்டமைப்பு xhci_pipe, pipe);
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, கட்டமைப்பு usb_xhci_s, usb);

    எனில் (cmd) {
        மாறிலி கட்டமைப்பு usb_ctrlrequest *req = cmd;
        எனில் (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            திரும்பு 0;
        xhci_xfer_setup(pipe, dir, (வெறுமை*)req, data, datalen);
    } இல்லையெனில் {
        xhci_xfer_normal(pipe, data, datalen);
    }

    முழு cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    எனில் (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        திரும்பு -1;
    }

    திரும்பு 0;
}

முழு VISIBLE32FLAT
xhci_poll_intr(கட்டமைப்பு usb_pipe *p, வெறுமை *data)
{
    எனில் (!CONFIG_USB_XHCI)
        திரும்பு -1;

    கட்டமைப்பு xhci_pipe *pipe = container_of(p, கட்டமைப்பு xhci_pipe, pipe);
    கட்டமைப்பு usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, கட்டமைப்பு usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    வெறுமை *buf = pipe->buf;
    முழு bufused = pipe->bufused;

    எனில் (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        திரும்பு -1;
    }

    xhci_process_events(xhci);
    எனில் (xhci_ring_busy(&pipe->reqs))
        திரும்பு -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    திரும்பு 0;
}
