// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "block.h" // process_op
#અન્તર્ભાવ "hw/ata.h" // process_ata_op
#અન્તર્ભાવ "hw/ahci.h" // process_ahci_op
#અન્તર્ભાવ "hw/esp-scsi.h" // esp_scsi_process_op
#અન્તર્ભાવ "hw/lsi-scsi.h" // lsi_scsi_process_op
#અન્તર્ભાવ "hw/megasas.h" // megasas_process_op
#અન્તર્ભાવ "hw/mpt-scsi.h" // mpt_scsi_process_op
#અન્તર્ભાવ "hw/pci.h" // pci_bdf_to_bus
#અન્તર્ભાવ "hw/pvscsi.h" // pvscsi_process_op
#અન્તર્ભાવ "hw/rtc.h" // rtc_read
#અન્તર્ભાવ "hw/usb-msc.h" // usb_process_op
#અન્તર્ભાવ "hw/usb-uas.h" // uas_process_op
#અન્તર્ભાવ "hw/virtio-blk.h" // process_virtio_blk_op
#અન્તર્ભાવ "hw/virtio-scsi.h" // virtio_scsi_process_op
#અન્તર્ભાવ "hw/nvme.h" // nvme_process_op
#અન્તર્ભાવ "malloc.h" // malloc_low
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // call32
#અન્તર્ભાવ "std/disk.h" // struct dpte_s
#અન્તર્ભાવ "string.h" // checksum
#અન્તર્ભાવ "util.h" // process_floppy_op

u8 Floppy_ગણતરી VARFSEG;
u8 CDCount;
સંરચના drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

સંરચના drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    જો (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        પાછા ખાલી;
    પાછા GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

પૂર્ણાંક getDriveId(u8 exttype, સંરચના drive_s *drive)
{
    ASSERT32FLAT();
    પૂર્ણાંક i;
    માટે (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        જો (getDrive(exttype, i) == drive)
            પાછા i;
    પાછા -1;
}

પૂર્ણાંક create_bounce_buf(શૂન્ય)
{
    જો (bounce_buf_fl)
        પાછા 0;

    u8 *buf = malloc_નીચલું(CDROM_SECTOR_SIZE);
    જો (!buf) {
        warn_noalloc();
        પાછા -1;
    }
    bounce_buf_fl = buf;
    પાછા 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

સ્થિર પૂર્ણાંક
host_lchs_supplied(સંરચના drive_s *drive)
{
    પાછા (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

સ્થિર u8
get_translation(સંરચના drive_s *drive)
{
    જો (host_lchs_supplied(drive))
        પાછા TRANSLATION_HOST;
    u8 type = drive->type;
    જો (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        પાછા translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    જો (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        પાછા TRANSLATION_LBA;

    જો (cylinders <= 1024 && heads <= 16 && spt <= 63)
        પાછા TRANSLATION_NONE;
    જો (cylinders * heads <= 131072)
        પાછા TRANSLATION_LARGE;
    પાછા TRANSLATION_LBA;
}

સ્થિર શૂન્ય
setup_translation(સંરચના drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    અચલ અક્ષર *desc = ખાલી;

    પસંદગી (translation) {
    મૂળભૂત:
    સ્થિતિ TRANSLATION_NONE:
        desc = "none";
        તોડો;
    સ્થિતિ TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        જો (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            તોડો;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        જો (heads>128)
            heads = 255;
        નહીંતર જો (heads>64)
            heads = 128;
        નહીંતર જો (heads>32)
            heads = 64;
        નહીંતર જો (heads>16)
            heads = 32;
        નહીંતર
            heads = 16;
        cylinders = sect / heads;
        તોડો;
    સ્થિતિ TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        જો (heads==16) {
            જો (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    સ્થિતિ TRANSLATION_LARGE:
        જો (translation == TRANSLATION_LARGE)
            desc = "large";
        જ્યારે (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            જો (heads > 127)
                તોડો;
        }
        તોડો;
    સ્થિતિ TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        તોડો;
    }
    // clip to 1024 cylinders in lchs
    જો (cylinders > 1024)
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
સ્થિર શૂન્ય
fill_fdpt(સંરચના drive_s *drive, પૂર્ણાંક hdid)
{
    જો (hdid > 1)
        પાછા;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    સંરચના fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    જો (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, કદ(*fdpt));
    }

    જો (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 સંરચના extended_bios_data_area_s, fdpt[0])));
    નહીંતર
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 સંરચના extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
સ્થિર શૂન્ય
add_drive(સંરચના drive_s **idmap, u8 *ગણતરી, સંરચના drive_s *drive)
{
    જો (*ગણતરી >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        પાછા;
    }
    idmap[*ગણતરી] = drive;
    *ગણતરી = *ગણતરી + 1;
}

// Map a hard drive
શૂન્ય
map_hd_drive(સંરચના drive_s *drive)
{
    ASSERT32FLAT();
    સંરચના bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    પૂર્ણાંક hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
શૂન્ય
map_cd_drive(સંરચના drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
શૂન્ય
map_floppy_drive(સંરચના drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_ગણતરી, drive);

    // Update equipment word bits for floppy
    જો (Floppy_ગણતરી == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } નહીંતર જો (Floppy_ગણતરી >= 2) {
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
સ્થિર પૂર્ણાંક
fill_generic_edd(સંરચના segoff_s edd, સંરચના drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    સંરચના int13dpt_s *param_far = (શૂન્ય*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    જો (size < 26)
        પાછા DISK_RET_EPARAM;

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
    જો (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } નહીંતર {
        જો (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } નહીંતર {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_ગણતરી, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    જો (size < 30 || !dpte_so)
        પાછા DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    જો (size < 66 || !bus_iface)
        પાછા DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->ચાવી, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    અચલ અક્ષર *host_bus = "ISA ";
    જો ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        જો (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , કદ(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    અચલ અક્ષર *iface_type = "ATA     ";
    જો ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , કદ(param_far->iface_type));
    જો (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (શૂન્ય*)param_far+30, 43));
    } નહીંતર {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (શૂન્ય*)param_far+30, 35));
    }

    પાછા DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
સ્થિર u32
edd_pci_path(u16 bdf, u8 channel)
{
    પાછા (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

સંરચના dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
સ્થિર પૂર્ણાંક
fill_ata_edd(સંરચના segoff_s edd, સંરચના drive_s *drive_gf)
{
    જો (!CONFIG_ATA)
        પાછા DISK_RET_EPARAM;

    // Fill in dpte
    સંરચના atadrive_s *adrive_gf = container_of(
        drive_gf, સંરચના atadrive_s, drive);
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    પૂર્ણાંક bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    જો (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        જો ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            જો (translation == TRANSLATION_LBA)
                options |= 1<<9;
            જો (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } નહીંતર {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    જો (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_નીચલું(DefaultDPTE.iobase1, iobase1);
    SET_નીચલું(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_નીચલું(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_નીચલું(DefaultDPTE.unused, 0xcb);
    SET_નીચલું(DefaultDPTE.irq, irq);
    SET_નીચલું(DefaultDPTE.blkcount, 1);
    SET_નીચલું(DefaultDPTE.dma, 0);
    SET_નીચલું(DefaultDPTE.pio, 0);
    SET_નીચલું(DefaultDPTE.options, options);
    SET_નીચલું(DefaultDPTE.reserved, 0);
    SET_નીચલું(DefaultDPTE.revision, 0x11);

    u8 sum = checksum_far(SEG_નીચલું, &DefaultDPTE, 15);
    SET_નીચલું(DefaultDPTE.checksum, -sum);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    જો (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    પાછા fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_નીચલું, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
પૂર્ણાંક noinline
fill_edd(સંરચના segoff_s edd, સંરચના drive_s *drive_fl)
{
    પસંદગી (GET_FLATPTR(drive_fl->type)) {
    સ્થિતિ DTYPE_ATA:
    સ્થિતિ DTYPE_ATA_ATAPI:
        પાછા fill_ata_edd(edd, drive_fl);
    સ્થિતિ DTYPE_VIRTIO_BLK:
    સ્થિતિ DTYPE_VIRTIO_SCSI:
        પાછા fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    મૂળભૂત:
        પાછા fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

શૂન્ય
block_setup(શૂન્ય)
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
પૂર્ણાંક
default_process_op(સંરચના disk_op_s *op)
{
    પસંદગી (op->command) {
    સ્થિતિ CMD_FORMAT:
    સ્થિતિ CMD_RESET:
    સ્થિતિ CMD_ISREADY:
    સ્થિતિ CMD_VERIFY:
    સ્થિતિ CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        પાછા DISK_RET_SUCCESS;
    મૂળભૂત:
        પાછા DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
સ્થિર પૂર્ણાંક
process_op_both(સંરચના disk_op_s *op)
{
    પસંદગી (GET_FLATPTR(op->drive_fl->type)) {
    સ્થિતિ DTYPE_ATA_ATAPI:
        પાછા ata_atapi_process_op(op);
    સ્થિતિ DTYPE_USB:
        પાછા usb_process_op(op);
    સ્થિતિ DTYPE_UAS:
        પાછા uas_process_op(op);
    સ્થિતિ DTYPE_LSI_SCSI:
        પાછા lsi_scsi_process_op(op);
    સ્થિતિ DTYPE_ESP_SCSI:
        પાછા esp_scsi_process_op(op);
    સ્થિતિ DTYPE_MEGASAS:
        પાછા megasas_process_op(op);
    સ્થિતિ DTYPE_MPT_SCSI:
        પાછા mpt_scsi_process_op(op);
    મૂળભૂત:
        જો (!MODESEGMENT)
            પાછા DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        પાછા call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
પૂર્ણાંક VISIBLE32FLAT
process_op_32(સંરચના disk_op_s *op)
{
    ASSERT32FLAT();
    પસંદગી (op->drive_fl->type) {
    સ્થિતિ DTYPE_VIRTIO_BLK:
        પાછા virtio_blk_process_op(op);
    સ્થિતિ DTYPE_AHCI:
        પાછા ahci_process_op(op);
    સ્થિતિ DTYPE_AHCI_ATAPI:
        પાછા ahci_atapi_process_op(op);
    સ્થિતિ DTYPE_SDCARD:
        પાછા sdcard_process_op(op);
    સ્થિતિ DTYPE_USB_32:
        પાછા usb_process_op(op);
    સ્થિતિ DTYPE_UAS_32:
        પાછા uas_process_op(op);
    સ્થિતિ DTYPE_VIRTIO_SCSI:
        પાછા virtio_scsi_process_op(op);
    સ્થિતિ DTYPE_PVSCSI:
        પાછા pvscsi_process_op(op);
    સ્થિતિ DTYPE_NVME:
        પાછા nvme_process_op(op);
    મૂળભૂત:
        પાછા process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
સ્થિર પૂર્ણાંક
process_op_16(સંરચના disk_op_s *op)
{
    ASSERT16();
    પસંદગી (GET_FLATPTR(op->drive_fl->type)) {
    સ્થિતિ DTYPE_FLOPPY:
        પાછા floppy_process_op(op);
    સ્થિતિ DTYPE_ATA:
        પાછા ata_process_op(op);
    સ્થિતિ DTYPE_RAMDISK:
        પાછા ramdisk_process_op(op);
    સ્થિતિ DTYPE_CDEMU:
        પાછા cdemu_process_op(op);
    મૂળભૂત:
        પાછા process_op_both(op);
    }
}

// Execute a disk_op_s request.
પૂર્ણાંક
process_op(સંરચના disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->ગણતરી, op->command);

    પૂર્ણાંક ret, origcount = op->ગણતરી;
    જો (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->ગણતરી = 0;
        પાછા DISK_RET_EBOUNDARY;
    }
    જો (MODESEGMENT)
        ret = process_op_16(op);
    નહીંતર
        ret = process_op_32(op);
    જો (ret && op->ગણતરી == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->ગણતરી = 0;
    પાછા ret;
}
