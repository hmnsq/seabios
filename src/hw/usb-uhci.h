#यदि_अपरिभाषितम् __USB_UHCI_H
#परिभाषय __USB_UHCI_H

// usb-uhci.c
शून्य uhci_सज्जता(शून्य);
संरचना usbdevice_s;
संरचना usb_endpoint_descriptor;
संरचना usb_pipe;
संरचना usb_pipe *uhci_realloc_pipe(संरचना usbdevice_s *usbdev
                                   , संरचना usb_pipe *upipe
                                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णाङ्क uhci_send_pipe(संरचना usb_pipe *p, पूर्णाङ्क dir, ध्रुव शून्य *cmd
                   , शून्य *दत्त, पूर्णाङ्क datasize);
पूर्णाङ्क uhci_poll_intr(संरचना usb_pipe *p, शून्य *दत्त);


/****************************************************************
 * uhci structs and flags
 ****************************************************************/

/* USB port status and control registers */
#परिभाषय USBPORTSC1      16
#परिभाषय USBPORTSC2      18
#परिभाषय   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#परिभाषय   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#परिभाषय   USBPORTSC_PE          0x0004  /* Port Enable */
#परिभाषय   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#परिभाषय   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#परिभाषय   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#परिभाषय   USBPORTSC_RD          0x0040  /* Resume Detect */
#परिभाषय   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#परिभाषय   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#परिभाषय   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#परिभाषय USBLEGSUP               0xc0
#परिभाषय   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#परिभाषय USBCMD          0
#परिभाषय   USBCMD_RS             0x0001  /* Run/Stop */
#परिभाषय   USBCMD_HCRESET        0x0002  /* Host reset */
#परिभाषय   USBCMD_GRESET         0x0004  /* Global reset */
#परिभाषय   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#परिभाषय   USBCMD_FGR            0x0010  /* Force Global Resume */
#परिभाषय   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#परिभाषय   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#परिभाषय   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#परिभाषय USBSTS          2
#परिभाषय   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#परिभाषय   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#परिभाषय   USBSTS_RD             0x0004  /* Resume Detect */
#परिभाषय   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#परिभाषय   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#परिभाषय   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#परिभाषय USBINTR         4
#परिभाषय   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#परिभाषय   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#परिभाषय   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#परिभाषय   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#परिभाषय USBFRNUM        6
#परिभाषय USBFLBASEADD    8
#परिभाषय USBSOF          12
#परिभाषय   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

संरचना uhci_framelist {
    u32 links[1024];
} PACKED;

#परिभाषय TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#परिभाषय TD_CTRL_C_ERR_आवरणम्      (3 << 27)       /* Error Counter bits */
#परिभाषय TD_CTRL_C_ERR_सरणम्     27
#परिभाषय TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#परिभाषय TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#परिभाषय TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#परिभाषय TD_CTRL_सक्रिय          (1 << 23)       /* TD Active */
#परिभाषय TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#परिभाषय TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#परिभाषय TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#परिभाषय TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#परिभाषय TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#परिभाषय TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#परिभाषय TD_CTRL_ACTLEN_आवरणम्     0x7FF   /* actual length, encoded as n - 1 */

#परिभाषय TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#परिभाषय uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_सरणम्)

#परिभाषय TD_TOKEN_DEVADDR_सरणम्  8
#परिभाषय TD_TOKEN_TOGGLE_सरणम्   19
#परिभाषय TD_TOKEN_TOGGLE         (1 << 19)
#परिभाषय TD_TOKEN_EXPLEN_सरणम्   21
#परिभाषय TD_TOKEN_EXPLEN_आवरणम्    0x7FF   /* expected length, encoded as n-1 */
#परिभाषय TD_TOKEN_PID_आवरणम्       0xFF

#परिभाषय uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_आवरणम्) << \
                                        TD_TOKEN_EXPLEN_सरणम्)

#परिभाषय uhci_expected_दीर्घता(token) ((((token) >> TD_TOKEN_EXPLEN_सरणम्) + \
                                        1) & TD_TOKEN_EXPLEN_आवरणम्)

संरचना uhci_td {
    u32 link;
    u32 स्थितिः;
    u32 token;
    शून्य *बफर;
} PACKED;

संरचना uhci_qh {
    u32 link;
    u32 element;
} PACKED;

#परिभाषय UHCI_PTR_द्वयङ्काः           0x000F
#परिभाषय UHCI_PTR_पदांशः           0x0001
#परिभाषय UHCI_PTR_QH             0x0002
#परिभाषय UHCI_PTR_गभीरता          0x0004
#परिभाषय UHCI_PTR_BREADTH        0x0000

#यदि_अन्तः // usb-uhci.h
