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
স্থির বর্ণ *
glob_prefix(ধ্রুব বর্ণ *glob, ধ্রুব বর্ণ *str)
{
    প্রতি (;;) {
        যদি (!*glob && (!*str || *str == '/'))
            প্রত্যাগম (বর্ণ*)str;
        যদি (*glob == '*') {
            যদি (!*str || *str == '/' || *str == glob[1])
                glob++;
            অন্যথা
                str++;
            অনুবর্ত;
        }
        যদি (*glob != *str)
            প্রত্যাগম রিক্ত;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

স্থির বর্ণ *
build_pci_path(বর্ণ *buf, পূর্ণাঙ্ক max, ধ্রুব বর্ণ *devname, সংরচনা pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    বর্ণ *p = buf;
    যদি (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } অন্যথা {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        যদি (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    পূর্ণাঙ্ক dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    যদি (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    প্রত্যাগম p;
}

স্থির বর্ণ *
build_scsi_path(বর্ণ *buf, পূর্ণাঙ্ক max,
                সংরচনা pci_device *pci, পূর্ণাঙ্ক target, পূর্ণাঙ্ক lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    বর্ণ *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    প্রত্যাগম p;
}

স্থির বর্ণ *
build_ata_path(বর্ণ *buf, পূর্ণাঙ্ক max,
               সংরচনা pci_device *pci, পূর্ণাঙ্ক chanid, পূর্ণাঙ্ক slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    বর্ণ *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    প্রত্যাগম p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

প্রকারনির্দেশ সংরচনা BootDeviceLCHS {
    বর্ণ *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

স্থির BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
স্থির পূর্ণাঙ্ক Bios_Geometry_গুনতি;

স্থির বর্ণ *
parse_u32(বর্ণ *cur, u32 *n)
{
    u32 m = 0;
    যদি (cur) {
        যাবত্ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        যদি (*cur != '\0')
            cur++;
    }
    *n = m;
    প্রত্যাগম cur;
}

স্থির শূন্য
loadBiosGeometry(শূন্য)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        প্রত্যাগম;
    বর্ণ *f = romfile_loadfile("bios-geometry", রিক্ত);
    যদি (!f)
        প্রত্যাগম;

    পূর্ণাঙ্ক i = 0;
    Bios_Geometry_গুনতি = 1;
    যাবত্ (f[i]) {
        যদি (f[i] == '\n')
            Bios_Geometry_গুনতি++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_গুনতি * পরিমাণ(BootDeviceLCHS));
    যদি (!BiosGeometry) {
        warn_noalloc();
        মুক্ত(f);
        Bios_Geometry_গুনতি = 0;
        প্রত্যাগম;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    কুরু {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        যদি (f)
            *(f++) = '\0';
        বর্ণ *chs_মানসমূহ = strchr(d->name, ' ');
        যদি (chs_মানসমূহ)
            *(chs_মানসমূহ++) = '\0';
        chs_মানসমূহ = parse_u32(chs_মানসমূহ, &d->lcyls);
        chs_মানসমূহ = parse_u32(chs_মানসমূহ, &d->lheads);
        chs_মানসমূহ = parse_u32(chs_মানসমূহ, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } যাবত্ (f);
}

// Search the bios-geometry list for the given glob pattern.
স্থির BootDeviceLCHS *
boot_lchs_find(ধ্রুব বর্ণ *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    পূর্ণাঙ্ক i;
    প্রতি (i = 0; i < Bios_Geometry_গুনতি; i++)
        যদি (glob_prefix(glob, BiosGeometry[i].name))
            প্রত্যাগম &BiosGeometry[i];
    প্রত্যাগম রিক্ত;
}

পূর্ণাঙ্ক boot_lchs_find_pci_device(সংরচনা pci_device *pci, সংরচনা chs_s *chs)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        প্রত্যাগম -1;
    বর্ণ desc[256];
    build_pci_path(desc, পরিমাণ(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    যদি (!b)
        প্রত্যাগম -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    প্রত্যাগম 0;
}

পূর্ণাঙ্ক boot_lchs_find_scsi_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক target, পূর্ণাঙ্ক lun,
                               সংরচনা chs_s *chs)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        প্রত্যাগম -1;
    যদি (!pci)
        // support only pci machine for now
        প্রত্যাগম -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    বর্ণ desc[256];
    build_scsi_path(desc, পরিমাণ(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    যদি (!b)
        প্রত্যাগম -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    প্রত্যাগম 0;
}

পূর্ণাঙ্ক boot_lchs_find_ata_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক chanid, পূর্ণাঙ্ক slave,
                              সংরচনা chs_s *chs)
{
    যদি (!CONFIG_HOST_BIOS_GEOMETRY)
        প্রত্যাগম -1;
    যদি (!pci)
        // support only pci machine for now
        প্রত্যাগম -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    বর্ণ desc[256];
    build_ata_path(desc, পরিমাণ(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    যদি (!b)
        প্রত্যাগম -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    প্রত্যাগম 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

স্থির বর্ণ **Bootorder VARVERIFY32INIT;
স্থির পূর্ণাঙ্ক Bootorder_গুনতি;

স্থির শূন্য
loadBootOrder(শূন্য)
{
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম;

    বর্ণ *f = romfile_loadfile("bootorder", রিক্ত);
    যদি (!f)
        প্রত্যাগম;

    পূর্ণাঙ্ক i = 0;
    Bootorder_গুনতি = 1;
    যাবত্ (f[i]) {
        যদি (f[i] == '\n')
            Bootorder_গুনতি++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_গুনতি*পরিমাণ(বর্ণ*));
    যদি (!Bootorder) {
        warn_noalloc();
        মুক্ত(f);
        Bootorder_গুনতি = 0;
        প্রত্যাগম;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    কুরু {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        যদি (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } যাবত্ (f);
}

// Search the bootorder list for the given glob pattern.
স্থির পূর্ণাঙ্ক
find_prio(ধ্রুব বর্ণ *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    পূর্ণাঙ্ক i;
    প্রতি (i = 0; i < Bootorder_গুনতি; i++)
        যদি (glob_prefix(glob, Bootorder[i]))
            প্রত্যাগম i+1;
    প্রত্যাগম -1;
}

u8 is_bootprio_strict(শূন্য)
{
    স্থির পূর্ণাঙ্ক prio_halt = -2;

    যদি (prio_halt == -2)
        prio_halt = find_prio("HALT");
    প্রত্যাগম prio_halt >= 0;
}

পূর্ণাঙ্ক bootprio_find_pci_device(সংরচনা pci_device *pci)
{
    যদি (CONFIG_CSM)
        প্রত্যাগম csm_bootprio_pci(pci);
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    বর্ণ desc[256];
    build_pci_path(desc, পরিমাণ(desc), "*", pci);
    প্রত্যাগম find_prio(desc);
}

পূর্ণাঙ্ক bootprio_find_mmio_device(শূন্য *mmio)
{
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    বর্ণ desc[256];
    snprintf(desc, পরিমাণ(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    প্রত্যাগম find_prio(desc);
}

পূর্ণাঙ্ক bootprio_find_scsi_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক target, পূর্ণাঙ্ক lun)
{
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    যদি (!pci)
        // support only pci machine for now
        প্রত্যাগম -1;
    বর্ণ desc[256];
    build_scsi_path(desc, পরিমাণ(desc), pci, target, lun);
    প্রত্যাগম find_prio(desc);
}

পূর্ণাঙ্ক bootprio_find_scsi_mmio_device(শূন্য *mmio, পূর্ণাঙ্ক target, পূর্ণাঙ্ক lun)
{
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    বর্ণ desc[256];
    snprintf(desc, পরিমাণ(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    প্রত্যাগম find_prio(desc);
}

পূর্ণাঙ্ক bootprio_find_ata_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক chanid, পূর্ণাঙ্ক slave)
{
    যদি (CONFIG_CSM)
        প্রত্যাগম csm_bootprio_ata(pci, chanid, slave);
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    যদি (!pci)
        // support only pci machine for now
        প্রত্যাগম -1;
    বর্ণ desc[256];
    build_ata_path(desc, পরিমাণ(desc), pci, chanid, slave);
    প্রত্যাগম find_prio(desc);
}

পূর্ণাঙ্ক bootprio_find_fdc_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক port, পূর্ণাঙ্ক fdid)
{
    যদি (CONFIG_CSM)
        প্রত্যাগম csm_bootprio_fdc(pci, port, fdid);
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    যদি (!pci)
        // support only pci machine for now
        প্রত্যাগম -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    বর্ণ desc[256], *p;
    p = build_pci_path(desc, পরিমাণ(desc), "isa", pci);
    snprintf(p, desc+পরিমাণ(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    প্রত্যাগম find_prio(desc);
}

পূর্ণাঙ্ক bootprio_find_pci_rom(সংরচনা pci_device *pci, পূর্ণাঙ্ক instance)
{
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    বর্ণ desc[256], *p;
    p = build_pci_path(desc, পরিমাণ(desc), "*", pci);
    যদি (instance)
        snprintf(p, desc+পরিমাণ(desc)-p, ":rom%x", instance);
    প্রত্যাগম find_prio(desc);
}

পূর্ণাঙ্ক bootprio_find_named_rom(ধ্রুব বর্ণ *name, পূর্ণাঙ্ক instance)
{
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    বর্ণ desc[256], *p;
    p = desc + snprintf(desc, পরিমাণ(desc), "/rom@%s", name);
    যদি (instance)
        snprintf(p, desc+পরিমাণ(desc)-p, ":rom%x", instance);
    প্রত্যাগম find_prio(desc);
}

স্থির পূর্ণাঙ্ক usb_portmap(সংরচনা usbdevice_s *usbdev)
{
    যদি (usbdev->hub->op->portmap)
        প্রত্যাগম usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    প্রত্যাগম usbdev->port + 1;
}

স্থির বর্ণ *
build_usb_path(বর্ণ *buf, পূর্ণাঙ্ক max, সংরচনা usbhub_s *hub)
{
    যদি (!hub->usbdev)
        // Root hub - nothing to add.
        প্রত্যাগম buf;
    বর্ণ *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    প্রত্যাগম p;
}

পূর্ণাঙ্ক bootprio_find_usb(সংরচনা usbdevice_s *usbdev, পূর্ণাঙ্ক lun)
{
    যদি (!CONFIG_BOOTORDER)
        প্রত্যাগম -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    বর্ণ desc[256], *p;

    যদি (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, পরিমাণ(desc), "usb", usbdev->hub->cntl->pci);
    অন্যথা যদি (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, পরিমাণ(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    অন্যথা
        প্রত্যাগম -1;

    p = build_usb_path(p, desc+পরিমাণ(desc)-p, usbdev->hub);
    snprintf(p, desc+পরিমাণ(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    পূর্ণাঙ্ক ret = find_prio(desc);
    যদি (ret >= 0)
        প্রত্যাগম ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+পরিমাণ(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    প্রত্যাগম find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

স্থির পূর্ণাঙ্ক BootRetryTime;
স্থির পূর্ণাঙ্ক CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

স্থির পূর্ণাঙ্ক DefaultFloppyPrio = 101;
স্থির পূর্ণাঙ্ক DefaultCDPrio     = 102;
স্থির পূর্ণাঙ্ক DefaultHDPrio     = 103;
স্থির পূর্ণাঙ্ক DefaultBEVPrio    = 104;

শূন্য
boot_init(শূন্য)
{
    যদি (! CONFIG_BOOT)
        প্রত্যাগম;

    যদি (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        যদি (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        পূর্ণাঙ্ক i;
        প্রতি (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            বিকল্প (val) {
            পক্ষ 1: DefaultFloppyPrio = i; ভঙ্গ;
            পক্ষ 2: DefaultHDPrio = i;     ভঙ্গ;
            পক্ষ 3: DefaultCDPrio = i;     ভঙ্গ;
            পক্ষ 4: DefaultBEVPrio = i;    ভঙ্গ;
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

সংরচনা bootentry_s {
    পূর্ণাঙ্ক type;
    সঙ্ঘ {
        u32 data;
        সংরচনা segoff_s vector;
        সংরচনা drive_s *drive;
    };
    পূর্ণাঙ্ক priority;
    ধ্রুব বর্ণ *description;
    সংরচনা hlist_গ্রন্থি গ্রন্থি;
};
স্থির সংরচনা hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

স্থির শূন্য
bootentry_add(পূর্ণাঙ্ক type, পূর্ণাঙ্ক prio, u32 data, ধ্রুব বর্ণ *desc)
{
    যদি (! CONFIG_BOOT)
        প্রত্যাগম;
    সংরচনা bootentry_s *be = malloc_tmp(পরিমাণ(*be));
    যদি (!be) {
        warn_noalloc();
        প্রত্যাগম;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    সংরচনা hlist_গ্রন্থি **pprev;
    সংরচনা bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, গ্রন্থি) {
        যদি (be->priority < pos->priority)
            ভঙ্গ;
        যদি (be->priority > pos->priority)
            অনুবর্ত;
        যদি (be->type < pos->type)
            ভঙ্গ;
        যদি (be->type > pos->type)
            অনুবর্ত;
        যদি (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            ভঙ্গ;
    }
    hlist_add(&be->গ্রন্থি, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
স্থির অন্তঃস্থ পূর্ণাঙ্ক defPrio(পূর্ণাঙ্ক priority, পূর্ণাঙ্ক defaultprio) {
    প্রত্যাগম (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
শূন্য
boot_add_bev(u16 seg, u16 bev, u16 desc, পূর্ণাঙ্ক prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
শূন্য
boot_add_bcv(u16 seg, u16 ip, u16 desc, পূর্ণাঙ্ক prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

শূন্য
boot_add_floppy(সংরচনা drive_s *drive, ধ্রুব বর্ণ *desc, পূর্ণাঙ্ক prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

শূন্য
boot_add_hd(সংরচনা drive_s *drive, ধ্রুব বর্ণ *desc, পূর্ণাঙ্ক prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

শূন্য
boot_add_cd(সংরচনা drive_s *drive, ধ্রুব বর্ণ *desc, পূর্ণাঙ্ক prio)
{
    যদি (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        বর্ণ *extra = cdrom_media_info(drive);
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
boot_add_cbfs(শূন্য *data, ধ্রুব বর্ণ *desc, পূর্ণাঙ্ক prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
স্থির পূর্ণাঙ্ক
check_for_keystroke(শূন্য)
{
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    প্রত্যাগম !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
স্থির পূর্ণাঙ্ক
get_raw_keystroke(শূন্য)
{
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    প্রত্যাগম br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
পূর্ণাঙ্ক
get_keystroke_full(পূর্ণাঙ্ক msec)
{
    u32 end = irqtimer_calc(msec);
    প্রতি (;;) {
        যদি (check_for_keystroke())
            প্রত্যাগম get_raw_keystroke();
        যদি (irqtimer_check(end))
            প্রত্যাগম -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
পূর্ণাঙ্ক
get_keystroke(পূর্ণাঙ্ক msec)
{
    পূর্ণাঙ্ক keystroke = get_keystroke_full(msec);

    যদি (keystroke < 0)
        প্রত্যাগম keystroke;
    প্রত্যাগম keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

স্থির ধ্রুব বর্ণ menuchars[] = {
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
        প্রত্যাগম;
    পূর্ণাঙ্ক show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    যদি (!show_boot_menu)
        প্রত্যাগম;

    // skip menu if only one boot device and no TPM
    যদি (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        প্রত্যাগম;
    }

    পূর্ণাঙ্ক menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    পূর্ণাঙ্ক menukey = romfile_loadint("etc/boot-menu-key", 1);
    পূর্ণাঙ্ক scan_code;
    যদি (menutime >= 0) {
        যাবত্ (get_keystroke(0) >= 0)
            ;

        বর্ণ *bootmsg = romfile_loadfile("etc/boot-menu-message", রিক্ত);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        মুক্ত(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        যদি (scan_code != menukey)
            প্রত্যাগম;
    }

    যাবত্ (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    পূর্ণাঙ্ক maxmenu = 0;
    সংরচনা bootentry_s *pos, *boot = রিক্ত;
    hlist_for_each_entry(pos, &BootList, গ্রন্থি) {
        বর্ণ desc[77];
        যদি (maxmenu >= ARRAY_SIZE(menuchars)) {
            ভঙ্গ;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    যদি (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    পূর্ণাঙ্ক esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    প্রতি (;;) {
        পূর্ণাঙ্ক keystroke = get_keystroke_full(1000);
        যদি (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            অনুবর্ত;
        যদি (keystroke < 0) // timeout
            অনুবর্ত;

        scan_code = keystroke >> 8;
        পূর্ণাঙ্ক চাবি_ascii = keystroke & 0xff;
        যদি (tpm_can_show_menu() && চাবি_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        যদি (scan_code == 1) {
            // ESC
            printf("\n");
            প্রত্যাগম;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, গ্রন্থি) {
            যদি (maxmenu >= ARRAY_SIZE(menuchars))
                ভঙ্গ;
            যদি (চাবি_ascii == menuchars[maxmenu]) {
                boot = pos;
                ভঙ্গ;
            }
            maxmenu++;
        }
        যদি (boot)
            ভঙ্গ;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->গ্রন্থি);
    boot->priority = 0;
    hlist_add_head(&boot->গ্রন্থি, &BootList);
}

// BEV (Boot Execution Vector) list
সংরচনা bev_s {
    পূর্ণাঙ্ক type;
    u32 vector;
};
স্থির সংরচনা bev_s BEV[20];
স্থির পূর্ণাঙ্ক BEVCount;
স্থির পূর্ণাঙ্ক HaveHDBoot, HaveFDBoot;

স্থির শূন্য
add_bev(পূর্ণাঙ্ক type, u32 vector)
{
    যদি (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        প্রত্যাগম;
    যদি (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        প্রত্যাগম;
    যদি (BEVCount >= ARRAY_SIZE(BEV))
        প্রত্যাগম;
    সংরচনা bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
শূন্য
bcv_prepboot(শূন্য)
{
    যদি (! CONFIG_BOOT)
        প্রত্যাগম;

    পূর্ণাঙ্ক haltprio = find_prio("HALT");
    যদি (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    সংরচনা bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, গ্রন্থি) {
        বিকল্প (pos->type) {
        পক্ষ IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            ভঙ্গ;
        পক্ষ IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            ভঙ্গ;
        পক্ষ IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            ভঙ্গ;
        পক্ষ IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        স্বভাব:
            add_bev(pos->type, pos->data);
            ভঙ্গ;
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
call_boot_entry(সংরচনা segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
স্থির শূন্য
boot_disk(u8 bootdrv, পূর্ণাঙ্ক checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    যদি (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        প্রত্যাগম;
    }

    যদি (checksig) {
        সংরচনা mbr_s *mbr = (শূন্য*)0;
        যদি (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            প্রত্যাগম;
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
boot_cdrom(সংরচনা drive_s *drive)
{
    যদি (! CONFIG_CDROM_BOOT)
        প্রত্যাগম;
    printf("Booting from DVD/CD...\n");

    পূর্ণাঙ্ক status = cdrom_boot(drive);
    যদি (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        প্রত্যাগম;
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
boot_cbfs(সংরচনা cbfs_file *file)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        প্রত্যাগম;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
স্থির শূন্য
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    সংরচনা segoff_s so;
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
    প্রতি (;;) {
        যদি (BootRetryTime != (u32)-1 && irqtimer_check(end))
            ভঙ্গ;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
স্থির শূন্য
do_boot(পূর্ণাঙ্ক seq_nr)
{
    যদি (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    যদি (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    সংরচনা bev_s *ie = &BEV[seq_nr];
    বিকল্প (ie->type) {
    পক্ষ IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        ভঙ্গ;
    পক্ষ IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        ভঙ্গ;
    পক্ষ IPL_TYPE_CDROM:
        boot_cdrom((শূন্য*)ie->vector);
        ভঙ্গ;
    পক্ষ IPL_TYPE_CBFS:
        boot_cbfs((শূন্য*)ie->vector);
        ভঙ্গ;
    পক্ষ IPL_TYPE_BEV:
        boot_rom(ie->vector);
        ভঙ্গ;
    পক্ষ IPL_TYPE_HALT:
        boot_fail();
        ভঙ্গ;
    }

    // Boot failed: invoke the boot recovery function
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

পূর্ণাঙ্ক BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
শূন্য VISIBLE32FLAT
handle_18(শূন্য)
{
    debug_enter(রিক্ত, DEBUG_HDL_18);
    পূর্ণাঙ্ক seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
শূন্য VISIBLE32FLAT
handle_19(শূন্য)
{
    debug_enter(রিক্ত, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
