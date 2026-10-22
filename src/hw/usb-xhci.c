// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // memalign_low
#समावेश "memmap.h" // PAGE_SIZE
#समावेश "output.h" // dprintf
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#समावेश "pci_regs.h" // PCI_BASE_ADDRESS_0
#समावेश "string.h" // memcpy
#समावेश "usb.h" // struct usb_s
#समावेश "usb-xhci.h" // struct ehci_qh
#समावेश "util.h" // timer_calc
#समावेश "x86.h" // readl

// --------------------------------------------------------------
// configuration

#घोषणा XHCI_RING_वस्तुएँ          16
#घोषणा XHCI_RING_आकार           (XHCI_RING_वस्तुएँ*माप(काष्ठा xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#घोषणा XHCI_RING(_trb)          \
    ((काष्ठा xhci_ring*)((u32)(_trb) & ~(XHCI_RING_आकार-1)))

// --------------------------------------------------------------
// bit definitions

#घोषणा XHCI_CMD_RS              (1<<0)
#घोषणा XHCI_CMD_HCRST           (1<<1)
#घोषणा XHCI_CMD_INTE            (1<<2)
#घोषणा XHCI_CMD_HSEE            (1<<3)
#घोषणा XHCI_CMD_LHCRST          (1<<7)
#घोषणा XHCI_CMD_CSS             (1<<8)
#घोषणा XHCI_CMD_CRS             (1<<9)
#घोषणा XHCI_CMD_EWE             (1<<10)
#घोषणा XHCI_CMD_EU3S            (1<<11)

#घोषणा XHCI_STS_HCH             (1<<0)
#घोषणा XHCI_STS_HSE             (1<<2)
#घोषणा XHCI_STS_EINT            (1<<3)
#घोषणा XHCI_STS_PCD             (1<<4)
#घोषणा XHCI_STS_SSS             (1<<8)
#घोषणा XHCI_STS_RSS             (1<<9)
#घोषणा XHCI_STS_SRE             (1<<10)
#घोषणा XHCI_STS_CNR             (1<<11)
#घोषणा XHCI_STS_HCE             (1<<12)

#घोषणा XHCI_PORTSC_CCS          (1<<0)
#घोषणा XHCI_PORTSC_PED          (1<<1)
#घोषणा XHCI_PORTSC_OCA          (1<<3)
#घोषणा XHCI_PORTSC_PR           (1<<4)
#घोषणा XHCI_PORTSC_PLS_SHIFT        5
#घोषणा XHCI_PORTSC_PLS_MASK     0xf
#घोषणा XHCI_PORTSC_PP           (1<<9)
#घोषणा XHCI_PORTSC_SPEED_SHIFT      10
#घोषणा XHCI_PORTSC_SPEED_MASK   0xf
#घोषणा XHCI_PORTSC_SPEED_FULL   (1<<10)
#घोषणा XHCI_PORTSC_SPEED_निचला    (2<<10)
#घोषणा XHCI_PORTSC_SPEED_ऊपरी   (3<<10)
#घोषणा XHCI_PORTSC_SPEED_SUPER  (4<<10)
#घोषणा XHCI_PORTSC_PIC_SHIFT        14
#घोषणा XHCI_PORTSC_PIC_MASK     0x3
#घोषणा XHCI_PORTSC_LWS          (1<<16)
#घोषणा XHCI_PORTSC_CSC          (1<<17)
#घोषणा XHCI_PORTSC_PEC          (1<<18)
#घोषणा XHCI_PORTSC_WRC          (1<<19)
#घोषणा XHCI_PORTSC_OCC          (1<<20)
#घोषणा XHCI_PORTSC_PRC          (1<<21)
#घोषणा XHCI_PORTSC_PLC          (1<<22)
#घोषणा XHCI_PORTSC_CEC          (1<<23)
#घोषणा XHCI_PORTSC_CAS          (1<<24)
#घोषणा XHCI_PORTSC_WCE          (1<<25)
#घोषणा XHCI_PORTSC_WDE          (1<<26)
#घोषणा XHCI_PORTSC_WOE          (1<<27)
#घोषणा XHCI_PORTSC_DR           (1<<30)
#घोषणा XHCI_PORTSC_WPR          (1<<31)

#घोषणा TRB_C               (1<<0)
#घोषणा TRB_TYPE_SHIFT          10
#घोषणा TRB_TYPE_MASK       0x3f
#घोषणा TRB_TYPE(t)         (((t) >> TRB_TYPE_SHIFT) & TRB_TYPE_MASK)

#घोषणा TRB_EV_ED           (1<<2)

#घोषणा TRB_TR_ENT          (1<<1)
#घोषणा TRB_TR_ISP          (1<<2)
#घोषणा TRB_TR_NS           (1<<3)
#घोषणा TRB_TR_CH           (1<<4)
#घोषणा TRB_TR_IOC          (1<<5)
#घोषणा TRB_TR_IDT          (1<<6)
#घोषणा TRB_TR_TBC_SHIFT        7
#घोषणा TRB_TR_TBC_MASK     0x3
#घोषणा TRB_TR_BEI          (1<<9)
#घोषणा TRB_TR_TLBPC_SHIFT      16
#घोषणा TRB_TR_TLBPC_MASK   0xf
#घोषणा TRB_TR_FRAMEID_SHIFT    20
#घोषणा TRB_TR_FRAMEID_MASK 0x7ff
#घोषणा TRB_TR_SIA          (1<<31)

#घोषणा TRB_TR_DIR          (1<<16)

#घोषणा TRB_CR_SLOTID_SHIFT     24
#घोषणा TRB_CR_SLOTID_MASK  0xff
#घोषणा TRB_CR_EPID_SHIFT       16
#घोषणा TRB_CR_EPID_MASK    0x1f

#घोषणा TRB_CR_BSR          (1<<9)
#घोषणा TRB_CR_DC           (1<<9)

#घोषणा TRB_LK_TC           (1<<1)

#घोषणा TRB_INTR_SHIFT          22
#घोषणा TRB_INTR_MASK       0x3ff
#घोषणा TRB_INTR(t)         (((t).status >> TRB_INTR_SHIFT) & TRB_INTR_MASK)

प्रकार क्रमागत TRBType {
    TRB_RESERVED = 0,
    TR_NORMAL,
    TR_SETUP,
    TR_तथ्य,
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

प्रकार क्रमागत TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_तथ्य_बफर_ERROR,
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
    CC_STOPPED_लंबाई_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_बफर_OVERRUN = 31,
    CC_EVENT_LOST_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_STREAM_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_SPLIT_TRANSACTION_ERROR
} TRBCCode;

क्रमागत {
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

#घोषणा xhci_get_field(तथ्य, field)             \
    (((तथ्य) >> field##_SHIFT) & field##_MASK)

// --------------------------------------------------------------
// state structs

काष्ठा xhci_ring {
    काष्ठा xhci_trb      ring[XHCI_RING_वस्तुएँ];
    काष्ठा xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    काष्ठा mutex_s       lock;
};

काष्ठा xhci_portmap {
    u8 start;
    u8 गिनती;
};

काष्ठा usb_xhci_s {
    काष्ठा usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    काष्ठा xhci_portmap  usb2;
    काष्ठा xhci_portmap  usb3;

    /* xhci registers */
    काष्ठा xhci_caps     *caps;
    काष्ठा xhci_op       *op;
    काष्ठा xhci_pr       *pr;
    काष्ठा xhci_ir       *ir;
    काष्ठा xhci_db       *db;

    /* xhci data structures */
    काष्ठा xhci_devlist  *devs;
    काष्ठा xhci_ring     *cmds;
    काष्ठा xhci_ring     *evts;
    काष्ठा xhci_er_seg   *eseg;
};

काष्ठा xhci_pipe {
    काष्ठा xhci_ring     reqs;

    काष्ठा usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    व्योम                 *buf;
    पूर्णांक                  bufused;
};

// --------------------------------------------------------------
// tables

अटल स्थिर अक्षर *speed_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

अटल स्थिर पूर्णांक speed_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

अटल स्थिर पूर्णांक speed_to_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

अटल पूर्णांक wait_bit(u32 *reg, u32 mask, पूर्णांक मूल्य, u32 timeout)
{
    u32 end = timer_calc(timeout);

    जबतक ((readl(reg) & mask) != मूल्य) {
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
        yield();
    }
    वापस 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#घोषणा XHCI_TIME_POSTPOWER 20

// Check if device attached to port
अटल व्योम
xhci_print_port_state(पूर्णांक loglevel, स्थिर अक्षर *prefix, u32 port, u32 portsc)
{
    u32 pls = xhci_get_field(portsc, XHCI_PORTSC_PLS);
    u32 speed = xhci_get_field(portsc, XHCI_PORTSC_SPEED);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            prefix, port + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, speed, speed_name[speed]);
}

अटल पूर्णांक
xhci_hub_detect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_xhci_s *xhci = container_of(hub->cntl, काष्ठा usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    वापस (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
अटल पूर्णांक
xhci_hub_reset(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_xhci_s *xhci = container_of(hub->cntl, काष्ठा usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[port].portsc);
    अगर (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        वापस -1;

    चयन (xhci_get_field(portsc, XHCI_PORTSC_PLS)) {
    हाल PLS_U0:
        // A USB3 port - controller automatically performs reset
        अवरोध;
    हाल PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_port_state(3, __func__, port, portsc);
        writel(&xhci->pr[port].portsc, portsc | XHCI_PORTSC_PR);
        अवरोध;
    शेष:
        वापस -1;
    }

    // Wait for device to complete reset and be enabled
    u32 end = timer_calc(100);
    क्रम (;;) {
        portsc = readl(&xhci->pr[port].portsc);
        अगर (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            वापस -1;
        अगर (portsc & XHCI_PORTSC_PED)
            // Reset complete
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
        yield();
    }

    पूर्णांक rc = speed_from_xhci[xhci_get_field(portsc, XHCI_PORTSC_SPEED)];
    xhci_print_port_state(1, "XHCI", port, portsc);
    वापस rc;
}

अटल पूर्णांक
xhci_hub_portmap(काष्ठा usbhub_s *hub, u32 vport)
{
    काष्ठा usb_xhci_s *xhci = container_of(hub->cntl, काष्ठा usb_xhci_s, usb);
    u32 pport = vport + 1;

    अगर (vport + 1 >= xhci->usb3.start &&
        vport + 1 < xhci->usb3.start + xhci->usb3.गिनती)
        pport = vport + 2 - xhci->usb3.start;

    अगर (vport + 1 >= xhci->usb2.start &&
        vport + 1 < xhci->usb2.start + xhci->usb2.गिनती)
        pport = vport + 2 - xhci->usb2.start;

    वापस pport;
}

अटल व्योम
xhci_hub_disconnect(काष्ठा usbhub_s *hub, u32 port)
{
    // XXX - should turn the port power off.
}

अटल काष्ठा usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
अटल पूर्णांक
xhci_check_ports(काष्ठा usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_TIME_POSTPOWER);

    काष्ठा usbhub_s hub;
    स_रखो(&hub, 0, माप(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    वापस hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

अटल व्योम
xhci_free_pipes(काष्ठा usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

अटल व्योम
configure_xhci(व्योम *तथ्य)
{
    काष्ठा usb_xhci_s *xhci = तथ्य;
    u32 reg;

    xhci->devs = memalign_ऊपरी(64, माप(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ऊपरी(64, माप(*xhci->eseg));
    xhci->cmds = memalign_ऊपरी(XHCI_RING_आकार, माप(*xhci->cmds));
    xhci->evts = memalign_ऊपरी(XHCI_RING_आकार, माप(*xhci->evts));
    अगर (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        जाओ fail;
    }
    स_रखो(xhci->devs, 0, माप(*xhci->devs) * (xhci->slots + 1));
    स_रखो(xhci->cmds, 0, माप(*xhci->cmds));
    स_रखो(xhci->evts, 0, माप(*xhci->evts));
    स_रखो(xhci->eseg, 0, माप(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    अगर (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        अगर (wait_bit(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            जाओ fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    अगर (wait_bit(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        जाओ fail;
    अगर (wait_bit(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        जाओ fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_निचला, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ऊपरी, 0);
    writel(&xhci->op->crcr_निचला, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ऊपरी, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_निचला = (u32)xhci->evts;
    xhci->eseg->ptr_ऊपरी = 0;
    xhci->eseg->आकार = XHCI_RING_वस्तुएँ;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_निचला, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ऊपरी, 0);
    writel(&xhci->ir->erstba_निचला, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ऊपरी, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    अगर (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ऊपरी(64, माप(*spba) * spb);
        व्योम *pad = memalign_ऊपरी(PAGE_आकार, PAGE_आकार * spb);
        अगर (!spba || !pad) {
            warn_noalloc();
            मुक्त(spba);
            मुक्त(pad);
            जाओ fail;
        }
        पूर्णांक i;
        क्रम (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * PAGE_आकार);
        xhci->devs[0].ptr_निचला = (u32)spba;
        xhci->devs[0].ptr_ऊपरी = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    पूर्णांक गिनती = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    अगर (गिनती)
        // Success
        वापस;

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

अटल काष्ठा usb_xhci_s*
xhci_controller_setup(व्योम *baseaddr)
{
    काष्ठा usb_xhci_s *xhci = malloc_ऊपरी(माप(*xhci));
    अगर (!xhci) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(xhci, 0, माप(*xhci));
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

    अगर (xhci->xcap) {
        u32 off;
        व्योम *addr = baseaddr + xhci->xcap;
        करो {
            काष्ठा xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            चयन (cap & 0xff) {
            हाल 0x02:
                name  = readl(&xcap->तथ्य[0]);
                ports = readl(&xcap->तथ्य[1]);
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
                अगर (name == 0x20425355 /* "USB " */) {
                    अगर (major == 2) {
                        xhci->usb2.start = start;
                        xhci->usb2.गिनती = गिनती;
                    }
                    अगर (major == 3) {
                        xhci->usb3.start = start;
                        xhci->usb3.गिनती = गिनती;
                    }
                }
                अवरोध;
            शेष:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                अवरोध;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } जबतक (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    अगर (PAGE_आकार != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        मुक्त(xhci);
        वापस शून्य;
    }

    वापस xhci;
}

अटल व्योम
xhci_controller_setup_pci(काष्ठा pci_device *pci)
{
    काष्ठा usb_xhci_s *xhci;
    व्योम *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    अगर (!baseaddr)
        वापस;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_setup(baseaddr);
    अगर (!xhci)
        वापस;

    xhci->usb.pci = pci;
    run_thread(configure_xhci, xhci);
}

अटल व्योम
xhci_controller_setup_acpi(काष्ठा acpi_device *dev)
{
    काष्ठा usb_xhci_s *xhci;
    u64 mem, unused;
    व्योम *baseaddr;

    अगर (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
        वापस;
    अगर (mem >= 0x100000000ll)
        वापस;

    baseaddr = (व्योम*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_setup(baseaddr);
    अगर (!xhci)
        वापस;

    xhci->usb.mmio = baseaddr;
    run_thread(configure_xhci, xhci);
}

व्योम
xhci_setup(व्योम)
{
    अगर (! CONFIG_USB_XHCI)
        वापस;

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_setup_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    काष्ठा acpi_device *dev;
    क्रम (dev = acpi_dsdt_find_eisaid(शून्य, xhci_eisaid);
         dev != शून्य;
         dev = acpi_dsdt_find_eisaid(dev, xhci_eisaid)) {
        xhci_controller_setup_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
अटल व्योम xhci_doorbell(काष्ठा usb_xhci_s *xhci, u32 slotid, u32 मूल्य)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, मूल्य);
    काष्ठा xhci_db *db = xhci->db;
    व्योम *addr = &db[slotid].doorbell;
    writel(addr, मूल्य);
}

// Dequeue events on the XHCI command ring generated by the hardware
अटल व्योम xhci_process_events(काष्ठा usb_xhci_s *xhci)
{
    काष्ठा xhci_ring *evts = xhci->evts;

    क्रम (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        काष्ठा xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        अगर ((control & TRB_C) != (cs ? 1 : 0))
            वापस;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->status >> 24) & 0xff;
        चयन (evt_type) {
        हाल ER_TRANSFER:
        हाल ER_COMMAND_COMPLETE:
        {
            काष्ठा xhci_trb  *rtrb = (व्योम*)etrb->ptr_निचला;
            काष्ठा xhci_ring *ring = XHCI_RING(rtrb);
            काष्ठा xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            स_नकल(evt, etrb, माप(*etrb));
            ring->eidx = eidx;
            अवरोध;
        }
        हाल ER_PORT_STATUS_CHANGE:
        {
            u32 port = ((etrb->ptr_निचला >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[port].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_MASK<<XHCI_PORTSC_PLS_SHIFT))
                          | (1<<XHCI_PORTSC_PLS_SHIFT));
            writel(&xhci->pr[port].portsc, pclear);

            xhci_print_port_state(3, __func__, port, portsc);
            अवरोध;
        }
        शेष:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            अवरोध;
        }

        /* move ring index, notify xhci */
        nidx++;
        अगर (nidx == XHCI_RING_वस्तुएँ) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        काष्ठा xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_निचला, erdp);
        writel(&ir->erdp_ऊपरी, 0);
    }
}

// Check if a ring has any pending TRBs
अटल पूर्णांक xhci_ring_busy(काष्ठा xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    वापस (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
अटल पूर्णांक xhci_event_wait(काष्ठा usb_xhci_s *xhci,
                           काष्ठा xhci_ring *ring,
                           u32 timeout)
{
    u32 end = timer_calc(timeout);

    क्रम (;;) {
        xhci_process_events(xhci);
        अगर (!xhci_ring_busy(ring)) {
            u32 status = ring->evt.status;
            वापस (status >> 24) & 0xff;
        }
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
अटल व्योम xhci_trb_fill(काष्ठा xhci_ring *ring
                          , व्योम *तथ्य, u32 xferlen, u32 flags)
{
    काष्ठा xhci_trb *dst = &ring->ring[ring->nidx];
    अगर (flags & TRB_TR_IDT) {
        स_नकल(&dst->ptr_निचला, तथ्य, xferlen);
    } अन्यथा {
        dst->ptr_निचला = (u32)तथ्य;
        dst->ptr_ऊपरी = 0;
    }
    dst->status = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
अटल व्योम xhci_trb_queue(काष्ठा xhci_ring *ring,
                           व्योम *तथ्य, u32 xferlen, u32 flags)
{
    अगर (ring->nidx >= ARRAY_आकार(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, तथ्य, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
अटल पूर्णांक xhci_cmd_submit(काष्ठा usb_xhci_s *xhci, काष्ठा xhci_inctx *inctx
                           , u32 flags)
{
    अगर (inctx) {
        काष्ठा xhci_slotctx *slot = (व्योम*)&inctx[1 << xhci->context64];
        u32 port = ((slot->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[port].portsc);
        अगर (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_port_state(1, __func__, port, portsc);
            वापस -1;
        }
    }

    mutex_lock(&xhci->cmds->lock);
    xhci_trb_queue(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    पूर्णांक rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->lock);
    वापस rc;
}

अटल पूर्णांक xhci_cmd_enable_slot(काष्ठा usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    पूर्णांक cc = xhci_cmd_submit(xhci, शून्य, CR_ENABLE_SLOT << 10);
    अगर (cc != CC_SUCCESS)
        वापस -1;
    वापस (xhci->cmds->evt.control >> 24) & 0xff;
}

अटल पूर्णांक xhci_cmd_disable_slot(काष्ठा usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    वापस xhci_cmd_submit(xhci, शून्य, (CR_DISABLE_SLOT << 10) | (slotid << 24));
}

अटल पूर्णांक xhci_cmd_address_device(काष्ठा usb_xhci_s *xhci, u32 slotid
                                   , काष्ठा xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    वापस xhci_cmd_submit(xhci, inctx
                           , (CR_ADDRESS_DEVICE << 10) | (slotid << 24));
}

अटल पूर्णांक xhci_cmd_configure_endpoint(काष्ठा usb_xhci_s *xhci, u32 slotid
                                       , काष्ठा xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    वापस xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

अटल पूर्णांक xhci_cmd_evaluate_context(काष्ठा usb_xhci_s *xhci, u32 slotid
                                     , काष्ठा xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->add, inctx->del);
    वापस xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

अटल काष्ठा xhci_inctx *
xhci_alloc_inctx(काष्ठा usbdevice_s *usbdev, पूर्णांक maxepid)
{
    काष्ठा usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, काष्ठा usb_xhci_s, usb);
    पूर्णांक आकार = (माप(काष्ठा xhci_inctx) * 33) << xhci->context64;
    काष्ठा xhci_inctx *in = memalign_tmphigh(2048 << xhci->context64, आकार);
    अगर (!in) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(in, 0, आकार);

    काष्ठा xhci_slotctx *slot = (व्योम*)&in[1 << xhci->context64];
    slot->ctx[0]    |= maxepid << 27; // context entries
    slot->ctx[0]    |= speed_to_xhci[usbdev->speed] << 20;

    // Set high-speed hub flags.
    काष्ठा usbdevice_s *hubdev = usbdev->hub->usbdev;
    अगर (hubdev) {
        अगर (usbdev->speed == USB_LOWSPEED || usbdev->speed == USB_FULLSPEED) {
            काष्ठा xhci_pipe *hpipe = container_of(
                hubdev->defpipe, काष्ठा xhci_pipe, pipe);
            अगर (hubdev->speed == USB_HIGHSPEED) {
                slot->ctx[2] |= hpipe->slotid;
                slot->ctx[2] |= (usbdev->port+1) << 8;
            } अन्यथा {
                काष्ठा xhci_slotctx *hslot = (व्योम*)xhci->devs[hpipe->slotid].ptr_निचला;
                slot->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        जबतक (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->port+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        slot->ctx[0]    |= route;
    }

    slot->ctx[1]    |= (usbdev->port+1) << 16;

    वापस in;
}

अटल पूर्णांक xhci_config_hub(काष्ठा usbhub_s *hub)
{
    काष्ठा usb_xhci_s *xhci = container_of(
        hub->cntl, काष्ठा usb_xhci_s, usb);
    काष्ठा xhci_pipe *pipe = container_of(
        hub->usbdev->defpipe, काष्ठा xhci_pipe, pipe);
    काष्ठा xhci_slotctx *hdslot = (व्योम*)xhci->devs[pipe->slotid].ptr_निचला;
    अगर ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        वापस 0;
    काष्ठा xhci_inctx *in = xhci_alloc_inctx(hub->usbdev, 1);
    अगर (!in)
        वापस -1;
    in->add = 0x01;
    काष्ठा xhci_slotctx *slot = (व्योम*)&in[1 << xhci->context64];
    slot->ctx[0] |= 1 << 26;
    slot->ctx[1] |= hub->portcount << 24;

    पूर्णांक cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
    मुक्त(in);
    अगर (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        वापस -1;
    }
    वापस 0;
}

अटल काष्ठा usb_pipe *
xhci_alloc_pipe(काष्ठा usbdevice_s *usbdev
                , काष्ठा usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    काष्ठा usb_xhci_s *xhci = container_of(
        usbdev->hub->cntl, काष्ठा usb_xhci_s, usb);
    काष्ठा xhci_pipe *pipe;
    u32 epid;

    अगर (epdesc->bEndpointAddress == 0) {
        epid = 1;
    } अन्यथा {
        epid = (epdesc->bEndpointAddress & 0x0f) * 2;
        epid += (epdesc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;
    }

    अगर (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ऊपरी(XHCI_RING_आकार, माप(*pipe));
    अन्यथा
        pipe = memalign_निचला(XHCI_RING_आकार, माप(*pipe));
    अगर (!pipe) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(pipe, 0, माप(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    अगर (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ऊपरी(pipe->pipe.maxpacket);
        अगर (!pipe->buf) {
            warn_noalloc();
            मुक्त(pipe);
            वापस शून्य;
        }
    }

    // Allocate input context and initialize endpoint info.
    काष्ठा xhci_inctx *in = xhci_alloc_inctx(usbdev, epid);
    अगर (!in)
        जाओ fail;
    in->add = 0x01 | (1 << epid);
    काष्ठा xhci_epctx *ep = (व्योम*)&in[(pipe->epid+1) << xhci->context64];
    अगर (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    अगर (epdesc->bEndpointAddress & USB_DIR_IN
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_निचला  = (u32)&pipe->reqs.ring[0];
    ep->deq_निचला  |= 1;         // dcs
    ep->लंबाई   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    अगर (pipe->epid == 1) {
        अगर (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            पूर्णांक ret = xhci_config_hub(usbdev->hub);
            अगर (ret)
                जाओ fail;
        }
        // Enable slot.
        u32 आकार = (माप(काष्ठा xhci_slotctx) * 32) << xhci->context64;
        काष्ठा xhci_slotctx *dev = memalign_ऊपरी(1024 << xhci->context64, आकार);
        अगर (!dev) {
            warn_noalloc();
            जाओ fail;
        }
        पूर्णांक slotid = xhci_cmd_enable_slot(xhci);
        अगर (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            मुक्त(dev);
            जाओ fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        स_रखो(dev, 0, आकार);
        xhci->devs[slotid].ptr_निचला = (u32)dev;
        xhci->devs[slotid].ptr_ऊपरी = 0;

        // Send set_address command.
        पूर्णांक cc = xhci_cmd_address_device(xhci, slotid, in);
        अगर (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_slot(xhci, slotid);
            अगर (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                जाओ fail;
            }
            xhci->devs[slotid].ptr_निचला = 0;
            मुक्त(dev);
            जाओ fail;
        }
        pipe->slotid = slotid;
    } अन्यथा {
        काष्ठा xhci_pipe *defpipe = container_of(
            usbdev->defpipe, काष्ठा xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        पूर्णांक cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, in);
        अगर (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            जाओ fail;
        }
    }
    मुक्त(in);
    वापस &pipe->pipe;

fail:
    मुक्त(pipe->buf);
    मुक्त(pipe);
    मुक्त(in);
    वापस शून्य;
}

काष्ठा usb_pipe *
xhci_realloc_pipe(काष्ठा usbdevice_s *usbdev, काष्ठा usb_pipe *upipe
                  , काष्ठा usb_endpoint_descriptor *epdesc)
{
    अगर (!CONFIG_USB_XHCI)
        वापस शून्य;
    अगर (!epdesc) {
        usb_add_freelist(upipe);
        वापस शून्य;
    }
    अगर (!upipe)
        वापस xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    पूर्णांक oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    काष्ठा xhci_pipe *pipe = container_of(upipe, काष्ठा xhci_pipe, pipe);
    काष्ठा usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, काष्ठा usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    अगर (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        वापस upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    काष्ठा xhci_inctx *in = xhci_alloc_inctx(usbdev, 1);
    अगर (!in)
        वापस upipe;
    in->add = (1 << 1);
    काष्ठा xhci_epctx *ep = (व्योम*)&in[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    पूर्णांक cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, in);
    अगर (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    मुक्त(in);

    वापस upipe;
}

// Submit a USB "setup" message request to the pipe's ring
अटल व्योम xhci_xfer_setup(काष्ठा xhci_pipe *pipe, पूर्णांक dir, व्योम *cmd
                            , व्योम *तथ्य, पूर्णांक datalen)
{
    काष्ठा usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, काष्ठा usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, cmd, USB_CONTROL_SETUP_आकार
                   , (TR_SETUP << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    अगर (datalen)
        xhci_trb_queue(&pipe->reqs, तथ्य, datalen, (TR_तथ्य << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_queue(&pipe->reqs, शून्य, 0, (TR_STATUS << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
अटल व्योम xhci_xfer_normal(काष्ठा xhci_pipe *pipe,
                             व्योम *तथ्य, पूर्णांक datalen)
{
    काष्ठा usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, काष्ठा usb_xhci_s, usb);
    xhci_trb_queue(&pipe->reqs, तथ्य, datalen, (TR_NORMAL << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

पूर्णांक
xhci_send_pipe(काष्ठा usb_pipe *p, पूर्णांक dir, स्थिर व्योम *cmd
               , व्योम *तथ्य, पूर्णांक datalen)
{
    अगर (!CONFIG_USB_XHCI)
        वापस -1;
    काष्ठा xhci_pipe *pipe = container_of(p, काष्ठा xhci_pipe, pipe);
    काष्ठा usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, काष्ठा usb_xhci_s, usb);

    अगर (cmd) {
        स्थिर काष्ठा usb_ctrlrequest *req = cmd;
        अगर (req->bRequest == USB_REQ_SET_ADDRESS)
            // Set address command sent during xhci_alloc_pipe.
            वापस 0;
        xhci_xfer_setup(pipe, dir, (व्योम*)req, तथ्य, datalen);
    } अन्यथा {
        xhci_xfer_normal(pipe, तथ्य, datalen);
    }

    पूर्णांक cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_time(p, datalen));
    अगर (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        वापस -1;
    }

    वापस 0;
}

पूर्णांक VISIBLE32FLAT
xhci_poll_intr(काष्ठा usb_pipe *p, व्योम *तथ्य)
{
    अगर (!CONFIG_USB_XHCI)
        वापस -1;

    काष्ठा xhci_pipe *pipe = container_of(p, काष्ठा xhci_pipe, pipe);
    काष्ठा usb_xhci_s *xhci = container_of(
        pipe->pipe.cntl, काष्ठा usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    व्योम *buf = pipe->buf;
    पूर्णांक bufused = pipe->bufused;

    अगर (!bufused) {
        xhci_xfer_normal(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        वापस -1;
    }

    xhci_process_events(xhci);
    अगर (xhci_ring_busy(&pipe->reqs))
        वापस -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.status,
            pipe->reqs.evt.control,
            तथ्य, buf, len);
    स_नकल(तथ्य, buf, len);
    xhci_xfer_normal(pipe, buf, len);
    वापस 0;
}
