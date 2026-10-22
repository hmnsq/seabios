#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USB_UHCI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USB_UHCI_H

// usb-uhci.c
ਖਾਲੀ uhci_ਤਿਆਰੀ(ਖਾਲੀ);
ਬਣਤਰ usbdevice_s;
ਬਣਤਰ usb_endpoint_descriptor;
ਬਣਤਰ usb_pipe;
ਬਣਤਰ usb_pipe *uhci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev
                                   , ਬਣਤਰ usb_pipe *upipe
                                   , ਬਣਤਰ usb_endpoint_descriptor *epdesc);
ਪੂਰਨ uhci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
                   , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize);
ਪੂਰਨ uhci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ);


/****************************************************************
 * uhci structs and flags
 ****************************************************************/

/* USB port status and control registers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBPORTSC1      16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBPORTSC2      18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_PE          0x0004  /* Port Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_RD          0x0040  /* Resume Detect */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBLEGSUP               0xc0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBCMD          0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_RS             0x0001  /* Run/Stop */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_HCRESET        0x0002  /* Host reset */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_GRESET         0x0004  /* Global reset */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_FGR            0x0010  /* Force Global Resume */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBSTS          2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBSTS_RD             0x0004  /* Resume Detect */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBINTR         4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBFRNUM        6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBFLBASEADD    8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USBSOF          12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

ਬਣਤਰ uhci_framelist {
    u32 links[1024];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_C_ERR_ਨਕਾਬ      (3 << 27)       /* Error Counter bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_C_ERR_ਖਿਸਕਾਓ     27
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_ਸਰਗਰਮ          (1 << 23)       /* TD Active */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_ACTLEN_ਨਕਾਬ     0x7FF   /* actual length, encoded as n - 1 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_ਖਿਸਕਾਓ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_TOKEN_DEVADDR_ਖਿਸਕਾਓ  8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_TOKEN_TOGGLE_ਖਿਸਕਾਓ   19
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_TOKEN_TOGGLE         (1 << 19)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_TOKEN_EXPLEN_ਖਿਸਕਾਓ   21
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_TOKEN_EXPLEN_ਨਕਾਬ    0x7FF   /* expected length, encoded as n-1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_TOKEN_PID_ਨਕਾਬ       0xFF

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_ਨਕਾਬ) << \
                                        TD_TOKEN_EXPLEN_ਖਿਸਕਾਓ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ uhci_expected_ਲੰਮਾਈ(token) ((((token) >> TD_TOKEN_EXPLEN_ਖਿਸਕਾਓ) + \
                                        1) & TD_TOKEN_EXPLEN_ਨਕਾਬ)

ਬਣਤਰ uhci_td {
    u32 link;
    u32 ਸਥਿਤੀ;
    u32 token;
    ਖਾਲੀ *ਬਫਰ;
} PACKED;

ਬਣਤਰ uhci_qh {
    u32 link;
    u32 element;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UHCI_PTR_ਬਿੱਟ           0x000F
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UHCI_PTR_ਪਦ           0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UHCI_PTR_QH             0x0002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UHCI_PTR_ਡੂੰਘਾਈ          0x0004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UHCI_PTR_BREADTH        0x0000

#ਜੇ_ਅੰਤ // usb-uhci.h
