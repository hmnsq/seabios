// Standard disk BIOS definitions.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __DISK_H
#ವ್ಯಾಖ್ಯಾನಿಸು __DISK_H

#ಅನ್ತರ್ಭಾವ "types.h" // u8

#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_SUCCESS       0x00
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_EPARAM        0x01
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_EADDRNOTFOUND 0x02
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_EWRITEPROTECT 0x03
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ECHANGED      0x06
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_EBOUNDARY     0x09
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_EBADTRACK     0x0c
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ECONTROLLER   0x20
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ETIMEOUT      0x80
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ENOTLOCKED    0xb0
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ELOCKED       0xb1
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ENOTREMOVABLE 0xb2
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ETOOMANYLOCKS 0xb4
#ವ್ಯಾಖ್ಯಾನಿಸು DISK_RET_ENOTREADY     0xAA


/****************************************************************
 * Interface structs
 ****************************************************************/

// Bios disk structures.
ರಚನೆ int13ext_s {
    u8  ಗಾತ್ರ_2;
    u8  reserved;
    u16 ಎಣಿಕೆ_2;
    ರಚನೆ segoff_s ದತ್ತಾಂಶ;
    u64 lba;
} PACKED;

// DPTE definition
ರಚನೆ dpte_s {
    u16 iobase1;
    u16 iobase2;
    u8  ಪೂರ್ವಪ್ರತ್ಯಯ;
    u8  ಬಳಸದ;
    u8  irq;
    u8  blkcount;
    u8  dma;
    u8  pio;
    u16 options;
    u16 reserved;
    u8  revision;
    u8  checksum;
};

// Disk Physical Table definition
ರಚನೆ int13dpt_s {
    u16 ಗಾತ್ರ_2;
    u16 infos;
    u32 cylinders;
    u32 heads;
    u32 spt;
    u64 sector_ಎಣಿಕೆ;
    u16 blksize;
    ರಚನೆ segoff_s dpte;
    u16 ಕೀಲಿ;
    u8  dpi_ಉದ್ದಳತೆ;
    u8  reserved1;
    u16 reserved2;
    u8  host_bus[4];
    u8  iface_type[8];
    u64 iface_ಮಾರ್ಗ;
    ಒಕ್ಕೂಟ {
        ರಚನೆ {
            u64 device_ಮಾರ್ಗ;
            u8  reserved3;
            u8  checksum;
        } phoenix;
        ರಚನೆ {
            u64 device_ಮಾರ್ಗ[2];
            u8  reserved3;
            u8  checksum;
        } t13;
    };
} PACKED;

// Floppy info
ರಚನೆ fdpt_s {
    u16 cylinders;
    u8 heads;
    u8 a0h_signature;
    u8 phys_sectors;
    u16 precompensation;
    u8 reserved;
    u8 drive_control_byte;
    u16 phys_cylinders;
    u8 phys_heads;
    u16 landing_zone;
    u8 sectors;
    u8 checksum;
} PACKED;

// Floppy "Disk Base Table"
ರಚನೆ floppy_dbt_s {
    u8 specify1;
    u8 specify2;
    u8 shutoff_ticks;
    u8 bps_ಸಂಕೇತ;
    u8 sectors;
    u8 interblock_len;
    u8 ದತ್ತಾಂಶ_len;
    u8 gap_len;
    u8 fill_byte;
    u8 settle_ಸಮಯ;
    u8 startup_ಸಮಯ;
} PACKED;

ರಚನೆ floppy_ext_dbt_s {
    ರಚನೆ floppy_dbt_s dbt;
    // Extra fields
    u8 max_track;
    u8 ದತ್ತಾಂಶ_ದರ;
    u8 drive_type;
} PACKED;


/****************************************************************
 * Master boot record
 ****************************************************************/

ರಚನೆ packed_chs_s {
    u8 heads;
    u8 sptcyl;
    u8 cyllow;
} PACKED;

ರಚನೆ ವಿಭಜನೆ_s {
    u8 ಸ್ಥಿತಿ_2;
    ರಚನೆ packed_chs_s ಮೊದಲ;
    u8 type;
    ರಚನೆ packed_chs_s ಕೊನೆಯ;
    u32 lba;
    u32 ಎಣಿಕೆ_2;
} PACKED;

ರಚನೆ mbr_s {
    u8 ಸಂಕೇತ_2[440];
    // 0x01b8
    u32 diskseg;
    // 0x01bc
    u16 null;
    // 0x01be
    ರಚನೆ ವಿಭಜನೆ_s partitions[4];
    // 0x01fe
    u16 signature;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು MBR_SIGNATURE 0xaa55


/****************************************************************
 * ElTorito CDROM interface
 ****************************************************************/

ರಚನೆ eltorito_s {
    u8 ಗಾತ್ರ_2;
    u8 media;
    u8 emulated_drive;
    u8 controller_ಸೂಚ್ಯಂಕ;
    u32 ilba;
    u16 device_spec;
    u16 ಬಫರ್_segment;
    u16 ತುಂಬು_segment;
    u16 sector_ಎಣಿಕೆ;
    ರಚನೆ packed_chs_s chs;
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // disk.h
