#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __MPTABLE_H
#ವ್ಯಾಖ್ಯಾನಿಸು __MPTABLE_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#ವ್ಯಾಖ್ಯಾನಿಸು MPTABLE_SIGNATURE 0x5f504d5f  // "_MP_"

ರಚನೆ mptable_floating_s {
    u32 signature;
    u32 physaddr;
    u8 ಉದ್ದಳತೆ;
    u8 spec_rev;
    u8 checksum;
    u8 feature1;
    u8 feature2;
    u8 reserved[3];
};

#ವ್ಯಾಖ್ಯಾನಿಸು MPCONFIG_SIGNATURE 0x504d4350  // "PCMP"

ರಚನೆ mptable_config_s {
    u32 signature;
    u16 ಉದ್ದಳತೆ;
    u8 spec;
    u8 checksum;
    ಅಕ್ಷರ oemid[8];
    ಅಕ್ಷರ productid[12];
    u32 oemptr;
    u16 oemsize;
    u16 entrycount;
    u32 lapic;
    u16 exttable_ಉದ್ದಳತೆ;
    u8 exttable_checksum;
    u8 reserved;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು MPT_TYPE_CPU 0
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_TYPE_BUS 1
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_TYPE_IOAPIC 2
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_TYPE_INTSRC 3
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_TYPE_LOCAL_INT 4

ರಚನೆ mpt_cpu {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 cpuflag;
    u32 cpusignature;
    u32 featureflag;
    u32 reserved[2];
} PACKED;

ರಚನೆ mpt_bus {
    u8 type;
    u8 busid;
    ಅಕ್ಷರ bustype[6];
} PACKED;

ರಚನೆ mpt_ioapic {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 flags;
    u32 apicaddr;
} PACKED;

ರಚನೆ mpt_intsrc {
    u8 type;
    u8 irqtype;
    u16 irqflag;
    u8 srcbus;
    u8 srcbusirq;
    u8 dstapic;
    u8 dstirq;
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // mptable.h
