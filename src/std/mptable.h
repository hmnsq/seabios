#अगर_अघोषित __MPTABLE_H
#घोषणा __MPTABLE_H

#समावेश "types.h" // u32

#घोषणा MPTABLE_SIGNATURE 0x5f504d5f  // "_MP_"

काष्ठा mptable_floating_s {
    u32 signature;
    u32 physaddr;
    u8 लंबाई;
    u8 spec_rev;
    u8 checksum;
    u8 feature1;
    u8 feature2;
    u8 reserved[3];
};

#घोषणा MPCONFIG_SIGNATURE 0x504d4350  // "PCMP"

काष्ठा mptable_config_s {
    u32 signature;
    u16 लंबाई;
    u8 spec;
    u8 checksum;
    अक्षर oemid[8];
    अक्षर productid[12];
    u32 oemptr;
    u16 oemsize;
    u16 entrycount;
    u32 lapic;
    u16 exttable_लंबाई;
    u8 exttable_checksum;
    u8 reserved;
} PACKED;

#घोषणा MPT_TYPE_CPU 0
#घोषणा MPT_TYPE_BUS 1
#घोषणा MPT_TYPE_IOAPIC 2
#घोषणा MPT_TYPE_INTSRC 3
#घोषणा MPT_TYPE_LOCAL_INT 4

काष्ठा mpt_cpu {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 cpuflag;
    u32 cpusignature;
    u32 featureflag;
    u32 reserved[2];
} PACKED;

काष्ठा mpt_bus {
    u8 type;
    u8 busid;
    अक्षर bustype[6];
} PACKED;

काष्ठा mpt_ioapic {
    u8 type;
    u8 apicid;
    u8 apicver;
    u8 flags;
    u32 apicaddr;
} PACKED;

काष्ठा mpt_intsrc {
    u8 type;
    u8 irqtype;
    u16 irqflag;
    u8 srcbus;
    u8 srcbusirq;
    u8 dstapic;
    u8 dstirq;
} PACKED;

#पूर्ण_अगर // mptable.h
