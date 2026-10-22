#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PIRTABLE_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PIRTABLE_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

ರಚನೆ link_info {
    u8 link;
    u16 bitmap;
} PACKED;

ರಚನೆ pir_ಸ್ಥಾನ {
    u8 bus;
    u8 dev;
    ರಚನೆ link_info links[4];
    u8 ಸ್ಥಾನ_nr;
    u8 reserved;
} PACKED;

ರಚನೆ pir_ಶೀರ್ಷಿಕೆ {
    u32 signature;
    u16 ಆವೃತ್ತಿ_2;
    u16 ಗಾತ್ರ_2;
    u8 ಮಾರ್ಗನಿರ್ದೇಶಕ_bus;
    u8 ಮಾರ್ಗನಿರ್ದೇಶಕ_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_ದತ್ತಾಂಶ;
    u8 reserved[11];
    u8 checksum;
    ರಚನೆ pir_ಸ್ಥಾನ slots[0];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು PIR_SIGNATURE 0x52495024 // $PIR

#ಆದರೆ_ಅಂತ್ಯ // pirtable.h
