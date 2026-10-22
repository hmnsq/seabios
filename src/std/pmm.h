#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PMM_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PMM_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PMM_SIGNATURE 0x4d4d5024 // $PMM

ਬਣਤਰ pmmheader {
    u32 signature;
    u8 ਸੰਸਕਰਣ_2;
    u8 ਲੰਮਾਈ;
    u8 checksum;
    ਬਣਤਰ segoff_s ਇੰਦਰਾਜ;
    u8 reserved[5];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#ਜੇ_ਅੰਤ // pmm.h
