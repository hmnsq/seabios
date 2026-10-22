#यदि_अपरिभाषितम् __MPTABLE_H
#परिभाषय __MPTABLE_H

#अन्तर्भाव "types.h" // u32

#परिभाषय MPTABLE_SIGNATURE 0x5f504d5f  // "_MP_"

संरचना mptable_floating_s {
    u32 signature;
    u32 physaddr;
    u8 दीर्घता;
    u8 spec_rev;
    u8 checksum;
    u8 feature1;
    u8 feature2;
    u8 reserved[3];
};

#परिभाषय MPCONFIG_SIGNATURE 0x504d4350  // "PCMP"

संरचना mptable_config_s {
    u32 signature;
    u16 दीर्घता;
    u8 spec;
    u8 checksum;
    वर्ण oemid[8];
    वर्ण productid[12];
    u32 oemptr;
    u16 oemsize;
    u16 entrycount;
    u32 lapic;
    u16 exttable_दीर्घता;
    u8 exttable_checksum;
    u8 reserved;
} PACKED;

#परिभाषय MPT_TYPE_CPU 0
#परिभाषय MPT_TYPE_BUS 1
#परिभाषय MPT_TYPE_IOAPIC 2
#परिभाषय MPT_TYPE_INTSRC 3
#परिभाषय MPT_TYPE_LOCAL_INT 4

संरचना mpt_cpu {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 cpuflag;
    u32 cpusignature;
    u32 featureflag;
    u32 reserved[2];
} PACKED;

संरचना mpt_bus {
    u8 type;
    u8 busid;
    वर्ण bustype[6];
} PACKED;

संरचना mpt_ioapic {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 flags;
    u32 apicaddr;
} PACKED;

संरचना mpt_intsrc {
    u8 type;
    u8 irqtype;
    u16 irqflag;
    u8 srcbus;
    u8 srcbusirq;
    u8 dstapic;
    u8 dstirq;
} PACKED;

#यदि_अन्तः // mptable.h
