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
ರಚನೆ drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

ರಚನೆ drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    ಆದರೆ (extdriveoffset >= ARRAY_ಗಾತ್ರ(IDMap[0]))
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

ಪೂರ್ಣಾಂಕ getDriveId(u8 exttype, ರಚನೆ drive_s *drive)
{
    ASSERT32FLAT();
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(IDMap[0]); i++)
        ಆದರೆ (getDrive(exttype, i) == drive)
            ಹಿಂತಿರುಗಿ i;
    ಹಿಂತಿರುಗಿ -1;
}

ಪೂರ್ಣಾಂಕ create_bounce_buf(ಶೂನ್ಯ)
{
    ಆದರೆ (bounce_buf_fl)
        ಹಿಂತಿರುಗಿ 0;

    u8 *buf = malloc_ಕೆಳಗಿನ(CDROM_SECTOR_ಗಾತ್ರ);
    ಆದರೆ (!buf) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    bounce_buf_fl = buf;
    ಹಿಂತಿರುಗಿ 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
host_lchs_supplied(ರಚನೆ drive_s *drive)
{
    ಹಿಂತಿರುಗಿ (drive->lchs.ತಲೆ <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

ಸ್ಥಿರ u8
get_translation(ರಚನೆ drive_s *drive)
{
    ಆದರೆ (host_lchs_supplied(drive))
        ಹಿಂತಿರುಗಿ TRANSLATION_HOST;
    u8 type = drive->type;
    ಆದರೆ (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        ಹಿಂತಿರುಗಿ translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.ತಲೆ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    ಆದರೆ (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        ಹಿಂತಿರುಗಿ TRANSLATION_LBA;

    ಆದರೆ (cylinders <= 1024 && heads <= 16 && spt <= 63)
        ಹಿಂತಿರುಗಿ TRANSLATION_NONE;
    ಆದರೆ (cylinders * heads <= 131072)
        ಹಿಂತಿರುಗಿ TRANSLATION_LARGE;
    ಹಿಂತಿರುಗಿ TRANSLATION_LBA;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಸಿದ್ಧತೆ_translation(ರಚನೆ drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.ತಲೆ ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc = ಖಾಲಿ;

    ಆಯ್ಕೆ (translation) {
    ಪೂರ್ವನಿಯೋಜಿತ:
    ಸ್ಥಿತಿ TRANSLATION_NONE:
        desc = "none";
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        ಆದರೆ (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            ನಿಲ್ಲಿಸು;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        ಆದರೆ (heads>128)
            heads = 255;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (heads>64)
            heads = 128;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (heads>32)
            heads = 64;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (heads>16)
            heads = 32;
        ಇಲ್ಲದಿದ್ದರೆ
            heads = 16;
        cylinders = sect / heads;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        ಆದರೆ (heads==16) {
            ಆದರೆ (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    ಸ್ಥಿತಿ TRANSLATION_LARGE:
        ಆದರೆ (translation == TRANSLATION_LARGE)
            desc = "large";
        ಆಗ (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            ಆದರೆ (heads > 127)
                ನಿಲ್ಲಿಸು;
        }
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.ತಲೆ;
        spt = drive->lchs.sector;
        ನಿಲ್ಲಿಸು;
    }
    // clip to 1024 cylinders in lchs
    ಆದರೆ (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%u\n"
            , drive
            , drive->pchs.cylinder, drive->pchs.ತಲೆ, drive->pchs.sector
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    drive->lchs.ತಲೆ = heads;
    drive->lchs.cylinder = cylinders;
    drive->lchs.sector = spt;
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
ಸ್ಥಿರ ಶೂನ್ಯ
fill_fdpt(ರಚನೆ drive_s *drive, ಪೂರ್ಣಾಂಕ hdid)
{
    ಆದರೆ (hdid > 1)
        ಹಿಂತಿರುಗಿ;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.ತಲೆ;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.ತಲೆ;
    u16 nps = drive->pchs.sector;

    ರಚನೆ fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    ಆದರೆ (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, ಗಾತ್ರ(*fdpt));
    }

    ಆದರೆ (hdid == 0)
        ಗಣ_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s, fdpt[0])));
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s, fdpt[1])));
}

// Find spot to add a drive
ಸ್ಥಿರ ಶೂನ್ಯ
ಕೂಡಿಸು_drive(ರಚನೆ drive_s **idmap, u8 *ಎಣಿಕೆ_2, ರಚನೆ drive_s *drive)
{
    ಆದರೆ (*ಎಣಿಕೆ_2 >= ARRAY_ಗಾತ್ರ(IDMap[0])) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    idmap[*ಎಣಿಕೆ_2] = drive;
    *ಎಣಿಕೆ_2 = *ಎಣಿಕೆ_2 + 1;
}

// Map a hard drive
ಶೂನ್ಯ
map_hd_drive(ರಚನೆ drive_s *drive)
{
    ASSERT32FLAT();
    ರಚನೆ bios_ದತ್ತಾಂಶ_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    ಪೂರ್ಣಾಂಕ hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    ಕೂಡಿಸು_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    ಸಿದ್ಧತೆ_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
ಶೂನ್ಯ
map_cd_drive(ರಚನೆ drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    ಕೂಡಿಸು_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
ಶೂನ್ಯ
map_floppy_drive(ರಚನೆ drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    ಕೂಡಿಸು_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_ಎಣಿಕೆ, drive);

    // Update equipment word bits for floppy
    ಆದರೆ (Floppy_ಎಣಿಕೆ == 1) {
        // 1 drive, ready for boot
        ಗಣ_equipment_flags(0x41, 0x01);
        ಗಣ_BDA(floppy_harddisk_info, 0x07);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Floppy_ಎಣಿಕೆ >= 2) {
        // 2 drives, ready for boot
        ಗಣ_equipment_flags(0x41, 0x41);
        ಗಣ_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * Extended Disk Drive (EDD) get drive parameters
 ****************************************************************/

// flags for bus_iface field in fill_generic_edd()
#ವ್ಯಾಖ್ಯಾನಿಸು EDD_ISA        0x01
#ವ್ಯಾಖ್ಯಾನಿಸು EDD_PCI        0x02
#ವ್ಯಾಖ್ಯಾನಿಸು EDD_BUS_ಮುಖವಾಡ   0x0f
#ವ್ಯಾಖ್ಯಾನಿಸು EDD_ATA        0x10
#ವ್ಯಾಖ್ಯಾನಿಸು EDD_SCSI       0x20
#ವ್ಯಾಖ್ಯಾನಿಸು EDD_IFACE_ಮುಖವಾಡ 0xf0

// Fill in EDD info
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
fill_generic_edd(ರಚನೆ segoff_s edd, ರಚನೆ drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_ಮಾರ್ಗ, u32 device_ಮಾರ್ಗ)
{
    u16 seg = edd.seg;
    ರಚನೆ int13dpt_s *param_far = (ಶೂನ್ಯ*)(edd.ಸ್ಥಾನಾಂತರ_2+0);
    u16 ಗಾತ್ರ_2 = GET_FARVAR(seg, param_far->ಗಾತ್ರ_2);
    u16 t13 = ಗಾತ್ರ_2 == 74;

    // Buffer is too small
    ಆದರೆ (ಗಾತ್ರ_2 < 26)
        ಹಿಂತಿರುಗಿ DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.ತಲೆ);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , ಗಾತ್ರ_2, type, npc, nph, nps, (u32)lba, blksize);

    ಗಣ_FARVAR(seg, param_far->ಗಾತ್ರ_2, 26);
    ಆದರೆ (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        ಗಣ_FARVAR(seg, param_far->infos, 0x74);
        ಗಣ_FARVAR(seg, param_far->cylinders, 0xffffffff);
        ಗಣ_FARVAR(seg, param_far->heads, 0xffffffff);
        ಗಣ_FARVAR(seg, param_far->spt, 0xffffffff);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (lba > (u64)nps*nph*0x3fff) {
            ಗಣ_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            ಗಣ_FARVAR(seg, param_far->cylinders, 0x3fff);
        } ಇಲ್ಲದಿದ್ದರೆ {
            ಗಣ_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            ಗಣ_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        ಗಣ_FARVAR(seg, param_far->heads, (u32)nph);
        ಗಣ_FARVAR(seg, param_far->spt, (u32)nps);
    }
    ಗಣ_FARVAR(seg, param_far->sector_ಎಣಿಕೆ, lba);
    ಗಣ_FARVAR(seg, param_far->blksize, blksize);

    ಆದರೆ (ಗಾತ್ರ_2 < 30 || !dpte_so)
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;

    // EDD 2.x

    ಗಣ_FARVAR(seg, param_far->ಗಾತ್ರ_2, 30);
    ಗಣ_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    ಆದರೆ (ಗಾತ್ರ_2 < 66 || !bus_iface)
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;

    // EDD 3.x
    ಗಣ_FARVAR(seg, param_far->ಕೀಲಿ, 0xbedd);
    ಗಣ_FARVAR(seg, param_far->dpi_ಉದ್ದಳತೆ, t13 ? 44 : 36);
    ಗಣ_FARVAR(seg, param_far->reserved1, 0);
    ಗಣ_FARVAR(seg, param_far->reserved2, 0);

    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *host_bus = "ISA ";
    ಆದರೆ ((bus_iface & EDD_BUS_ಮುಖವಾಡ) == EDD_PCI) {
        host_bus = "PCI ";
        ಆದರೆ (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_ಮಾರ್ಗ &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , ಗಾತ್ರ(param_far->host_bus));
    ಗಣ_FARVAR(seg, param_far->iface_ಮಾರ್ಗ, iface_ಮಾರ್ಗ);

    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *iface_type = "ATA     ";
    ಆದರೆ ((bus_iface & EDD_IFACE_ಮುಖವಾಡ) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , ಗಾತ್ರ(param_far->iface_type));
    ಆದರೆ (t13) {
        ಗಣ_FARVAR(seg, param_far->t13.device_ಮಾರ್ಗ[0], device_ಮಾರ್ಗ);
        ಗಣ_FARVAR(seg, param_far->t13.device_ಮಾರ್ಗ[1], 0);

        ಗಣ_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (ಶೂನ್ಯ*)param_far+30, 43));
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಗಣ_FARVAR(seg, param_far->phoenix.device_ಮಾರ್ಗ, device_ಮಾರ್ಗ);

        ಗಣ_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (ಶೂನ್ಯ*)param_far+30, 35));
    }

    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
ಸ್ಥಿರ u32
edd_pci_ಮಾರ್ಗ(u16 bdf, u8 channel)
{
    ಹಿಂತಿರುಗಿ (pci_bdf_ಗಮ್ಯ_bus(bdf) | (pci_bdf_ಗಮ್ಯ_dev(bdf) << 8)
            | (pci_bdf_ಗಮ್ಯ_fn(bdf) << 16) | ((u32)channel << 24));
}

ರಚನೆ dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
fill_ata_edd(ರಚನೆ segoff_s edd, ರಚನೆ drive_s *drive_gf)
{
    ಆದರೆ (!CONFIG_ATA)
        ಹಿಂತಿರುಗಿ DISK_RET_EPARAM;

    // Fill in dpte
    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        drive_gf, ರಚನೆ atadrive_s, drive);
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    ಪೂರ್ಣಾಂಕ bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    ಆದರೆ (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        ಆದರೆ ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            ಆದರೆ (translation == TRANSLATION_LBA)
                options |= 1<<9;
            ಆದರೆ (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    ಆದರೆ (CONFIG_ATA_PIO32)
        options |= 1<<7;

    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.iobase1, iobase1);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.ಪೂರ್ವಪ್ರತ್ಯಯ, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.ಬಳಸದ, 0xcb);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.irq, irq);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.blkcount, 1);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.dma, 0);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.pio, 0);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.options, options);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.reserved, 0);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.revision, 0x11);

    u8 ಮೊತ್ತ = checksum_far(SEG_ಕೆಳಗಿನ, &DefaultDPTE, 15);
    ಗಣ_ಕೆಳಗಿನ(DefaultDPTE.checksum, -ಮೊತ್ತ);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    ಆದರೆ (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_ಮಾರ್ಗ(bdf, channel);
    }
    ಹಿಂತಿರುಗಿ fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_ಕೆಳಗಿನ, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
ಪೂರ್ಣಾಂಕ noinline
fill_edd(ರಚನೆ segoff_s edd, ರಚನೆ drive_s *drive_fl)
{
    ಆಯ್ಕೆ (GET_FLATPTR(drive_fl->type)) {
    ಸ್ಥಿತಿ DTYPE_ATA:
    ಸ್ಥಿತಿ DTYPE_ATA_ATAPI:
        ಹಿಂತಿರುಗಿ fill_ata_edd(edd, drive_fl);
    ಸ್ಥಿತಿ DTYPE_VIRTIO_BLK:
    ಸ್ಥಿತಿ DTYPE_VIRTIO_SCSI:
        ಹಿಂತಿರುಗಿ fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_ಮಾರ್ಗ(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

ಶೂನ್ಯ
ಖಂಡ_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    floppy_ಸಿದ್ಧತೆ();
    ata_ಸಿದ್ಧತೆ();
    ahci_ಸಿದ್ಧತೆ();
    sdcard_ಸಿದ್ಧತೆ();
    ramdisk_ಸಿದ್ಧತೆ();
    virtio_blk_ಸಿದ್ಧತೆ();
    virtio_scsi_ಸಿದ್ಧತೆ();
    lsi_scsi_ಸಿದ್ಧತೆ();
    esp_scsi_ಸಿದ್ಧತೆ();
    megasas_ಸಿದ್ಧತೆ();
    pvscsi_ಸಿದ್ಧತೆ();
    mpt_scsi_ಸಿದ್ಧತೆ();
    nvme_ಸಿದ್ಧತೆ();
}

// Fallback handler for command requests not implemented by drivers
ಪೂರ್ಣಾಂಕ
default_process_op(ರಚನೆ disk_op_s *op)
{
    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_FORMAT:
    ಸ್ಥಿತಿ CMD_RESET:
    ಸ್ಥಿತಿ CMD_ISREADY:
    ಸ್ಥಿತಿ CMD_VERIFY:
    ಸ್ಥಿತಿ CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
process_op_both(ರಚನೆ disk_op_s *op)
{
    ಆಯ್ಕೆ (GET_FLATPTR(op->drive_fl->type)) {
    ಸ್ಥಿತಿ DTYPE_ATA_ATAPI:
        ಹಿಂತಿರುಗಿ ata_atapi_process_op(op);
    ಸ್ಥಿತಿ DTYPE_USB:
        ಹಿಂತಿರುಗಿ usb_process_op(op);
    ಸ್ಥಿತಿ DTYPE_UAS:
        ಹಿಂತಿರುಗಿ uas_process_op(op);
    ಸ್ಥಿತಿ DTYPE_LSI_SCSI:
        ಹಿಂತಿರುಗಿ lsi_scsi_process_op(op);
    ಸ್ಥಿತಿ DTYPE_ESP_SCSI:
        ಹಿಂತಿರುಗಿ esp_scsi_process_op(op);
    ಸ್ಥಿತಿ DTYPE_MEGASAS:
        ಹಿಂತಿರುಗಿ megasas_process_op(op);
    ಸ್ಥಿತಿ DTYPE_MPT_SCSI:
        ಹಿಂತಿರುಗಿ mpt_scsi_process_op(op);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಆದರೆ (!MODESEGMENT)
            ಹಿಂತಿರುಗಿ DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        ಹಿಂತಿರುಗಿ call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
ಪೂರ್ಣಾಂಕ VISIBLE32FLAT
process_op_32(ರಚನೆ disk_op_s *op)
{
    ASSERT32FLAT();
    ಆಯ್ಕೆ (op->drive_fl->type) {
    ಸ್ಥಿತಿ DTYPE_VIRTIO_BLK:
        ಹಿಂತಿರುಗಿ virtio_blk_process_op(op);
    ಸ್ಥಿತಿ DTYPE_AHCI:
        ಹಿಂತಿರುಗಿ ahci_process_op(op);
    ಸ್ಥಿತಿ DTYPE_AHCI_ATAPI:
        ಹಿಂತಿರುಗಿ ahci_atapi_process_op(op);
    ಸ್ಥಿತಿ DTYPE_SDCARD:
        ಹಿಂತಿರುಗಿ sdcard_process_op(op);
    ಸ್ಥಿತಿ DTYPE_USB_32:
        ಹಿಂತಿರುಗಿ usb_process_op(op);
    ಸ್ಥಿತಿ DTYPE_UAS_32:
        ಹಿಂತಿರುಗಿ uas_process_op(op);
    ಸ್ಥಿತಿ DTYPE_VIRTIO_SCSI:
        ಹಿಂತಿರುಗಿ virtio_scsi_process_op(op);
    ಸ್ಥಿತಿ DTYPE_PVSCSI:
        ಹಿಂತಿರುಗಿ pvscsi_process_op(op);
    ಸ್ಥಿತಿ DTYPE_NVME:
        ಹಿಂತಿರುಗಿ nvme_process_op(op);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
process_op_16(ರಚನೆ disk_op_s *op)
{
    ASSERT16();
    ಆಯ್ಕೆ (GET_FLATPTR(op->drive_fl->type)) {
    ಸ್ಥಿತಿ DTYPE_FLOPPY:
        ಹಿಂತಿರುಗಿ floppy_process_op(op);
    ಸ್ಥಿತಿ DTYPE_ATA:
        ಹಿಂತಿರುಗಿ ata_process_op(op);
    ಸ್ಥಿತಿ DTYPE_RAMDISK:
        ಹಿಂತಿರುಗಿ ramdisk_process_op(op);
    ಸ್ಥಿತಿ DTYPE_CDEMU:
        ಹಿಂತಿರುಗಿ cdemu_process_op(op);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ process_op_both(op);
    }
}

// Execute a disk_op_s request.
ಪೂರ್ಣಾಂಕ
process_op(ರಚನೆ disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->ಎಣಿಕೆ_2, op->command);

    ಪೂರ್ಣಾಂಕ ret, origcount = op->ಎಣಿಕೆ_2;
    ಆದರೆ (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->ಎಣಿಕೆ_2 = 0;
        ಹಿಂತಿರುಗಿ DISK_RET_EBOUNDARY;
    }
    ಆದರೆ (MODESEGMENT)
        ret = process_op_16(op);
    ಇಲ್ಲದಿದ್ದರೆ
        ret = process_op_32(op);
    ಆದರೆ (ret && op->ಎಣಿಕೆ_2 == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->ಎಣಿಕೆ_2 = 0;
    ಹಿಂತಿರುಗಿ ret;
}
