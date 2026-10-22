// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // memalign_low
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-xhci.h" // struct ehci_qh
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc
#ಅನ್ತರ್ಭಾವ "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_ವಸ್ತುಗಳು          16
#define XHCI_RING_SIZE           (XHCI_RING_ವಸ್ತುಗಳು*ಪರಿಮಾಣ(ಸಂರಚನಾ xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((ಸಂರಚನಾ xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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
#define XHCI_PORTSC_SPEED_ಕೆಳಗಿನ    (2<<10)
#define XHCI_PORTSC_SPEED_ಮೇಲಿನ   (3<<10)
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

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ TRBType {
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

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ TRBCCode {
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

ಗಣನಾ {
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

ಸಂರಚನಾ xhci_ring {
    ಸಂರಚನಾ xhci_trb      ring[XHCI_RING_ವಸ್ತುಗಳು];
    ಸಂರಚನಾ xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    ಸಂರಚನಾ mutex_s       lock;
};

ಸಂರಚನಾ xhci_portmap {
    u8 start;
    u8 ಎಣಿಕೆ;
};

ಸಂರಚನಾ usb_xhci_s {
    ಸಂರಚನಾ usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    ಸಂರಚನಾ xhci_portmap  usb2;
    ಸಂರಚನಾ xhci_portmap  usb3;

    /* xhci registers */
    ಸಂರಚನಾ xhci_caps     *caps;
    ಸಂರಚನಾ xhci_op       *op;
    ಸಂರಚನಾ xhci_pr       *pr;
    ಸಂರಚನಾ xhci_ir       *ir;
    ಸಂರಚನಾ xhci_db       *db;

    /* xhci data structures */
    ಸಂರಚನಾ xhci_devlist  *devs;
    ಸಂರಚನಾ xhci_ring     *cmds;
    ಸಂರಚನಾ xhci_ring     *evts;
    ಸಂರಚನಾ xhci_er_seg   *eseg;
};

ಸಂರಚನಾ xhci_pipe {
    ಸಂರಚನಾ xhci_ring     reqs;

    ಸಂರಚನಾ usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    ಶೂನ್ಯ                 *buf;
    ಪೂರ್ಣಾಙ್ಕ                  bufused;
};

// --------------------------------------------------------------
// tables

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wait_bit(u32 *reg, u32 mask, ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ, u32 timeout)
{
    u32 end = timer_calc(timeout);

    ಯಾವತ್ ((readl(reg) & mask) != ಮೌಲ್ಯ) {
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
ಸ್ಥಿರ ಶೂನ್ಯ
xhci_print_port_state(ಪೂರ್ಣಾಙ್ಕ loglevel, ಧ್ರುವ ವರ್ಣ *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
xhci_hub_detect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(hub->cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ಪ್ರತ್ಯಾಗಮ (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
xhci_hub_reset(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(hub->cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    ಯದಿ (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        ಪ್ರತ್ಯಾಗಮ -1;

    ವಿಕಲ್ಪ (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    ಪಕ್ಷ PLS_U0:
        // A USB3 port - controller automatically performs reset
        ಭಙ್ಗ;
    ಪಕ್ಷ PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    ಪ್ರತಿ (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        ಯದಿ (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            ಪ್ರತ್ಯಾಗಮ -1;
        ಯದಿ (portsc & XHCI_PORTSC_PED)
            // Reset complete
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }

    ಪೂರ್ಣಾಙ್ಕ rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
xhci_hub_portmap(ಸಂರಚನಾ usbhub_s *hub, u32 vport)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(hub->cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    u32 pport = vport + 1;

    ಯದಿ (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.ಎಣಿಕೆ)
        pport = vport + 2 - xhci->usb3.start;

    ಯದಿ (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.ಎಣಿಕೆ)
        pport = vport + 2 - xhci->usb2.start;

    ಪ್ರತ್ಯಾಗಮ pport;
}

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_hub_disconnect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

ಸ್ಥಿರ ಸಂರಚನಾ usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
xhci_check_ports(ಸಂರಚನಾ usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    ಸಂರಚನಾ usbhub_s hub;
    memset(&hub, 0, ಪರಿಮಾಣ(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    ಪ್ರತ್ಯಾಗಮ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_free_pipes(ಸಂರಚನಾ usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

ಸ್ಥಿರ ಶೂನ್ಯ
configure_xhci(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_ಮೇಲಿನ(64, ಪರಿಮಾಣ(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ಮೇಲಿನ(64, ಪರಿಮಾಣ(*xhci->eseg));
    xhci->cmds = memalign_ಮೇಲಿನ(XHCI_RING_SIZE, ಪರಿಮಾಣ(*xhci->cmds));
    xhci->evts = memalign_ಮೇಲಿನ(XHCI_RING_SIZE, ಪರಿಮಾಣ(*xhci->evts));
    ಯದಿ (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        ಗಚ್ಛ fail;
    }
    memset(xhci->devs, 0, ಪರಿಮಾಣ(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, ಪರಿಮಾಣ(*xhci->cmds));
    memset(xhci->evts, 0, ಪರಿಮಾಣ(*xhci->evts));
    memset(xhci->eseg, 0, ಪರಿಮಾಣ(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    ಯದಿ (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        ಯದಿ (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            ಗಚ್ಛ fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    ಯದಿ (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        ಗಚ್ಛ fail;
    ಯದಿ (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        ಗಚ್ಛ fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_ಕೆಳಗಿನ, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ಮೇಲಿನ, 0);
    writel(&xhci->op->crcr_ಕೆಳಗಿನ, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ಮೇಲಿನ, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_ಕೆಳಗಿನ = (u32)xhci->evts;
    xhci->eseg->ptr_ಮೇಲಿನ = 0;
    xhci->eseg->size = XHCI_RING_ವಸ್ತುಗಳು;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_ಕೆಳಗಿನ, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ಮೇಲಿನ, 0);
    writel(&xhci->ir->erstba_ಕೆಳಗಿನ, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ಮೇಲಿನ, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    ಯದಿ (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ಮೇಲಿನ(64, ಪರಿಮಾಣ(*spba) * spb);
        ಶೂನ್ಯ *pad = memalign_ಮೇಲಿನ(PAGE_SIZE, PAGE_SIZE * spb);
        ಯದಿ (!spba || !pad) {
            warn_noalloc();
            ಮುಕ್ತ(spba);
            ಮುಕ್ತ(pad);
            ಗಚ್ಛ fail;
        }
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_ಕೆಳಗಿನ = (u32)spba;
        xhci->devs[0].ptr_ಮೇಲಿನ = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    ಯದಿ (ಎಣಿಕೆ)
        // Success
        ಪ್ರತ್ಯಾಗಮ;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    ಮುಕ್ತ(xhci->eseg);
    ಮುಕ್ತ(xhci->evts);
    ಮುಕ್ತ(xhci->cmds);
    ಮುಕ್ತ(xhci->devs);
    ಮುಕ್ತ(xhci);
}

ಸ್ಥಿರ ಸಂರಚನಾ usb_xhci_s*
xhci_controller_setup(ಶೂನ್ಯ *baseaddr)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*xhci));
    ಯದಿ (!xhci) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(xhci, 0, ಪರಿಮಾಣ(*xhci));
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

    ಯದಿ (xhci->xcap) {
        u32 off;
        ಶೂನ್ಯ *addr = baseaddr + xhci->xcap;
        ಕುರು {
            ಸಂರಚನಾ xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            ವಿಕಲ್ಪ (cap & 0xff) {
            ಪಕ್ಷ 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 ಎಣಿಕೆ = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , ಎಣಿಕೆ, start
                        , ports >> 16);
                ಯದಿ (name == 0x20425355 /* "USB " */) {
                    ಯದಿ (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.ಎಣಿಕೆ = ಎಣಿಕೆ;
                    }
                    ಯದಿ (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.ಎಣಿಕೆ = ಎಣಿಕೆ;
                    }
                }
                ಭಙ್ಗ;
            ಸ್ವಭಾವ:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                ಭಙ್ಗ;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } ಯಾವತ್ (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    ಯದಿ (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        ಮುಕ್ತ(xhci);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    ಪ್ರತ್ಯಾಗಮ xhci;
}

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_controller_setup_pci(ಸಂರಚನಾ pci_device *pci)
{
    ಸಂರಚನಾ usb_xhci_s *xhci;
    ಶೂನ್ಯ *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ಯದಿ (!baseaddr)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    ಯದಿ (!xhci)
        ಪ್ರತ್ಯಾಗಮ;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_controller_setup_acpi(ಸಂರಚನಾ acpi_device *dev)
{
    ಸಂರಚನಾ usb_xhci_s *xhci;
    u64 mem, unused;
    ಶೂನ್ಯ *baseaddr;

    ಯದಿ (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (mem >= 0x100000000ll)
        ಪ್ರತ್ಯಾಗಮ;

    baseaddr = (ಶೂನ್ಯ*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    ಯದಿ (!xhci)
        ಪ್ರತ್ಯಾಗಮ;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

ಶೂನ್ಯ
xhci_setup(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_XHCI)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    ಸಂರಚನಾ acpi_device *dev;
    ಪ್ರತಿ (dev = acpi_dsdt_find_eisaid(ರಿಕ್ತ, xhci_eisaid);
         dev != ರಿಕ್ತ;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_doorbell(ಸಂರಚನಾ usb_xhci_s *xhci, u32 slotid, u32 ಮೌಲ್ಯ)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, ಮೌಲ್ಯ);
    ಸಂರಚನಾ xhci_db *db = xhci->db;
    ಶೂನ್ಯ *addr = &db[slotid].doorbell;
    writel(addr, ಮೌಲ್ಯ);
}

// Dequeue events on the XHCI command ring generated by the hardware
ಸ್ಥಿರ ಶೂನ್ಯ xhci_process_events(ಸಂರಚನಾ usb_xhci_s *xhci)
{
    ಸಂರಚನಾ xhci_ring *evts = xhci->evts;

    ಪ್ರತಿ (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        ಸಂರಚನಾ xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        ಯದಿ ((control & TRB_C) != (cs ? 1 : 0))
            ಪ್ರತ್ಯಾಗಮ;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        ವಿಕಲ್ಪ (evt_type) {
        ಪಕ್ಷ ER_TRANSFER:
        ಪಕ್ಷ ER_COMMAND_COMPLETE:
        {
            ಸಂರಚನಾ xhci_trb  *rtrb = (ಶೂನ್ಯ*)etrb->ptr_ಕೆಳಗಿನ;
            ಸಂರಚನಾ xhci_ring *ring = XHCI_RING(rtrb);
            ಸಂರಚನಾ xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, ಪರಿಮಾಣ(*etrb));
            ring->eidx = eidx;
            ಭಙ್ಗ;
        }
        ಪಕ್ಷ ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_ಕೆಳಗಿನ >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            ಭಙ್ಗ;
        }
        ಸ್ವಭಾವ:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            ಭಙ್ಗ;
        }

        /* move ring index, notify xhci */
        nidx++;
        ಯದಿ (nidx == XHCI_RING_ವಸ್ತುಗಳು) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        ಸಂರಚನಾ xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_ಕೆಳಗಿನ, erdp);
        writel(&ir->erdp_ಮೇಲಿನ, 0);
    }
}

// Check if a ring has any pending TRBs
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_ring_busy(ಸಂರಚನಾ xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    ಪ್ರತ್ಯಾಗಮ (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_event_wait(ಸಂರಚನಾ usb_xhci_s *xhci,
                           ಸಂರಚನಾ xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    ಪ್ರತಿ (;;) {
        xhci_process_events(xhci);
        ಯದಿ (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            ಪ್ರತ್ಯಾಗಮ (status >> 24) & 0xff;
        }
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_trb_fill(ಸಂರಚನಾ xhci_ring *ring
                          , ಶೂನ್ಯ *data, u32 xferlen, u32 flags)
{
    ಸಂರಚನಾ xhci_trb *dst = &ring->ring[ring->nidx];
    ಯದಿ (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_ಕೆಳಗಿನ, data, xferlen);
    } ಅನ್ಯಥಾ {
        dst->ptr_ಕೆಳಗಿನ = (u32)data;
        dst->ptr_ಮೇಲಿನ = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ಸ್ಥಿರ ಶೂನ್ಯ xhci_trb_queue(ಸಂರಚನಾ xhci_ring *ring,
                           ಶೂನ್ಯ *data, u32 xferlen, u32 flags)
{
    ಯದಿ (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_cmd_submit(ಸಂರಚನಾ usb_xhci_s *xhci, ಸಂರಚನಾ xhci_inctx *inctx
                           , u32 flags)
{
    ಯದಿ (inctx) {
        ಸಂರಚನಾ xhci_slotctx *slot = (ಶೂನ್ಯ*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        ಯದಿ (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    ಪೂರ್ಣಾಙ್ಕ rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_cmd_enable_slot(ಸಂರಚನಾ usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    ಪೂರ್ಣಾಙ್ಕ cc = xhci_cmd_submit(xhci, ರಿಕ್ತ, CR_ENABLE_SLOT << 10);
    ಯದಿ (cc != CC_SUCCESS)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ (xhci->cmds->evt.control >> 24) & 0xff;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_cmd_disable_slot(ಸಂರಚನಾ usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ಪ್ರತ್ಯಾಗಮ xhci_cmd_submit(xhci, ರಿಕ್ತ, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_cmd_address_device(ಸಂರಚನಾ usb_xhci_s *xhci, u32 slotid
                                   , ಸಂರಚನಾ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ಪ್ರತ್ಯಾಗಮ xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_cmd_configure_endpoint(ಸಂರಚನಾ usb_xhci_s *xhci, u32 slotid
                                       , ಸಂರಚನಾ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ಪ್ರತ್ಯಾಗಮ xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_cmd_evaluate_context(ಸಂರಚನಾ usb_xhci_s *xhci, u32 slotid
                                     , ಸಂರಚನಾ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    ಪ್ರತ್ಯಾಗಮ xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ಸ್ಥಿರ ಸಂರಚನಾ xhci_inctx *
xhci_alloc_inctx(ಸಂರಚನಾ usbdevice_s *usbdev, ಪೂರ್ಣಾಙ್ಕ maxepid)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    ಪೂರ್ಣಾಙ್ಕ size = (ಪರಿಮಾಣ(ಸಂರಚನಾ xhci_inctx) * 33) << xhci->context64;
    ಸಂರಚನಾ xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    ಯದಿ (!in) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(in, 0, size);

    ಸಂರಚನಾ xhci_slotctx *slot = (ಶೂನ್ಯ*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    ಸಂರಚನಾ usbdevice_s *hubdev = usbdev->hub->usbdev;
    ಯದಿ (hubdev) {
        ಯದಿ (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            ಸಂರಚನಾ xhci_pipe *hpipe = container_of(
                hubdev->defpipe, ಸಂರಚನಾ xhci_pipe, pipe);
            ಯದಿ (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } ಅನ್ಯಥಾ {
                ಸಂರಚನಾ xhci_slotctx *hslot = (ಶೂನ್ಯ*)xhci->devs[hpipe->slotid].ptr_ಕೆಳಗಿನ;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        ಯಾವತ್ (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    ಪ್ರತ್ಯಾಗಮ in;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xhci_config_hub(ಸಂರಚನಾ usbhub_s *hub)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        hub->cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    ಸಂರಚನಾ xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, ಸಂರಚನಾ xhci_pipe, pipe);
    ಸಂರಚನಾ xhci_slotctx *hdslot = (ಶೂನ್ಯ*)xhci->devs[pipe->slotid].ptr_ಕೆಳಗಿನ;
    ಯದಿ ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        ಪ್ರತ್ಯಾಗಮ 0;
    ಸಂರಚನಾ xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    ಯದಿ (!in)
        ಪ್ರತ್ಯಾಗಮ -1;
    in->add = 0x01;
    ಸಂರಚನಾ xhci_slotctx *slot = (ಶೂನ್ಯ*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    ಪೂರ್ಣಾಙ್ಕ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    ಮುಕ್ತ(in);
    ಯದಿ (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಸಂರಚನಾ usb_pipe *
xhci_alloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev
                , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    ಸಂರಚನಾ xhci_pipe *pipe;
    u32 epid;

    ಯದಿ (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } ಅನ್ಯಥಾ {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    ಯದಿ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ಮೇಲಿನ(XHCI_RING_SIZE, ಪರಿಮಾಣ(*pipe));
    ಅನ್ಯಥಾ
        pipe = memalign_ಕೆಳಗಿನ(XHCI_RING_SIZE, ಪರಿಮಾಣ(*pipe));
    ಯದಿ (!pipe) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(pipe, 0, ಪರಿಮಾಣ(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ಮೇಲಿನ(pipe->pipe.maxpacket);
        ಯದಿ (!pipe->buf) {
            warn_noalloc();
            ಮುಕ್ತ(pipe);
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        }
    }

    // Allocate input context and initialize endpoint info.
    ಸಂರಚನಾ xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    ಯದಿ (!in)
        ಗಚ್ಛ fail;
    in->add = 0x01 | (1 << epid);
    ಸಂರಚನಾ xhci_epctx *ep = (ಶೂನ್ಯ*)&in[(pipe->epid+1) << xhci->context64];
    ಯದಿ (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    ಯದಿ (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_ಕೆಳಗಿನ  = (u32)&pipe->reqs.ring[0];
    ep->deq_ಕೆಳಗಿನ  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ಯದಿ (pipe->epid == 1) {
        ಯದಿ (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            ಪೂರ್ಣಾಙ್ಕ ret = xhci_config_hub(usbdev->hub);
            ಯದಿ (ret)
                ಗಚ್ಛ fail;
        }
        // Enable slot.
        u32 size = (ಪರಿಮಾಣ(ಸಂರಚನಾ xhci_slotctx) * 32) << xhci->context64;
        ಸಂರಚನಾ xhci_slotctx *dev = memalign_ಮೇಲಿನ(1024 << xhci->context64, size);
        ಯದಿ (!dev) {
            warn_noalloc();
            ಗಚ್ಛ fail;
        }
        ಪೂರ್ಣಾಙ್ಕ slotid = xhci_cmd_enable_slot(xhci);
        ಯದಿ (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            ಮುಕ್ತ(dev);
            ಗಚ್ಛ fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_ಕೆಳಗಿನ = (u32)dev;
        xhci->devs[slotid].ptr_ಮೇಲಿನ = 0;

        // Send set_address command.
        ಪೂರ್ಣಾಙ್ಕ cc = xhci_cmd_address_device(xhci, slotid, in);
        ಯದಿ (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            ಯದಿ (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                ಗಚ್ಛ fail;
            }
            xhci->devs[slotid].ptr_ಕೆಳಗಿನ = 0;
            ಮುಕ್ತ(dev);
            ಗಚ್ಛ fail;
        }
        pipe->slotid = slotid;
    } ಅನ್ಯಥಾ {
        ಸಂರಚನಾ xhci_pipe *defpipe = container_of(
            usbdev->defpipe, ಸಂರಚನಾ xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        ಪೂರ್ಣಾಙ್ಕ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        ಯದಿ (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            ಗಚ್ಛ fail;
        }
    }
    ಮುಕ್ತ(in);
    ಪ್ರತ್ಯಾಗಮ &pipe->pipe;

fail:
    ಮುಕ್ತ(pipe->buf);
    ಮುಕ್ತ(pipe);
    ಮುಕ್ತ(in);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ usb_pipe *
xhci_realloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev, ಸಂರಚನಾ usb_pipe *upipe
                  , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಯದಿ (!CONFIG_USB_XHCI)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಯದಿ (!epdesc) {
        usb_add_freelist(upipe);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    ಯದಿ (!upipe)
        ಪ್ರತ್ಯಾಗಮ xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ಪೂರ್ಣಾಙ್ಕ oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    ಸಂರಚನಾ xhci_pipe *pipe = container_of(upipe, ಸಂರಚನಾ xhci_pipe, pipe);
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ಯದಿ (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        ಪ್ರತ್ಯಾಗಮ upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    ಸಂರಚನಾ xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    ಯದಿ (!in)
        ಪ್ರತ್ಯಾಗಮ upipe;
    in->add = (1 << 1);
    ಸಂರಚನಾ xhci_epctx *ep = (ಶೂನ್ಯ*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    ಪೂರ್ಣಾಙ್ಕ cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    ಯದಿ (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    ಮುಕ್ತ(in);

    ಪ್ರತ್ಯಾಗಮ upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_xfer_setup(ಸಂರಚನಾ xhci_pipe *pipe, ಪೂರ್ಣಾಙ್ಕ dir, ಶೂನ್ಯ *cmd
                            , ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datalen)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    ಯದಿ (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, ರಿಕ್ತ, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_xfer_normal(ಸಂರಚನಾ xhci_pipe *pipe,
                             ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datalen)
{
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

ಪೂರ್ಣಾಙ್ಕ
xhci_send_pipe(ಸಂರಚನಾ usb_pipe *p, ಪೂರ್ಣಾಙ್ಕ dir, ಧ್ರುವ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datalen)
{
    ಯದಿ (!CONFIG_USB_XHCI)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ xhci_pipe *pipe = container_of(p, ಸಂರಚನಾ xhci_pipe, pipe);
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ಸಂರಚನಾ usb_xhci_s, usb);

    ಯದಿ (cmd) {
        ಧ್ರುವ ಸಂರಚನಾ usb_ctrlrequest *req = cmd;
        ಯದಿ (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            ಪ್ರತ್ಯಾಗಮ 0;
        xhci_xfer_setup(pipe, dir, (ಶೂನ್ಯ*)req, data, datalen);
    } ಅನ್ಯಥಾ {
        xhci_xfer_normal(pipe, data, datalen);
    }

    ಪೂರ್ಣಾಙ್ಕ cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    ಯದಿ (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ VISIBLE32FLAT
xhci_poll_intr(ಸಂರಚನಾ usb_pipe *p, ಶೂನ್ಯ *data)
{
    ಯದಿ (!CONFIG_USB_XHCI)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ xhci_pipe *pipe = container_of(p, ಸಂರಚನಾ xhci_pipe, pipe);
    ಸಂರಚನಾ usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ಸಂರಚನಾ usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    ಶೂನ್ಯ *buf = pipe->buf;
    ಪೂರ್ಣಾಙ್ಕ bufused = pipe->bufused;

    ಯದಿ (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    xhci_process_events(xhci);
    ಯದಿ (xhci_ring_busy(&pipe->reqs))
        ಪ್ರತ್ಯಾಗಮ -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    ಪ್ರತ್ಯಾಗಮ 0;
}
