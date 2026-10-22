// Standard disk BIOS definitions.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __DISK_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __DISK_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_SUCCESS       0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_EPARAM        0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_EADDRNOTFOUND 0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_EWRITEPROTECT 0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ECHANGED      0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_EBOUNDARY     0x09
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_EBADTRACK     0x0c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ECONTROLLER   0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ETIMEOUT      0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ENOTLOCKED    0xb0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ELOCKED       0xb1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ENOTREMOVABLE 0xb2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ETOOMANYLOCKS 0xb4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_RET_ENOTREADY     0xAA


/****************************************************************
 * Interface structs
 ****************************************************************/

// Bios disk structures.
ਬਣਤਰ int13ext_s {
    u8  ਮਾਪ;
    u8  reserved;
    u16 ਗਿਣਤੀ_2;
    ਬਣਤਰ segoff_s ਅੰਕੜੇ;
    u64 lba;
} PACKED;

// DPTE definition
ਬਣਤਰ dpte_s {
    u16 iobase1;
    u16 iobase2;
    u8  ਅਗੇਤਰ;
    u8  ਅਣਵਰਤਿਆ;
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
ਬਣਤਰ int13dpt_s {
    u16 ਮਾਪ;
    u16 infos;
    u32 cylinders;
    u32 heads;
    u32 spt;
    u64 sector_ਗਿਣਤੀ;
    u16 blksize;
    ਬਣਤਰ segoff_s dpte;
    u16 ਕੁੰਜੀ;
    u8  dpi_ਲੰਮਾਈ;
    u8  reserved1;
    u16 reserved2;
    u8  host_bus[4];
    u8  iface_type[8];
    u64 iface_ਰਸਤਾ;
    ਸੰਘ {
        ਬਣਤਰ {
            u64 device_ਰਸਤਾ;
            u8  reserved3;
            u8  checksum;
        } phoenix;
        ਬਣਤਰ {
            u64 device_ਰਸਤਾ[2];
            u8  reserved3;
            u8  checksum;
        } t13;
    };
} PACKED;

// Floppy info
ਬਣਤਰ fdpt_s {
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
ਬਣਤਰ floppy_dbt_s {
    u8 specify1;
    u8 specify2;
    u8 shutoff_ticks;
    u8 bps_ਕੋਡ;
    u8 sectors;
    u8 interblock_len;
    u8 ਅੰਕੜੇ_len;
    u8 gap_len;
    u8 fill_byte;
    u8 settle_ਸਮਾਂ;
    u8 startup_ਸਮਾਂ;
} PACKED;

ਬਣਤਰ floppy_ext_dbt_s {
    ਬਣਤਰ floppy_dbt_s dbt;
    // Extra fields
    u8 max_track;
    u8 ਅੰਕੜੇ_ਦਰ;
    u8 drive_type;
} PACKED;


/****************************************************************
 * Master boot record
 ****************************************************************/

ਬਣਤਰ packed_chs_s {
    u8 heads;
    u8 sptcyl;
    u8 cyllow;
} PACKED;

ਬਣਤਰ ਹਿੱਸੇ_ਕਰੋ_s {
    u8 ਸਥਿਤੀ;
    ਬਣਤਰ packed_chs_s ਪਹਿਲਾ;
    u8 type;
    ਬਣਤਰ packed_chs_s ਆਖਰੀ;
    u32 lba;
    u32 ਗਿਣਤੀ_2;
} PACKED;

ਬਣਤਰ mbr_s {
    u8 ਕੋਡ[440];
    // 0x01b8
    u32 diskseg;
    // 0x01bc
    u16 null;
    // 0x01be
    ਬਣਤਰ ਹਿੱਸੇ_ਕਰੋ_s partitions[4];
    // 0x01fe
    u16 signature;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MBR_SIGNATURE 0xaa55


/****************************************************************
 * ElTorito CDROM interface
 ****************************************************************/

ਬਣਤਰ eltorito_s {
    u8 ਮਾਪ;
    u8 media;
    u8 emulated_drive;
    u8 controller_ਸੂਚਕਾਂਕ;
    u32 ilba;
    u16 device_spec;
    u16 ਬਫਰ_segment;
    u16 ਲੱਦੋ_segment;
    u16 sector_ਗਿਣਤੀ;
    ਬਣਤਰ packed_chs_s chs;
} PACKED;

#ਜੇ_ਅੰਤ // disk.h
