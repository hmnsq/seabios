#ifndef __PMM_H
#define __PMM_H

#ضمن "types.h" // u32

#define PMM_SIGNATURE 0x4d4d5024 // $PMM

بنية pmmheader {
    u32 signature;
    u8 version;
    u8 length;
    u8 checksum;
    بنية segoff_s entry;
    u8 reserved[5];
} PACKED;

#define PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#endif // pmm.h
