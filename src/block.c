// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "block.h" // process_op
#ಅನ್ತರ್ಭಾವ "hw/ata.h" // process_ata_op
#ಅನ್ತರ್ಭಾವ "hw/ahci.h" // process_ahci_op
#ಅನ್ತರ್ಭಾವ "hw/esp-scsi.h" // esp_scsi_process_op
#ಅನ್ತರ್ಭಾವ "hw/lsi-scsi.h" // lsi_scsi_process_op
#ಅನ್ತರ್ಭಾವ "hw/megasas.h" // megasas_process_op
#ಅನ್ತರ್ಭಾವ "hw/mpt-scsi.h" // mpt_scsi_process_op
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_bdf_to_bus
#ಅನ್ತರ್ಭಾವ "hw/pvscsi.h" // pvscsi_process_op
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // rtc_read
#ಅನ್ತರ್ಭಾವ "hw/usb-msc.h" // usb_process_op
#ಅನ್ತರ್ಭಾವ "hw/usb-uas.h" // uas_process_op
#ಅನ್ತರ್ಭಾವ "hw/virtio-blk.h" // process_virtio_blk_op
#ಅನ್ತರ್ಭಾವ "hw/virtio-scsi.h" // virtio_scsi_process_op
#ಅನ್ತರ್ಭಾವ "hw/nvme.h" // nvme_process_op
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_low
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // call32
#ಅನ್ತರ್ಭಾವ "std/disk.h" // struct dpte_s
#ಅನ್ತರ್ಭಾವ "string.h" // checksum
#ಅನ್ತರ್ಭಾವ "util.h" // process_floppy_op

u8 Floppy_ಎಣಿಕೆ VARFSEG;
u8 CDCount;
ಸಂರಚನಾ drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

ಸಂರಚನಾ drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    ಯದಿ (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

ಪೂರ್ಣಾಙ್ಕ getDriveId(u8 exttype, ಸಂರಚನಾ drive_s *drive)
{
    ASSERT32FLAT();
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        ಯದಿ (getDrive(exttype, i) == drive)
            ಪ್ರತ್ಯಾಗಮ i;
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಪೂರ್ಣಾಙ್ಕ create_bounce_buf(ಶೂನ್ಯ)
{
    ಯದಿ (bounce_buf_fl)
        ಪ್ರತ್ಯಾಗಮ 0;

    u8 *buf = malloc_ಕೆಳಗಿನ(CDROM_SECTOR_SIZE);
    ಯದಿ (!buf) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    bounce_buf_fl = buf;
    ಪ್ರತ್ಯಾಗಮ 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
host_lchs_supplied(ಸಂರಚನಾ drive_s *drive)
{
    ಪ್ರತ್ಯಾಗಮ (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

ಸ್ಥಿರ u8
get_translation(ಸಂರಚನಾ drive_s *drive)
{
    ಯದಿ (host_lchs_supplied(drive))
        ಪ್ರತ್ಯಾಗಮ TRANSLATION_HOST;
    u8 type = drive->type;
    ಯದಿ (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        ಪ್ರತ್ಯಾಗಮ translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    ಯದಿ (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        ಪ್ರತ್ಯಾಗಮ TRANSLATION_LBA;

    ಯದಿ (cylinders <= 1024 && heads <= 16 && spt <= 63)
        ಪ್ರತ್ಯಾಗಮ TRANSLATION_NONE;
    ಯದಿ (cylinders * heads <= 131072)
        ಪ್ರತ್ಯಾಗಮ TRANSLATION_LARGE;
    ಪ್ರತ್ಯಾಗಮ TRANSLATION_LBA;
}

ಸ್ಥಿರ ಶೂನ್ಯ
setup_translation(ಸಂರಚನಾ drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ಧ್ರುವ ವರ್ಣ *desc = ರಿಕ್ತ;

    ವಿಕಲ್ಪ (translation) {
    ಸ್ವಭಾವ:
    ಪಕ್ಷ TRANSLATION_NONE:
        desc = "none";
        ಭಙ್ಗ;
    ಪಕ್ಷ TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        ಯದಿ (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            ಭಙ್ಗ;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        ಯದಿ (heads>128)
            heads = 255;
        ಅನ್ಯಥಾ ಯದಿ (heads>64)
            heads = 128;
        ಅನ್ಯಥಾ ಯದಿ (heads>32)
            heads = 64;
        ಅನ್ಯಥಾ ಯದಿ (heads>16)
            heads = 32;
        ಅನ್ಯಥಾ
            heads = 16;
        cylinders = sect / heads;
        ಭಙ್ಗ;
    ಪಕ್ಷ TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        ಯದಿ (heads==16) {
            ಯದಿ (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    ಪಕ್ಷ TRANSLATION_LARGE:
        ಯದಿ (translation == TRANSLATION_LARGE)
            desc = "large";
        ಯಾವತ್ (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            ಯದಿ (heads > 127)
                ಭಙ್ಗ;
        }
        ಭಙ್ಗ;
    ಪಕ್ಷ TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        ಭಙ್ಗ;
    }
    // clip to 1024 cylinders in lchs
    ಯದಿ (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%u\n"
            , drive
            , drive->pchs.cylinder, drive->pchs.head, drive->pchs.sector
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    drive->lchs.head = heads;
    drive->lchs.cylinder = cylinders;
    drive->lchs.sector = spt;
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
ಸ್ಥಿರ ಶೂನ್ಯ
fill_fdpt(ಸಂರಚನಾ drive_s *drive, ಪೂರ್ಣಾಙ್ಕ hdid)
{
    ಯದಿ (hdid > 1)
        ಪ್ರತ್ಯಾಗಮ;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    ಸಂರಚನಾ fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    ಯದಿ (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, ಪರಿಮಾಣ(*fdpt));
    }

    ಯದಿ (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 ಸಂರಚನಾ extended_bios_data_area_s, fdpt[0])));
    ಅನ್ಯಥಾ
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 ಸಂರಚನಾ extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
ಸ್ಥಿರ ಶೂನ್ಯ
add_drive(ಸಂರಚನಾ drive_s **idmap, u8 *ಎಣಿಕೆ, ಸಂರಚನಾ drive_s *drive)
{
    ಯದಿ (*ಎಣಿಕೆ >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    idmap[*ಎಣಿಕೆ] = drive;
    *ಎಣಿಕೆ = *ಎಣಿಕೆ + 1;
}

// Map a hard drive
ಶೂನ್ಯ
map_hd_drive(ಸಂರಚನಾ drive_s *drive)
{
    ASSERT32FLAT();
    ಸಂರಚನಾ bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    ಪೂರ್ಣಾಙ್ಕ hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
ಶೂನ್ಯ
map_cd_drive(ಸಂರಚನಾ drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
ಶೂನ್ಯ
map_floppy_drive(ಸಂರಚನಾ drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_ಎಣಿಕೆ, drive);

    // Update equipment word bits for floppy
    ಯದಿ (Floppy_ಎಣಿಕೆ == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } ಅನ್ಯಥಾ ಯದಿ (Floppy_ಎಣಿಕೆ >= 2) {
        // 2 drives, ready for boot
        set_equipment_flags(0x41, 0x41);
        SET_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * Extended Disk Drive (EDD) get drive parameters
 ****************************************************************/

// flags for bus_iface field in fill_generic_edd()
#define EDD_ISA        0x01
#define EDD_PCI        0x02
#define EDD_BUS_MASK   0x0f
#define EDD_ATA        0x10
#define EDD_SCSI       0x20
#define EDD_IFACE_MASK 0xf0

// Fill in EDD info
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
fill_generic_edd(ಸಂರಚನಾ segoff_s edd, ಸಂರಚನಾ drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    ಸಂರಚನಾ int13dpt_s *param_far = (ಶೂನ್ಯ*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    ಯದಿ (size < 26)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.head);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , size, type, npc, nph, nps, (u32)lba, blksize);

    SET_FARVAR(seg, param_far->size, 26);
    ಯದಿ (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } ಅನ್ಯಥಾ {
        ಯದಿ (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } ಅನ್ಯಥಾ {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_ಎಣಿಕೆ, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    ಯದಿ (size < 30 || !dpte_so)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    ಯದಿ (size < 66 || !bus_iface)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->ಕೀಲಿ, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    ಧ್ರುವ ವರ್ಣ *host_bus = "ISA ";
    ಯದಿ ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        ಯದಿ (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , ಪರಿಮಾಣ(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    ಧ್ರುವ ವರ್ಣ *iface_type = "ATA     ";
    ಯದಿ ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , ಪರಿಮಾಣ(param_far->iface_type));
    ಯದಿ (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (ಶೂನ್ಯ*)param_far+30, 43));
    } ಅನ್ಯಥಾ {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (ಶೂನ್ಯ*)param_far+30, 35));
    }

    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
ಸ್ಥಿರ u32
edd_pci_path(u16 bdf, u8 channel)
{
    ಪ್ರತ್ಯಾಗಮ (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

ಸಂರಚನಾ dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
fill_ata_edd(ಸಂರಚನಾ segoff_s edd, ಸಂರಚನಾ drive_s *drive_gf)
{
    ಯದಿ (!CONFIG_ATA)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EPARAM;

    // Fill in dpte
    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        drive_gf, ಸಂರಚನಾ atadrive_s, drive);
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    ಪೂರ್ಣಾಙ್ಕ bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    ಯದಿ (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        ಯದಿ ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            ಯದಿ (translation == TRANSLATION_LBA)
                options |= 1<<9;
            ಯದಿ (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } ಅನ್ಯಥಾ {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    ಯದಿ (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_ಕೆಳಗಿನ(DefaultDPTE.iobase1, iobase1);
    SET_ಕೆಳಗಿನ(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_ಕೆಳಗಿನ(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_ಕೆಳಗಿನ(DefaultDPTE.unused, 0xcb);
    SET_ಕೆಳಗಿನ(DefaultDPTE.irq, irq);
    SET_ಕೆಳಗಿನ(DefaultDPTE.blkcount, 1);
    SET_ಕೆಳಗಿನ(DefaultDPTE.dma, 0);
    SET_ಕೆಳಗಿನ(DefaultDPTE.pio, 0);
    SET_ಕೆಳಗಿನ(DefaultDPTE.options, options);
    SET_ಕೆಳಗಿನ(DefaultDPTE.reserved, 0);
    SET_ಕೆಳಗಿನ(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_ಕೆಳಗಿನ, &DefaultDPTE, 15);
    SET_ಕೆಳಗಿನ(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    ಯದಿ (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    ಪ್ರತ್ಯಾಗಮ fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_ಕೆಳಗಿನ, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
ಪೂರ್ಣಾಙ್ಕ noinline
fill_edd(ಸಂರಚನಾ segoff_s edd, ಸಂರಚನಾ drive_s *drive_fl)
{
    ವಿಕಲ್ಪ (GET_FLATPTR(drive_fl->type)) {
    ಪಕ್ಷ DTYPE_ATA:
    ಪಕ್ಷ DTYPE_ATA_ATAPI:
        ಪ್ರತ್ಯಾಗಮ fill_ata_edd(edd, drive_fl);
    ಪಕ್ಷ DTYPE_VIRTIO_BLK:
    ಪಕ್ಷ DTYPE_VIRTIO_SCSI:
        ಪ್ರತ್ಯಾಗಮ fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

ಶೂನ್ಯ
block_setup(ಶೂನ್ಯ)
{
    floppy_setup();
    ata_setup();
    ahci_setup();
    sdcard_setup();
    ramdisk_setup();
    virtio_blk_setup();
    virtio_scsi_setup();
    lsi_scsi_setup();
    esp_scsi_setup();
    megasas_setup();
    pvscsi_setup();
    mpt_scsi_setup();
    nvme_setup();
}

// Fallback handler for command requests not implemented by drivers
ಪೂರ್ಣಾಙ್ಕ
default_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_FORMAT:
    ಪಕ್ಷ CMD_RESET:
    ಪಕ್ಷ CMD_ISREADY:
    ಪಕ್ಷ CMD_VERIFY:
    ಪಕ್ಷ CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
process_op_both(ಸಂರಚನಾ disk_op_s *op)
{
    ವಿಕಲ್ಪ (GET_FLATPTR(op->drive_fl->type)) {
    ಪಕ್ಷ DTYPE_ATA_ATAPI:
        ಪ್ರತ್ಯಾಗಮ ata_atapi_process_op(op);
    ಪಕ್ಷ DTYPE_USB:
        ಪ್ರತ್ಯಾಗಮ usb_process_op(op);
    ಪಕ್ಷ DTYPE_UAS:
        ಪ್ರತ್ಯಾಗಮ uas_process_op(op);
    ಪಕ್ಷ DTYPE_LSI_SCSI:
        ಪ್ರತ್ಯಾಗಮ lsi_scsi_process_op(op);
    ಪಕ್ಷ DTYPE_ESP_SCSI:
        ಪ್ರತ್ಯಾಗಮ esp_scsi_process_op(op);
    ಪಕ್ಷ DTYPE_MEGASAS:
        ಪ್ರತ್ಯಾಗಮ megasas_process_op(op);
    ಪಕ್ಷ DTYPE_MPT_SCSI:
        ಪ್ರತ್ಯಾಗಮ mpt_scsi_process_op(op);
    ಸ್ವಭಾವ:
        ಯದಿ (!MODESEGMENT)
            ಪ್ರತ್ಯಾಗಮ DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        ಪ್ರತ್ಯಾಗಮ call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
ಪೂರ್ಣಾಙ್ಕ VISIBLE32FLAT
process_op_32(ಸಂರಚನಾ disk_op_s *op)
{
    ASSERT32FLAT();
    ವಿಕಲ್ಪ (op->drive_fl->type) {
    ಪಕ್ಷ DTYPE_VIRTIO_BLK:
        ಪ್ರತ್ಯಾಗಮ virtio_blk_process_op(op);
    ಪಕ್ಷ DTYPE_AHCI:
        ಪ್ರತ್ಯಾಗಮ ahci_process_op(op);
    ಪಕ್ಷ DTYPE_AHCI_ATAPI:
        ಪ್ರತ್ಯಾಗಮ ahci_atapi_process_op(op);
    ಪಕ್ಷ DTYPE_SDCARD:
        ಪ್ರತ್ಯಾಗಮ sdcard_process_op(op);
    ಪಕ್ಷ DTYPE_USB_32:
        ಪ್ರತ್ಯಾಗಮ usb_process_op(op);
    ಪಕ್ಷ DTYPE_UAS_32:
        ಪ್ರತ್ಯಾಗಮ uas_process_op(op);
    ಪಕ್ಷ DTYPE_VIRTIO_SCSI:
        ಪ್ರತ್ಯಾಗಮ virtio_scsi_process_op(op);
    ಪಕ್ಷ DTYPE_PVSCSI:
        ಪ್ರತ್ಯಾಗಮ pvscsi_process_op(op);
    ಪಕ್ಷ DTYPE_NVME:
        ಪ್ರತ್ಯಾಗಮ nvme_process_op(op);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
process_op_16(ಸಂರಚನಾ disk_op_s *op)
{
    ASSERT16();
    ವಿಕಲ್ಪ (GET_FLATPTR(op->drive_fl->type)) {
    ಪಕ್ಷ DTYPE_FLOPPY:
        ಪ್ರತ್ಯಾಗಮ floppy_process_op(op);
    ಪಕ್ಷ DTYPE_ATA:
        ಪ್ರತ್ಯಾಗಮ ata_process_op(op);
    ಪಕ್ಷ DTYPE_RAMDISK:
        ಪ್ರತ್ಯಾಗಮ ramdisk_process_op(op);
    ಪಕ್ಷ DTYPE_CDEMU:
        ಪ್ರತ್ಯಾಗಮ cdemu_process_op(op);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ process_op_both(op);
    }
}

// Execute a disk_op_s request.
ಪೂರ್ಣಾಙ್ಕ
process_op(ಸಂರಚನಾ disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->ಎಣಿಕೆ, op->command);

    ಪೂರ್ಣಾಙ್ಕ ret, origcount = op->ಎಣಿಕೆ;
    ಯದಿ (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->ಎಣಿಕೆ = 0;
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBOUNDARY;
    }
    ಯದಿ (MODESEGMENT)
        ret = process_op_16(op);
    ಅನ್ಯಥಾ
        ret = process_op_32(op);
    ಯದಿ (ret && op->ಎಣಿಕೆ == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->ಎಣಿಕೆ = 0;
    ಪ್ರತ್ಯಾಗಮ ret;
}
