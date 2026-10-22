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
    for (;;) {
        ᮚᮦᮔ᮪ (!*glob && (!*str || *str == '/'))
            ᮝᮜᮤ (var_na*)str;
        ᮚᮦᮔ᮪ (*glob == '*') {
            ᮚᮦᮔ᮪ (!*str || *str == '/' || *str == glob[1])
                glob++;
            ᮜᮤᮚ
                str++;
            continue;
        }
        ᮚᮦᮔ᮪ (*glob != *str)
            ᮝᮜᮤ NULL;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

static var_na *
build_pci_path(var_na *buf, puur_naa_nka max, const var_na *devname, sa_nracanaa pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    var_na *p = buf;
    ᮚᮦᮔ᮪ (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } ᮜᮤᮚ {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        ᮚᮦᮔ᮪ (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    puur_naa_nka dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    ᮚᮦᮔ᮪ (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    ᮝᮜᮤ p;
}

static var_na *
build_scsi_path(var_na *buf, puur_naa_nka max,
                sa_nracanaa pci_device *pci, puur_naa_nka target, puur_naa_nka lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    var_na *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    ᮝᮜᮤ p;
}

static var_na *
build_ata_path(var_na *buf, puur_naa_nka max,
               sa_nracanaa pci_device *pci, puur_naa_nka chanid, puur_naa_nka slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    var_na *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    ᮝᮜᮤ p;
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
static puur_naa_nka BiosGeometryCount;

static var_na *
parse_u32(var_na *cur, u32 *n)
{
    u32 m = 0;
    ᮚᮦᮔ᮪ (cur) {
        ᮞᮨᮓᮨᮜ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        ᮚᮦᮔ᮪ (*cur != '\0')
            cur++;
    }
    *n = m;
    ᮝᮜᮤ cur;
}

static void
loadBiosGeometry(void)
{
    ᮚᮦᮔ᮪ (!CONFIG_HOST_BIOS_GEOMETRY)
        ᮝᮜᮤ;
    var_na *f = romfile_loadfile("bios-geometry", NULL);
    ᮚᮦᮔ᮪ (!f)
        ᮝᮜᮤ;

    puur_naa_nka i = 0;
    BiosGeometryCount = 1;
    ᮞᮨᮓᮨᮜ (f[i]) {
        ᮚᮦᮔ᮪ (f[i] == '\n')
            BiosGeometryCount++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(BiosGeometryCount * parimaa_na(BootDeviceLCHS));
    ᮚᮦᮔ᮪ (!BiosGeometry) {
        warn_noalloc();
        free(f);
        BiosGeometryCount = 0;
        ᮝᮜᮤ;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    do {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        ᮚᮦᮔ᮪ (f)
            *(f++) = '\0';
        var_na *chs_values = strchr(d->name, ' ');
        ᮚᮦᮔ᮪ (chs_values)
            *(chs_values++) = '\0';
        chs_values = parse_u32(chs_values, &d->lcyls);
        chs_values = parse_u32(chs_values, &d->lheads);
        chs_values = parse_u32(chs_values, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } ᮞᮨᮓᮨᮜ (f);
}

// Search the bios-geometry list for the given glob pattern.
static BootDeviceLCHS *
boot_lchs_find(const var_na *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    puur_naa_nka i;
    for (i = 0; i < BiosGeometryCount; i++)
        ᮚᮦᮔ᮪ (glob_prefix(glob, BiosGeometry[i].name))
            ᮝᮜᮤ &BiosGeometry[i];
    ᮝᮜᮤ NULL;
}

puur_naa_nka boot_lchs_find_pci_device(sa_nracanaa pci_device *pci, sa_nracanaa chs_s *chs)
{
    ᮚᮦᮔ᮪ (!CONFIG_HOST_BIOS_GEOMETRY)
        ᮝᮜᮤ -1;
    var_na desc[256];
    build_pci_path(desc, parimaa_na(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ᮚᮦᮔ᮪ (!b)
        ᮝᮜᮤ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ᮝᮜᮤ 0;
}

puur_naa_nka boot_lchs_find_scsi_device(sa_nracanaa pci_device *pci, puur_naa_nka target, puur_naa_nka lun,
                               sa_nracanaa chs_s *chs)
{
    ᮚᮦᮔ᮪ (!CONFIG_HOST_BIOS_GEOMETRY)
        ᮝᮜᮤ -1;
    ᮚᮦᮔ᮪ (!pci)
        // support only pci machine for now
        ᮝᮜᮤ -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    var_na desc[256];
    build_scsi_path(desc, parimaa_na(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ᮚᮦᮔ᮪ (!b)
        ᮝᮜᮤ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ᮝᮜᮤ 0;
}

puur_naa_nka boot_lchs_find_ata_device(sa_nracanaa pci_device *pci, puur_naa_nka chanid, puur_naa_nka slave,
                              sa_nracanaa chs_s *chs)
{
    ᮚᮦᮔ᮪ (!CONFIG_HOST_BIOS_GEOMETRY)
        ᮝᮜᮤ -1;
    ᮚᮦᮔ᮪ (!pci)
        // support only pci machine for now
        ᮝᮜᮤ -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    var_na desc[256];
    build_ata_path(desc, parimaa_na(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ᮚᮦᮔ᮪ (!b)
        ᮝᮜᮤ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ᮝᮜᮤ 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

static var_na **Bootorder VARVERIFY32INIT;
static puur_naa_nka BootorderCount;

static void
loadBootOrder(void)
{
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ;

    var_na *f = romfile_loadfile("bootorder", NULL);
    ᮚᮦᮔ᮪ (!f)
        ᮝᮜᮤ;

    puur_naa_nka i = 0;
    BootorderCount = 1;
    ᮞᮨᮓᮨᮜ (f[i]) {
        ᮚᮦᮔ᮪ (f[i] == '\n')
            BootorderCount++;
        i++;
    }
    Bootorder = malloc_tmphigh(BootorderCount*parimaa_na(var_na*));
    ᮚᮦᮔ᮪ (!Bootorder) {
        warn_noalloc();
        free(f);
        BootorderCount = 0;
        ᮝᮜᮤ;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    do {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        ᮚᮦᮔ᮪ (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } ᮞᮨᮓᮨᮜ (f);
}

// Search the bootorder list for the given glob pattern.
static puur_naa_nka
find_prio(const var_na *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    puur_naa_nka i;
    for (i = 0; i < BootorderCount; i++)
        ᮚᮦᮔ᮪ (glob_prefix(glob, Bootorder[i]))
            ᮝᮜᮤ i+1;
    ᮝᮜᮤ -1;
}

u8 is_bootprio_strict(void)
{
    static puur_naa_nka prio_halt = -2;

    ᮚᮦᮔ᮪ (prio_halt == -2)
        prio_halt = find_prio("HALT");
    ᮝᮜᮤ prio_halt >= 0;
}

puur_naa_nka bootprio_find_pci_device(sa_nracanaa pci_device *pci)
{
    ᮚᮦᮔ᮪ (CONFIG_CSM)
        ᮝᮜᮤ csm_bootprio_pci(pci);
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    var_na desc[256];
    build_pci_path(desc, parimaa_na(desc), "*", pci);
    ᮝᮜᮤ find_prio(desc);
}

puur_naa_nka bootprio_find_mmio_device(void *mmio)
{
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    var_na desc[256];
    snprintf(desc, parimaa_na(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    ᮝᮜᮤ find_prio(desc);
}

puur_naa_nka bootprio_find_scsi_device(sa_nracanaa pci_device *pci, puur_naa_nka target, puur_naa_nka lun)
{
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    ᮚᮦᮔ᮪ (!pci)
        // support only pci machine for now
        ᮝᮜᮤ -1;
    var_na desc[256];
    build_scsi_path(desc, parimaa_na(desc), pci, target, lun);
    ᮝᮜᮤ find_prio(desc);
}

puur_naa_nka bootprio_find_scsi_mmio_device(void *mmio, puur_naa_nka target, puur_naa_nka lun)
{
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    var_na desc[256];
    snprintf(desc, parimaa_na(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    ᮝᮜᮤ find_prio(desc);
}

puur_naa_nka bootprio_find_ata_device(sa_nracanaa pci_device *pci, puur_naa_nka chanid, puur_naa_nka slave)
{
    ᮚᮦᮔ᮪ (CONFIG_CSM)
        ᮝᮜᮤ csm_bootprio_ata(pci, chanid, slave);
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    ᮚᮦᮔ᮪ (!pci)
        // support only pci machine for now
        ᮝᮜᮤ -1;
    var_na desc[256];
    build_ata_path(desc, parimaa_na(desc), pci, chanid, slave);
    ᮝᮜᮤ find_prio(desc);
}

puur_naa_nka bootprio_find_fdc_device(sa_nracanaa pci_device *pci, puur_naa_nka port, puur_naa_nka fdid)
{
    ᮚᮦᮔ᮪ (CONFIG_CSM)
        ᮝᮜᮤ csm_bootprio_fdc(pci, port, fdid);
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    ᮚᮦᮔ᮪ (!pci)
        // support only pci machine for now
        ᮝᮜᮤ -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    var_na desc[256], *p;
    p = build_pci_path(desc, parimaa_na(desc), "isa", pci);
    snprintf(p, desc+parimaa_na(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    ᮝᮜᮤ find_prio(desc);
}

puur_naa_nka bootprio_find_pci_rom(sa_nracanaa pci_device *pci, puur_naa_nka instance)
{
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    var_na desc[256], *p;
    p = build_pci_path(desc, parimaa_na(desc), "*", pci);
    ᮚᮦᮔ᮪ (instance)
        snprintf(p, desc+parimaa_na(desc)-p, ":rom%x", instance);
    ᮝᮜᮤ find_prio(desc);
}

puur_naa_nka bootprio_find_named_rom(const var_na *name, puur_naa_nka instance)
{
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    var_na desc[256], *p;
    p = desc + snprintf(desc, parimaa_na(desc), "/rom@%s", name);
    ᮚᮦᮔ᮪ (instance)
        snprintf(p, desc+parimaa_na(desc)-p, ":rom%x", instance);
    ᮝᮜᮤ find_prio(desc);
}

static puur_naa_nka usb_portmap(sa_nracanaa usbdevice_s *usbdev)
{
    ᮚᮦᮔ᮪ (usbdev->hub->op->portmap)
        ᮝᮜᮤ usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    ᮝᮜᮤ usbdev->port + 1;
}

static var_na *
build_usb_path(var_na *buf, puur_naa_nka max, sa_nracanaa usbhub_s *hub)
{
    ᮚᮦᮔ᮪ (!hub->usbdev)
        // Root hub - nothing to add.
        ᮝᮜᮤ buf;
    var_na *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    ᮝᮜᮤ p;
}

puur_naa_nka bootprio_find_usb(sa_nracanaa usbdevice_s *usbdev, puur_naa_nka lun)
{
    ᮚᮦᮔ᮪ (!CONFIG_BOOTORDER)
        ᮝᮜᮤ -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    var_na desc[256], *p;

    ᮚᮦᮔ᮪ (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, parimaa_na(desc), "usb", usbdev->hub->cntl->pci);
    ᮜᮤᮚ ᮚᮦᮔ᮪ (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, parimaa_na(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    ᮜᮤᮚ
        ᮝᮜᮤ -1;

    p = build_usb_path(p, desc+parimaa_na(desc)-p, usbdev->hub);
    snprintf(p, desc+parimaa_na(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    puur_naa_nka ret = find_prio(desc);
    ᮚᮦᮔ᮪ (ret >= 0)
        ᮝᮜᮤ ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+parimaa_na(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    ᮝᮜᮤ find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

static puur_naa_nka BootRetryTime;
static puur_naa_nka CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

static puur_naa_nka DefaultFloppyPrio = 101;
static puur_naa_nka DefaultCDPrio     = 102;
static puur_naa_nka DefaultHDPrio     = 103;
static puur_naa_nka DefaultBEVPrio    = 104;

void
boot_init(void)
{
    ᮚᮦᮔ᮪ (! CONFIG_BOOT)
        ᮝᮜᮤ;

    ᮚᮦᮔ᮪ (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        ᮚᮦᮔ᮪ (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        puur_naa_nka i;
        for (i=101; i<104; i++) {
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
    puur_naa_nka type;
    sa_ngha {
        u32 data;
        sa_nracanaa segoff_s vector;
        sa_nracanaa drive_s *drive;
    };
    puur_naa_nka priority;
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
bootentry_add(puur_naa_nka type, puur_naa_nka prio, u32 data, const var_na *desc)
{
    ᮚᮦᮔ᮪ (! CONFIG_BOOT)
        ᮝᮜᮤ;
    sa_nracanaa bootentry_s *be = malloc_tmp(parimaa_na(*be));
    ᮚᮦᮔ᮪ (!be) {
        warn_noalloc();
        ᮝᮜᮤ;
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
        ᮚᮦᮔ᮪ (be->priority < pos->priority)
            bha_nga;
        ᮚᮦᮔ᮪ (be->priority > pos->priority)
            continue;
        ᮚᮦᮔ᮪ (be->type < pos->type)
            bha_nga;
        ᮚᮦᮔ᮪ (be->type > pos->type)
            continue;
        ᮚᮦᮔ᮪ (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            bha_nga;
    }
    hlist_add(&be->node, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
static anta_hstha puur_naa_nka defPrio(puur_naa_nka priority, puur_naa_nka defaultprio) {
    ᮝᮜᮤ (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
void
boot_add_bev(u16 seg, u16 bev, u16 desc, puur_naa_nka prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
void
boot_add_bcv(u16 seg, u16 ip, u16 desc, puur_naa_nka prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

void
boot_add_floppy(sa_nracanaa drive_s *drive, const var_na *desc, puur_naa_nka prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

void
boot_add_hd(sa_nracanaa drive_s *drive, const var_na *desc, puur_naa_nka prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

void
boot_add_cd(sa_nracanaa drive_s *drive, const var_na *desc, puur_naa_nka prio)
{
    ᮚᮦᮔ᮪ (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        var_na *extra = cdrom_media_info(drive);
        ᮚᮦᮔ᮪ (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            free(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
void
boot_add_cbfs(void *data, const var_na *desc, puur_naa_nka prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
static puur_naa_nka
check_for_keystroke(void)
{
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    ᮝᮜᮤ !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
static puur_naa_nka
get_raw_keystroke(void)
{
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    ᮝᮜᮤ br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
puur_naa_nka
get_keystroke_full(puur_naa_nka msec)
{
    u32 end = irqtimer_calc(msec);
    for (;;) {
        ᮚᮦᮔ᮪ (check_for_keystroke())
            ᮝᮜᮤ get_raw_keystroke();
        ᮚᮦᮔ᮪ (irqtimer_check(end))
            ᮝᮜᮤ -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
puur_naa_nka
get_keystroke(puur_naa_nka msec)
{
    puur_naa_nka keystroke = get_keystroke_full(msec);

    ᮚᮦᮔ᮪ (keystroke < 0)
        ᮝᮜᮤ keystroke;
    ᮝᮜᮤ keystroke >> 8;
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
    ᮚᮦᮔ᮪ (! CONFIG_BOOTMENU)
        ᮝᮜᮤ;
    puur_naa_nka show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    ᮚᮦᮔ᮪ (!show_boot_menu)
        ᮝᮜᮤ;

    // skip menu if only one boot device and no TPM
    ᮚᮦᮔ᮪ (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        ᮝᮜᮤ;
    }

    puur_naa_nka menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    puur_naa_nka menukey = romfile_loadint("etc/boot-menu-key", 1);
    puur_naa_nka scan_code;
    ᮚᮦᮔ᮪ (menutime >= 0) {
        ᮞᮨᮓᮨᮜ (get_keystroke(0) >= 0)
            ;

        var_na *bootmsg = romfile_loadfile("etc/boot-menu-message", NULL);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        free(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        ᮚᮦᮔ᮪ (scan_code != menukey)
            ᮝᮜᮤ;
    }

    ᮞᮨᮓᮨᮜ (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    puur_naa_nka maxmenu = 0;
    sa_nracanaa bootentry_s *pos, *boot = NULL;
    hlist_for_each_entry(pos, &BootList, node) {
        var_na desc[77];
        ᮚᮦᮔ᮪ (maxmenu >= ARRAY_SIZE(menuchars)) {
            bha_nga;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    ᮚᮦᮔ᮪ (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    puur_naa_nka esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    for (;;) {
        puur_naa_nka keystroke = get_keystroke_full(1000);
        ᮚᮦᮔ᮪ (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            continue;
        ᮚᮦᮔ᮪ (keystroke < 0) // timeout
            continue;

        scan_code = keystroke >> 8;
        puur_naa_nka key_ascii = keystroke & 0xff;
        ᮚᮦᮔ᮪ (tpm_can_show_menu() && key_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        ᮚᮦᮔ᮪ (scan_code == 1) {
            // ESC
            printf("\n");
            ᮝᮜᮤ;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, node) {
            ᮚᮦᮔ᮪ (maxmenu >= ARRAY_SIZE(menuchars))
                bha_nga;
            ᮚᮦᮔ᮪ (key_ascii == menuchars[maxmenu]) {
                boot = pos;
                bha_nga;
            }
            maxmenu++;
        }
        ᮚᮦᮔ᮪ (boot)
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
    puur_naa_nka type;
    u32 vector;
};
static sa_nracanaa bev_s BEV[20];
static puur_naa_nka BEVCount;
static puur_naa_nka HaveHDBoot, HaveFDBoot;

static void
add_bev(puur_naa_nka type, u32 vector)
{
    ᮚᮦᮔ᮪ (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        ᮝᮜᮤ;
    ᮚᮦᮔ᮪ (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        ᮝᮜᮤ;
    ᮚᮦᮔ᮪ (BEVCount >= ARRAY_SIZE(BEV))
        ᮝᮜᮤ;
    sa_nracanaa bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
void
bcv_prepboot(void)
{
    ᮚᮦᮔ᮪ (! CONFIG_BOOT)
        ᮝᮜᮤ;

    puur_naa_nka haltprio = find_prio("HALT");
    ᮚᮦᮔ᮪ (haltprio >= 0)
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
boot_disk(u8 bootdrv, puur_naa_nka checksig)
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

    ᮚᮦᮔ᮪ (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        ᮝᮜᮤ;
    }

    ᮚᮦᮔ᮪ (checksig) {
        sa_nracanaa mbr_s *mbr = (void*)0;
        ᮚᮦᮔ᮪ (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            ᮝᮜᮤ;
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
    ᮚᮦᮔ᮪ (! CONFIG_CDROM_BOOT)
        ᮝᮜᮤ;
    printf("Booting from DVD/CD...\n");

    puur_naa_nka status = cdrom_boot(drive);
    ᮚᮦᮔ᮪ (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        ᮝᮜᮤ;
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
    ᮚᮦᮔ᮪ (!CONFIG_COREBOOT_FLASH)
        ᮝᮜᮤ;
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
    ᮚᮦᮔ᮪ (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    ᮜᮤᮚ
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    for (;;) {
        ᮚᮦᮔ᮪ (BootRetryTime != (u32)-1 && irqtimer_check(end))
            bha_nga;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
static void
do_boot(puur_naa_nka seq_nr)
{
    ᮚᮦᮔ᮪ (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    ᮚᮦᮔ᮪ (seq_nr >= BEVCount)
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

puur_naa_nka BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
void VISIBLE32FLAT
handle_18(void)
{
    debug_enter(NULL, DEBUG_HDL_18);
    puur_naa_nka seq = BootSequence + 1;
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
