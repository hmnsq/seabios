#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USB_UHCI_H
#ವ್ಯಾಖ್ಯಾನಿಸು __USB_UHCI_H

// usb-uhci.c
ಶೂನ್ಯ uhci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ರಚನೆ usbdevice_s;
ರಚನೆ usb_endpoint_descriptor;
ರಚನೆ usb_pipe;
ರಚನೆ usb_pipe *uhci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev
                                   , ರಚನೆ usb_pipe *upipe
                                   , ರಚನೆ usb_endpoint_descriptor *epdesc);
ಪೂರ್ಣಾಂಕ uhci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
                   , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize);
ಪೂರ್ಣಾಂಕ uhci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ);


/****************************************************************
 * uhci structs and flags
 ****************************************************************/

/* USB port status and control registers */
#ವ್ಯಾಖ್ಯಾನಿಸು USBPORTSC1      16
#ವ್ಯಾಖ್ಯಾನಿಸು USBPORTSC2      18
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_PE          0x0004  /* Port Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_RD          0x0040  /* Resume Detect */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#ವ್ಯಾಖ್ಯಾನಿಸು USBLEGSUP               0xc0
#ವ್ಯಾಖ್ಯಾನಿಸು   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#ವ್ಯಾಖ್ಯಾನಿಸು USBCMD          0
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_RS             0x0001  /* Run/Stop */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_HCRESET        0x0002  /* Host reset */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_GRESET         0x0004  /* Global reset */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_FGR            0x0010  /* Force Global Resume */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#ವ್ಯಾಖ್ಯಾನಿಸು USBSTS          2
#ವ್ಯಾಖ್ಯಾನಿಸು   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBSTS_RD             0x0004  /* Resume Detect */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#ವ್ಯಾಖ್ಯಾನಿಸು USBINTR         4
#ವ್ಯಾಖ್ಯಾನಿಸು   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#ವ್ಯಾಖ್ಯಾನಿಸು   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#ವ್ಯಾಖ್ಯಾನಿಸು USBFRNUM        6
#ವ್ಯಾಖ್ಯಾನಿಸು USBFLBASEADD    8
#ವ್ಯಾಖ್ಯಾನಿಸು USBSOF          12
#ವ್ಯಾಖ್ಯಾನಿಸು   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

ರಚನೆ uhci_framelist {
    u32 links[1024];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_C_ERR_ಮುಖವಾಡ      (3 << 27)       /* Error Counter bits */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_C_ERR_ಸರಿಸು     27
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_ಸಕ್ರಿಯ          (1 << 23)       /* TD Active */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_ACTLEN_ಮುಖವಾಡ     0x7FF   /* actual length, encoded as n - 1 */

#ವ್ಯಾಖ್ಯಾನಿಸು TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#ವ್ಯಾಖ್ಯಾನಿಸು uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_ಸರಿಸು)

#ವ್ಯಾಖ್ಯಾನಿಸು TD_TOKEN_DEVADDR_ಸರಿಸು  8
#ವ್ಯಾಖ್ಯಾನಿಸು TD_TOKEN_TOGGLE_ಸರಿಸು   19
#ವ್ಯಾಖ್ಯಾನಿಸು TD_TOKEN_TOGGLE         (1 << 19)
#ವ್ಯಾಖ್ಯಾನಿಸು TD_TOKEN_EXPLEN_ಸರಿಸು   21
#ವ್ಯಾಖ್ಯಾನಿಸು TD_TOKEN_EXPLEN_ಮುಖವಾಡ    0x7FF   /* expected length, encoded as n-1 */
#ವ್ಯಾಖ್ಯಾನಿಸು TD_TOKEN_PID_ಮುಖವಾಡ       0xFF

#ವ್ಯಾಖ್ಯಾನಿಸು uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_ಮುಖವಾಡ) << \
                                        TD_TOKEN_EXPLEN_ಸರಿಸು)

#ವ್ಯಾಖ್ಯಾನಿಸು uhci_expected_ಉದ್ದಳತೆ(token) ((((token) >> TD_TOKEN_EXPLEN_ಸರಿಸು) + \
                                        1) & TD_TOKEN_EXPLEN_ಮುಖವಾಡ)

ರಚನೆ uhci_td {
    u32 link;
    u32 ಸ್ಥಿತಿ_2;
    u32 token;
    ಶೂನ್ಯ *ಬಫರ್;
} PACKED;

ರಚನೆ uhci_qh {
    u32 link;
    u32 element;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು UHCI_PTR_ಬಿಟ್ಟುಗಳು           0x000F
#ವ್ಯಾಖ್ಯಾನಿಸು UHCI_PTR_ಪದ           0x0001
#ವ್ಯಾಖ್ಯಾನಿಸು UHCI_PTR_QH             0x0002
#ವ್ಯಾಖ್ಯಾನಿಸು UHCI_PTR_ಆಳ          0x0004
#ವ್ಯಾಖ್ಯಾನಿಸು UHCI_PTR_BREADTH        0x0000

#ಆದರೆ_ಅಂತ್ಯ // usb-uhci.h
