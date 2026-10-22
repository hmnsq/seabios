#اگر_غیر_تعریف_شدہ __PMM_H
#تعریف_کرو __PMM_H

#شامل "types.h" // u32

#تعریف_کرو PMM_SIGNATURE 0x4d4d5024 // $PMM

ساخت pmmheader {
    u32 signature;
    u8 نسخہ_2;
    u8 لمبائی;
    u8 checksum;
    ساخت segoff_s اندراج_2;
    u8 reserved[5];
} PACKED;

#تعریف_کرو PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#اگر_ختم // pmm.h
