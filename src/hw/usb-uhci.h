#اگر_غیر_تعریف_شدہ __USB_UHCI_H
#تعریف_کرو __USB_UHCI_H

// usb-uhci.c
خالی uhci_تیاری(خالی);
ساخت usbdevice_s;
ساخت usb_endpoint_descriptor;
ساخت usb_pipe;
ساخت usb_pipe *uhci_realloc_pipe(ساخت usbdevice_s *usbdev
                                   , ساخت usb_pipe *upipe
                                   , ساخت usb_endpoint_descriptor *epdesc);
صحیح uhci_send_pipe(ساخت usb_pipe *p, صحیح dir, ثابت خالی *cmd
                   , خالی *ڈیٹا, صحیح datasize);
صحیح uhci_poll_intr(ساخت usb_pipe *p, خالی *ڈیٹا);


/****************************************************************
 * uhci structs and flags
 ****************************************************************/

/* USB port status and control registers */
#تعریف_کرو USBPORTSC1      16
#تعریف_کرو USBPORTSC2      18
#تعریف_کرو   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#تعریف_کرو   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#تعریف_کرو   USBPORTSC_PE          0x0004  /* Port Enable */
#تعریف_کرو   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#تعریف_کرو   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#تعریف_کرو   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#تعریف_کرو   USBPORTSC_RD          0x0040  /* Resume Detect */
#تعریف_کرو   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#تعریف_کرو   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#تعریف_کرو   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#تعریف_کرو USBLEGSUP               0xc0
#تعریف_کرو   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#تعریف_کرو USBCMD          0
#تعریف_کرو   USBCMD_RS             0x0001  /* Run/Stop */
#تعریف_کرو   USBCMD_HCRESET        0x0002  /* Host reset */
#تعریف_کرو   USBCMD_GRESET         0x0004  /* Global reset */
#تعریف_کرو   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#تعریف_کرو   USBCMD_FGR            0x0010  /* Force Global Resume */
#تعریف_کرو   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#تعریف_کرو   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#تعریف_کرو   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#تعریف_کرو USBSTS          2
#تعریف_کرو   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#تعریف_کرو   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#تعریف_کرو   USBSTS_RD             0x0004  /* Resume Detect */
#تعریف_کرو   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#تعریف_کرو   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#تعریف_کرو   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#تعریف_کرو USBINTR         4
#تعریف_کرو   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#تعریف_کرو   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#تعریف_کرو   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#تعریف_کرو   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#تعریف_کرو USBFRNUM        6
#تعریف_کرو USBFLBASEADD    8
#تعریف_کرو USBSOF          12
#تعریف_کرو   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

ساخت uhci_framelist {
    u32 links[1024];
} PACKED;

#تعریف_کرو TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#تعریف_کرو TD_CTRL_C_ERR_نقاب      (3 << 27)       /* Error Counter bits */
#تعریف_کرو TD_CTRL_C_ERR_سرکاؤ     27
#تعریف_کرو TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#تعریف_کرو TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#تعریف_کرو TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#تعریف_کرو TD_CTRL_فعال          (1 << 23)       /* TD Active */
#تعریف_کرو TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#تعریف_کرو TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#تعریف_کرو TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#تعریف_کرو TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#تعریف_کرو TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#تعریف_کرو TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#تعریف_کرو TD_CTRL_ACTLEN_نقاب     0x7FF   /* actual length, encoded as n - 1 */

#تعریف_کرو TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#تعریف_کرو uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_سرکاؤ)

#تعریف_کرو TD_TOKEN_DEVADDR_سرکاؤ  8
#تعریف_کرو TD_TOKEN_TOGGLE_سرکاؤ   19
#تعریف_کرو TD_TOKEN_TOGGLE         (1 << 19)
#تعریف_کرو TD_TOKEN_EXPLEN_سرکاؤ   21
#تعریف_کرو TD_TOKEN_EXPLEN_نقاب    0x7FF   /* expected length, encoded as n-1 */
#تعریف_کرو TD_TOKEN_PID_نقاب       0xFF

#تعریف_کرو uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_نقاب) << \
                                        TD_TOKEN_EXPLEN_سرکاؤ)

#تعریف_کرو uhci_expected_لمبائی(token) ((((token) >> TD_TOKEN_EXPLEN_سرکاؤ) + \
                                        1) & TD_TOKEN_EXPLEN_نقاب)

ساخت uhci_td {
    u32 link;
    u32 حالت_2;
    u32 token;
    خالی *بفر;
} PACKED;

ساخت uhci_qh {
    u32 link;
    u32 element;
} PACKED;

#تعریف_کرو UHCI_PTR_بٹ           0x000F
#تعریف_کرو UHCI_PTR_رکن           0x0001
#تعریف_کرو UHCI_PTR_QH             0x0002
#تعریف_کرو UHCI_PTR_گہرائی          0x0004
#تعریف_کرو UHCI_PTR_BREADTH        0x0000

#اگر_ختم // usb-uhci.h
