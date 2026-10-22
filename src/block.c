// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ata.h" // process_ata_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ahci.h" // process_ahci_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/esp-scsi.h" // esp_scsi_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/lsi-scsi.h" // lsi_scsi_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/megasas.h" // megasas_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/mpt-scsi.h" // mpt_scsi_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_bdf_to_bus
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pvscsi.h" // pvscsi_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // rtc_read
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/usb-msc.h" // usb_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/usb-uas.h" // uas_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/virtio-blk.h" // process_virtio_blk_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/virtio-scsi.h" // virtio_scsi_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/nvme.h" // nvme_process_op
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_low
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // call32
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // struct dpte_s
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // checksum
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // process_floppy_op

u8 Floppy_ਗਿਣਤੀ VARFSEG;
u8 CDCount;
ਬਣਤਰ drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

ਬਣਤਰ drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    ਜੇ (extdriveoffset >= ARRAY_ਮਾਪ(IDMap[0]))
        ਵਾਪਸ ਨਿਰਾ;
    ਵਾਪਸ GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

ਪੂਰਨ getDriveId(u8 exttype, ਬਣਤਰ drive_s *drive)
{
    ASSERT32FLAT();
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(IDMap[0]); i++)
        ਜੇ (getDrive(exttype, i) == drive)
            ਵਾਪਸ i;
    ਵਾਪਸ -1;
}

ਪੂਰਨ create_bounce_buf(ਖਾਲੀ)
{
    ਜੇ (bounce_buf_fl)
        ਵਾਪਸ 0;

    u8 *buf = malloc_ਨੀਵਾਂ(CDROM_SECTOR_ਮਾਪ);
    ਜੇ (!buf) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    bounce_buf_fl = buf;
    ਵਾਪਸ 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ
host_lchs_supplied(ਬਣਤਰ drive_s *drive)
{
    ਵਾਪਸ (drive->lchs.ਸਿਰ <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

ਸਥਿਰ u8
get_translation(ਬਣਤਰ drive_s *drive)
{
    ਜੇ (host_lchs_supplied(drive))
        ਵਾਪਸ TRANSLATION_HOST;
    u8 type = drive->type;
    ਜੇ (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        ਵਾਪਸ translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.ਸਿਰ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    ਜੇ (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        ਵਾਪਸ TRANSLATION_LBA;

    ਜੇ (cylinders <= 1024 && heads <= 16 && spt <= 63)
        ਵਾਪਸ TRANSLATION_NONE;
    ਜੇ (cylinders * heads <= 131072)
        ਵਾਪਸ TRANSLATION_LARGE;
    ਵਾਪਸ TRANSLATION_LBA;
}

ਸਥਿਰ ਖਾਲੀ
ਤਿਆਰੀ_translation(ਬਣਤਰ drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.ਸਿਰ ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ਅਚਲ ਅੱਖਰ *desc = ਨਿਰਾ;

    ਚੋਣ (translation) {
    ਮੂਲ:
    ਹਾਲਤ TRANSLATION_NONE:
        desc = "none";
        ਰੋਕੋ;
    ਹਾਲਤ TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        ਜੇ (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            ਰੋਕੋ;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        ਜੇ (heads>128)
            heads = 255;
        ਨਹੀਂਤਾਂ ਜੇ (heads>64)
            heads = 128;
        ਨਹੀਂਤਾਂ ਜੇ (heads>32)
            heads = 64;
        ਨਹੀਂਤਾਂ ਜੇ (heads>16)
            heads = 32;
        ਨਹੀਂਤਾਂ
            heads = 16;
        cylinders = sect / heads;
        ਰੋਕੋ;
    ਹਾਲਤ TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        ਜੇ (heads==16) {
            ਜੇ (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    ਹਾਲਤ TRANSLATION_LARGE:
        ਜੇ (translation == TRANSLATION_LARGE)
            desc = "large";
        ਜਦੋਂ (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            ਜੇ (heads > 127)
                ਰੋਕੋ;
        }
        ਰੋਕੋ;
    ਹਾਲਤ TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.ਸਿਰ;
        spt = drive->lchs.sector;
        ਰੋਕੋ;
    }
    // clip to 1024 cylinders in lchs
    ਜੇ (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%u\n"
            , drive
            , drive->pchs.cylinder, drive->pchs.ਸਿਰ, drive->pchs.sector
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    drive->lchs.ਸਿਰ = heads;
    drive->lchs.cylinder = cylinders;
    drive->lchs.sector = spt;
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
ਸਥਿਰ ਖਾਲੀ
fill_fdpt(ਬਣਤਰ drive_s *drive, ਪੂਰਨ hdid)
{
    ਜੇ (hdid > 1)
        ਵਾਪਸ;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.ਸਿਰ;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.ਸਿਰ;
    u16 nps = drive->pchs.sector;

    ਬਣਤਰ fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    ਜੇ (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, ਆਕਾਰ(*fdpt));
    }

    ਜੇ (hdid == 0)
        ਸੈੱਟ_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s, fdpt[0])));
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s, fdpt[1])));
}

// Find spot to add a drive
ਸਥਿਰ ਖਾਲੀ
ਜੋੜੋ_drive(ਬਣਤਰ drive_s **idmap, u8 *ਗਿਣਤੀ_2, ਬਣਤਰ drive_s *drive)
{
    ਜੇ (*ਗਿਣਤੀ_2 >= ARRAY_ਮਾਪ(IDMap[0])) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    idmap[*ਗਿਣਤੀ_2] = drive;
    *ਗਿਣਤੀ_2 = *ਗਿਣਤੀ_2 + 1;
}

// Map a hard drive
ਖਾਲੀ
map_hd_drive(ਬਣਤਰ drive_s *drive)
{
    ASSERT32FLAT();
    ਬਣਤਰ bios_ਅੰਕੜੇ_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    ਪੂਰਨ hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    ਜੋੜੋ_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    ਤਿਆਰੀ_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
ਖਾਲੀ
map_cd_drive(ਬਣਤਰ drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    ਜੋੜੋ_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
ਖਾਲੀ
map_floppy_drive(ਬਣਤਰ drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    ਜੋੜੋ_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_ਗਿਣਤੀ, drive);

    // Update equipment word bits for floppy
    ਜੇ (Floppy_ਗਿਣਤੀ == 1) {
        // 1 drive, ready for boot
        ਸੈੱਟ_equipment_flags(0x41, 0x01);
        ਸੈੱਟ_BDA(floppy_harddisk_info, 0x07);
    } ਨਹੀਂਤਾਂ ਜੇ (Floppy_ਗਿਣਤੀ >= 2) {
        // 2 drives, ready for boot
        ਸੈੱਟ_equipment_flags(0x41, 0x41);
        ਸੈੱਟ_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * Extended Disk Drive (EDD) get drive parameters
 ****************************************************************/

// flags for bus_iface field in fill_generic_edd()
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EDD_ISA        0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EDD_PCI        0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EDD_BUS_ਨਕਾਬ   0x0f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EDD_ATA        0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EDD_SCSI       0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EDD_IFACE_ਨਕਾਬ 0xf0

// Fill in EDD info
ਸਥਿਰ ਪੂਰਨ
fill_generic_edd(ਬਣਤਰ segoff_s edd, ਬਣਤਰ drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_ਰਸਤਾ, u32 device_ਰਸਤਾ)
{
    u16 seg = edd.seg;
    ਬਣਤਰ int13dpt_s *param_far = (ਖਾਲੀ*)(edd.ਵਿੱਥ_2+0);
    u16 ਮਾਪ = GET_FARVAR(seg, param_far->ਮਾਪ);
    u16 t13 = ਮਾਪ == 74;

    // Buffer is too small
    ਜੇ (ਮਾਪ < 26)
        ਵਾਪਸ DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.ਸਿਰ);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , ਮਾਪ, type, npc, nph, nps, (u32)lba, blksize);

    ਸੈੱਟ_FARVAR(seg, param_far->ਮਾਪ, 26);
    ਜੇ (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        ਸੈੱਟ_FARVAR(seg, param_far->infos, 0x74);
        ਸੈੱਟ_FARVAR(seg, param_far->cylinders, 0xffffffff);
        ਸੈੱਟ_FARVAR(seg, param_far->heads, 0xffffffff);
        ਸੈੱਟ_FARVAR(seg, param_far->spt, 0xffffffff);
    } ਨਹੀਂਤਾਂ {
        ਜੇ (lba > (u64)nps*nph*0x3fff) {
            ਸੈੱਟ_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            ਸੈੱਟ_FARVAR(seg, param_far->cylinders, 0x3fff);
        } ਨਹੀਂਤਾਂ {
            ਸੈੱਟ_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            ਸੈੱਟ_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        ਸੈੱਟ_FARVAR(seg, param_far->heads, (u32)nph);
        ਸੈੱਟ_FARVAR(seg, param_far->spt, (u32)nps);
    }
    ਸੈੱਟ_FARVAR(seg, param_far->sector_ਗਿਣਤੀ, lba);
    ਸੈੱਟ_FARVAR(seg, param_far->blksize, blksize);

    ਜੇ (ਮਾਪ < 30 || !dpte_so)
        ਵਾਪਸ DISK_RET_SUCCESS;

    // EDD 2.x

    ਸੈੱਟ_FARVAR(seg, param_far->ਮਾਪ, 30);
    ਸੈੱਟ_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    ਜੇ (ਮਾਪ < 66 || !bus_iface)
        ਵਾਪਸ DISK_RET_SUCCESS;

    // EDD 3.x
    ਸੈੱਟ_FARVAR(seg, param_far->ਕੁੰਜੀ, 0xbedd);
    ਸੈੱਟ_FARVAR(seg, param_far->dpi_ਲੰਮਾਈ, t13 ? 44 : 36);
    ਸੈੱਟ_FARVAR(seg, param_far->reserved1, 0);
    ਸੈੱਟ_FARVAR(seg, param_far->reserved2, 0);

    ਅਚਲ ਅੱਖਰ *host_bus = "ISA ";
    ਜੇ ((bus_iface & EDD_BUS_ਨਕਾਬ) == EDD_PCI) {
        host_bus = "PCI ";
        ਜੇ (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_ਰਸਤਾ &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , ਆਕਾਰ(param_far->host_bus));
    ਸੈੱਟ_FARVAR(seg, param_far->iface_ਰਸਤਾ, iface_ਰਸਤਾ);

    ਅਚਲ ਅੱਖਰ *iface_type = "ATA     ";
    ਜੇ ((bus_iface & EDD_IFACE_ਨਕਾਬ) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , ਆਕਾਰ(param_far->iface_type));
    ਜੇ (t13) {
        ਸੈੱਟ_FARVAR(seg, param_far->t13.device_ਰਸਤਾ[0], device_ਰਸਤਾ);
        ਸੈੱਟ_FARVAR(seg, param_far->t13.device_ਰਸਤਾ[1], 0);

        ਸੈੱਟ_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (ਖਾਲੀ*)param_far+30, 43));
    } ਨਹੀਂਤਾਂ {
        ਸੈੱਟ_FARVAR(seg, param_far->phoenix.device_ਰਸਤਾ, device_ਰਸਤਾ);

        ਸੈੱਟ_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (ਖਾਲੀ*)param_far+30, 35));
    }

    ਵਾਪਸ DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
ਸਥਿਰ u32
edd_pci_ਰਸਤਾ(u16 bdf, u8 channel)
{
    ਵਾਪਸ (pci_bdf_ਮੰਜ਼ਿਲ_bus(bdf) | (pci_bdf_ਮੰਜ਼ਿਲ_dev(bdf) << 8)
            | (pci_bdf_ਮੰਜ਼ਿਲ_fn(bdf) << 16) | ((u32)channel << 24));
}

ਬਣਤਰ dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
ਸਥਿਰ ਪੂਰਨ
fill_ata_edd(ਬਣਤਰ segoff_s edd, ਬਣਤਰ drive_s *drive_gf)
{
    ਜੇ (!CONFIG_ATA)
        ਵਾਪਸ DISK_RET_EPARAM;

    // Fill in dpte
    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        drive_gf, ਬਣਤਰ atadrive_s, drive);
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    ਪੂਰਨ bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    ਜੇ (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        ਜੇ ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            ਜੇ (translation == TRANSLATION_LBA)
                options |= 1<<9;
            ਜੇ (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } ਨਹੀਂਤਾਂ {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    ਜੇ (CONFIG_ATA_PIO32)
        options |= 1<<7;

    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.iobase1, iobase1);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.ਅਗੇਤਰ, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.ਅਣਵਰਤਿਆ, 0xcb);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.irq, irq);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.blkcount, 1);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.dma, 0);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.pio, 0);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.options, options);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.reserved, 0);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.revision, 0x11);

    u8 ਜੋੜ = checksum_far(SEG_ਨੀਵਾਂ, &DefaultDPTE, 15);
    ਸੈੱਟ_ਨੀਵਾਂ(DefaultDPTE.checksum, -ਜੋੜ);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    ਜੇ (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_ਰਸਤਾ(bdf, channel);
    }
    ਵਾਪਸ fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_ਨੀਵਾਂ, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
ਪੂਰਨ noinline
fill_edd(ਬਣਤਰ segoff_s edd, ਬਣਤਰ drive_s *drive_fl)
{
    ਚੋਣ (GET_FLATPTR(drive_fl->type)) {
    ਹਾਲਤ DTYPE_ATA:
    ਹਾਲਤ DTYPE_ATA_ATAPI:
        ਵਾਪਸ fill_ata_edd(edd, drive_fl);
    ਹਾਲਤ DTYPE_VIRTIO_BLK:
    ਹਾਲਤ DTYPE_VIRTIO_SCSI:
        ਵਾਪਸ fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_ਰਸਤਾ(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    ਮੂਲ:
        ਵਾਪਸ fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

ਖਾਲੀ
ਖੰਡ_ਤਿਆਰੀ(ਖਾਲੀ)
{
    floppy_ਤਿਆਰੀ();
    ata_ਤਿਆਰੀ();
    ahci_ਤਿਆਰੀ();
    sdcard_ਤਿਆਰੀ();
    ramdisk_ਤਿਆਰੀ();
    virtio_blk_ਤਿਆਰੀ();
    virtio_scsi_ਤਿਆਰੀ();
    lsi_scsi_ਤਿਆਰੀ();
    esp_scsi_ਤਿਆਰੀ();
    megasas_ਤਿਆਰੀ();
    pvscsi_ਤਿਆਰੀ();
    mpt_scsi_ਤਿਆਰੀ();
    nvme_ਤਿਆਰੀ();
}

// Fallback handler for command requests not implemented by drivers
ਪੂਰਨ
default_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_FORMAT:
    ਹਾਲਤ CMD_RESET:
    ਹਾਲਤ CMD_ISREADY:
    ਹਾਲਤ CMD_VERIFY:
    ਹਾਲਤ CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        ਵਾਪਸ DISK_RET_SUCCESS;
    ਮੂਲ:
        ਵਾਪਸ DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
ਸਥਿਰ ਪੂਰਨ
process_op_both(ਬਣਤਰ disk_op_s *op)
{
    ਚੋਣ (GET_FLATPTR(op->drive_fl->type)) {
    ਹਾਲਤ DTYPE_ATA_ATAPI:
        ਵਾਪਸ ata_atapi_process_op(op);
    ਹਾਲਤ DTYPE_USB:
        ਵਾਪਸ usb_process_op(op);
    ਹਾਲਤ DTYPE_UAS:
        ਵਾਪਸ uas_process_op(op);
    ਹਾਲਤ DTYPE_LSI_SCSI:
        ਵਾਪਸ lsi_scsi_process_op(op);
    ਹਾਲਤ DTYPE_ESP_SCSI:
        ਵਾਪਸ esp_scsi_process_op(op);
    ਹਾਲਤ DTYPE_MEGASAS:
        ਵਾਪਸ megasas_process_op(op);
    ਹਾਲਤ DTYPE_MPT_SCSI:
        ਵਾਪਸ mpt_scsi_process_op(op);
    ਮੂਲ:
        ਜੇ (!MODESEGMENT)
            ਵਾਪਸ DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        ਵਾਪਸ call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
ਪੂਰਨ VISIBLE32FLAT
process_op_32(ਬਣਤਰ disk_op_s *op)
{
    ASSERT32FLAT();
    ਚੋਣ (op->drive_fl->type) {
    ਹਾਲਤ DTYPE_VIRTIO_BLK:
        ਵਾਪਸ virtio_blk_process_op(op);
    ਹਾਲਤ DTYPE_AHCI:
        ਵਾਪਸ ahci_process_op(op);
    ਹਾਲਤ DTYPE_AHCI_ATAPI:
        ਵਾਪਸ ahci_atapi_process_op(op);
    ਹਾਲਤ DTYPE_SDCARD:
        ਵਾਪਸ sdcard_process_op(op);
    ਹਾਲਤ DTYPE_USB_32:
        ਵਾਪਸ usb_process_op(op);
    ਹਾਲਤ DTYPE_UAS_32:
        ਵਾਪਸ uas_process_op(op);
    ਹਾਲਤ DTYPE_VIRTIO_SCSI:
        ਵਾਪਸ virtio_scsi_process_op(op);
    ਹਾਲਤ DTYPE_PVSCSI:
        ਵਾਪਸ pvscsi_process_op(op);
    ਹਾਲਤ DTYPE_NVME:
        ਵਾਪਸ nvme_process_op(op);
    ਮੂਲ:
        ਵਾਪਸ process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
ਸਥਿਰ ਪੂਰਨ
process_op_16(ਬਣਤਰ disk_op_s *op)
{
    ASSERT16();
    ਚੋਣ (GET_FLATPTR(op->drive_fl->type)) {
    ਹਾਲਤ DTYPE_FLOPPY:
        ਵਾਪਸ floppy_process_op(op);
    ਹਾਲਤ DTYPE_ATA:
        ਵਾਪਸ ata_process_op(op);
    ਹਾਲਤ DTYPE_RAMDISK:
        ਵਾਪਸ ramdisk_process_op(op);
    ਹਾਲਤ DTYPE_CDEMU:
        ਵਾਪਸ cdemu_process_op(op);
    ਮੂਲ:
        ਵਾਪਸ process_op_both(op);
    }
}

// Execute a disk_op_s request.
ਪੂਰਨ
process_op(ਬਣਤਰ disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->ਗਿਣਤੀ_2, op->command);

    ਪੂਰਨ ret, origcount = op->ਗਿਣਤੀ_2;
    ਜੇ (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->ਗਿਣਤੀ_2 = 0;
        ਵਾਪਸ DISK_RET_EBOUNDARY;
    }
    ਜੇ (MODESEGMENT)
        ret = process_op_16(op);
    ਨਹੀਂਤਾਂ
        ret = process_op_32(op);
    ਜੇ (ret && op->ਗਿਣਤੀ_2 == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->ਗਿਣਤੀ_2 = 0;
    ਵਾਪਸ ret;
}
