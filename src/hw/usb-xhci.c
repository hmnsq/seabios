// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // memalign_low
#अन्तर्भाव "memmap.h" // PAGE_SIZE
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#अन्तर्भाव "pci_regs.h" // PCI_BASE_ADDRESS_0
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-xhci.h" // struct ehci_qh
#अन्तर्भाव "util.h" // timer_calc
#अन्तर्भाव "x86.h" // readl

// --------------------------------------------------------------
// configuration

#define XHCI_RING_वस्तुएँ          16
#define XHCI_RING_SIZE           (XHCI_RING_वस्तुएँ*परिमाण(संरचना xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#define XHCI_RING(_trb)          \
    ((संरचना xhci_ring*)((u32)(_trb) & ~(XHCI_RING_SIZE-1)))

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
#define XHCI_PORTSC_SPEED_निचला    (2<<10)
#define XHCI_PORTSC_SPEED_ऊपरी   (3<<10)
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

प्रकारनिर्देश गणना TRBType {
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

प्रकारनिर्देश गणना TRBCCode {
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

गणना {
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

संरचना xhci_ring {
    संरचना xhci_trb      ring[XHCI_RING_वस्तुएँ];
    संरचना xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    संरचना mutex_s       lock;
};

संरचना xhci_portmap {
    u8 start;
    u8 गिनती;
};

संरचना usb_xhci_s {
    संरचना usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    संरचना xhci_portmap  usb2;
    संरचना xhci_portmap  usb3;

    /* xhci registers */
    संरचना xhci_caps     *caps;
    संरचना xhci_op       *op;
    संरचना xhci_pr       *pr;
    संरचना xhci_ir       *ir;
    संरचना xhci_db       *db;

    /* xhci data structures */
    संरचना xhci_devlist  *devs;
    संरचना xhci_ring     *cmds;
    संरचना xhci_ring     *evts;
    संरचना xhci_er_seg   *eseg;
};

संरचना xhci_pipe {
    संरचना xhci_ring     reqs;

    संरचना usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    शून्य                 *buf;
    पूर्णाङ्क                  bufused;
};

// --------------------------------------------------------------
// tables

स्थिर ध्रुव वर्ण *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

स्थिर ध्रुव पूर्णाङ्क speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

स्थिर ध्रुव पूर्णाङ्क speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

स्थिर पूर्णाङ्क wait_bit(u32 *reg, u32 mask, पूर्णाङ्क मूल्य, u32 timeout)
{
    u32 end = timer_calc(timeout);

    यावत् ((readl(reg) & mask) != मूल्य) {
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }
    प्रत्यागम 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#define XHCI_TIME_POSTPOWER 20

// Check if device attached to port
स्थिर शून्य
xhci_print_port_state(पूर्णाङ्क loglevel, ध्रुव वर्ण *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

स्थिर पूर्णाङ्क
xhci_hub_detect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_xhci_s *xhci = container_of(hub->cntl, संरचना usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    प्रत्यागम (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
स्थिर पूर्णाङ्क
xhci_hub_reset(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_xhci_s *xhci = container_of(hub->cntl, संरचना usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    यदि (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        प्रत्यागम -1;

    विकल्प (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    पक्ष PLS_U0:
        // A USB3 port - controller automatically performs reset
        भङ्ग;
    पक्ष PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        भङ्ग;
    स्वभाव:
        प्रत्यागम -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    प्रति (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        यदि (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            प्रत्यागम -1;
        यदि (portsc & XHCI_PORTSC_PED)
            // Reset complete
            भङ्ग;
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }

    पूर्णाङ्क rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    प्रत्यागम rc;
}

स्थिर पूर्णाङ्क
xhci_hub_portmap(संरचना usbhub_s *hub, u32 vport)
{
    संरचना usb_xhci_s *xhci = container_of(hub->cntl, संरचना usb_xhci_s, usb);
    u32 pport = vport + 1;

    यदि (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.गिनती)
        pport = vport + 2 - xhci->usb3.start;

    यदि (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.गिनती)
        pport = vport + 2 - xhci->usb2.start;

    प्रत्यागम pport;
}

स्थिर शून्य
xhci_hub_disconnect(संरचना usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

स्थिर संरचना usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
स्थिर पूर्णाङ्क
xhci_check_ports(संरचना usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    संरचना usbhub_s hub;
    स_रखो(&hub, 0, परिमाण(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    प्रत्यागम hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

स्थिर शून्य
xhci_free_pipes(संरचना usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

स्थिर शून्य
configure_xhci(शून्य *data)
{
    संरचना usb_xhci_s *xhci = data;
    u32 reg;

    xhci->devs = memalign_ऊपरी(64, परिमाण(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ऊपरी(64, परिमाण(*xhci->eseg));
    xhci->cmds = memalign_ऊपरी(XHCI_RING_SIZE, परिमाण(*xhci->cmds));
    xhci->evts = memalign_ऊपरी(XHCI_RING_SIZE, परिमाण(*xhci->evts));
    यदि (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        गच्छ fail;
    }
    स_रखो(xhci->devs, 0, परिमाण(*xhci->devs) * (xhci->slots + 1));
    स_रखो(xhci->cmds, 0, परिमाण(*xhci->cmds));
    स_रखो(xhci->evts, 0, परिमाण(*xhci->evts));
    स_रखो(xhci->eseg, 0, परिमाण(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    यदि (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        यदि (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            गच्छ fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    यदि (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        गच्छ fail;
    यदि (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        गच्छ fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_निचला, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ऊपरी, 0);
    writel(&xhci->op->crcr_निचला, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ऊपरी, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_निचला = (u32)xhci->evts;
    xhci->eseg->ptr_ऊपरी = 0;
    xhci->eseg->size = XHCI_RING_वस्तुएँ;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_निचला, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ऊपरी, 0);
    writel(&xhci->ir->erstba_निचला, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ऊपरी, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    यदि (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ऊपरी(64, परिमाण(*spba) * spb);
        शून्य *pad = memalign_ऊपरी(PAGE_SIZE, PAGE_SIZE * spb);
        यदि (!spba || !pad) {
            warn_noalloc();
            मुक्त(spba);
            मुक्त(pad);
            गच्छ fail;
        }
        पूर्णाङ्क i;
        प्रति (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_SIZE);
        xhci->devs[0].ptr_निचला = (u32)spba;
        xhci->devs[0].ptr_ऊपरी = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    पूर्णाङ्क गिनती = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    यदि (गिनती)
        // Success
        प्रत्यागम;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    मुक्त(xhci->eseg);
    मुक्त(xhci->evts);
    मुक्त(xhci->cmds);
    मुक्त(xhci->devs);
    मुक्त(xhci);
}

स्थिर संरचना usb_xhci_s*
xhci_controller_setup(शून्य *baseaddr)
{
    संरचना usb_xhci_s *xhci = malloc_ऊपरी(परिमाण(*xhci));
    यदि (!xhci) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स_रखो(xhci, 0, परिमाण(*xhci));
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

    यदि (xhci->xcap) {
        u32 off;
        शून्य *addr = baseaddr + xhci->xcap;
        कुरु {
            संरचना xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            विकल्प (cap & 0xff) {
            पक्ष 0x02:
                name  = readl(&xcap->data[0]);
                ports = readl(&xcap->data[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 गिनती = (ports >> 8) & 0xff;
                u8 start = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , गिनती, start
                        , ports >> 16);
                यदि (name == 0x20425355 /* "USB " */) {
                    यदि (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.गिनती = गिनती;
                    }
                    यदि (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.गिनती = गिनती;
                    }
                }
                भङ्ग;
            स्वभाव:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                भङ्ग;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } यावत् (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    यदि (PAGE_SIZE != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        मुक्त(xhci);
        प्रत्यागम रिक्त;
    }

    प्रत्यागम xhci;
}

स्थिर शून्य
xhci_controller_setup_pci(संरचना pci_device *pci)
{
    संरचना usb_xhci_s *xhci;
    शून्य *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    यदि (!baseaddr)
        प्रत्यागम;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    यदि (!xhci)
        प्रत्यागम;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

स्थिर शून्य
xhci_controller_setup_acpi(संरचना acpi_device *dev)
{
    संरचना usb_xhci_s *xhci;
    u64 mem, unused;
    शून्य *baseaddr;

    यदि (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        प्रत्यागम;
    यदि (mem >= 0x100000000ll)
        प्रत्यागम;

    baseaddr = (शून्य*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    यदि (!xhci)
        प्रत्यागम;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

शून्य
xhci_setup(शून्य)
{
    यदि (! CONFIG_USB_XHCI)
        प्रत्यागम;

    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    संरचना acpi_device *dev;
    प्रति (dev = acpi_dsdt_find_eisaid(रिक्त, xhci_eisaid);
         dev != रिक्त;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
स्थिर शून्य xhci_doorbell(संरचना usb_xhci_s *xhci, u32 slotid, u32 मूल्य)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, मूल्य);
    संरचना xhci_db *db = xhci->db;
    शून्य *addr = &db[slotid].doorbell;
    writel(addr, मूल्य);
}

// Dequeue events on the XHCI command ring generated by the hardware
स्थिर शून्य xhci_process_events(संरचना usb_xhci_s *xhci)
{
    संरचना xhci_ring *evts = xhci->evts;

    प्रति (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        संरचना xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        यदि ((control & TRB_C) != (cs ? 1 : 0))
            प्रत्यागम;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        विकल्प (evt_type) {
        पक्ष ER_TRANSFER:
        पक्ष ER_COMMAND_COMPLETE:
        {
            संरचना xhci_trb  *rtrb = (शून्य*)etrb->ptr_निचला;
            संरचना xhci_ring *ring = XHCI_RING(rtrb);
            संरचना xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            स_नकल(evt, etrb, परिमाण(*etrb));
            ring->eidx = eidx;
            भङ्ग;
        }
        पक्ष ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_निचला >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            भङ्ग;
        }
        स्वभाव:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            भङ्ग;
        }

        /* move ring index, notify xhci */
        nidx++;
        यदि (nidx == XHCI_RING_वस्तुएँ) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        संरचना xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_निचला, erdp);
        writel(&ir->erdp_ऊपरी, 0);
    }
}

// Check if a ring has any pending TRBs
स्थिर पूर्णाङ्क xhci_ring_busy(संरचना xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    प्रत्यागम (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
स्थिर पूर्णाङ्क xhci_event_wait(संरचना usb_xhci_s *xhci,
                           संरचना xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    प्रति (;;) {
        xhci_process_events(xhci);
        यदि (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            प्रत्यागम (status >> 24) & 0xff;
        }
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
स्थिर शून्य xhci_trb_fill(संरचना xhci_ring *ring
                          , शून्य *data, u32 xferlen, u32 flags)
{
    संरचना xhci_trb *dst = &ring->ring[ring->nidx];
    यदि (flags & TRB_TR_IDT) {
        स_नकल(&dst->ptr_निचला, data, xferlen);
    } अन्यथा {
        dst->ptr_निचला = (u32)data;
        dst->ptr_ऊपरी = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
स्थिर शून्य xhci_trb_queue(संरचना xhci_ring *ring,
                           शून्य *data, u32 xferlen, u32 flags)
{
    यदि (ring->nidx >= ARRAY_SIZE(ring->ring) - 1) {
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
स्थिर पूर्णाङ्क xhci_cmd_submit(संरचना usb_xhci_s *xhci, संरचना xhci_inctx *inctx
                           , u32 flags)
{
    यदि (inctx) {
        संरचना xhci_slotctx *slot = (शून्य*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        यदि (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            प्रत्यागम -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    पूर्णाङ्क rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    प्रत्यागम rc;
}

स्थिर पूर्णाङ्क xhci_cmd_enable_slot(संरचना usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    पूर्णाङ्क cc = xhci_cmd_submit(xhci, रिक्त, CR_ENABLE_SLOT << 10);
    यदि (cc != CC_SUCCESS)
        प्रत्यागम -1;
    प्रत्यागम (xhci->cmds->evt.control >> 24) & 0xff;
}

स्थिर पूर्णाङ्क xhci_cmd_disable_slot(संरचना usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    प्रत्यागम xhci_cmd_submit(xhci, रिक्त, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

स्थिर पूर्णाङ्क xhci_cmd_address_device(संरचना usb_xhci_s *xhci, u32 slotid
                                   , संरचना xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    प्रत्यागम xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

स्थिर पूर्णाङ्क xhci_cmd_configure_endpoint(संरचना usb_xhci_s *xhci, u32 slotid
                                       , संरचना xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    प्रत्यागम xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

स्थिर पूर्णाङ्क xhci_cmd_evaluate_context(संरचना usb_xhci_s *xhci, u32 slotid
                                     , संरचना xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    प्रत्यागम xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

स्थिर संरचना xhci_inctx *
xhci_alloc_inctx(संरचना usbdevice_s *usbdev, पूर्णाङ्क maxepid)
{
    संरचना usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, संरचना usb_xhci_s, usb);
    पूर्णाङ्क size = (परिमाण(संरचना xhci_inctx) * 33) << xhci->context64;
    संरचना xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, size);
    यदि (!in) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स_रखो(in, 0, size);

    संरचना xhci_slotctx *slot = (शून्य*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    संरचना usbdevice_s *hubdev = usbdev->hub->usbdev;
    यदि (hubdev) {
        यदि (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            संरचना xhci_pipe *hpipe = container_of(
                hubdev->defpipe, संरचना xhci_pipe, pipe);
            यदि (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } अन्यथा {
                संरचना xhci_slotctx *hslot = (शून्य*)xhci->devs[hpipe->slotid].ptr_निचला;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        यावत् (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    प्रत्यागम in;
}

स्थिर पूर्णाङ्क xhci_config_hub(संरचना usbhub_s *hub)
{
    संरचना usb_xhci_s *xhci = container_of(
        hub->cntl, संरचना usb_xhci_s, usb);
    संरचना xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, संरचना xhci_pipe, pipe);
    संरचना xhci_slotctx *hdslot = (शून्य*)xhci->devs[pipe->slotid].ptr_निचला;
    यदि ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        प्रत्यागम 0;
    संरचना xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    यदि (!in)
        प्रत्यागम -1;
    in->add = 0x01;
    संरचना xhci_slotctx *slot = (शून्य*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    पूर्णाङ्क cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    मुक्त(in);
    यदि (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        प्रत्यागम -1;
    }
    प्रत्यागम 0;
}

स्थिर संरचना usb_pipe *
xhci_alloc_pipe(संरचना usbdevice_s *usbdev
                , संरचना usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    संरचना usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, संरचना usb_xhci_s, usb);
    संरचना xhci_pipe *pipe;
    u32 epid;

    यदि (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } अन्यथा {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    यदि (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ऊपरी(XHCI_RING_SIZE, परिमाण(*pipe));
    अन्यथा
        pipe = memalign_निचला(XHCI_RING_SIZE, परिमाण(*pipe));
    यदि (!pipe) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स_रखो(pipe, 0, परिमाण(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    यदि (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ऊपरी(pipe->pipe.maxpacket);
        यदि (!pipe->buf) {
            warn_noalloc();
            मुक्त(pipe);
            प्रत्यागम रिक्त;
        }
    }

    // Allocate input context and initialize endpoint info.
    संरचना xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    यदि (!in)
        गच्छ fail;
    in->add = 0x01 | (1 << epid);
    संरचना xhci_epctx *ep = (शून्य*)&in[(pipe->epid+1) << xhci->context64];
    यदि (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    यदि (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_निचला  = (u32)&pipe->reqs.ring[0];
    ep->deq_निचला  |= 1;         // dcs
    ep->length   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    यदि (pipe->epid == 1) {
        यदि (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            पूर्णाङ्क ret = xhci_config_hub(usbdev->hub);
            यदि (ret)
                गच्छ fail;
        }
        // Enable slot.
        u32 size = (परिमाण(संरचना xhci_slotctx) * 32) << xhci->context64;
        संरचना xhci_slotctx *dev = memalign_ऊपरी(1024 << xhci->context64, size);
        यदि (!dev) {
            warn_noalloc();
            गच्छ fail;
        }
        पूर्णाङ्क slotid = xhci_cmd_enable_slot(xhci);
        यदि (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            मुक्त(dev);
            गच्छ fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        स_रखो(dev, 0, size);
        xhci->devs[slotid].ptr_निचला = (u32)dev;
        xhci->devs[slotid].ptr_ऊपरी = 0;

        // Send set_address command.
        पूर्णाङ्क cc = xhci_cmd_address_device(xhci, slotid, in);
        यदि (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            यदि (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                गच्छ fail;
            }
            xhci->devs[slotid].ptr_निचला = 0;
            मुक्त(dev);
            गच्छ fail;
        }
        pipe->slotid = slotid;
    } अन्यथा {
        संरचना xhci_pipe *defpipe = container_of(
            usbdev->defpipe, संरचना xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        पूर्णाङ्क cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        यदि (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            गच्छ fail;
        }
    }
    मुक्त(in);
    प्रत्यागम &pipe->pipe;

fail:
    मुक्त(pipe->buf);
    मुक्त(pipe);
    मुक्त(in);
    प्रत्यागम रिक्त;
}

संरचना usb_pipe *
xhci_realloc_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *upipe
                  , संरचना usb_endpoint_descriptor *epdesc)
{
    यदि (!CONFIG_USB_XHCI)
        प्रत्यागम रिक्त;
    यदि (!epdesc) {
        usb_add_freelist(upipe);
        प्रत्यागम रिक्त;
    }
    यदि (!upipe)
        प्रत्यागम xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    पूर्णाङ्क oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    संरचना xhci_pipe *pipe = container_of(upipe, संरचना xhci_pipe, pipe);
    संरचना usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, संरचना usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    यदि (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        प्रत्यागम upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    संरचना xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    यदि (!in)
        प्रत्यागम upipe;
    in->add = (1 << 1);
    संरचना xhci_epctx *ep = (शून्य*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    पूर्णाङ्क cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    यदि (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    मुक्त(in);

    प्रत्यागम upipe;
}

// Submit a USB "setup" message request to the pipe's ring
स्थिर शून्य xhci_xfer_setup(संरचना xhci_pipe *pipe, पूर्णाङ्क dir, शून्य *cmd
                            , शून्य *data, पूर्णाङ्क datalen)
{
    संरचना usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, संरचना usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_SIZE
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    यदि (datalen)
        xhci_trb_queue(&pipe->reqs, data, datalen, (TR_DATA << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, रिक्त, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
स्थिर शून्य xhci_xfer_normal(संरचना xhci_pipe *pipe,
                             शून्य *data, पूर्णाङ्क datalen)
{
    संरचना usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, संरचना usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, data, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

पूर्णाङ्क
xhci_send_pipe(संरचना usb_pipe *p, पूर्णाङ्क dir, ध्रुव शून्य *cmd
               , शून्य *data, पूर्णाङ्क datalen)
{
    यदि (!CONFIG_USB_XHCI)
        प्रत्यागम -1;
    संरचना xhci_pipe *pipe = container_of(p, संरचना xhci_pipe, pipe);
    संरचना usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, संरचना usb_xhci_s, usb);

    यदि (cmd) {
        ध्रुव संरचना usb_ctrlrequest *req = cmd;
        यदि (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            प्रत्यागम 0;
        xhci_xfer_setup(pipe, dir, (शून्य*)req, data, datalen);
    } अन्यथा {
        xhci_xfer_normal(pipe, data, datalen);
    }

    पूर्णाङ्क cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    यदि (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        प्रत्यागम -1;
    }

    प्रत्यागम 0;
}

पूर्णाङ्क VISIBLE32FLAT
xhci_poll_intr(संरचना usb_pipe *p, शून्य *data)
{
    यदि (!CONFIG_USB_XHCI)
        प्रत्यागम -1;

    संरचना xhci_pipe *pipe = container_of(p, संरचना xhci_pipe, pipe);
    संरचना usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, संरचना usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    शून्य *buf = pipe->buf;
    पूर्णाङ्क bufused = pipe->bufused;

    यदि (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        प्रत्यागम -1;
    }

    xhci_process_events(xhci);
    यदि (xhci_ring_busy(&pipe->reqs))
        प्रत्यागम -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            data, buf, len);
    स_नकल(data, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    प्रत्यागम 0;
}
