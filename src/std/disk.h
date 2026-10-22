// Standard disk BIOS definitions.
#यदि_अपरिभाषितम् __DISK_H
#परिभाषय __DISK_H

#अन्तर्भाव "types.h" // u8

#परिभाषय DISK_RET_SUCCESS       0x00
#परिभाषय DISK_RET_EPARAM        0x01
#परिभाषय DISK_RET_EADDRNOTFOUND 0x02
#परिभाषय DISK_RET_EWRITEPROTECT 0x03
#परिभाषय DISK_RET_ECHANGED      0x06
#परिभाषय DISK_RET_EBOUNDARY     0x09
#परिभाषय DISK_RET_EBADTRACK     0x0c
#परिभाषय DISK_RET_ECONTROLLER   0x20
#परिभाषय DISK_RET_ETIMEOUT      0x80
#परिभाषय DISK_RET_ENOTLOCKED    0xb0
#परिभाषय DISK_RET_ELOCKED       0xb1
#परिभाषय DISK_RET_ENOTREMOVABLE 0xb2
#परिभाषय DISK_RET_ETOOMANYLOCKS 0xb4
#परिभाषय DISK_RET_ENOTREADY     0xAA


/****************************************************************
 * Interface structs
 ****************************************************************/

// Bios disk structures.
संरचना int13ext_s {
    u8  आकार;
    u8  reserved;
    u16 गणनम्;
    संरचना segoff_s दत्त;
    u64 lba;
} PACKED;

// DPTE definition
संरचना dpte_s {
    u16 iobase1;
    u16 iobase2;
    u8  उपसर्गः;
    u8  अप्रयुक्तम्;
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
संरचना int13dpt_s {
    u16 आकार;
    u16 infos;
    u32 cylinders;
    u32 heads;
    u32 spt;
    u64 sector_गणनम्;
    u16 blksize;
    संरचना segoff_s dpte;
    u16 कुञ्चिका;
    u8  dpi_दीर्घता;
    u8  reserved1;
    u16 reserved2;
    u8  host_bus[4];
    u8  iface_type[8];
    u64 iface_मार्गः;
    सङ्घ {
        संरचना {
            u64 device_मार्गः;
            u8  reserved3;
            u8  checksum;
        } phoenix;
        संरचना {
            u64 device_मार्गः[2];
            u8  reserved3;
            u8  checksum;
        } t13;
    };
} PACKED;

// Floppy info
संरचना fdpt_s {
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
संरचना floppy_dbt_s {
    u8 specify1;
    u8 specify2;
    u8 shutoff_ticks;
    u8 bps_संहिता;
    u8 sectors;
    u8 interblock_len;
    u8 दत्त_len;
    u8 gap_len;
    u8 fill_byte;
    u8 settle_time;
    u8 startup_time;
} PACKED;

संरचना floppy_ext_dbt_s {
    संरचना floppy_dbt_s dbt;
    // Extra fields
    u8 max_track;
    u8 दत्त_दरः;
    u8 drive_type;
} PACKED;


/****************************************************************
 * Master boot record
 ****************************************************************/

संरचना packed_chs_s {
    u8 heads;
    u8 sptcyl;
    u8 cyllow;
} PACKED;

संरचना partition_s {
    u8 स्थितिः;
    संरचना packed_chs_s first;
    u8 type;
    संरचना packed_chs_s चरमः;
    u32 lba;
    u32 गणनम्;
} PACKED;

संरचना mbr_s {
    u8 संहिता[440];
    // 0x01b8
    u32 diskseg;
    // 0x01bc
    u16 null;
    // 0x01be
    संरचना partition_s partitions[4];
    // 0x01fe
    u16 signature;
} PACKED;

#परिभाषय MBR_SIGNATURE 0xaa55


/****************************************************************
 * ElTorito CDROM interface
 ****************************************************************/

संरचना eltorito_s {
    u8 आकार;
    u8 media;
    u8 emulated_drive;
    u8 controller_सूचकांक;
    u32 ilba;
    u16 device_spec;
    u16 बफर_segment;
    u16 आरोपय_segment;
    u16 sector_गणनम्;
    संरचना packed_chs_s chs;
} PACKED;

#यदि_अन्तः // disk.h
