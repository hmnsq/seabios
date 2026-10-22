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

u8 Floppy_عدد VARFSEG;
u8 CDCount;
بنية drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

بنية drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    إذا (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        أرجع عدم;
    أرجع GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

صحيح getDriveId(u8 exttype, بنية drive_s *drive)
{
    ASSERT32FLAT();
    صحيح i;
    لكل (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        إذا (getDrive(exttype, i) == drive)
            أرجع i;
    أرجع -1;
}

صحيح create_bounce_buf(فراغ)
{
    إذا (bounce_buf_fl)
        أرجع 0;

    u8 *buf = malloc_أدنى(CDROM_SECTOR_SIZE);
    إذا (!buf) {
        warn_noalloc();
        أرجع -1;
    }
    bounce_buf_fl = buf;
    أرجع 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

ساكن صحيح
host_lchs_supplied(بنية drive_s *drive)
{
    أرجع (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

ساكن u8
get_translation(بنية drive_s *drive)
{
    إذا (host_lchs_supplied(drive))
        أرجع TRANSLATION_HOST;
    u8 type = drive->type;
    إذا (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        أرجع translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    إذا (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        أرجع TRANSLATION_LBA;

    إذا (cylinders <= 1024 && heads <= 16 && spt <= 63)
        أرجع TRANSLATION_NONE;
    إذا (cylinders * heads <= 131072)
        أرجع TRANSLATION_LARGE;
    أرجع TRANSLATION_LBA;
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
        إذا (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            اكسر;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        إذا (heads>128)
            heads = 255;
        وإلا إذا (heads>64)
            heads = 128;
        وإلا إذا (heads>32)
            heads = 64;
        وإلا إذا (heads>16)
            heads = 32;
        وإلا
            heads = 16;
        cylinders = sect / heads;
        اكسر;
    حالة TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        إذا (heads==16) {
            إذا (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    حالة TRANSLATION_LARGE:
        إذا (translation == TRANSLATION_LARGE)
            desc = "large";
        طالما (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            إذا (heads > 127)
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
    إذا (cylinders > 1024)
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
    إذا (hdid > 1)
        أرجع;

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

    إذا (nlc != npc || nlh != nph || nls != nps) {
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

    إذا (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 بنية extended_bios_data_area_s, fdpt[0])));
    وإلا
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 بنية extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
ساكن فراغ
add_drive(بنية drive_s **idmap, u8 *عدد, بنية drive_s *drive)
{
    إذا (*عدد >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        أرجع;
    }
    idmap[*عدد] = drive;
    *عدد = *عدد + 1;
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
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_عدد, drive);

    // Update equipment word bits for floppy
    إذا (Floppy_عدد == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } وإلا إذا (Floppy_عدد >= 2) {
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
    إذا (size < 26)
        أرجع DISK_RET_EPARAM;

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
    إذا (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } وإلا {
        إذا (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } وإلا {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_عدد, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    إذا (size < 30 || !dpte_so)
        أرجع DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    إذا (size < 66 || !bus_iface)
        أرجع DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->مفتاح, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    ثابت حرف *host_bus = "ISA ";
    إذا ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        إذا (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , حجم(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    ثابت حرف *iface_type = "ATA     ";
    إذا ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , حجم(param_far->iface_type));
    إذا (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (فراغ*)param_far+30, 43));
    } وإلا {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (فراغ*)param_far+30, 35));
    }

    أرجع DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
ساكن u32
edd_pci_path(u16 bdf, u8 channel)
{
    أرجع (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

بنية dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
ساكن صحيح
fill_ata_edd(بنية segoff_s edd, بنية drive_s *drive_gf)
{
    إذا (!CONFIG_ATA)
        أرجع DISK_RET_EPARAM;

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
    إذا (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        إذا ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            إذا (translation == TRANSLATION_LBA)
                options |= 1<<9;
            إذا (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } وإلا {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    إذا (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_أدنى(DefaultDPTE.iobase1, iobase1);
    SET_أدنى(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_أدنى(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_أدنى(DefaultDPTE.unused, 0xcb);
    SET_أدنى(DefaultDPTE.irq, irq);
    SET_أدنى(DefaultDPTE.blkcount, 1);
    SET_أدنى(DefaultDPTE.dma, 0);
    SET_أدنى(DefaultDPTE.pio, 0);
    SET_أدنى(DefaultDPTE.options, options);
    SET_أدنى(DefaultDPTE.reserved, 0);
    SET_أدنى(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_أدنى, &DefaultDPTE, 15);
    SET_أدنى(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    إذا (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    أرجع fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_أدنى, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
صحيح noinline
fill_edd(بنية segoff_s edd, بنية drive_s *drive_fl)
{
    بدل (GET_FLATPTR(drive_fl->type)) {
    حالة DTYPE_ATA:
    حالة DTYPE_ATA_ATAPI:
        أرجع fill_ata_edd(edd, drive_fl);
    حالة DTYPE_VIRTIO_BLK:
    حالة DTYPE_VIRTIO_SCSI:
        أرجع fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    افتراضي:
        أرجع fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
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
        أرجع DISK_RET_SUCCESS;
    افتراضي:
        أرجع DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
ساكن صحيح
process_op_both(بنية disk_op_s *op)
{
    بدل (GET_FLATPTR(op->drive_fl->type)) {
    حالة DTYPE_ATA_ATAPI:
        أرجع ata_atapi_process_op(op);
    حالة DTYPE_USB:
        أرجع usb_process_op(op);
    حالة DTYPE_UAS:
        أرجع uas_process_op(op);
    حالة DTYPE_LSI_SCSI:
        أرجع lsi_scsi_process_op(op);
    حالة DTYPE_ESP_SCSI:
        أرجع esp_scsi_process_op(op);
    حالة DTYPE_MEGASAS:
        أرجع megasas_process_op(op);
    حالة DTYPE_MPT_SCSI:
        أرجع mpt_scsi_process_op(op);
    افتراضي:
        إذا (!MODESEGMENT)
            أرجع DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        أرجع call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
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
        أرجع virtio_blk_process_op(op);
    حالة DTYPE_AHCI:
        أرجع ahci_process_op(op);
    حالة DTYPE_AHCI_ATAPI:
        أرجع ahci_atapi_process_op(op);
    حالة DTYPE_SDCARD:
        أرجع sdcard_process_op(op);
    حالة DTYPE_USB_32:
        أرجع usb_process_op(op);
    حالة DTYPE_UAS_32:
        أرجع uas_process_op(op);
    حالة DTYPE_VIRTIO_SCSI:
        أرجع virtio_scsi_process_op(op);
    حالة DTYPE_PVSCSI:
        أرجع pvscsi_process_op(op);
    حالة DTYPE_NVME:
        أرجع nvme_process_op(op);
    افتراضي:
        أرجع process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
ساكن صحيح
process_op_16(بنية disk_op_s *op)
{
    ASSERT16();
    بدل (GET_FLATPTR(op->drive_fl->type)) {
    حالة DTYPE_FLOPPY:
        أرجع floppy_process_op(op);
    حالة DTYPE_ATA:
        أرجع ata_process_op(op);
    حالة DTYPE_RAMDISK:
        أرجع ramdisk_process_op(op);
    حالة DTYPE_CDEMU:
        أرجع cdemu_process_op(op);
    افتراضي:
        أرجع process_op_both(op);
    }
}

// Execute a disk_op_s request.
صحيح
process_op(بنية disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->عدد, op->command);

    صحيح ret, origcount = op->عدد;
    إذا (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->عدد = 0;
        أرجع DISK_RET_EBOUNDARY;
    }
    إذا (MODESEGMENT)
        ret = process_op_16(op);
    وإلا
        ret = process_op_32(op);
    إذا (ret && op->عدد == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->عدد = 0;
    أرجع ret;
}
