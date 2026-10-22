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
    အတွက် (;;) {
        အကယ်၍ (!*glob && (!*str || *str == '/'))
            ပြန်ပေး (var_na*)str;
        အကယ်၍ (*glob == '*') {
            အကယ်၍ (!*str || *str == '/' || *str == glob[1])
                glob++;
            မဟုတ်ပါက
                str++;
            continue;
        }
        အကယ်၍ (*glob != *str)
            ပြန်ပေး NULL;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

static var_na *
build_pci_path(var_na *buf, ကိန်းပြည့် max, const var_na *devname, sa_nracanaa pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    var_na *p = buf;
    အကယ်၍ (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } မဟုတ်ပါက {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        အကယ်၍ (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    ကိန်းပြည့် dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    အကယ်၍ (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    ပြန်ပေး p;
}

static var_na *
build_scsi_path(var_na *buf, ကိန်းပြည့် max,
                sa_nracanaa pci_device *pci, ကိန်းပြည့် target, ကိန်းပြည့် lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    var_na *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    ပြန်ပေး p;
}

static var_na *
build_ata_path(var_na *buf, ကိန်းပြည့် max,
               sa_nracanaa pci_device *pci, ကိန်းပြည့် chanid, ကိန်းပြည့် slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    var_na *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    ပြန်ပေး p;
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
static ကိန်းပြည့် BiosGeometryCount;

static var_na *
parse_u32(var_na *cur, u32 *n)
{
    u32 m = 0;
    အကယ်၍ (cur) {
        နေစဉ် ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        အကယ်၍ (*cur != '\0')
            cur++;
    }
    *n = m;
    ပြန်ပေး cur;
}

static void
loadBiosGeometry(void)
{
    အကယ်၍ (!CONFIG_HOST_BIOS_GEOMETRY)
        ပြန်ပေး;
    var_na *f = romfile_loadfile("bios-geometry", NULL);
    အကယ်၍ (!f)
        ပြန်ပေး;

    ကိန်းပြည့် i = 0;
    BiosGeometryCount = 1;
    နေစဉ် (f[i]) {
        အကယ်၍ (f[i] == '\n')
            BiosGeometryCount++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(BiosGeometryCount * parimaa_na(BootDeviceLCHS));
    အကယ်၍ (!BiosGeometry) {
        warn_noalloc();
        free(f);
        BiosGeometryCount = 0;
        ပြန်ပေး;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    do {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        အကယ်၍ (f)
            *(f++) = '\0';
        var_na *chs_values = strchr(d->name, ' ');
        အကယ်၍ (chs_values)
            *(chs_values++) = '\0';
        chs_values = parse_u32(chs_values, &d->lcyls);
        chs_values = parse_u32(chs_values, &d->lheads);
        chs_values = parse_u32(chs_values, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } နေစဉ် (f);
}

// Search the bios-geometry list for the given glob pattern.
static BootDeviceLCHS *
boot_lchs_find(const var_na *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    ကိန်းပြည့် i;
    အတွက် (i = 0; i < BiosGeometryCount; i++)
        အကယ်၍ (glob_prefix(glob, BiosGeometry[i].name))
            ပြန်ပေး &BiosGeometry[i];
    ပြန်ပေး NULL;
}

ကိန်းပြည့် boot_lchs_find_pci_device(sa_nracanaa pci_device *pci, sa_nracanaa chs_s *chs)
{
    အကယ်၍ (!CONFIG_HOST_BIOS_GEOMETRY)
        ပြန်ပေး -1;
    var_na desc[256];
    build_pci_path(desc, parimaa_na(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    အကယ်၍ (!b)
        ပြန်ပေး -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ပြန်ပေး 0;
}

ကိန်းပြည့် boot_lchs_find_scsi_device(sa_nracanaa pci_device *pci, ကိန်းပြည့် target, ကိန်းပြည့် lun,
                               sa_nracanaa chs_s *chs)
{
    အကယ်၍ (!CONFIG_HOST_BIOS_GEOMETRY)
        ပြန်ပေး -1;
    အကယ်၍ (!pci)
        // support only pci machine for now
        ပြန်ပေး -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    var_na desc[256];
    build_scsi_path(desc, parimaa_na(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    အကယ်၍ (!b)
        ပြန်ပေး -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ပြန်ပေး 0;
}

ကိန်းပြည့် boot_lchs_find_ata_device(sa_nracanaa pci_device *pci, ကိန်းပြည့် chanid, ကိန်းပြည့် slave,
                              sa_nracanaa chs_s *chs)
{
    အကယ်၍ (!CONFIG_HOST_BIOS_GEOMETRY)
        ပြန်ပေး -1;
    အကယ်၍ (!pci)
        // support only pci machine for now
        ပြန်ပေး -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    var_na desc[256];
    build_ata_path(desc, parimaa_na(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    အကယ်၍ (!b)
        ပြန်ပေး -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ပြန်ပေး 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

static var_na **Bootorder VARVERIFY32INIT;
static ကိန်းပြည့် BootorderCount;

static void
loadBootOrder(void)
{
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး;

    var_na *f = romfile_loadfile("bootorder", NULL);
    အကယ်၍ (!f)
        ပြန်ပေး;

    ကိန်းပြည့် i = 0;
    BootorderCount = 1;
    နေစဉ် (f[i]) {
        အကယ်၍ (f[i] == '\n')
            BootorderCount++;
        i++;
    }
    Bootorder = malloc_tmphigh(BootorderCount*parimaa_na(var_na*));
    အကယ်၍ (!Bootorder) {
        warn_noalloc();
        free(f);
        BootorderCount = 0;
        ပြန်ပေး;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    do {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        အကယ်၍ (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } နေစဉ် (f);
}

// Search the bootorder list for the given glob pattern.
static ကိန်းပြည့်
find_prio(const var_na *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    ကိန်းပြည့် i;
    အတွက် (i = 0; i < BootorderCount; i++)
        အကယ်၍ (glob_prefix(glob, Bootorder[i]))
            ပြန်ပေး i+1;
    ပြန်ပေး -1;
}

u8 is_bootprio_strict(void)
{
    static ကိန်းပြည့် prio_halt = -2;

    အကယ်၍ (prio_halt == -2)
        prio_halt = find_prio("HALT");
    ပြန်ပေး prio_halt >= 0;
}

ကိန်းပြည့် bootprio_find_pci_device(sa_nracanaa pci_device *pci)
{
    အကယ်၍ (CONFIG_CSM)
        ပြန်ပေး csm_bootprio_pci(pci);
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    var_na desc[256];
    build_pci_path(desc, parimaa_na(desc), "*", pci);
    ပြန်ပေး find_prio(desc);
}

ကိန်းပြည့် bootprio_find_mmio_device(void *mmio)
{
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    var_na desc[256];
    snprintf(desc, parimaa_na(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    ပြန်ပေး find_prio(desc);
}

ကိန်းပြည့် bootprio_find_scsi_device(sa_nracanaa pci_device *pci, ကိန်းပြည့် target, ကိန်းပြည့် lun)
{
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    အကယ်၍ (!pci)
        // support only pci machine for now
        ပြန်ပေး -1;
    var_na desc[256];
    build_scsi_path(desc, parimaa_na(desc), pci, target, lun);
    ပြန်ပေး find_prio(desc);
}

ကိန်းပြည့် bootprio_find_scsi_mmio_device(void *mmio, ကိန်းပြည့် target, ကိန်းပြည့် lun)
{
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    var_na desc[256];
    snprintf(desc, parimaa_na(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    ပြန်ပေး find_prio(desc);
}

ကိန်းပြည့် bootprio_find_ata_device(sa_nracanaa pci_device *pci, ကိန်းပြည့် chanid, ကိန်းပြည့် slave)
{
    အကယ်၍ (CONFIG_CSM)
        ပြန်ပေး csm_bootprio_ata(pci, chanid, slave);
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    အကယ်၍ (!pci)
        // support only pci machine for now
        ပြန်ပေး -1;
    var_na desc[256];
    build_ata_path(desc, parimaa_na(desc), pci, chanid, slave);
    ပြန်ပေး find_prio(desc);
}

ကိန်းပြည့် bootprio_find_fdc_device(sa_nracanaa pci_device *pci, ကိန်းပြည့် port, ကိန်းပြည့် fdid)
{
    အကယ်၍ (CONFIG_CSM)
        ပြန်ပေး csm_bootprio_fdc(pci, port, fdid);
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    အကယ်၍ (!pci)
        // support only pci machine for now
        ပြန်ပေး -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    var_na desc[256], *p;
    p = build_pci_path(desc, parimaa_na(desc), "isa", pci);
    snprintf(p, desc+parimaa_na(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    ပြန်ပေး find_prio(desc);
}

ကိန်းပြည့် bootprio_find_pci_rom(sa_nracanaa pci_device *pci, ကိန်းပြည့် instance)
{
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    var_na desc[256], *p;
    p = build_pci_path(desc, parimaa_na(desc), "*", pci);
    အကယ်၍ (instance)
        snprintf(p, desc+parimaa_na(desc)-p, ":rom%x", instance);
    ပြန်ပေး find_prio(desc);
}

ကိန်းပြည့် bootprio_find_named_rom(const var_na *name, ကိန်းပြည့် instance)
{
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    var_na desc[256], *p;
    p = desc + snprintf(desc, parimaa_na(desc), "/rom@%s", name);
    အကယ်၍ (instance)
        snprintf(p, desc+parimaa_na(desc)-p, ":rom%x", instance);
    ပြန်ပေး find_prio(desc);
}

static ကိန်းပြည့် usb_portmap(sa_nracanaa usbdevice_s *usbdev)
{
    အကယ်၍ (usbdev->hub->op->portmap)
        ပြန်ပေး usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    ပြန်ပေး usbdev->port + 1;
}

static var_na *
build_usb_path(var_na *buf, ကိန်းပြည့် max, sa_nracanaa usbhub_s *hub)
{
    အကယ်၍ (!hub->usbdev)
        // Root hub - nothing to add.
        ပြန်ပေး buf;
    var_na *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    ပြန်ပေး p;
}

ကိန်းပြည့် bootprio_find_usb(sa_nracanaa usbdevice_s *usbdev, ကိန်းပြည့် lun)
{
    အကယ်၍ (!CONFIG_BOOTORDER)
        ပြန်ပေး -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    var_na desc[256], *p;

    အကယ်၍ (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, parimaa_na(desc), "usb", usbdev->hub->cntl->pci);
    မဟုတ်ပါက အကယ်၍ (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, parimaa_na(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    မဟုတ်ပါက
        ပြန်ပေး -1;

    p = build_usb_path(p, desc+parimaa_na(desc)-p, usbdev->hub);
    snprintf(p, desc+parimaa_na(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    ကိန်းပြည့် ret = find_prio(desc);
    အကယ်၍ (ret >= 0)
        ပြန်ပေး ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+parimaa_na(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    ပြန်ပေး find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

static ကိန်းပြည့် BootRetryTime;
static ကိန်းပြည့် CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

static ကိန်းပြည့် DefaultFloppyPrio = 101;
static ကိန်းပြည့် DefaultCDPrio     = 102;
static ကိန်းပြည့် DefaultHDPrio     = 103;
static ကိန်းပြည့် DefaultBEVPrio    = 104;

void
boot_init(void)
{
    အကယ်၍ (! CONFIG_BOOT)
        ပြန်ပေး;

    အကယ်၍ (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        အကယ်၍ (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        ကိန်းပြည့် i;
        အတွက် (i=101; i<104; i++) {
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
    ကိန်းပြည့် type;
    sa_ngha {
        u32 data;
        sa_nracanaa segoff_s vector;
        sa_nracanaa drive_s *drive;
    };
    ကိန်းပြည့် priority;
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
bootentry_add(ကိန်းပြည့် type, ကိန်းပြည့် prio, u32 data, const var_na *desc)
{
    အကယ်၍ (! CONFIG_BOOT)
        ပြန်ပေး;
    sa_nracanaa bootentry_s *be = malloc_tmp(parimaa_na(*be));
    အကယ်၍ (!be) {
        warn_noalloc();
        ပြန်ပေး;
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
        အကယ်၍ (be->priority < pos->priority)
            bha_nga;
        အကယ်၍ (be->priority > pos->priority)
            continue;
        အကယ်၍ (be->type < pos->type)
            bha_nga;
        အကယ်၍ (be->type > pos->type)
            continue;
        အကယ်၍ (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            bha_nga;
    }
    hlist_add(&be->node, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
static anta_hstha ကိန်းပြည့် defPrio(ကိန်းပြည့် priority, ကိန်းပြည့် defaultprio) {
    ပြန်ပေး (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
void
boot_add_bev(u16 seg, u16 bev, u16 desc, ကိန်းပြည့် prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
void
boot_add_bcv(u16 seg, u16 ip, u16 desc, ကိန်းပြည့် prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

void
boot_add_floppy(sa_nracanaa drive_s *drive, const var_na *desc, ကိန်းပြည့် prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

void
boot_add_hd(sa_nracanaa drive_s *drive, const var_na *desc, ကိန်းပြည့် prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

void
boot_add_cd(sa_nracanaa drive_s *drive, const var_na *desc, ကိန်းပြည့် prio)
{
    အကယ်၍ (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        var_na *extra = cdrom_media_info(drive);
        အကယ်၍ (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            free(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
void
boot_add_cbfs(void *data, const var_na *desc, ကိန်းပြည့် prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
static ကိန်းပြည့်
check_for_keystroke(void)
{
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    ပြန်ပေး !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
static ကိန်းပြည့်
get_raw_keystroke(void)
{
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    ပြန်ပေး br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
ကိန်းပြည့်
get_keystroke_full(ကိန်းပြည့် msec)
{
    u32 end = irqtimer_calc(msec);
    အတွက် (;;) {
        အကယ်၍ (check_for_keystroke())
            ပြန်ပေး get_raw_keystroke();
        အကယ်၍ (irqtimer_check(end))
            ပြန်ပေး -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
ကိန်းပြည့်
get_keystroke(ကိန်းပြည့် msec)
{
    ကိန်းပြည့် keystroke = get_keystroke_full(msec);

    အကယ်၍ (keystroke < 0)
        ပြန်ပေး keystroke;
    ပြန်ပေး keystroke >> 8;
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
    အကယ်၍ (! CONFIG_BOOTMENU)
        ပြန်ပေး;
    ကိန်းပြည့် show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    အကယ်၍ (!show_boot_menu)
        ပြန်ပေး;

    // skip menu if only one boot device and no TPM
    အကယ်၍ (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        ပြန်ပေး;
    }

    ကိန်းပြည့် menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    ကိန်းပြည့် menukey = romfile_loadint("etc/boot-menu-key", 1);
    ကိန်းပြည့် scan_code;
    အကယ်၍ (menutime >= 0) {
        နေစဉ် (get_keystroke(0) >= 0)
            ;

        var_na *bootmsg = romfile_loadfile("etc/boot-menu-message", NULL);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        free(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        အကယ်၍ (scan_code != menukey)
            ပြန်ပေး;
    }

    နေစဉ် (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    ကိန်းပြည့် maxmenu = 0;
    sa_nracanaa bootentry_s *pos, *boot = NULL;
    hlist_for_each_entry(pos, &BootList, node) {
        var_na desc[77];
        အကယ်၍ (maxmenu >= ARRAY_SIZE(menuchars)) {
            bha_nga;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    အကယ်၍ (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    ကိန်းပြည့် esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    အတွက် (;;) {
        ကိန်းပြည့် keystroke = get_keystroke_full(1000);
        အကယ်၍ (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            continue;
        အကယ်၍ (keystroke < 0) // timeout
            continue;

        scan_code = keystroke >> 8;
        ကိန်းပြည့် key_ascii = keystroke & 0xff;
        အကယ်၍ (tpm_can_show_menu() && key_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        အကယ်၍ (scan_code == 1) {
            // ESC
            printf("\n");
            ပြန်ပေး;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, node) {
            အကယ်၍ (maxmenu >= ARRAY_SIZE(menuchars))
                bha_nga;
            အကယ်၍ (key_ascii == menuchars[maxmenu]) {
                boot = pos;
                bha_nga;
            }
            maxmenu++;
        }
        အကယ်၍ (boot)
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
    ကိန်းပြည့် type;
    u32 vector;
};
static sa_nracanaa bev_s BEV[20];
static ကိန်းပြည့် BEVCount;
static ကိန်းပြည့် HaveHDBoot, HaveFDBoot;

static void
add_bev(ကိန်းပြည့် type, u32 vector)
{
    အကယ်၍ (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        ပြန်ပေး;
    အကယ်၍ (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        ပြန်ပေး;
    အကယ်၍ (BEVCount >= ARRAY_SIZE(BEV))
        ပြန်ပေး;
    sa_nracanaa bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
void
bcv_prepboot(void)
{
    အကယ်၍ (! CONFIG_BOOT)
        ပြန်ပေး;

    ကိန်းပြည့် haltprio = find_prio("HALT");
    အကယ်၍ (haltprio >= 0)
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
boot_disk(u8 bootdrv, ကိန်းပြည့် checksig)
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

    အကယ်၍ (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        ပြန်ပေး;
    }

    အကယ်၍ (checksig) {
        sa_nracanaa mbr_s *mbr = (void*)0;
        အကယ်၍ (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            ပြန်ပေး;
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
    အကယ်၍ (! CONFIG_CDROM_BOOT)
        ပြန်ပေး;
    printf("Booting from DVD/CD...\n");

    ကိန်းပြည့် status = cdrom_boot(drive);
    အကယ်၍ (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        ပြန်ပေး;
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
    အကယ်၍ (!CONFIG_COREBOOT_FLASH)
        ပြန်ပေး;
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
    အကယ်၍ (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    မဟုတ်ပါက
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    အတွက် (;;) {
        အကယ်၍ (BootRetryTime != (u32)-1 && irqtimer_check(end))
            bha_nga;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
static void
do_boot(ကိန်းပြည့် seq_nr)
{
    အကယ်၍ (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    အကယ်၍ (seq_nr >= BEVCount)
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

ကိန်းပြည့် BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
void VISIBLE32FLAT
handle_18(void)
{
    debug_enter(NULL, DEBUG_HDL_18);
    ကိန်းပြည့် seq = BootSequence + 1;
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
