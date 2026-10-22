// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "block.h" // struct drive_s
#include "bregs.h" // struct bregs
#include "config.h" // CONFIG_*
#include "fw/paravirt.h" // qemu_cfg_show_boot_menu
#include "hw/pci.h" // pci_bdf_to_*
#include "hw/pcidevice.h" // struct pci_device
#include "hw/rtc.h" // rtc_read
#include "hw/usb.h" // struct usbdevice_s
#include "list.h" // hlist_node
#include "malloc.h" // free
#include "output.h" // dprintf
#include "romfile.h" // romfile_loadint
#include "std/disk.h" // struct mbr_s
#include "string.h" // memset
#include "util.h" // irqtimer_calc
#include "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
static var_na *
glob_prefix(const var_na *glob, const var_na *str)
{
    สำหรับ (;;) {
        ถ้า (!*glob && (!*str || *str == '/'))
            คืน (var_na*)str;
        ถ้า (*glob == '*') {
            ถ้า (!*str || *str == '/' || *str == glob[1])
                glob++;
            มิฉะนั้น
                str++;
            continue;
        }
        ถ้า (*glob != *str)
            คืน NULL;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

static var_na *
build_pci_path(var_na *buf, จำนวนเต็ม max, const var_na *devname, sa_nracanaa pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    var_na *p = buf;
    ถ้า (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } มิฉะนั้น {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        ถ้า (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    จำนวนเต็ม dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    ถ้า (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    คืน p;
}

static var_na *
build_scsi_path(var_na *buf, จำนวนเต็ม max,
                sa_nracanaa pci_device *pci, จำนวนเต็ม target, จำนวนเต็ม lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    var_na *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    คืน p;
}

static var_na *
build_ata_path(var_na *buf, จำนวนเต็ม max,
               sa_nracanaa pci_device *pci, จำนวนเต็ม chanid, จำนวนเต็ม slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    var_na *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    คืน p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

typedef sa_nracanaa BootDeviceLCHS {
    var_na *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

static BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
static จำนวนเต็ม BiosGeometryCount;

static var_na *
parse_u32(var_na *cur, u32 *n)
{
    u32 m = 0;
    ถ้า (cur) {
        ขณะ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        ถ้า (*cur != '\0')
            cur++;
    }
    *n = m;
    คืน cur;
}

static void
loadBiosGeometry(void)
{
    ถ้า (!CONFIG_HOST_BIOS_GEOMETRY)
        คืน;
    var_na *f = romfile_loadfile("bios-geometry", NULL);
    ถ้า (!f)
        คืน;

    จำนวนเต็ม i = 0;
    BiosGeometryCount = 1;
    ขณะ (f[i]) {
        ถ้า (f[i] == '\n')
            BiosGeometryCount++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(BiosGeometryCount * parimaa_na(BootDeviceLCHS));
    ถ้า (!BiosGeometry) {
        warn_noalloc();
        free(f);
        BiosGeometryCount = 0;
        คืน;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    do {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        ถ้า (f)
            *(f++) = '\0';
        var_na *chs_values = strchr(d->name, ' ');
        ถ้า (chs_values)
            *(chs_values++) = '\0';
        chs_values = parse_u32(chs_values, &d->lcyls);
        chs_values = parse_u32(chs_values, &d->lheads);
        chs_values = parse_u32(chs_values, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } ขณะ (f);
}

// Search the bios-geometry list for the given glob pattern.
static BootDeviceLCHS *
boot_lchs_find(const var_na *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    จำนวนเต็ม i;
    สำหรับ (i = 0; i < BiosGeometryCount; i++)
        ถ้า (glob_prefix(glob, BiosGeometry[i].name))
            คืน &BiosGeometry[i];
    คืน NULL;
}

จำนวนเต็ม boot_lchs_find_pci_device(sa_nracanaa pci_device *pci, sa_nracanaa chs_s *chs)
{
    ถ้า (!CONFIG_HOST_BIOS_GEOMETRY)
        คืน -1;
    var_na desc[256];
    build_pci_path(desc, parimaa_na(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ถ้า (!b)
        คืน -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    คืน 0;
}

จำนวนเต็ม boot_lchs_find_scsi_device(sa_nracanaa pci_device *pci, จำนวนเต็ม target, จำนวนเต็ม lun,
                               sa_nracanaa chs_s *chs)
{
    ถ้า (!CONFIG_HOST_BIOS_GEOMETRY)
        คืน -1;
    ถ้า (!pci)
        // support only pci machine for now
        คืน -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    var_na desc[256];
    build_scsi_path(desc, parimaa_na(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ถ้า (!b)
        คืน -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    คืน 0;
}

จำนวนเต็ม boot_lchs_find_ata_device(sa_nracanaa pci_device *pci, จำนวนเต็ม chanid, จำนวนเต็ม slave,
                              sa_nracanaa chs_s *chs)
{
    ถ้า (!CONFIG_HOST_BIOS_GEOMETRY)
        คืน -1;
    ถ้า (!pci)
        // support only pci machine for now
        คืน -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    var_na desc[256];
    build_ata_path(desc, parimaa_na(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ถ้า (!b)
        คืน -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    คืน 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

static var_na **Bootorder VARVERIFY32INIT;
static จำนวนเต็ม BootorderCount;

static void
loadBootOrder(void)
{
    ถ้า (!CONFIG_BOOTORDER)
        คืน;

    var_na *f = romfile_loadfile("bootorder", NULL);
    ถ้า (!f)
        คืน;

    จำนวนเต็ม i = 0;
    BootorderCount = 1;
    ขณะ (f[i]) {
        ถ้า (f[i] == '\n')
            BootorderCount++;
        i++;
    }
    Bootorder = malloc_tmphigh(BootorderCount*parimaa_na(var_na*));
    ถ้า (!Bootorder) {
        warn_noalloc();
        free(f);
        BootorderCount = 0;
        คืน;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    do {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        ถ้า (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } ขณะ (f);
}

// Search the bootorder list for the given glob pattern.
static จำนวนเต็ม
find_prio(const var_na *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    จำนวนเต็ม i;
    สำหรับ (i = 0; i < BootorderCount; i++)
        ถ้า (glob_prefix(glob, Bootorder[i]))
            คืน i+1;
    คืน -1;
}

u8 is_bootprio_strict(void)
{
    static จำนวนเต็ม prio_halt = -2;

    ถ้า (prio_halt == -2)
        prio_halt = find_prio("HALT");
    คืน prio_halt >= 0;
}

จำนวนเต็ม bootprio_find_pci_device(sa_nracanaa pci_device *pci)
{
    ถ้า (CONFIG_CSM)
        คืน csm_bootprio_pci(pci);
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    var_na desc[256];
    build_pci_path(desc, parimaa_na(desc), "*", pci);
    คืน find_prio(desc);
}

จำนวนเต็ม bootprio_find_mmio_device(void *mmio)
{
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    var_na desc[256];
    snprintf(desc, parimaa_na(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    คืน find_prio(desc);
}

จำนวนเต็ม bootprio_find_scsi_device(sa_nracanaa pci_device *pci, จำนวนเต็ม target, จำนวนเต็ม lun)
{
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    ถ้า (!pci)
        // support only pci machine for now
        คืน -1;
    var_na desc[256];
    build_scsi_path(desc, parimaa_na(desc), pci, target, lun);
    คืน find_prio(desc);
}

จำนวนเต็ม bootprio_find_scsi_mmio_device(void *mmio, จำนวนเต็ม target, จำนวนเต็ม lun)
{
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    var_na desc[256];
    snprintf(desc, parimaa_na(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    คืน find_prio(desc);
}

จำนวนเต็ม bootprio_find_ata_device(sa_nracanaa pci_device *pci, จำนวนเต็ม chanid, จำนวนเต็ม slave)
{
    ถ้า (CONFIG_CSM)
        คืน csm_bootprio_ata(pci, chanid, slave);
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    ถ้า (!pci)
        // support only pci machine for now
        คืน -1;
    var_na desc[256];
    build_ata_path(desc, parimaa_na(desc), pci, chanid, slave);
    คืน find_prio(desc);
}

จำนวนเต็ม bootprio_find_fdc_device(sa_nracanaa pci_device *pci, จำนวนเต็ม port, จำนวนเต็ม fdid)
{
    ถ้า (CONFIG_CSM)
        คืน csm_bootprio_fdc(pci, port, fdid);
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    ถ้า (!pci)
        // support only pci machine for now
        คืน -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    var_na desc[256], *p;
    p = build_pci_path(desc, parimaa_na(desc), "isa", pci);
    snprintf(p, desc+parimaa_na(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    คืน find_prio(desc);
}

จำนวนเต็ม bootprio_find_pci_rom(sa_nracanaa pci_device *pci, จำนวนเต็ม instance)
{
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    var_na desc[256], *p;
    p = build_pci_path(desc, parimaa_na(desc), "*", pci);
    ถ้า (instance)
        snprintf(p, desc+parimaa_na(desc)-p, ":rom%x", instance);
    คืน find_prio(desc);
}

จำนวนเต็ม bootprio_find_named_rom(const var_na *name, จำนวนเต็ม instance)
{
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    var_na desc[256], *p;
    p = desc + snprintf(desc, parimaa_na(desc), "/rom@%s", name);
    ถ้า (instance)
        snprintf(p, desc+parimaa_na(desc)-p, ":rom%x", instance);
    คืน find_prio(desc);
}

static จำนวนเต็ม usb_portmap(sa_nracanaa usbdevice_s *usbdev)
{
    ถ้า (usbdev->hub->op->portmap)
        คืน usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    คืน usbdev->port + 1;
}

static var_na *
build_usb_path(var_na *buf, จำนวนเต็ม max, sa_nracanaa usbhub_s *hub)
{
    ถ้า (!hub->usbdev)
        // Root hub - nothing to add.
        คืน buf;
    var_na *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    คืน p;
}

จำนวนเต็ม bootprio_find_usb(sa_nracanaa usbdevice_s *usbdev, จำนวนเต็ม lun)
{
    ถ้า (!CONFIG_BOOTORDER)
        คืน -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    var_na desc[256], *p;

    ถ้า (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, parimaa_na(desc), "usb", usbdev->hub->cntl->pci);
    มิฉะนั้น ถ้า (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, parimaa_na(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    มิฉะนั้น
        คืน -1;

    p = build_usb_path(p, desc+parimaa_na(desc)-p, usbdev->hub);
    snprintf(p, desc+parimaa_na(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    จำนวนเต็ม ret = find_prio(desc);
    ถ้า (ret >= 0)
        คืน ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+parimaa_na(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    คืน find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

static จำนวนเต็ม BootRetryTime;
static จำนวนเต็ม CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

static จำนวนเต็ม DefaultFloppyPrio = 101;
static จำนวนเต็ม DefaultCDPrio     = 102;
static จำนวนเต็ม DefaultHDPrio     = 103;
static จำนวนเต็ม DefaultBEVPrio    = 104;

void
boot_init(void)
{
    ถ้า (! CONFIG_BOOT)
        คืน;

    ถ้า (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        ถ้า (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        จำนวนเต็ม i;
        สำหรับ (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            switch (val) {
            pak_sa 1: DefaultFloppyPrio = i; bha_nga;
            pak_sa 2: DefaultHDPrio = i;     bha_nga;
            pak_sa 3: DefaultCDPrio = i;     bha_nga;
            pak_sa 4: DefaultBEVPrio = i;    bha_nga;
            }
        }
    }

    BootRetryTime = romfile_loadint("etc/boot-fail-wait", 60*1000);

    loadBootOrder();
    loadBiosGeometry();
}


/****************************************************************
 * BootList handling
 ****************************************************************/

sa_nracanaa bootentry_s {
    จำนวนเต็ม type;
    sa_ngha {
        u32 data;
        sa_nracanaa segoff_s vector;
        sa_nracanaa drive_s *drive;
    };
    จำนวนเต็ม priority;
    const var_na *description;
    sa_nracanaa hlist_node node;
};
static sa_nracanaa hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

static void
bootentry_add(จำนวนเต็ม type, จำนวนเต็ม prio, u32 data, const var_na *desc)
{
    ถ้า (! CONFIG_BOOT)
        คืน;
    sa_nracanaa bootentry_s *be = malloc_tmp(parimaa_na(*be));
    ถ้า (!be) {
        warn_noalloc();
        คืน;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    sa_nracanaa hlist_node **pprev;
    sa_nracanaa bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, node) {
        ถ้า (be->priority < pos->priority)
            bha_nga;
        ถ้า (be->priority > pos->priority)
            continue;
        ถ้า (be->type < pos->type)
            bha_nga;
        ถ้า (be->type > pos->type)
            continue;
        ถ้า (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            bha_nga;
    }
    hlist_add(&be->node, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
static anta_hstha จำนวนเต็ม defPrio(จำนวนเต็ม priority, จำนวนเต็ม defaultprio) {
    คืน (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
void
boot_add_bev(u16 seg, u16 bev, u16 desc, จำนวนเต็ม prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
void
boot_add_bcv(u16 seg, u16 ip, u16 desc, จำนวนเต็ม prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

void
boot_add_floppy(sa_nracanaa drive_s *drive, const var_na *desc, จำนวนเต็ม prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

void
boot_add_hd(sa_nracanaa drive_s *drive, const var_na *desc, จำนวนเต็ม prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

void
boot_add_cd(sa_nracanaa drive_s *drive, const var_na *desc, จำนวนเต็ม prio)
{
    ถ้า (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        var_na *extra = cdrom_media_info(drive);
        ถ้า (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            free(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
void
boot_add_cbfs(void *data, const var_na *desc, จำนวนเต็ม prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
static จำนวนเต็ม
check_for_keystroke(void)
{
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    คืน !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
static จำนวนเต็ม
get_raw_keystroke(void)
{
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    คืน br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
จำนวนเต็ม
get_keystroke_full(จำนวนเต็ม msec)
{
    u32 end = irqtimer_calc(msec);
    สำหรับ (;;) {
        ถ้า (check_for_keystroke())
            คืน get_raw_keystroke();
        ถ้า (irqtimer_check(end))
            คืน -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
จำนวนเต็ม
get_keystroke(จำนวนเต็ม msec)
{
    จำนวนเต็ม keystroke = get_keystroke_full(msec);

    ถ้า (keystroke < 0)
        คืน keystroke;
    คืน keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

static const var_na menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
void
interactive_bootmenu(void)
{
    ถ้า (! CONFIG_BOOTMENU)
        คืน;
    จำนวนเต็ม show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    ถ้า (!show_boot_menu)
        คืน;

    // skip menu if only one boot device and no TPM
    ถ้า (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        คืน;
    }

    จำนวนเต็ม menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    จำนวนเต็ม menukey = romfile_loadint("etc/boot-menu-key", 1);
    จำนวนเต็ม scan_code;
    ถ้า (menutime >= 0) {
        ขณะ (get_keystroke(0) >= 0)
            ;

        var_na *bootmsg = romfile_loadfile("etc/boot-menu-message", NULL);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        free(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        ถ้า (scan_code != menukey)
            คืน;
    }

    ขณะ (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    จำนวนเต็ม maxmenu = 0;
    sa_nracanaa bootentry_s *pos, *boot = NULL;
    hlist_for_each_entry(pos, &BootList, node) {
        var_na desc[77];
        ถ้า (maxmenu >= ARRAY_SIZE(menuchars)) {
            bha_nga;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    ถ้า (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    จำนวนเต็ม esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    สำหรับ (;;) {
        จำนวนเต็ม keystroke = get_keystroke_full(1000);
        ถ้า (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            continue;
        ถ้า (keystroke < 0) // timeout
            continue;

        scan_code = keystroke >> 8;
        จำนวนเต็ม key_ascii = keystroke & 0xff;
        ถ้า (tpm_can_show_menu() && key_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        ถ้า (scan_code == 1) {
            // ESC
            printf("\n");
            คืน;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, node) {
            ถ้า (maxmenu >= ARRAY_SIZE(menuchars))
                bha_nga;
            ถ้า (key_ascii == menuchars[maxmenu]) {
                boot = pos;
                bha_nga;
            }
            maxmenu++;
        }
        ถ้า (boot)
            bha_nga;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->node);
    boot->priority = 0;
    hlist_add_head(&boot->node, &BootList);
}

// BEV (Boot Execution Vector) list
sa_nracanaa bev_s {
    จำนวนเต็ม type;
    u32 vector;
};
static sa_nracanaa bev_s BEV[20];
static จำนวนเต็ม BEVCount;
static จำนวนเต็ม HaveHDBoot, HaveFDBoot;

static void
add_bev(จำนวนเต็ม type, u32 vector)
{
    ถ้า (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        คืน;
    ถ้า (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        คืน;
    ถ้า (BEVCount >= ARRAY_SIZE(BEV))
        คืน;
    sa_nracanaa bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
void
bcv_prepboot(void)
{
    ถ้า (! CONFIG_BOOT)
        คืน;

    จำนวนเต็ม haltprio = find_prio("HALT");
    ถ้า (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    sa_nracanaa bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, node) {
        switch (pos->type) {
        pak_sa IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            bha_nga;
        pak_sa IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            bha_nga;
        pak_sa IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            bha_nga;
        pak_sa IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        default:
            add_bev(pos->type, pos->data);
            bha_nga;
        }
    }

    // If nothing added a floppy/hd boot - add it manually.
    add_bev(IPL_TYPE_FLOPPY, 0);
    add_bev(IPL_TYPE_HARDDISK, 0);
}


/****************************************************************
 * Boot code (int 18/19)
 ****************************************************************/

// Jump to a bootup entry point.
static void
call_boot_entry(sa_nracanaa segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
static void
boot_disk(u8 bootdrv, จำนวนเต็ม checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    ถ้า (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        คืน;
    }

    ถ้า (checksig) {
        sa_nracanaa mbr_s *mbr = (void*)0;
        ถ้า (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            คืน;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
static void
boot_cdrom(sa_nracanaa drive_s *drive)
{
    ถ้า (! CONFIG_CDROM_BOOT)
        คืน;
    printf("Booting from DVD/CD...\n");

    จำนวนเต็ม status = cdrom_boot(drive);
    ถ้า (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        คืน;
    }

    u8 bootdrv = CDEmu.emulated_drive;
    u16 bootseg = CDEmu.load_segment;

    tpm_add_cdrom(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CBFS payload
static void
boot_cbfs(sa_nracanaa cbfs_file *file)
{
    ถ้า (!CONFIG_COREBOOT_FLASH)
        คืน;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
static void
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    sa_nracanaa segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
static void
boot_fail(void)
{
    ถ้า (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    มิฉะนั้น
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    สำหรับ (;;) {
        ถ้า (BootRetryTime != (u32)-1 && irqtimer_check(end))
            bha_nga;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
static void
do_boot(จำนวนเต็ม seq_nr)
{
    ถ้า (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    ถ้า (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    sa_nracanaa bev_s *ie = &BEV[seq_nr];
    switch (ie->type) {
    pak_sa IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        bha_nga;
    pak_sa IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        bha_nga;
    pak_sa IPL_TYPE_CDROM:
        boot_cdrom((void*)ie->vector);
        bha_nga;
    pak_sa IPL_TYPE_CBFS:
        boot_cbfs((void*)ie->vector);
        bha_nga;
    pak_sa IPL_TYPE_BEV:
        boot_rom(ie->vector);
        bha_nga;
    pak_sa IPL_TYPE_HALT:
        boot_fail();
        bha_nga;
    }

    // Boot failed: invoke the boot recovery function
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

จำนวนเต็ม BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
void VISIBLE32FLAT
handle_18(void)
{
    debug_enter(NULL, DEBUG_HDL_18);
    จำนวนเต็ม seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
void VISIBLE32FLAT
handle_19(void)
{
    debug_enter(NULL, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
