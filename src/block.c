// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "block.h" // process_op
#అన్తర్భావ "hw/ata.h" // process_ata_op
#అన్తర్భావ "hw/ahci.h" // process_ahci_op
#అన్తర్భావ "hw/esp-scsi.h" // esp_scsi_process_op
#అన్తర్భావ "hw/lsi-scsi.h" // lsi_scsi_process_op
#అన్తర్భావ "hw/megasas.h" // megasas_process_op
#అన్తర్భావ "hw/mpt-scsi.h" // mpt_scsi_process_op
#అన్తర్భావ "hw/pci.h" // pci_bdf_to_bus
#అన్తర్భావ "hw/pvscsi.h" // pvscsi_process_op
#అన్తర్భావ "hw/rtc.h" // rtc_read
#అన్తర్భావ "hw/usb-msc.h" // usb_process_op
#అన్తర్భావ "hw/usb-uas.h" // uas_process_op
#అన్తర్భావ "hw/virtio-blk.h" // process_virtio_blk_op
#అన్తర్భావ "hw/virtio-scsi.h" // virtio_scsi_process_op
#అన్తర్భావ "hw/nvme.h" // nvme_process_op
#అన్తర్భావ "malloc.h" // malloc_low
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // call32
#అన్తర్భావ "std/disk.h" // struct dpte_s
#అన్తర్భావ "string.h" // checksum
#అన్తర్భావ "util.h" // process_floppy_op

u8 Floppy_లెక్క VARFSEG;
u8 CDCount;
నిర్మాణం drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

నిర్మాణం drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    ఒకవేళ (extdriveoffset >= ARRAY_కొలత(IDMap[0]))
        తిరిగి ఖాళీ;
    తిరిగి GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

పూర్ణాంకం getDriveId(u8 exttype, నిర్మాణం drive_s *drive)
{
    ASSERT32FLAT();
    పూర్ణాంకం i;
    కొరకు (i = 0; i < ARRAY_కొలత(IDMap[0]); i++)
        ఒకవేళ (getDrive(exttype, i) == drive)
            తిరిగి i;
    తిరిగి -1;
}

పూర్ణాంకం create_bounce_buf(శూన్య)
{
    ఒకవేళ (bounce_buf_fl)
        తిరిగి 0;

    u8 *buf = malloc_దిగువ(CDROM_SECTOR_కొలత);
    ఒకవేళ (!buf) {
        warn_noalloc();
        తిరిగి -1;
    }
    bounce_buf_fl = buf;
    తిరిగి 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

స్థిర పూర్ణాంకం
host_lchs_supplied(నిర్మాణం drive_s *drive)
{
    తిరిగి (drive->lchs.తల <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

స్థిర u8
get_translation(నిర్మాణం drive_s *drive)
{
    ఒకవేళ (host_lchs_supplied(drive))
        తిరిగి TRANSLATION_HOST;
    u8 type = drive->type;
    ఒకవేళ (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        తిరిగి translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.తల;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    ఒకవేళ (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        తిరిగి TRANSLATION_LBA;

    ఒకవేళ (cylinders <= 1024 && heads <= 16 && spt <= 63)
        తిరిగి TRANSLATION_NONE;
    ఒకవేళ (cylinders * heads <= 131072)
        తిరిగి TRANSLATION_LARGE;
    తిరిగి TRANSLATION_LBA;
}

స్థిర శూన్య
అమరిక_translation(నిర్మాణం drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.తల ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    స్థిరరాశి అక్షరం *desc = ఖాళీ;

    ఎంపిక (translation) {
    అప్రమేయ:
    స్థితి TRANSLATION_NONE:
        desc = "none";
        ఆపు;
    స్థితి TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        ఒకవేళ (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            ఆపు;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        ఒకవేళ (heads>128)
            heads = 255;
        లేకపోతే ఒకవేళ (heads>64)
            heads = 128;
        లేకపోతే ఒకవేళ (heads>32)
            heads = 64;
        లేకపోతే ఒకవేళ (heads>16)
            heads = 32;
        లేకపోతే
            heads = 16;
        cylinders = sect / heads;
        ఆపు;
    స్థితి TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        ఒకవేళ (heads==16) {
            ఒకవేళ (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    స్థితి TRANSLATION_LARGE:
        ఒకవేళ (translation == TRANSLATION_LARGE)
            desc = "large";
        అయితే (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            ఒకవేళ (heads > 127)
                ఆపు;
        }
        ఆపు;
    స్థితి TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.తల;
        spt = drive->lchs.sector;
        ఆపు;
    }
    // clip to 1024 cylinders in lchs
    ఒకవేళ (cylinders > 1024)
        cylinders = 1024;
    dprintf(1, "drive %p: PCHS=%u/%d/%d translation=%s LCHS=%d/%d/%d s=%u\n"
            , drive
            , drive->pchs.cylinder, drive->pchs.తల, drive->pchs.sector
            , desc
            , cylinders, heads, spt
            , (u32)sectors);

    drive->lchs.తల = heads;
    drive->lchs.cylinder = cylinders;
    drive->lchs.sector = spt;
}


/****************************************************************
 * Drive mapping
 ****************************************************************/

// Fill in Fixed Disk Parameter Table (located in ebda).
స్థిర శూన్య
fill_fdpt(నిర్మాణం drive_s *drive, పూర్ణాంకం hdid)
{
    ఒకవేళ (hdid > 1)
        తిరిగి;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.తల;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.తల;
    u16 nps = drive->pchs.sector;

    నిర్మాణం fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    ఒకవేళ (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, పరిమాణం(*fdpt));
    }

    ఒకవేళ (hdid == 0)
        సమితి_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 నిర్మాణం extended_bios_దత్తాంశం_area_s, fdpt[0])));
    లేకపోతే
        సమితి_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 నిర్మాణం extended_bios_దత్తాంశం_area_s, fdpt[1])));
}

// Find spot to add a drive
స్థిర శూన్య
కూడు_drive(నిర్మాణం drive_s **idmap, u8 *లెక్క, నిర్మాణం drive_s *drive)
{
    ఒకవేళ (*లెక్క >= ARRAY_కొలత(IDMap[0])) {
        warn_noalloc();
        తిరిగి;
    }
    idmap[*లెక్క] = drive;
    *లెక్క = *లెక్క + 1;
}

// Map a hard drive
శూన్య
map_hd_drive(నిర్మాణం drive_s *drive)
{
    ASSERT32FLAT();
    నిర్మాణం bios_దత్తాంశం_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    పూర్ణాంకం hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    కూడు_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    అమరిక_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
శూన్య
map_cd_drive(నిర్మాణం drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    కూడు_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
శూన్య
map_floppy_drive(నిర్మాణం drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    కూడు_drive(IDMap[EXTTYPE_FLOPPY], &Floppy_లెక్క, drive);

    // Update equipment word bits for floppy
    ఒకవేళ (Floppy_లెక్క == 1) {
        // 1 drive, ready for boot
        సమితి_equipment_flags(0x41, 0x01);
        సమితి_BDA(floppy_harddisk_info, 0x07);
    } లేకపోతే ఒకవేళ (Floppy_లెక్క >= 2) {
        // 2 drives, ready for boot
        సమితి_equipment_flags(0x41, 0x41);
        సమితి_BDA(floppy_harddisk_info, 0x77);
    }
}


/****************************************************************
 * Extended Disk Drive (EDD) get drive parameters
 ****************************************************************/

// flags for bus_iface field in fill_generic_edd()
#నిర్వచించు EDD_ISA        0x01
#నిర్వచించు EDD_PCI        0x02
#నిర్వచించు EDD_BUS_ముసుగు   0x0f
#నిర్వచించు EDD_ATA        0x10
#నిర్వచించు EDD_SCSI       0x20
#నిర్వచించు EDD_IFACE_ముసుగు 0xf0

// Fill in EDD info
స్థిర పూర్ణాంకం
fill_generic_edd(నిర్మాణం segoff_s edd, నిర్మాణం drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_మార్గం, u32 device_మార్గం)
{
    u16 seg = edd.seg;
    నిర్మాణం int13dpt_s *param_far = (శూన్య*)(edd.స్థానభ్రంశం_2+0);
    u16 కొలత = GET_FARVAR(seg, param_far->కొలత);
    u16 t13 = కొలత == 74;

    // Buffer is too small
    ఒకవేళ (కొలత < 26)
        తిరిగి DISK_RET_EPARAM;

    // EDD 1.x

    u8  type    = GET_FLATPTR(drive_fl->type);
    u16 npc     = GET_FLATPTR(drive_fl->pchs.cylinder);
    u16 nph     = GET_FLATPTR(drive_fl->pchs.తల);
    u16 nps     = GET_FLATPTR(drive_fl->pchs.sector);
    u64 lba     = GET_FLATPTR(drive_fl->sectors);
    u16 blksize = GET_FLATPTR(drive_fl->blksize);

    dprintf(DEBUG_HDL_13, "disk_1348 size=%d t=%d chs=%d,%d,%d lba=%d bs=%d\n"
            , కొలత, type, npc, nph, nps, (u32)lba, blksize);

    సమితి_FARVAR(seg, param_far->కొలత, 26);
    ఒకవేళ (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        సమితి_FARVAR(seg, param_far->infos, 0x74);
        సమితి_FARVAR(seg, param_far->cylinders, 0xffffffff);
        సమితి_FARVAR(seg, param_far->heads, 0xffffffff);
        సమితి_FARVAR(seg, param_far->spt, 0xffffffff);
    } లేకపోతే {
        ఒకవేళ (lba > (u64)nps*nph*0x3fff) {
            సమితి_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            సమితి_FARVAR(seg, param_far->cylinders, 0x3fff);
        } లేకపోతే {
            సమితి_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            సమితి_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        సమితి_FARVAR(seg, param_far->heads, (u32)nph);
        సమితి_FARVAR(seg, param_far->spt, (u32)nps);
    }
    సమితి_FARVAR(seg, param_far->sector_లెక్క, lba);
    సమితి_FARVAR(seg, param_far->blksize, blksize);

    ఒకవేళ (కొలత < 30 || !dpte_so)
        తిరిగి DISK_RET_SUCCESS;

    // EDD 2.x

    సమితి_FARVAR(seg, param_far->కొలత, 30);
    సమితి_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    ఒకవేళ (కొలత < 66 || !bus_iface)
        తిరిగి DISK_RET_SUCCESS;

    // EDD 3.x
    సమితి_FARVAR(seg, param_far->కీలకం, 0xbedd);
    సమితి_FARVAR(seg, param_far->dpi_నిడివి, t13 ? 44 : 36);
    సమితి_FARVAR(seg, param_far->reserved1, 0);
    సమితి_FARVAR(seg, param_far->reserved2, 0);

    స్థిరరాశి అక్షరం *host_bus = "ISA ";
    ఒకవేళ ((bus_iface & EDD_BUS_ముసుగు) == EDD_PCI) {
        host_bus = "PCI ";
        ఒకవేళ (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_మార్గం &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , పరిమాణం(param_far->host_bus));
    సమితి_FARVAR(seg, param_far->iface_మార్గం, iface_మార్గం);

    స్థిరరాశి అక్షరం *iface_type = "ATA     ";
    ఒకవేళ ((bus_iface & EDD_IFACE_ముసుగు) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , పరిమాణం(param_far->iface_type));
    ఒకవేళ (t13) {
        సమితి_FARVAR(seg, param_far->t13.device_మార్గం[0], device_మార్గం);
        సమితి_FARVAR(seg, param_far->t13.device_మార్గం[1], 0);

        సమితి_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (శూన్య*)param_far+30, 43));
    } లేకపోతే {
        సమితి_FARVAR(seg, param_far->phoenix.device_మార్గం, device_మార్గం);

        సమితి_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (శూన్య*)param_far+30, 35));
    }

    తిరిగి DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
స్థిర u32
edd_pci_మార్గం(u16 bdf, u8 channel)
{
    తిరిగి (pci_bdf_గమ్యం_bus(bdf) | (pci_bdf_గమ్యం_dev(bdf) << 8)
            | (pci_bdf_గమ్యం_fn(bdf) << 16) | ((u32)channel << 24));
}

నిర్మాణం dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
స్థిర పూర్ణాంకం
fill_ata_edd(నిర్మాణం segoff_s edd, నిర్మాణం drive_s *drive_gf)
{
    ఒకవేళ (!CONFIG_ATA)
        తిరిగి DISK_RET_EPARAM;

    // Fill in dpte
    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        drive_gf, నిర్మాణం atadrive_s, drive);
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    పూర్ణాంకం bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    ఒకవేళ (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        ఒకవేళ ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            ఒకవేళ (translation == TRANSLATION_LBA)
                options |= 1<<9;
            ఒకవేళ (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } లేకపోతే {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    ఒకవేళ (CONFIG_ATA_PIO32)
        options |= 1<<7;

    సమితి_దిగువ(DefaultDPTE.iobase1, iobase1);
    సమితి_దిగువ(DefaultDPTE.iobase2, iobase2 + ATA_CB_DC);
    సమితి_దిగువ(DefaultDPTE.ఉపసర్గ, ((slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0)
                                 | ATA_CB_DH_LBA));
    సమితి_దిగువ(DefaultDPTE.వాడని, 0xcb);
    సమితి_దిగువ(DefaultDPTE.irq, irq);
    సమితి_దిగువ(DefaultDPTE.blkcount, 1);
    సమితి_దిగువ(DefaultDPTE.dma, 0);
    సమితి_దిగువ(DefaultDPTE.pio, 0);
    సమితి_దిగువ(DefaultDPTE.options, options);
    సమితి_దిగువ(DefaultDPTE.reserved, 0);
    సమితి_దిగువ(DefaultDPTE.revision, 0x11);

    u8 కూడిక = checksum_far(SEG_దిగువ, &DefaultDPTE, 15);
    సమితి_దిగువ(DefaultDPTE.checksum, -కూడిక);

    u32 bustype = EDD_ISA, ifpath = iobase1;
    ఒకవేళ (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_మార్గం(bdf, channel);
    }
    తిరిగి fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_దిగువ, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
పూర్ణాంకం noinline
fill_edd(నిర్మాణం segoff_s edd, నిర్మాణం drive_s *drive_fl)
{
    ఎంపిక (GET_FLATPTR(drive_fl->type)) {
    స్థితి DTYPE_ATA:
    స్థితి DTYPE_ATA_ATAPI:
        తిరిగి fill_ata_edd(edd, drive_fl);
    స్థితి DTYPE_VIRTIO_BLK:
    స్థితి DTYPE_VIRTIO_SCSI:
        తిరిగి fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_మార్గం(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    అప్రమేయ:
        తిరిగి fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

శూన్య
ఖండం_అమరిక(శూన్య)
{
    floppy_అమరిక();
    ata_అమరిక();
    ahci_అమరిక();
    sdcard_అమరిక();
    ramdisk_అమరిక();
    virtio_blk_అమరిక();
    virtio_scsi_అమరిక();
    lsi_scsi_అమరిక();
    esp_scsi_అమరిక();
    megasas_అమరిక();
    pvscsi_అమరిక();
    mpt_scsi_అమరిక();
    nvme_అమరిక();
}

// Fallback handler for command requests not implemented by drivers
పూర్ణాంకం
default_process_op(నిర్మాణం disk_op_s *op)
{
    ఎంపిక (op->command) {
    స్థితి CMD_FORMAT:
    స్థితి CMD_RESET:
    స్థితి CMD_ISREADY:
    స్థితి CMD_VERIFY:
    స్థితి CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        తిరిగి DISK_RET_SUCCESS;
    అప్రమేయ:
        తిరిగి DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
స్థిర పూర్ణాంకం
process_op_both(నిర్మాణం disk_op_s *op)
{
    ఎంపిక (GET_FLATPTR(op->drive_fl->type)) {
    స్థితి DTYPE_ATA_ATAPI:
        తిరిగి ata_atapi_process_op(op);
    స్థితి DTYPE_USB:
        తిరిగి usb_process_op(op);
    స్థితి DTYPE_UAS:
        తిరిగి uas_process_op(op);
    స్థితి DTYPE_LSI_SCSI:
        తిరిగి lsi_scsi_process_op(op);
    స్థితి DTYPE_ESP_SCSI:
        తిరిగి esp_scsi_process_op(op);
    స్థితి DTYPE_MEGASAS:
        తిరిగి megasas_process_op(op);
    స్థితి DTYPE_MPT_SCSI:
        తిరిగి mpt_scsi_process_op(op);
    అప్రమేయ:
        ఒకవేళ (!MODESEGMENT)
            తిరిగి DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        తిరిగి call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
పూర్ణాంకం VISIBLE32FLAT
process_op_32(నిర్మాణం disk_op_s *op)
{
    ASSERT32FLAT();
    ఎంపిక (op->drive_fl->type) {
    స్థితి DTYPE_VIRTIO_BLK:
        తిరిగి virtio_blk_process_op(op);
    స్థితి DTYPE_AHCI:
        తిరిగి ahci_process_op(op);
    స్థితి DTYPE_AHCI_ATAPI:
        తిరిగి ahci_atapi_process_op(op);
    స్థితి DTYPE_SDCARD:
        తిరిగి sdcard_process_op(op);
    స్థితి DTYPE_USB_32:
        తిరిగి usb_process_op(op);
    స్థితి DTYPE_UAS_32:
        తిరిగి uas_process_op(op);
    స్థితి DTYPE_VIRTIO_SCSI:
        తిరిగి virtio_scsi_process_op(op);
    స్థితి DTYPE_PVSCSI:
        తిరిగి pvscsi_process_op(op);
    స్థితి DTYPE_NVME:
        తిరిగి nvme_process_op(op);
    అప్రమేయ:
        తిరిగి process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
స్థిర పూర్ణాంకం
process_op_16(నిర్మాణం disk_op_s *op)
{
    ASSERT16();
    ఎంపిక (GET_FLATPTR(op->drive_fl->type)) {
    స్థితి DTYPE_FLOPPY:
        తిరిగి floppy_process_op(op);
    స్థితి DTYPE_ATA:
        తిరిగి ata_process_op(op);
    స్థితి DTYPE_RAMDISK:
        తిరిగి ramdisk_process_op(op);
    స్థితి DTYPE_CDEMU:
        తిరిగి cdemu_process_op(op);
    అప్రమేయ:
        తిరిగి process_op_both(op);
    }
}

// Execute a disk_op_s request.
పూర్ణాంకం
process_op(నిర్మాణం disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->లెక్క, op->command);

    పూర్ణాంకం ret, origcount = op->లెక్క;
    ఒకవేళ (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->లెక్క = 0;
        తిరిగి DISK_RET_EBOUNDARY;
    }
    ఒకవేళ (MODESEGMENT)
        ret = process_op_16(op);
    లేకపోతే
        ret = process_op_32(op);
    ఒకవేళ (ret && op->లెక్క == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->లెక్క = 0;
    తిరిగి ret;
}
