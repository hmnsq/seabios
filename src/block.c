// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "block.h" // process_op
#ضمن "hw/ata.h" // process_ata_op
#ضمن "hw/ahci.h" // process_ahci_op
#ضمن "hw/esp-scsi.h" // esp_scsi_process_op
#ضمن "hw/lsi-scsi.h" // lsi_scsi_process_op
#ضمن "hw/megasas.h" // megasas_process_op
#ضمن "hw/mpt-scsi.h" // mpt_scsi_process_op
#ضمن "hw/pci.h" // pci_bdf_to_bus
#ضمن "hw/pvscsi.h" // pvscsi_process_op
#ضمن "hw/rtc.h" // rtc_read
#ضمن "hw/usb-msc.h" // usb_process_op
#ضمن "hw/usb-uas.h" // uas_process_op
#ضمن "hw/virtio-blk.h" // process_virtio_blk_op
#ضمن "hw/virtio-scsi.h" // virtio_scsi_process_op
#ضمن "hw/nvme.h" // nvme_process_op
#ضمن "malloc.h" // malloc_low
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // call32
#ضمن "std/disk.h" // struct dpte_s
#ضمن "string.h" // checksum
#ضمن "util.h" // process_floppy_op

u8 FloppyCount VARFSEG;
u8 CDCount;
بنية drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

بنية drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    אם (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        החזר عدم;
    החזר GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

صحيح getDriveId(u8 exttype, بنية drive_s *drive)
{
    ASSERT32FLAT();
    صحيح i;
    עבור (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        אם (getDrive(exttype, i) == drive)
            החזר i;
    החזר -1;
}

صحيح create_bounce_buf(فراغ)
{
    אם (bounce_buf_fl)
        החזר 0;

    u8 *buf = malloc_low(CDROM_SECTOR_SIZE);
    אם (!buf) {
        warn_noalloc();
        החזר -1;
    }
    bounce_buf_fl = buf;
    החזר 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

ساكن صحيح
host_lchs_supplied(بنية drive_s *drive)
{
    החזר (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

ساكن u8
get_translation(بنية drive_s *drive)
{
    אם (host_lchs_supplied(drive))
        החזר TRANSLATION_HOST;
    u8 type = drive->type;
    אם (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        החזר translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    אם (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        החזר TRANSLATION_LBA;

    אם (cylinders <= 1024 && heads <= 16 && spt <= 63)
        החזר TRANSLATION_NONE;
    אם (cylinders * heads <= 131072)
        החזר TRANSLATION_LARGE;
    החזר TRANSLATION_LBA;
}

ساكن فراغ
setup_translation(بنية drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ثابت حرف *desc = عدم;

    بدل (translation) {
    افتراضي:
    حالة TRANSLATION_NONE:
        desc = "none";
        اكسر;
    حالة TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        אם (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            اكسر;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        אם (heads>128)
            heads = 255;
        אחרת אם (heads>64)
            heads = 128;
        אחרת אם (heads>32)
            heads = 64;
        אחרת אם (heads>16)
            heads = 32;
        אחרת
            heads = 16;
        cylinders = sect / heads;
        اكسر;
    حالة TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        אם (heads==16) {
            אם (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    حالة TRANSLATION_LARGE:
        אם (translation == TRANSLATION_LARGE)
            desc = "large";
        כלעוד (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            אם (heads > 127)
                اكسر;
        }
        اكسر;
    حالة TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        اكسر;
    }
    // clip to 1024 cylinders in lchs
    אם (cylinders > 1024)
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
ساكن فراغ
fill_fdpt(بنية drive_s *drive, صحيح hdid)
{
    אם (hdid > 1)
        החזר;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    بنية fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    אם (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, حجم(*fdpt));
    }

    אם (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 بنية extended_bios_data_area_s, fdpt[0])));
    אחרת
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 بنية extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
ساكن فراغ
add_drive(بنية drive_s **idmap, u8 *count, بنية drive_s *drive)
{
    אם (*count >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        החזר;
    }
    idmap[*count] = drive;
    *count = *count + 1;
}

// Map a hard drive
فراغ
map_hd_drive(بنية drive_s *drive)
{
    ASSERT32FLAT();
    بنية bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    صحيح hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
فراغ
map_cd_drive(بنية drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
فراغ
map_floppy_drive(بنية drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &FloppyCount, drive);

    // Update equipment word bits for floppy
    אם (FloppyCount == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } אחרת אם (FloppyCount >= 2) {
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
ساكن صحيح
fill_generic_edd(بنية segoff_s edd, بنية drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    بنية int13dpt_s *param_far = (فراغ*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    אם (size < 26)
        החזר DISK_RET_EPARAM;

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
    אם (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } אחרת {
        אם (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } אחרת {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_count, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    אם (size < 30 || !dpte_so)
        החזר DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    אם (size < 66 || !bus_iface)
        החזר DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->key, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    ثابت حرف *host_bus = "ISA ";
    אם ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        אם (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , حجم(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    ثابت حرف *iface_type = "ATA     ";
    אם ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , حجم(param_far->iface_type));
    אם (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (فراغ*)param_far+30, 43));
    } אחרת {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (فراغ*)param_far+30, 35));
    }

    החזר DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
ساكن u32
edd_pci_path(u16 bdf, u8 channel)
{
    החזר (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

بنية dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
ساكن صحيح
fill_ata_edd(بنية segoff_s edd, بنية drive_s *drive_gf)
{
    אם (!CONFIG_ATA)
        החזר DISK_RET_EPARAM;

    // Fill in dpte
    بنية atadrive_s *adrive_gf = container_of(
        drive_gf, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    صحيح bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    אם (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        אם ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            אם (translation == TRANSLATION_LBA)
                options |= 1<<9;
            אם (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } אחרת {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    אם (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_LOW(DefaultDPTE.iobase1, iobase1);
    SET_LOW(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_LOW(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_LOW(DefaultDPTE.unused, 0xcb);
    SET_LOW(DefaultDPTE.irq, irq);
    SET_LOW(DefaultDPTE.blkcount, 1);
    SET_LOW(DefaultDPTE.dma, 0);
    SET_LOW(DefaultDPTE.pio, 0);
    SET_LOW(DefaultDPTE.options, options);
    SET_LOW(DefaultDPTE.reserved, 0);
    SET_LOW(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_LOW, &DefaultDPTE, 15);
    SET_LOW(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    אם (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    החזר fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_LOW, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
صحيح noinline
fill_edd(بنية segoff_s edd, بنية drive_s *drive_fl)
{
    بدل (GET_FLATPTR(drive_fl->type)) {
    حالة DTYPE_ATA:
    حالة DTYPE_ATA_ATAPI:
        החזר fill_ata_edd(edd, drive_fl);
    حالة DTYPE_VIRTIO_BLK:
    حالة DTYPE_VIRTIO_SCSI:
        החזר fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    افتراضي:
        החזר fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

فراغ
block_setup(فراغ)
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
صحيح
default_process_op(بنية disk_op_s *op)
{
    بدل (op->command) {
    حالة CMD_FORMAT:
    حالة CMD_RESET:
    حالة CMD_ISREADY:
    حالة CMD_VERIFY:
    حالة CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        החזר DISK_RET_SUCCESS;
    افتراضي:
        החזר DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
ساكن صحيح
process_op_both(بنية disk_op_s *op)
{
    بدل (GET_FLATPTR(op->drive_fl->type)) {
    حالة DTYPE_ATA_ATAPI:
        החזר ata_atapi_process_op(op);
    حالة DTYPE_USB:
        החזר usb_process_op(op);
    حالة DTYPE_UAS:
        החזר uas_process_op(op);
    حالة DTYPE_LSI_SCSI:
        החזר lsi_scsi_process_op(op);
    حالة DTYPE_ESP_SCSI:
        החזר esp_scsi_process_op(op);
    حالة DTYPE_MEGASAS:
        החזר megasas_process_op(op);
    حالة DTYPE_MPT_SCSI:
        החזר mpt_scsi_process_op(op);
    افتراضي:
        אם (!MODESEGMENT)
            החזר DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        החזר call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
صحيح VISIBLE32FLAT
process_op_32(بنية disk_op_s *op)
{
    ASSERT32FLAT();
    بدل (op->drive_fl->type) {
    حالة DTYPE_VIRTIO_BLK:
        החזר virtio_blk_process_op(op);
    حالة DTYPE_AHCI:
        החזר ahci_process_op(op);
    حالة DTYPE_AHCI_ATAPI:
        החזר ahci_atapi_process_op(op);
    حالة DTYPE_SDCARD:
        החזר sdcard_process_op(op);
    حالة DTYPE_USB_32:
        החזר usb_process_op(op);
    حالة DTYPE_UAS_32:
        החזר uas_process_op(op);
    حالة DTYPE_VIRTIO_SCSI:
        החזר virtio_scsi_process_op(op);
    حالة DTYPE_PVSCSI:
        החזר pvscsi_process_op(op);
    حالة DTYPE_NVME:
        החזר nvme_process_op(op);
    افتراضي:
        החזר process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
ساكن صحيح
process_op_16(بنية disk_op_s *op)
{
    ASSERT16();
    بدل (GET_FLATPTR(op->drive_fl->type)) {
    حالة DTYPE_FLOPPY:
        החזר floppy_process_op(op);
    حالة DTYPE_ATA:
        החזר ata_process_op(op);
    حالة DTYPE_RAMDISK:
        החזר ramdisk_process_op(op);
    حالة DTYPE_CDEMU:
        החזר cdemu_process_op(op);
    افتراضي:
        החזר process_op_both(op);
    }
}

// Execute a disk_op_s request.
صحيح
process_op(بنية disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->count, op->command);

    صحيح ret, origcount = op->count;
    אם (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->count = 0;
        החזר DISK_RET_EBOUNDARY;
    }
    אם (MODESEGMENT)
        ret = process_op_16(op);
    אחרת
        ret = process_op_32(op);
    אם (ret && op->count == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->count = 0;
    החזר ret;
}
