// Disk setup and access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBAL
#include "block.h" // process_op
#include "hw/ata.h" // process_ata_op
#include "hw/ahci.h" // process_ahci_op
#include "hw/esp-scsi.h" // esp_scsi_process_op
#include "hw/lsi-scsi.h" // lsi_scsi_process_op
#include "hw/megasas.h" // megasas_process_op
#include "hw/mpt-scsi.h" // mpt_scsi_process_op
#include "hw/pci.h" // pci_bdf_to_bus
#include "hw/pvscsi.h" // pvscsi_process_op
#include "hw/rtc.h" // rtc_read
#include "hw/usb-msc.h" // usb_process_op
#include "hw/usb-uas.h" // uas_process_op
#include "hw/virtio-blk.h" // process_virtio_blk_op
#include "hw/virtio-scsi.h" // virtio_scsi_process_op
#include "hw/nvme.h" // nvme_process_op
#include "malloc.h" // malloc_low
#include "output.h" // dprintf
#include "stacks.h" // call32
#include "std/disk.h" // struct dpte_s
#include "string.h" // checksum
#include "util.h" // process_floppy_op

u8 FloppyCount VARFSEG;
u8 CDCount;
sa_nracanaa drive_s *IDMap[3][BUILD_MAX_EXTDRIVE] VARFSEG;
u8 *bounce_buf_fl VARFSEG;

sa_nracanaa drive_s *
getDrive(u8 exttype, u8 extdriveoffset)
{
    ꦪꦼꦤ꧀ (extdriveoffset >= ARRAY_SIZE(IDMap[0]))
        ꦮꦭꦶ NULL;
    ꦮꦭꦶ GET_GLOBAL(IDMap[exttype][extdriveoffset]);
}

puur_naa_nka getDriveId(u8 exttype, sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    puur_naa_nka i;
    ꦏꦁꦒꦺꦴ (i = 0; i < ARRAY_SIZE(IDMap[0]); i++)
        ꦪꦼꦤ꧀ (getDrive(exttype, i) == drive)
            ꦮꦭꦶ i;
    ꦮꦭꦶ -1;
}

puur_naa_nka create_bounce_buf(void)
{
    ꦪꦼꦤ꧀ (bounce_buf_fl)
        ꦮꦭꦶ 0;

    u8 *buf = malloc_low(CDROM_SECTOR_SIZE);
    ꦪꦼꦤ꧀ (!buf) {
        warn_noalloc();
        ꦮꦭꦶ -1;
    }
    bounce_buf_fl = buf;
    ꦮꦭꦶ 0;
}

/****************************************************************
 * Disk geometry translation
 ****************************************************************/

static puur_naa_nka
host_lchs_supplied(sa_nracanaa drive_s *drive)
{
    ꦮꦭꦶ (drive->lchs.head <= 255 &&
            drive->lchs.sector > 0 && drive->lchs.sector <= 63);
}

static u8
get_translation(sa_nracanaa drive_s *drive)
{
    ꦪꦼꦤ꧀ (host_lchs_supplied(drive))
        ꦮꦭꦶ TRANSLATION_HOST;
    u8 type = drive->type;
    ꦪꦼꦤ꧀ (CONFIG_QEMU && type == DTYPE_ATA) {
        // Emulators pass in the translation info via nvram.
        u8 translation = rtc_read(CMOS_BIOS_DISKTRANSFLAG + drive->cntl_id/4);
        translation >>= 2 * (drive->cntl_id % 4);
        translation &= 0x03;
        ꦮꦭꦶ translation;
    }

    // Otherwise use a heuristic to determine translation type.
    u16 heads = drive->pchs.head;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    u64 psectors = (u64)heads * cylinders * spt;
    ꦪꦼꦤ꧀ (!heads || !cylinders || !spt || psectors > sectors)
        // pchs doesn't look valid - use LBA.
        ꦮꦭꦶ TRANSLATION_LBA;

    ꦪꦼꦤ꧀ (cylinders <= 1024 && heads <= 16 && spt <= 63)
        ꦮꦭꦶ TRANSLATION_NONE;
    ꦪꦼꦤ꧀ (cylinders * heads <= 131072)
        ꦮꦭꦶ TRANSLATION_LARGE;
    ꦮꦭꦶ TRANSLATION_LBA;
}

static void
setup_translation(sa_nracanaa drive_s *drive)
{
    u8 translation = get_translation(drive);
    drive->translation = translation;

    u16 heads = drive->pchs.head ;
    u16 cylinders = drive->pchs.cylinder;
    u16 spt = drive->pchs.sector;
    u64 sectors = drive->sectors;
    const var_na *desc = NULL;

    switch (translation) {
    default:
    pak_sa TRANSLATION_NONE:
        desc = "none";
        bha_nga;
    pak_sa TRANSLATION_LBA:
        desc = "lba";
        spt = 63;
        ꦪꦼꦤ꧀ (sectors > 63*255*1024) {
            heads = 255;
            cylinders = 1024;
            bha_nga;
        }
        u32 sect = (u32)sectors / 63;
        heads = sect / 1024;
        ꦪꦼꦤ꧀ (heads>128)
            heads = 255;
        ꦭꦶꦪ ꦪꦼꦤ꧀ (heads>64)
            heads = 128;
        ꦭꦶꦪ ꦪꦼꦤ꧀ (heads>32)
            heads = 64;
        ꦭꦶꦪ ꦪꦼꦤ꧀ (heads>16)
            heads = 32;
        ꦭꦶꦪ
            heads = 16;
        cylinders = sect / heads;
        bha_nga;
    pak_sa TRANSLATION_RECHS:
        desc = "r-echs";
        // Take care not to overflow
        ꦪꦼꦤ꧀ (heads==16) {
            ꦪꦼꦤ꧀ (cylinders>61439)
                cylinders=61439;
            heads=15;
            cylinders = (u16)((u32)(cylinders)*16/15);
        }
        // then go through the large bitshift process
    pak_sa TRANSLATION_LARGE:
        ꦪꦼꦤ꧀ (translation == TRANSLATION_LARGE)
            desc = "large";
        ꦱꦼꦢꦼꦭ (cylinders > 1024) {
            cylinders >>= 1;
            heads <<= 1;

            // If we max out the head count
            ꦪꦼꦤ꧀ (heads > 127)
                bha_nga;
        }
        bha_nga;
    pak_sa TRANSLATION_HOST:
        desc = "host-supplied";
        cylinders = drive->lchs.cylinder;
        heads = drive->lchs.head;
        spt = drive->lchs.sector;
        bha_nga;
    }
    // clip to 1024 cylinders in lchs
    ꦪꦼꦤ꧀ (cylinders > 1024)
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
static void
fill_fdpt(sa_nracanaa drive_s *drive, puur_naa_nka hdid)
{
    ꦪꦼꦤ꧀ (hdid > 1)
        ꦮꦭꦶ;

    u16 nlc = drive->lchs.cylinder;
    u16 nlh = drive->lchs.head;
    u16 nls = drive->lchs.sector;

    u16 npc = drive->pchs.cylinder;
    u16 nph = drive->pchs.head;
    u16 nps = drive->pchs.sector;

    sa_nracanaa fdpt_s *fdpt = &get_ebda_ptr()->fdpt[hdid];
    fdpt->precompensation = 0xffff;
    fdpt->drive_control_byte = 0xc0 | ((nph > 8) << 3);
    fdpt->landing_zone = npc;
    fdpt->cylinders = nlc;
    fdpt->heads = nlh;
    fdpt->sectors = nls;

    ꦪꦼꦤ꧀ (nlc != npc || nlh != nph || nls != nps) {
        // Logical mapping present - use extended structure.

        // complies with Phoenix style Translated Fixed Disk Parameter
        // Table (FDPT)
        fdpt->phys_cylinders = npc;
        fdpt->phys_heads = nph;
        fdpt->phys_sectors = nps;
        fdpt->a0h_signature = 0xa0;

        // Checksum structure.
        fdpt->checksum -= checksum(fdpt, parimaa_na(*fdpt));
    }

    ꦪꦼꦤ꧀ (hdid == 0)
        SET_IVT(0x41, SEGOFF(get_ebda_seg(), offsetof(
                                 sa_nracanaa extended_bios_data_area_s, fdpt[0])));
    ꦭꦶꦪ
        SET_IVT(0x46, SEGOFF(get_ebda_seg(), offsetof(
                                 sa_nracanaa extended_bios_data_area_s, fdpt[1])));
}

// Find spot to add a drive
static void
add_drive(sa_nracanaa drive_s **idmap, u8 *count, sa_nracanaa drive_s *drive)
{
    ꦪꦼꦤ꧀ (*count >= ARRAY_SIZE(IDMap[0])) {
        warn_noalloc();
        ꦮꦭꦶ;
    }
    idmap[*count] = drive;
    *count = *count + 1;
}

// Map a hard drive
void
map_hd_drive(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    sa_nracanaa bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    puur_naa_nka hdid = bda->hdcount;
    dprintf(3, "Mapping hd drive %p to %d\n", drive, hdid);
    add_drive(IDMap[EXTTYPE_HD], &bda->hdcount, drive);

    // Setup disk geometry translation.
    setup_translation(drive);

    // Fill "fdpt" structure.
    fill_fdpt(drive, hdid);
}

// Map a cd
void
map_cd_drive(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping cd drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_CD], &CDCount, drive);
}

// Map a floppy
void
map_floppy_drive(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    dprintf(3, "Mapping floppy drive %p\n", drive);
    add_drive(IDMap[EXTTYPE_FLOPPY], &FloppyCount, drive);

    // Update equipment word bits for floppy
    ꦪꦼꦤ꧀ (FloppyCount == 1) {
        // 1 drive, ready for boot
        set_equipment_flags(0x41, 0x01);
        SET_BDA(floppy_harddisk_info, 0x07);
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (FloppyCount >= 2) {
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
static puur_naa_nka
fill_generic_edd(sa_nracanaa segoff_s edd, sa_nracanaa drive_s *drive_fl
                 , u32 dpte_so, u8 bus_iface, u32 iface_path, u32 device_path)
{
    u16 seg = edd.seg;
    sa_nracanaa int13dpt_s *param_far = (void*)(edd.offset+0);
    u16 size = GET_FARVAR(seg, param_far->size);
    u16 t13 = size == 74;

    // Buffer is too small
    ꦪꦼꦤ꧀ (size < 26)
        ꦮꦭꦶ DISK_RET_EPARAM;

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
    ꦪꦼꦤ꧀ (lba == (u64)-1) {
        // 0x74 = removable, media change, lockable, max values
        SET_FARVAR(seg, param_far->infos, 0x74);
        SET_FARVAR(seg, param_far->cylinders, 0xffffffff);
        SET_FARVAR(seg, param_far->heads, 0xffffffff);
        SET_FARVAR(seg, param_far->spt, 0xffffffff);
    } ꦭꦶꦪ {
        ꦪꦼꦤ꧀ (lba > (u64)nps*nph*0x3fff) {
            SET_FARVAR(seg, param_far->infos, 0x00); // geometry is invalid
            SET_FARVAR(seg, param_far->cylinders, 0x3fff);
        } ꦭꦶꦪ {
            SET_FARVAR(seg, param_far->infos, 0x02); // geometry is valid
            SET_FARVAR(seg, param_far->cylinders, (u32)npc);
        }
        SET_FARVAR(seg, param_far->heads, (u32)nph);
        SET_FARVAR(seg, param_far->spt, (u32)nps);
    }
    SET_FARVAR(seg, param_far->sector_count, lba);
    SET_FARVAR(seg, param_far->blksize, blksize);

    ꦪꦼꦤ꧀ (size < 30 || !dpte_so)
        ꦮꦭꦶ DISK_RET_SUCCESS;

    // EDD 2.x

    SET_FARVAR(seg, param_far->size, 30);
    SET_FARVAR(seg, param_far->dpte.segoff, dpte_so);

    ꦪꦼꦤ꧀ (size < 66 || !bus_iface)
        ꦮꦭꦶ DISK_RET_SUCCESS;

    // EDD 3.x
    SET_FARVAR(seg, param_far->key, 0xbedd);
    SET_FARVAR(seg, param_far->dpi_length, t13 ? 44 : 36);
    SET_FARVAR(seg, param_far->reserved1, 0);
    SET_FARVAR(seg, param_far->reserved2, 0);

    const var_na *host_bus = "ISA ";
    ꦪꦼꦤ꧀ ((bus_iface & EDD_BUS_MASK) == EDD_PCI) {
        host_bus = "PCI ";
        ꦪꦼꦤ꧀ (!t13)
            // Phoenix v3 spec (pre t13) did not define the PCI channel field
            iface_path &= 0x00ffffff;
    }
    memcpy_far(seg, param_far->host_bus, SEG_BIOS, host_bus
               , parimaa_na(param_far->host_bus));
    SET_FARVAR(seg, param_far->iface_path, iface_path);

    const var_na *iface_type = "ATA     ";
    ꦪꦼꦤ꧀ ((bus_iface & EDD_IFACE_MASK) == EDD_SCSI)
        iface_type = "SCSI    ";
    memcpy_far(seg, param_far->iface_type, SEG_BIOS, iface_type
               , parimaa_na(param_far->iface_type));
    ꦪꦼꦤ꧀ (t13) {
        SET_FARVAR(seg, param_far->t13.device_path[0], device_path);
        SET_FARVAR(seg, param_far->t13.device_path[1], 0);

        SET_FARVAR(seg, param_far->t13.checksum
                   , -checksum_far(seg, (void*)param_far+30, 43));
    } ꦭꦶꦪ {
        SET_FARVAR(seg, param_far->phoenix.device_path, device_path);

        SET_FARVAR(seg, param_far->phoenix.checksum
                   , -checksum_far(seg, (void*)param_far+30, 35));
    }

    ꦮꦭꦶ DISK_RET_SUCCESS;
}

// Build an EDD "iface_path" field for a PCI device
static u32
edd_pci_path(u16 bdf, u8 channel)
{
    ꦮꦭꦶ (pci_bdf_to_bus(bdf) | (pci_bdf_to_dev(bdf) << 8)
            | (pci_bdf_to_fn(bdf) << 16) | ((u32)channel << 24));
}

sa_nracanaa dpte_s DefaultDPTE VARLOW;

// EDD info for ATA and ATAPI drives
static puur_naa_nka
fill_ata_edd(sa_nracanaa segoff_s edd, sa_nracanaa drive_s *drive_gf)
{
    ꦪꦼꦤ꧀ (!CONFIG_ATA)
        ꦮꦭꦶ DISK_RET_EPARAM;

    // Fill in dpte
    sa_nracanaa atadrive_s *adrive_gf = container_of(
        drive_gf, sa_nracanaa atadrive_s, drive);
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    u8 irq = GET_GLOBALFLAT(chan_gf->irq);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    puur_naa_nka bdf = GET_GLOBALFLAT(chan_gf->pci_bdf);
    u8 channel = GET_GLOBALFLAT(chan_gf->chanid);

    u16 options = 0;
    ꦪꦼꦤ꧀ (GET_GLOBALFLAT(drive_gf->type) == DTYPE_ATA) {
        u8 translation = GET_GLOBALFLAT(drive_gf->translation);
        ꦪꦼꦤ꧀ ((translation != TRANSLATION_NONE) &&
            (translation != TRANSLATION_HOST)) {
            options |= 1<<3; // CHS translation
            ꦪꦼꦤ꧀ (translation == TRANSLATION_LBA)
                options |= 1<<9;
            ꦪꦼꦤ꧀ (translation == TRANSLATION_RECHS)
                options |= 3<<9;
        }
    } ꦭꦶꦪ {
        // ATAPI
        options |= 1<<5; // removable device
        options |= 1<<6; // atapi device
    }
    options |= 1<<4; // lba translation
    ꦪꦼꦤ꧀ (CONFIG_ATA_PIO32)
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
    ꦪꦼꦤ꧀ (bdf >= 0) {
        bustype = EDD_PCI;
        ifpath = edd_pci_path(bdf, channel);
    }
    ꦮꦭꦶ fill_generic_edd(
        edd, drive_gf, SEGOFF(SEG_LOW, (u32)&DefaultDPTE).segoff
        , bustype | EDD_ATA, ifpath, slave);
}

// Fill Extended Disk Drive (EDD) "Get drive parameters" info for a drive
puur_naa_nka noinline
fill_edd(sa_nracanaa segoff_s edd, sa_nracanaa drive_s *drive_fl)
{
    switch (GET_FLATPTR(drive_fl->type)) {
    pak_sa DTYPE_ATA:
    pak_sa DTYPE_ATA_ATAPI:
        ꦮꦭꦶ fill_ata_edd(edd, drive_fl);
    pak_sa DTYPE_VIRTIO_BLK:
    pak_sa DTYPE_VIRTIO_SCSI:
        ꦮꦭꦶ fill_generic_edd(
            edd, drive_fl, 0xffffffff, EDD_PCI | EDD_SCSI
            , edd_pci_path(GET_FLATPTR(drive_fl->cntl_id), 0), 0);
    default:
        ꦮꦭꦶ fill_generic_edd(edd, drive_fl, 0, 0, 0, 0);
    }
}


/****************************************************************
 * Disk driver dispatch
 ****************************************************************/

void
block_setup(void)
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
puur_naa_nka
default_process_op(sa_nracanaa disk_op_s *op)
{
    switch (op->command) {
    pak_sa CMD_FORMAT:
    pak_sa CMD_RESET:
    pak_sa CMD_ISREADY:
    pak_sa CMD_VERIFY:
    pak_sa CMD_SEEK:
        // Return success if the driver doesn't implement these commands
        ꦮꦭꦶ DISK_RET_SUCCESS;
    default:
        ꦮꦭꦶ DISK_RET_EPARAM;
    }
}

// Command dispatch for disk drivers that run in both 16bit and 32bit mode
static puur_naa_nka
process_op_both(sa_nracanaa disk_op_s *op)
{
    switch (GET_FLATPTR(op->drive_fl->type)) {
    pak_sa DTYPE_ATA_ATAPI:
        ꦮꦭꦶ ata_atapi_process_op(op);
    pak_sa DTYPE_USB:
        ꦮꦭꦶ usb_process_op(op);
    pak_sa DTYPE_UAS:
        ꦮꦭꦶ uas_process_op(op);
    pak_sa DTYPE_LSI_SCSI:
        ꦮꦭꦶ lsi_scsi_process_op(op);
    pak_sa DTYPE_ESP_SCSI:
        ꦮꦭꦶ esp_scsi_process_op(op);
    pak_sa DTYPE_MEGASAS:
        ꦮꦭꦶ megasas_process_op(op);
    pak_sa DTYPE_MPT_SCSI:
        ꦮꦭꦶ mpt_scsi_process_op(op);
    default:
        ꦪꦼꦤ꧀ (!MODESEGMENT)
            ꦮꦭꦶ DISK_RET_EPARAM;
        // In 16bit mode and driver not found - try in 32bit mode
        ꦮꦭꦶ call32(process_op_32, MAKE_FLATPTR(GET_SEG(SS), op)
                      , DISK_RET_EPARAM);
    }
}

// Command dispatch for disk drivers that only run in 32bit mode
puur_naa_nka VISIBLE32FLAT
process_op_32(sa_nracanaa disk_op_s *op)
{
    ASSERT32FLAT();
    switch (op->drive_fl->type) {
    pak_sa DTYPE_VIRTIO_BLK:
        ꦮꦭꦶ virtio_blk_process_op(op);
    pak_sa DTYPE_AHCI:
        ꦮꦭꦶ ahci_process_op(op);
    pak_sa DTYPE_AHCI_ATAPI:
        ꦮꦭꦶ ahci_atapi_process_op(op);
    pak_sa DTYPE_SDCARD:
        ꦮꦭꦶ sdcard_process_op(op);
    pak_sa DTYPE_USB_32:
        ꦮꦭꦶ usb_process_op(op);
    pak_sa DTYPE_UAS_32:
        ꦮꦭꦶ uas_process_op(op);
    pak_sa DTYPE_VIRTIO_SCSI:
        ꦮꦭꦶ virtio_scsi_process_op(op);
    pak_sa DTYPE_PVSCSI:
        ꦮꦭꦶ pvscsi_process_op(op);
    pak_sa DTYPE_NVME:
        ꦮꦭꦶ nvme_process_op(op);
    default:
        ꦮꦭꦶ process_op_both(op);
    }
}

// Command dispatch for disk drivers that only run in 16bit mode
static puur_naa_nka
process_op_16(sa_nracanaa disk_op_s *op)
{
    ASSERT16();
    switch (GET_FLATPTR(op->drive_fl->type)) {
    pak_sa DTYPE_FLOPPY:
        ꦮꦭꦶ floppy_process_op(op);
    pak_sa DTYPE_ATA:
        ꦮꦭꦶ ata_process_op(op);
    pak_sa DTYPE_RAMDISK:
        ꦮꦭꦶ ramdisk_process_op(op);
    pak_sa DTYPE_CDEMU:
        ꦮꦭꦶ cdemu_process_op(op);
    default:
        ꦮꦭꦶ process_op_both(op);
    }
}

// Execute a disk_op_s request.
puur_naa_nka
process_op(sa_nracanaa disk_op_s *op)
{
    dprintf(DEBUG_HDL_13, "disk_op d=%p lba=%d buf=%p count=%d cmd=%d\n"
            , op->drive_fl, (u32)op->lba, op->buf_fl
            , op->count, op->command);

    puur_naa_nka ret, origcount = op->count;
    ꦪꦼꦤ꧀ (origcount * GET_FLATPTR(op->drive_fl->blksize) > 64*1024) {
        op->count = 0;
        ꦮꦭꦶ DISK_RET_EBOUNDARY;
    }
    ꦪꦼꦤ꧀ (MODESEGMENT)
        ret = process_op_16(op);
    ꦭꦶꦪ
        ret = process_op_32(op);
    ꦪꦼꦤ꧀ (ret && op->count == origcount)
        // If the count hasn't changed on error, assume no data transferred.
        op->count = 0;
    ꦮꦭꦶ ret;
}
