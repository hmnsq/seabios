#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USB_OHCI_H
#ವ್ಯಾಖ್ಯಾನಿಸು __USB_OHCI_H

// usb-ohci.c
ಶೂನ್ಯ ohci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ರಚನೆ usbdevice_s;
ರಚನೆ usb_endpoint_descriptor;
ರಚನೆ usb_pipe;
ರಚನೆ usb_pipe *ohci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev
                                   , ರಚನೆ usb_pipe *upipe
                                   , ರಚನೆ usb_endpoint_descriptor *epdesc);
ಪೂರ್ಣಾಂಕ ohci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
                   , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize);
ಪೂರ್ಣಾಂಕ ohci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ);


/****************************************************************
 * ohci structs and flags
 ****************************************************************/

ರಚನೆ ohci_ed {
    u32 hwINFO;
    u32 hw_ಬಾಲ_P;
    u32 hw_ತಲೆ_P;
    u32 hw_ಮುಂದಿನ_ED;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು ED_ISO          (1 << 15)
#ವ್ಯಾಖ್ಯಾನಿಸು ED_SKIP         (1 << 14)
#ವ್ಯಾಖ್ಯಾನಿಸು ED_LOWSPEED     (1 << 13)
#ವ್ಯಾಖ್ಯಾನಿಸು ED_ಹೊರಹೋಗು          (0x01 << 11)
#ವ್ಯಾಖ್ಯಾನಿಸು ED_ಒಳ           (0x02 << 11)

#ವ್ಯಾಖ್ಯಾನಿಸು ED_C            (0x02)
#ವ್ಯಾಖ್ಯಾನಿಸು ED_H            (0x01)

ರಚನೆ ohci_td {
    u32 hwINFO;
    u32 hwCBP;
    u32 hw_ಮುಂದಿನ_TD;
    u32 hwBE;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು TD_CC       0xf0000000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_CC_GET(td_p) ((td_p >>28) & 0x0f)
#ವ್ಯಾಖ್ಯಾನಿಸು TD_DI       0x00E00000

#ವ್ಯಾಖ್ಯಾನಿಸು TD_ಮುಗಿದ     0x00020000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_ISO      0x00010000

#ವ್ಯಾಖ್ಯಾನಿಸು TD_EC       0x0C000000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_T        0x03000000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_T_DATA0  0x02000000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_T_DATA1  0x03000000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_T_TOGGLE 0x00000000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_DP       0x00180000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_DP_ಸಿದ್ಧತೆ 0x00000000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_DP_ಒಳ    0x00100000
#ವ್ಯಾಖ್ಯಾನಿಸು TD_DP_ಹೊರಹೋಗು   0x00080000

#ವ್ಯಾಖ್ಯಾನಿಸು TD_R        0x00040000

ರಚನೆ ohci_hcca {
    u32  int_ಕೋಷ್ಟಕ[32];
    u32  ಚೌಕಟ್ಟು_no;
    u32  ಮುಗಿದ_ತಲೆ;
    u8   reserved[120];
} PACKED;

ರಚನೆ ohci_regs {
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
    u32  roothub_ಸ್ಥಿತಿ;
    u32  roothub_portstatus[15];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_CTRL_CBSR  (3 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_CTRL_PLE   (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_CTRL_CLE   (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_CTRL_BLE   (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_CTRL_HCFS  (3 << 6)
#       define OHCI_USB_RESET   (0 << 6)
#       define OHCI_USB_OPER    (2 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_CTRL_RWC   (1 << 9)

#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_HCR        (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_CLF        (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_BLF        (1 << 2)

#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_INTR_MIE   (1 << 31)

#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_CCS            0x00000001
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_PES            0x00000002
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_PSS            0x00000004
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_POCI           0x00000008
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_PRS            0x00000010
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_PPS            0x00000100
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_LSDA           0x00000200
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_CSC            0x00010000
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_PESC           0x00020000
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_PSSC           0x00040000
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_OCIC           0x00080000
#ವ್ಯಾಖ್ಯಾನಿಸು RH_PS_PRSC           0x00100000

#ವ್ಯಾಖ್ಯಾನಿಸು RH_HS_LPS            0x00000001
#ವ್ಯಾಖ್ಯಾನಿಸು RH_HS_OCI            0x00000002
#ವ್ಯಾಖ್ಯಾನಿಸು RH_HS_DRWE           0x00008000
#ವ್ಯಾಖ್ಯಾನಿಸು RH_HS_LPSC           0x00010000
#ವ್ಯಾಖ್ಯಾನಿಸು RH_HS_OCIC           0x00020000
#ವ್ಯಾಖ್ಯಾನಿಸು RH_HS_CRWE           0x80000000

#ವ್ಯಾಖ್ಯಾನಿಸು RH_B_DR         0x0000ffff
#ವ್ಯಾಖ್ಯಾನಿಸು RH_B_PPCM       0xffff0000

#ವ್ಯಾಖ್ಯಾನಿಸು RH_A_NDP        (0xff << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು RH_A_PSM        (1 << 8)
#ವ್ಯಾಖ್ಯಾನಿಸು RH_A_NPS        (1 << 9)
#ವ್ಯಾಖ್ಯಾನಿಸು RH_A_DT         (1 << 10)
#ವ್ಯಾಖ್ಯಾನಿಸು RH_A_OCPM       (1 << 11)
#ವ್ಯಾಖ್ಯಾನಿಸು RH_A_NOCP       (1 << 12)
#ವ್ಯಾಖ್ಯಾನಿಸು RH_A_POTPGT     (0xff << 24)

#ಆದರೆ_ಅಂತ್ಯ // usb-ohci.h
