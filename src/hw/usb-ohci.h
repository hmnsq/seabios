#جے_بنا_تعریف __USB_OHCI_H
#تعریف_کرو __USB_OHCI_H

// usb-ohci.c
فراغ ohci_تیاری(فراغ);
بنية usbdevice_s;
بنية usb_endpoint_descriptor;
بنية usb_pipe;
بنية usb_pipe *ohci_realloc_pipe(بنية usbdevice_s *usbdev
                                   , بنية usb_pipe *upipe
                                   , بنية usb_endpoint_descriptor *epdesc);
صحيح ohci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
                   , فراغ *انکڑے, صحيح datasize);
صحيح ohci_poll_intr(بنية usb_pipe *p, فراغ *انکڑے);


/****************************************************************
 * ohci structs and flags
 ****************************************************************/

بنية ohci_ed {
    u32 hwINFO;
    u32 hw_پوچھ_P;
    u32 hw_سر_P;
    u32 hw_اگلا_ED;
} PACKED;

#تعریف_کرو ED_ISO          (1 << 15)
#تعریف_کرو ED_SKIP         (1 << 14)
#تعریف_کرو ED_LOWSPEED     (1 << 13)
#تعریف_کرو ED_باہرلا          (0x01 << 11)
#تعریف_کرو ED_وچ           (0x02 << 11)

#تعریف_کرو ED_C            (0x02)
#تعریف_کرو ED_H            (0x01)

بنية ohci_td {
    u32 hwINFO;
    u32 hwCBP;
    u32 hw_اگلا_TD;
    u32 hwBE;
} PACKED;

#تعریف_کرو TD_CC       0xf0000000
#تعریف_کرو TD_CC_GET(td_p) ((td_p >>28) & 0x0f)
#تعریف_کرو TD_DI       0x00E00000

#تعریف_کرو TD_پورا     0x00020000
#تعریف_کرو TD_ISO      0x00010000

#تعریف_کرو TD_EC       0x0C000000
#تعریف_کرو TD_T        0x03000000
#تعریف_کرو TD_T_DATA0  0x02000000
#تعریف_کرو TD_T_DATA1  0x03000000
#تعریف_کرو TD_T_TOGGLE 0x00000000
#تعریف_کرو TD_DP       0x00180000
#تعریف_کرو TD_DP_تیاری 0x00000000
#تعریف_کرو TD_DP_وچ    0x00100000
#تعریف_کرو TD_DP_باہرلا   0x00080000

#تعریف_کرو TD_R        0x00040000

بنية ohci_hcca {
    u32  int_جدول[32];
    u32  چوکھٹا_no;
    u32  پورا_سر;
    u8   reserved[120];
} PACKED;

بنية ohci_regs {
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
    u32  roothub_حالت;
    u32  roothub_portstatus[15];
} PACKED;

#تعریف_کرو OHCI_CTRL_CBSR  (3 << 0)
#تعریف_کرو OHCI_CTRL_PLE   (1 << 2)
#تعریف_کرو OHCI_CTRL_CLE   (1 << 4)
#تعریف_کرو OHCI_CTRL_BLE   (1 << 5)
#تعریف_کرو OHCI_CTRL_HCFS  (3 << 6)
#       define OHCI_USB_RESET   (0 << 6)
#       define OHCI_USB_OPER    (2 << 6)
#تعریف_کرو OHCI_CTRL_RWC   (1 << 9)

#تعریف_کرو OHCI_HCR        (1 << 0)
#تعریف_کرو OHCI_CLF        (1 << 1)
#تعریف_کرو OHCI_BLF        (1 << 2)

#تعریف_کرو OHCI_INTR_MIE   (1 << 31)

#تعریف_کرو RH_PS_CCS            0x00000001
#تعریف_کرو RH_PS_PES            0x00000002
#تعریف_کرو RH_PS_PSS            0x00000004
#تعریف_کرو RH_PS_POCI           0x00000008
#تعریف_کرو RH_PS_PRS            0x00000010
#تعریف_کرو RH_PS_PPS            0x00000100
#تعریف_کرو RH_PS_LSDA           0x00000200
#تعریف_کرو RH_PS_CSC            0x00010000
#تعریف_کرو RH_PS_PESC           0x00020000
#تعریف_کرو RH_PS_PSSC           0x00040000
#تعریف_کرو RH_PS_OCIC           0x00080000
#تعریف_کرو RH_PS_PRSC           0x00100000

#تعریف_کرو RH_HS_LPS            0x00000001
#تعریف_کرو RH_HS_OCI            0x00000002
#تعریف_کرو RH_HS_DRWE           0x00008000
#تعریف_کرو RH_HS_LPSC           0x00010000
#تعریف_کرو RH_HS_OCIC           0x00020000
#تعریف_کرو RH_HS_CRWE           0x80000000

#تعریف_کرو RH_B_DR         0x0000ffff
#تعریف_کرو RH_B_PPCM       0xffff0000

#تعریف_کرو RH_A_NDP        (0xff << 0)
#تعریف_کرو RH_A_PSM        (1 << 8)
#تعریف_کرو RH_A_NPS        (1 << 9)
#تعریف_کرو RH_A_DT         (1 << 10)
#تعریف_کرو RH_A_OCPM       (1 << 11)
#تعریف_کرو RH_A_NOCP       (1 << 12)
#تعریف_کرو RH_A_POTPGT     (0xff << 24)

#جے_انت // usb-ohci.h
