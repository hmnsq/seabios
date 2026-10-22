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

#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_RING_ವಸ್ತುಗಳು          16
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_RING_ಗಾತ್ರ           (XHCI_RING_ವಸ್ತುಗಳು*ಗಾತ್ರ(ರಚನೆ xhci_trb))

/*
 *  xhci_ring structs are allocated with XHCI_RING_SIZE alignment,
 *  then we can get it from a trb pointer (provided by evt ring).
 */
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_RING(_trb)          \
    ((ರಚನೆ xhci_ring*)((u32)(_trb) & ~(XHCI_RING_ಗಾತ್ರ-1)))

// --------------------------------------------------------------
// bit definitions

#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_RS              (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_HCRST           (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_INTE            (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_HSEE            (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_LHCRST          (1<<7)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_CSS             (1<<8)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_CRS             (1<<9)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_EWE             (1<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_CMD_EU3S            (1<<11)

#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_HCH             (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_HSE             (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_EINT            (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_PCD             (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_SSS             (1<<8)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_RSS             (1<<9)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_SRE             (1<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_CNR             (1<<11)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_STS_HCE             (1<<12)

#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_CCS          (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PED          (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_OCA          (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PR           (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PLS_ಸರಿಸು        5
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PLS_ಮುಖವಾಡ     0xf
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PP           (1<<9)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_ವೇಗ_ಸರಿಸು      10
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_ವೇಗ_ಮುಖವಾಡ   0xf
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_ವೇಗ_ತುಂಬಿದ   (1<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_ವೇಗ_ಕೆಳಗಿನ    (2<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_ವೇಗ_ಮೇಲಿನ   (3<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_ವೇಗ_SUPER  (4<<10)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PIC_ಸರಿಸು        14
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PIC_ಮುಖವಾಡ     0x3
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_LWS          (1<<16)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_CSC          (1<<17)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PEC          (1<<18)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_WRC          (1<<19)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_OCC          (1<<20)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PRC          (1<<21)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_PLC          (1<<22)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_CEC          (1<<23)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_CAS          (1<<24)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_WCE          (1<<25)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_WDE          (1<<26)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_WOE          (1<<27)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_DR           (1<<30)
#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_PORTSC_WPR          (1<<31)

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_C               (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TYPE_ಸರಿಸು          10
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TYPE_ಮುಖವಾಡ       0x3f
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TYPE(t)         (((t) >> TRB_TYPE_ಸರಿಸು) & TRB_TYPE_ಮುಖವಾಡ)

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_EV_ED           (1<<2)

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_ENT          (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_ISP          (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_NS           (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_CH           (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_IOC          (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_IDT          (1<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_TBC_ಸರಿಸು        7
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_TBC_ಮುಖವಾಡ     0x3
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_BEI          (1<<9)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_TLBPC_ಸರಿಸು      16
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_TLBPC_ಮುಖವಾಡ   0xf
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_FRAMEID_ಸರಿಸು    20
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_FRAMEID_ಮುಖವಾಡ 0x7ff
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_SIA          (1<<31)

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_TR_DIR          (1<<16)

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_CR_SLOTID_ಸರಿಸು     24
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_CR_SLOTID_ಮುಖವಾಡ  0xff
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_CR_EPID_ಸರಿಸು       16
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_CR_EPID_ಮುಖವಾಡ    0x1f

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_CR_BSR          (1<<9)
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_CR_DC           (1<<9)

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_LK_TC           (1<<1)

#ವ್ಯಾಖ್ಯಾನಿಸು TRB_INTR_ಸರಿಸು          22
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_INTR_ಮುಖವಾಡ       0x3ff
#ವ್ಯಾಖ್ಯಾನಿಸು TRB_INTR(t)         (((t).ಸ್ಥಿತಿ_2 >> TRB_INTR_ಸರಿಸು) & TRB_INTR_ಮುಖವಾಡ)

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ TRBType {
    TRB_RESERVED = 0,
    TR_ಸಾಮಾನ್ಯ,
    TR_ಸಿದ್ಧತೆ,
    TR_ದತ್ತಾಂಶ,
    TR_ಸ್ಥಿತಿ,
    TR_ISOCH,
    TR_LINK,
    TR_EVDATA,
    TR_NOOP,
    CR_ENABLE_ಸ್ಥಾನ,
    CR_DISABLE_ಸ್ಥಾನ,
    CR_ವಿಳಾಸ_DEVICE,
    CR_CONFIGURE_ENDPOINT,
    CR_EVALUATE_CONTEXT,
    CR_RESET_ENDPOINT,
    CR_STOP_ENDPOINT,
    CR_ಗಣ_TR_ಸರತಿಯಿಂದ_ತೆಗೆ,
    CR_RESET_DEVICE,
    CR_FORCE_EVENT,
    CR_NEGOTIATE_BW,
    CR_ಗಣ_LATENCY_TOLERANCE,
    CR_GET_ಪೋರ್ಟ್_BANDWIDTH,
    CR_FORCE_ಶೀರ್ಷಿಕೆ,
    CR_NOOP,
    ER_TRANSFER = 32,
    ER_COMMAND_COMPLETE,
    ER_ಪೋರ್ಟ್_ಸ್ಥಿತಿ_CHANGE,
    ER_BANDWIDTH_REQUEST,
    ER_DOORBELL,
    ER_HOST_CONTROLLER,
    ER_DEVICE_NOTIFICATION,
    ER_MFINDEX_WRAP,
} TRBType;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ TRBCCode {
    CC_INVALID = 0,
    CC_SUCCESS,
    CC_ದತ್ತಾಂಶ_ಬಫರ್_ERROR,
    CC_BABBLE_DETECTED,
    CC_USB_TRANSACTION_ERROR,
    CC_TRB_ERROR,
    CC_STALL_ERROR,
    CC_RESOURCE_ERROR,
    CC_BANDWIDTH_ERROR,
    CC_NO_SLOTS_ERROR,
    CC_INVALID_ಹರಿವು_TYPE_ERROR,
    CC_ಸ್ಥಾನ_NOT_ENABLED_ERROR,
    CC_EP_NOT_ENABLED_ERROR,
    CC_SHORT_PACKET,
    CC_RING_UNDERRUN,
    CC_RING_OVERRUN,
    CC_VF_ER_ತುಂಬಿದ,
    CC_PARAMETER_ERROR,
    CC_BANDWIDTH_OVERRUN,
    CC_CONTEXT_STATE_ERROR,
    CC_NO_PING_RESPONSE_ERROR,
    CC_EVENT_RING_ತುಂಬಿದ_ERROR,
    CC_INCOMPATIBLE_DEVICE_ERROR,
    CC_MISSED_SERVICE_ERROR,
    CC_COMMAND_RING_STOPPED,
    CC_COMMAND_ABORTED,
    CC_STOPPED,
    CC_STOPPED_ಉದ್ದಳತೆ_INVALID,
    CC_MAX_EXIT_LATENCY_TOO_LARGE_ERROR = 29,
    CC_ISOCH_ಬಫರ್_OVERRUN = 31,
    CC_EVENT_ಕಳೆದುಹೋದ_ERROR,
    CC_UNDEFINED_ERROR,
    CC_INVALID_ಹರಿವು_ID_ERROR,
    CC_SECONDARY_BANDWIDTH_ERROR,
    CC_ವಿಭಜಿಸು_TRANSACTION_ERROR
} TRBCCode;

ಎಣಿಕೆ {
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
    PLS_COMPILANCE_ಬಹುಲಕ = 10,
    PLS_TEST_ಬಹುಲಕ       = 11,
    PLS_RESUME          = 15,
};

#ವ್ಯಾಖ್ಯಾನಿಸು xhci_get_ಕ್ಷೇತ್ರ(ದತ್ತಾಂಶ, ಕ್ಷೇತ್ರ)             \
    (((ದತ್ತಾಂಶ) >> ಕ್ಷೇತ್ರ##_SHIFT) & ಕ್ಷೇತ್ರ##_MASK)

// --------------------------------------------------------------
// state structs

ರಚನೆ xhci_ring {
    ರಚನೆ xhci_trb      ring[XHCI_RING_ವಸ್ತುಗಳು];
    ರಚನೆ xhci_trb      evt;
    u32                  eidx;
    u32                  nidx;
    u32                  cs;
    ರಚನೆ mutex_s       ಬೀಗ_2;
};

ರಚನೆ xhci_portmap {
    u8 ಆರಂಭ;
    u8 ಎಣಿಕೆ_2;
};

ರಚನೆ usb_xhci_s {
    ರಚನೆ usb_s         usb;

    /* devinfo */
    u32                  xcap;
    u32                  ports;
    u32                  slots;
    u8                   context64;
    ರಚನೆ xhci_portmap  usb2;
    ರಚನೆ xhci_portmap  usb3;

    /* xhci registers */
    ರಚನೆ xhci_caps     *caps;
    ರಚನೆ xhci_op       *op;
    ರಚನೆ xhci_pr       *pr;
    ರಚನೆ xhci_ir       *ir;
    ರಚನೆ xhci_db       *db;

    /* xhci data structures */
    ರಚನೆ xhci_devlist  *devs;
    ರಚನೆ xhci_ring     *cmds;
    ರಚನೆ xhci_ring     *evts;
    ರಚನೆ xhci_er_seg   *eseg;
};

ರಚನೆ xhci_pipe {
    ರಚನೆ xhci_ring     reqs;

    ರಚನೆ usb_pipe      pipe;
    u32                  slotid;
    u32                  epid;
    ಶೂನ್ಯ                 *buf;
    ಪೂರ್ಣಾಂಕ                  bufused;
};

// --------------------------------------------------------------
// tables

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ವೇಗ_name[16] = {
    [ 0 ] = " - ",
    [ 1 ] = "Full",
    [ 2 ] = "Low",
    [ 3 ] = "High",
    [ 4 ] = "Super",
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ವೇಗ_from_xhci[16] = {
    [ 0 ] = -1,
    [ 1 ] = USB_FULLSPEED,
    [ 2 ] = USB_LOWSPEED,
    [ 3 ] = USB_HIGHSPEED,
    [ 4 ] = USB_SUPERSPEED,
    [ 5 ... 15 ] = -1,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ವೇಗ_ಗಮ್ಯ_xhci[] = {
    [ USB_FULLSPEED  ] = 1,
    [ USB_LOWSPEED   ] = 2,
    [ USB_HIGHSPEED  ] = 3,
    [ USB_SUPERSPEED ] = 4,
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wait_ಬಿಟ್(u32 *reg, u32 ಮುಖವಾಡ, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ, u32 timeout)
{
    u32 ಕೊನೆ = timer_calc(timeout);

    ಆಗ ((readl(reg) & ಮುಖವಾಡ) != ಮೌಲ್ಯ) {
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
    ಹಿಂತಿರುಗಿ 0;
}


/****************************************************************
 * Root hub
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು XHCI_ಸಮಯ_POSTPOWER 20

// Check if device attached to port
ಸ್ಥಿರ ಶೂನ್ಯ
xhci_print_ಪೋರ್ಟ್_state(ಪೂರ್ಣಾಂಕ loglevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, u32 ಪೋರ್ಟ್, u32 portsc)
{
    u32 pls = xhci_get_ಕ್ಷೇತ್ರ(portsc, XHCI_PORTSC_PLS);
    u32 ವೇಗ = xhci_get_ಕ್ಷೇತ್ರ(portsc, XHCI_PORTSC_ವೇಗ);

    dprintf(loglevel, "%s port #%d: 0x%08x,%s%s pls %d, speed %d [%s]\n",
            ಪೂರ್ವಪ್ರತ್ಯಯ, ಪೋರ್ಟ್ + 1, portsc,
            (portsc & XHCI_PORTSC_PP)  ? " powered," : "",
            (portsc & XHCI_PORTSC_PED) ? " enabled," : "",
            pls, ವೇಗ, ವೇಗ_name[ವೇಗ]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
xhci_hub_detect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_xhci_s *xhci = container_ದ(hub->cntl, ರಚನೆ usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[ಪೋರ್ಟ್].portsc);
    ಹಿಂತಿರುಗಿ (portsc & XHCI_PORTSC_CCS) ? 1 : 0;
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
xhci_hub_reset(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_xhci_s *xhci = container_ದ(hub->cntl, ರಚನೆ usb_xhci_s, usb);
    u32 portsc = readl(&xhci->pr[ಪೋರ್ಟ್].portsc);
    ಆದರೆ (!(portsc & XHCI_PORTSC_CCS))
        // Device no longer connected?!
        ಹಿಂತಿರುಗಿ -1;

    ಆಯ್ಕೆ (xhci_get_ಕ್ಷೇತ್ರ(portsc, XHCI_PORTSC_PLS)) {
    ಸ್ಥಿತಿ PLS_U0:
        // A USB3 port - controller automatically performs reset
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ PLS_POLLING:
        // A USB2 port - perform device reset
        xhci_print_ಪೋರ್ಟ್_state(3, __func__, ಪೋರ್ಟ್, portsc);
        writel(&xhci->pr[ಪೋರ್ಟ್].portsc, portsc | XHCI_PORTSC_PR);
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ -1;
    }

    // Wait for device to complete reset and be enabled
    u32 ಕೊನೆ = timer_calc(100);
    ಗಾಗಿ (;;) {
        portsc = readl(&xhci->pr[ಪೋರ್ಟ್].portsc);
        ಆದರೆ (!(portsc & XHCI_PORTSC_CCS))
            // Device disconnected during reset
            ಹಿಂತಿರುಗಿ -1;
        ಆದರೆ (portsc & XHCI_PORTSC_PED)
            // Reset complete
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }

    ಪೂರ್ಣಾಂಕ rc = ವೇಗ_from_xhci[xhci_get_ಕ್ಷೇತ್ರ(portsc, XHCI_PORTSC_ವೇಗ)];
    xhci_print_ಪೋರ್ಟ್_state(1, "XHCI", ಪೋರ್ಟ್, portsc);
    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
xhci_hub_portmap(ರಚನೆ usbhub_s *hub, u32 vport)
{
    ರಚನೆ usb_xhci_s *xhci = container_ದ(hub->cntl, ರಚನೆ usb_xhci_s, usb);
    u32 pport = vport + 1;

    ಆದರೆ (vport + 1 >= xhci->usb3.ಆರಂಭ &&
        vport + 1 < xhci->usb3.ಆರಂಭ + xhci->usb3.ಎಣಿಕೆ_2)
        pport = vport + 2 - xhci->usb3.ಆರಂಭ;

    ಆದರೆ (vport + 1 >= xhci->usb2.ಆರಂಭ &&
        vport + 1 < xhci->usb2.ಆರಂಭ + xhci->usb2.ಎಣಿಕೆ_2)
        pport = vport + 2 - xhci->usb2.ಆರಂಭ;

    ಹಿಂತಿರುಗಿ pport;
}

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_hub_disconnect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    // XXX - should turn the port power off.
}

ಸ್ಥಿರ ರಚನೆ usbhub_op_s xhci_hub_ops = {
    .detect = xhci_hub_detect,
    .reset = xhci_hub_reset,
    .portmap = xhci_hub_portmap,
    .disconnect = xhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
xhci_check_ports(ರಚನೆ usb_xhci_s *xhci)
{
    // Wait for port power to stabilize.
    msleep(XHCI_ಸಮಯ_POSTPOWER);

    ರಚನೆ usbhub_s hub;
    memset(&hub, 0, ಗಾತ್ರ(hub));
    hub.cntl = &xhci->usb;
    hub.portcount = xhci->ports;
    hub.op = &xhci_hub_ops;
    usb_enumerate(&hub);
    ಹಿಂತಿರುಗಿ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_free_pipes(ರಚನೆ usb_xhci_s *xhci)
{
    // XXX - should walk list of pipes and free unused pipes.
}

ಸ್ಥಿರ ಶೂನ್ಯ
configure_xhci(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ usb_xhci_s *xhci = ದತ್ತಾಂಶ;
    u32 reg;

    xhci->devs = memalign_ಮೇಲಿನ(64, ಗಾತ್ರ(*xhci->devs) * (xhci->slots + 1));
    xhci->eseg = memalign_ಮೇಲಿನ(64, ಗಾತ್ರ(*xhci->eseg));
    xhci->cmds = memalign_ಮೇಲಿನ(XHCI_RING_ಗಾತ್ರ, ಗಾತ್ರ(*xhci->cmds));
    xhci->evts = memalign_ಮೇಲಿನ(XHCI_RING_ಗಾತ್ರ, ಗಾತ್ರ(*xhci->evts));
    ಆದರೆ (!xhci->devs || !xhci->cmds || !xhci->evts || !xhci->eseg) {
        warn_noalloc();
        ಹೋಗು fail;
    }
    memset(xhci->devs, 0, ಗಾತ್ರ(*xhci->devs) * (xhci->slots + 1));
    memset(xhci->cmds, 0, ಗಾತ್ರ(*xhci->cmds));
    memset(xhci->evts, 0, ಗಾತ್ರ(*xhci->evts));
    memset(xhci->eseg, 0, ಗಾತ್ರ(*xhci->eseg));

    reg = readl(&xhci->op->usbcmd);
    ಆದರೆ (reg & XHCI_CMD_RS) {
        reg &= ~XHCI_CMD_RS;
        writel(&xhci->op->usbcmd, reg);
        ಆದರೆ (wait_ಬಿಟ್(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32) != 0)
            ಹೋಗು fail;
    }

    dprintf(3, "%s: resetting\n", __func__);
    writel(&xhci->op->usbcmd, XHCI_CMD_HCRST);
    ಆದರೆ (wait_ಬಿಟ್(&xhci->op->usbcmd, XHCI_CMD_HCRST, 0, 1000) != 0)
        ಹೋಗು fail;
    ಆದರೆ (wait_ಬಿಟ್(&xhci->op->usbsts, XHCI_STS_CNR, 0, 1000) != 0)
        ಹೋಗು fail;

    writel(&xhci->op->config, xhci->slots);
    writel(&xhci->op->dcbaap_ಕೆಳಗಿನ, (u32)xhci->devs);
    writel(&xhci->op->dcbaap_ಮೇಲಿನ, 0);
    writel(&xhci->op->crcr_ಕೆಳಗಿನ, (u32)xhci->cmds | 1);
    writel(&xhci->op->crcr_ಮೇಲಿನ, 0);
    xhci->cmds->cs = 1;

    xhci->eseg->ptr_ಕೆಳಗಿನ = (u32)xhci->evts;
    xhci->eseg->ptr_ಮೇಲಿನ = 0;
    xhci->eseg->ಗಾತ್ರ_2 = XHCI_RING_ವಸ್ತುಗಳು;
    writel(&xhci->ir->erstsz, 1);
    writel(&xhci->ir->erdp_ಕೆಳಗಿನ, (u32)xhci->evts);
    writel(&xhci->ir->erdp_ಮೇಲಿನ, 0);
    writel(&xhci->ir->erstba_ಕೆಳಗಿನ, (u32)xhci->eseg);
    writel(&xhci->ir->erstba_ಮೇಲಿನ, 0);
    xhci->evts->cs = 1;

    reg = readl(&xhci->caps->hcsparams2);
    u32 spb = (reg >> 21 & 0x1f) << 5 | reg >> 27;
    ಆದರೆ (spb) {
        dprintf(3, "%s: setup %d scratch pad buffers\n", __func__, spb);
        u64 *spba = memalign_ಮೇಲಿನ(64, ಗಾತ್ರ(*spba) * spb);
        ಶೂನ್ಯ *pad = memalign_ಮೇಲಿನ(ಪುಟ_ಗಾತ್ರ, ಪುಟ_ಗಾತ್ರ * spb);
        ಆದರೆ (!spba || !pad) {
            warn_noalloc();
            ಬಿಡುಗಡೆ(spba);
            ಬಿಡುಗಡೆ(pad);
            ಹೋಗು fail;
        }
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i = 0; i < spb; i++)
            spba[i] = (u32)pad + (i * ಪುಟ_ಗಾತ್ರ);
        xhci->devs[0].ptr_ಕೆಳಗಿನ = (u32)spba;
        xhci->devs[0].ptr_ಮೇಲಿನ = 0;
    }

    reg = readl(&xhci->op->usbcmd);
    reg |= XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);

    // Find devices
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = xhci_check_ports(xhci);
    xhci_free_pipes(xhci);
    ಆದರೆ (ಎಣಿಕೆ_2)
        // Success
        ಹಿಂತಿರುಗಿ;

    // No devices found - shutdown and free controller.
    dprintf(1, "XHCI no devices found\n");
    reg = readl(&xhci->op->usbcmd);
    reg &= ~XHCI_CMD_RS;
    writel(&xhci->op->usbcmd, reg);
    wait_ಬಿಟ್(&xhci->op->usbsts, XHCI_STS_HCH, XHCI_STS_HCH, 32);

fail:
    ಬಿಡುಗಡೆ(xhci->eseg);
    ಬಿಡುಗಡೆ(xhci->evts);
    ಬಿಡುಗಡೆ(xhci->cmds);
    ಬಿಡುಗಡೆ(xhci->devs);
    ಬಿಡುಗಡೆ(xhci);
}

ಸ್ಥಿರ ರಚನೆ usb_xhci_s*
xhci_controller_ಸಿದ್ಧತೆ(ಶೂನ್ಯ *baseaddr)
{
    ರಚನೆ usb_xhci_s *xhci = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*xhci));
    ಆದರೆ (!xhci) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(xhci, 0, ಗಾತ್ರ(*xhci));
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

    ಆದರೆ (xhci->xcap) {
        u32 off;
        ಶೂನ್ಯ *addr = baseaddr + xhci->xcap;
        ಮಾಡು {
            ರಚನೆ xhci_xcap *xcap = addr;
            u32 ports, name, cap = readl(&xcap->cap);
            ಆಯ್ಕೆ (cap & 0xff) {
            ಸ್ಥಿತಿ 0x02:
                name  = readl(&xcap->ದತ್ತಾಂಶ[0]);
                ports = readl(&xcap->ದತ್ತಾಂಶ[1]);
                u8 major = (cap >> 24) & 0xff;
                u8 minor = (cap >> 16) & 0xff;
                u8 ಎಣಿಕೆ_2 = (ports >> 8) & 0xff;
                u8 ಆರಂಭ = (ports >> 0) & 0xff;
                dprintf(1, "XHCI    protocol %c%c%c%c %x.%02x"
                        ", %d ports (offset %d), def %x\n"
                        , (name >>  0) & 0xff
                        , (name >>  8) & 0xff
                        , (name >> 16) & 0xff
                        , (name >> 24) & 0xff
                        , major, minor
                        , ಎಣಿಕೆ_2, ಆರಂಭ
                        , ports >> 16);
                ಆದರೆ (name == 0x20425355 /* "USB " */) {
                    ಆದರೆ (major == 2) {
                        xhci->usb2.ಆರಂಭ = ಆರಂಭ;
                        xhci->usb2.ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2;
                    }
                    ಆದರೆ (major == 3) {
                        xhci->usb3.ಆರಂಭ = ಆರಂಭ;
                        xhci->usb3.ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2;
                    }
                }
                ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:
                dprintf(1, "XHCI    extcap 0x%x @ %p\n", cap & 0xff, addr);
                ನಿಲ್ಲಿಸು;
            }
            off = (cap >> 8) & 0xff;
            addr += off << 2;
        } ಆಗ (off > 0);
    }

    u32 pagesize = readl(&xhci->op->pagesize);
    ಆದರೆ (ಪುಟ_ಗಾತ್ರ != (pagesize<<12)) {
        dprintf(1, "XHCI driver does not support page size code %d\n"
                , pagesize<<12);
        ಬಿಡುಗಡೆ(xhci);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    ಹಿಂತಿರುಗಿ xhci;
}

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_controller_ಸಿದ್ಧತೆ_pci(ರಚನೆ pci_device *pci)
{
    ರಚನೆ usb_xhci_s *xhci;
    ಶೂನ್ಯ *baseaddr;

    baseaddr = pci_enable_membar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!baseaddr)
        ಹಿಂತಿರುಗಿ;

    dprintf(1, "PCI: XHCI at %pP (mmio %p)\n", pci, baseaddr);
    pci_enable_busmaster(pci);

    xhci = xhci_controller_ಸಿದ್ಧತೆ(baseaddr);
    ಆದರೆ (!xhci)
        ಹಿಂತಿರುಗಿ;

    xhci->usb.pci = pci;
    ಸತತ_thread(configure_xhci, xhci);
}

ಸ್ಥಿರ ಶೂನ್ಯ
xhci_controller_ಸಿದ್ಧತೆ_acpi(ರಚನೆ acpi_device *dev)
{
    ರಚನೆ usb_xhci_s *xhci;
    u64 mem, ಬಳಸದ;
    ಶೂನ್ಯ *baseaddr;

    ಆದರೆ (acpi_dsdt_ಕಂಡುಹಿಡಿ_mem(dev, &mem, &ಬಳಸದ) < 0)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (mem >= 0x100000000ll)
        ಹಿಂತಿರುಗಿ;

    baseaddr = (ಶೂನ್ಯ*)(u32)mem;
    dprintf(1, "ACPI: XHCI at mmio %p\n", baseaddr);

    xhci = xhci_controller_ಸಿದ್ಧತೆ(baseaddr);
    ಆದರೆ (!xhci)
        ಹಿಂತಿರುಗಿ;

    xhci->usb.mmio = baseaddr;
    ಸತತ_thread(configure_xhci, xhci);
}

ಶೂನ್ಯ
xhci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_XHCI)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_XHCI)
            xhci_controller_ಸಿದ್ಧತೆ_pci(pci);
    }

    u16 xhci_eisaid = 0x0d10;
    ರಚನೆ acpi_device *dev;
    ಗಾಗಿ (dev = acpi_dsdt_ಕಂಡುಹಿಡಿ_eisaid(ಖಾಲಿ, xhci_eisaid);
         dev != ಖಾಲಿ;
         dev = acpi_dsdt_ಕಂಡುಹಿಡಿ_eisaid(dev, xhci_eisaid)) {
        xhci_controller_ಸಿದ್ಧತೆ_acpi(dev);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Signal the hardware to process events on a TRB ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_doorbell(ರಚನೆ usb_xhci_s *xhci, u32 slotid, u32 ಮೌಲ್ಯ)
{
    dprintf(5, "%s: slotid %d, epid %d\n", __func__, slotid, ಮೌಲ್ಯ);
    ರಚನೆ xhci_db *db = xhci->db;
    ಶೂನ್ಯ *addr = &db[slotid].doorbell;
    writel(addr, ಮೌಲ್ಯ);
}

// Dequeue events on the XHCI command ring generated by the hardware
ಸ್ಥಿರ ಶೂನ್ಯ xhci_process_events(ರಚನೆ usb_xhci_s *xhci)
{
    ರಚನೆ xhci_ring *evts = xhci->evts;

    ಗಾಗಿ (;;) {
        /* check for event */
        u32 nidx = evts->nidx;
        u32 cs = evts->cs;
        ರಚನೆ xhci_trb *etrb = evts->ring + nidx;
        u32 control = etrb->control;
        ಆದರೆ ((control & TRB_C) != (cs ? 1 : 0))
            ಹಿಂತಿರುಗಿ;

        /* process event */
        u32 evt_type = TRB_TYPE(control);
        u32 evt_cc = (etrb->ಸ್ಥಿತಿ_2 >> 24) & 0xff;
        ಆಯ್ಕೆ (evt_type) {
        ಸ್ಥಿತಿ ER_TRANSFER:
        ಸ್ಥಿತಿ ER_COMMAND_COMPLETE:
        {
            ರಚನೆ xhci_trb  *rtrb = (ಶೂನ್ಯ*)etrb->ptr_ಕೆಳಗಿನ;
            ರಚನೆ xhci_ring *ring = XHCI_RING(rtrb);
            ರಚನೆ xhci_trb  *evt = &ring->evt;
            u32 eidx = rtrb - ring->ring + 1;
            dprintf(5, "%s: ring %p [trb %p, evt %p, type %d, eidx %d, cc %d]\n",
                    __func__, ring, rtrb, evt, evt_type, eidx, evt_cc);
            ಸ್ಮರಣೆ_ನಕಲು(evt, etrb, ಗಾತ್ರ(*etrb));
            ring->eidx = eidx;
            ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ ER_ಪೋರ್ಟ್_ಸ್ಥಿತಿ_CHANGE:
        {
            u32 ಪೋರ್ಟ್ = ((etrb->ptr_ಕೆಳಗಿನ >> 24) & 0xff) - 1;
            // Read status, and clear port status change bits
            u32 portsc = readl(&xhci->pr[ಪೋರ್ಟ್].portsc);
            u32 pclear = (((portsc & ~(XHCI_PORTSC_PED|XHCI_PORTSC_PR))
                           & ~(XHCI_PORTSC_PLS_ಮುಖವಾಡ<<XHCI_PORTSC_PLS_ಸರಿಸು))
                          | (1<<XHCI_PORTSC_PLS_ಸರಿಸು));
            writel(&xhci->pr[ಪೋರ್ಟ್].portsc, pclear);

            xhci_print_ಪೋರ್ಟ್_state(3, __func__, ಪೋರ್ಟ್, portsc);
            ನಿಲ್ಲಿಸು;
        }
        ಪೂರ್ವನಿಯೋಜಿತ:
            dprintf(1, "%s: unknown event, type %d, cc %d\n",
                    __func__, evt_type, evt_cc);
            ನಿಲ್ಲಿಸು;
        }

        /* move ring index, notify xhci */
        nidx++;
        ಆದರೆ (nidx == XHCI_RING_ವಸ್ತುಗಳು) {
            nidx = 0;
            cs = cs ? 0 : 1;
            evts->cs = cs;
        }
        evts->nidx = nidx;
        ರಚನೆ xhci_ir *ir = xhci->ir;
        u32 erdp = (u32)(evts->ring + nidx);
        writel(&ir->erdp_ಕೆಳಗಿನ, erdp);
        writel(&ir->erdp_ಮೇಲಿನ, 0);
    }
}

// Check if a ring has any pending TRBs
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_ring_busy(ರಚನೆ xhci_ring *ring)
{
    u32 eidx = ring->eidx;
    u32 nidx = ring->nidx;
    ಹಿಂತಿರುಗಿ (eidx != nidx);
}

// Wait for a ring to empty (all TRBs processed by hardware)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_event_wait(ರಚನೆ usb_xhci_s *xhci,
                           ರಚನೆ xhci_ring *ring,
                           u32 timeout)
{
    u32 ಕೊನೆ = timer_calc(timeout);

    ಗಾಗಿ (;;) {
        xhci_process_events(xhci);
        ಆದರೆ (!xhci_ring_busy(ring)) {
            u32 ಸ್ಥಿತಿ_2 = ring->evt.ಸ್ಥಿತಿ_2;
            ಹಿಂತಿರುಗಿ (ಸ್ಥಿತಿ_2 >> 24) & 0xff;
        }
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
}

// Add a TRB to the given ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_trb_fill(ರಚನೆ xhci_ring *ring
                          , ಶೂನ್ಯ *ದತ್ತಾಂಶ, u32 xferlen, u32 flags)
{
    ರಚನೆ xhci_trb *dst = &ring->ring[ring->nidx];
    ಆದರೆ (flags & TRB_TR_IDT) {
        ಸ್ಮರಣೆ_ನಕಲು(&dst->ptr_ಕೆಳಗಿನ, ದತ್ತಾಂಶ, xferlen);
    } ಇಲ್ಲದಿದ್ದರೆ {
        dst->ptr_ಕೆಳಗಿನ = (u32)ದತ್ತಾಂಶ;
        dst->ptr_ಮೇಲಿನ = 0;
    }
    dst->ಸ್ಥಿತಿ_2 = xferlen;
    dst->control = flags | (ring->cs ? TRB_C : 0);
}

// Queue a TRB onto a ring, wrapping ring as needed
ಸ್ಥಿರ ಶೂನ್ಯ xhci_trb_ಸರತಿ(ರಚನೆ xhci_ring *ring,
                           ಶೂನ್ಯ *ದತ್ತಾಂಶ, u32 xferlen, u32 flags)
{
    ಆದರೆ (ring->nidx >= ARRAY_ಗಾತ್ರ(ring->ring) - 1) {
        xhci_trb_fill(ring, ring->ring, 0, (TR_LINK << 10) | TRB_LK_TC);
        ring->nidx = 0;
        ring->cs ^= 1;
        dprintf(5, "%s: ring %p [linked]\n", __func__, ring);
    }

    xhci_trb_fill(ring, ದತ್ತಾಂಶ, xferlen, flags);
    ring->nidx++;
    dprintf(5, "%s: ring %p [nidx %d, len %d]\n",
            __func__, ring, ring->nidx, xferlen);
}

// Submit a command to the xhci controller ring
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_cmd_submit(ರಚನೆ usb_xhci_s *xhci, ರಚನೆ xhci_inctx *inctx
                           , u32 flags)
{
    ಆದರೆ (inctx) {
        ರಚನೆ xhci_slotctx *ಸ್ಥಾನ = (ಶೂನ್ಯ*)&inctx[1 << xhci->context64];
        u32 ಪೋರ್ಟ್ = ((ಸ್ಥಾನ->ctx[1] >> 16) & 0xff) - 1;
        u32 portsc = readl(&xhci->pr[ಪೋರ್ಟ್].portsc);
        ಆದರೆ (!(portsc & XHCI_PORTSC_CCS)) {
            // Device no longer connected?!
            xhci_print_ಪೋರ್ಟ್_state(1, __func__, ಪೋರ್ಟ್, portsc);
            ಹಿಂತಿರುಗಿ -1;
        }
    }

    mutex_ಬೀಗ(&xhci->cmds->ಬೀಗ_2);
    xhci_trb_ಸರತಿ(xhci->cmds, inctx, 0, flags);
    xhci_doorbell(xhci, 0, 0);
    ಪೂರ್ಣಾಂಕ rc = xhci_event_wait(xhci, xhci->cmds, 1000);
    mutex_unlock(&xhci->cmds->ಬೀಗ_2);
    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_cmd_enable_ಸ್ಥಾನ(ರಚನೆ usb_xhci_s *xhci)
{
    dprintf(3, "%s:\n", __func__);
    ಪೂರ್ಣಾಂಕ cc = xhci_cmd_submit(xhci, ಖಾಲಿ, CR_ENABLE_ಸ್ಥಾನ << 10);
    ಆದರೆ (cc != CC_SUCCESS)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ (xhci->cmds->evt.control >> 24) & 0xff;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_cmd_disable_ಸ್ಥಾನ(ರಚನೆ usb_xhci_s *xhci, u32 slotid)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ಹಿಂತಿರುಗಿ xhci_cmd_submit(xhci, ಖಾಲಿ, (CR_DISABLE_ಸ್ಥಾನ << 10) | (slotid << 24));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_cmd_ವಿಳಾಸ_device(ರಚನೆ usb_xhci_s *xhci, u32 slotid
                                   , ರಚನೆ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d\n", __func__, slotid);
    ಹಿಂತಿರುಗಿ xhci_cmd_submit(xhci, inctx
                           , (CR_ವಿಳಾಸ_DEVICE << 10) | (slotid << 24));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_cmd_configure_endpoint(ರಚನೆ usb_xhci_s *xhci, u32 slotid
                                       , ರಚನೆ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->ಕೂಡಿಸು, inctx->del);
    ಹಿಂತಿರುಗಿ xhci_cmd_submit(xhci, inctx
                           , (CR_CONFIGURE_ENDPOINT << 10) | (slotid << 24));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_cmd_evaluate_context(ರಚನೆ usb_xhci_s *xhci, u32 slotid
                                     , ರಚನೆ xhci_inctx *inctx)
{
    dprintf(3, "%s: slotid %d, add 0x%x, del 0x%x\n", __func__,
            slotid, inctx->ಕೂಡಿಸು, inctx->del);
    ಹಿಂತಿರುಗಿ xhci_cmd_submit(xhci, inctx
                           , (CR_EVALUATE_CONTEXT << 10) | (slotid << 24));
}

ಸ್ಥಿರ ರಚನೆ xhci_inctx *
xhci_alloc_inctx(ರಚನೆ usbdevice_s *usbdev, ಪೂರ್ಣಾಂಕ maxepid)
{
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_xhci_s, usb);
    ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = (ಗಾತ್ರ(ರಚನೆ xhci_inctx) * 33) << xhci->context64;
    ರಚನೆ xhci_inctx *ಒಳ = memalign_tmphigh(2048 << xhci->context64, ಗಾತ್ರ_2);
    ಆದರೆ (!ಒಳ) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(ಒಳ, 0, ಗಾತ್ರ_2);

    ರಚನೆ xhci_slotctx *ಸ್ಥಾನ = (ಶೂನ್ಯ*)&ಒಳ[1 << xhci->context64];
    ಸ್ಥಾನ->ctx[0]    |= maxepid << 27; // context entries
    ಸ್ಥಾನ->ctx[0]    |= ವೇಗ_ಗಮ್ಯ_xhci[usbdev->ವೇಗ] << 20;

    // Set high-speed hub flags.
    ರಚನೆ usbdevice_s *hubdev = usbdev->hub->usbdev;
    ಆದರೆ (hubdev) {
        ಆದರೆ (usbdev->ವೇಗ == USB_LOWSPEED || usbdev->ವೇಗ == USB_FULLSPEED) {
            ರಚನೆ xhci_pipe *hpipe = container_ದ(
                hubdev->defpipe, ರಚನೆ xhci_pipe, pipe);
            ಆದರೆ (hubdev->ವೇಗ == USB_HIGHSPEED) {
                ಸ್ಥಾನ->ctx[2] |= hpipe->slotid;
                ಸ್ಥಾನ->ctx[2] |= (usbdev->ಪೋರ್ಟ್+1) << 8;
            } ಇಲ್ಲದಿದ್ದರೆ {
                ರಚನೆ xhci_slotctx *hslot = (ಶೂನ್ಯ*)xhci->devs[hpipe->slotid].ptr_ಕೆಳಗಿನ;
                ಸ್ಥಾನ->ctx[2] = hslot->ctx[2];
            }
        }
        u32 route = 0;
        ಆಗ (usbdev->hub->usbdev) {
            route <<= 4;
            route |= (usbdev->ಪೋರ್ಟ್+1) & 0xf;
            usbdev = usbdev->hub->usbdev;
        }
        ಸ್ಥಾನ->ctx[0]    |= route;
    }

    ಸ್ಥಾನ->ctx[1]    |= (usbdev->ಪೋರ್ಟ್+1) << 16;

    ಹಿಂತಿರುಗಿ ಒಳ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xhci_config_hub(ರಚನೆ usbhub_s *hub)
{
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        hub->cntl, ರಚನೆ usb_xhci_s, usb);
    ರಚನೆ xhci_pipe *pipe = container_ದ(
        hub->usbdev->defpipe, ರಚನೆ xhci_pipe, pipe);
    ರಚನೆ xhci_slotctx *hdslot = (ಶೂನ್ಯ*)xhci->devs[pipe->slotid].ptr_ಕೆಳಗಿನ;
    ಆದರೆ ((hdslot->ctx[3] >> 27) == 3)
        // Already configured
        ಹಿಂತಿರುಗಿ 0;
    ರಚನೆ xhci_inctx *ಒಳ = xhci_alloc_inctx(hub->usbdev, 1);
    ಆದರೆ (!ಒಳ)
        ಹಿಂತಿರುಗಿ -1;
    ಒಳ->ಕೂಡಿಸು = 0x01;
    ರಚನೆ xhci_slotctx *ಸ್ಥಾನ = (ಶೂನ್ಯ*)&ಒಳ[1 << xhci->context64];
    ಸ್ಥಾನ->ctx[0] |= 1 << 26;
    ಸ್ಥಾನ->ctx[1] |= hub->portcount << 24;

    ಪೂರ್ಣಾಂಕ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, ಒಳ);
    ಬಿಡುಗಡೆ(ಒಳ);
    ಆದರೆ (cc != CC_SUCCESS) {
        dprintf(1, "%s: configure hub: failed (cc %d)\n", __func__, cc);
        ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ರಚನೆ usb_pipe *
xhci_alloc_pipe(ರಚನೆ usbdevice_s *usbdev
                , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    u8 eptype = epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ;
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_xhci_s, usb);
    ರಚನೆ xhci_pipe *pipe;
    u32 epid;

    ಆದರೆ (epdesc->b_Endpoint_ವಿಳಾಸ == 0) {
        epid = 1;
    } ಇಲ್ಲದಿದ್ದರೆ {
        epid = (epdesc->b_Endpoint_ವಿಳಾಸ & 0x0f) * 2;
        epid += (epdesc->b_Endpoint_ವಿಳಾಸ & USB_DIR_ಒಳ) ? 1 : 0;
    }

    ಆದರೆ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_ಮೇಲಿನ(XHCI_RING_ಗಾತ್ರ, ಗಾತ್ರ(*pipe));
    ಇಲ್ಲದಿದ್ದರೆ
        pipe = memalign_ಕೆಳಗಿನ(XHCI_RING_ಗಾತ್ರ, ಗಾತ್ರ(*pipe));
    ಆದರೆ (!pipe) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(pipe, 0, ಗಾತ್ರ(*pipe));

    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->epid = epid;
    pipe->reqs.cs = 1;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_INT) {
        pipe->buf = malloc_ಮೇಲಿನ(pipe->pipe.maxpacket);
        ಆದರೆ (!pipe->buf) {
            warn_noalloc();
            ಬಿಡುಗಡೆ(pipe);
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        }
    }

    // Allocate input context and initialize endpoint info.
    ರಚನೆ xhci_inctx *ಒಳ = xhci_alloc_inctx(usbdev, epid);
    ಆದರೆ (!ಒಳ)
        ಹೋಗು fail;
    ಒಳ->ಕೂಡಿಸು = 0x01 | (1 << epid);
    ರಚನೆ xhci_epctx *ep = (ಶೂನ್ಯ*)&ಒಳ[(pipe->epid+1) << xhci->context64];
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_INT)
        ep->ctx[0] = (usb_get_period(usbdev, epdesc) + 3) << 16;
    ep->ctx[1]   |= eptype << 3;
    ಆದರೆ (epdesc->b_Endpoint_ವಿಳಾಸ & USB_DIR_ಒಳ
        || eptype == USB_ENDPOINT_XFER_CONTROL)
        ep->ctx[1] |= 1 << 5;
    ep->ctx[1]   |= pipe->pipe.maxpacket << 16;
    ep->deq_ಕೆಳಗಿನ  = (u32)&pipe->reqs.ring[0];
    ep->deq_ಕೆಳಗಿನ  |= 1;         // dcs
    ep->ಉದ್ದಳತೆ   = pipe->pipe.maxpacket;

    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ಆದರೆ (pipe->epid == 1) {
        ಆದರೆ (usbdev->hub->usbdev) {
            // Make sure parent hub is configured.
            ಪೂರ್ಣಾಂಕ ret = xhci_config_hub(usbdev->hub);
            ಆದರೆ (ret)
                ಹೋಗು fail;
        }
        // Enable slot.
        u32 ಗಾತ್ರ_2 = (ಗಾತ್ರ(ರಚನೆ xhci_slotctx) * 32) << xhci->context64;
        ರಚನೆ xhci_slotctx *dev = memalign_ಮೇಲಿನ(1024 << xhci->context64, ಗಾತ್ರ_2);
        ಆದರೆ (!dev) {
            warn_noalloc();
            ಹೋಗು fail;
        }
        ಪೂರ್ಣಾಂಕ slotid = xhci_cmd_enable_ಸ್ಥಾನ(xhci);
        ಆದರೆ (slotid < 0) {
            dprintf(1, "%s: enable slot: failed\n", __func__);
            ಬಿಡುಗಡೆ(dev);
            ಹೋಗು fail;
        }
        dprintf(3, "%s: enable slot: got slotid %d\n", __func__, slotid);
        memset(dev, 0, ಗಾತ್ರ_2);
        xhci->devs[slotid].ptr_ಕೆಳಗಿನ = (u32)dev;
        xhci->devs[slotid].ptr_ಮೇಲಿನ = 0;

        // Send set_address command.
        ಪೂರ್ಣಾಂಕ cc = xhci_cmd_ವಿಳಾಸ_device(xhci, slotid, ಒಳ);
        ಆದರೆ (cc != CC_SUCCESS) {
            dprintf(1, "%s: address device: failed (cc %d)\n", __func__, cc);
            cc = xhci_cmd_disable_ಸ್ಥಾನ(xhci, slotid);
            ಆದರೆ (cc != CC_SUCCESS) {
                dprintf(1, "%s: disable failed (cc %d)\n", __func__, cc);
                ಹೋಗು fail;
            }
            xhci->devs[slotid].ptr_ಕೆಳಗಿನ = 0;
            ಬಿಡುಗಡೆ(dev);
            ಹೋಗು fail;
        }
        pipe->slotid = slotid;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ರಚನೆ xhci_pipe *defpipe = container_ದ(
            usbdev->defpipe, ರಚನೆ xhci_pipe, pipe);
        pipe->slotid = defpipe->slotid;
        // Send configure command.
        ಪೂರ್ಣಾಂಕ cc = xhci_cmd_configure_endpoint(xhci, pipe->slotid, ಒಳ);
        ಆದರೆ (cc != CC_SUCCESS) {
            dprintf(1, "%s: configure endpoint: failed (cc %d)\n", __func__, cc);
            ಹೋಗು fail;
        }
    }
    ಬಿಡುಗಡೆ(ಒಳ);
    ಹಿಂತಿರುಗಿ &pipe->pipe;

fail:
    ಬಿಡುಗಡೆ(pipe->buf);
    ಬಿಡುಗಡೆ(pipe);
    ಬಿಡುಗಡೆ(ಒಳ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ usb_pipe *
xhci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev, ರಚನೆ usb_pipe *upipe
                  , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಆದರೆ (!CONFIG_USB_XHCI)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಆದರೆ (!epdesc) {
        usb_ಕೂಡಿಸು_freelist(upipe);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಆದರೆ (!upipe)
        ಹಿಂತಿರುಗಿ xhci_alloc_pipe(usbdev, epdesc);
    u8 eptype = epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ;
    ಪೂರ್ಣಾಂಕ oldmaxpacket = upipe->maxpacket;
    usb_desc2pipe(upipe, usbdev, epdesc);
    ರಚನೆ xhci_pipe *pipe = container_ದ(upipe, ರಚನೆ xhci_pipe, pipe);
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        pipe->pipe.cntl, ರಚನೆ usb_xhci_s, usb);
    dprintf(3, "%s: usbdev %p, ring %p, slotid %d, epid %d\n", __func__,
            usbdev, &pipe->reqs, pipe->slotid, pipe->epid);
    ಆದರೆ (eptype != USB_ENDPOINT_XFER_CONTROL || upipe->maxpacket == oldmaxpacket)
        ಹಿಂತಿರುಗಿ upipe;

    // maxpacket has changed on control endpoint - update controller.
    dprintf(1, "%s: reconf ctl endpoint pkt size: %d -> %d\n",
            __func__, oldmaxpacket, pipe->pipe.maxpacket);
    ರಚನೆ xhci_inctx *ಒಳ = xhci_alloc_inctx(usbdev, 1);
    ಆದರೆ (!ಒಳ)
        ಹಿಂತಿರುಗಿ upipe;
    ಒಳ->ಕೂಡಿಸು = (1 << 1);
    ರಚನೆ xhci_epctx *ep = (ಶೂನ್ಯ*)&ಒಳ[2 << xhci->context64];
    ep->ctx[1] |= (pipe->pipe.maxpacket << 16);
    ಪೂರ್ಣಾಂಕ cc = xhci_cmd_evaluate_context(xhci, pipe->slotid, ಒಳ);
    ಆದರೆ (cc != CC_SUCCESS) {
        dprintf(1, "%s: reconf ctl endpoint: failed (cc %d)\n",
                __func__, cc);
    }
    ಬಿಡುಗಡೆ(ಒಳ);

    ಹಿಂತಿರುಗಿ upipe;
}

// Submit a USB "setup" message request to the pipe's ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_xfer_ಸಿದ್ಧತೆ(ರಚನೆ xhci_pipe *pipe, ಪೂರ್ಣಾಂಕ dir, ಶೂನ್ಯ *cmd
                            , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datalen)
{
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        pipe->pipe.cntl, ರಚನೆ usb_xhci_s, usb);
    xhci_trb_ಸರತಿ(&pipe->reqs, cmd, USB_CONTROL_ಸಿದ್ಧತೆ_ಗಾತ್ರ
                   , (TR_ಸಿದ್ಧತೆ << 10) | TRB_TR_IDT
                   | ((datalen ? (dir ? 3 : 2) : 0) << 16));
    ಆದರೆ (datalen)
        xhci_trb_ಸರತಿ(&pipe->reqs, ದತ್ತಾಂಶ, datalen, (TR_ದತ್ತಾಂಶ << 10)
                       | ((dir ? 1 : 0) << 16));
    xhci_trb_ಸರತಿ(&pipe->reqs, ಖಾಲಿ, 0, (TR_ಸ್ಥಿತಿ << 10) | TRB_TR_IOC
                   | ((dir ? 0 : 1) << 16));
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

// Submit a USB transfer request to the pipe's ring
ಸ್ಥಿರ ಶೂನ್ಯ xhci_xfer_ಸಾಮಾನ್ಯ(ರಚನೆ xhci_pipe *pipe,
                             ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datalen)
{
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        pipe->pipe.cntl, ರಚನೆ usb_xhci_s, usb);
    xhci_trb_ಸರತಿ(&pipe->reqs, ದತ್ತಾಂಶ, datalen, (TR_ಸಾಮಾನ್ಯ << 10) | TRB_TR_IOC);
    xhci_doorbell(xhci, pipe->slotid, pipe->epid);
}

ಪೂರ್ಣಾಂಕ
xhci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datalen)
{
    ಆದರೆ (!CONFIG_USB_XHCI)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ xhci_pipe *pipe = container_ದ(p, ರಚನೆ xhci_pipe, pipe);
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        pipe->pipe.cntl, ರಚನೆ usb_xhci_s, usb);

    ಆದರೆ (cmd) {
        ಸ್ಥಿರಾಂಕ ರಚನೆ usb_ctrlrequest *req = cmd;
        ಆದರೆ (req->bRequest == USB_REQ_ಗಣ_ವಿಳಾಸ)
            // Set address command sent during xhci_alloc_pipe.
            ಹಿಂತಿರುಗಿ 0;
        xhci_xfer_ಸಿದ್ಧತೆ(pipe, dir, (ಶೂನ್ಯ*)req, ದತ್ತಾಂಶ, datalen);
    } ಇಲ್ಲದಿದ್ದರೆ {
        xhci_xfer_ಸಾಮಾನ್ಯ(pipe, ದತ್ತಾಂಶ, datalen);
    }

    ಪೂರ್ಣಾಂಕ cc = xhci_event_wait(xhci, &pipe->reqs, usb_xfer_ಸಮಯ(p, datalen));
    ಆದರೆ (cc != CC_SUCCESS) {
        dprintf(1, "%s: xfer failed (cc %d)\n", __func__, cc);
        ಹಿಂತಿರುಗಿ -1;
    }

    ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ VISIBLE32FLAT
xhci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ಆದರೆ (!CONFIG_USB_XHCI)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ xhci_pipe *pipe = container_ದ(p, ರಚನೆ xhci_pipe, pipe);
    ರಚನೆ usb_xhci_s *xhci = container_ದ(
        pipe->pipe.cntl, ರಚನೆ usb_xhci_s, usb);
    u32 len = pipe->pipe.maxpacket;
    ಶೂನ್ಯ *buf = pipe->buf;
    ಪೂರ್ಣಾಂಕ bufused = pipe->bufused;

    ಆದರೆ (!bufused) {
        xhci_xfer_ಸಾಮಾನ್ಯ(pipe, buf, len);
        bufused = 1;
        pipe->bufused = bufused;
        ಹಿಂತಿರುಗಿ -1;
    }

    xhci_process_events(xhci);
    ಆದರೆ (xhci_ring_busy(&pipe->reqs))
        ಹಿಂತಿರುಗಿ -1;
    dprintf(5, "%s: st %x ct %x [ %p <= %p / %d ]\n", __func__,
            pipe->reqs.evt.ಸ್ಥಿತಿ_2,
            pipe->reqs.evt.control,
            ದತ್ತಾಂಶ, buf, len);
    ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, buf, len);
    xhci_xfer_ಸಾಮಾನ್ಯ(pipe, buf, len);
    ಹಿಂತಿರುಗಿ 0;
}
