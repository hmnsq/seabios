// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // memalign_low
#അന്തര്ഭാവ "memmap.h" // PAGE_SIZE
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#അന്തര്ഭാവ "pci_regs.h" // PCI_BASE_ADDRESS_0
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-xhci.h" // struct ehci_qh
#അന്തര്ഭാവ "util.h" // timer_calc
#അന്തര്ഭാവ "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_ഇനങ്ങൾ          16
#define XHCI_RING_SIZE           (XHCI_RING_ഇനങ്ങൾ*വലുപ്പം(ഘടന xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((ഘടന xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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
#define XHCI_PORTSC_SPEED_താഴെയുള്ള    (2<<10)
#define XHCI_PORTSC_SPEED_മുകളിലെ   (3<<10)
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

തരനിർവചനം എണ്ണൽ TRBType {
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

തരനിർവചനം എണ്ണൽ TRBCCode {
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

എണ്ണൽ {
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

ഘടന xhci_ring {
    ഘടന xhci_trb      ring[XHCI_RING_ഇനങ്ങൾ];
    ഘടന xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    ഘടന mutex_s       lock;
};

ഘടന xhci_portmap {
    u8 start;
    u8 എണ്ണം;
};

ഘടന usb_xhci_s {
    ഘടന usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    ഘടന xhci_portmap  usb2;
    ഘടന xhci_portmap  usb3;

    /* xhci registers */
    ഘടന xhci_caps     *caps;
    ഘടന xhci_op       *op;
    ഘടന xhci_pr       *pr;
    ഘടന xhci_ir       *ir;
    ഘടന xhci_db       *db;

    /* xhci data structures */
    ഘടന xhci_devlist  *devs;
    ഘടന xhci_ring     *cmds;
    ഘടന xhci_ring     *evts;
    ഘടന xhci_er_seg   *eseg;
};

ഘടന xhci_pipe {
    ഘടന xhci_ring     reqs;

    ഘടന usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    ശൂന്യം                 *buf;
    പൂർണ്ണസംഖ്യ                  bufused;
};

// --------------------------------------------------------------
// tables

സ്ഥിതം സ്ഥിരം അക്ഷരം *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

സ്ഥിതം പൂർണ്ണസംഖ്യ wait_bit(u32 *reg, u32 mask, പൂർണ്ണസംഖ്യ മൂല്യം, u32 timeout)
{
    u32 end = timer_calc(timeout);

    ആകുമ്പോൾ ((readl(reg) & mask) != മൂല്യം) {
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ -1;
        }
        yield();
    }
    തിരികെ 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
സ്ഥിതം ശൂന്യം
xhci_print_port_state(പൂർണ്ണസംഖ്യ loglevel, സ്ഥിരം അക്ഷരം *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
xhci_hub_detect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_xhci_s *xhci = container_of(hub->cntl, ഘടന usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    തിരികെ (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
സ്ഥിതം പൂർണ്ണസംഖ്യ
xhci_hub_reset(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_xhci_s *xhci = container_of(hub->cntl, ഘടന usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    എങ്കിൽ (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        തിരികെ -1;

    തിരഞ്ഞെടുപ്പ് (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    അവസ്ഥ PLS_U0:
        // A USB3 port - controller automatically performs reset
        നിർത്തുക;
    അവസ്ഥ PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        തിരികെ -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    വേണ്ടി (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        എങ്കിൽ (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            തിരികെ -1;
        എങ്കിൽ (portsc & XHCI_PORTSC_PED)
            // Reset complete
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ -1;
        }
        yield();
    }

    പൂർണ്ണസംഖ്യ rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
xhci_hub_portmap(ഘടന usbhub_s *hub, u32 vport)
{
    ഘടന usb_xhci_s *xhci = container_of(hub->cntl, ഘടന usb_xhci_s, usb);
    u32 pport = vport + 1;

    എങ്കിൽ (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.എണ്ണം)
        pport = vport + 2 - xhci->usb3.start;

    എങ്കിൽ (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.എണ്ണം)
        pport = vport + 2 - xhci->usb2.start;

    തിരികെ pport;
}

സ്ഥിതം ശൂന്യം
xhci_hub_disconnect(ഘടന usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

സ്ഥിതം ഘടന usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
സ്ഥിതം പൂർണ്ണസംഖ്യ
xhci_check_ports(ഘടന usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    ഘടന usbhub_s hub;
    memset(&hub, 0, വലുപ്പം(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    തിരികെ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

സ്ഥിതം ശൂന്യം
xhci_free_pipes(ഘടന usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

സ്ഥിതം ശൂന്യം
configure_xhci(ശൂന്യം *data)
{
    ഘടന usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_മുകളിലെ(64, വലുപ്പം(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_മുകളിലെ(64, വലുപ്പം(*xhci->eseg));
    xhci->cmds = memalign_മുകളിലെ(XHCI_RING_SIZE, വലുപ്പം(*xhci->cmds));
    xhci->evts = memalign_മുകളിലെ(XHCI_RING_SIZE, വലുപ്പം(*xhci->evts));
    എങ്കിൽ (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        പോകുക fail;
    }
    memset(xhci->devs, 0, വലുപ്പം(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, വലുപ്പം(*xhci->cmds));
    memset(xhci->evts, 0, വലുപ്പം(*xhci->evts));
    memset(xhci->eseg, 0, വലുപ്പം(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    എങ്കിൽ (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        എങ്കിൽ (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            പോകുക fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    എങ്കിൽ (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        പോകുക fail;
    എങ്കിൽ (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        പോകുക fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_താഴെയുള്ള, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_മുകളിലെ, 0);
    writel(&xhci->op->crcr_താഴെയുള്ള, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_മുകളിലെ, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_താഴെയുള്ള = (u32)xhci->evts;
    xhci->eseg->ptr_മുകളിലെ = 0;
    xhci->eseg->size = XHCI_RING_ഇനങ്ങൾ;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_താഴെയുള്ള, (u32)xhci->evts);
    writel(&xhci->ir->erdp_മുകളിലെ, 0);
    writel(&xhci->ir->erstba_താഴെയുള്ള, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_മുകളിലെ, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    എങ്കിൽ (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_മുകളിലെ(64, വലുപ്പം(*spba) * spb);
        ശൂന്യം *pad = memalign_മുകളിലെ(PAGE_SIZE, PAGE_SIZE * spb);
        എങ്കിൽ (!spba || !pad) {
            warn_noalloc();
            സ്വതന്ത്രം(spba);
            സ്വതന്ത്രം(pad);
            പോകുക fail;
        }
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_താഴെയുള്ള = (u32)spba;
        xhci->devs[0].ptr_മുകളിലെ = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    പൂർണ്ണസംഖ്യ എണ്ണം = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    എങ്കിൽ (എണ്ണം)
        // Success
        തിരികെ;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    സ്വതന്ത്രം(xhci->eseg);
    സ്വതന്ത്രം(xhci->evts);
    സ്വതന്ത്രം(xhci->cmds);
    സ്വതന്ത്രം(xhci->devs);
    സ്വതന്ത്രം(xhci);
}

സ്ഥിതം ഘടന usb_xhci_s*
xhci_controller_setup(ശൂന്യം *baseaddr)
{
    ഘടന usb_xhci_s *xhci = malloc_മുകളിലെ(വലുപ്പം(*xhci));
    എങ്കിൽ (!xhci) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(xhci, 0, വലുപ്പം(*xhci));
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

    എങ്കിൽ (xhci->xcap) {
        u32 off;
        ശൂന്യം *addr = baseaddr + xhci->xcap;
        ചെയ്യുക {
            ഘടന xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            തിരഞ്ഞെടുപ്പ് (cap & 0xff) {
            അവസ്ഥ 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 എണ്ണം = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , എണ്ണം, start
                        , ports >> 16);
                എങ്കിൽ (name == 0x20425355 /* "USB " */) {
                    എങ്കിൽ (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.എണ്ണം = എണ്ണം;
                    }
                    എങ്കിൽ (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.എണ്ണം = എണ്ണം;
                    }
                }
                നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                നിർത്തുക;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } ആകുമ്പോൾ (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    എങ്കിൽ (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        സ്വതന്ത്രം(xhci);
        തിരികെ ശൂന്യ;
    }

    തിരികെ xhci;
}

സ്ഥിതം ശൂന്യം
xhci_controller_setup_pci(ഘടന pci_device *pci)
{
    ഘടന usb_xhci_s *xhci;
    ശൂന്യം *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    എങ്കിൽ (!baseaddr)
        തിരികെ;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    എങ്കിൽ (!xhci)
        തിരികെ;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

സ്ഥിതം ശൂന്യം
xhci_controller_setup_acpi(ഘടന acpi_device *dev)
{
    ഘടന usb_xhci_s *xhci;
    u64 mem, unused;
    ശൂന്യം *baseaddr;

    എങ്കിൽ (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        തിരികെ;
    എങ്കിൽ (mem >= 0x100000000ll)
        തിരികെ;

    baseaddr = (ശൂന്യം*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    എങ്കിൽ (!xhci)
        തിരികെ;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

ശൂന്യം
xhci_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_XHCI)
        തിരികെ;

    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    ഘടന acpi_device *dev;
    വേണ്ടി (dev = acpi_dsdt_find_eisaid(ശൂന്യ, xhci_eisaid);
         dev != ശൂന്യ;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
സ്ഥിതം ശൂന്യം xhci_doorbell(ഘടന usb_xhci_s *xhci, u32 slotid, u32 മൂല്യം)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, മൂല്യം);
    ഘടന xhci_db *db = xhci->db;
    ശൂന്യം *addr = &db[slotid].doorbell;
    writel(addr, മൂല്യം);
}

// Dequeue events on the XHCI command ring generated by the hardware
സ്ഥിതം ശൂന്യം xhci_process_events(ഘടന usb_xhci_s *xhci)
{
    ഘടന xhci_ring *evts = xhci->evts;

    വേണ്ടി (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        ഘടന xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        എങ്കിൽ ((control & TRB_C) != (cs ? 1 : 0))
            തിരികെ;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        തിരഞ്ഞെടുപ്പ് (evt_type) {
        അവസ്ഥ ER_TRANSFER:
        അവസ്ഥ ER_COMMAND_COMPLETE:
        {
            ഘടന xhci_trb  *rtrb = (ശൂന്യം*)etrb->ptr_താഴെയുള്ള;
            ഘടന xhci_ring *ring = XHCI_RING(rtrb);
            ഘടന xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            memcpy(evt, etrb, വലുപ്പം(*etrb));
            ring->eidx = eidx;
            നിർത്തുക;
        }
        അവസ്ഥ ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_താഴെയുള്ള >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            നിർത്തുക;
        }
        സ്ഥിരസ്ഥിതി:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            നിർത്തുക;
        }

        /* move ring index, notify xhci */
        nidx++;
        എങ്കിൽ (nidx == XHCI_RING_ഇനങ്ങൾ) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        ഘടന xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_താഴെയുള്ള, erdp);
        writel(&ir->erdp_മുകളിലെ, 0);
    }
}

// Check if a ring has any pending TRBs
സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_ring_busy(ഘടന xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    തിരികെ (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_event_wait(ഘടന usb_xhci_s *xhci,
                           ഘടന xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    വേണ്ടി (;;) {
        xhci_process_events(xhci);
        എങ്കിൽ (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            തിരികെ (status >> 24) & 0xff;
        }
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
സ്ഥിതം ശൂന്യം xhci_trb_fill(ഘടന xhci_ring *ring
                          , ശൂന്യം *data, u32 xferlen, u32 flags)
{
    ഘടന xhci_trb *dst = &ring->ring[ring->nidx];
    എങ്കിൽ (flags & TRB_TR_IDT) {
        memcpy(&dst->ptr_താഴെയുള്ള, data, xferlen);
    } അല്ലെങ്കിൽ {
        dst->ptr_താഴെയുള്ള = (u32)data;
        dst->ptr_മുകളിലെ = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
സ്ഥിതം ശൂന്യം xhci_trb_queue(ഘടന xhci_ring *ring,
                           ശൂന്യം *data, u32 xferlen, u32 flags)
{
    എങ്കിൽ (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_cmd_submit(ഘടന usb_xhci_s *xhci, ഘടന xhci_inctx *inctx
                           , u32 flags)
{
    എങ്കിൽ (inctx) {
        ഘടന xhci_slotctx *slot = (ശൂന്യം*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        എങ്കിൽ (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            തിരികെ -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    പൂർണ്ണസംഖ്യ rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_cmd_enable_slot(ഘടന usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    പൂർണ്ണസംഖ്യ cc = xhci_cmd_submit(xhci, ശൂന്യ, CR_ENABLE_SLOT << 10);
    എങ്കിൽ (cc != CC_SUCCESS)
        തിരികെ -1;
    തിരികെ (xhci->cmds->evt.control >> 24) & 0xff;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_cmd_disable_slot(ഘടന usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    തിരികെ xhci_cmd_submit(xhci, ശൂന്യ, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_cmd_address_device(ഘടന usb_xhci_s *xhci, u32 slotid
                                   , ഘടന xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    തിരികെ xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_cmd_configure_endpoint(ഘടന usb_xhci_s *xhci, u32 slotid
                                       , ഘടന xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    തിരികെ xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_cmd_evaluate_context(ഘടന usb_xhci_s *xhci, u32 slotid
                                     , ഘടന xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    തിരികെ xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

സ്ഥിതം ഘടന xhci_inctx *
xhci_alloc_inctx(ഘടന usbdevice_s *usbdev, പൂർണ്ണസംഖ്യ maxepid)
{
    ഘടന usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ഘടന usb_xhci_s, usb);
    പൂർണ്ണസംഖ്യ size = (വലുപ്പം(ഘടന xhci_inctx) * 33) << xhci->context64;
    ഘടന xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    എങ്കിൽ (!in) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(in, 0, size);

    ഘടന xhci_slotctx *slot = (ശൂന്യം*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    ഘടന usbdevice_s *hubdev = usbdev->hub->usbdev;
    എങ്കിൽ (hubdev) {
        എങ്കിൽ (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            ഘടന xhci_pipe *hpipe = container_of(
                hubdev->defpipe, ഘടന xhci_pipe, pipe);
            എങ്കിൽ (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } അല്ലെങ്കിൽ {
                ഘടന xhci_slotctx *hslot = (ശൂന്യം*)xhci->devs[hpipe->slotid].ptr_താഴെയുള്ള;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        ആകുമ്പോൾ (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    തിരികെ in;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xhci_config_hub(ഘടന usbhub_s *hub)
{
    ഘടന usb_xhci_s *xhci = container_of(
        hub->cntl, ഘടന usb_xhci_s, usb);
    ഘടന xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, ഘടന xhci_pipe, pipe);
    ഘടന xhci_slotctx *hdslot = (ശൂന്യം*)xhci->devs[pipe->slotid].ptr_താഴെയുള്ള;
    എങ്കിൽ ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        തിരികെ 0;
    ഘടന xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    എങ്കിൽ (!in)
        തിരികെ -1;
    in->add = 0x01;
    ഘടന xhci_slotctx *slot = (ശൂന്യം*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    പൂർണ്ണസംഖ്യ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    സ്വതന്ത്രം(in);
    എങ്കിൽ (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        തിരികെ -1;
    }
    തിരികെ 0;
}

സ്ഥിതം ഘടന usb_pipe *
xhci_alloc_pipe(ഘടന usbdevice_s *usbdev
                , ഘടന usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ഘടന usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, ഘടന usb_xhci_s, usb);
    ഘടന xhci_pipe *pipe;
    u32 epid;

    എങ്കിൽ (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } അല്ലെങ്കിൽ {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_മുകളിലെ(XHCI_RING_SIZE, വലുപ്പം(*pipe));
    അല്ലെങ്കിൽ
        pipe = memalign_താഴെയുള്ള(XHCI_RING_SIZE, വലുപ്പം(*pipe));
    എങ്കിൽ (!pipe) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(pipe, 0, വലുപ്പം(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_മുകളിലെ(pipe->pipe.maxpacket);
        എങ്കിൽ (!pipe->buf) {
            warn_noalloc();
            സ്വതന്ത്രം(pipe);
            തിരികെ ശൂന്യ;
        }
    }

    // Allocate input context and initialize endpoint info.
    ഘടന xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    എങ്കിൽ (!in)
        പോകുക fail;
    in->add = 0x01 | (1 << epid);
    ഘടന xhci_epctx *ep = (ശൂന്യം*)&in[(pipe->epid+1) << xhci->context64];
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    എങ്കിൽ (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_താഴെയുള്ള  = (u32)&pipe->reqs.ring[0];
    ep->deq_താഴെയുള്ള  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    എങ്കിൽ (pipe->epid == 1) {
        എങ്കിൽ (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            പൂർണ്ണസംഖ്യ ret = xhci_config_hub(usbdev->hub);
            എങ്കിൽ (ret)
                പോകുക fail;
        }
        // Enable slot.
        u32 size = (വലുപ്പം(ഘടന xhci_slotctx) * 32) << xhci->context64;
        ഘടന xhci_slotctx *dev = memalign_മുകളിലെ(1024 << xhci->context64, size);
        എങ്കിൽ (!dev) {
            warn_noalloc();
            പോകുക fail;
        }
        പൂർണ്ണസംഖ്യ slotid = xhci_cmd_enable_slot(xhci);
        എങ്കിൽ (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            സ്വതന്ത്രം(dev);
            പോകുക fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, size);
        xhci->devs[slotid].ptr_താഴെയുള്ള = (u32)dev;
        xhci->devs[slotid].ptr_മുകളിലെ = 0;

        // Send set_address command.
        പൂർണ്ണസംഖ്യ cc = xhci_cmd_address_device(xhci, slotid, in);
        എങ്കിൽ (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            എങ്കിൽ (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                പോകുക fail;
            }
            xhci->devs[slotid].ptr_താഴെയുള്ള = 0;
            സ്വതന്ത്രം(dev);
            പോകുക fail;
        }
        pipe->slotid = slotid;
    } അല്ലെങ്കിൽ {
        ഘടന xhci_pipe *defpipe = container_of(
            usbdev->defpipe, ഘടന xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        പൂർണ്ണസംഖ്യ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        എങ്കിൽ (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            പോകുക fail;
        }
    }
    സ്വതന്ത്രം(in);
    തിരികെ &pipe->pipe;

fail:
    സ്വതന്ത്രം(pipe->buf);
    സ്വതന്ത്രം(pipe);
    സ്വതന്ത്രം(in);
    തിരികെ ശൂന്യ;
}

ഘടന usb_pipe *
xhci_realloc_pipe(ഘടന usbdevice_s *usbdev, ഘടന usb_pipe *upipe
                  , ഘടന usb_endpoint_descriptor *epdesc)
{
    എങ്കിൽ (!CONFIG_USB_XHCI)
        തിരികെ ശൂന്യ;
    എങ്കിൽ (!epdesc) {
        usb_add_freelist(upipe);
        തിരികെ ശൂന്യ;
    }
    എങ്കിൽ (!upipe)
        തിരികെ xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    പൂർണ്ണസംഖ്യ oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    ഘടന xhci_pipe *pipe = container_of(upipe, ഘടന xhci_pipe, pipe);
    ഘടന usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ഘടന usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    എങ്കിൽ (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        തിരികെ upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    ഘടന xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    എങ്കിൽ (!in)
        തിരികെ upipe;
    in->add = (1 << 1);
    ഘടന xhci_epctx *ep = (ശൂന്യം*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    പൂർണ്ണസംഖ്യ cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    എങ്കിൽ (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    സ്വതന്ത്രം(in);

    തിരികെ upipe;
}

// Submit a USB "setup" message request to the pipe's ring
സ്ഥിതം ശൂന്യം xhci_xfer_setup(ഘടന xhci_pipe *pipe, പൂർണ്ണസംഖ്യ dir, ശൂന്യം *cmd
                            , ശൂന്യം *data, പൂർണ്ണസംഖ്യ datalen)
{
    ഘടന usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ഘടന usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    എങ്കിൽ (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, ശൂന്യ, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
സ്ഥിതം ശൂന്യം xhci_xfer_normal(ഘടന xhci_pipe *pipe,
                             ശൂന്യം *data, പൂർണ്ണസംഖ്യ datalen)
{
    ഘടന usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ഘടന usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

പൂർണ്ണസംഖ്യ
xhci_send_pipe(ഘടന usb_pipe *p, പൂർണ്ണസംഖ്യ dir, സ്ഥിരം ശൂന്യം *cmd
               , ശൂന്യം *data, പൂർണ്ണസംഖ്യ datalen)
{
    എങ്കിൽ (!CONFIG_USB_XHCI)
        തിരികെ -1;
    ഘടന xhci_pipe *pipe = container_of(p, ഘടന xhci_pipe, pipe);
    ഘടന usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ഘടന usb_xhci_s, usb);

    എങ്കിൽ (cmd) {
        സ്ഥിരം ഘടന usb_ctrlrequest *req = cmd;
        എങ്കിൽ (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            തിരികെ 0;
        xhci_xfer_setup(pipe, dir, (ശൂന്യം*)req, data, datalen);
    } അല്ലെങ്കിൽ {
        xhci_xfer_normal(pipe, data, datalen);
    }

    പൂർണ്ണസംഖ്യ cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    എങ്കിൽ (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        തിരികെ -1;
    }

    തിരികെ 0;
}

പൂർണ്ണസംഖ്യ VISIBLE32FLAT
xhci_poll_intr(ഘടന usb_pipe *p, ശൂന്യം *data)
{
    എങ്കിൽ (!CONFIG_USB_XHCI)
        തിരികെ -1;

    ഘടന xhci_pipe *pipe = container_of(p, ഘടന xhci_pipe, pipe);
    ഘടന usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, ഘടന usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    ശൂന്യം *buf = pipe->buf;
    പൂർണ്ണസംഖ്യ bufused = pipe->bufused;

    എങ്കിൽ (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        തിരികെ -1;
    }

    xhci_process_events(xhci);
    എങ്കിൽ (xhci_ring_busy(&pipe->reqs))
        തിരികെ -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    memcpy(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    തിരികെ 0;
}
