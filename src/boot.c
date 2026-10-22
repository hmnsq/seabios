// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "fw/paravirt.h" // qemu_cfg_show_boot_menu
#অন্তর্ভাব "hw/pci.h" // pci_bdf_to_*
#অন্তর্ভাব "hw/pcidevice.h" // struct pci_device
#অন্তর্ভাব "hw/rtc.h" // rtc_read
#অন্তর্ভাব "hw/usb.h" // struct usbdevice_s
#অন্তর্ভাব "list.h" // hlist_node
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "std/disk.h" // struct mbr_s
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // irqtimer_calc
#অন্তর্ভাব "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
স্থির অক্ষর *
glob_prefix(ধ্রুবক অক্ষর *glob, ধ্রুবক অক্ষর *str)
{
    জন্য (;;) {
        যদি (!*glob && (!*str || *str == '/'))
            ফেরত (অক্ষর*)str;
        যদি (*glob == '*') {
            যদি (!*str || *str == '/' || *str == glob[1])
                glob++;
            অন্যথা
                str++;
            চালু;
        }
        যদি (*glob != *str)
            ফেরত ফাঁকা;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

স্থির অক্ষর *
build_pci_path(অক্ষর *buf, পূর্ণসংখ্যা max, ধ্রুবক অক্ষর *devname, কাঠামো pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    অক্ষর *p = buf;
    যদি (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } অন্যথা {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        যদি (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    পূর্ণসংখ্যা dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    যদি (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    ফেরত p;
}

স্থির অক্ষর *
build_scsi_path(অক্ষর *buf, পূর্ণসংখ্যা max,
                কাঠামো pci_device *pci, পূর্ণসংখ্যা target, পূর্ণসংখ্যা lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    অক্ষর *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    ফেরত p;
}

স্থির অক্ষর *
build_ata_path(অক্ষর *buf, পূর্ণসংখ্যা max,
               কাঠামো pci_device *pci, পূর্ণসংখ্যা chanid, পূর্ণসংখ্যা slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    অক্ষর *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    ফেরত p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

প্রকারলেখ কাঠামো BootDeviceLCHS {
    অক্ষর *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

স্থির BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
স্থির পূর্ণসংখ্যা Bios_Geometry_গুনতি;

স্থির অক্ষর *
parse_u32(অক্ষর *cur, u32 *n)
{
    u32 m = 0;
    যদি (cur) {
        যখন ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        যদি (*cur != '\0')
            cur++;
    }
    *n = m;
    ফেরত cur;
}

স্থির শূন্য
loadBiosGeometry(শূন্য)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        ফেরত;
    অক্ষর *f = romfile_loadfile("bios-geometry", ফাঁকা);
    যদি (!f)
        ফেরত;

    পূর্ণসংখ্যা i = 0;
    Bios_Geometry_গুনতি = 1;
    যখন (f[i]) {
        যদি (f[i] == '\n')
            Bios_Geometry_গুনতি++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_গুনতি * আকার(BootDeviceLCHS));
    যদি (!BiosGeometry) {
        warn_noalloc();
        মুক্ত(f);
        Bios_Geometry_গুনতি = 0;
        ফেরত;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    করো {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        যদি (f)
            *(f++) = '\0';
        অক্ষর *chs_মানসমূহ = strchr(d->name, ' ');
        যদি (chs_মানসমূহ)
            *(chs_মানসমূহ++) = '\0';
        chs_মানসমূহ = parse_u32(chs_মানসমূহ, &d->lcyls);
        chs_মানসমূহ = parse_u32(chs_মানসমূহ, &d->lheads);
        chs_মানসমূহ = parse_u32(chs_মানসমূহ, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } যখন (f);
}

// Search the bios-geometry list for the given glob pattern.
স্থির BootDeviceLCHS *
boot_lchs_find(ধ্রুবক অক্ষর *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < Bios_Geometry_গুনতি; i++)
        যদি (glob_prefix(glob, BiosGeometry[i].name))
            ফেরত &BiosGeometry[i];
    ফেরত ফাঁকা;
}

পূর্ণসংখ্যা boot_lchs_find_pci_device(কাঠামো pci_device *pci, কাঠামো chs_s *chs)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        ফেরত -1;
    অক্ষর desc[256];
    build_pci_path(desc, আকার(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    যদি (!b)
        ফেরত -1;
    chs->cylinder = (u16)b->lcyls;
    chs->শীর্ষ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ফেরত 0;
}

পূর্ণসংখ্যা boot_lchs_find_scsi_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা target, পূর্ণসংখ্যা lun,
                               কাঠামো chs_s *chs)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        ফেরত -1;
    যদি (!pci)
        // support only pci machine for now
        ফেরত -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    অক্ষর desc[256];
    build_scsi_path(desc, আকার(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    যদি (!b)
        ফেরত -1;
    chs->cylinder = (u16)b->lcyls;
    chs->শীর্ষ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ফেরত 0;
}

পূর্ণসংখ্যা boot_lchs_find_ata_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা chanid, পূর্ণসংখ্যা slave,
                              কাঠামো chs_s *chs)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        ফেরত -1;
    যদি (!pci)
        // support only pci machine for now
        ফেরত -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    অক্ষর desc[256];
    build_ata_path(desc, আকার(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    যদি (!b)
        ফেরত -1;
    chs->cylinder = (u16)b->lcyls;
    chs->শীর্ষ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ফেরত 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

স্থির অক্ষর **Bootorder VARVERIFY32INIT;
স্থির পূর্ণসংখ্যা Bootorder_গুনতি;

স্থির শূন্য
loadBootOrder(শূন্য)
{
    যদি (!CONFIG_BOOTORDER)
        ফেরত;

    অক্ষর *f = romfile_loadfile("bootorder", ফাঁকা);
    যদি (!f)
        ফেরত;

    পূর্ণসংখ্যা i = 0;
    Bootorder_গুনতি = 1;
    যখন (f[i]) {
        যদি (f[i] == '\n')
            Bootorder_গুনতি++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_গুনতি*আকার(অক্ষর*));
    যদি (!Bootorder) {
        warn_noalloc();
        মুক্ত(f);
        Bootorder_গুনতি = 0;
        ফেরত;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    করো {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        যদি (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } যখন (f);
}

// Search the bootorder list for the given glob pattern.
স্থির পূর্ণসংখ্যা
find_prio(ধ্রুবক অক্ষর *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < Bootorder_গুনতি; i++)
        যদি (glob_prefix(glob, Bootorder[i]))
            ফেরত i+1;
    ফেরত -1;
}

u8 is_bootprio_strict(শূন্য)
{
    স্থির পূর্ণসংখ্যা prio_halt = -2;

    যদি (prio_halt == -2)
        prio_halt = find_prio("HALT");
    ফেরত prio_halt >= 0;
}

পূর্ণসংখ্যা bootprio_find_pci_device(কাঠামো pci_device *pci)
{
    যদি (CONFIG_CSM)
        ফেরত csm_bootprio_pci(pci);
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    অক্ষর desc[256];
    build_pci_path(desc, আকার(desc), "*", pci);
    ফেরত find_prio(desc);
}

পূর্ণসংখ্যা bootprio_find_mmio_device(শূন্য *mmio)
{
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    অক্ষর desc[256];
    snprintf(desc, আকার(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    ফেরত find_prio(desc);
}

পূর্ণসংখ্যা bootprio_find_scsi_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা target, পূর্ণসংখ্যা lun)
{
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    যদি (!pci)
        // support only pci machine for now
        ফেরত -1;
    অক্ষর desc[256];
    build_scsi_path(desc, আকার(desc), pci, target, lun);
    ফেরত find_prio(desc);
}

পূর্ণসংখ্যা bootprio_find_scsi_mmio_device(শূন্য *mmio, পূর্ণসংখ্যা target, পূর্ণসংখ্যা lun)
{
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    অক্ষর desc[256];
    snprintf(desc, আকার(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    ফেরত find_prio(desc);
}

পূর্ণসংখ্যা bootprio_find_ata_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা chanid, পূর্ণসংখ্যা slave)
{
    যদি (CONFIG_CSM)
        ফেরত csm_bootprio_ata(pci, chanid, slave);
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    যদি (!pci)
        // support only pci machine for now
        ফেরত -1;
    অক্ষর desc[256];
    build_ata_path(desc, আকার(desc), pci, chanid, slave);
    ফেরত find_prio(desc);
}

পূর্ণসংখ্যা bootprio_find_fdc_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা port, পূর্ণসংখ্যা fdid)
{
    যদি (CONFIG_CSM)
        ফেরত csm_bootprio_fdc(pci, port, fdid);
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    যদি (!pci)
        // support only pci machine for now
        ফেরত -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    অক্ষর desc[256], *p;
    p = build_pci_path(desc, আকার(desc), "isa", pci);
    snprintf(p, desc+আকার(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    ফেরত find_prio(desc);
}

পূর্ণসংখ্যা bootprio_find_pci_rom(কাঠামো pci_device *pci, পূর্ণসংখ্যা instance)
{
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    অক্ষর desc[256], *p;
    p = build_pci_path(desc, আকার(desc), "*", pci);
    যদি (instance)
        snprintf(p, desc+আকার(desc)-p, ":rom%x", instance);
    ফেরত find_prio(desc);
}

পূর্ণসংখ্যা bootprio_find_named_rom(ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা instance)
{
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    অক্ষর desc[256], *p;
    p = desc + snprintf(desc, আকার(desc), "/rom@%s", name);
    যদি (instance)
        snprintf(p, desc+আকার(desc)-p, ":rom%x", instance);
    ফেরত find_prio(desc);
}

স্থির পূর্ণসংখ্যা usb_portmap(কাঠামো usbdevice_s *usbdev)
{
    যদি (usbdev->hub->op->portmap)
        ফেরত usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    ফেরত usbdev->port + 1;
}

স্থির অক্ষর *
build_usb_path(অক্ষর *buf, পূর্ণসংখ্যা max, কাঠামো usbhub_s *hub)
{
    যদি (!hub->usbdev)
        // Root hub - nothing to add.
        ফেরত buf;
    অক্ষর *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    ফেরত p;
}

পূর্ণসংখ্যা bootprio_find_usb(কাঠামো usbdevice_s *usbdev, পূর্ণসংখ্যা lun)
{
    যদি (!CONFIG_BOOTORDER)
        ফেরত -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    অক্ষর desc[256], *p;

    যদি (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, আকার(desc), "usb", usbdev->hub->cntl->pci);
    অন্যথা যদি (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, আকার(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    অন্যথা
        ফেরত -1;

    p = build_usb_path(p, desc+আকার(desc)-p, usbdev->hub);
    snprintf(p, desc+আকার(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    পূর্ণসংখ্যা ret = find_prio(desc);
    যদি (ret >= 0)
        ফেরত ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+আকার(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    ফেরত find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

স্থির পূর্ণসংখ্যা BootRetryTime;
স্থির পূর্ণসংখ্যা CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

স্থির পূর্ণসংখ্যা DefaultFloppyPrio = 101;
স্থির পূর্ণসংখ্যা DefaultCDPrio     = 102;
স্থির পূর্ণসংখ্যা DefaultHDPrio     = 103;
স্থির পূর্ণসংখ্যা DefaultBEVPrio    = 104;

শূন্য
boot_init(শূন্য)
{
    যদি (! CONFIG_BOOT)
        ফেরত;

    যদি (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        যদি (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        পূর্ণসংখ্যা i;
        জন্য (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            নির্বাচন (val) {
            ক্ষেত্র 1: DefaultFloppyPrio = i; বিরতি;
            ক্ষেত্র 2: DefaultHDPrio = i;     বিরতি;
            ক্ষেত্র 3: DefaultCDPrio = i;     বিরতি;
            ক্ষেত্র 4: DefaultBEVPrio = i;    বিরতি;
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

কাঠামো bootentry_s {
    পূর্ণসংখ্যা type;
    সংঘ {
        u32 তথ্য;
        কাঠামো segoff_s vector;
        কাঠামো drive_s *drive;
    };
    পূর্ণসংখ্যা priority;
    ধ্রুবক অক্ষর *description;
    কাঠামো hlist_গ্রন্থি গ্রন্থি;
};
স্থির কাঠামো hlist_শীর্ষ BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

স্থির শূন্য
bootentry_add(পূর্ণসংখ্যা type, পূর্ণসংখ্যা prio, u32 তথ্য, ধ্রুবক অক্ষর *desc)
{
    যদি (! CONFIG_BOOT)
        ফেরত;
    কাঠামো bootentry_s *be = malloc_tmp(আকার(*be));
    যদি (!be) {
        warn_noalloc();
        ফেরত;
    }
    be->type = type;
    be->priority = prio;
    be->তথ্য = তথ্য;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, তথ্য);

    // Add entry in sorted order.
    কাঠামো hlist_গ্রন্থি **pprev;
    কাঠামো bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, গ্রন্থি) {
        যদি (be->priority < pos->priority)
            বিরতি;
        যদি (be->priority > pos->priority)
            চালু;
        যদি (be->type < pos->type)
            বিরতি;
        যদি (be->type > pos->type)
            চালু;
        যদি (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            বিরতি;
    }
    hlist_add(&be->গ্রন্থি, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
স্থির অন্তঃরেখ পূর্ণসংখ্যা defPrio(পূর্ণসংখ্যা priority, পূর্ণসংখ্যা defaultprio) {
    ফেরত (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
শূন্য
boot_add_bev(u16 seg, u16 bev, u16 desc, পূর্ণসংখ্যা prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
শূন্য
boot_add_bcv(u16 seg, u16 ip, u16 desc, পূর্ণসংখ্যা prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

শূন্য
boot_add_floppy(কাঠামো drive_s *drive, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

শূন্য
boot_add_hd(কাঠামো drive_s *drive, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

শূন্য
boot_add_cd(কাঠামো drive_s *drive, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio)
{
    যদি (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        অক্ষর *extra = cdrom_media_info(drive);
        যদি (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            মুক্ত(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
শূন্য
boot_add_cbfs(শূন্য *তথ্য, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)তথ্য, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
স্থির পূর্ণসংখ্যা
check_for_keystroke(শূন্য)
{
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    ফেরত !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
স্থির পূর্ণসংখ্যা
get_raw_keystroke(শূন্য)
{
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    ফেরত br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
পূর্ণসংখ্যা
get_keystroke_full(পূর্ণসংখ্যা msec)
{
    u32 end = irqtimer_calc(msec);
    জন্য (;;) {
        যদি (check_for_keystroke())
            ফেরত get_raw_keystroke();
        যদি (irqtimer_check(end))
            ফেরত -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
পূর্ণসংখ্যা
get_keystroke(পূর্ণসংখ্যা msec)
{
    পূর্ণসংখ্যা keystroke = get_keystroke_full(msec);

    যদি (keystroke < 0)
        ফেরত keystroke;
    ফেরত keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

স্থির ধ্রুবক অক্ষর menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
শূন্য
interactive_bootmenu(শূন্য)
{
    যদি (! CONFIG_BOOTMENU)
        ফেরত;
    পূর্ণসংখ্যা show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    যদি (!show_boot_menu)
        ফেরত;

    // skip menu if only one boot device and no TPM
    যদি (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->পরবর্তী) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        ফেরত;
    }

    পূর্ণসংখ্যা menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    পূর্ণসংখ্যা menukey = romfile_loadint("etc/boot-menu-key", 1);
    পূর্ণসংখ্যা scan_code;
    যদি (menutime >= 0) {
        যখন (get_keystroke(0) >= 0)
            ;

        অক্ষর *bootmsg = romfile_loadfile("etc/boot-menu-message", ফাঁকা);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        মুক্ত(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        যদি (scan_code != menukey)
            ফেরত;
    }

    যখন (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    পূর্ণসংখ্যা maxmenu = 0;
    কাঠামো bootentry_s *pos, *boot = ফাঁকা;
    hlist_for_each_entry(pos, &BootList, গ্রন্থি) {
        অক্ষর desc[77];
        যদি (maxmenu >= ARRAY_আকার(menuchars)) {
            বিরতি;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_আকার(desc)));
        maxmenu++;
    }
    যদি (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    পূর্ণসংখ্যা esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    জন্য (;;) {
        পূর্ণসংখ্যা keystroke = get_keystroke_full(1000);
        যদি (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            চালু;
        যদি (keystroke < 0) // timeout
            চালু;

        scan_code = keystroke >> 8;
        পূর্ণসংখ্যা চাবি_ascii = keystroke & 0xff;
        যদি (tpm_can_show_menu() && চাবি_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        যদি (scan_code == 1) {
            // ESC
            printf("\n");
            ফেরত;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, গ্রন্থি) {
            যদি (maxmenu >= ARRAY_আকার(menuchars))
                বিরতি;
            যদি (চাবি_ascii == menuchars[maxmenu]) {
                boot = pos;
                বিরতি;
            }
            maxmenu++;
        }
        যদি (boot)
            বিরতি;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->গ্রন্থি);
    boot->priority = 0;
    hlist_add_শীর্ষ(&boot->গ্রন্থি, &BootList);
}

// BEV (Boot Execution Vector) list
কাঠামো bev_s {
    পূর্ণসংখ্যা type;
    u32 vector;
};
স্থির কাঠামো bev_s BEV[20];
স্থির পূর্ণসংখ্যা BEVCount;
স্থির পূর্ণসংখ্যা HaveHDBoot, HaveFDBoot;

স্থির শূন্য
add_bev(পূর্ণসংখ্যা type, u32 vector)
{
    যদি (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        ফেরত;
    যদি (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        ফেরত;
    যদি (BEVCount >= ARRAY_আকার(BEV))
        ফেরত;
    কাঠামো bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
শূন্য
bcv_prepboot(শূন্য)
{
    যদি (! CONFIG_BOOT)
        ফেরত;

    পূর্ণসংখ্যা haltprio = find_prio("HALT");
    যদি (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    কাঠামো bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, গ্রন্থি) {
        নির্বাচন (pos->type) {
        ক্ষেত্র IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            বিরতি;
        ক্ষেত্র IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            বিরতি;
        ক্ষেত্র IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            বিরতি;
        ক্ষেত্র IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        পূর্বনির্ধারিত:
            add_bev(pos->type, pos->তথ্য);
            বিরতি;
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
স্থির শূন্য
call_boot_entry(কাঠামো segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
স্থির শূন্য
boot_disk(u8 bootdrv, পূর্ণসংখ্যা checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    যদি (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        ফেরত;
    }

    যদি (checksig) {
        কাঠামো mbr_s *mbr = (শূন্য*)0;
        যদি (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            ফেরত;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
স্থির শূন্য
boot_cdrom(কাঠামো drive_s *drive)
{
    যদি (! CONFIG_CDROM_BOOT)
        ফেরত;
    printf("Booting from DVD/CD...\n");

    পূর্ণসংখ্যা status = cdrom_boot(drive);
    যদি (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        ফেরত;
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
স্থির শূন্য
boot_cbfs(কাঠামো cbfs_file *file)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        ফেরত;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
স্থির শূন্য
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    কাঠামো segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
স্থির শূন্য
boot_fail(শূন্য)
{
    যদি (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    অন্যথা
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    জন্য (;;) {
        যদি (BootRetryTime != (u32)-1 && irqtimer_check(end))
            বিরতি;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
স্থির শূন্য
do_boot(পূর্ণসংখ্যা seq_nr)
{
    যদি (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    যদি (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    কাঠামো bev_s *ie = &BEV[seq_nr];
    নির্বাচন (ie->type) {
    ক্ষেত্র IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        বিরতি;
    ক্ষেত্র IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        বিরতি;
    ক্ষেত্র IPL_TYPE_CDROM:
        boot_cdrom((শূন্য*)ie->vector);
        বিরতি;
    ক্ষেত্র IPL_TYPE_CBFS:
        boot_cbfs((শূন্য*)ie->vector);
        বিরতি;
    ক্ষেত্র IPL_TYPE_BEV:
        boot_rom(ie->vector);
        বিরতি;
    ক্ষেত্র IPL_TYPE_HALT:
        boot_fail();
        বিরতি;
    }

    // Boot failed: invoke the boot recovery function
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

পূর্ণসংখ্যা BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
শূন্য VISIBLE32FLAT
handle_18(শূন্য)
{
    debug_enter(ফাঁকা, DEBUG_HDL_18);
    পূর্ণসংখ্যা seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
শূন্য VISIBLE32FLAT
handle_19(শূন্য)
{
    debug_enter(ফাঁকা, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
