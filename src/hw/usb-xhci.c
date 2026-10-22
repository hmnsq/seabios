// Code for handling XHCI "Super speed" USB controllers.
//
// Copyright (C) 2013  Gerd Hoffmann <kraxel@redhat.com>
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // memalign_low
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SIZE
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_XHCI
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-xhci.h" // struct ehci_qh
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // readl

// --------------------------------------------------------------
// configuration

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_RING_ਚੀਜ਼ਾਂ          16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_RING_ਮਾਪ           (XHCI_RING_ਚੀਜ਼ਾਂ*ਆਕਾਰ(ਬਣਤਰ xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_RING(_trb)          \
    ((ਬਣਤਰ xhci_ring*)((u32)(_trb) & ~(XHCI_RING_ਮਾਪ-1)))

// --------------------------------------------------------------
// bit definitions

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_RS              (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_HCRST           (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_INTE            (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_HSEE            (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_LHCRST          (1<<7)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_CSS             (1<<8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_CRS             (1<<9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_EWE             (1<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_CMD_EU3S            (1<<11)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_HCH             (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_HSE             (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_EINT            (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_PCD             (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_SSS             (1<<8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_RSS             (1<<9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_SRE             (1<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_CNR             (1<<11)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_STS_HCE             (1<<12)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_CCS          (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PED          (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_OCA          (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PR           (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PLS_ਖਿਸਕਾਓ        5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PLS_ਨਕਾਬ     0xf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PP           (1<<9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_ਰਫਤਾਰ_ਖਿਸਕਾਓ      10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_ਰਫਤਾਰ_ਨਕਾਬ   0xf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_ਰਫਤਾਰ_ਭਰਿਆ   (1<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_ਰਫਤਾਰ_ਨੀਵਾਂ    (2<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_ਰਫਤਾਰ_ਉੱਚਾ   (3<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_ਰਫਤਾਰ_SUPER  (4<<10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PIC_ਖਿਸਕਾਓ        14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PIC_ਨਕਾਬ     0x3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_LWS          (1<<16)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_CSC          (1<<17)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PEC          (1<<18)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_WRC          (1<<19)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_OCC          (1<<20)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PRC          (1<<21)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_PLC          (1<<22)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_CEC          (1<<23)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_CAS          (1<<24)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_WCE          (1<<25)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_WDE          (1<<26)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_WOE          (1<<27)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_DR           (1<<30)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_PORTSC_WPR          (1<<31)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_C               (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TYPE_ਖਿਸਕਾਓ          10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TYPE_ਨਕਾਬ       0x3f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TYPE(t)         (((t) >> TRB_TYPE_ਖਿਸਕਾਓ) & TRB_TYPE_ਨਕਾਬ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_EV_ED           (1<<2)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_ENT          (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_ISP          (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_NS           (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_CH           (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_IOC          (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_IDT          (1<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_TBC_ਖਿਸਕਾਓ        7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_TBC_ਨਕਾਬ     0x3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_BEI          (1<<9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_TLBPC_ਖਿਸਕਾਓ      16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_TLBPC_ਨਕਾਬ   0xf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_FRAMEID_ਖਿਸਕਾਓ    20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_FRAMEID_ਨਕਾਬ 0x7ff
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_SIA          (1<<31)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_TR_DIR          (1<<16)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_CR_SLOTID_ਖਿਸਕਾਓ     24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_CR_SLOTID_ਨਕਾਬ  0xff
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_CR_EPID_ਖਿਸਕਾਓ       16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_CR_EPID_ਨਕਾਬ    0x1f

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_CR_BSR          (1<<9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_CR_DC           (1<<9)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_LK_TC           (1<<1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_INTR_ਖਿਸਕਾਓ          22
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_INTR_ਨਕਾਬ       0x3ff
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRB_INTR(t)         (((t).ਸਥਿਤੀ >> TRB_INTR_ਖਿਸਕਾਓ) & TRB_INTR_ਨਕਾਬ)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ TRBType {
    TRB_RESERVED = 0,
    TR_ਸਾਧਾਰਨ,
    TR_ਤਿਆਰੀ,
    TR_ਅੰਕੜੇ,
    TR_ਸਥਿਤੀ,
    TR_ISOCH,
    TR_LINK,
    TR_EVDATA,
    TR_NOOP,
    CR_ENABLE_ਥਾਂ,
    CR_DISABLE_ਥਾਂ,
    CR_ਪਤਾ_DEVICE,
    CR_CONFIGURE_ENDPOINT,
    CR_EVALUATE_CONTEXT,
    CR_RESET_ENDPOINT,
    CR_STOP_ENDPOINT,
    CR_ਸੈੱਟ_TR_ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ,
    CR_RESET_DEVICE,
    CR_FORCE_EVENT,
    CR_NEGOTIATE_BW,
    CR_ਸੈੱਟ_LATENCY_TOLERANCE,
    CR_GET_ਪੋਰਟ_BANDWIDTH,
    CR_FORCE_ਸਿਰਲੇਖ,
    CR_NOOP,
    ER_TRANSFER = 32,
    ER_COMMAND_COMPLETE,
    ER_ਪੋਰਟ_ਸਥਿਤੀ_CHANGE,
    ER_BANDWIDTH_REQUEST,
    ER_DOORBELL,
    ER_HOST_CONTROLLER,
    ER_DEVICE_NOTIFICATION,
    ER_MFINDEX_WRAP,
} TRBType;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_ਅੰਕੜੇ_ਬਫਰ_ERROR,
    CC_BABBLE_DETECTED,
    CC_USB_TRANSACTION_ERROR,
    CC_TRB_ERROR,
    CC_STALL_ERROR,
    CC_RESOURCE_ERROR,
    CC_BANDWIDTH_ERROR,
    CC_NO_SLOTS_ERROR,
    CC_INVALID_ਧਾਰਾ_TYPE_ERROR,
    CC_ਥਾਂ_NOT_ENABLED_ERROR,
    CC_EP_NOT_ENABLED_ERROR,
    CC_SHORT_PACKET,
    CC_RING_UNDERRUN,
    CC_RING_OVERRUN,
    CC_VF_ER_ਭਰਿਆ,
    CC_PARAMETER_ERROR,
    CC_BANDWIDTH_OVERRUN,
    CC_CONTEXT_STATE_ERROR,
    CC_NO_PING_RESPONSE_ERROR,
    CC_EVENT_RING_ਭਰਿਆ_ERROR,
    CC_INCOMPATIBLE_DEVICE_ERROR,
    CC_MISSED_SERVICE_ERROR,
    CC_COMMAND_RING_STOPPED,
    CC_COMMAND_ABORTED,
    CC_STOPPED,
    CC_STOPPED_ਲੰਮਾਈ_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_ਬਫਰ_OVERRUN = 31,
    CC_EVENT_ਗੁਆਚਾ_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_ਧਾਰਾ_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_ਵੰਡੋ_TRANSACTION_ERROR
} TRBCCode;

ਗਿਣਤੀ {
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
    PLS_COMPILANCE_ਬਹੁਲਕ = 10,
    PLS_TEST_ਬਹੁਲਕ       = 11,
    PLS_RESUME          = 15,
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xhci_get_ਖੇਤਰ(ਅੰਕੜੇ, ਖੇਤਰ)             \
    (((ਅੰਕੜੇ) >> ਖੇਤਰ##_SHIFT) & ਖੇਤਰ##_MASK)

// --------------------------------------------------------------
// state structs

ਬਣਤਰ xhci_ring {
    ਬਣਤਰ xhci_trb      ring[XHCI_RING_ਚੀਜ਼ਾਂ];
    ਬਣਤਰ xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    ਬਣਤਰ mutex_s       ਤਾਲਾ;
};

ਬਣਤਰ xhci_portmap {
    u8 ਸ਼ੁਰੂ;
    u8 ਗਿਣਤੀ_2;
};

ਬਣਤਰ usb_xhci_s {
    ਬਣਤਰ usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    ਬਣਤਰ xhci_portmap  usb2;
    ਬਣਤਰ xhci_portmap  usb3;

    /* xhci registers */
    ਬਣਤਰ xhci_caps     *caps;
    ਬਣਤਰ xhci_op       *op;
    ਬਣਤਰ xhci_pr       *pr;
    ਬਣਤਰ xhci_ir       *ir;
    ਬਣਤਰ xhci_db       *db;

    /* xhci data structures */
    ਬਣਤਰ xhci_devlist  *devs;
    ਬਣਤਰ xhci_ring     *cmds;
    ਬਣਤਰ xhci_ring     *evts;
    ਬਣਤਰ xhci_er_seg   *eseg;
};

ਬਣਤਰ xhci_pipe {
    ਬਣਤਰ xhci_ring     reqs;

    ਬਣਤਰ usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    ਖਾਲੀ                 *buf;
    ਪੂਰਨ                  bufused;
};

// --------------------------------------------------------------
// tables

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਰਫਤਾਰ_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਰਫਤਾਰ_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਰਫਤਾਰ_ਮੰਜ਼ਿਲ_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ਸਥਿਰ ਪੂਰਨ wait_ਦੋਆਧਾਰੀ_ਅੰਕ(u32 *reg, u32 ਨਕਾਬ, ਪੂਰਨ ਮੁੱਲ, u32 timeout)
{
    u32 ਸਿਰਾ = timer_calc(timeout);

    ਜਦੋਂ ((readl(reg) & ਨਕਾਬ) != ਮੁੱਲ) {
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }
    ਵਾਪਸ 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XHCI_ਸਮਾਂ_POSTPOWER 20

// Check if device attached to port
ਸਥਿਰ ਖਾਲੀ
xhci_print_ਪੋਰਟ_state(ਪੂਰਨ loglevel, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, u32 ਪੋਰਟ, u32 portsc)
{
    u32 pls = xhci_get_ਖੇਤਰ(portsc, XHCI_PORTSC_PLS);
    u32 ਰਫਤਾਰ = xhci_get_ਖੇਤਰ(portsc, XHCI_PORTSC_ਰਫਤਾਰ);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            ਅਗੇਤਰ, ਪੋਰਟ + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, ਰਫਤਾਰ, ਰਫਤਾਰ_name[ਰਫਤਾਰ]);
}

ਸਥਿਰ ਪੂਰਨ
xhci_hub_detect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[ਪੋਰਟ].portsc);
    ਵਾਪਸ (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ਸਥਿਰ ਪੂਰਨ
xhci_hub_reset(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[ਪੋਰਟ].portsc);
    ਜੇ (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        ਵਾਪਸ -1;

    ਚੋਣ (xhci_get_ਖੇਤਰ(portsc, XHCI_PORTSC_PLS)) {
    ਹਾਲਤ PLS_U0:
        // A USB3 port - controller automatically performs reset
        ਰੋਕੋ;
    ਹਾਲਤ PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_ਪੋਰਟ_state(3, __func__, ਪੋਰਟ, portsc);
        writel(&xhci->pr[ਪੋਰਟ].portsc, portsc | XHCI_PORTSC_PR);
        ਰੋਕੋ;
    ਮੂਲ:
        ਵਾਪਸ -1;
    }

    // Wait for device to complete reset and be enabled
    u32 ਸਿਰਾ = timer_calc(100);
    ਲਈ (;;) {
        portsc = readl(&xhci->pr[ਪੋਰਟ].portsc);
        ਜੇ (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            ਵਾਪਸ -1;
        ਜੇ (portsc & XHCI_PORTSC_PED)
            // Reset complete
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }

    ਪੂਰਨ rc = ਰਫਤਾਰ_from_xhci[xhci_get_ਖੇਤਰ(portsc, XHCI_PORTSC_ਰਫਤਾਰ)];
    xhci_print_ਪੋਰਟ_state(1, "XHCI", ਪੋਰਟ, portsc);
    ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ
xhci_hub_portmap(ਬਣਤਰ usbhub_s *hub, u32 vport)
{
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_xhci_s, usb);
    u32 pport = vport + 1;

    ਜੇ (vport + 1 >= xhci->usb3.ਸ਼ੁਰੂ &&
        vport + 1 < xhci->usb3.ਸ਼ੁਰੂ + xhci->usb3.ਗਿਣਤੀ_2)
        pport = vport + 2 - xhci->usb3.ਸ਼ੁਰੂ;

    ਜੇ (vport + 1 >= xhci->usb2.ਸ਼ੁਰੂ &&
        vport + 1 < xhci->usb2.ਸ਼ੁਰੂ + xhci->usb2.ਗਿਣਤੀ_2)
        pport = vport + 2 - xhci->usb2.ਸ਼ੁਰੂ;

    ਵਾਪਸ pport;
}

ਸਥਿਰ ਖਾਲੀ
xhci_hub_disconnect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    // XXX - should turn the port power off.
}

ਸਥਿਰ ਬਣਤਰ usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ਸਥਿਰ ਪੂਰਨ
xhci_check_ports(ਬਣਤਰ usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_ਸਮਾਂ_POSTPOWER);

    ਬਣਤਰ usbhub_s hub;
    memset(&hub, 0, ਆਕਾਰ(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    ਵਾਪਸ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ
xhci_free_pipes(ਬਣਤਰ usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

ਸਥਿਰ ਖਾਲੀ
configure_xhci(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ usb_xhci_s *xhci = ਅੰਕੜੇ;
    u32 reg;

    xhci->devs = memalign_ਉੱਚਾ(64, ਆਕਾਰ(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ਉੱਚਾ(64, ਆਕਾਰ(*xhci->eseg));
    xhci->cmds = memalign_ਉੱਚਾ(XHCI_RING_ਮਾਪ, ਆਕਾਰ(*xhci->cmds));
    xhci->evts = memalign_ਉੱਚਾ(XHCI_RING_ਮਾਪ, ਆਕਾਰ(*xhci->evts));
    ਜੇ (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        ਜਾਓ fail;
    }
    memset(xhci->devs, 0, ਆਕਾਰ(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, ਆਕਾਰ(*xhci->cmds));
    memset(xhci->evts, 0, ਆਕਾਰ(*xhci->evts));
    memset(xhci->eseg, 0, ਆਕਾਰ(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    ਜੇ (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        ਜੇ (wait_ਦੋਆਧਾਰੀ_ਅੰਕ(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            ਜਾਓ fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    ਜੇ (wait_ਦੋਆਧਾਰੀ_ਅੰਕ(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        ਜਾਓ fail;
    ਜੇ (wait_ਦੋਆਧਾਰੀ_ਅੰਕ(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        ਜਾਓ fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_ਨੀਵਾਂ, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ਉੱਚਾ, 0);
    writel(&xhci->op->crcr_ਨੀਵਾਂ, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ਉੱਚਾ, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_ਨੀਵਾਂ = (u32)xhci->evts;
    xhci->eseg->ptr_ਉੱਚਾ = 0;
    xhci->eseg->ਮਾਪ = XHCI_RING_ਚੀਜ਼ਾਂ;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_ਨੀਵਾਂ, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ਉੱਚਾ, 0);
    writel(&xhci->ir->erstba_ਨੀਵਾਂ, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ਉੱਚਾ, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    ਜੇ (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ਉੱਚਾ(64, ਆਕਾਰ(*spba) * spb);
        ਖਾਲੀ *pad = memalign_ਉੱਚਾ(ਪੰਨਾ_ਮਾਪ, ਪੰਨਾ_ਮਾਪ * spb);
        ਜੇ (!spba || !pad) {
            warn_noalloc();
            ਮੁਕਤ(spba);
            ਮੁਕਤ(pad);
            ਜਾਓ fail;
        }
        ਪੂਰਨ i;
        ਲਈ (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * ਪੰਨਾ_ਮਾਪ);
        xhci->devs[0].ptr_ਨੀਵਾਂ = (u32)spba;
        xhci->devs[0].ptr_ਉੱਚਾ = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    ਪੂਰਨ ਗਿਣਤੀ_2 = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    ਜੇ (ਗਿਣਤੀ_2)
        // Success
        ਵਾਪਸ;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_ਦੋਆਧਾਰੀ_ਅੰਕ(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    ਮੁਕਤ(xhci->eseg);
    ਮੁਕਤ(xhci->evts);
    ਮੁਕਤ(xhci->cmds);
    ਮੁਕਤ(xhci->devs);
    ਮੁਕਤ(xhci);
}

ਸਥਿਰ ਬਣਤਰ usb_xhci_s*
xhci_controller_ਤਿਆਰੀ(ਖਾਲੀ *baseaddr)
{
    ਬਣਤਰ usb_xhci_s *xhci = malloc_ਉੱਚਾ(ਆਕਾਰ(*xhci));
    ਜੇ (!xhci) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(xhci, 0, ਆਕਾਰ(*xhci));
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

    ਜੇ (xhci->xcap) {
        u32 off;
        ਖਾਲੀ *addr = baseaddr + xhci->xcap;
        ਕਰੋ {
            ਬਣਤਰ xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            ਚੋਣ (cap & 0xff) {
            ਹਾਲਤ 0x02:
                name  = readl(&xcap->ਅੰਕੜੇ[0]);
                ports = readl(&xcap->ਅੰਕੜੇ[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 ਗਿਣਤੀ_2 = (ports >> 8) & 0xff;
                u8 ਸ਼ੁਰੂ = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , ਗਿਣਤੀ_2, ਸ਼ੁਰੂ
                        , ports >> 16);
                ਜੇ (name == 0x20425355 /* "USB " */) {
                    ਜੇ (major == 2) {
                        xhci->usb2.ਸ਼ੁਰੂ = ਸ਼ੁਰੂ;
                        xhci->usb2.ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2;
                    }
                    ਜੇ (major == 3) {
                        xhci->usb3.ਸ਼ੁਰੂ = ਸ਼ੁਰੂ;
                        xhci->usb3.ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2;
                    }
                }
                ਰੋਕੋ;
            ਮੂਲ:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                ਰੋਕੋ;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } ਜਦੋਂ (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    ਜੇ (ਪੰਨਾ_ਮਾਪ != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        ਮੁਕਤ(xhci);
        ਵਾਪਸ ਨਿਰਾ;
    }

    ਵਾਪਸ xhci;
}

ਸਥਿਰ ਖਾਲੀ
xhci_controller_ਤਿਆਰੀ_pci(ਬਣਤਰ pci_device *pci)
{
    ਬਣਤਰ usb_xhci_s *xhci;
    ਖਾਲੀ *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!baseaddr)
        ਵਾਪਸ;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_ਤਿਆਰੀ(baseaddr);
    ਜੇ (!xhci)
        ਵਾਪਸ;

    xhci->usb.pci = pci;
    ਲੜੀਵਾਰ_thread(configure_xhci, xhci);
}

ਸਥਿਰ ਖਾਲੀ
xhci_controller_ਤਿਆਰੀ_acpi(ਬਣਤਰ acpi_device *dev)
{
    ਬਣਤਰ usb_xhci_s *xhci;
    u64 mem, ਅਣਵਰਤਿਆ;
    ਖਾਲੀ *baseaddr;

    ਜੇ (acpi_dsdt_ਲੱਭੋ_mem(dev, &mem, &ਅਣਵਰਤਿਆ) < 0)
        ਵਾਪਸ;
    ਜੇ (mem >= 0x100000000ll)
        ਵਾਪਸ;

    baseaddr = (ਖਾਲੀ*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_ਤਿਆਰੀ(baseaddr);
    ਜੇ (!xhci)
        ਵਾਪਸ;

    xhci->usb.mmio = baseaddr;
    ਲੜੀਵਾਰ_thread(configure_xhci, xhci);
}

ਖਾਲੀ
xhci_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_XHCI)
        ਵਾਪਸ;

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_ਤਿਆਰੀ_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    ਬਣਤਰ acpi_device *dev;
    ਲਈ (dev = acpi_dsdt_ਲੱਭੋ_eisaid(ਨਿਰਾ, xhci_eisaid);
         dev != ਨਿਰਾ;
         dev = acpi_dsdt_ਲੱਭੋ_eisaid(dev, xhci_eisaid)) {
        xhci_controller_ਤਿਆਰੀ_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ਸਥਿਰ ਖਾਲੀ xhci_doorbell(ਬਣਤਰ usb_xhci_s *xhci, u32 slotid, u32 ਮੁੱਲ)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, ਮੁੱਲ);
    ਬਣਤਰ xhci_db *db = xhci->db;
    ਖਾਲੀ *addr = &db[slotid].doorbell;
    writel(addr, ਮੁੱਲ);
}

// Dequeue events on the XHCI command ring generated by the hardware
ਸਥਿਰ ਖਾਲੀ xhci_process_events(ਬਣਤਰ usb_xhci_s *xhci)
{
    ਬਣਤਰ xhci_ring *evts = xhci->evts;

    ਲਈ (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        ਬਣਤਰ xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        ਜੇ ((control & TRB_C) != (cs ? 1 : 0))
            ਵਾਪਸ;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->ਸਥਿਤੀ >> 24) & 0xff;
        ਚੋਣ (evt_type) {
        ਹਾਲਤ ER_TRANSFER:
        ਹਾਲਤ ER_COMMAND_COMPLETE:
        {
            ਬਣਤਰ xhci_trb  *rtrb = (ਖਾਲੀ*)etrb->ptr_ਨੀਵਾਂ;
            ਬਣਤਰ xhci_ring *ring = XHCI_RING(rtrb);
            ਬਣਤਰ xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(evt, etrb, ਆਕਾਰ(*etrb));
            ring->eidx = eidx;
            ਰੋਕੋ;
        }
        ਹਾਲਤ ER_ਪੋਰਟ_ਸਥਿਤੀ_CHANGE:
        {
            u32 ਪੋਰਟ = ((etrb->ptr_ਨੀਵਾਂ >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[ਪੋਰਟ].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_ਨਕਾਬ<<XHCI_PORTSC_PLS_ਖਿਸਕਾਓ))
                          | (1<<XHCI_PORTSC_PLS_ਖਿਸਕਾਓ));
            writel(&xhci->pr[ਪੋਰਟ].portsc, pclear);

            xhci_print_ਪੋਰਟ_state(3, __func__, ਪੋਰਟ, portsc);
            ਰੋਕੋ;
        }
        ਮੂਲ:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            ਰੋਕੋ;
        }

        /* move ring index, notify xhci */
        nidx++;
        ਜੇ (nidx == XHCI_RING_ਚੀਜ਼ਾਂ) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        ਬਣਤਰ xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_ਨੀਵਾਂ, erdp);
        writel(&ir->erdp_ਉੱਚਾ, 0);
    }
}

// Check if a ring has any pending TRBs
ਸਥਿਰ ਪੂਰਨ xhci_ring_busy(ਬਣਤਰ xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    ਵਾਪਸ (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ਸਥਿਰ ਪੂਰਨ xhci_event_wait(ਬਣਤਰ usb_xhci_s *xhci,
                           ਬਣਤਰ xhci_ring *ring,
                           u32 timeout)
{
    u32 ਸਿਰਾ = timer_calc(timeout);

    ਲਈ (;;) {
        xhci_process_events(xhci);
        ਜੇ (!xhci_ring_busy(ring)) {
            u32 ਸਥਿਤੀ = ring->evt.ਸਥਿਤੀ;
            ਵਾਪਸ (ਸਥਿਤੀ >> 24) & 0xff;
        }
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ਸਥਿਰ ਖਾਲੀ xhci_trb_fill(ਬਣਤਰ xhci_ring *ring
                          , ਖਾਲੀ *ਅੰਕੜੇ, u32 xferlen, u32 flags)
{
    ਬਣਤਰ xhci_trb *dst = &ring->ring[ring->nidx];
    ਜੇ (flags & TRB_TR_IDT) {
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&dst->ptr_ਨੀਵਾਂ, ਅੰਕੜੇ, xferlen);
    } ਨਹੀਂਤਾਂ {
        dst->ptr_ਨੀਵਾਂ = (u32)ਅੰਕੜੇ;
        dst->ptr_ਉੱਚਾ = 0;
    }
    dst->ਸਥਿਤੀ = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ਸਥਿਰ ਖਾਲੀ xhci_trb_ਕਤਾਰ(ਬਣਤਰ xhci_ring *ring,
                           ਖਾਲੀ *ਅੰਕੜੇ, u32 xferlen, u32 flags)
{
    ਜੇ (ring->nidx >= ARRAY_ਮਾਪ(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, ਅੰਕੜੇ, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
ਸਥਿਰ ਪੂਰਨ xhci_cmd_submit(ਬਣਤਰ usb_xhci_s *xhci, ਬਣਤਰ xhci_inctx *inctx
                           , u32 flags)
{
    ਜੇ (inctx) {
        ਬਣਤਰ xhci_slotctx *ਥਾਂ = (ਖਾਲੀ*)&inctx[1 << xhci->context64];
        u32 ਪੋਰਟ = ((ਥਾਂ->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[ਪੋਰਟ].portsc);
        ਜੇ (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_ਪੋਰਟ_state(1, __func__, ਪੋਰਟ, portsc);
            ਵਾਪਸ -1;
        }
    }

    mutex_ਤਾਲਾ(&xhci->cmds->ਤਾਲਾ);
    xhci_trb_ਕਤਾਰ(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    ਪੂਰਨ rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->ਤਾਲਾ);
    ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ xhci_cmd_enable_ਥਾਂ(ਬਣਤਰ usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    ਪੂਰਨ cc = xhci_cmd_submit(xhci, ਨਿਰਾ, CR_ENABLE_ਥਾਂ << 10);
    ਜੇ (cc != CC_SUCCESS)
        ਵਾਪਸ -1;
    ਵਾਪਸ (xhci->cmds->evt.control >> 24) & 0xff;
}

ਸਥਿਰ ਪੂਰਨ xhci_cmd_disable_ਥਾਂ(ਬਣਤਰ usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ਵਾਪਸ xhci_cmd_submit(xhci, ਨਿਰਾ, (CR_DISABLE_ਥਾਂ << 10) | (slotid << 24));
}

ਸਥਿਰ ਪੂਰਨ xhci_cmd_ਪਤਾ_device(ਬਣਤਰ usb_xhci_s *xhci, u32 slotid
                                   , ਬਣਤਰ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ਵਾਪਸ xhci_cmd_submit(xhci, inctx
                           , (CR_ਪਤਾ_DEVICE << 10) | (slotid << 24));
}

ਸਥਿਰ ਪੂਰਨ xhci_cmd_configure_endpoint(ਬਣਤਰ usb_xhci_s *xhci, u32 slotid
                                       , ਬਣਤਰ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->ਜੋੜੋ, inctx->del);
    ਵਾਪਸ xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ਸਥਿਰ ਪੂਰਨ xhci_cmd_evaluate_context(ਬਣਤਰ usb_xhci_s *xhci, u32 slotid
                                     , ਬਣਤਰ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->ਜੋੜੋ, inctx->del);
    ਵਾਪਸ xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ਸਥਿਰ ਬਣਤਰ xhci_inctx *
xhci_alloc_inctx(ਬਣਤਰ usbdevice_s *usbdev, ਪੂਰਨ maxepid)
{
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_xhci_s, usb);
    ਪੂਰਨ ਮਾਪ = (ਆਕਾਰ(ਬਣਤਰ xhci_inctx) * 33) << xhci->context64;
    ਬਣਤਰ xhci_inctx *ਵਿੱਚ_2 = memalign_tmphigh(2048 << xhci->context64, ਮਾਪ);
    ਜੇ (!ਵਿੱਚ_2) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(ਵਿੱਚ_2, 0, ਮਾਪ);

    ਬਣਤਰ xhci_slotctx *ਥਾਂ = (ਖਾਲੀ*)&ਵਿੱਚ_2[1 << xhci->context64];
    ਥਾਂ->ctx[0]    |= maxepid << 27; // context entries
    ਥਾਂ->ctx[0]    |= ਰਫਤਾਰ_ਮੰਜ਼ਿਲ_xhci[usbdev->ਰਫਤਾਰ] << 20;

    // Set high-speed hub flags.
    ਬਣਤਰ usbdevice_s *hubdev = usbdev->hub->usbdev;
    ਜੇ (hubdev) {
        ਜੇ (usbdev->ਰਫਤਾਰ == USB_LOWSPEED || usbdev->ਰਫਤਾਰ == USB_FULLSPEED) {
            ਬਣਤਰ xhci_pipe *hpipe = container_ਦਾ(
                hubdev->defpipe, ਬਣਤਰ xhci_pipe, pipe);
            ਜੇ (hubdev->ਰਫਤਾਰ == USB_HIGHSPEED) {
                ਥਾਂ->ctx[2] |= hpipe->slotid;
                ਥਾਂ->ctx[2] |= (usbdev->ਪੋਰਟ+1) << 8;
            } ਨਹੀਂਤਾਂ {
                ਬਣਤਰ xhci_slotctx *hslot = (ਖਾਲੀ*)xhci->devs[hpipe->slotid].ptr_ਨੀਵਾਂ;
                ਥਾਂ->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        ਜਦੋਂ (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->ਪੋਰਟ+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        ਥਾਂ->ctx[0]    |= route;
    }

    ਥਾਂ->ctx[1]    |= (usbdev->ਪੋਰਟ+1) << 16;

    ਵਾਪਸ ਵਿੱਚ_2;
}

ਸਥਿਰ ਪੂਰਨ xhci_config_hub(ਬਣਤਰ usbhub_s *hub)
{
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        hub->cntl, ਬਣਤਰ usb_xhci_s, usb);
    ਬਣਤਰ xhci_pipe *pipe = container_ਦਾ(
        hub->usbdev->defpipe, ਬਣਤਰ xhci_pipe, pipe);
    ਬਣਤਰ xhci_slotctx *hdslot = (ਖਾਲੀ*)xhci->devs[pipe->slotid].ptr_ਨੀਵਾਂ;
    ਜੇ ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        ਵਾਪਸ 0;
    ਬਣਤਰ xhci_inctx *ਵਿੱਚ_2 = xhci_alloc_inctx(hub->usbdev, 1);
    ਜੇ (!ਵਿੱਚ_2)
        ਵਾਪਸ -1;
    ਵਿੱਚ_2->ਜੋੜੋ = 0x01;
    ਬਣਤਰ xhci_slotctx *ਥਾਂ = (ਖਾਲੀ*)&ਵਿੱਚ_2[1 << xhci->context64];
    ਥਾਂ->ctx[0] |= 1 << 26;
    ਥਾਂ->ctx[1] |= hub->portcount << 24;

    ਪੂਰਨ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, ਵਿੱਚ_2);
    ਮੁਕਤ(ਵਿੱਚ_2);
    ਜੇ (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        ਵਾਪਸ -1;
    }
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਬਣਤਰ usb_pipe *
xhci_alloc_pipe(ਬਣਤਰ usbdevice_s *usbdev
                , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ;
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_xhci_s, usb);
    ਬਣਤਰ xhci_pipe *pipe;
    u32 epid;

    ਜੇ (epdesc->b_Endpoint_ਪਤਾ == 0) {
        epid = 1;
    } ਨਹੀਂਤਾਂ {
        epid = (epdesc->b_Endpoint_ਪਤਾ & 0x0f) * 2;
        epid += (epdesc->b_Endpoint_ਪਤਾ & USB_DIR_ਵਿੱਚ) ? 1 : 0;
    }

    ਜੇ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ਉੱਚਾ(XHCI_RING_ਮਾਪ, ਆਕਾਰ(*pipe));
    ਨਹੀਂਤਾਂ
        pipe = memalign_ਨੀਵਾਂ(XHCI_RING_ਮਾਪ, ਆਕਾਰ(*pipe));
    ਜੇ (!pipe) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(pipe, 0, ਆਕਾਰ(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    ਜੇ (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ਉੱਚਾ(pipe->pipe.maxpacket);
        ਜੇ (!pipe->buf) {
            warn_noalloc();
            ਮੁਕਤ(pipe);
            ਵਾਪਸ ਨਿਰਾ;
        }
    }

    // Allocate input context and initialize endpoint info.
    ਬਣਤਰ xhci_inctx *ਵਿੱਚ_2 = xhci_alloc_inctx(usbdev, epid);
    ਜੇ (!ਵਿੱਚ_2)
        ਜਾਓ fail;
    ਵਿੱਚ_2->ਜੋੜੋ = 0x01 | (1 << epid);
    ਬਣਤਰ xhci_epctx *ep = (ਖਾਲੀ*)&ਵਿੱਚ_2[(pipe->epid+1) << xhci->context64];
    ਜੇ (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    ਜੇ (epdesc->b_Endpoint_ਪਤਾ & USB_DIR_ਵਿੱਚ
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_ਨੀਵਾਂ  = (u32)&pipe->reqs.ring[0];
    ep->deq_ਨੀਵਾਂ  |= 1;         // dcs
    ep->ਲੰਮਾਈ   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ਜੇ (pipe->epid == 1) {
        ਜੇ (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            ਪੂਰਨ ret = xhci_config_hub(usbdev->hub);
            ਜੇ (ret)
                ਜਾਓ fail;
        }
        // Enable slot.
        u32 ਮਾਪ = (ਆਕਾਰ(ਬਣਤਰ xhci_slotctx) * 32) << xhci->context64;
        ਬਣਤਰ xhci_slotctx *dev = memalign_ਉੱਚਾ(1024 << xhci->context64, ਮਾਪ);
        ਜੇ (!dev) {
            warn_noalloc();
            ਜਾਓ fail;
        }
        ਪੂਰਨ slotid = xhci_cmd_enable_ਥਾਂ(xhci);
        ਜੇ (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            ਮੁਕਤ(dev);
            ਜਾਓ fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, ਮਾਪ);
        xhci->devs[slotid].ptr_ਨੀਵਾਂ = (u32)dev;
        xhci->devs[slotid].ptr_ਉੱਚਾ = 0;

        // Send set_address command.
        ਪੂਰਨ cc = xhci_cmd_ਪਤਾ_device(xhci, slotid, ਵਿੱਚ_2);
        ਜੇ (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_ਥਾਂ(xhci, slotid);
            ਜੇ (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                ਜਾਓ fail;
            }
            xhci->devs[slotid].ptr_ਨੀਵਾਂ = 0;
            ਮੁਕਤ(dev);
            ਜਾਓ fail;
        }
        pipe->slotid = slotid;
    } ਨਹੀਂਤਾਂ {
        ਬਣਤਰ xhci_pipe *defpipe = container_ਦਾ(
            usbdev->defpipe, ਬਣਤਰ xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        ਪੂਰਨ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, ਵਿੱਚ_2);
        ਜੇ (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            ਜਾਓ fail;
        }
    }
    ਮੁਕਤ(ਵਿੱਚ_2);
    ਵਾਪਸ &pipe->pipe;

fail:
    ਮੁਕਤ(pipe->buf);
    ਮੁਕਤ(pipe);
    ਮੁਕਤ(ਵਿੱਚ_2);
    ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ usb_pipe *
xhci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev, ਬਣਤਰ usb_pipe *upipe
                  , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਜੇ (!CONFIG_USB_XHCI)
        ਵਾਪਸ ਨਿਰਾ;
    ਜੇ (!epdesc) {
        usb_ਜੋੜੋ_freelist(upipe);
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਜੇ (!upipe)
        ਵਾਪਸ xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ;
    ਪੂਰਨ oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    ਬਣਤਰ xhci_pipe *pipe = container_ਦਾ(upipe, ਬਣਤਰ xhci_pipe, pipe);
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        pipe->pipe.cntl, ਬਣਤਰ usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ਜੇ (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        ਵਾਪਸ upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    ਬਣਤਰ xhci_inctx *ਵਿੱਚ_2 = xhci_alloc_inctx(usbdev, 1);
    ਜੇ (!ਵਿੱਚ_2)
        ਵਾਪਸ upipe;
    ਵਿੱਚ_2->ਜੋੜੋ = (1 << 1);
    ਬਣਤਰ xhci_epctx *ep = (ਖਾਲੀ*)&ਵਿੱਚ_2[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    ਪੂਰਨ cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, ਵਿੱਚ_2);
    ਜੇ (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    ਮੁਕਤ(ਵਿੱਚ_2);

    ਵਾਪਸ upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ਸਥਿਰ ਖਾਲੀ xhci_xfer_ਤਿਆਰੀ(ਬਣਤਰ xhci_pipe *pipe, ਪੂਰਨ dir, ਖਾਲੀ *cmd
                            , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datalen)
{
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        pipe->pipe.cntl, ਬਣਤਰ usb_xhci_s, usb);
    xhci_trb_ਕਤਾਰ(&pipe->reqs, cmd, USB_CONTROL_ਤਿਆਰੀ_ਮਾਪ
                   , (TR_ਤਿਆਰੀ << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    ਜੇ (datalen)
        xhci_trb_ਕਤਾਰ(&pipe->reqs, ਅੰਕੜੇ, datalen, (TR_ਅੰਕੜੇ << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_ਕਤਾਰ(&pipe->reqs, ਨਿਰਾ, 0, (TR_ਸਥਿਤੀ << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ਸਥਿਰ ਖਾਲੀ xhci_xfer_ਸਾਧਾਰਨ(ਬਣਤਰ xhci_pipe *pipe,
                             ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datalen)
{
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        pipe->pipe.cntl, ਬਣਤਰ usb_xhci_s, usb);
    xhci_trb_ਕਤਾਰ(&pipe->reqs, ਅੰਕੜੇ, datalen, (TR_ਸਾਧਾਰਨ << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

ਪੂਰਨ
xhci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
               , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datalen)
{
    ਜੇ (!CONFIG_USB_XHCI)
        ਵਾਪਸ -1;
    ਬਣਤਰ xhci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ xhci_pipe, pipe);
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        pipe->pipe.cntl, ਬਣਤਰ usb_xhci_s, usb);

    ਜੇ (cmd) {
        ਅਚਲ ਬਣਤਰ usb_ctrlrequest *req = cmd;
        ਜੇ (req->bRequest == USB_REQ_ਸੈੱਟ_ਪਤਾ)
            // Set address command sent during xhci_alloc_pipe.
            ਵਾਪਸ 0;
        xhci_xfer_ਤਿਆਰੀ(pipe, dir, (ਖਾਲੀ*)req, ਅੰਕੜੇ, datalen);
    } ਨਹੀਂਤਾਂ {
        xhci_xfer_ਸਾਧਾਰਨ(pipe, ਅੰਕੜੇ, datalen);
    }

    ਪੂਰਨ cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_ਸਮਾਂ(p, datalen));
    ਜੇ (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        ਵਾਪਸ -1;
    }

    ਵਾਪਸ 0;
}

ਪੂਰਨ VISIBLE32FLAT
xhci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਜੇ (!CONFIG_USB_XHCI)
        ਵਾਪਸ -1;

    ਬਣਤਰ xhci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ xhci_pipe, pipe);
    ਬਣਤਰ usb_xhci_s *xhci = container_ਦਾ(
        pipe->pipe.cntl, ਬਣਤਰ usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    ਖਾਲੀ *buf = pipe->buf;
    ਪੂਰਨ bufused = pipe->bufused;

    ਜੇ (!bufused) {
        xhci_xfer_ਸਾਧਾਰਨ(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        ਵਾਪਸ -1;
    }

    xhci_process_events(xhci);
    ਜੇ (xhci_ring_busy(&pipe->reqs))
        ਵਾਪਸ -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.ਸਥਿਤੀ,
            pipe->reqs.evt.control,
            ਅੰਕੜੇ, buf, len);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, buf, len);
    xhci_xfer_ਸਾਧਾਰਨ(pipe, buf, len);
    ਵਾਪਸ 0;
}
