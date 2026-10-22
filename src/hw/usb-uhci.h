#که_نه_تعریف_شوی __USB_UHCI_H
#تعریف_کړه __USB_UHCI_H

// usb-uhci.c
فراغ uhci_تیاری(فراغ);
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
#تعریف_کړه USBPORTSC1      16
#تعریف_کړه USBPORTSC2      18
#تعریف_کړه   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#تعریف_کړه   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#تعریف_کړه   USBPORTSC_PE          0x0004  /* Port Enable */
#تعریف_کړه   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#تعریف_کړه   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#تعریف_کړه   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#تعریف_کړه   USBPORTSC_RD          0x0040  /* Resume Detect */
#تعریف_کړه   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#تعریف_کړه   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#تعریف_کړه   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#تعریف_کړه USBLEGSUP               0xc0
#تعریف_کړه   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#تعریف_کړه USBCMD          0
#تعریف_کړه   USBCMD_RS             0x0001  /* Run/Stop */
#تعریف_کړه   USBCMD_HCRESET        0x0002  /* Host reset */
#تعریف_کړه   USBCMD_GRESET         0x0004  /* Global reset */
#تعریف_کړه   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#تعریف_کړه   USBCMD_FGR            0x0010  /* Force Global Resume */
#تعریف_کړه   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#تعریف_کړه   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#تعریف_کړه   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#تعریف_کړه USBSTS          2
#تعریف_کړه   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#تعریف_کړه   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#تعریف_کړه   USBSTS_RD             0x0004  /* Resume Detect */
#تعریف_کړه   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#تعریف_کړه   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#تعریف_کړه   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#تعریف_کړه USBINTR         4
#تعریف_کړه   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#تعریف_کړه   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#تعریف_کړه   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#تعریف_کړه   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#تعریف_کړه USBFRNUM        6
#تعریف_کړه USBFLBASEADD    8
#تعریف_کړه USBSOF          12
#تعریف_کړه   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

بنية uhci_framelist {
    u32 links[1024];
} PACKED;

#تعریف_کړه TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#تعریف_کړه TD_CTRL_C_ERR_ماسک      (3 << 27)       /* Error Counter bits */
#تعریف_کړه TD_CTRL_C_ERR_ښويدنه     27
#تعریف_کړه TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#تعریف_کړه TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#تعریف_کړه TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#تعریف_کړه TD_CTRL_فعال          (1 << 23)       /* TD Active */
#تعریف_کړه TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#تعریف_کړه TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#تعریف_کړه TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#تعریف_کړه TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#تعریف_کړه TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#تعریف_کړه TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#تعریف_کړه TD_CTRL_ACTLEN_ماسک     0x7FF   /* actual length, encoded as n - 1 */

#تعریف_کړه TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#تعریف_کړه uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_ښويدنه)

#تعریف_کړه TD_TOKEN_DEVADDR_ښويدنه  8
#تعریف_کړه TD_TOKEN_TOGGLE_ښويدنه   19
#تعریف_کړه TD_TOKEN_TOGGLE         (1 << 19)
#تعریف_کړه TD_TOKEN_EXPLEN_ښويدنه   21
#تعریف_کړه TD_TOKEN_EXPLEN_ماسک    0x7FF   /* expected length, encoded as n-1 */
#تعریف_کړه TD_TOKEN_PID_ماسک       0xFF

#تعریف_کړه uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_ماسک) << \
                                        TD_TOKEN_EXPLEN_ښويدنه)

#تعریف_کړه uhci_expected_اوږدتیا(token) ((((token) >> TD_TOKEN_EXPLEN_ښويدنه) + \
                                        1) & TD_TOKEN_EXPLEN_ماسک)

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

#تعریف_کړه UHCI_PTR_بټونه           0x000F
#تعریف_کړه UHCI_PTR_غړی           0x0001
#تعریف_کړه UHCI_PTR_QH             0x0002
#تعریف_کړه UHCI_PTR_ژوروالی          0x0004
#تعریف_کړه UHCI_PTR_BREADTH        0x0000

#د_که_پای // usb-uhci.h
