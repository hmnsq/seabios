#నిర్వచితం_కాకపోతే __USB_UHCI_H
#నిర్వచించు __USB_UHCI_H

// usb-uhci.c
శూన్య uhci_అమరిక(శూన్య);
నిర్మాణం usbdevice_s;
నిర్మాణం usb_endpoint_descriptor;
నిర్మాణం usb_pipe;
నిర్మాణం usb_pipe *uhci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev
                                   , నిర్మాణం usb_pipe *upipe
                                   , నిర్మాణం usb_endpoint_descriptor *epdesc);
పూర్ణాంకం uhci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
                   , శూన్య *దత్తాంశం, పూర్ణాంకం datasize);
పూర్ణాంకం uhci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం);


/****************************************************************
 * uhci structs and flags
 ****************************************************************/

/* USB port status and control registers */
#నిర్వచించు USBPORTSC1      16
#నిర్వచించు USBPORTSC2      18
#నిర్వచించు   USBPORTSC_CCS         0x0001  /* Current Connect Status
                                         * ("device present") */
#నిర్వచించు   USBPORTSC_CSC         0x0002  /* Connect Status Change */
#నిర్వచించు   USBPORTSC_PE          0x0004  /* Port Enable */
#నిర్వచించు   USBPORTSC_PEC         0x0008  /* Port Enable Change */
#నిర్వచించు   USBPORTSC_DPLUS       0x0010  /* D+ high (line status) */
#నిర్వచించు   USBPORTSC_DMINUS      0x0020  /* D- high (line status) */
#నిర్వచించు   USBPORTSC_RD          0x0040  /* Resume Detect */
#నిర్వచించు   USBPORTSC_RES1        0x0080  /* reserved, always 1 */
#నిర్వచించు   USBPORTSC_LSDA        0x0100  /* Low Speed Device Attached */
#నిర్వచించు   USBPORTSC_PR          0x0200  /* Port Reset */

/* Legacy support register */
#నిర్వచించు USBLEGSUP               0xc0
#నిర్వచించు   USBLEGSUP_RWC         0x8f00  /* the R/WC bits */

/* Command register */
#నిర్వచించు USBCMD          0
#నిర్వచించు   USBCMD_RS             0x0001  /* Run/Stop */
#నిర్వచించు   USBCMD_HCRESET        0x0002  /* Host reset */
#నిర్వచించు   USBCMD_GRESET         0x0004  /* Global reset */
#నిర్వచించు   USBCMD_EGSM           0x0008  /* Global Suspend Mode */
#నిర్వచించు   USBCMD_FGR            0x0010  /* Force Global Resume */
#నిర్వచించు   USBCMD_SWDBG          0x0020  /* SW Debug mode */
#నిర్వచించు   USBCMD_CF             0x0040  /* Config Flag (sw only) */
#నిర్వచించు   USBCMD_MAXP           0x0080  /* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#నిర్వచించు USBSTS          2
#నిర్వచించు   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#నిర్వచించు   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#నిర్వచించు   USBSTS_RD             0x0004  /* Resume Detect */
#నిర్వచించు   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#నిర్వచించు   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#నిర్వచించు   USBSTS_HCH            0x0020  /* HC Halted */

/* Interrupt enable register */
#నిర్వచించు USBINTR         4
#నిర్వచించు   USBINTR_TIMEOUT       0x0001  /* Timeout/CRC error enable */
#నిర్వచించు   USBINTR_RESUME        0x0002  /* Resume interrupt enable */
#నిర్వచించు   USBINTR_IOC           0x0004  /* Interrupt On Complete enable */
#నిర్వచించు   USBINTR_SP            0x0008  /* Short packet interrupt enable */

#నిర్వచించు USBFRNUM        6
#నిర్వచించు USBFLBASEADD    8
#నిర్వచించు USBSOF          12
#నిర్వచించు   USBSOF_DEFAULT        64      /* Frame length is exactly 1 ms */

నిర్మాణం uhci_framelist {
    u32 links[1024];
} PACKED;

#నిర్వచించు TD_CTRL_SPD             (1 << 29)       /* Short Packet Detect */
#నిర్వచించు TD_CTRL_C_ERR_ముసుగు      (3 << 27)       /* Error Counter bits */
#నిర్వచించు TD_CTRL_C_ERR_జరుపు     27
#నిర్వచించు TD_CTRL_LS              (1 << 26)       /* Low Speed Device */
#నిర్వచించు TD_CTRL_IOS             (1 << 25)       /* Isochronous Select */
#నిర్వచించు TD_CTRL_IOC             (1 << 24)       /* Interrupt on Complete */
#నిర్వచించు TD_CTRL_చురుకైన          (1 << 23)       /* TD Active */
#నిర్వచించు TD_CTRL_STALLED         (1 << 22)       /* TD Stalled */
#నిర్వచించు TD_CTRL_DBUFERR         (1 << 21)       /* Data Buffer Error */
#నిర్వచించు TD_CTRL_BABBLE          (1 << 20)       /* Babble Detected */
#నిర్వచించు TD_CTRL_NAK             (1 << 19)       /* NAK Received */
#నిర్వచించు TD_CTRL_CRCTIMEO        (1 << 18)       /* CRC/Time Out Error */
#నిర్వచించు TD_CTRL_BITSTUFF        (1 << 17)       /* Bit Stuff Error */
#నిర్వచించు TD_CTRL_ACTLEN_ముసుగు     0x7FF   /* actual length, encoded as n - 1 */

#నిర్వచించు TD_CTRL_ANY_ERROR       (TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
                                 TD_CTRL_BABBLE | TD_CTRL_CRCTIMEO | \
                                 TD_CTRL_BITSTUFF)
#నిర్వచించు uhci_maxerr(err)                ((err) << TD_CTRL_C_ERR_జరుపు)

#నిర్వచించు TD_TOKEN_DEVADDR_జరుపు  8
#నిర్వచించు TD_TOKEN_TOGGLE_జరుపు   19
#నిర్వచించు TD_TOKEN_TOGGLE         (1 << 19)
#నిర్వచించు TD_TOKEN_EXPLEN_జరుపు   21
#నిర్వచించు TD_TOKEN_EXPLEN_ముసుగు    0x7FF   /* expected length, encoded as n-1 */
#నిర్వచించు TD_TOKEN_PID_ముసుగు       0xFF

#నిర్వచించు uhci_explen(len)        ((((len) - 1) & TD_TOKEN_EXPLEN_ముసుగు) << \
                                        TD_TOKEN_EXPLEN_జరుపు)

#నిర్వచించు uhci_expected_నిడివి(token) ((((token) >> TD_TOKEN_EXPLEN_జరుపు) + \
                                        1) & TD_TOKEN_EXPLEN_ముసుగు)

నిర్మాణం uhci_td {
    u32 link;
    u32 స్థితి_2;
    u32 token;
    శూన్య *బఫరు;
} PACKED;

నిర్మాణం uhci_qh {
    u32 link;
    u32 element;
} PACKED;

#నిర్వచించు UHCI_PTR_బిట్లు           0x000F
#నిర్వచించు UHCI_PTR_పదం           0x0001
#నిర్వచించు UHCI_PTR_QH             0x0002
#నిర్వచించు UHCI_PTR_లోతు          0x0004
#నిర్వచించు UHCI_PTR_BREADTH        0x0000

#అయితే_ముగింపు // usb-uhci.h
