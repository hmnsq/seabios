// Standard disk BIOS definitions.
#که_نه_تعریف_شوی __DISK_H
#تعریف_کړه __DISK_H

#ضمن "types.h" // u8

#تعریف_کړه DISK_RET_SUCCESS       0x00
#تعریف_کړه DISK_RET_EPARAM        0x01
#تعریف_کړه DISK_RET_EADDRNOTFOUND 0x02
#تعریف_کړه DISK_RET_EWRITEPROTECT 0x03
#تعریف_کړه DISK_RET_ECHANGED      0x06
#تعریف_کړه DISK_RET_EBOUNDARY     0x09
#تعریف_کړه DISK_RET_EBADTRACK     0x0c
#تعریف_کړه DISK_RET_ECONTROLLER   0x20
#تعریف_کړه DISK_RET_ETIMEOUT      0x80
#تعریف_کړه DISK_RET_ENOTLOCKED    0xb0
#تعریف_کړه DISK_RET_ELOCKED       0xb1
#تعریف_کړه DISK_RET_ENOTREMOVABLE 0xb2
#تعریف_کړه DISK_RET_ETOOMANYLOCKS 0xb4
#تعریف_کړه DISK_RET_ENOTREADY     0xAA


/****************************************************************
 * Interface structs
 ****************************************************************/

// Bios disk structures.
بنية int13ext_s {
    u8  اندازه;
    u8  reserved;
    u16 شمېر;
    بنية segoff_s معلومات;
    u64 lba;
} PACKED;

// DPTE definition
بنية dpte_s {
    u16 iobase1;
    u16 iobase2;
    u8  مختاړی;
    u8  ناکارول_شوی;
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
بنية int13dpt_s {
    u16 اندازه;
    u16 infos;
    u32 cylinders;
    u32 heads;
    u32 spt;
    u64 sector_شمېر;
    u16 blksize;
    بنية segoff_s dpte;
    u16 کلی;
    u8  dpi_اوږدتیا;
    u8  reserved1;
    u16 reserved2;
    u8  host_bus[4];
    u8  iface_type[8];
    u64 iface_لار;
    اتحاد {
        بنية {
            u64 device_لار;
            u8  reserved3;
            u8  checksum;
        } phoenix;
        بنية {
            u64 device_لار[2];
            u8  reserved3;
            u8  checksum;
        } t13;
    };
} PACKED;

// Floppy info
بنية fdpt_s {
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
بنية floppy_dbt_s {
    u8 specify1;
    u8 specify2;
    u8 shutoff_ticks;
    u8 bps_کوډ;
    u8 sectors;
    u8 interblock_len;
    u8 معلومات_len;
    u8 gap_len;
    u8 fill_byte;
    u8 settle_وخت;
    u8 startup_وخت;
} PACKED;

بنية floppy_ext_dbt_s {
    بنية floppy_dbt_s dbt;
    // Extra fields
    u8 max_track;
    u8 معلومات_شرح;
    u8 drive_type;
} PACKED;


/****************************************************************
 * Master boot record
 ****************************************************************/

بنية packed_chs_s {
    u8 heads;
    u8 sptcyl;
    u8 cyllow;
} PACKED;

بنية برخه_کول_s {
    u8 حالت;
    بنية packed_chs_s لومړی;
    u8 type;
    بنية packed_chs_s وروستی;
    u32 lba;
    u32 شمېر;
} PACKED;

بنية mbr_s {
    u8 کوډ[440];
    // 0x01b8
    u32 diskseg;
    // 0x01bc
    u16 null;
    // 0x01be
    بنية برخه_کول_s partitions[4];
    // 0x01fe
    u16 signature;
} PACKED;

#تعریف_کړه MBR_SIGNATURE 0xaa55


/****************************************************************
 * ElTorito CDROM interface
 ****************************************************************/

بنية eltorito_s {
    u8 اندازه;
    u8 media;
    u8 emulated_drive;
    u8 controller_شاخص;
    u32 ilba;
    u16 device_spec;
    u16 بفر_segment;
    u16 بارول_segment;
    u16 sector_شمېر;
    بنية packed_chs_s chs;
} PACKED;

#د_که_پای // disk.h
