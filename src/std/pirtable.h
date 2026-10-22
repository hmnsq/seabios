#నిర్వచితం_కాకపోతే __PIRTABLE_H
#నిర్వచించు __PIRTABLE_H

#అన్తర్భావ "types.h" // u32

నిర్మాణం link_info {
    u8 link;
    u16 bitmap;
} PACKED;

నిర్మాణం pir_స్థానం {
    u8 bus;
    u8 dev;
    నిర్మాణం link_info links[4];
    u8 స్థానం_nr;
    u8 reserved;
} PACKED;

నిర్మాణం pir_శీర్షిక {
    u32 signature;
    u16 సంస్కరణ_2;
    u16 కొలత;
    u8 మార్గనిర్దేశకం_bus;
    u8 మార్గనిర్దేశకం_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_దత్తాంశం;
    u8 reserved[11];
    u8 checksum;
    నిర్మాణం pir_స్థానం slots[0];
} PACKED;

#నిర్వచించు PIR_SIGNATURE 0x52495024 // $PIR

#అయితే_ముగింపు // pirtable.h
