#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USB_OHCI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USB_OHCI_H

// usb-ohci.c
ਖਾਲੀ ohci_ਤਿਆਰੀ(ਖਾਲੀ);
ਬਣਤਰ usbdevice_s;
ਬਣਤਰ usb_endpoint_descriptor;
ਬਣਤਰ usb_pipe;
ਬਣਤਰ usb_pipe *ohci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev
                                   , ਬਣਤਰ usb_pipe *upipe
                                   , ਬਣਤਰ usb_endpoint_descriptor *epdesc);
ਪੂਰਨ ohci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
                   , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize);
ਪੂਰਨ ohci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ);


/****************************************************************
 * ohci structs and flags
 ****************************************************************/

ਬਣਤਰ ohci_ed {
    u32 hwINFO;
    u32 hw_ਪੂਛ_P;
    u32 hw_ਸਿਰ_P;
    u32 hw_ਅਗਲਾ_ED;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ED_ISO          (1 << 15)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ED_SKIP         (1 << 14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ED_LOWSPEED     (1 << 13)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ED_ਬਾਹਰਲਾ          (0x01 << 11)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ED_ਵਿੱਚ           (0x02 << 11)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ED_C            (0x02)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ED_H            (0x01)

ਬਣਤਰ ohci_td {
    u32 hwINFO;
    u32 hwCBP;
    u32 hw_ਅਗਲਾ_TD;
    u32 hwBE;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CC       0xf0000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_CC_GET(td_p) ((td_p >>28) & 0x0f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_DI       0x00E00000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_ਪੂਰਾ     0x00020000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_ISO      0x00010000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_EC       0x0C000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_T        0x03000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_T_DATA0  0x02000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_T_DATA1  0x03000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_T_TOGGLE 0x00000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_DP       0x00180000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_DP_ਤਿਆਰੀ 0x00000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_DP_ਵਿੱਚ    0x00100000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_DP_ਬਾਹਰਲਾ   0x00080000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TD_R        0x00040000

ਬਣਤਰ ohci_hcca {
    u32  int_ਸਾਰਣੀ[32];
    u32  ਚੌਖਟਾ_no;
    u32  ਪੂਰਾ_ਸਿਰ;
    u8   reserved[120];
} PACKED;

ਬਣਤਰ ohci_regs {
    u32  revision;
    u32  control;
    u32  cmdstatus;
    u32  intrstatus;
    u32  intrenable;
    u32  intrdisable;

    u32  hcca;
    u32  ed_periodcurrent;
    u32  ed_controlhead;
    u32  ed_controlcurrent;
    u32  ed_bulkhead;
    u32  ed_bulkcurrent;
    u32  donehead;

    u32  fminterval;
    u32  fmremaining;
    u32  fmnumber;
    u32  periodicstart;
    u32  lsthresh;

    u32  roothub_a;
    u32  roothub_b;
    u32  roothub_ਸਥਿਤੀ;
    u32  roothub_portstatus[15];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_CTRL_CBSR  (3 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_CTRL_PLE   (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_CTRL_CLE   (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_CTRL_BLE   (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_CTRL_HCFS  (3 << 6)
#       define OHCI_USB_RESET   (0 << 6)
#       define OHCI_USB_OPER    (2 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_CTRL_RWC   (1 << 9)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_HCR        (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_CLF        (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_BLF        (1 << 2)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_INTR_MIE   (1 << 31)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_CCS            0x00000001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_PES            0x00000002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_PSS            0x00000004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_POCI           0x00000008
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_PRS            0x00000010
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_PPS            0x00000100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_LSDA           0x00000200
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_CSC            0x00010000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_PESC           0x00020000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_PSSC           0x00040000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_OCIC           0x00080000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_PS_PRSC           0x00100000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_HS_LPS            0x00000001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_HS_OCI            0x00000002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_HS_DRWE           0x00008000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_HS_LPSC           0x00010000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_HS_OCIC           0x00020000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_HS_CRWE           0x80000000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_B_DR         0x0000ffff
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_B_PPCM       0xffff0000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_A_NDP        (0xff << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_A_PSM        (1 << 8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_A_NPS        (1 << 9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_A_DT         (1 << 10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_A_OCPM       (1 << 11)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_A_NOCP       (1 << 12)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RH_A_POTPGT     (0xff << 24)

#ਜੇ_ਅੰਤ // usb-ohci.h
