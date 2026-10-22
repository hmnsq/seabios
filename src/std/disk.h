// Standard disk BIOS definitions.
#నిర్వచితం_కాకపోతే __DISK_H
#నిర్వచించు __DISK_H

#అన్తర్భావ "types.h" // u8

#నిర్వచించు DISK_RET_SUCCESS       0x00
#నిర్వచించు DISK_RET_EPARAM        0x01
#నిర్వచించు DISK_RET_EADDRNOTFOUND 0x02
#నిర్వచించు DISK_RET_EWRITEPROTECT 0x03
#నిర్వచించు DISK_RET_ECHANGED      0x06
#నిర్వచించు DISK_RET_EBOUNDARY     0x09
#నిర్వచించు DISK_RET_EBADTRACK     0x0c
#నిర్వచించు DISK_RET_ECONTROLLER   0x20
#నిర్వచించు DISK_RET_ETIMEOUT      0x80
#నిర్వచించు DISK_RET_ENOTLOCKED    0xb0
#నిర్వచించు DISK_RET_ELOCKED       0xb1
#నిర్వచించు DISK_RET_ENOTREMOVABLE 0xb2
#నిర్వచించు DISK_RET_ETOOMANYLOCKS 0xb4
#నిర్వచించు DISK_RET_ENOTREADY     0xAA


/****************************************************************
 * Interface structs
 ****************************************************************/

// Bios disk structures.
నిర్మాణం int13ext_s {
    u8  కొలత;
    u8  reserved;
    u16 లెక్క;
    నిర్మాణం segoff_s దత్తాంశం;
    u64 lba;
} PACKED;

// DPTE definition
నిర్మాణం dpte_s {
    u16 iobase1;
    u16 iobase2;
    u8  ఉపసర్గ;
    u8  వాడని;
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
నిర్మాణం int13dpt_s {
    u16 కొలత;
    u16 infos;
    u32 cylinders;
    u32 heads;
    u32 spt;
    u64 sector_లెక్క;
    u16 blksize;
    నిర్మాణం segoff_s dpte;
    u16 కీలకం;
    u8  dpi_నిడివి;
    u8  reserved1;
    u16 reserved2;
    u8  host_bus[4];
    u8  iface_type[8];
    u64 iface_మార్గం;
    సంఘం {
        నిర్మాణం {
            u64 device_మార్గం;
            u8  reserved3;
            u8  checksum;
        } phoenix;
        నిర్మాణం {
            u64 device_మార్గం[2];
            u8  reserved3;
            u8  checksum;
        } t13;
    };
} PACKED;

// Floppy info
నిర్మాణం fdpt_s {
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
నిర్మాణం floppy_dbt_s {
    u8 specify1;
    u8 specify2;
    u8 shutoff_ticks;
    u8 bps_సంకేతం;
    u8 sectors;
    u8 interblock_len;
    u8 దత్తాంశం_len;
    u8 gap_len;
    u8 fill_byte;
    u8 settle_సమయం;
    u8 startup_సమయం;
} PACKED;

నిర్మాణం floppy_ext_dbt_s {
    నిర్మాణం floppy_dbt_s dbt;
    // Extra fields
    u8 max_track;
    u8 దత్తాంశం_రేటు;
    u8 drive_type;
} PACKED;


/****************************************************************
 * Master boot record
 ****************************************************************/

నిర్మాణం packed_chs_s {
    u8 heads;
    u8 sptcyl;
    u8 cyllow;
} PACKED;

నిర్మాణం విభజన_s {
    u8 స్థితి_2;
    నిర్మాణం packed_chs_s మొదటి;
    u8 type;
    నిర్మాణం packed_chs_s చివరి;
    u32 lba;
    u32 లెక్క;
} PACKED;

నిర్మాణం mbr_s {
    u8 సంకేతం_2[440];
    // 0x01b8
    u32 diskseg;
    // 0x01bc
    u16 null;
    // 0x01be
    నిర్మాణం విభజన_s partitions[4];
    // 0x01fe
    u16 signature;
} PACKED;

#నిర్వచించు MBR_SIGNATURE 0xaa55


/****************************************************************
 * ElTorito CDROM interface
 ****************************************************************/

నిర్మాణం eltorito_s {
    u8 కొలత;
    u8 media;
    u8 emulated_drive;
    u8 controller_సూచీ;
    u32 ilba;
    u16 device_spec;
    u16 బఫరు_segment;
    u16 ఎక్కించు_segment;
    u16 sector_లెక్క;
    నిర్మాణం packed_chs_s chs;
} PACKED;

#అయితే_ముగింపు // disk.h
