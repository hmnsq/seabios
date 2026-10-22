#నిర్వచితం_కాకపోతే __MPTABLE_H
#నిర్వచించు __MPTABLE_H

#అన్తర్భావ "types.h" // u32

#నిర్వచించు MPTABLE_SIGNATURE 0x5f504d5f  // "_MP_"

నిర్మాణం mptable_floating_s {
    u32 signature;
    u32 physaddr;
    u8 నిడివి;
    u8 spec_rev;
    u8 checksum;
    u8 feature1;
    u8 feature2;
    u8 reserved[3];
};

#నిర్వచించు MPCONFIG_SIGNATURE 0x504d4350  // "PCMP"

నిర్మాణం mptable_config_s {
    u32 signature;
    u16 నిడివి;
    u8 spec;
    u8 checksum;
    అక్షరం oemid[8];
    అక్షరం productid[12];
    u32 oemptr;
    u16 oemsize;
    u16 entrycount;
    u32 lapic;
    u16 exttable_నిడివి;
    u8 exttable_checksum;
    u8 reserved;
} PACKED;

#నిర్వచించు MPT_TYPE_CPU 0
#నిర్వచించు MPT_TYPE_BUS 1
#నిర్వచించు MPT_TYPE_IOAPIC 2
#నిర్వచించు MPT_TYPE_INTSRC 3
#నిర్వచించు MPT_TYPE_LOCAL_INT 4

నిర్మాణం mpt_cpu {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 cpuflag;
    u32 cpusignature;
    u32 featureflag;
    u32 reserved[2];
} PACKED;

నిర్మాణం mpt_bus {
    u8 type;
    u8 busid;
    అక్షరం bustype[6];
} PACKED;

నిర్మాణం mpt_ioapic {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 flags;
    u32 apicaddr;
} PACKED;

నిర్మాణం mpt_intsrc {
    u8 type;
    u8 irqtype;
    u16 irqflag;
    u8 srcbus;
    u8 srcbusirq;
    u8 dstapic;
    u8 dstirq;
} PACKED;

#అయితే_ముగింపు // mptable.h
