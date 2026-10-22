#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PMM_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PMM_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#ವ್ಯಾಖ್ಯಾನಿಸು PMM_SIGNATURE 0x4d4d5024 // $PMM

ರಚನೆ pmmheader {
    u32 signature;
    u8 ಆವೃತ್ತಿ_2;
    u8 ಉದ್ದಳತೆ;
    u8 checksum;
    ರಚನೆ segoff_s ನಮೂದು;
    u8 reserved[5];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#ಆದರೆ_ಅಂತ್ಯ // pmm.h
