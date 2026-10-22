#اگر_تعریف_نشده __USB_UHCI_H
#تعریف_کن __USB_UHCI_H

// usb-uhci.c
فراغ uhci_راهاندازی(فراغ);
بنية usbdevice_s;
بنية usb_endpoint_descriptor;
بنية usb_pipe;
بنية usb_pipe *uhci_realloc_pipe(بنية usbdevice_s *usbdev
                                   , بنية usb_pipe *upipe
                                   , بنية usb_endpoint_descriptor *epdesc);
صحيح uhci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
                   , فراغ *داده, صحيح datasize);
صحيح uhci_poll_intr(بنية usb_pipe *p, فراغ *داده);


/****************************************************************
 * uhci structs and flags
 ****************************************************************/

/* USB port status and control registers */
#تعریف_کن USBPORTSC1      16
#تعریف_کن USBPORTSC2      18
#تعریف_کن   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#تعریف_کن   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#تعریف_کن   USBPORTSC_PE          0x0004  /* Port Enable */
#تعریف_کن   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#تعریف_کن   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#تعریف_کن   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#تعریف_کن   USBPORTSC_RD          0x0040  /* Resume Detect */
#تعریف_کن   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#تعریف_کن   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#تعریف_کن   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#تعریف_کن USBLEGSUP               0xc0
#تعریف_کن   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#تعریف_کن USBCMD          0
#تعریف_کن   USBCMD_RS             0x0001  /* Run/Stop */
#تعریف_کن   USBCMD_HCRESET        0x0002  /* Host reset */
#تعریف_کن   USBCMD_GRESET         0x0004  /* Global reset */
#تعریف_کن   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#تعریف_کن   USBCMD_FGR            0x0010  /* Force Global Resume */
#تعریف_کن   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#تعریف_کن   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#تعریف_کن   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#تعریف_کن USBSTS          2
#تعریف_کن   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#تعریف_کن   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#تعریف_کن   USBSTS_RD             0x0004  /* Resume Detect */
#تعریف_کن   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#تعریف_کن   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#تعریف_کن   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#تعریف_کن USBINTR         4
#تعریف_کن   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#تعریف_کن   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#تعریف_کن   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#تعریف_کن   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#تعریف_کن USBFRNUM        6
#تعریف_کن USBFLBASEADD    8
#تعریف_کن USBSOF          12
#تعریف_کن   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

بنية uhci_framelist {
    u32 links[1024];
} PACKED;

#تعریف_کن TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#تعریف_کن TD_CTRL_C_ERR_ماسک      (3 << 27)       /* Error Counter bits */
#تعریف_کن TD_CTRL_C_ERR_لغزش     27
#تعریف_کن TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#تعریف_کن TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#تعریف_کن TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#تعریف_کن TD_CTRL_فعال          (1 << 23)       /* TD Active */
#تعریف_کن TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#تعریف_کن TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#تعریف_کن TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#تعریف_کن TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#تعریف_کن TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#تعریف_کن TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#تعریف_کن TD_CTRL_ACTLEN_ماسک     0x7FF   /* actual length, encoded as n - 1 */

#تعریف_کن TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#تعریف_کن uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_لغزش)

#تعریف_کن TD_TOKEN_DEVADDR_لغزش  8
#تعریف_کن TD_TOKEN_TOGGLE_لغزش   19
#تعریف_کن TD_TOKEN_TOGGLE         (1 << 19)
#تعریف_کن TD_TOKEN_EXPLEN_لغزش   21
#تعریف_کن TD_TOKEN_EXPLEN_ماسک    0x7FF   /* expected length, encoded as n-1 */
#تعریف_کن TD_TOKEN_PID_ماسک       0xFF

#تعریف_کن uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_ماسک) << \
                                        TD_TOKEN_EXPLEN_لغزش)

#تعریف_کن uhci_expected_درازی(token) ((((token) >> TD_TOKEN_EXPLEN_لغزش) + \
                                        1) & TD_TOKEN_EXPLEN_ماسک)

بنية uhci_td {
    u32 link;
    u32 وضعیت;
    u32 token;
    فراغ *میانگیر;
} PACKED;

بنية uhci_qh {
    u32 link;
    u32 element;
} PACKED;

#تعریف_کن UHCI_PTR_بیتها           0x000F
#تعریف_کن UHCI_PTR_جمله           0x0001
#تعریف_کن UHCI_PTR_QH             0x0002
#تعریف_کن UHCI_PTR_عمق          0x0004
#تعریف_کن UHCI_PTR_BREADTH        0x0000

#پایان_اگر // usb-uhci.h
