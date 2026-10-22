// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // GET_GLOBAL
#অন্তর্ভাৱ "block.h" // process_op
#অন্তর্ভাৱ "hw/ata.h" // process_ata_op
#অন্তর্ভাৱ "hw/ahci.h" // process_ahci_op
#অন্তর্ভাৱ "hw/esp-scsi.h" // esp_scsi_process_op
#অন্তর্ভাৱ "hw/lsi-scsi.h" // lsi_scsi_process_op
#অন্তর্ভাৱ "hw/megasas.h" // megasas_process_op
#অন্তর্ভাৱ "hw/mpt-scsi.h" // mpt_scsi_process_op
#অন্তর্ভাৱ "hw/pci.h" // pci_bdf_to_bus
#অন্তর্ভাৱ "hw/pvscsi.h" // pvscsi_process_op
#অন্তর্ভাৱ "hw/rtc.h" // rtc_read
#অন্তর্ভাৱ "hw/usb-msc.h" // usb_process_op
#অন্তর্ভাৱ "hw/usb-uas.h" // uas_process_op
#অন্তর্ভাৱ "hw/virtio-blk.h" // process_virtio_blk_op
#অন্তর্ভাৱ "hw/virtio-scsi.h" // virtio_scsi_process_op
#অন্তর্ভাৱ "hw/nvme.h" // nvme_process_op
#অন্তর্ভাৱ "malloc.h" // malloc_low
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "stacks.h" // call32
#অন্তর্ভাৱ "std/disk.h" // struct dpte_s
#অন্তর্ভাৱ "string.h" // checksum
#অন্তর্ভাৱ "util.h" // process_floppy_op

u8 Floppy_গুনতি VARFSEG;
u8 CDCount;
সংরচনা drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

সংরচনা drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    যদি (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        ঘূৰাই রিক্ত;
    ঘূৰাই GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

পূর্ণাঙ্ক getDriveId(u8 exttype, সংরচনা drive_s *drive)
{
    ASSERT32FLAT();
    পূর্ণাঙ্ক i;
    বাবে (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        যদি (getDrive(exttype, i) == drive)
            ঘূৰাই i;
    ঘূৰাই -1;
}

পূর্ণাঙ্ক create_bounce_buf(শূন্য)
{
    যদি (bounce_buf_fl)
        ঘূৰাই 0;

    u8 *buf = malloc_নিচেৰ(CDROM_SECTOR_SIZE);
    যদি (!buf) {
        warn_noalloc();
        ঘূৰাই -1;
    }
    bounce_buf_fl = buf;
    ঘূৰাই 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

স্থির পূর্ণাঙ্ক
host_lchs_supplied(সংরচনা drive_s *drive)
{
    ঘূৰাই (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

স্থির u8
get_translation(সংরচনা drive_s *drive)
{
    যদি (host_lchs_supplied(drive))
        ঘূৰাই TRANSLATION_HOST;
    u8 type = drive->type;
    যদি (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        ঘূৰাই translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    যদি (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        ঘূৰাই TRANSLATION_LBA;

    যদি (cylinders <= 1024 && heads <= 16 && spt <= 63)
        ঘূৰাই TRANSLATION_NONE;
    যদি (cylinders * heads <= 131072)
        ঘূৰাই TRANSLATION_LARGE;
    ঘূৰাই TRANSLATION_LBA;
}

স্থির শূন্য
setup_translation(সংরচনা drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ধ্রুৱ ৱর্ণ *desc = রিক্ত;

    ৱিকল্প (translation) {
    স্ৱভাৱ:
    পক্ষ TRANSLATION_NONE:
        desc = "none";
        ভঙ্গ;
    পক্ষ TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        যদি (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            ভঙ্গ;
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
        ভঙ্গ;
    পক্ষ TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        যদি (heads==16) {
            যদি (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    পক্ষ TRANSLATION_LARGE:
        যদি (translation == TRANSLATION_LARGE)
            desc = "large";
        যেতিয়ালৈ (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            যদি (heads > 127)
                ভঙ্গ;
        }
        ভঙ্গ;
    পক্ষ TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        ভঙ্গ;
    }
    // clip to 1024 cylinders in lchs
    যদি (cylinders > 1024)
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
স্থির শূন্য
fill_fdpt(সংরচনা drive_s *drive, পূর্ণাঙ্ক hdid)
{
    যদি (hdid > 1)
        ঘূৰাই;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    সংরচনা fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
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
        fdpt->checksum -= checksum(fdpt, পরিমাণ(*fdpt));
    }

    যদি (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 সংরচনা extended_bios_data_area_s, fdpt[0])));
    অন্যথা
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 সংরচনা extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
স্থির শূন্য
add_drive(সংরচনা drive_s **idmap, u8 *গুনতি, সংরচনা drive_s *drive)
{
    যদি (*গুনতি >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        ঘূৰাই;
    }
    idmap[*গুনতি] = drive;
    *গুনতি = *গুনতি + 1;
}

// Map a hard drive
শূন্য
map_hd_drive(সংরচনা drive_s *drive)
{
    ASSERT32FLAT();
    সংরচনা bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    পূর্ণাঙ্ক hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
শূন্য
map_cd_drive(সংরচনা drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
শূন্য
map_floppy_drive(সংরচনা drive_s *drive)
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
স্থির পূর্ণাঙ্ক
fill_generic_edd(সংরচনা segoff_s edd, সংরচনা drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    সংরচনা int13dpt_s *param_far = (শূন্য*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    যদি (size < 26)
        ঘূৰাই DISK_RET_EPARAM;

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

    যদি (size < 30 || !dpte_so)
        ঘূৰাই DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    যদি (size < 66 || !bus_iface)
        ঘূৰাই DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->চাবি, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    ধ্রুৱ ৱর্ণ *host_bus = "ISA ";
    যদি ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        যদি (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , পরিমাণ(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    ধ্রুৱ ৱর্ণ *iface_type = "ATA     ";
    যদি ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , পরিমাণ(param_far->iface_type));
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

    ঘূৰাই DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
স্থির u32
edd_pci_path(u16 bdf, u8 channel)
{
    ঘূৰাই (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

সংরচনা dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
স্থির পূর্ণাঙ্ক
fill_ata_edd(সংরচনা segoff_s edd, সংরচনা drive_s *drive_gf)
{
    যদি (!CONFIG_ATA)
        ঘূৰাই DISK_RET_EPARAM;

    // Fill in dpte
    সংরচনা atadrive_s *adrive_gf = container_of(
        drive_gf, সংরচনা atadrive_s, drive);
    সংরচনা ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    পূর্ণাঙ্ক bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
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

    SET_নিচেৰ(DefaultDPTE.iobase1, iobase1);
    SET_নিচেৰ(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_নিচেৰ(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_নিচেৰ(DefaultDPTE.unused, 0xcb);
    SET_নিচেৰ(DefaultDPTE.irq, irq);
    SET_নিচেৰ(DefaultDPTE.blkcount, 1);
    SET_নিচেৰ(DefaultDPTE.dma, 0);
    SET_নিচেৰ(DefaultDPTE.pio, 0);
    SET_নিচেৰ(DefaultDPTE.options, options);
    SET_নিচেৰ(DefaultDPTE.reserved, 0);
    SET_নিচেৰ(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_নিচেৰ, &DefaultDPTE, 15);
    SET_নিচেৰ(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    যদি (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    ঘূৰাই fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_নিচেৰ, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
পূর্ণাঙ্ক noinline
fill_edd(সংরচনা segoff_s edd, সংরচনা drive_s *drive_fl)
{
    ৱিকল্প (GET_FLATPTR(drive_fl->type)) {
    পক্ষ DTYPE_ATA:
    পক্ষ DTYPE_ATA_ATAPI:
        ঘূৰাই fill_ata_edd(edd, drive_fl);
    পক্ষ DTYPE_VIRTIO_BLK:
    পক্ষ DTYPE_VIRTIO_SCSI:
        ঘূৰাই fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    স্ৱভাৱ:
        ঘূৰাই fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
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
পূর্ণাঙ্ক
default_process_op(সংরচনা disk_op_s *op)
{
    ৱিকল্প (op->command) {
    পক্ষ CMD_FORMAT:
    পক্ষ CMD_RESET:
    পক্ষ CMD_ISREADY:
    পক্ষ CMD_VERIFY:
    পক্ষ CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        ঘূৰাই DISK_RET_SUCCESS;
    স্ৱভাৱ:
        ঘূৰাই DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
স্থির পূর্ণাঙ্ক
process_op_both(সংরচনা disk_op_s *op)
{
    ৱিকল্প (GET_FLATPTR(op->drive_fl->type)) {
    পক্ষ DTYPE_ATA_ATAPI:
        ঘূৰাই ata_atapi_process_op(op);
    পক্ষ DTYPE_USB:
        ঘূৰাই usb_process_op(op);
    পক্ষ DTYPE_UAS:
        ঘূৰাই uas_process_op(op);
    পক্ষ DTYPE_LSI_SCSI:
        ঘূৰাই lsi_scsi_process_op(op);
    পক্ষ DTYPE_ESP_SCSI:
        ঘূৰাই esp_scsi_process_op(op);
    পক্ষ DTYPE_MEGASAS:
        ঘূৰাই megasas_process_op(op);
    পক্ষ DTYPE_MPT_SCSI:
        ঘূৰাই mpt_scsi_process_op(op);
    স্ৱভাৱ:
        যদি (!MODESEGMENT)
            ঘূৰাই DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        ঘূৰাই call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
পূর্ণাঙ্ক VISIBLE32FLAT
process_op_32(সংরচনা disk_op_s *op)
{
    ASSERT32FLAT();
    ৱিকল্প (op->drive_fl->type) {
    পক্ষ DTYPE_VIRTIO_BLK:
        ঘূৰাই virtio_blk_process_op(op);
    পক্ষ DTYPE_AHCI:
        ঘূৰাই ahci_process_op(op);
    পক্ষ DTYPE_AHCI_ATAPI:
        ঘূৰাই ahci_atapi_process_op(op);
    পক্ষ DTYPE_SDCARD:
        ঘূৰাই sdcard_process_op(op);
    পক্ষ DTYPE_USB_32:
        ঘূৰাই usb_process_op(op);
    পক্ষ DTYPE_UAS_32:
        ঘূৰাই uas_process_op(op);
    পক্ষ DTYPE_VIRTIO_SCSI:
        ঘূৰাই virtio_scsi_process_op(op);
    পক্ষ DTYPE_PVSCSI:
        ঘূৰাই pvscsi_process_op(op);
    পক্ষ DTYPE_NVME:
        ঘূৰাই nvme_process_op(op);
    স্ৱভাৱ:
        ঘূৰাই process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
স্থির পূর্ণাঙ্ক
process_op_16(সংরচনা disk_op_s *op)
{
    ASSERT16();
    ৱিকল্প (GET_FLATPTR(op->drive_fl->type)) {
    পক্ষ DTYPE_FLOPPY:
        ঘূৰাই floppy_process_op(op);
    পক্ষ DTYPE_ATA:
        ঘূৰাই ata_process_op(op);
    পক্ষ DTYPE_RAMDISK:
        ঘূৰাই ramdisk_process_op(op);
    পক্ষ DTYPE_CDEMU:
        ঘূৰাই cdemu_process_op(op);
    স্ৱভাৱ:
        ঘূৰাই process_op_both(op);
    }
}

// Execute a disk_op_s request.
পূর্ণাঙ্ক
process_op(সংরচনা disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->গুনতি, op->command);

    পূর্ণাঙ্ক ret, origcount = op->গুনতি;
    যদি (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->গুনতি = 0;
        ঘূৰাই DISK_RET_EBOUNDARY;
    }
    যদি (MODESEGMENT)
        ret = process_op_16(op);
    অন্যথা
        ret = process_op_32(op);
    যদি (ret && op->গুনতি == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->গুনতি = 0;
    ঘূৰাই ret;
}
