// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "block.h" // process_op
#অন্তর্ভাব "hw/ata.h" // process_ata_op
#অন্তর্ভাব "hw/ahci.h" // process_ahci_op
#অন্তর্ভাব "hw/esp-scsi.h" // esp_scsi_process_op
#অন্তর্ভাব "hw/lsi-scsi.h" // lsi_scsi_process_op
#অন্তর্ভাব "hw/megasas.h" // megasas_process_op
#অন্তর্ভাব "hw/mpt-scsi.h" // mpt_scsi_process_op
#অন্তর্ভাব "hw/pci.h" // pci_bdf_to_bus
#অন্তর্ভাব "hw/pvscsi.h" // pvscsi_process_op
#অন্তর্ভাব "hw/rtc.h" // rtc_read
#অন্তর্ভাব "hw/usb-msc.h" // usb_process_op
#অন্তর্ভাব "hw/usb-uas.h" // uas_process_op
#অন্তর্ভাব "hw/virtio-blk.h" // process_virtio_blk_op
#অন্তর্ভাব "hw/virtio-scsi.h" // virtio_scsi_process_op
#অন্তর্ভাব "hw/nvme.h" // nvme_process_op
#অন্তর্ভাব "malloc.h" // malloc_low
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "stacks.h" // call32
#অন্তর্ভাব "std/disk.h" // struct dpte_s
#অন্তর্ভাব "string.h" // checksum
#অন্তর্ভাব "util.h" // process_floppy_op

u8 Floppy_গুনতি VARFSEG;
u8 CDCount;
কাঠামো drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

কাঠামো drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    যদি (extdriveoffset >= ARRAY_আকার(IDMap[0]))
        ফেরত ফাঁকা;
    ফেরত GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

পূর্ণসংখ্যা getDriveId(u8 exttype, কাঠামো drive_s *drive)
{
    ASSERT32FLAT();
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < ARRAY_আকার(IDMap[0]); i++)
        যদি (getDrive(exttype, i) == drive)
            ফেরত i;
    ফেরত -1;
}

পূর্ণসংখ্যা create_bounce_buf(শূন্য)
{
    যদি (bounce_buf_fl)
        ফেরত 0;

    u8 *buf = malloc_নিচের(CDROM_SECTOR_আকার);
    যদি (!buf) {
        warn_noalloc();
        ফেরত -1;
    }
    bounce_buf_fl = buf;
    ফেরত 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

স্থির পূর্ণসংখ্যা
host_lchs_supplied(কাঠামো drive_s *drive)
{
    ফেরত (drive->lchs.শীর্ষ <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

স্থির u8
get_translation(কাঠামো drive_s *drive)
{
    যদি (host_lchs_supplied(drive))
        ফেরত TRANSLATION_HOST;
    u8 type = drive->type;
    যদি (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        ফেরত translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.শীর্ষ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    যদি (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        ফেরত TRANSLATION_LBA;

    যদি (cylinders <= 1024 && heads <= 16 && spt <= 63)
        ফেরত TRANSLATION_NONE;
    যদি (cylinders * heads <= 131072)
        ফেরত TRANSLATION_LARGE;
    ফেরত TRANSLATION_LBA;
}

স্থির শূন্য
setup_translation(কাঠামো drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.শীর্ষ ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ধ্রুবক অক্ষর *desc = ফাঁকা;

    নির্বাচন (translation) {
    পূর্বনির্ধারিত:
    ক্ষেত্র TRANSLATION_NONE:
        desc = "none";
        বিরতি;
    ক্ষেত্র TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        যদি (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            বিরতি;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        যদি (heads>128)
            heads = 255;
        অন্যথা যদি (heads>64)
            heads = 128;
        অন্যথা যদি (heads>32)
            heads = 64;
        অন্যথা যদি (heads>16)
            heads = 32;
        অন্যথা
            heads = 16;
        cylinders = sect / heads;
        বিরতি;
    ক্ষেত্র TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        যদি (heads==16) {
            যদি (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    ক্ষেত্র TRANSLATION_LARGE:
        যদি (translation == TRANSLATION_LARGE)
            desc = "large";
        যখন (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            যদি (heads > 127)
                বিরতি;
        }
        বিরতি;
    ক্ষেত্র TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.শীর্ষ;
        spt = drive->lchs.sector;
        বিরতি;
    }
    // clip to 1024 cylinders in lchs
    যদি (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%u\n"
            , drive
            , drive->pchs.cylinder, drive->pchs.শীর্ষ, drive->pchs.sector
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    drive->lchs.শীর্ষ = heads;
    drive->lchs.cylinder = cylinders;
    drive->lchs.sector = spt;
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
স্থির শূন্য
fill_fdpt(কাঠামো drive_s *drive, পূর্ণসংখ্যা hdid)
{
    যদি (hdid > 1)
        ফেরত;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.শীর্ষ;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.শীর্ষ;
    u16 nps = drive->pchs.sector;

    কাঠামো fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    যদি (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, আকার(*fdpt));
    }

    যদি (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 কাঠামো extended_bios_তথ্য_area_s, fdpt[0])));
    অন্যথা
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 কাঠামো extended_bios_তথ্য_area_s, fdpt[1])));
}

// Find spot to add a drive
স্থির শূন্য
add_drive(কাঠামো drive_s **idmap, u8 *গুনতি, কাঠামো drive_s *drive)
{
    যদি (*গুনতি >= ARRAY_আকার(IDMap[0])) {
        warn_noalloc();
        ফেরত;
    }
    idmap[*গুনতি] = drive;
    *গুনতি = *গুনতি + 1;
}

// Map a hard drive
শূন্য
map_hd_drive(কাঠামো drive_s *drive)
{
    ASSERT32FLAT();
    কাঠামো bios_তথ্য_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    পূর্ণসংখ্যা hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
শূন্য
map_cd_drive(কাঠামো drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
শূন্য
map_floppy_drive(কাঠামো drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_গুনতি, drive);

    // Update equipment word bits for floppy
    যদি (Floppy_গুনতি == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } অন্যথা যদি (Floppy_গুনতি >= 2) {
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
স্থির পূর্ণসংখ্যা
fill_generic_edd(কাঠামো segoff_s edd, কাঠামো drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    কাঠামো int13dpt_s *param_far = (শূন্য*)(edd.offset+0);
    u16 আকার_2 = GET_FARVAR(seg, param_far->আকার_2);
    u16 t13 = আকার_2 == 74;

    // Buffer is too small
    যদি (আকার_2 < 26)
        ফেরত DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.শীর্ষ);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , আকার_2, type, npc, nph, nps, (u32)lba, blksize);

    SET_FARVAR(seg, param_far->আকার_2, 26);
    যদি (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } অন্যথা {
        যদি (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } অন্যথা {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_গুনতি, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    যদি (আকার_2 < 30 || !dpte_so)
        ফেরত DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->আকার_2, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    যদি (আকার_2 < 66 || !bus_iface)
        ফেরত DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->চাবি, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_দৈর্ঘ্য, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    ধ্রুবক অক্ষর *host_bus = "ISA ";
    যদি ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        যদি (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , আকার(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    ধ্রুবক অক্ষর *iface_type = "ATA     ";
    যদি ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , আকার(param_far->iface_type));
    যদি (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (শূন্য*)param_far+30, 43));
    } অন্যথা {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (শূন্য*)param_far+30, 35));
    }

    ফেরত DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
স্থির u32
edd_pci_path(u16 bdf, u8 channel)
{
    ফেরত (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

কাঠামো dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
স্থির পূর্ণসংখ্যা
fill_ata_edd(কাঠামো segoff_s edd, কাঠামো drive_s *drive_gf)
{
    যদি (!CONFIG_ATA)
        ফেরত DISK_RET_EPARAM;

    // Fill in dpte
    কাঠামো atadrive_s *adrive_gf = container_of(
        drive_gf, কাঠামো atadrive_s, drive);
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    পূর্ণসংখ্যা bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    যদি (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        যদি ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            যদি (translation == TRANSLATION_LBA)
                options |= 1<<9;
            যদি (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } অন্যথা {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    যদি (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_নিচের(DefaultDPTE.iobase1, iobase1);
    SET_নিচের(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_নিচের(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_নিচের(DefaultDPTE.unused, 0xcb);
    SET_নিচের(DefaultDPTE.irq, irq);
    SET_নিচের(DefaultDPTE.blkcount, 1);
    SET_নিচের(DefaultDPTE.dma, 0);
    SET_নিচের(DefaultDPTE.pio, 0);
    SET_নিচের(DefaultDPTE.options, options);
    SET_নিচের(DefaultDPTE.reserved, 0);
    SET_নিচের(DefaultDPTE.revision, 0x11);

    u8 যোগ = checksum_far(SEG_নিচের, &DefaultDPTE, 15);
    SET_নিচের(DefaultDPTE.checksum, -যোগ);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    যদি (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    ফেরত fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_নিচের, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
পূর্ণসংখ্যা noinline
fill_edd(কাঠামো segoff_s edd, কাঠামো drive_s *drive_fl)
{
    নির্বাচন (GET_FLATPTR(drive_fl->type)) {
    ক্ষেত্র DTYPE_ATA:
    ক্ষেত্র DTYPE_ATA_ATAPI:
        ফেরত fill_ata_edd(edd, drive_fl);
    ক্ষেত্র DTYPE_VIRTIO_BLK:
    ক্ষেত্র DTYPE_VIRTIO_SCSI:
        ফেরত fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    পূর্বনির্ধারিত:
        ফেরত fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

শূন্য
block_setup(শূন্য)
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
পূর্ণসংখ্যা
default_process_op(কাঠামো disk_op_s *op)
{
    নির্বাচন (op->command) {
    ক্ষেত্র CMD_FORMAT:
    ক্ষেত্র CMD_RESET:
    ক্ষেত্র CMD_ISREADY:
    ক্ষেত্র CMD_VERIFY:
    ক্ষেত্র CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        ফেরত DISK_RET_SUCCESS;
    পূর্বনির্ধারিত:
        ফেরত DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
স্থির পূর্ণসংখ্যা
process_op_both(কাঠামো disk_op_s *op)
{
    নির্বাচন (GET_FLATPTR(op->drive_fl->type)) {
    ক্ষেত্র DTYPE_ATA_ATAPI:
        ফেরত ata_atapi_process_op(op);
    ক্ষেত্র DTYPE_USB:
        ফেরত usb_process_op(op);
    ক্ষেত্র DTYPE_UAS:
        ফেরত uas_process_op(op);
    ক্ষেত্র DTYPE_LSI_SCSI:
        ফেরত lsi_scsi_process_op(op);
    ক্ষেত্র DTYPE_ESP_SCSI:
        ফেরত esp_scsi_process_op(op);
    ক্ষেত্র DTYPE_MEGASAS:
        ফেরত megasas_process_op(op);
    ক্ষেত্র DTYPE_MPT_SCSI:
        ফেরত mpt_scsi_process_op(op);
    পূর্বনির্ধারিত:
        যদি (!MODESEGMENT)
            ফেরত DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        ফেরত call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
পূর্ণসংখ্যা VISIBLE32FLAT
process_op_32(কাঠামো disk_op_s *op)
{
    ASSERT32FLAT();
    নির্বাচন (op->drive_fl->type) {
    ক্ষেত্র DTYPE_VIRTIO_BLK:
        ফেরত virtio_blk_process_op(op);
    ক্ষেত্র DTYPE_AHCI:
        ফেরত ahci_process_op(op);
    ক্ষেত্র DTYPE_AHCI_ATAPI:
        ফেরত ahci_atapi_process_op(op);
    ক্ষেত্র DTYPE_SDCARD:
        ফেরত sdcard_process_op(op);
    ক্ষেত্র DTYPE_USB_32:
        ফেরত usb_process_op(op);
    ক্ষেত্র DTYPE_UAS_32:
        ফেরত uas_process_op(op);
    ক্ষেত্র DTYPE_VIRTIO_SCSI:
        ফেরত virtio_scsi_process_op(op);
    ক্ষেত্র DTYPE_PVSCSI:
        ফেরত pvscsi_process_op(op);
    ক্ষেত্র DTYPE_NVME:
        ফেরত nvme_process_op(op);
    পূর্বনির্ধারিত:
        ফেরত process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
স্থির পূর্ণসংখ্যা
process_op_16(কাঠামো disk_op_s *op)
{
    ASSERT16();
    নির্বাচন (GET_FLATPTR(op->drive_fl->type)) {
    ক্ষেত্র DTYPE_FLOPPY:
        ফেরত floppy_process_op(op);
    ক্ষেত্র DTYPE_ATA:
        ফেরত ata_process_op(op);
    ক্ষেত্র DTYPE_RAMDISK:
        ফেরত ramdisk_process_op(op);
    ক্ষেত্র DTYPE_CDEMU:
        ফেরত cdemu_process_op(op);
    পূর্বনির্ধারিত:
        ফেরত process_op_both(op);
    }
}

// Execute a disk_op_s request.
পূর্ণসংখ্যা
process_op(কাঠামো disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->গুনতি, op->command);

    পূর্ণসংখ্যা ret, origcount = op->গুনতি;
    যদি (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->গুনতি = 0;
        ফেরত DISK_RET_EBOUNDARY;
    }
    যদি (MODESEGMENT)
        ret = process_op_16(op);
    অন্যথা
        ret = process_op_32(op);
    যদি (ret && op->গুনতি == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->গুনতি = 0;
    ফেরত ret;
}
