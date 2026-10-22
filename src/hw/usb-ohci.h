#నిర్వచితం_కాకపోతే __USB_OHCI_H
#నిర్వచించు __USB_OHCI_H

// usb-ohci.c
శూన్య ohci_అమరిక(శూన్య);
నిర్మాణం usbdevice_s;
నిర్మాణం usb_endpoint_descriptor;
నిర్మాణం usb_pipe;
నిర్మాణం usb_pipe *ohci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev
                                   , నిర్మాణం usb_pipe *upipe
                                   , నిర్మాణం usb_endpoint_descriptor *epdesc);
పూర్ణాంకం ohci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
                   , శూన్య *దత్తాంశం, పూర్ణాంకం datasize);
పూర్ణాంకం ohci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం);


/****************************************************************
 * ohci structs and flags
 ****************************************************************/

నిర్మాణం ohci_ed {
    u32 hwINFO;
    u32 hw_తోక_P;
    u32 hw_తల_P;
    u32 hw_తదుపరి_ED;
} PACKED;

#నిర్వచించు ED_ISO          (1 << 15)
#నిర్వచించు ED_SKIP         (1 << 14)
#నిర్వచించు ED_LOWSPEED     (1 << 13)
#నిర్వచించు ED_బహిర్గతం          (0x01 << 11)
#నిర్వచించు ED_లోపల           (0x02 << 11)

#నిర్వచించు ED_C            (0x02)
#నిర్వచించు ED_H            (0x01)

నిర్మాణం ohci_td {
    u32 hwINFO;
    u32 hwCBP;
    u32 hw_తదుపరి_TD;
    u32 hwBE;
} PACKED;

#నిర్వచించు TD_CC       0xf0000000
#నిర్వచించు TD_CC_GET(td_p) ((td_p >>28) & 0x0f)
#నిర్వచించు TD_DI       0x00E00000

#నిర్వచించు TD_పూర్తి     0x00020000
#నిర్వచించు TD_ISO      0x00010000

#నిర్వచించు TD_EC       0x0C000000
#నిర్వచించు TD_T        0x03000000
#నిర్వచించు TD_T_DATA0  0x02000000
#నిర్వచించు TD_T_DATA1  0x03000000
#నిర్వచించు TD_T_TOGGLE 0x00000000
#నిర్వచించు TD_DP       0x00180000
#నిర్వచించు TD_DP_అమరిక 0x00000000
#నిర్వచించు TD_DP_లోపల    0x00100000
#నిర్వచించు TD_DP_బహిర్గతం   0x00080000

#నిర్వచించు TD_R        0x00040000

నిర్మాణం ohci_hcca {
    u32  int_పట్టిక[32];
    u32  చట్రం_no;
    u32  పూర్తి_తల;
    u8   reserved[120];
} PACKED;

నిర్మాణం ohci_regs {
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
    u32  roothub_స్థితి;
    u32  roothub_portstatus[15];
} PACKED;

#నిర్వచించు OHCI_CTRL_CBSR  (3 << 0)
#నిర్వచించు OHCI_CTRL_PLE   (1 << 2)
#నిర్వచించు OHCI_CTRL_CLE   (1 << 4)
#నిర్వచించు OHCI_CTRL_BLE   (1 << 5)
#నిర్వచించు OHCI_CTRL_HCFS  (3 << 6)
#       define OHCI_USB_RESET   (0 << 6)
#       define OHCI_USB_OPER    (2 << 6)
#నిర్వచించు OHCI_CTRL_RWC   (1 << 9)

#నిర్వచించు OHCI_HCR        (1 << 0)
#నిర్వచించు OHCI_CLF        (1 << 1)
#నిర్వచించు OHCI_BLF        (1 << 2)

#నిర్వచించు OHCI_INTR_MIE   (1 << 31)

#నిర్వచించు RH_PS_CCS            0x00000001
#నిర్వచించు RH_PS_PES            0x00000002
#నిర్వచించు RH_PS_PSS            0x00000004
#నిర్వచించు RH_PS_POCI           0x00000008
#నిర్వచించు RH_PS_PRS            0x00000010
#నిర్వచించు RH_PS_PPS            0x00000100
#నిర్వచించు RH_PS_LSDA           0x00000200
#నిర్వచించు RH_PS_CSC            0x00010000
#నిర్వచించు RH_PS_PESC           0x00020000
#నిర్వచించు RH_PS_PSSC           0x00040000
#నిర్వచించు RH_PS_OCIC           0x00080000
#నిర్వచించు RH_PS_PRSC           0x00100000

#నిర్వచించు RH_HS_LPS            0x00000001
#నిర్వచించు RH_HS_OCI            0x00000002
#నిర్వచించు RH_HS_DRWE           0x00008000
#నిర్వచించు RH_HS_LPSC           0x00010000
#నిర్వచించు RH_HS_OCIC           0x00020000
#నిర్వచించు RH_HS_CRWE           0x80000000

#నిర్వచించు RH_B_DR         0x0000ffff
#నిర్వచించు RH_B_PPCM       0xffff0000

#నిర్వచించు RH_A_NDP        (0xff << 0)
#నిర్వచించు RH_A_PSM        (1 << 8)
#నిర్వచించు RH_A_NPS        (1 << 9)
#నిర్వచించు RH_A_DT         (1 << 10)
#నిర్వచించు RH_A_OCPM       (1 << 11)
#నిర్వచించు RH_A_NOCP       (1 << 12)
#నిర్వచించు RH_A_POTPGT     (0xff << 24)

#అయితే_ముగింపు // usb-ohci.h
