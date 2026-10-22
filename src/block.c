// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "block.h" // process_op
#समावेश "hw/ata.h" // process_ata_op
#समावेश "hw/ahci.h" // process_ahci_op
#समावेश "hw/esp-scsi.h" // esp_scsi_process_op
#समावेश "hw/lsi-scsi.h" // lsi_scsi_process_op
#समावेश "hw/megasas.h" // megasas_process_op
#समावेश "hw/mpt-scsi.h" // mpt_scsi_process_op
#समावेश "hw/pci.h" // pci_bdf_to_bus
#समावेश "hw/pvscsi.h" // pvscsi_process_op
#समावेश "hw/rtc.h" // rtc_read
#समावेश "hw/usb-msc.h" // usb_process_op
#समावेश "hw/usb-uas.h" // uas_process_op
#समावेश "hw/virtio-blk.h" // process_virtio_blk_op
#समावेश "hw/virtio-scsi.h" // virtio_scsi_process_op
#समावेश "hw/nvme.h" // nvme_process_op
#समावेश "malloc.h" // malloc_low
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // call32
#समावेश "std/disk.h" // struct dpte_s
#समावेश "string.h" // checksum
#समावेश "util.h" // process_floppy_op

u8 Floppy_गिनती VARFSEG;
u8 CDCount;
काष्ठा drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

काष्ठा drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    अगर (extdriveoffset >= ARRAY_आकार(IDMap[0]))
        वापस शून्य;
    वापस GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

पूर्णांक getDriveId(u8 exttype, काष्ठा drive_s *drive)
{
    ASSERT32FLAT();
    पूर्णांक i;
    क्रम (i = 0; i < ARRAY_आकार(IDMap[0]); i++)
        अगर (getDrive(exttype, i) == drive)
            वापस i;
    वापस -1;
}

पूर्णांक create_bounce_buf(व्योम)
{
    अगर (bounce_buf_fl)
        वापस 0;

    u8 *buf = malloc_निचला(CDROM_SECTOR_आकार);
    अगर (!buf) {
        warn_noalloc();
        वापस -1;
    }
    bounce_buf_fl = buf;
    वापस 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

अटल पूर्णांक
host_lchs_supplied(काष्ठा drive_s *drive)
{
    वापस (drive->lchs.शीर्ष <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

अटल u8
get_translation(काष्ठा drive_s *drive)
{
    अगर (host_lchs_supplied(drive))
        वापस TRANSLATION_HOST;
    u8 type = drive->type;
    अगर (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        वापस translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.शीर्ष;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    अगर (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        वापस TRANSLATION_LBA;

    अगर (cylinders <= 1024 && heads <= 16 && spt <= 63)
        वापस TRANSLATION_NONE;
    अगर (cylinders * heads <= 131072)
        वापस TRANSLATION_LARGE;
    वापस TRANSLATION_LBA;
}

अटल व्योम
setup_translation(काष्ठा drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.शीर्ष ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    स्थिर अक्षर *desc = शून्य;

    चयन (translation) {
    शेष:
    हाल TRANSLATION_NONE:
        desc = "none";
        अवरोध;
    हाल TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        अगर (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            अवरोध;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        अगर (heads>128)
            heads = 255;
        अन्यथा अगर (heads>64)
            heads = 128;
        अन्यथा अगर (heads>32)
            heads = 64;
        अन्यथा अगर (heads>16)
            heads = 32;
        अन्यथा
            heads = 16;
        cylinders = sect / heads;
        अवरोध;
    हाल TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        अगर (heads==16) {
            अगर (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    हाल TRANSLATION_LARGE:
        अगर (translation == TRANSLATION_LARGE)
            desc = "large";
        जबतक (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            अगर (heads > 127)
                अवरोध;
        }
        अवरोध;
    हाल TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.शीर्ष;
        spt = drive->lchs.sector;
        अवरोध;
    }
    // clip to 1024 cylinders in lchs
    अगर (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%u\n"
            , drive
            , drive->pchs.cylinder, drive->pchs.शीर्ष, drive->pchs.sector
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    drive->lchs.शीर्ष = heads;
    drive->lchs.cylinder = cylinders;
    drive->lchs.sector = spt;
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
अटल व्योम
fill_fdpt(काष्ठा drive_s *drive, पूर्णांक hdid)
{
    अगर (hdid > 1)
        वापस;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.शीर्ष;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.शीर्ष;
    u16 nps = drive->pchs.sector;

    काष्ठा fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    अगर (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, माप(*fdpt));
    }

    अगर (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), दुरत्व(
                                 काष्ठा extended_bios_तथ्य_area_s, fdpt[0])));
    अन्यथा
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), दुरत्व(
                                 काष्ठा extended_bios_तथ्य_area_s, fdpt[1])));
}

// Find spot to add a drive
अटल व्योम
add_drive(काष्ठा drive_s **idmap, u8 *गिनती, काष्ठा drive_s *drive)
{
    अगर (*गिनती >= ARRAY_आकार(IDMap[0])) {
        warn_noalloc();
        वापस;
    }
    idmap[*गिनती] = drive;
    *गिनती = *गिनती + 1;
}

// Map a hard drive
व्योम
map_hd_drive(काष्ठा drive_s *drive)
{
    ASSERT32FLAT();
    काष्ठा bios_तथ्य_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    पूर्णांक hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
व्योम
map_cd_drive(काष्ठा drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
व्योम
map_floppy_drive(काष्ठा drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_गिनती, drive);

    // Update equipment word bits for floppy
    अगर (Floppy_गिनती == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } अन्यथा अगर (Floppy_गिनती >= 2) {
        // 2 drives, ready for boot
        set_equipment_flags(0x41, 0x41);
        SET_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * Extended Disk Drive (EDD) get drive parameters
 ****************************************************************/

// flags for bus_iface field in fill_generic_edd()
#घोषणा EDD_ISA        0x01
#घोषणा EDD_PCI        0x02
#घोषणा EDD_BUS_MASK   0x0f
#घोषणा EDD_ATA        0x10
#घोषणा EDD_SCSI       0x20
#घोषणा EDD_IFACE_MASK 0xf0

// Fill in EDD info
अटल पूर्णांक
fill_generic_edd(काष्ठा segoff_s edd, काष्ठा drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    काष्ठा int13dpt_s *param_far = (व्योम*)(edd.offset+0);
    u16 आकार = GET_FARVAR(seg, param_far->आकार);
    u16 t13 = आकार == 74;

    // Buffer is too small
    अगर (आकार < 26)
        वापस DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.शीर्ष);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , आकार, type, npc, nph, nps, (u32)lba, blksize);

    SET_FARVAR(seg, param_far->आकार, 26);
    अगर (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } अन्यथा {
        अगर (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } अन्यथा {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_गिनती, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    अगर (आकार < 30 || !dpte_so)
        वापस DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->आकार, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    अगर (आकार < 66 || !bus_iface)
        वापस DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->कुंजी, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_लंबाई, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    स्थिर अक्षर *host_bus = "ISA ";
    अगर ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        अगर (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , माप(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    स्थिर अक्षर *iface_type = "ATA     ";
    अगर ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , माप(param_far->iface_type));
    अगर (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (व्योम*)param_far+30, 43));
    } अन्यथा {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (व्योम*)param_far+30, 35));
    }

    वापस DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
अटल u32
edd_pci_path(u16 bdf, u8 channel)
{
    वापस (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

काष्ठा dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
अटल पूर्णांक
fill_ata_edd(काष्ठा segoff_s edd, काष्ठा drive_s *drive_gf)
{
    अगर (!CONFIG_ATA)
        वापस DISK_RET_EPARAM;

    // Fill in dpte
    काष्ठा atadrive_s *adrive_gf = container_of(
        drive_gf, काष्ठा atadrive_s, drive);
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    पूर्णांक bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    अगर (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        अगर ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            अगर (translation == TRANSLATION_LBA)
                options |= 1<<9;
            अगर (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } अन्यथा {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    अगर (CONFIG_ATA_PIO32)
        options |= 1<<7;

    SET_निचला(DefaultDPTE.iobase1, iobase1);
    SET_निचला(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    SET_निचला(DefaultDPTE.prefix, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    SET_निचला(DefaultDPTE.unused, 0xcb);
    SET_निचला(DefaultDPTE.irq, irq);
    SET_निचला(DefaultDPTE.blkcount, 1);
    SET_निचला(DefaultDPTE.dma, 0);
    SET_निचला(DefaultDPTE.pio, 0);
    SET_निचला(DefaultDPTE.options, options);
    SET_निचला(DefaultDPTE.reserved, 0);
    SET_निचला(DefaultDPTE.revision, 0x11);

    u8 योग = checksum_far(SEG_निचला, &DefaultDPTE, 15);
    SET_निचला(DefaultDPTE.checksum, -योग);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    अगर (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    वापस fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_निचला, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
पूर्णांक noinline
fill_edd(काष्ठा segoff_s edd, काष्ठा drive_s *drive_fl)
{
    चयन (GET_FLATPTR(drive_fl->type)) {
    हाल DTYPE_ATA:
    हाल DTYPE_ATA_ATAPI:
        वापस fill_ata_edd(edd, drive_fl);
    हाल DTYPE_VIRTIO_BLK:
    हाल DTYPE_VIRTIO_SCSI:
        वापस fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    शेष:
        वापस fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

व्योम
block_setup(व्योम)
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
पूर्णांक
default_process_op(काष्ठा disk_op_s *op)
{
    चयन (op->command) {
    हाल CMD_FORMAT:
    हाल CMD_RESET:
    हाल CMD_ISREADY:
    हाल CMD_VERIFY:
    हाल CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        वापस DISK_RET_SUCCESS;
    शेष:
        वापस DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
अटल पूर्णांक
process_op_both(काष्ठा disk_op_s *op)
{
    चयन (GET_FLATPTR(op->drive_fl->type)) {
    हाल DTYPE_ATA_ATAPI:
        वापस ata_atapi_process_op(op);
    हाल DTYPE_USB:
        वापस usb_process_op(op);
    हाल DTYPE_UAS:
        वापस uas_process_op(op);
    हाल DTYPE_LSI_SCSI:
        वापस lsi_scsi_process_op(op);
    हाल DTYPE_ESP_SCSI:
        वापस esp_scsi_process_op(op);
    हाल DTYPE_MEGASAS:
        वापस megasas_process_op(op);
    हाल DTYPE_MPT_SCSI:
        वापस mpt_scsi_process_op(op);
    शेष:
        अगर (!MODESEGMENT)
            वापस DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        वापस call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
पूर्णांक VISIBLE32FLAT
process_op_32(काष्ठा disk_op_s *op)
{
    ASSERT32FLAT();
    चयन (op->drive_fl->type) {
    हाल DTYPE_VIRTIO_BLK:
        वापस virtio_blk_process_op(op);
    हाल DTYPE_AHCI:
        वापस ahci_process_op(op);
    हाल DTYPE_AHCI_ATAPI:
        वापस ahci_atapi_process_op(op);
    हाल DTYPE_SDCARD:
        वापस sdcard_process_op(op);
    हाल DTYPE_USB_32:
        वापस usb_process_op(op);
    हाल DTYPE_UAS_32:
        वापस uas_process_op(op);
    हाल DTYPE_VIRTIO_SCSI:
        वापस virtio_scsi_process_op(op);
    हाल DTYPE_PVSCSI:
        वापस pvscsi_process_op(op);
    हाल DTYPE_NVME:
        वापस nvme_process_op(op);
    शेष:
        वापस process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
अटल पूर्णांक
process_op_16(काष्ठा disk_op_s *op)
{
    ASSERT16();
    चयन (GET_FLATPTR(op->drive_fl->type)) {
    हाल DTYPE_FLOPPY:
        वापस floppy_process_op(op);
    हाल DTYPE_ATA:
        वापस ata_process_op(op);
    हाल DTYPE_RAMDISK:
        वापस ramdisk_process_op(op);
    हाल DTYPE_CDEMU:
        वापस cdemu_process_op(op);
    शेष:
        वापस process_op_both(op);
    }
}

// Execute a disk_op_s request.
पूर्णांक
process_op(काष्ठा disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->गिनती, op->command);

    पूर्णांक ret, origcount = op->गिनती;
    अगर (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->गिनती = 0;
        वापस DISK_RET_EBOUNDARY;
    }
    अगर (MODESEGMENT)
        ret = process_op_16(op);
    अन्यथा
        ret = process_op_32(op);
    अगर (ret && op->गिनती == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->गिनती = 0;
    वापस ret;
}
