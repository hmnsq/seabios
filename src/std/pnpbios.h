#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PNPHEADER_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PNPHEADER_H

#ವ್ಯಾಖ್ಯಾನಿಸು PNP_SIGNATURE 0x506e5024 // $PnP

ರಚನೆ pnpheader {
    u32 signature;
    u8 ಆವೃತ್ತಿ_2;
    u8 ಉದ್ದಳತೆ;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_ತಳ;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು FUNCTION_NOT_SUPPORTED 0x82

#ಆದರೆ_ಅಂತ್ಯ // pnpheader.h
