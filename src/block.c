// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "block.h" // process_op
#अन्तर्भाव "hw/ata.h" // process_ata_op
#अन्तर्भाव "hw/ahci.h" // process_ahci_op
#अन्तर्भाव "hw/esp-scsi.h" // esp_scsi_process_op
#अन्तर्भाव "hw/lsi-scsi.h" // lsi_scsi_process_op
#अन्तर्भाव "hw/megasas.h" // megasas_process_op
#अन्तर्भाव "hw/mpt-scsi.h" // mpt_scsi_process_op
#अन्तर्भाव "hw/pci.h" // pci_bdf_to_bus
#अन्तर्भाव "hw/pvscsi.h" // pvscsi_process_op
#अन्तर्भाव "hw/rtc.h" // rtc_read
#अन्तर्भाव "hw/usb-msc.h" // usb_process_op
#अन्तर्भाव "hw/usb-uas.h" // uas_process_op
#अन्तर्भाव "hw/virtio-blk.h" // process_virtio_blk_op
#अन्तर्भाव "hw/virtio-scsi.h" // virtio_scsi_process_op
#अन्तर्भाव "hw/nvme.h" // nvme_process_op
#अन्तर्भाव "malloc.h" // malloc_low
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // call32
#अन्तर्भाव "std/disk.h" // struct dpte_s
#अन्तर्भाव "string.h" // checksum
#अन्तर्भाव "util.h" // process_floppy_op

u8 Floppy_गणनम् VARFSEG;
u8 CDCount;
संरचना drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

संरचना drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    यदि (extdriveoffset >= ARRAY_आकार(IDMap[0]))
        प्रत्यागम रिक्त;
    प्रत्यागम GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

पूर्णाङ्क getDriveId(u8 exttype, संरचना drive_s *drive)
{
    ASSERT32FLAT();
    पूर्णाङ्क i;
    प्रति (i = 0; i < ARRAY_आकार(IDMap[0]); i++)
        यदि (getDrive(exttype, i) == drive)
            प्रत्यागम i;
    प्रत्यागम -1;
}

पूर्णाङ्क create_bounce_buf(शून्य)
{
    यदि (bounce_buf_fl)
        प्रत्यागम 0;

    u8 *buf = malloc_नीचः(CDROM_SECTOR_आकार);
    यदि (!buf) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    bounce_buf_fl = buf;
    प्रत्यागम 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

स्थिर पूर्णाङ्क
host_lchs_supplied(संरचना drive_s *drive)
{
    प्रत्यागम (drive->lchs.शीर्ष <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

स्थिर u8
get_translation(संरचना drive_s *drive)
{
    यदि (host_lchs_supplied(drive))
        प्रत्यागम TRANSLATION_HOST;
    u8 type = drive->type;
    यदि (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        प्रत्यागम translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.शीर्ष;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    यदि (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        प्रत्यागम TRANSLATION_LBA;

    यदि (cylinders <= 1024 && heads <= 16 && spt <= 63)
        प्रत्यागम TRANSLATION_NONE;
    यदि (cylinders * heads <= 131072)
        प्रत्यागम TRANSLATION_LARGE;
    प्रत्यागम TRANSLATION_LBA;
}

स्थिर शून्य
सज्जता_translation(संरचना drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.शीर्ष ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    ध्रुव वर्ण *desc = रिक्त;

    विकल्प (translation) {
    स्वभाव:
    पक्ष TRANSLATION_NONE:
        desc = "none";
        भङ्ग;
    पक्ष TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        यदि (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            भङ्ग;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        यदि (heads>128)
            heads = 255;
        अन्यथा यदि (heads>64)
            heads = 128;
        अन्यथा यदि (heads>32)
            heads = 64;
        अन्यथा यदि (heads>16)
            heads = 32;
        अन्यथा
            heads = 16;
        cylinders = sect / heads;
        भङ्ग;
    पक्ष TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        यदि (heads==16) {
            यदि (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    पक्ष TRANSLATION_LARGE:
        यदि (translation == TRANSLATION_LARGE)
            desc = "large";
        यावत् (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            यदि (heads > 127)
                भङ्ग;
        }
        भङ्ग;
    पक्ष TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.शीर्ष;
        spt = drive->lchs.sector;
        भङ्ग;
    }
    // clip to 1024 cylinders in lchs
    यदि (cylinders > 1024)
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
स्थिर शून्य
fill_fdpt(संरचना drive_s *drive, पूर्णाङ्क hdid)
{
    यदि (hdid > 1)
        प्रत्यागम;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.शीर्ष;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.शीर्ष;
    u16 nps = drive->pchs.sector;

    संरचना fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    यदि (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, परिमाण(*fdpt));
    }

    यदि (hdid == 0)
        समुच्चयः_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 संरचना extended_bios_दत्त_area_s, fdpt[0])));
    अन्यथा
        समुच्चयः_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 संरचना extended_bios_दत्त_area_s, fdpt[1])));
}

// Find spot to add a drive
स्थिर शून्य
सङ्कलय_drive(संरचना drive_s **idmap, u8 *गणनम्, संरचना drive_s *drive)
{
    यदि (*गणनम् >= ARRAY_आकार(IDMap[0])) {
        warn_noalloc();
        प्रत्यागम;
    }
    idmap[*गणनम्] = drive;
    *गणनम् = *गणनम् + 1;
}

// Map a hard drive
शून्य
map_hd_drive(संरचना drive_s *drive)
{
    ASSERT32FLAT();
    संरचना bios_दत्त_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    पूर्णाङ्क hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    सङ्कलय_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    सज्जता_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
शून्य
map_cd_drive(संरचना drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    सङ्कलय_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
शून्य
map_floppy_drive(संरचना drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    सङ्कलय_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_गणनम्, drive);

    // Update equipment word bits for floppy
    यदि (Floppy_गणनम् == 1) {
        // 1 drive, ready for boot
        समुच्चयः_equipment_flags(0x41, 0x01);
        समुच्चयः_BDA(floppy_harddisk_info, 0x07);
    } अन्यथा यदि (Floppy_गणनम् >= 2) {
        // 2 drives, ready for boot
        समुच्चयः_equipment_flags(0x41, 0x41);
        समुच्चयः_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * Extended Disk Drive (EDD) get drive parameters
 ****************************************************************/

// flags for bus_iface field in fill_generic_edd()
#परिभाषय EDD_ISA        0x01
#परिभाषय EDD_PCI        0x02
#परिभाषय EDD_BUS_आवरणम्   0x0f
#परिभाषय EDD_ATA        0x10
#परिभाषय EDD_SCSI       0x20
#परिभाषय EDD_IFACE_आवरणम् 0xf0

// Fill in EDD info
स्थिर पूर्णाङ्क
fill_generic_edd(संरचना segoff_s edd, संरचना drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_मार्गः, u32 device_मार्गः)
{
    u16 seg = edd.seg;
    संरचना int13dpt_s *param_far = (शून्य*)(edd.विस्थापनम्_2+0);
    u16 आकार = GET_FARVAR(seg, param_far->आकार);
    u16 t13 = आकार == 74;

    // Buffer is too small
    यदि (आकार < 26)
        प्रत्यागम DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.शीर्ष);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , आकार, type, npc, nph, nps, (u32)lba, blksize);

    समुच्चयः_FARVAR(seg, param_far->आकार, 26);
    यदि (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        समुच्चयः_FARVAR(seg, param_far->infos, 0x74);
        समुच्चयः_FARVAR(seg, param_far->cylinders, 0xffffffff);
        समुच्चयः_FARVAR(seg, param_far->heads, 0xffffffff);
        समुच्चयः_FARVAR(seg, param_far->spt, 0xffffffff);
    } अन्यथा {
        यदि (lba > (u64)nps*nph*0x3fff) {
            समुच्चयः_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            समुच्चयः_FARVAR(seg, param_far->cylinders, 0x3fff);
        } अन्यथा {
            समुच्चयः_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            समुच्चयः_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        समुच्चयः_FARVAR(seg, param_far->heads, (u32)nph);
        समुच्चयः_FARVAR(seg, param_far->spt, (u32)nps);
    }
    समुच्चयः_FARVAR(seg, param_far->sector_गणनम्, lba);
    समुच्चयः_FARVAR(seg, param_far->blksize, blksize);

    यदि (आकार < 30 || !dpte_so)
        प्रत्यागम DISK_RET_SUCCESS;

    // EDD 2.x

    समुच्चयः_FARVAR(seg, param_far->आकार, 30);
    समुच्चयः_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    यदि (आकार < 66 || !bus_iface)
        प्रत्यागम DISK_RET_SUCCESS;

    // EDD 3.x
    समुच्चयः_FARVAR(seg, param_far->कुञ्चिका, 0xbedd);
    समुच्चयः_FARVAR(seg, param_far->dpi_दीर्घता, t13 ? 44 : 36);
    समुच्चयः_FARVAR(seg, param_far->reserved1, 0);
    समुच्चयः_FARVAR(seg, param_far->reserved2, 0);

    ध्रुव वर्ण *host_bus = "ISA ";
    यदि ((bus_iface & EDD_BUS_आवरणम्) == EDD_PCI) {
        host_bus = "PCI ";
        यदि (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_मार्गः &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , परिमाण(param_far->host_bus));
    समुच्चयः_FARVAR(seg, param_far->iface_मार्गः, iface_मार्गः);

    ध्रुव वर्ण *iface_type = "ATA     ";
    यदि ((bus_iface & EDD_IFACE_आवरणम्) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , परिमाण(param_far->iface_type));
    यदि (t13) {
        समुच्चयः_FARVAR(seg, param_far->t13.device_मार्गः[0], device_मार्गः);
        समुच्चयः_FARVAR(seg, param_far->t13.device_मार्गः[1], 0);

        समुच्चयः_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (शून्य*)param_far+30, 43));
    } अन्यथा {
        समुच्चयः_FARVAR(seg, param_far->phoenix.device_मार्गः, device_मार्गः);

        समुच्चयः_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (शून्य*)param_far+30, 35));
    }

    प्रत्यागम DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
स्थिर u32
edd_pci_मार्गः(u16 bdf, u8 channel)
{
    प्रत्यागम (pci_bdf_गन्तव्यम्_bus(bdf) | (pci_bdf_गन्तव्यम्_dev(bdf) << 8)
            | (pci_bdf_गन्तव्यम्_fn(bdf) << 16) | ((u32)channel << 24));
}

संरचना dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
स्थिर पूर्णाङ्क
fill_ata_edd(संरचना segoff_s edd, संरचना drive_s *drive_gf)
{
    यदि (!CONFIG_ATA)
        प्रत्यागम DISK_RET_EPARAM;

    // Fill in dpte
    संरचना atadrive_s *adrive_gf = container_सम्बन्धि(
        drive_gf, संरचना atadrive_s, drive);
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    पूर्णाङ्क bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    यदि (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        यदि ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            यदि (translation == TRANSLATION_LBA)
                options |= 1<<9;
            यदि (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } अन्यथा {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    यदि (CONFIG_ATA_PIO32)
        options |= 1<<7;

    समुच्चयः_नीचः(DefaultDPTE.iobase1, iobase1);
    समुच्चयः_नीचः(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    समुच्चयः_नीचः(DefaultDPTE.उपसर्गः, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    समुच्चयः_नीचः(DefaultDPTE.अप्रयुक्तम्, 0xcb);
    समुच्चयः_नीचः(DefaultDPTE.irq, irq);
    समुच्चयः_नीचः(DefaultDPTE.blkcount, 1);
    समुच्चयः_नीचः(DefaultDPTE.dma, 0);
    समुच्चयः_नीचः(DefaultDPTE.pio, 0);
    समुच्चयः_नीचः(DefaultDPTE.options, options);
    समुच्चयः_नीचः(DefaultDPTE.reserved, 0);
    समुच्चयः_नीचः(DefaultDPTE.revision, 0x11);

    u8 योग = checksum_far(SEG_नीचः, &DefaultDPTE, 15);
    समुच्चयः_नीचः(DefaultDPTE.checksum, -योग);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    यदि (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_मार्गः(bdf, channel);
    }
    प्रत्यागम fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_नीचः, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
पूर्णाङ्क noinline
fill_edd(संरचना segoff_s edd, संरचना drive_s *drive_fl)
{
    विकल्प (GET_FLATPTR(drive_fl->type)) {
    पक्ष DTYPE_ATA:
    पक्ष DTYPE_ATA_ATAPI:
        प्रत्यागम fill_ata_edd(edd, drive_fl);
    पक्ष DTYPE_VIRTIO_BLK:
    पक्ष DTYPE_VIRTIO_SCSI:
        प्रत्यागम fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_मार्गः(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    स्वभाव:
        प्रत्यागम fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

शून्य
खण्डम्_सज्जता(शून्य)
{
    floppy_सज्जता();
    ata_सज्जता();
    ahci_सज्जता();
    sdcard_सज्जता();
    ramdisk_सज्जता();
    virtio_blk_सज्जता();
    virtio_scsi_सज्जता();
    lsi_scsi_सज्जता();
    esp_scsi_सज्जता();
    megasas_सज्जता();
    pvscsi_सज्जता();
    mpt_scsi_सज्जता();
    nvme_सज्जता();
}

// Fallback handler for command requests not implemented by drivers
पूर्णाङ्क
default_process_op(संरचना disk_op_s *op)
{
    विकल्प (op->command) {
    पक्ष CMD_FORMAT:
    पक्ष CMD_RESET:
    पक्ष CMD_ISREADY:
    पक्ष CMD_VERIFY:
    पक्ष CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        प्रत्यागम DISK_RET_SUCCESS;
    स्वभाव:
        प्रत्यागम DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
स्थिर पूर्णाङ्क
process_op_both(संरचना disk_op_s *op)
{
    विकल्प (GET_FLATPTR(op->drive_fl->type)) {
    पक्ष DTYPE_ATA_ATAPI:
        प्रत्यागम ata_atapi_process_op(op);
    पक्ष DTYPE_USB:
        प्रत्यागम usb_process_op(op);
    पक्ष DTYPE_UAS:
        प्रत्यागम uas_process_op(op);
    पक्ष DTYPE_LSI_SCSI:
        प्रत्यागम lsi_scsi_process_op(op);
    पक्ष DTYPE_ESP_SCSI:
        प्रत्यागम esp_scsi_process_op(op);
    पक्ष DTYPE_MEGASAS:
        प्रत्यागम megasas_process_op(op);
    पक्ष DTYPE_MPT_SCSI:
        प्रत्यागम mpt_scsi_process_op(op);
    स्वभाव:
        यदि (!MODESEGMENT)
            प्रत्यागम DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        प्रत्यागम call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
पूर्णाङ्क VISIBLE32FLAT
process_op_32(संरचना disk_op_s *op)
{
    ASSERT32FLAT();
    विकल्प (op->drive_fl->type) {
    पक्ष DTYPE_VIRTIO_BLK:
        प्रत्यागम virtio_blk_process_op(op);
    पक्ष DTYPE_AHCI:
        प्रत्यागम ahci_process_op(op);
    पक्ष DTYPE_AHCI_ATAPI:
        प्रत्यागम ahci_atapi_process_op(op);
    पक्ष DTYPE_SDCARD:
        प्रत्यागम sdcard_process_op(op);
    पक्ष DTYPE_USB_32:
        प्रत्यागम usb_process_op(op);
    पक्ष DTYPE_UAS_32:
        प्रत्यागम uas_process_op(op);
    पक्ष DTYPE_VIRTIO_SCSI:
        प्रत्यागम virtio_scsi_process_op(op);
    पक्ष DTYPE_PVSCSI:
        प्रत्यागम pvscsi_process_op(op);
    पक्ष DTYPE_NVME:
        प्रत्यागम nvme_process_op(op);
    स्वभाव:
        प्रत्यागम process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
स्थिर पूर्णाङ्क
process_op_16(संरचना disk_op_s *op)
{
    ASSERT16();
    विकल्प (GET_FLATPTR(op->drive_fl->type)) {
    पक्ष DTYPE_FLOPPY:
        प्रत्यागम floppy_process_op(op);
    पक्ष DTYPE_ATA:
        प्रत्यागम ata_process_op(op);
    पक्ष DTYPE_RAMDISK:
        प्रत्यागम ramdisk_process_op(op);
    पक्ष DTYPE_CDEMU:
        प्रत्यागम cdemu_process_op(op);
    स्वभाव:
        प्रत्यागम process_op_both(op);
    }
}

// Execute a disk_op_s request.
पूर्णाङ्क
process_op(संरचना disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->गणनम्, op->command);

    पूर्णाङ्क ret, origcount = op->गणनम्;
    यदि (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->गणनम् = 0;
        प्रत्यागम DISK_RET_EBOUNDARY;
    }
    यदि (MODESEGMENT)
        ret = process_op_16(op);
    अन्यथा
        ret = process_op_32(op);
    यदि (ret && op->गणनम् == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->गणनम् = 0;
    प्रत्यागम ret;
}
