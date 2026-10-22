#که_نه_تعریف_شوی __PMM_H
#تعریف_کړه __PMM_H

#ضمن "types.h" // u32

#تعریف_کړه PMM_SIGNATURE 0x4d4d5024 // $PMM

بنية pmmheader {
    u32 signature;
    u8 نسخه_2;
    u8 اوږدتیا;
    u8 checksum;
    بنية segoff_s ثبت;
    u8 reserved[5];
} PACKED;

#تعریف_کړه PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#د_که_پای // pmm.h
