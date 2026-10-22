#ifndef __PMM_H
#define __PMM_H

#অন্তর্ভাব "types.h" // u32

#define PMM_SIGNATURE 0x4d4d5024 // $PMM

কাঠামো pmmheader {
    u32 signature;
    u8 version;
    u8 দৈর্ঘ্য;
    u8 checksum;
    কাঠামো segoff_s entry;
    u8 reserved[5];
} PACKED;

#define PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#endif // pmm.h
