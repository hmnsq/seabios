#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __MPTABLE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __MPTABLE_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPTABLE_SIGNATURE 0x5f504d5f  // "_MP_"

ਬਣਤਰ mptable_floating_s {
    u32 signature;
    u32 physaddr;
    u8 ਲੰਮਾਈ;
    u8 spec_rev;
    u8 checksum;
    u8 feature1;
    u8 feature2;
    u8 reserved[3];
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPCONFIG_SIGNATURE 0x504d4350  // "PCMP"

ਬਣਤਰ mptable_config_s {
    u32 signature;
    u16 ਲੰਮਾਈ;
    u8 spec;
    u8 checksum;
    ਅੱਖਰ oemid[8];
    ਅੱਖਰ productid[12];
    u32 oemptr;
    u16 oemsize;
    u16 entrycount;
    u32 lapic;
    u16 exttable_ਲੰਮਾਈ;
    u8 exttable_checksum;
    u8 reserved;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_TYPE_CPU 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_TYPE_BUS 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_TYPE_IOAPIC 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_TYPE_INTSRC 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_TYPE_LOCAL_INT 4

ਬਣਤਰ mpt_cpu {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 cpuflag;
    u32 cpusignature;
    u32 featureflag;
    u32 reserved[2];
} PACKED;

ਬਣਤਰ mpt_bus {
    u8 type;
    u8 busid;
    ਅੱਖਰ bustype[6];
} PACKED;

ਬਣਤਰ mpt_ioapic {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 flags;
    u32 apicaddr;
} PACKED;

ਬਣਤਰ mpt_intsrc {
    u8 type;
    u8 irqtype;
    u16 irqflag;
    u8 srcbus;
    u8 srcbusirq;
    u8 dstapic;
    u8 dstirq;
} PACKED;

#ਜੇ_ਅੰਤ // mptable.h
