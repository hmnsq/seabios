// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "block.h" // process_op
#அந்தர்பாவ "hw/ata.h" // process_ata_op
#அந்தர்பாவ "hw/ahci.h" // process_ahci_op
#அந்தர்பாவ "hw/esp-scsi.h" // esp_scsi_process_op
#அந்தர்பாவ "hw/lsi-scsi.h" // lsi_scsi_process_op
#அந்தர்பாவ "hw/megasas.h" // megasas_process_op
#அந்தர்பாவ "hw/mpt-scsi.h" // mpt_scsi_process_op
#அந்தர்பாவ "hw/pci.h" // pci_bdf_to_bus
#அந்தர்பாவ "hw/pvscsi.h" // pvscsi_process_op
#அந்தர்பாவ "hw/rtc.h" // rtc_read
#அந்தர்பாவ "hw/usb-msc.h" // usb_process_op
#அந்தர்பாவ "hw/usb-uas.h" // uas_process_op
#அந்தர்பாவ "hw/virtio-blk.h" // process_virtio_blk_op
#அந்தர்பாவ "hw/virtio-scsi.h" // virtio_scsi_process_op
#அந்தர்பாவ "hw/nvme.h" // nvme_process_op
#அந்தர்பாவ "malloc.h" // malloc_low
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "stacks.h" // call32
#அந்தர்பாவ "std/disk.h" // struct dpte_s
#அந்தர்பாவ "string.h" // checksum
#அந்தர்பாவ "util.h" // process_floppy_op

u8 Floppy_மொத்தம் VARFSEG;
u8 CDCount;
கட்டமைப்பு drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

கட்டமைப்பு drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    எனில் (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        திரும்பு வெற்று;
    திரும்பு GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

முழு getDriveId(u8 exttype, கட்டமைப்பு drive_s *drive)
{
    ASSERT32FLAT();
    முழு i;
    க்கு (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        எனில் (getDrive(exttype, i) == drive)
            திரும்பு i;
    திரும்பு -1;
}

முழு create_bounce_buf(வெறுமை)
{
    எனில் (bounce_buf_fl)
        திரும்பு 0;

    u8 *buf = malloc_கீழ்(CDROM_SECTOR_SIZE);
    எனில் (!buf) {
        warn_noalloc();
        திரும்பு -1;
    }
    bounce_buf_fl = buf;
    திரும்பு 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

நிலையான முழு
host_lchs_supplied(கட்டமைப்பு drive_s *drive)
{
    திரும்பு (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

நிலையான u8
get_translation(கட்டமைப்பு drive_s *drive)
{
    எனில் (host_lchs_supplied(drive))
        திரும்பு TRANSLATION_HOST;
    u8 type = drive->type;
    எனில் (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        திரும்பு translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    எனில் (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        திரும்பு TRANSLATION_LBA;

    எனில் (cylinders <= 1024 && heads <= 16 && spt <= 63)
        திரும்பு TRANSLATION_NONE;
    எனில் (cylinders * heads <= 131072)
        திரும்பு TRANSLATION_LARGE;
    திரும்பு TRANSLATION_LBA;
}

நிலையான வெறுமை
setup_translation(கட்டமைப்பு drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    மாறிலி எழுத்து *desc = வெற்று;

    தேர்வு (translation) {
    இயல்பு:
    நிலை TRANSLATION_NONE:
        desc = "none";
        நிறுத்து;
    நிலை TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        எனில் (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            நிறுத்து;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        எனில் (heads>128)
            heads = 255;
        இல்லையெனில் எனில் (heads>64)
            heads = 128;
        இல்லையெனில் எனில் (heads>32)
            heads = 64;
        இல்லையெனில் எனில் (heads>16)
            heads = 32;
        இல்லையெனில்
            heads = 16;
        cylinders = sect / heads;
        நிறுத்து;
    நிலை TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        எனில் (heads==16) {
            எனில் (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    நிலை TRANSLATION_LARGE:
        எனில் (translation == TRANSLATION_LARGE)
            desc = "large";
        போது (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            எனில் (heads > 127)
                நிறுத்து;
        }
        நிறுத்து;
    நிலை TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        நிறுத்து;
    }
    // clip to 1024 cylinders in lchs
    எனில் (cylinders > 1024)
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
நிலையான வெறுமை
fill_fdpt(கட்டமைப்பு drive_s *drive, முழு hdid)
{
    எனில் (hdid > 1)
        திரும்பு;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    கட்டமைப்பு fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    எனில் (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, அளவு(*fdpt));
    }

    எனில் (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 கட்டமைப்பு extended_bios_data_area_s, fdpt[0])));
    இல்லையெனில்
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 கட்டமைப்பு extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
நிலையான வெறுமை
add_drive(கட்டமைப்பு drive_s **idmap, u8 *மொத்தம், கட்டமைப்பு drive_s *drive)
{
    எனில் (*மொத்தம் >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        திரும்பு;
    }
    idmap[*மொத்தம்] = drive;
    *மொத்தம் = *மொத்தம் + 1;
}

// Map a hard drive
வெறுமை
map_hd_drive(கட்டமைப்பு drive_s *drive)
{
    ASSERT32FLAT();
    கட்டமைப்பு bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    முழு hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
வெறுமை
map_cd_drive(கட்டமைப்பு drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
வெறுமை
map_floppy_drive(கட்டமைப்பு drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_மொத்தம், drive);

    // Update equipment word bits for floppy
    எனில் (Floppy_மொத்தம் == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } இல்லையெனில் எனில் (Floppy_மொத்தம் >= 2) {
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
நிலையான முழு
fill_generic_edd(கட்டமைப்பு segoff_s edd, கட்டமைப்பு drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    கட்டமைப்பு int13dpt_s *param_far = (வெறுமை*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    எனில் (size < 26)
        திரும்பு DISK_RET_EPARAM;

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
    எனில் (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } இல்லையெனில் {
        எனில் (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } இல்லையெனில் {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_மொத்தம், lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    எனில் (size < 30 || !dpte_so)
        திரும்பு DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    எனில் (size < 66 || !bus_iface)
        திரும்பு DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->சாவி, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    மாறிலி எழுத்து *host_bus = "ISA ";
    எனில் ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        எனில் (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , அளவு(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    மாறிலி எழுத்து *iface_type = "ATA     ";
    எனில் ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , அளவு(param_far->iface_type));
    எனில் (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (வெறுமை*)param_far+30, 43));
    } இல்லையெனில் {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (வெறுமை*)param_far+30, 35));
    }

    திரும்பு DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
நிலையான u32
edd_pci_path(u16 bdf, u8 channel)
{
    திரும்பு (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

கட்டமைப்பு dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
நிலையான முழு
fill_ata_edd(கட்டமைப்பு segoff_s edd, கட்டமைப்பு drive_s *drive_gf)
{
    எனில் (!CONFIG_ATA)
        திரும்பு DISK_RET_EPARAM;

    // Fill in dpte
    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        drive_gf, கட்டமைப்பு atadrive_s, drive);
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    முழு bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    எனில் (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        எனில் ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            எனில் (translation == TRANSLATION_LBA)
                options |= 1<<9;
            எனில் (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } இல்லையெனில் {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    எனில் (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_கீழ்(DefaultDPTE.iobase1, iobase1);
    SET_கீழ்(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_கீழ்(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_கீழ்(DefaultDPTE.unused, 0xcb);
    SET_கீழ்(DefaultDPTE.irq, irq);
    SET_கீழ்(DefaultDPTE.blkcount, 1);
    SET_கீழ்(DefaultDPTE.dma, 0);
    SET_கீழ்(DefaultDPTE.pio, 0);
    SET_கீழ்(DefaultDPTE.options, options);
    SET_கீழ்(DefaultDPTE.reserved, 0);
    SET_கீழ்(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_கீழ், &DefaultDPTE, 15);
    SET_கீழ்(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    எனில் (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    திரும்பு fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_கீழ், (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
முழு noinline
fill_edd(கட்டமைப்பு segoff_s edd, கட்டமைப்பு drive_s *drive_fl)
{
    தேர்வு (GET_FLATPTR(drive_fl->type)) {
    நிலை DTYPE_ATA:
    நிலை DTYPE_ATA_ATAPI:
        திரும்பு fill_ata_edd(edd, drive_fl);
    நிலை DTYPE_VIRTIO_BLK:
    நிலை DTYPE_VIRTIO_SCSI:
        திரும்பு fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    இயல்பு:
        திரும்பு fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

வெறுமை
block_setup(வெறுமை)
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
முழு
default_process_op(கட்டமைப்பு disk_op_s *op)
{
    தேர்வு (op->command) {
    நிலை CMD_FORMAT:
    நிலை CMD_RESET:
    நிலை CMD_ISREADY:
    நிலை CMD_VERIFY:
    நிலை CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        திரும்பு DISK_RET_SUCCESS;
    இயல்பு:
        திரும்பு DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
நிலையான முழு
process_op_both(கட்டமைப்பு disk_op_s *op)
{
    தேர்வு (GET_FLATPTR(op->drive_fl->type)) {
    நிலை DTYPE_ATA_ATAPI:
        திரும்பு ata_atapi_process_op(op);
    நிலை DTYPE_USB:
        திரும்பு usb_process_op(op);
    நிலை DTYPE_UAS:
        திரும்பு uas_process_op(op);
    நிலை DTYPE_LSI_SCSI:
        திரும்பு lsi_scsi_process_op(op);
    நிலை DTYPE_ESP_SCSI:
        திரும்பு esp_scsi_process_op(op);
    நிலை DTYPE_MEGASAS:
        திரும்பு megasas_process_op(op);
    நிலை DTYPE_MPT_SCSI:
        திரும்பு mpt_scsi_process_op(op);
    இயல்பு:
        எனில் (!MODESEGMENT)
            திரும்பு DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        திரும்பு call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
முழு VISIBLE32FLAT
process_op_32(கட்டமைப்பு disk_op_s *op)
{
    ASSERT32FLAT();
    தேர்வு (op->drive_fl->type) {
    நிலை DTYPE_VIRTIO_BLK:
        திரும்பு virtio_blk_process_op(op);
    நிலை DTYPE_AHCI:
        திரும்பு ahci_process_op(op);
    நிலை DTYPE_AHCI_ATAPI:
        திரும்பு ahci_atapi_process_op(op);
    நிலை DTYPE_SDCARD:
        திரும்பு sdcard_process_op(op);
    நிலை DTYPE_USB_32:
        திரும்பு usb_process_op(op);
    நிலை DTYPE_UAS_32:
        திரும்பு uas_process_op(op);
    நிலை DTYPE_VIRTIO_SCSI:
        திரும்பு virtio_scsi_process_op(op);
    நிலை DTYPE_PVSCSI:
        திரும்பு pvscsi_process_op(op);
    நிலை DTYPE_NVME:
        திரும்பு nvme_process_op(op);
    இயல்பு:
        திரும்பு process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
நிலையான முழு
process_op_16(கட்டமைப்பு disk_op_s *op)
{
    ASSERT16();
    தேர்வு (GET_FLATPTR(op->drive_fl->type)) {
    நிலை DTYPE_FLOPPY:
        திரும்பு floppy_process_op(op);
    நிலை DTYPE_ATA:
        திரும்பு ata_process_op(op);
    நிலை DTYPE_RAMDISK:
        திரும்பு ramdisk_process_op(op);
    நிலை DTYPE_CDEMU:
        திரும்பு cdemu_process_op(op);
    இயல்பு:
        திரும்பு process_op_both(op);
    }
}

// Execute a disk_op_s request.
முழு
process_op(கட்டமைப்பு disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->மொத்தம், op->command);

    முழு ret, origcount = op->மொத்தம்;
    எனில் (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->மொத்தம் = 0;
        திரும்பு DISK_RET_EBOUNDARY;
    }
    எனில் (MODESEGMENT)
        ret = process_op_16(op);
    இல்லையெனில்
        ret = process_op_32(op);
    எனில் (ret && op->மொத்தம் == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->மொத்தம் = 0;
    திரும்பு ret;
}
