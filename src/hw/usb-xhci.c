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

#تعریف_کرو XHCI_RING_اشیاء          16
#تعریف_کرو XHCI_RING_سائز           (XHCI_RING_اشیاء*حجم(ساخت xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#تعریف_کرو XHCI_RING(_trb)          \
    ((ساخت xhci_ring*)((u32)(_trb) & ~(XHCI_RING_سائز-1)))

// --------------------------------------------------------------
// bit definitions

#تعریف_کرو XHCI_CMD_RS              (1<<0)
#تعریف_کرو XHCI_CMD_HCRST           (1<<1)
#تعریف_کرو XHCI_CMD_INTE            (1<<2)
#تعریف_کرو XHCI_CMD_HSEE            (1<<3)
#تعریف_کرو XHCI_CMD_LHCRST          (1<<7)
#تعریف_کرو XHCI_CMD_CSS             (1<<8)
#تعریف_کرو XHCI_CMD_CRS             (1<<9)
#تعریف_کرو XHCI_CMD_EWE             (1<<10)
#تعریف_کرو XHCI_CMD_EU3S            (1<<11)

#تعریف_کرو XHCI_STS_HCH             (1<<0)
#تعریف_کرو XHCI_STS_HSE             (1<<2)
#تعریف_کرو XHCI_STS_EINT            (1<<3)
#تعریف_کرو XHCI_STS_PCD             (1<<4)
#تعریف_کرو XHCI_STS_SSS             (1<<8)
#تعریف_کرو XHCI_STS_RSS             (1<<9)
#تعریف_کرو XHCI_STS_SRE             (1<<10)
#تعریف_کرو XHCI_STS_CNR             (1<<11)
#تعریف_کرو XHCI_STS_HCE             (1<<12)

#تعریف_کرو XHCI_PORTSC_CCS          (1<<0)
#تعریف_کرو XHCI_PORTSC_PED          (1<<1)
#تعریف_کرو XHCI_PORTSC_OCA          (1<<3)
#تعریف_کرو XHCI_PORTSC_PR           (1<<4)
#تعریف_کرو XHCI_PORTSC_PLS_سرکاؤ        5
#تعریف_کرو XHCI_PORTSC_PLS_نقاب     0xf
#تعریف_کرو XHCI_PORTSC_PP           (1<<9)
#تعریف_کرو XHCI_PORTSC_رفتار_سرکاؤ      10
#تعریف_کرو XHCI_PORTSC_رفتار_نقاب   0xf
#تعریف_کرو XHCI_PORTSC_رفتار_بھرا   (1<<10)
#تعریف_کرو XHCI_PORTSC_رفتار_کم    (2<<10)
#تعریف_کرو XHCI_PORTSC_رفتار_بلند   (3<<10)
#تعریف_کرو XHCI_PORTSC_رفتار_SUPER  (4<<10)
#تعریف_کرو XHCI_PORTSC_PIC_سرکاؤ        14
#تعریف_کرو XHCI_PORTSC_PIC_نقاب     0x3
#تعریف_کرو XHCI_PORTSC_LWS          (1<<16)
#تعریف_کرو XHCI_PORTSC_CSC          (1<<17)
#تعریف_کرو XHCI_PORTSC_PEC          (1<<18)
#تعریف_کرو XHCI_PORTSC_WRC          (1<<19)
#تعریف_کرو XHCI_PORTSC_OCC          (1<<20)
#تعریف_کرو XHCI_PORTSC_PRC          (1<<21)
#تعریف_کرو XHCI_PORTSC_PLC          (1<<22)
#تعریف_کرو XHCI_PORTSC_CEC          (1<<23)
#تعریف_کرو XHCI_PORTSC_CAS          (1<<24)
#تعریف_کرو XHCI_PORTSC_WCE          (1<<25)
#تعریف_کرو XHCI_PORTSC_WDE          (1<<26)
#تعریف_کرو XHCI_PORTSC_WOE          (1<<27)
#تعریف_کرو XHCI_PORTSC_DR           (1<<30)
#تعریف_کرو XHCI_PORTSC_WPR          (1<<31)

#تعریف_کرو TRB_C               (1<<0)
#تعریف_کرو TRB_TYPE_سرکاؤ          10
#تعریف_کرو TRB_TYPE_نقاب       0x3f
#تعریف_کرو TRB_TYPE(t)         (((t) >> TRB_TYPE_سرکاؤ) & TRB_TYPE_نقاب)

#تعریف_کرو TRB_EV_ED           (1<<2)

#تعریف_کرو TRB_TR_ENT          (1<<1)
#تعریف_کرو TRB_TR_ISP          (1<<2)
#تعریف_کرو TRB_TR_NS           (1<<3)
#تعریف_کرو TRB_TR_CH           (1<<4)
#تعریف_کرو TRB_TR_IOC          (1<<5)
#تعریف_کرو TRB_TR_IDT          (1<<6)
#تعریف_کرو TRB_TR_TBC_سرکاؤ        7
#تعریف_کرو TRB_TR_TBC_نقاب     0x3
#تعریف_کرو TRB_TR_BEI          (1<<9)
#تعریف_کرو TRB_TR_TLBPC_سرکاؤ      16
#تعریف_کرو TRB_TR_TLBPC_نقاب   0xf
#تعریف_کرو TRB_TR_FRAMEID_سرکاؤ    20
#تعریف_کرو TRB_TR_FRAMEID_نقاب 0x7ff
#تعریف_کرو TRB_TR_SIA          (1<<31)

#تعریف_کرو TRB_TR_DIR          (1<<16)

#تعریف_کرو TRB_CR_SLOTID_سرکاؤ     24
#تعریف_کرو TRB_CR_SLOTID_نقاب  0xff
#تعریف_کرو TRB_CR_EPID_سرکاؤ       16
#تعریف_کرو TRB_CR_EPID_نقاب    0x1f

#تعریف_کرو TRB_CR_BSR          (1<<9)
#تعریف_کرو TRB_CR_DC           (1<<9)

#تعریف_کرو TRB_LK_TC           (1<<1)

#تعریف_کرو TRB_INTR_سرکاؤ          22
#تعریف_کرو TRB_INTR_نقاب       0x3ff
#تعریف_کرو TRB_INTR(t)         (((t).حالت_2 >> TRB_INTR_سرکاؤ) & TRB_INTR_نقاب)

قسمتعریف شمار TRBType {
    TRB_RESERVED = 0,
    TR_معمول,
    TR_تیاری,
    TR_ڈیٹا,
    TR_حالت,
    TR_ISOCH,
    TR_LINK,
    TR_EVDATA,
    TR_NOOP,
    CR_ENABLE_خانہ,
    CR_DISABLE_خانہ,
    CR_پتہ_DEVICE,
    CR_CONFIGURE_ENDPOINT,
    CR_EVALUATE_CONTEXT,
    CR_RESET_ENDPOINT,
    CR_STOP_ENDPOINT,
    CR_سیٹ_TR_قطار_سے_نکالو,
    CR_RESET_DEVICE,
    CR_FORCE_EVENT,
    CR_NEGOTIATE_BW,
    CR_سیٹ_LATENCY_TOLERANCE,
    CR_GET_پورٹ_BANDWIDTH,
    CR_FORCE_سرخی,
    CR_NOOP,
    ER_TRANSFER = 32,
    ER_COMMAND_COMPLETE,
    ER_پورٹ_حالت_CHANGE,
    ER_BANDWIDTH_REQUEST,
    ER_DOORBELL,
    ER_HOST_CONTROLLER,
    ER_DEVICE_NOTIFICATION,
    ER_MFINDEX_WRAP,
} TRBType;

قسمتعریف شمار TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_ڈیٹا_بفر_ERROR,
    CC_BABBLE_DETECTED,
    CC_USB_TRANSACTION_ERROR,
    CC_TRB_ERROR,
    CC_STALL_ERROR,
    CC_RESOURCE_ERROR,
    CC_BANDWIDTH_ERROR,
    CC_NO_SLOTS_ERROR,
    CC_INVALID_دھارا_TYPE_ERROR,
    CC_خانہ_NOT_ENABLED_ERROR,
    CC_EP_NOT_ENABLED_ERROR,
    CC_SHORT_PACKET,
    CC_RING_UNDERRUN,
    CC_RING_OVERRUN,
    CC_VF_ER_بھرا,
    CC_PARAMETER_ERROR,
    CC_BANDWIDTH_OVERRUN,
    CC_CONTEXT_STATE_ERROR,
    CC_NO_PING_RESPONSE_ERROR,
    CC_EVENT_RING_بھرا_ERROR,
    CC_INCOMPATIBLE_DEVICE_ERROR,
    CC_MISSED_SERVICE_ERROR,
    CC_COMMAND_RING_STOPPED,
    CC_COMMAND_ABORTED,
    CC_STOPPED,
    CC_STOPPED_لمبائی_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_بفر_OVERRUN = 31,
    CC_EVENT_گمشدہ_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_دھارا_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_بانٹو_TRANSACTION_ERROR
} TRBCCode;

شمار {
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
    PLS_COMPILANCE_منوال = 10,
    PLS_TEST_منوال       = 11,
    PLS_RESUME          = 15,
};

#تعریف_کرو xhci_get_میدان(ڈیٹا, میدان)             \
    (((ڈیٹا) >> میدان##_SHIFT) & میدان##_MASK)

// --------------------------------------------------------------
// state structs

ساخت xhci_ring {
    ساخت xhci_trb      ring[XHCI_RING_اشیاء];
    ساخت xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    ساخت mutex_s       تالا_2;
};

ساخت xhci_portmap {
    u8 آغاز;
    u8 گنتی;
};

ساخت usb_xhci_s {
    ساخت usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    ساخت xhci_portmap  usb2;
    ساخت xhci_portmap  usb3;

    /* xhci registers */
    ساخت xhci_caps     *caps;
    ساخت xhci_op       *op;
    ساخت xhci_pr       *pr;
    ساخت xhci_ir       *ir;
    ساخت xhci_db       *db;

    /* xhci data structures */
    ساخت xhci_devlist  *devs;
    ساخت xhci_ring     *cmds;
    ساخت xhci_ring     *evts;
    ساخت xhci_er_seg   *eseg;
};

ساخت xhci_pipe {
    ساخت xhci_ring     reqs;

    ساخت usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    خالی                 *buf;
    صحیح                  bufused;
};

// --------------------------------------------------------------
// tables

ساکن ثابت حرف *رفتار_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ساکن ثابت صحیح رفتار_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ساکن ثابت صحیح رفتار_مقصد_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ساکن صحیح wait_ثنائی_ہندسہ(u32 *reg, u32 نقاب, صحیح قدر, u32 timeout)
{
    u32 انتہا = timer_calc(timeout);

    جب ((readl(reg) & نقاب) != قدر) {
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس -1;
        }
        yield();
    }
    واپس 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#تعریف_کرو XHCI_وقت_POSTPOWER 20

// Check if device attached to port
ساکن خالی
xhci_print_پورٹ_state(صحیح loglevel, ثابت حرف *سابقہ, u32 پورٹ, u32 portsc)
{
    u32 pls = xhci_get_میدان(portsc, XHCI_PORTSC_PLS);
    u32 رفتار = xhci_get_میدان(portsc, XHCI_PORTSC_رفتار);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            سابقہ, پورٹ + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, رفتار, رفتار_name[رفتار]);
}

ساکن صحیح
xhci_hub_detect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_xhci_s *xhci = container_کا(hub->cntl, ساخت usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[پورٹ].portsc);
    واپس (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ساکن صحیح
xhci_hub_reset(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_xhci_s *xhci = container_کا(hub->cntl, ساخت usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[پورٹ].portsc);
    اگر (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        واپس -1;

    انتخاب (xhci_get_میدان(portsc, XHCI_PORTSC_PLS)) {
    حالت PLS_U0:
        // A USB3 port - controller automatically performs reset
        توڑو;
    حالت PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_پورٹ_state(3, __func__, پورٹ, portsc);
        writel(&xhci->pr[پورٹ].portsc, portsc | XHCI_PORTSC_PR);
        توڑو;
    طےشدہ:
        واپس -1;
    }

    // Wait for device to complete reset and be enabled
    u32 انتہا = timer_calc(100);
    برائے (;;) {
        portsc = readl(&xhci->pr[پورٹ].portsc);
        اگر (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            واپس -1;
        اگر (portsc & XHCI_PORTSC_PED)
            // Reset complete
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس -1;
        }
        yield();
    }

    صحیح rc = رفتار_from_xhci[xhci_get_میدان(portsc, XHCI_PORTSC_رفتار)];
    xhci_print_پورٹ_state(1, "XHCI", پورٹ, portsc);
    واپس rc;
}

ساکن صحیح
xhci_hub_portmap(ساخت usbhub_s *hub, u32 vport)
{
    ساخت usb_xhci_s *xhci = container_کا(hub->cntl, ساخت usb_xhci_s, usb);
    u32 pport = vport + 1;

    اگر (vport + 1 >= xhci->usb3.آغاز &&
        vport + 1 < xhci->usb3.آغاز + xhci->usb3.گنتی)
        pport = vport + 2 - xhci->usb3.آغاز;

    اگر (vport + 1 >= xhci->usb2.آغاز &&
        vport + 1 < xhci->usb2.آغاز + xhci->usb2.گنتی)
        pport = vport + 2 - xhci->usb2.آغاز;

    واپس pport;
}

ساکن خالی
xhci_hub_disconnect(ساخت usbhub_s *hub, u32 پورٹ)
{
    // XXX - should turn the port power off.
}

ساکن ساخت usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساکن صحیح
xhci_check_ports(ساخت usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_وقت_POSTPOWER);

    ساخت usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    واپس hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

ساکن خالی
xhci_free_pipes(ساخت usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

ساکن خالی
configure_xhci(خالی *ڈیٹا)
{
    ساخت usb_xhci_s *xhci = ڈیٹا;
    u32 reg;

    xhci->devs = memalign_بلند(64, حجم(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_بلند(64, حجم(*xhci->eseg));
    xhci->cmds = memalign_بلند(XHCI_RING_سائز, حجم(*xhci->cmds));
    xhci->evts = memalign_بلند(XHCI_RING_سائز, حجم(*xhci->evts));
    اگر (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        جاؤ fail;
    }
    memset(xhci->devs, 0, حجم(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, حجم(*xhci->cmds));
    memset(xhci->evts, 0, حجم(*xhci->evts));
    memset(xhci->eseg, 0, حجم(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    اگر (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        اگر (wait_ثنائی_ہندسہ(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            جاؤ fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    اگر (wait_ثنائی_ہندسہ(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        جاؤ fail;
    اگر (wait_ثنائی_ہندسہ(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        جاؤ fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_کم, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_بلند, 0);
    writel(&xhci->op->crcr_کم, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_بلند, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_کم = (u32)xhci->evts;
    xhci->eseg->ptr_بلند = 0;
    xhci->eseg->سائز = XHCI_RING_اشیاء;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_کم, (u32)xhci->evts);
    writel(&xhci->ir->erdp_بلند, 0);
    writel(&xhci->ir->erstba_کم, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_بلند, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    اگر (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_بلند(64, حجم(*spba) * spb);
        خالی *pad = memalign_بلند(صفحہ_سائز, صفحہ_سائز * spb);
        اگر (!spba || !pad) {
            warn_noalloc();
            آزاد(spba);
            آزاد(pad);
            جاؤ fail;
        }
        صحیح i;
        برائے (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * صفحہ_سائز);
        xhci->devs[0].ptr_کم = (u32)spba;
        xhci->devs[0].ptr_بلند = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    صحیح گنتی = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    اگر (گنتی)
        // Success
        واپس;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_ثنائی_ہندسہ(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    آزاد(xhci->eseg);
    آزاد(xhci->evts);
    آزاد(xhci->cmds);
    آزاد(xhci->devs);
    آزاد(xhci);
}

ساکن ساخت usb_xhci_s*
xhci_controller_تیاری(خالی *baseaddr)
{
    ساخت usb_xhci_s *xhci = malloc_بلند(حجم(*xhci));
    اگر (!xhci) {
        warn_noalloc();
        واپس تہی;
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

    اگر (xhci->xcap) {
        u32 off;
        خالی *addr = baseaddr + xhci->xcap;
        کرو {
            ساخت xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            انتخاب (cap & 0xff) {
            حالت 0x02:
                name  = readl(&xcap->ڈیٹا[0]);
                ports = readl(&xcap->ڈیٹا[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 گنتی = (ports >> 8) & 0xff;
                u8 آغاز = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , گنتی, آغاز
                        , ports >> 16);
                اگر (name == 0x20425355 /* "USB " */) {
                    اگر (major == 2) {
                        xhci->usb2.آغاز = آغاز;
                        xhci->usb2.گنتی = گنتی;
                    }
                    اگر (major == 3) {
                        xhci->usb3.آغاز = آغاز;
                        xhci->usb3.گنتی = گنتی;
                    }
                }
                توڑو;
            طےشدہ:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                توڑو;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } جب (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    اگر (صفحہ_سائز != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        آزاد(xhci);
        واپس تہی;
    }

    واپس xhci;
}

ساکن خالی
xhci_controller_تیاری_pci(ساخت pci_device *pci)
{
    ساخت usb_xhci_s *xhci;
    خالی *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_بنیاد_پتہ_0);
    اگر (!baseaddr)
        واپس;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_تیاری(baseaddr);
    اگر (!xhci)
        واپس;

    xhci->usb.pci = pci;
    تسلسل_thread(configure_xhci, xhci);
}

ساکن خالی
xhci_controller_تیاری_acpi(ساخت acpi_device *dev)
{
    ساخت usb_xhci_s *xhci;
    u64 mem, غیر_مستعمل;
    خالی *baseaddr;

    اگر (acpi_dsdt_ڈھونڈو_mem(dev, &mem, &غیر_مستعمل) < 0)
        واپس;
    اگر (mem >= 0x100000000ll)
        واپس;

    baseaddr = (خالی*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_تیاری(baseaddr);
    اگر (!xhci)
        واپس;

    xhci->usb.mmio = baseaddr;
    تسلسل_thread(configure_xhci, xhci);
}

خالی
xhci_تیاری(خالی)
{
    اگر (! CONFIG_USB_XHCI)
        واپس;

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_تیاری_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    ساخت acpi_device *dev;
    برائے (dev = acpi_dsdt_ڈھونڈو_eisaid(تہی, xhci_eisaid);
         dev != تہی;
         dev = acpi_dsdt_ڈھونڈو_eisaid(dev, xhci_eisaid)) {
        xhci_controller_تیاری_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ساکن خالی xhci_doorbell(ساخت usb_xhci_s *xhci, u32 slotid, u32 قدر)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, قدر);
    ساخت xhci_db *db = xhci->db;
    خالی *addr = &db[slotid].doorbell;
    writel(addr, قدر);
}

// Dequeue events on the XHCI command ring generated by the hardware
ساکن خالی xhci_process_events(ساخت usb_xhci_s *xhci)
{
    ساخت xhci_ring *evts = xhci->evts;

    برائے (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        ساخت xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        اگر ((control & TRB_C) != (cs ? 1 : 0))
            واپس;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->حالت_2 >> 24) & 0xff;
        انتخاب (evt_type) {
        حالت ER_TRANSFER:
        حالت ER_COMMAND_COMPLETE:
        {
            ساخت xhci_trb  *rtrb = (خالی*)etrb->ptr_کم;
            ساخت xhci_ring *ring = XHCI_RING(rtrb);
            ساخت xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            حافظہ_نقل(evt, etrb, حجم(*etrb));
            ring->eidx = eidx;
            توڑو;
        }
        حالت ER_پورٹ_حالت_CHANGE:
        {
            u32 پورٹ = ((etrb->ptr_کم >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[پورٹ].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_نقاب<<XHCI_PORTSC_PLS_سرکاؤ))
                          | (1<<XHCI_PORTSC_PLS_سرکاؤ));
            writel(&xhci->pr[پورٹ].portsc, pclear);

            xhci_print_پورٹ_state(3, __func__, پورٹ, portsc);
            توڑو;
        }
        طےشدہ:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            توڑو;
        }

        /* move ring index, notify xhci */
        nidx++;
        اگر (nidx == XHCI_RING_اشیاء) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        ساخت xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_کم, erdp);
        writel(&ir->erdp_بلند, 0);
    }
}

// Check if a ring has any pending TRBs
ساکن صحیح xhci_ring_busy(ساخت xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    واپس (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ساکن صحیح xhci_event_wait(ساخت usb_xhci_s *xhci,
                           ساخت xhci_ring *ring,
                           u32 timeout)
{
    u32 انتہا = timer_calc(timeout);

    برائے (;;) {
        xhci_process_events(xhci);
        اگر (!xhci_ring_busy(ring)) {
            u32 حالت_2 = ring->evt.حالت_2;
            واپس (حالت_2 >> 24) & 0xff;
        }
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ساکن خالی xhci_trb_fill(ساخت xhci_ring *ring
                          , خالی *ڈیٹا, u32 xferlen, u32 flags)
{
    ساخت xhci_trb *dst = &ring->ring[ring->nidx];
    اگر (flags & TRB_TR_IDT) {
        حافظہ_نقل(&dst->ptr_کم, ڈیٹا, xferlen);
    } ورنہ {
        dst->ptr_کم = (u32)ڈیٹا;
        dst->ptr_بلند = 0;
    }
    dst->حالت_2 = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ساکن خالی xhci_trb_قطار(ساخت xhci_ring *ring,
                           خالی *ڈیٹا, u32 xferlen, u32 flags)
{
    اگر (ring->nidx >= ARRAY_سائز(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, ڈیٹا, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
ساکن صحیح xhci_cmd_submit(ساخت usb_xhci_s *xhci, ساخت xhci_inctx *inctx
                           , u32 flags)
{
    اگر (inctx) {
        ساخت xhci_slotctx *خانہ = (خالی*)&inctx[1 << xhci->context64];
        u32 پورٹ = ((خانہ->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[پورٹ].portsc);
        اگر (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_پورٹ_state(1, __func__, پورٹ, portsc);
            واپس -1;
        }
    }

    mutex_تالا(&xhci->cmds->تالا_2);
    xhci_trb_قطار(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    صحیح rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->تالا_2);
    واپس rc;
}

ساکن صحیح xhci_cmd_enable_خانہ(ساخت usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    صحیح cc = xhci_cmd_submit(xhci, تہی, CR_ENABLE_خانہ << 10);
    اگر (cc != CC_SUCCESS)
        واپس -1;
    واپس (xhci->cmds->evt.control >> 24) & 0xff;
}

ساکن صحیح xhci_cmd_disable_خانہ(ساخت usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    واپس xhci_cmd_submit(xhci, تہی, (CR_DISABLE_خانہ << 10) | (slotid << 24));
}

ساکن صحیح xhci_cmd_پتہ_device(ساخت usb_xhci_s *xhci, u32 slotid
                                   , ساخت xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    واپس xhci_cmd_submit(xhci, inctx
                           , (CR_پتہ_DEVICE << 10) | (slotid << 24));
}

ساکن صحیح xhci_cmd_configure_endpoint(ساخت usb_xhci_s *xhci, u32 slotid
                                       , ساخت xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->جوڑو, inctx->del);
    واپس xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ساکن صحیح xhci_cmd_evaluate_context(ساخت usb_xhci_s *xhci, u32 slotid
                                     , ساخت xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->جوڑو, inctx->del);
    واپس xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ساکن ساخت xhci_inctx *
xhci_alloc_inctx(ساخت usbdevice_s *usbdev, صحیح maxepid)
{
    ساخت usb_xhci_s *xhci = container_کا(
        usbdev->hub->cntl, ساخت usb_xhci_s, usb);
    صحیح سائز = (حجم(ساخت xhci_inctx) * 33) << xhci->context64;
    ساخت xhci_inctx *اندر = memalign_tmphigh(2048 << xhci->context64, سائز);
    اگر (!اندر) {
        warn_noalloc();
        واپس تہی;
    }
    memset(اندر, 0, سائز);

    ساخت xhci_slotctx *خانہ = (خالی*)&اندر[1 << xhci->context64];
    خانہ->ctx[0]    |= maxepid << 27; // context entries
    خانہ->ctx[0]    |= رفتار_مقصد_xhci[usbdev->رفتار] << 20;

    // Set high-speed hub flags.
    ساخت usbdevice_s *hubdev = usbdev->hub->usbdev;
    اگر (hubdev) {
        اگر (usbdev->رفتار == USB_LOWSPEED || usbdev->رفتار == USB_FULLSPEED) {
            ساخت xhci_pipe *hpipe = container_کا(
                hubdev->defpipe, ساخت xhci_pipe, pipe);
            اگر (hubdev->رفتار == USB_HIGHSPEED) {
                خانہ->ctx[2] |= hpipe->slotid;
                خانہ->ctx[2] |= (usbdev->پورٹ+1) << 8;
            } ورنہ {
                ساخت xhci_slotctx *hslot = (خالی*)xhci->devs[hpipe->slotid].ptr_کم;
                خانہ->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        جب (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->پورٹ+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        خانہ->ctx[0]    |= route;
    }

    خانہ->ctx[1]    |= (usbdev->پورٹ+1) << 16;

    واپس اندر;
}

ساکن صحیح xhci_config_hub(ساخت usbhub_s *hub)
{
    ساخت usb_xhci_s *xhci = container_کا(
        hub->cntl, ساخت usb_xhci_s, usb);
    ساخت xhci_pipe *pipe = container_کا(
        hub->usbdev->defpipe, ساخت xhci_pipe, pipe);
    ساخت xhci_slotctx *hdslot = (خالی*)xhci->devs[pipe->slotid].ptr_کم;
    اگر ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        واپس 0;
    ساخت xhci_inctx *اندر = xhci_alloc_inctx(hub->usbdev, 1);
    اگر (!اندر)
        واپس -1;
    اندر->جوڑو = 0x01;
    ساخت xhci_slotctx *خانہ = (خالی*)&اندر[1 << xhci->context64];
    خانہ->ctx[0] |= 1 << 26;
    خانہ->ctx[1] |= hub->portcount << 24;

    صحیح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, اندر);
    آزاد(اندر);
    اگر (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        واپس -1;
    }
    واپس 0;
}

ساکن ساخت usb_pipe *
xhci_alloc_pipe(ساخت usbdevice_s *usbdev
                , ساخت usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب;
    ساخت usb_xhci_s *xhci = container_کا(
        usbdev->hub->cntl, ساخت usb_xhci_s, usb);
    ساخت xhci_pipe *pipe;
    u32 epid;

    اگر (epdesc->b_Endpoint_پتہ == 0) {
        epid = 1;
    } ورنہ {
        epid = (epdesc->b_Endpoint_پتہ & 0x0f) * 2;
        epid += (epdesc->b_Endpoint_پتہ & USB_DIR_اندر) ? 1 : 0;
    }

    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_بلند(XHCI_RING_سائز, حجم(*pipe));
    ورنہ
        pipe = memalign_کم(XHCI_RING_سائز, حجم(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        واپس تہی;
    }
    memset(pipe, 0, حجم(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    اگر (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_بلند(pipe->pipe.maxpacket);
        اگر (!pipe->buf) {
            warn_noalloc();
            آزاد(pipe);
            واپس تہی;
        }
    }

    // Allocate input context and initialize endpoint info.
    ساخت xhci_inctx *اندر = xhci_alloc_inctx(usbdev, epid);
    اگر (!اندر)
        جاؤ fail;
    اندر->جوڑو = 0x01 | (1 << epid);
    ساخت xhci_epctx *ep = (خالی*)&اندر[(pipe->epid+1) << xhci->context64];
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    اگر (epdesc->b_Endpoint_پتہ & USB_DIR_اندر
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_کم  = (u32)&pipe->reqs.ring[0];
    ep->deq_کم  |= 1;         // dcs
    ep->لمبائی   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    اگر (pipe->epid == 1) {
        اگر (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            صحیح ret = xhci_config_hub(usbdev->hub);
            اگر (ret)
                جاؤ fail;
        }
        // Enable slot.
        u32 سائز = (حجم(ساخت xhci_slotctx) * 32) << xhci->context64;
        ساخت xhci_slotctx *dev = memalign_بلند(1024 << xhci->context64, سائز);
        اگر (!dev) {
            warn_noalloc();
            جاؤ fail;
        }
        صحیح slotid = xhci_cmd_enable_خانہ(xhci);
        اگر (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            آزاد(dev);
            جاؤ fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, سائز);
        xhci->devs[slotid].ptr_کم = (u32)dev;
        xhci->devs[slotid].ptr_بلند = 0;

        // Send set_address command.
        صحیح cc = xhci_cmd_پتہ_device(xhci, slotid, اندر);
        اگر (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_خانہ(xhci, slotid);
            اگر (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                جاؤ fail;
            }
            xhci->devs[slotid].ptr_کم = 0;
            آزاد(dev);
            جاؤ fail;
        }
        pipe->slotid = slotid;
    } ورنہ {
        ساخت xhci_pipe *defpipe = container_کا(
            usbdev->defpipe, ساخت xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        صحیح cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, اندر);
        اگر (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            جاؤ fail;
        }
    }
    آزاد(اندر);
    واپس &pipe->pipe;

fail:
    آزاد(pipe->buf);
    آزاد(pipe);
    آزاد(اندر);
    واپس تہی;
}

ساخت usb_pipe *
xhci_realloc_pipe(ساخت usbdevice_s *usbdev, ساخت usb_pipe *upipe
                  , ساخت usb_endpoint_descriptor *epdesc)
{
    اگر (!CONFIG_USB_XHCI)
        واپس تہی;
    اگر (!epdesc) {
        usb_جوڑو_freelist(upipe);
        واپس تہی;
    }
    اگر (!upipe)
        واپس xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب;
    صحیح oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    ساخت xhci_pipe *pipe = container_کا(upipe, ساخت xhci_pipe, pipe);
    ساخت usb_xhci_s *xhci = container_کا(
        pipe->pipe.cntl, ساخت usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    اگر (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        واپس upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    ساخت xhci_inctx *اندر = xhci_alloc_inctx(usbdev, 1);
    اگر (!اندر)
        واپس upipe;
    اندر->جوڑو = (1 << 1);
    ساخت xhci_epctx *ep = (خالی*)&اندر[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    صحیح cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, اندر);
    اگر (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    آزاد(اندر);

    واپس upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ساکن خالی xhci_xfer_تیاری(ساخت xhci_pipe *pipe, صحیح dir, خالی *cmd
                            , خالی *ڈیٹا, صحیح datalen)
{
    ساخت usb_xhci_s *xhci = container_کا(
        pipe->pipe.cntl, ساخت usb_xhci_s, usb);
    xhci_trb_قطار(&pipe->reqs, cmd, USB_CONTROL_تیاری_سائز
                   , (TR_تیاری << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    اگر (datalen)
        xhci_trb_قطار(&pipe->reqs, ڈیٹا, datalen, (TR_ڈیٹا << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_قطار(&pipe->reqs, تہی, 0, (TR_حالت << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ساکن خالی xhci_xfer_معمول(ساخت xhci_pipe *pipe,
                             خالی *ڈیٹا, صحیح datalen)
{
    ساخت usb_xhci_s *xhci = container_کا(
        pipe->pipe.cntl, ساخت usb_xhci_s, usb);
    xhci_trb_قطار(&pipe->reqs, ڈیٹا, datalen, (TR_معمول << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

صحیح
xhci_send_pipe(ساخت usb_pipe *p, صحیح dir, ثابت خالی *cmd
               , خالی *ڈیٹا, صحیح datalen)
{
    اگر (!CONFIG_USB_XHCI)
        واپس -1;
    ساخت xhci_pipe *pipe = container_کا(p, ساخت xhci_pipe, pipe);
    ساخت usb_xhci_s *xhci = container_کا(
        pipe->pipe.cntl, ساخت usb_xhci_s, usb);

    اگر (cmd) {
        ثابت ساخت usb_ctrlrequest *req = cmd;
        اگر (req->bRequest == USB_REQ_سیٹ_پتہ)
            // Set address command sent during xhci_alloc_pipe.
            واپس 0;
        xhci_xfer_تیاری(pipe, dir, (خالی*)req, ڈیٹا, datalen);
    } ورنہ {
        xhci_xfer_معمول(pipe, ڈیٹا, datalen);
    }

    صحیح cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_وقت(p, datalen));
    اگر (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        واپس -1;
    }

    واپس 0;
}

صحیح VISIBLE32FLAT
xhci_poll_intr(ساخت usb_pipe *p, خالی *ڈیٹا)
{
    اگر (!CONFIG_USB_XHCI)
        واپس -1;

    ساخت xhci_pipe *pipe = container_کا(p, ساخت xhci_pipe, pipe);
    ساخت usb_xhci_s *xhci = container_کا(
        pipe->pipe.cntl, ساخت usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    خالی *buf = pipe->buf;
    صحیح bufused = pipe->bufused;

    اگر (!bufused) {
        xhci_xfer_معمول(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        واپس -1;
    }

    xhci_process_events(xhci);
    اگر (xhci_ring_busy(&pipe->reqs))
        واپس -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.حالت_2,
            pipe->reqs.evt.control,
            ڈیٹا, buf, len);
    حافظہ_نقل(ڈیٹا, buf, len);
    xhci_xfer_معمول(pipe, buf, len);
    واپس 0;
}
