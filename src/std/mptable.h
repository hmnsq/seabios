#اگر_غیر_تعریف_شدہ __MPTABLE_H
#تعریف_کرو __MPTABLE_H

#شامل "types.h" // u32

#تعریف_کرو MPTABLE_SIGNATURE 0x5f504d5f  // "_MP_"

ساخت mptable_floating_s {
    u32 signature;
    u32 physaddr;
    u8 لمبائی;
    u8 spec_rev;
    u8 checksum;
    u8 feature1;
    u8 feature2;
    u8 reserved[3];
};

#تعریف_کرو MPCONFIG_SIGNATURE 0x504d4350  // "PCMP"

ساخت mptable_config_s {
    u32 signature;
    u16 لمبائی;
    u8 spec;
    u8 checksum;
    حرف oemid[8];
    حرف productid[12];
    u32 oemptr;
    u16 oemsize;
    u16 entrycount;
    u32 lapic;
    u16 exttable_لمبائی;
    u8 exttable_checksum;
    u8 reserved;
} PACKED;

#تعریف_کرو MPT_TYPE_CPU 0
#تعریف_کرو MPT_TYPE_BUS 1
#تعریف_کرو MPT_TYPE_IOAPIC 2
#تعریف_کرو MPT_TYPE_INTSRC 3
#تعریف_کرو MPT_TYPE_LOCAL_INT 4

ساخت mpt_cpu {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 cpuflag;
    u32 cpusignature;
    u32 featureflag;
    u32 reserved[2];
} PACKED;

ساخت mpt_bus {
    u8 type;
    u8 busid;
    حرف bustype[6];
} PACKED;

ساخت mpt_ioapic {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 flags;
    u32 apicaddr;
} PACKED;

ساخت mpt_intsrc {
    u8 type;
    u8 irqtype;
    u16 irqflag;
    u8 srcbus;
    u8 srcbusirq;
    u8 dstapic;
    u8 dstirq;
} PACKED;

#اگر_ختم // mptable.h
