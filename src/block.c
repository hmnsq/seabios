// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "block.h" // process_op
#شامل "hw/ata.h" // process_ata_op
#شامل "hw/ahci.h" // process_ahci_op
#شامل "hw/esp-scsi.h" // esp_scsi_process_op
#شامل "hw/lsi-scsi.h" // lsi_scsi_process_op
#شامل "hw/megasas.h" // megasas_process_op
#شامل "hw/mpt-scsi.h" // mpt_scsi_process_op
#شامل "hw/pci.h" // pci_bdf_to_bus
#شامل "hw/pvscsi.h" // pvscsi_process_op
#شامل "hw/rtc.h" // rtc_read
#شامل "hw/usb-msc.h" // usb_process_op
#شامل "hw/usb-uas.h" // uas_process_op
#شامل "hw/virtio-blk.h" // process_virtio_blk_op
#شامل "hw/virtio-scsi.h" // virtio_scsi_process_op
#شامل "hw/nvme.h" // nvme_process_op
#شامل "malloc.h" // malloc_low
#شامل "output.h" // dprintf
#شامل "stacks.h" // call32
#شامل "std/disk.h" // struct dpte_s
#شامل "string.h" // checksum
#شامل "util.h" // process_floppy_op

u8 Floppy_شمار VARFSEG;
u8 CDCount;
ساختار drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

ساختار drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    اگر (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        بازگشت پوچ;
    بازگشت GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

صحیح getDriveId(u8 exttype, ساختار drive_s *drive)
{
    ASSERT32FLAT();
    صحیح i;
    برای (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        اگر (getDrive(exttype, i) == drive)
            بازگشت i;
    بازگشت -1;
}

صحیح create_bounce_buf(تهی)
{
    اگر (bounce_buf_fl)
        بازگشت 0;

    u8 *buf = malloc_کم(CDROM_SECTOR_SIZE);
    اگر (!buf) {
        warn_noalloc();
        بازگشت -1;
    }
    bounce_buf_fl = buf;
    بازگشت 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

ایستا صحیح
host_lchs_supplied(ساختار drive_s *drive)
{
    بازگشت (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

ایستا u8
get_translation(ساختار drive_s *drive)
{
    اگر (host_lchs_supplied(drive))
        بازگشت TRANSLATION_HOST;
    u8 type = drive->type;
    اگر (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        بازگشت translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    اگر (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        بازگشت TRANSLATION_LBA;

    اگر (cylinders <= 1024 && heads <= 16 && spt <= 63)
        بازگشت TRANSLATION_NONE;
    اگر (cylinders * heads <= 131072)
        بازگشت TRANSLATION_LARGE;
    بازگشت TRANSLATION_LBA;
}

ایستا تهی
setup_translation(ساختار drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ثابت نویسه *desc = پوچ;

    انتخاب (translation) {
    پیشفرض:
    حالت TRANSLATION_NONE:
        desc = "none";
        بشکن;
    حالت TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        اگر (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            بشکن;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        اگر (heads>128)
            heads = 255;
        وگرنه اگر (heads>64)
            heads = 128;
        وگرنه اگر (heads>32)
            heads = 64;
        وگرنه اگر (heads>16)
            heads = 32;
        وگرنه
            heads = 16;
        cylinders = sect / heads;
        بشکن;
    حالت TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        اگر (heads==16) {
            اگر (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    حالت TRANSLATION_LARGE:
        اگر (translation == TRANSLATION_LARGE)
            desc = "large";
        وقتی (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            اگر (heads > 127)
                بشکن;
        }
        بشکن;
    حالت TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        بشکن;
    }
    // clip to 1024 cylinders in lchs
    اگر (cylinders > 1024)
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
ایستا تهی
fill_fdpt(ساختار drive_s *drive, صحیح hdid)
{
    اگر (hdid > 1)
        بازگشت;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    ساختار fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    اگر (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, اندازه(*fdpt));
    }

    اگر (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 ساختار extended_bios_data_area_s, fdpt[0])));
    وگرنه
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 ساختار extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
ایستا تهی
add_drive(ساختار drive_s **idmap, u8 *شمار, ساختار drive_s *drive)
{
    اگر (*شمار >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        بازگشت;
    }
    idmap[*شمار] = drive;
    *شمار = *شمار + 1;
}

// Map a hard drive
تهی
map_hd_drive(ساختار drive_s *drive)
{
    ASSERT32FLAT();
    ساختار bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    صحیح hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
تهی
map_cd_drive(ساختار drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
تهی
map_floppy_drive(ساختار drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_شمار, drive);

    // Update equipment word bits for floppy
    اگر (Floppy_شمار == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } وگرنه اگر (Floppy_شمار >= 2) {
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
ایستا صحیح
fill_generic_edd(ساختار segoff_s edd, ساختار drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    ساختار int13dpt_s *param_far = (تهی*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    اگر (size < 26)
        بازگشت DISK_RET_EPARAM;

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
    اگر (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } وگرنه {
        اگر (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } وگرنه {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_شمار, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    اگر (size < 30 || !dpte_so)
        بازگشت DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    اگر (size < 66 || !bus_iface)
        بازگشت DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->کلید, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    ثابت نویسه *host_bus = "ISA ";
    اگر ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        اگر (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , اندازه(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    ثابت نویسه *iface_type = "ATA     ";
    اگر ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , اندازه(param_far->iface_type));
    اگر (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (تهی*)param_far+30, 43));
    } وگرنه {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (تهی*)param_far+30, 35));
    }

    بازگشت DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
ایستا u32
edd_pci_path(u16 bdf, u8 channel)
{
    بازگشت (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

ساختار dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
ایستا صحیح
fill_ata_edd(ساختار segoff_s edd, ساختار drive_s *drive_gf)
{
    اگر (!CONFIG_ATA)
        بازگشت DISK_RET_EPARAM;

    // Fill in dpte
    ساختار atadrive_s *adrive_gf = container_of(
        drive_gf, ساختار atadrive_s, drive);
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    صحیح bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    اگر (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        اگر ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            اگر (translation == TRANSLATION_LBA)
                options |= 1<<9;
            اگر (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } وگرنه {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    اگر (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_کم(DefaultDPTE.iobase1, iobase1);
    SET_کم(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_کم(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_کم(DefaultDPTE.unused, 0xcb);
    SET_کم(DefaultDPTE.irq, irq);
    SET_کم(DefaultDPTE.blkcount, 1);
    SET_کم(DefaultDPTE.dma, 0);
    SET_کم(DefaultDPTE.pio, 0);
    SET_کم(DefaultDPTE.options, options);
    SET_کم(DefaultDPTE.reserved, 0);
    SET_کم(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_کم, &DefaultDPTE, 15);
    SET_کم(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    اگر (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    بازگشت fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_کم, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
صحیح noinline
fill_edd(ساختار segoff_s edd, ساختار drive_s *drive_fl)
{
    انتخاب (GET_FLATPTR(drive_fl->type)) {
    حالت DTYPE_ATA:
    حالت DTYPE_ATA_ATAPI:
        بازگشت fill_ata_edd(edd, drive_fl);
    حالت DTYPE_VIRTIO_BLK:
    حالت DTYPE_VIRTIO_SCSI:
        بازگشت fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    پیشفرض:
        بازگشت fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

تهی
block_setup(تهی)
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
صحیح
default_process_op(ساختار disk_op_s *op)
{
    انتخاب (op->command) {
    حالت CMD_FORMAT:
    حالت CMD_RESET:
    حالت CMD_ISREADY:
    حالت CMD_VERIFY:
    حالت CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        بازگشت DISK_RET_SUCCESS;
    پیشفرض:
        بازگشت DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
ایستا صحیح
process_op_both(ساختار disk_op_s *op)
{
    انتخاب (GET_FLATPTR(op->drive_fl->type)) {
    حالت DTYPE_ATA_ATAPI:
        بازگشت ata_atapi_process_op(op);
    حالت DTYPE_USB:
        بازگشت usb_process_op(op);
    حالت DTYPE_UAS:
        بازگشت uas_process_op(op);
    حالت DTYPE_LSI_SCSI:
        بازگشت lsi_scsi_process_op(op);
    حالت DTYPE_ESP_SCSI:
        بازگشت esp_scsi_process_op(op);
    حالت DTYPE_MEGASAS:
        بازگشت megasas_process_op(op);
    حالت DTYPE_MPT_SCSI:
        بازگشت mpt_scsi_process_op(op);
    پیشفرض:
        اگر (!MODESEGMENT)
            بازگشت DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        بازگشت call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
صحیح VISIBLE32FLAT
process_op_32(ساختار disk_op_s *op)
{
    ASSERT32FLAT();
    انتخاب (op->drive_fl->type) {
    حالت DTYPE_VIRTIO_BLK:
        بازگشت virtio_blk_process_op(op);
    حالت DTYPE_AHCI:
        بازگشت ahci_process_op(op);
    حالت DTYPE_AHCI_ATAPI:
        بازگشت ahci_atapi_process_op(op);
    حالت DTYPE_SDCARD:
        بازگشت sdcard_process_op(op);
    حالت DTYPE_USB_32:
        بازگشت usb_process_op(op);
    حالت DTYPE_UAS_32:
        بازگشت uas_process_op(op);
    حالت DTYPE_VIRTIO_SCSI:
        بازگشت virtio_scsi_process_op(op);
    حالت DTYPE_PVSCSI:
        بازگشت pvscsi_process_op(op);
    حالت DTYPE_NVME:
        بازگشت nvme_process_op(op);
    پیشفرض:
        بازگشت process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
ایستا صحیح
process_op_16(ساختار disk_op_s *op)
{
    ASSERT16();
    انتخاب (GET_FLATPTR(op->drive_fl->type)) {
    حالت DTYPE_FLOPPY:
        بازگشت floppy_process_op(op);
    حالت DTYPE_ATA:
        بازگشت ata_process_op(op);
    حالت DTYPE_RAMDISK:
        بازگشت ramdisk_process_op(op);
    حالت DTYPE_CDEMU:
        بازگشت cdemu_process_op(op);
    پیشفرض:
        بازگشت process_op_both(op);
    }
}

// Execute a disk_op_s request.
صحیح
process_op(ساختار disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->شمار, op->command);

    صحیح ret, origcount = op->شمار;
    اگر (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->شمار = 0;
        بازگشت DISK_RET_EBOUNDARY;
    }
    اگر (MODESEGMENT)
        ret = process_op_16(op);
    وگرنه
        ret = process_op_32(op);
    اگر (ret && op->شمار == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->شمار = 0;
    بازگشت ret;
}
