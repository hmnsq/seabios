#اگر_تعریف_نشده __PMM_H
#تعریف_کن __PMM_H

#ضمن "types.h" // u32

#تعریف_کن PMM_SIGNATURE 0x4d4d5024 // $PMM

بنية pmmheader {
    u32 signature;
    u8 نسخه_2;
    u8 درازی;
    u8 checksum;
    بنية segoff_s مدخل;
    u8 reserved[5];
} PACKED;

#تعریف_کن PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#پایان_اگر // pmm.h
