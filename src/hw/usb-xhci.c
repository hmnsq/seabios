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

#وصف_ڪريو XHCI_RING_شيون          16
#وصف_ڪريو XHCI_RING_سائيز           (XHCI_RING_شيون*حجم(بنية xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#وصف_ڪريو XHCI_RING(_trb)          \
    ((بنية xhci_ring*)((u32)(_trb) & ~(XHCI_RING_سائيز-1)))

// --------------------------------------------------------------
// bit definitions

#وصف_ڪريو XHCI_CMD_RS              (1<<0)
#وصف_ڪريو XHCI_CMD_HCRST           (1<<1)
#وصف_ڪريو XHCI_CMD_INTE            (1<<2)
#وصف_ڪريو XHCI_CMD_HSEE            (1<<3)
#وصف_ڪريو XHCI_CMD_LHCRST          (1<<7)
#وصف_ڪريو XHCI_CMD_CSS             (1<<8)
#وصف_ڪريو XHCI_CMD_CRS             (1<<9)
#وصف_ڪريو XHCI_CMD_EWE             (1<<10)
#وصف_ڪريو XHCI_CMD_EU3S            (1<<11)

#وصف_ڪريو XHCI_STS_HCH             (1<<0)
#وصف_ڪريو XHCI_STS_HSE             (1<<2)
#وصف_ڪريو XHCI_STS_EINT            (1<<3)
#وصف_ڪريو XHCI_STS_PCD             (1<<4)
#وصف_ڪريو XHCI_STS_SSS             (1<<8)
#وصف_ڪريو XHCI_STS_RSS             (1<<9)
#وصف_ڪريو XHCI_STS_SRE             (1<<10)
#وصف_ڪريو XHCI_STS_CNR             (1<<11)
#وصف_ڪريو XHCI_STS_HCE             (1<<12)

#وصف_ڪريو XHCI_PORTSC_CCS          (1<<0)
#وصف_ڪريو XHCI_PORTSC_PED          (1<<1)
#وصف_ڪريو XHCI_PORTSC_OCA          (1<<3)
#وصف_ڪريو XHCI_PORTSC_PR           (1<<4)
#وصف_ڪريو XHCI_PORTSC_PLS_سرڪايو        5
#وصف_ڪريو XHCI_PORTSC_PLS_نقاب     0xf
#وصف_ڪريو XHCI_PORTSC_PP           (1<<9)
#وصف_ڪريو XHCI_PORTSC_رفتار_سرڪايو      10
#وصف_ڪريو XHCI_PORTSC_رفتار_نقاب   0xf
#وصف_ڪريو XHCI_PORTSC_رفتار_ڀريل   (1<<10)
#وصف_ڪريو XHCI_PORTSC_رفتار_هيٺيون    (2<<10)
#وصف_ڪريو XHCI_PORTSC_رفتار_مٿاهون   (3<<10)
#وصف_ڪريو XHCI_PORTSC_رفتار_SUPER  (4<<10)
#وصف_ڪريو XHCI_PORTSC_PIC_سرڪايو        14
#وصف_ڪريو XHCI_PORTSC_PIC_نقاب     0x3
#وصف_ڪريو XHCI_PORTSC_LWS          (1<<16)
#وصف_ڪريو XHCI_PORTSC_CSC          (1<<17)
#وصف_ڪريو XHCI_PORTSC_PEC          (1<<18)
#وصف_ڪريو XHCI_PORTSC_WRC          (1<<19)
#وصف_ڪريو XHCI_PORTSC_OCC          (1<<20)
#وصف_ڪريو XHCI_PORTSC_PRC          (1<<21)
#وصف_ڪريو XHCI_PORTSC_PLC          (1<<22)
#وصف_ڪريو XHCI_PORTSC_CEC          (1<<23)
#وصف_ڪريو XHCI_PORTSC_CAS          (1<<24)
#وصف_ڪريو XHCI_PORTSC_WCE          (1<<25)
#وصف_ڪريو XHCI_PORTSC_WDE          (1<<26)
#وصف_ڪريو XHCI_PORTSC_WOE          (1<<27)
#وصف_ڪريو XHCI_PORTSC_DR           (1<<30)
#وصف_ڪريو XHCI_PORTSC_WPR          (1<<31)

#وصف_ڪريو TRB_C               (1<<0)
#وصف_ڪريو TRB_TYPE_سرڪايو          10
#وصف_ڪريو TRB_TYPE_نقاب       0x3f
#وصف_ڪريو TRB_TYPE(t)         (((t) >> TRB_TYPE_سرڪايو) & TRB_TYPE_نقاب)

#وصف_ڪريو TRB_EV_ED           (1<<2)

#وصف_ڪريو TRB_TR_ENT          (1<<1)
#وصف_ڪريو TRB_TR_ISP          (1<<2)
#وصف_ڪريو TRB_TR_NS           (1<<3)
#وصف_ڪريو TRB_TR_CH           (1<<4)
#وصف_ڪريو TRB_TR_IOC          (1<<5)
#وصف_ڪريو TRB_TR_IDT          (1<<6)
#وصف_ڪريو TRB_TR_TBC_سرڪايو        7
#وصف_ڪريو TRB_TR_TBC_نقاب     0x3
#وصف_ڪريو TRB_TR_BEI          (1<<9)
#وصف_ڪريو TRB_TR_TLBPC_سرڪايو      16
#وصف_ڪريو TRB_TR_TLBPC_نقاب   0xf
#وصف_ڪريو TRB_TR_FRAMEID_سرڪايو    20
#وصف_ڪريو TRB_TR_FRAMEID_نقاب 0x7ff
#وصف_ڪريو TRB_TR_SIA          (1<<31)

#وصف_ڪريو TRB_TR_DIR          (1<<16)

#وصف_ڪريو TRB_CR_SLOTID_سرڪايو     24
#وصف_ڪريو TRB_CR_SLOTID_نقاب  0xff
#وصف_ڪريو TRB_CR_EPID_سرڪايو       16
#وصف_ڪريو TRB_CR_EPID_نقاب    0x1f

#وصف_ڪريو TRB_CR_BSR          (1<<9)
#وصف_ڪريو TRB_CR_DC           (1<<9)

#وصف_ڪريو TRB_LK_TC           (1<<1)

#وصف_ڪريو TRB_INTR_سرڪايو          22
#وصف_ڪريو TRB_INTR_نقاب       0x3ff
#وصف_ڪريو TRB_INTR(t)         (((t).حالت >> TRB_INTR_سرڪايو) & TRB_INTR_نقاب)

تعريفنوع تعداد TRBType {
    TRB_RESERVED = 0,
    TR_عام,
    TR_تياري,
    TR_معلومات,
    TR_حالت,
    TR_ISOCH,
    TR_LINK,
    TR_EVDATA,
    TR_NOOP,
    CR_ENABLE_جڳهه,
    CR_DISABLE_جڳهه,
    CR_پتو_DEVICE,
    CR_CONFIGURE_ENDPOINT,
    CR_EVALUATE_CONTEXT,
    CR_RESET_ENDPOINT,
    CR_STOP_ENDPOINT,
    CR_سيٽ_TR_قطار_مان_ڪڍو,
    CR_RESET_DEVICE,
    CR_FORCE_EVENT,
    CR_NEGOTIATE_BW,
    CR_سيٽ_LATENCY_TOLERANCE,
    CR_GET_پورٽ_BANDWIDTH,
    CR_FORCE_سرو,
    CR_NOOP,
    ER_TRANSFER = 32,
    ER_COMMAND_COMPLETE,
    ER_پورٽ_حالت_CHANGE,
    ER_BANDWIDTH_REQUEST,
    ER_DOORBELL,
    ER_HOST_CONTROLLER,
    ER_DEVICE_NOTIFICATION,
    ER_MFINDEX_WRAP,
} TRBType;

تعريفنوع تعداد TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_معلومات_بفر_ERROR,
    CC_BABBLE_DETECTED,
    CC_USB_TRANSACTION_ERROR,
    CC_TRB_ERROR,
    CC_STALL_ERROR,
    CC_RESOURCE_ERROR,
    CC_BANDWIDTH_ERROR,
    CC_NO_SLOTS_ERROR,
    CC_INVALID_وهڪرو_TYPE_ERROR,
    CC_جڳهه_NOT_ENABLED_ERROR,
    CC_EP_NOT_ENABLED_ERROR,
    CC_SHORT_PACKET,
    CC_RING_UNDERRUN,
    CC_RING_OVERRUN,
    CC_VF_ER_ڀريل,
    CC_PARAMETER_ERROR,
    CC_BANDWIDTH_OVERRUN,
    CC_CONTEXT_STATE_ERROR,
    CC_NO_PING_RESPONSE_ERROR,
    CC_EVENT_RING_ڀريل_ERROR,
    CC_INCOMPATIBLE_DEVICE_ERROR,
    CC_MISSED_SERVICE_ERROR,
    CC_COMMAND_RING_STOPPED,
    CC_COMMAND_ABORTED,
    CC_STOPPED,
    CC_STOPPED_ڊيگهائي_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_بفر_OVERRUN = 31,
    CC_EVENT_گم_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_وهڪرو_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_ورهايو_TRANSACTION_ERROR
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
    PLS_COMPILANCE_گهڻائي = 10,
    PLS_TEST_گهڻائي       = 11,
    PLS_RESUME          = 15,
};

#وصف_ڪريو xhci_get_ميدان(معلومات, ميدان)             \
    (((معلومات) >> ميدان##_SHIFT) & ميدان##_MASK)

// --------------------------------------------------------------
// state structs

بنية xhci_ring {
    بنية xhci_trb      ring[XHCI_RING_شيون];
    بنية xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    بنية mutex_s       تالو;
};

بنية xhci_portmap {
    u8 شروعات;
    u8 ڳڻپ;
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

ساكن ثابت حرف *رفتار_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ساكن ثابت صحيح رفتار_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ساكن ثابت صحيح رفتار_منزل_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ساكن صحيح wait_ٻٽو_انگ(u32 *reg, u32 نقاب, صحيح قدر, u32 timeout)
{
    u32 پڇاڙي = timer_calc(timeout);

    طالما ((readl(reg) & نقاب) != قدر) {
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
    أرجع 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#وصف_ڪريو XHCI_وقت_POSTPOWER 20

// Check if device attached to port
ساكن فراغ
xhci_print_پورٽ_state(صحيح loglevel, ثابت حرف *اڳياڙي, u32 پورٽ, u32 portsc)
{
    u32 pls = xhci_get_ميدان(portsc, XHCI_PORTSC_PLS);
    u32 رفتار = xhci_get_ميدان(portsc, XHCI_PORTSC_رفتار);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            اڳياڙي, پورٽ + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, رفتار, رفتار_name[رفتار]);
}

ساكن صحيح
xhci_hub_detect(بنية usbhub_s *hub, u32 پورٽ)
{
    بنية usb_xhci_s *xhci = container_جو(hub->cntl, بنية usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[پورٽ].portsc);
    أرجع (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ساكن صحيح
xhci_hub_reset(بنية usbhub_s *hub, u32 پورٽ)
{
    بنية usb_xhci_s *xhci = container_جو(hub->cntl, بنية usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[پورٽ].portsc);
    إذا (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        أرجع -1;

    بدل (xhci_get_ميدان(portsc, XHCI_PORTSC_PLS)) {
    حالة PLS_U0:
        // A USB3 port - controller automatically performs reset
        اكسر;
    حالة PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_پورٽ_state(3, __func__, پورٽ, portsc);
        writel(&xhci->pr[پورٽ].portsc, portsc | XHCI_PORTSC_PR);
        اكسر;
    افتراضي:
        أرجع -1;
    }

    // Wait for device to complete reset and be enabled
    u32 پڇاڙي = timer_calc(100);
    لكل (;;) {
        portsc = readl(&xhci->pr[پورٽ].portsc);
        إذا (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            أرجع -1;
        إذا (portsc & XHCI_PORTSC_PED)
            // Reset complete
            اكسر;
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }

    صحيح rc = رفتار_from_xhci[xhci_get_ميدان(portsc, XHCI_PORTSC_رفتار)];
    xhci_print_پورٽ_state(1, "XHCI", پورٽ, portsc);
    أرجع rc;
}

ساكن صحيح
xhci_hub_portmap(بنية usbhub_s *hub, u32 vport)
{
    بنية usb_xhci_s *xhci = container_جو(hub->cntl, بنية usb_xhci_s, usb);
    u32 pport = vport + 1;

    إذا (vport + 1 >= xhci->usb3.شروعات &&
        vport + 1 < xhci->usb3.شروعات + xhci->usb3.ڳڻپ)
        pport = vport + 2 - xhci->usb3.شروعات;

    إذا (vport + 1 >= xhci->usb2.شروعات &&
        vport + 1 < xhci->usb2.شروعات + xhci->usb2.ڳڻپ)
        pport = vport + 2 - xhci->usb2.شروعات;

    أرجع pport;
}

ساكن فراغ
xhci_hub_disconnect(بنية usbhub_s *hub, u32 پورٽ)
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
    msleep(XHCI_وقت_POSTPOWER);

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    أرجع hub.devcount;
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
configure_xhci(فراغ *معلومات)
{
    بنية usb_xhci_s *xhci = معلومات;
    u32 reg;

    xhci->devs = memalign_مٿاهون(64, حجم(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_مٿاهون(64, حجم(*xhci->eseg));
    xhci->cmds = memalign_مٿاهون(XHCI_RING_سائيز, حجم(*xhci->cmds));
    xhci->evts = memalign_مٿاهون(XHCI_RING_سائيز, حجم(*xhci->evts));
    إذا (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        اذهب fail;
    }
    memset(xhci->devs, 0, حجم(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, حجم(*xhci->cmds));
    memset(xhci->evts, 0, حجم(*xhci->evts));
    memset(xhci->eseg, 0, حجم(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    إذا (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        إذا (wait_ٻٽو_انگ(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            اذهب fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    إذا (wait_ٻٽو_انگ(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        اذهب fail;
    إذا (wait_ٻٽو_انگ(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        اذهب fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_هيٺيون, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_مٿاهون, 0);
    writel(&xhci->op->crcr_هيٺيون, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_مٿاهون, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_هيٺيون = (u32)xhci->evts;
    xhci->eseg->ptr_مٿاهون = 0;
    xhci->eseg->سائيز = XHCI_RING_شيون;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_هيٺيون, (u32)xhci->evts);
    writel(&xhci->ir->erdp_مٿاهون, 0);
    writel(&xhci->ir->erstba_هيٺيون, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_مٿاهون, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    إذا (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_مٿاهون(64, حجم(*spba) * spb);
        فراغ *pad = memalign_مٿاهون(صفحو_سائيز, صفحو_سائيز * spb);
        إذا (!spba || !pad) {
            warn_noalloc();
            حرر(spba);
            حرر(pad);
            اذهب fail;
        }
        صحيح i;
        لكل (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * صفحو_سائيز);
        xhci->devs[0].ptr_هيٺيون = (u32)spba;
        xhci->devs[0].ptr_مٿاهون = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    صحيح ڳڻپ = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    إذا (ڳڻپ)
        // Success
        أرجع;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_ٻٽو_انگ(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    حرر(xhci->eseg);
    حرر(xhci->evts);
    حرر(xhci->cmds);
    حرر(xhci->devs);
    حرر(xhci);
}

ساكن بنية usb_xhci_s*
xhci_controller_تياري(فراغ *baseaddr)
{
    بنية usb_xhci_s *xhci = malloc_مٿاهون(حجم(*xhci));
    إذا (!xhci) {
        warn_noalloc();
        أرجع عدم;
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

    إذا (xhci->xcap) {
        u32 off;
        فراغ *addr = baseaddr + xhci->xcap;
        افعل {
            بنية xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            بدل (cap & 0xff) {
            حالة 0x02:
                name  = readl(&xcap->معلومات[0]);
                ports = readl(&xcap->معلومات[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 ڳڻپ = (ports >> 8) & 0xff;
                u8 شروعات = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , ڳڻپ, شروعات
                        , ports >> 16);
                إذا (name == 0x20425355 /* "USB " */) {
                    إذا (major == 2) {
                        xhci->usb2.شروعات = شروعات;
                        xhci->usb2.ڳڻپ = ڳڻپ;
                    }
                    إذا (major == 3) {
                        xhci->usb3.شروعات = شروعات;
                        xhci->usb3.ڳڻپ = ڳڻپ;
                    }
                }
                اكسر;
            افتراضي:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                اكسر;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } طالما (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    إذا (صفحو_سائيز != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        حرر(xhci);
        أرجع عدم;
    }

    أرجع xhci;
}

ساكن فراغ
xhci_controller_تياري_pci(بنية pci_device *pci)
{
    بنية usb_xhci_s *xhci;
    فراغ *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_بنياد_پتو_0);
    إذا (!baseaddr)
        أرجع;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_تياري(baseaddr);
    إذا (!xhci)
        أرجع;

    xhci->usb.pci = pci;
    لاڳيتو_thread(configure_xhci, xhci);
}

ساكن فراغ
xhci_controller_تياري_acpi(بنية acpi_device *dev)
{
    بنية usb_xhci_s *xhci;
    u64 mem, اڻ_استعمال_ٿيل;
    فراغ *baseaddr;

    إذا (acpi_dsdt_ڳوليو_mem(dev, &mem, &اڻ_استعمال_ٿيل) < 0)
        أرجع;
    إذا (mem >= 0x100000000ll)
        أرجع;

    baseaddr = (فراغ*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_تياري(baseaddr);
    إذا (!xhci)
        أرجع;

    xhci->usb.mmio = baseaddr;
    لاڳيتو_thread(configure_xhci, xhci);
}

فراغ
xhci_تياري(فراغ)
{
    إذا (! CONFIG_USB_XHCI)
        أرجع;

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_تياري_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    بنية acpi_device *dev;
    لكل (dev = acpi_dsdt_ڳوليو_eisaid(عدم, xhci_eisaid);
         dev != عدم;
         dev = acpi_dsdt_ڳوليو_eisaid(dev, xhci_eisaid)) {
        xhci_controller_تياري_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ساكن فراغ xhci_doorbell(بنية usb_xhci_s *xhci, u32 slotid, u32 قدر)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, قدر);
    بنية xhci_db *db = xhci->db;
    فراغ *addr = &db[slotid].doorbell;
    writel(addr, قدر);
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
        إذا ((control & TRB_C) != (cs ? 1 : 0))
            أرجع;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->حالت >> 24) & 0xff;
        بدل (evt_type) {
        حالة ER_TRANSFER:
        حالة ER_COMMAND_COMPLETE:
        {
            بنية xhci_trb  *rtrb = (فراغ*)etrb->ptr_هيٺيون;
            بنية xhci_ring *ring = XHCI_RING(rtrb);
            بنية xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            يادگيري_نقل(evt, etrb, حجم(*etrb));
            ring->eidx = eidx;
            اكسر;
        }
        حالة ER_پورٽ_حالت_CHANGE:
        {
            u32 پورٽ = ((etrb->ptr_هيٺيون >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[پورٽ].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_نقاب<<XHCI_PORTSC_PLS_سرڪايو))
                          | (1<<XHCI_PORTSC_PLS_سرڪايو));
            writel(&xhci->pr[پورٽ].portsc, pclear);

            xhci_print_پورٽ_state(3, __func__, پورٽ, portsc);
            اكسر;
        }
        افتراضي:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            اكسر;
        }

        /* move ring index, notify xhci */
        nidx++;
        إذا (nidx == XHCI_RING_شيون) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        بنية xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_هيٺيون, erdp);
        writel(&ir->erdp_مٿاهون, 0);
    }
}

// Check if a ring has any pending TRBs
ساكن صحيح xhci_ring_busy(بنية xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    أرجع (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ساكن صحيح xhci_event_wait(بنية usb_xhci_s *xhci,
                           بنية xhci_ring *ring,
                           u32 timeout)
{
    u32 پڇاڙي = timer_calc(timeout);

    لكل (;;) {
        xhci_process_events(xhci);
        إذا (!xhci_ring_busy(ring)) {
            u32 حالت = ring->evt.حالت;
            أرجع (حالت >> 24) & 0xff;
        }
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ساكن فراغ xhci_trb_fill(بنية xhci_ring *ring
                          , فراغ *معلومات, u32 xferlen, u32 flags)
{
    بنية xhci_trb *dst = &ring->ring[ring->nidx];
    إذا (flags & TRB_TR_IDT) {
        يادگيري_نقل(&dst->ptr_هيٺيون, معلومات, xferlen);
    } وإلا {
        dst->ptr_هيٺيون = (u32)معلومات;
        dst->ptr_مٿاهون = 0;
    }
    dst->حالت = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ساكن فراغ xhci_trb_قطار(بنية xhci_ring *ring,
                           فراغ *معلومات, u32 xferlen, u32 flags)
{
    إذا (ring->nidx >= ARRAY_سائيز(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, معلومات, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
ساكن صحيح xhci_cmd_submit(بنية usb_xhci_s *xhci, بنية xhci_inctx *inctx
                           , u32 flags)
{
    إذا (inctx) {
        بنية xhci_slotctx *جڳهه = (فراغ*)&inctx[1 << xhci->context64];
        u32 پورٽ = ((جڳهه->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[پورٽ].portsc);
        إذا (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_پورٽ_state(1, __func__, پورٽ, portsc);
            أرجع -1;
        }
    }

    mutex_تالو(&xhci->cmds->تالو);
    xhci_trb_قطار(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    صحيح rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->تالو);
    أرجع rc;
}

ساكن صحيح xhci_cmd_enable_جڳهه(بنية usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    صحيح cc = xhci_cmd_submit(xhci, عدم, CR_ENABLE_جڳهه << 10);
    إذا (cc != CC_SUCCESS)
        أرجع -1;
    أرجع (xhci->cmds->evt.control >> 24) & 0xff;
}

ساكن صحيح xhci_cmd_disable_جڳهه(بنية usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    أرجع xhci_cmd_submit(xhci, عدم, (CR_DISABLE_جڳهه << 10) | (slotid << 24));
}

ساكن صحيح xhci_cmd_پتو_device(بنية usb_xhci_s *xhci, u32 slotid
                                   , بنية xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    أرجع xhci_cmd_submit(xhci, inctx
                           , (CR_پتو_DEVICE << 10) | (slotid << 24));
}

ساكن صحيح xhci_cmd_configure_endpoint(بنية usb_xhci_s *xhci, u32 slotid
                                       , بنية xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->جوڙيو, inctx->del);
    أرجع xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ساكن صحيح xhci_cmd_evaluate_context(بنية usb_xhci_s *xhci, u32 slotid
                                     , بنية xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->جوڙيو, inctx->del);
    أرجع xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ساكن بنية xhci_inctx *
xhci_alloc_inctx(بنية usbdevice_s *usbdev, صحيح maxepid)
{
    بنية usb_xhci_s *xhci = container_جو(
        usbdev->hub->cntl, بنية usb_xhci_s, usb);
    صحيح سائيز = (حجم(بنية xhci_inctx) * 33) << xhci->context64;
    بنية xhci_inctx *اندريون = memalign_tmphigh(2048 << xhci->context64, سائيز);
    إذا (!اندريون) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(اندريون, 0, سائيز);

    بنية xhci_slotctx *جڳهه = (فراغ*)&اندريون[1 << xhci->context64];
    جڳهه->ctx[0]    |= maxepid << 27; // context entries
    جڳهه->ctx[0]    |= رفتار_منزل_xhci[usbdev->رفتار] << 20;

    // Set high-speed hub flags.
    بنية usbdevice_s *hubdev = usbdev->hub->usbdev;
    إذا (hubdev) {
        إذا (usbdev->رفتار == USB_LOWSPEED || usbdev->رفتار == USB_FULLSPEED) {
            بنية xhci_pipe *hpipe = container_جو(
                hubdev->defpipe, بنية xhci_pipe, pipe);
            إذا (hubdev->رفتار == USB_HIGHSPEED) {
                جڳهه->ctx[2] |= hpipe->slotid;
                جڳهه->ctx[2] |= (usbdev->پورٽ+1) << 8;
            } وإلا {
                بنية xhci_slotctx *hslot = (فراغ*)xhci->devs[hpipe->slotid].ptr_هيٺيون;
                جڳهه->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        طالما (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->پورٽ+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        جڳهه->ctx[0]    |= route;
    }

    جڳهه->ctx[1]    |= (usbdev->پورٽ+1) << 16;

    أرجع اندريون;
}

ساكن صحيح xhci_config_hub(بنية usbhub_s *hub)
{
    بنية usb_xhci_s *xhci = container_جو(
        hub->cntl, بنية usb_xhci_s, usb);
    بنية xhci_pipe *pipe = container_جو(
        hub->usbdev->defpipe, بنية xhci_pipe, pipe);
    بنية xhci_slotctx *hdslot = (فراغ*)xhci->devs[pipe->slotid].ptr_هيٺيون;
    إذا ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        أرجع 0;
    بنية xhci_inctx *اندريون = xhci_alloc_inctx(hub->usbdev, 1);
    إذا (!اندريون)
        أرجع -1;
    اندريون->جوڙيو = 0x01;
    بنية xhci_slotctx *جڳهه = (فراغ*)&اندريون[1 << xhci->context64];
    جڳهه->ctx[0] |= 1 << 26;
    جڳهه->ctx[1] |= hub->portcount << 24;

    صحيح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, اندريون);
    حرر(اندريون);
    إذا (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        أرجع -1;
    }
    أرجع 0;
}

ساكن بنية usb_pipe *
xhci_alloc_pipe(بنية usbdevice_s *usbdev
                , بنية usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bm_خاصيتون & USB_ENDPOINT_XFERTYPE_نقاب;
    بنية usb_xhci_s *xhci = container_جو(
        usbdev->hub->cntl, بنية usb_xhci_s, usb);
    بنية xhci_pipe *pipe;
    u32 epid;

    إذا (epdesc->b_Endpoint_پتو == 0) {
        epid = 1;
    } وإلا {
        epid = (epdesc->b_Endpoint_پتو & 0x0f) * 2;
        epid += (epdesc->b_Endpoint_پتو & USB_DIR_اندريون) ? 1 : 0;
    }

    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_مٿاهون(XHCI_RING_سائيز, حجم(*pipe));
    وإلا
        pipe = memalign_هيٺيون(XHCI_RING_سائيز, حجم(*pipe));
    إذا (!pipe) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(pipe, 0, حجم(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    إذا (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_مٿاهون(pipe->pipe.maxpacket);
        إذا (!pipe->buf) {
            warn_noalloc();
            حرر(pipe);
            أرجع عدم;
        }
    }

    // Allocate input context and initialize endpoint info.
    بنية xhci_inctx *اندريون = xhci_alloc_inctx(usbdev, epid);
    إذا (!اندريون)
        اذهب fail;
    اندريون->جوڙيو = 0x01 | (1 << epid);
    بنية xhci_epctx *ep = (فراغ*)&اندريون[(pipe->epid+1) << xhci->context64];
    إذا (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    إذا (epdesc->b_Endpoint_پتو & USB_DIR_اندريون
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_هيٺيون  = (u32)&pipe->reqs.ring[0];
    ep->deq_هيٺيون  |= 1;         // dcs
    ep->ڊيگهائي   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    إذا (pipe->epid == 1) {
        إذا (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            صحيح ret = xhci_config_hub(usbdev->hub);
            إذا (ret)
                اذهب fail;
        }
        // Enable slot.
        u32 سائيز = (حجم(بنية xhci_slotctx) * 32) << xhci->context64;
        بنية xhci_slotctx *dev = memalign_مٿاهون(1024 << xhci->context64, سائيز);
        إذا (!dev) {
            warn_noalloc();
            اذهب fail;
        }
        صحيح slotid = xhci_cmd_enable_جڳهه(xhci);
        إذا (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            حرر(dev);
            اذهب fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, سائيز);
        xhci->devs[slotid].ptr_هيٺيون = (u32)dev;
        xhci->devs[slotid].ptr_مٿاهون = 0;

        // Send set_address command.
        صحيح cc = xhci_cmd_پتو_device(xhci, slotid, اندريون);
        إذا (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_جڳهه(xhci, slotid);
            إذا (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                اذهب fail;
            }
            xhci->devs[slotid].ptr_هيٺيون = 0;
            حرر(dev);
            اذهب fail;
        }
        pipe->slotid = slotid;
    } وإلا {
        بنية xhci_pipe *defpipe = container_جو(
            usbdev->defpipe, بنية xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        صحيح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, اندريون);
        إذا (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            اذهب fail;
        }
    }
    حرر(اندريون);
    أرجع &pipe->pipe;

fail:
    حرر(pipe->buf);
    حرر(pipe);
    حرر(اندريون);
    أرجع عدم;
}

بنية usb_pipe *
xhci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (!CONFIG_USB_XHCI)
        أرجع عدم;
    إذا (!epdesc) {
        usb_جوڙيو_freelist(upipe);
        أرجع عدم;
    }
    إذا (!upipe)
        أرجع xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bm_خاصيتون & USB_ENDPOINT_XFERTYPE_نقاب;
    صحيح oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    بنية xhci_pipe *pipe = container_جو(upipe, بنية xhci_pipe, pipe);
    بنية usb_xhci_s *xhci = container_جو(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    إذا (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        أرجع upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    بنية xhci_inctx *اندريون = xhci_alloc_inctx(usbdev, 1);
    إذا (!اندريون)
        أرجع upipe;
    اندريون->جوڙيو = (1 << 1);
    بنية xhci_epctx *ep = (فراغ*)&اندريون[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    صحيح cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, اندريون);
    إذا (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    حرر(اندريون);

    أرجع upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ساكن فراغ xhci_xfer_تياري(بنية xhci_pipe *pipe, صحيح dir, فراغ *cmd
                            , فراغ *معلومات, صحيح datalen)
{
    بنية usb_xhci_s *xhci = container_جو(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    xhci_trb_قطار(&pipe->reqs, cmd, USB_CONTROL_تياري_سائيز
                   , (TR_تياري << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    إذا (datalen)
        xhci_trb_قطار(&pipe->reqs, معلومات, datalen, (TR_معلومات << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_قطار(&pipe->reqs, عدم, 0, (TR_حالت << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ساكن فراغ xhci_xfer_عام(بنية xhci_pipe *pipe,
                             فراغ *معلومات, صحيح datalen)
{
    بنية usb_xhci_s *xhci = container_جو(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    xhci_trb_قطار(&pipe->reqs, معلومات, datalen, (TR_عام << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

صحيح
xhci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *معلومات, صحيح datalen)
{
    إذا (!CONFIG_USB_XHCI)
        أرجع -1;
    بنية xhci_pipe *pipe = container_جو(p, بنية xhci_pipe, pipe);
    بنية usb_xhci_s *xhci = container_جو(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);

    إذا (cmd) {
        ثابت بنية usb_ctrlrequest *req = cmd;
        إذا (req->bRequest == USB_REQ_سيٽ_پتو)
            // Set address command sent during xhci_alloc_pipe.
            أرجع 0;
        xhci_xfer_تياري(pipe, dir, (فراغ*)req, معلومات, datalen);
    } وإلا {
        xhci_xfer_عام(pipe, معلومات, datalen);
    }

    صحيح cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_وقت(p, datalen));
    إذا (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        أرجع -1;
    }

    أرجع 0;
}

صحيح VISIBLE32FLAT
xhci_poll_intr(بنية usb_pipe *p, فراغ *معلومات)
{
    إذا (!CONFIG_USB_XHCI)
        أرجع -1;

    بنية xhci_pipe *pipe = container_جو(p, بنية xhci_pipe, pipe);
    بنية usb_xhci_s *xhci = container_جو(
        pipe->pipe.cntl, بنية usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    فراغ *buf = pipe->buf;
    صحيح bufused = pipe->bufused;

    إذا (!bufused) {
        xhci_xfer_عام(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        أرجع -1;
    }

    xhci_process_events(xhci);
    إذا (xhci_ring_busy(&pipe->reqs))
        أرجع -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.حالت,
            pipe->reqs.evt.control,
            معلومات, buf, len);
    يادگيري_نقل(معلومات, buf, len);
    xhci_xfer_عام(pipe, buf, len);
    أرجع 0;
}
