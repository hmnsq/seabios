#جے_بنا_تعریف __PMM_H
#تعریف_کرو __PMM_H

#ضمن "types.h" // u32

#تعریف_کرو PMM_SIGNATURE 0x4d4d5024 // $PMM

بنية pmmheader {
    u32 signature;
    u8 نسخہ_2;
    u8 لمائی;
    u8 checksum;
    بنية segoff_s اندراج;
    u8 reserved[5];
} PACKED;

#تعریف_کرو PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#جے_انت // pmm.h
