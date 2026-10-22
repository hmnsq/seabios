#यदि_अपरिभाषितम् __USB_OHCI_H
#परिभाषय __USB_OHCI_H

// usb-ohci.c
शून्य ohci_सज्जता(शून्य);
संरचना usbdevice_s;
संरचना usb_endpoint_descriptor;
संरचना usb_pipe;
संरचना usb_pipe *ohci_realloc_pipe(संरचना usbdevice_s *usbdev
                                   , संरचना usb_pipe *upipe
                                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णाङ्क ohci_send_pipe(संरचना usb_pipe *p, पूर्णाङ्क dir, ध्रुव शून्य *cmd
                   , शून्य *दत्त, पूर्णाङ्क datasize);
पूर्णाङ्क ohci_poll_intr(संरचना usb_pipe *p, शून्य *दत्त);


/****************************************************************
 * ohci structs and flags
 ****************************************************************/

संरचना ohci_ed {
    u32 hwINFO;
    u32 hw_पुच्छ_P;
    u32 hw_शीर्ष_P;
    u32 hw_अग्र_ED;
} PACKED;

#परिभाषय ED_ISO          (1 << 15)
#परिभाषय ED_SKIP         (1 << 14)
#परिभाषय ED_LOWSPEED     (1 << 13)
#परिभाषय ED_बहिर्गमनम्          (0x01 << 11)
#परिभाषय ED_अन्तर्           (0x02 << 11)

#परिभाषय ED_C            (0x02)
#परिभाषय ED_H            (0x01)

संरचना ohci_td {
    u32 hwINFO;
    u32 hwCBP;
    u32 hw_अग्र_TD;
    u32 hwBE;
} PACKED;

#परिभाषय TD_CC       0xf0000000
#परिभाषय TD_CC_GET(td_p) ((td_p >>28) & 0x0f)
#परिभाषय TD_DI       0x00E00000

#परिभाषय TD_समाप्तम्     0x00020000
#परिभाषय TD_ISO      0x00010000

#परिभाषय TD_EC       0x0C000000
#परिभाषय TD_T        0x03000000
#परिभाषय TD_T_DATA0  0x02000000
#परिभाषय TD_T_DATA1  0x03000000
#परिभाषय TD_T_TOGGLE 0x00000000
#परिभाषय TD_DP       0x00180000
#परिभाषय TD_DP_सज्जता 0x00000000
#परिभाषय TD_DP_अन्तर्    0x00100000
#परिभाषय TD_DP_बहिर्गमनम्   0x00080000

#परिभाषय TD_R        0x00040000

संरचना ohci_hcca {
    u32  int_सारणी[32];
    u32  frame_no;
    u32  समाप्तम्_शीर्ष;
    u8   reserved[120];
} PACKED;

संरचना ohci_regs {
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
    u32  roothub_स्थितिः;
    u32  roothub_portstatus[15];
} PACKED;

#परिभाषय OHCI_CTRL_CBSR  (3 << 0)
#परिभाषय OHCI_CTRL_PLE   (1 << 2)
#परिभाषय OHCI_CTRL_CLE   (1 << 4)
#परिभाषय OHCI_CTRL_BLE   (1 << 5)
#परिभाषय OHCI_CTRL_HCFS  (3 << 6)
#       define OHCI_USB_RESET   (0 << 6)
#       define OHCI_USB_OPER    (2 << 6)
#परिभाषय OHCI_CTRL_RWC   (1 << 9)

#परिभाषय OHCI_HCR        (1 << 0)
#परिभाषय OHCI_CLF        (1 << 1)
#परिभाषय OHCI_BLF        (1 << 2)

#परिभाषय OHCI_INTR_MIE   (1 << 31)

#परिभाषय RH_PS_CCS            0x00000001
#परिभाषय RH_PS_PES            0x00000002
#परिभाषय RH_PS_PSS            0x00000004
#परिभाषय RH_PS_POCI           0x00000008
#परिभाषय RH_PS_PRS            0x00000010
#परिभाषय RH_PS_PPS            0x00000100
#परिभाषय RH_PS_LSDA           0x00000200
#परिभाषय RH_PS_CSC            0x00010000
#परिभाषय RH_PS_PESC           0x00020000
#परिभाषय RH_PS_PSSC           0x00040000
#परिभाषय RH_PS_OCIC           0x00080000
#परिभाषय RH_PS_PRSC           0x00100000

#परिभाषय RH_HS_LPS            0x00000001
#परिभाषय RH_HS_OCI            0x00000002
#परिभाषय RH_HS_DRWE           0x00008000
#परिभाषय RH_HS_LPSC           0x00010000
#परिभाषय RH_HS_OCIC           0x00020000
#परिभाषय RH_HS_CRWE           0x80000000

#परिभाषय RH_B_DR         0x0000ffff
#परिभाषय RH_B_PPCM       0xffff0000

#परिभाषय RH_A_NDP        (0xff << 0)
#परिभाषय RH_A_PSM        (1 << 8)
#परिभाषय RH_A_NPS        (1 << 9)
#परिभाषय RH_A_DT         (1 << 10)
#परिभाषय RH_A_OCPM       (1 << 11)
#परिभाषय RH_A_NOCP       (1 << 12)
#परिभाषय RH_A_POTPGT     (0xff << 24)

#यदि_अन्तः // usb-ohci.h
