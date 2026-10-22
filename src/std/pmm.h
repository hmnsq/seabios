#جيڪڏهن_وصف_نه_ٿيل __PMM_H
#وصف_ڪريو __PMM_H

#ضمن "types.h" // u32

#وصف_ڪريو PMM_SIGNATURE 0x4d4d5024 // $PMM

بنية pmmheader {
    u32 signature;
    u8 نسخو_2;
    u8 ڊيگهائي;
    u8 checksum;
    بنية segoff_s داخلا;
    u8 reserved[5];
} PACKED;

#وصف_ڪريو PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#جيڪڏهن_پورو // pmm.h
