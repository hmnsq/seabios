#جيڪڏهن_وصف_نه_ٿيل __USB_UHCI_H
#وصف_ڪريو __USB_UHCI_H

// usb-uhci.c
فراغ uhci_تياري(فراغ);
بنية usbdevice_s;
بنية usb_endpoint_descriptor;
بنية usb_pipe;
بنية usb_pipe *uhci_realloc_pipe(بنية usbdevice_s *usbdev
                                   , بنية usb_pipe *upipe
                                   , بنية usb_endpoint_descriptor *epdesc);
صحيح uhci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
                   , فراغ *معلومات, صحيح datasize);
صحيح uhci_poll_intr(بنية usb_pipe *p, فراغ *معلومات);


/****************************************************************
 * uhci structs and flags
 ****************************************************************/

/* USB port status and control registers */
#وصف_ڪريو USBPORTSC1      16
#وصف_ڪريو USBPORTSC2      18
#وصف_ڪريو   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#وصف_ڪريو   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#وصف_ڪريو   USBPORTSC_PE          0x0004  /* Port Enable */
#وصف_ڪريو   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#وصف_ڪريو   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#وصف_ڪريو   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#وصف_ڪريو   USBPORTSC_RD          0x0040  /* Resume Detect */
#وصف_ڪريو   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#وصف_ڪريو   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#وصف_ڪريو   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#وصف_ڪريو USBLEGSUP               0xc0
#وصف_ڪريو   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#وصف_ڪريو USBCMD          0
#وصف_ڪريو   USBCMD_RS             0x0001  /* Run/Stop */
#وصف_ڪريو   USBCMD_HCRESET        0x0002  /* Host reset */
#وصف_ڪريو   USBCMD_GRESET         0x0004  /* Global reset */
#وصف_ڪريو   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#وصف_ڪريو   USBCMD_FGR            0x0010  /* Force Global Resume */
#وصف_ڪريو   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#وصف_ڪريو   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#وصف_ڪريو   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#وصف_ڪريو USBSTS          2
#وصف_ڪريو   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#وصف_ڪريو   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#وصف_ڪريو   USBSTS_RD             0x0004  /* Resume Detect */
#وصف_ڪريو   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#وصف_ڪريو   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#وصف_ڪريو   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#وصف_ڪريو USBINTR         4
#وصف_ڪريو   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#وصف_ڪريو   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#وصف_ڪريو   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#وصف_ڪريو   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#وصف_ڪريو USBFRNUM        6
#وصف_ڪريو USBFLBASEADD    8
#وصف_ڪريو USBSOF          12
#وصف_ڪريو   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

بنية uhci_framelist {
    u32 links[1024];
} PACKED;

#وصف_ڪريو TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#وصف_ڪريو TD_CTRL_C_ERR_نقاب      (3 << 27)       /* Error Counter bits */
#وصف_ڪريو TD_CTRL_C_ERR_سرڪايو     27
#وصف_ڪريو TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#وصف_ڪريو TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#وصف_ڪريو TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#وصف_ڪريو TD_CTRL_سرگرم          (1 << 23)       /* TD Active */
#وصف_ڪريو TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#وصف_ڪريو TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#وصف_ڪريو TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#وصف_ڪريو TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#وصف_ڪريو TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#وصف_ڪريو TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#وصف_ڪريو TD_CTRL_ACTLEN_نقاب     0x7FF   /* actual length, encoded as n - 1 */

#وصف_ڪريو TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#وصف_ڪريو uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_سرڪايو)

#وصف_ڪريو TD_TOKEN_DEVADDR_سرڪايو  8
#وصف_ڪريو TD_TOKEN_TOGGLE_سرڪايو   19
#وصف_ڪريو TD_TOKEN_TOGGLE         (1 << 19)
#وصف_ڪريو TD_TOKEN_EXPLEN_سرڪايو   21
#وصف_ڪريو TD_TOKEN_EXPLEN_نقاب    0x7FF   /* expected length, encoded as n-1 */
#وصف_ڪريو TD_TOKEN_PID_نقاب       0xFF

#وصف_ڪريو uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_نقاب) << \
                                        TD_TOKEN_EXPLEN_سرڪايو)

#وصف_ڪريو uhci_expected_ڊيگهائي(token) ((((token) >> TD_TOKEN_EXPLEN_سرڪايو) + \
                                        1) & TD_TOKEN_EXPLEN_نقاب)

بنية uhci_td {
    u32 link;
    u32 حالت;
    u32 token;
    فراغ *بفر;
} PACKED;

بنية uhci_qh {
    u32 link;
    u32 element;
} PACKED;

#وصف_ڪريو UHCI_PTR_بٽ           0x000F
#وصف_ڪريو UHCI_PTR_رڪن           0x0001
#وصف_ڪريو UHCI_PTR_QH             0x0002
#وصف_ڪريو UHCI_PTR_اونهائي          0x0004
#وصف_ڪريو UHCI_PTR_BREADTH        0x0000

#جيڪڏهن_پورو // usb-uhci.h
