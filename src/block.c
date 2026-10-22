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

u8 Floppy_گنتی VARFSEG;
u8 CDCount;
ساخت drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

ساخت drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    اگر (extdriveoffset >= ARRAY_سائز(IDMap[0]))
        واپس تہی;
    واپس GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

صحیح getDriveId(u8 exttype, ساخت drive_s *drive)
{
    ASSERT32FLAT();
    صحیح i;
    برائے (i = 0; i < ARRAY_سائز(IDMap[0]); i++)
        اگر (getDrive(exttype, i) == drive)
            واپس i;
    واپس -1;
}

صحیح create_bounce_buf(خالی)
{
    اگر (bounce_buf_fl)
        واپس 0;

    u8 *buf = malloc_کم(CDROM_SECTOR_سائز);
    اگر (!buf) {
        warn_noalloc();
        واپس -1;
    }
    bounce_buf_fl = buf;
    واپس 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

ساکن صحیح
host_lchs_supplied(ساخت drive_s *drive)
{
    واپس (drive->lchs.سر <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

ساکن u8
get_translation(ساخت drive_s *drive)
{
    اگر (host_lchs_supplied(drive))
        واپس TRANSLATION_HOST;
    u8 type = drive->type;
    اگر (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        واپس translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.سر;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    اگر (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        واپس TRANSLATION_LBA;

    اگر (cylinders <= 1024 && heads <= 16 && spt <= 63)
        واپس TRANSLATION_NONE;
    اگر (cylinders * heads <= 131072)
        واپس TRANSLATION_LARGE;
    واپس TRANSLATION_LBA;
}

ساکن خالی
تیاری_translation(ساخت drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.سر ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ثابت حرف *desc = تہی;

    انتخاب (translation) {
    طےشدہ:
    حالت TRANSLATION_NONE:
        desc = "none";
        توڑو;
    حالت TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        اگر (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            توڑو;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        اگر (heads>128)
            heads = 255;
        ورنہ اگر (heads>64)
            heads = 128;
        ورنہ اگر (heads>32)
            heads = 64;
        ورنہ اگر (heads>16)
            heads = 32;
        ورنہ
            heads = 16;
        cylinders = sect / heads;
        توڑو;
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
        جب (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            اگر (heads > 127)
                توڑو;
        }
        توڑو;
    حالت TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.سر;
        spt = drive->lchs.sector;
        توڑو;
    }
    // clip to 1024 cylinders in lchs
    اگر (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%u\n"
            , drive
            , drive->pchs.cylinder, drive->pchs.سر, drive->pchs.sector
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    drive->lchs.سر = heads;
    drive->lchs.cylinder = cylinders;
    drive->lchs.sector = spt;
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
ساکن خالی
fill_fdpt(ساخت drive_s *drive, صحیح hdid)
{
    اگر (hdid > 1)
        واپس;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.سر;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.سر;
    u16 nps = drive->pchs.sector;

    ساخت fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
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
        fdpt->checksum -= checksum(fdpt, حجم(*fdpt));
    }

    اگر (hdid == 0)
        سیٹ_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 ساخت extended_bios_ڈیٹا_area_s, fdpt[0])));
    ورنہ
        سیٹ_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 ساخت extended_bios_ڈیٹا_area_s, fdpt[1])));
}

// Find spot to add a drive
ساکن خالی
جوڑو_drive(ساخت drive_s **idmap, u8 *گنتی, ساخت drive_s *drive)
{
    اگر (*گنتی >= ARRAY_سائز(IDMap[0])) {
        warn_noalloc();
        واپس;
    }
    idmap[*گنتی] = drive;
    *گنتی = *گنتی + 1;
}

// Map a hard drive
خالی
map_hd_drive(ساخت drive_s *drive)
{
    ASSERT32FLAT();
    ساخت bios_ڈیٹا_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    صحیح hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    جوڑو_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    تیاری_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
خالی
map_cd_drive(ساخت drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    جوڑو_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
خالی
map_floppy_drive(ساخت drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    جوڑو_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_گنتی, drive);

    // Update equipment word bits for floppy
    اگر (Floppy_گنتی == 1) {
        // 1 drive, ready for boot
        سیٹ_equipment_flags(0x41, 0x01);
        سیٹ_BDA(floppy_harddisk_info, 0x07);
    } ورنہ اگر (Floppy_گنتی >= 2) {
        // 2 drives, ready for boot
        سیٹ_equipment_flags(0x41, 0x41);
        سیٹ_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * Extended Disk Drive (EDD) get drive parameters
 ****************************************************************/

// flags for bus_iface field in fill_generic_edd()
#تعریف_کرو EDD_ISA        0x01
#تعریف_کرو EDD_PCI        0x02
#تعریف_کرو EDD_BUS_نقاب   0x0f
#تعریف_کرو EDD_ATA        0x10
#تعریف_کرو EDD_SCSI       0x20
#تعریف_کرو EDD_IFACE_نقاب 0xf0

// Fill in EDD info
ساکن صحیح
fill_generic_edd(ساخت segoff_s edd, ساخت drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_راستہ, u32 device_راستہ)
{
    u16 seg = edd.seg;
    ساخت int13dpt_s *param_far = (خالی*)(edd.انحراف_2+0);
    u16 سائز = GET_FARVAR(seg, param_far->سائز);
    u16 t13 = سائز == 74;

    // Buffer is too small
    اگر (سائز < 26)
        واپس DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.سر);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , سائز, type, npc, nph, nps, (u32)lba, blksize);

    سیٹ_FARVAR(seg, param_far->سائز, 26);
    اگر (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        سیٹ_FARVAR(seg, param_far->infos, 0x74);
        سیٹ_FARVAR(seg, param_far->cylinders, 0xffffffff);
        سیٹ_FARVAR(seg, param_far->heads, 0xffffffff);
        سیٹ_FARVAR(seg, param_far->spt, 0xffffffff);
    } ورنہ {
        اگر (lba > (u64)nps*nph*0x3fff) {
            سیٹ_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            سیٹ_FARVAR(seg, param_far->cylinders, 0x3fff);
        } ورنہ {
            سیٹ_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            سیٹ_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        سیٹ_FARVAR(seg, param_far->heads, (u32)nph);
        سیٹ_FARVAR(seg, param_far->spt, (u32)nps);
    }
    سیٹ_FARVAR(seg, param_far->sector_گنتی, lba);
    سیٹ_FARVAR(seg, param_far->blksize, blksize);

    اگر (سائز < 30 || !dpte_so)
        واپس DISK_RET_SUCCESS;

    // EDD 2.x

    سیٹ_FARVAR(seg, param_far->سائز, 30);
    سیٹ_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    اگر (سائز < 66 || !bus_iface)
        واپس DISK_RET_SUCCESS;

    // EDD 3.x
    سیٹ_FARVAR(seg, param_far->کلید, 0xbedd);
    سیٹ_FARVAR(seg, param_far->dpi_لمبائی, t13 ? 44 : 36);
    سیٹ_FARVAR(seg, param_far->reserved1, 0);
    سیٹ_FARVAR(seg, param_far->reserved2, 0);

    ثابت حرف *host_bus = "ISA ";
    اگر ((bus_iface & EDD_BUS_نقاب) == EDD_PCI) {
        host_bus = "PCI ";
        اگر (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_راستہ &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , حجم(param_far->host_bus));
    سیٹ_FARVAR(seg, param_far->iface_راستہ, iface_راستہ);

    ثابت حرف *iface_type = "ATA     ";
    اگر ((bus_iface & EDD_IFACE_نقاب) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , حجم(param_far->iface_type));
    اگر (t13) {
        سیٹ_FARVAR(seg, param_far->t13.device_راستہ[0], device_راستہ);
        سیٹ_FARVAR(seg, param_far->t13.device_راستہ[1], 0);

        سیٹ_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (خالی*)param_far+30, 43));
    } ورنہ {
        سیٹ_FARVAR(seg, param_far->phoenix.device_راستہ, device_راستہ);

        سیٹ_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (خالی*)param_far+30, 35));
    }

    واپس DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
ساکن u32
edd_pci_راستہ(u16 bdf, u8 channel)
{
    واپس (pci_bdf_مقصد_bus(bdf) | (pci_bdf_مقصد_dev(bdf) << 8)
            | (pci_bdf_مقصد_fn(bdf) << 16) | ((u32)channel << 24));
}

ساخت dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
ساکن صحیح
fill_ata_edd(ساخت segoff_s edd, ساخت drive_s *drive_gf)
{
    اگر (!CONFIG_ATA)
        واپس DISK_RET_EPARAM;

    // Fill in dpte
    ساخت atadrive_s *adrive_gf = container_کا(
        drive_gf, ساخت atadrive_s, drive);
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    } ورنہ {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    اگر (CONFIG_ATA_PIO32)
        options |= 1<<7;

    سیٹ_کم(DefaultDPTE.iobase1, iobase1);
    سیٹ_کم(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    سیٹ_کم(DefaultDPTE.سابقہ, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    سیٹ_کم(DefaultDPTE.غیر_مستعمل, 0xcb);
    سیٹ_کم(DefaultDPTE.irq, irq);
    سیٹ_کم(DefaultDPTE.blkcount, 1);
    سیٹ_کم(DefaultDPTE.dma, 0);
    سیٹ_کم(DefaultDPTE.pio, 0);
    سیٹ_کم(DefaultDPTE.options, options);
    سیٹ_کم(DefaultDPTE.reserved, 0);
    سیٹ_کم(DefaultDPTE.revision, 0x11);

    u8 مجموع = checksum_far(SEG_کم, &DefaultDPTE, 15);
    سیٹ_کم(DefaultDPTE.checksum, -مجموع);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    اگر (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_راستہ(bdf, channel);
    }
    واپس fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_کم, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
صحیح noinline
fill_edd(ساخت segoff_s edd, ساخت drive_s *drive_fl)
{
    انتخاب (GET_FLATPTR(drive_fl->type)) {
    حالت DTYPE_ATA:
    حالت DTYPE_ATA_ATAPI:
        واپس fill_ata_edd(edd, drive_fl);
    حالت DTYPE_VIRTIO_BLK:
    حالت DTYPE_VIRTIO_SCSI:
        واپس fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_راستہ(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    طےشدہ:
        واپس fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

خالی
بلاک_تیاری(خالی)
{
    floppy_تیاری();
    ata_تیاری();
    ahci_تیاری();
    sdcard_تیاری();
    ramdisk_تیاری();
    virtio_blk_تیاری();
    virtio_scsi_تیاری();
    lsi_scsi_تیاری();
    esp_scsi_تیاری();
    megasas_تیاری();
    pvscsi_تیاری();
    mpt_scsi_تیاری();
    nvme_تیاری();
}

// Fallback handler for command requests not implemented by drivers
صحیح
default_process_op(ساخت disk_op_s *op)
{
    انتخاب (op->command) {
    حالت CMD_FORMAT:
    حالت CMD_RESET:
    حالت CMD_ISREADY:
    حالت CMD_VERIFY:
    حالت CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        واپس DISK_RET_SUCCESS;
    طےشدہ:
        واپس DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
ساکن صحیح
process_op_both(ساخت disk_op_s *op)
{
    انتخاب (GET_FLATPTR(op->drive_fl->type)) {
    حالت DTYPE_ATA_ATAPI:
        واپس ata_atapi_process_op(op);
    حالت DTYPE_USB:
        واپس usb_process_op(op);
    حالت DTYPE_UAS:
        واپس uas_process_op(op);
    حالت DTYPE_LSI_SCSI:
        واپس lsi_scsi_process_op(op);
    حالت DTYPE_ESP_SCSI:
        واپس esp_scsi_process_op(op);
    حالت DTYPE_MEGASAS:
        واپس megasas_process_op(op);
    حالت DTYPE_MPT_SCSI:
        واپس mpt_scsi_process_op(op);
    طےشدہ:
        اگر (!MODESEGMENT)
            واپس DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        واپس call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
صحیح VISIBLE32FLAT
process_op_32(ساخت disk_op_s *op)
{
    ASSERT32FLAT();
    انتخاب (op->drive_fl->type) {
    حالت DTYPE_VIRTIO_BLK:
        واپس virtio_blk_process_op(op);
    حالت DTYPE_AHCI:
        واپس ahci_process_op(op);
    حالت DTYPE_AHCI_ATAPI:
        واپس ahci_atapi_process_op(op);
    حالت DTYPE_SDCARD:
        واپس sdcard_process_op(op);
    حالت DTYPE_USB_32:
        واپس usb_process_op(op);
    حالت DTYPE_UAS_32:
        واپس uas_process_op(op);
    حالت DTYPE_VIRTIO_SCSI:
        واپس virtio_scsi_process_op(op);
    حالت DTYPE_PVSCSI:
        واپس pvscsi_process_op(op);
    حالت DTYPE_NVME:
        واپس nvme_process_op(op);
    طےشدہ:
        واپس process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
ساکن صحیح
process_op_16(ساخت disk_op_s *op)
{
    ASSERT16();
    انتخاب (GET_FLATPTR(op->drive_fl->type)) {
    حالت DTYPE_FLOPPY:
        واپس floppy_process_op(op);
    حالت DTYPE_ATA:
        واپس ata_process_op(op);
    حالت DTYPE_RAMDISK:
        واپس ramdisk_process_op(op);
    حالت DTYPE_CDEMU:
        واپس cdemu_process_op(op);
    طےشدہ:
        واپس process_op_both(op);
    }
}

// Execute a disk_op_s request.
صحیح
process_op(ساخت disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->گنتی, op->command);

    صحیح ret, origcount = op->گنتی;
    اگر (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->گنتی = 0;
        واپس DISK_RET_EBOUNDARY;
    }
    اگر (MODESEGMENT)
        ret = process_op_16(op);
    ورنہ
        ret = process_op_32(op);
    اگر (ret && op->گنتی == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->گنتی = 0;
    واپس ret;
}
