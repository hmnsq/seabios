// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "block.h" // process_op
#അന്തര്ഭാവ "hw/ata.h" // process_ata_op
#അന്തര്ഭാവ "hw/ahci.h" // process_ahci_op
#അന്തര്ഭാവ "hw/esp-scsi.h" // esp_scsi_process_op
#അന്തര്ഭാവ "hw/lsi-scsi.h" // lsi_scsi_process_op
#അന്തര്ഭാവ "hw/megasas.h" // megasas_process_op
#അന്തര്ഭാവ "hw/mpt-scsi.h" // mpt_scsi_process_op
#അന്തര്ഭാവ "hw/pci.h" // pci_bdf_to_bus
#അന്തര്ഭാവ "hw/pvscsi.h" // pvscsi_process_op
#അന്തര്ഭാവ "hw/rtc.h" // rtc_read
#അന്തര്ഭാവ "hw/usb-msc.h" // usb_process_op
#അന്തര്ഭാവ "hw/usb-uas.h" // uas_process_op
#അന്തര്ഭാവ "hw/virtio-blk.h" // process_virtio_blk_op
#അന്തര്ഭാവ "hw/virtio-scsi.h" // virtio_scsi_process_op
#അന്തര്ഭാവ "hw/nvme.h" // nvme_process_op
#അന്തര്ഭാവ "malloc.h" // malloc_low
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // call32
#അന്തര്ഭാവ "std/disk.h" // struct dpte_s
#അന്തര്ഭാവ "string.h" // checksum
#അന്തര്ഭാവ "util.h" // process_floppy_op

u8 Floppy_എണ്ണം VARFSEG;
u8 CDCount;
ഘടന drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

ഘടന drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    എങ്കിൽ (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        തിരികെ ശൂന്യ;
    തിരികെ GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

പൂർണ്ണസംഖ്യ getDriveId(u8 exttype, ഘടന drive_s *drive)
{
    ASSERT32FLAT();
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        എങ്കിൽ (getDrive(exttype, i) == drive)
            തിരികെ i;
    തിരികെ -1;
}

പൂർണ്ണസംഖ്യ create_bounce_buf(ശൂന്യം)
{
    എങ്കിൽ (bounce_buf_fl)
        തിരികെ 0;

    u8 *buf = malloc_താഴെയുള്ള(CDROM_SECTOR_SIZE);
    എങ്കിൽ (!buf) {
        warn_noalloc();
        തിരികെ -1;
    }
    bounce_buf_fl = buf;
    തിരികെ 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

സ്ഥിതം പൂർണ്ണസംഖ്യ
host_lchs_supplied(ഘടന drive_s *drive)
{
    തിരികെ (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

സ്ഥിതം u8
get_translation(ഘടന drive_s *drive)
{
    എങ്കിൽ (host_lchs_supplied(drive))
        തിരികെ TRANSLATION_HOST;
    u8 type = drive->type;
    എങ്കിൽ (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        തിരികെ translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    എങ്കിൽ (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        തിരികെ TRANSLATION_LBA;

    എങ്കിൽ (cylinders <= 1024 && heads <= 16 && spt <= 63)
        തിരികെ TRANSLATION_NONE;
    എങ്കിൽ (cylinders * heads <= 131072)
        തിരികെ TRANSLATION_LARGE;
    തിരികെ TRANSLATION_LBA;
}

സ്ഥിതം ശൂന്യം
setup_translation(ഘടന drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    സ്ഥിരം അക്ഷരം *desc = ശൂന്യ;

    തിരഞ്ഞെടുപ്പ് (translation) {
    സ്ഥിരസ്ഥിതി:
    അവസ്ഥ TRANSLATION_NONE:
        desc = "none";
        നിർത്തുക;
    അവസ്ഥ TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        എങ്കിൽ (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            നിർത്തുക;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        എങ്കിൽ (heads>128)
            heads = 255;
        അല്ലെങ്കിൽ എങ്കിൽ (heads>64)
            heads = 128;
        അല്ലെങ്കിൽ എങ്കിൽ (heads>32)
            heads = 64;
        അല്ലെങ്കിൽ എങ്കിൽ (heads>16)
            heads = 32;
        അല്ലെങ്കിൽ
            heads = 16;
        cylinders = sect / heads;
        നിർത്തുക;
    അവസ്ഥ TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        എങ്കിൽ (heads==16) {
            എങ്കിൽ (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    അവസ്ഥ TRANSLATION_LARGE:
        എങ്കിൽ (translation == TRANSLATION_LARGE)
            desc = "large";
        ആകുമ്പോൾ (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            എങ്കിൽ (heads > 127)
                നിർത്തുക;
        }
        നിർത്തുക;
    അവസ്ഥ TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        നിർത്തുക;
    }
    // clip to 1024 cylinders in lchs
    എങ്കിൽ (cylinders > 1024)
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
സ്ഥിതം ശൂന്യം
fill_fdpt(ഘടന drive_s *drive, പൂർണ്ണസംഖ്യ hdid)
{
    എങ്കിൽ (hdid > 1)
        തിരികെ;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    ഘടന fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    എങ്കിൽ (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, വലുപ്പം(*fdpt));
    }

    എങ്കിൽ (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 ഘടന extended_bios_data_area_s, fdpt[0])));
    അല്ലെങ്കിൽ
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 ഘടന extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
സ്ഥിതം ശൂന്യം
add_drive(ഘടന drive_s **idmap, u8 *എണ്ണം, ഘടന drive_s *drive)
{
    എങ്കിൽ (*എണ്ണം >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        തിരികെ;
    }
    idmap[*എണ്ണം] = drive;
    *എണ്ണം = *എണ്ണം + 1;
}

// Map a hard drive
ശൂന്യം
map_hd_drive(ഘടന drive_s *drive)
{
    ASSERT32FLAT();
    ഘടന bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    പൂർണ്ണസംഖ്യ hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
ശൂന്യം
map_cd_drive(ഘടന drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
ശൂന്യം
map_floppy_drive(ഘടന drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_എണ്ണം, drive);

    // Update equipment word bits for floppy
    എങ്കിൽ (Floppy_എണ്ണം == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } അല്ലെങ്കിൽ എങ്കിൽ (Floppy_എണ്ണം >= 2) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
fill_generic_edd(ഘടന segoff_s edd, ഘടന drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    ഘടന int13dpt_s *param_far = (ശൂന്യം*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    എങ്കിൽ (size < 26)
        തിരികെ DISK_RET_EPARAM;

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
    എങ്കിൽ (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } അല്ലെങ്കിൽ {
        എങ്കിൽ (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } അല്ലെങ്കിൽ {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_എണ്ണം, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    എങ്കിൽ (size < 30 || !dpte_so)
        തിരികെ DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    എങ്കിൽ (size < 66 || !bus_iface)
        തിരികെ DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->താക്കോൽ, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    സ്ഥിരം അക്ഷരം *host_bus = "ISA ";
    എങ്കിൽ ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        എങ്കിൽ (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , വലുപ്പം(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    സ്ഥിരം അക്ഷരം *iface_type = "ATA     ";
    എങ്കിൽ ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , വലുപ്പം(param_far->iface_type));
    എങ്കിൽ (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (ശൂന്യം*)param_far+30, 43));
    } അല്ലെങ്കിൽ {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (ശൂന്യം*)param_far+30, 35));
    }

    തിരികെ DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
സ്ഥിതം u32
edd_pci_path(u16 bdf, u8 channel)
{
    തിരികെ (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

ഘടന dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
സ്ഥിതം പൂർണ്ണസംഖ്യ
fill_ata_edd(ഘടന segoff_s edd, ഘടന drive_s *drive_gf)
{
    എങ്കിൽ (!CONFIG_ATA)
        തിരികെ DISK_RET_EPARAM;

    // Fill in dpte
    ഘടന atadrive_s *adrive_gf = container_of(
        drive_gf, ഘടന atadrive_s, drive);
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    പൂർണ്ണസംഖ്യ bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    എങ്കിൽ (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        എങ്കിൽ ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            എങ്കിൽ (translation == TRANSLATION_LBA)
                options |= 1<<9;
            എങ്കിൽ (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } അല്ലെങ്കിൽ {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    എങ്കിൽ (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_താഴെയുള്ള(DefaultDPTE.iobase1, iobase1);
    SET_താഴെയുള്ള(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_താഴെയുള്ള(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_താഴെയുള്ള(DefaultDPTE.unused, 0xcb);
    SET_താഴെയുള്ള(DefaultDPTE.irq, irq);
    SET_താഴെയുള്ള(DefaultDPTE.blkcount, 1);
    SET_താഴെയുള്ള(DefaultDPTE.dma, 0);
    SET_താഴെയുള്ള(DefaultDPTE.pio, 0);
    SET_താഴെയുള്ള(DefaultDPTE.options, options);
    SET_താഴെയുള്ള(DefaultDPTE.reserved, 0);
    SET_താഴെയുള്ള(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_താഴെയുള്ള, &DefaultDPTE, 15);
    SET_താഴെയുള്ള(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    എങ്കിൽ (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    തിരികെ fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_താഴെയുള്ള, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
പൂർണ്ണസംഖ്യ noinline
fill_edd(ഘടന segoff_s edd, ഘടന drive_s *drive_fl)
{
    തിരഞ്ഞെടുപ്പ് (GET_FLATPTR(drive_fl->type)) {
    അവസ്ഥ DTYPE_ATA:
    അവസ്ഥ DTYPE_ATA_ATAPI:
        തിരികെ fill_ata_edd(edd, drive_fl);
    അവസ്ഥ DTYPE_VIRTIO_BLK:
    അവസ്ഥ DTYPE_VIRTIO_SCSI:
        തിരികെ fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    സ്ഥിരസ്ഥിതി:
        തിരികെ fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

ശൂന്യം
block_setup(ശൂന്യം)
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
പൂർണ്ണസംഖ്യ
default_process_op(ഘടന disk_op_s *op)
{
    തിരഞ്ഞെടുപ്പ് (op->command) {
    അവസ്ഥ CMD_FORMAT:
    അവസ്ഥ CMD_RESET:
    അവസ്ഥ CMD_ISREADY:
    അവസ്ഥ CMD_VERIFY:
    അവസ്ഥ CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        തിരികെ DISK_RET_SUCCESS;
    സ്ഥിരസ്ഥിതി:
        തിരികെ DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
സ്ഥിതം പൂർണ്ണസംഖ്യ
process_op_both(ഘടന disk_op_s *op)
{
    തിരഞ്ഞെടുപ്പ് (GET_FLATPTR(op->drive_fl->type)) {
    അവസ്ഥ DTYPE_ATA_ATAPI:
        തിരികെ ata_atapi_process_op(op);
    അവസ്ഥ DTYPE_USB:
        തിരികെ usb_process_op(op);
    അവസ്ഥ DTYPE_UAS:
        തിരികെ uas_process_op(op);
    അവസ്ഥ DTYPE_LSI_SCSI:
        തിരികെ lsi_scsi_process_op(op);
    അവസ്ഥ DTYPE_ESP_SCSI:
        തിരികെ esp_scsi_process_op(op);
    അവസ്ഥ DTYPE_MEGASAS:
        തിരികെ megasas_process_op(op);
    അവസ്ഥ DTYPE_MPT_SCSI:
        തിരികെ mpt_scsi_process_op(op);
    സ്ഥിരസ്ഥിതി:
        എങ്കിൽ (!MODESEGMENT)
            തിരികെ DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        തിരികെ call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
പൂർണ്ണസംഖ്യ VISIBLE32FLAT
process_op_32(ഘടന disk_op_s *op)
{
    ASSERT32FLAT();
    തിരഞ്ഞെടുപ്പ് (op->drive_fl->type) {
    അവസ്ഥ DTYPE_VIRTIO_BLK:
        തിരികെ virtio_blk_process_op(op);
    അവസ്ഥ DTYPE_AHCI:
        തിരികെ ahci_process_op(op);
    അവസ്ഥ DTYPE_AHCI_ATAPI:
        തിരികെ ahci_atapi_process_op(op);
    അവസ്ഥ DTYPE_SDCARD:
        തിരികെ sdcard_process_op(op);
    അവസ്ഥ DTYPE_USB_32:
        തിരികെ usb_process_op(op);
    അവസ്ഥ DTYPE_UAS_32:
        തിരികെ uas_process_op(op);
    അവസ്ഥ DTYPE_VIRTIO_SCSI:
        തിരികെ virtio_scsi_process_op(op);
    അവസ്ഥ DTYPE_PVSCSI:
        തിരികെ pvscsi_process_op(op);
    അവസ്ഥ DTYPE_NVME:
        തിരികെ nvme_process_op(op);
    സ്ഥിരസ്ഥിതി:
        തിരികെ process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
സ്ഥിതം പൂർണ്ണസംഖ്യ
process_op_16(ഘടന disk_op_s *op)
{
    ASSERT16();
    തിരഞ്ഞെടുപ്പ് (GET_FLATPTR(op->drive_fl->type)) {
    അവസ്ഥ DTYPE_FLOPPY:
        തിരികെ floppy_process_op(op);
    അവസ്ഥ DTYPE_ATA:
        തിരികെ ata_process_op(op);
    അവസ്ഥ DTYPE_RAMDISK:
        തിരികെ ramdisk_process_op(op);
    അവസ്ഥ DTYPE_CDEMU:
        തിരികെ cdemu_process_op(op);
    സ്ഥിരസ്ഥിതി:
        തിരികെ process_op_both(op);
    }
}

// Execute a disk_op_s request.
പൂർണ്ണസംഖ്യ
process_op(ഘടന disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->എണ്ണം, op->command);

    പൂർണ്ണസംഖ്യ ret, origcount = op->എണ്ണം;
    എങ്കിൽ (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->എണ്ണം = 0;
        തിരികെ DISK_RET_EBOUNDARY;
    }
    എങ്കിൽ (MODESEGMENT)
        ret = process_op_16(op);
    അല്ലെങ്കിൽ
        ret = process_op_32(op);
    എങ്കിൽ (ret && op->എണ്ണം == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->എണ്ണം = 0;
    തിരികെ ret;
}
