// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // struct drive_s
#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "fw/paravirt.h" // qemu_cfg_show_boot_menu
#شامل "hw/pci.h" // pci_bdf_to_*
#شامل "hw/pcidevice.h" // struct pci_device
#شامل "hw/rtc.h" // rtc_read
#شامل "hw/usb.h" // struct usbdevice_s
#شامل "list.h" // hlist_node
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadint
#شامل "std/disk.h" // struct mbr_s
#شامل "string.h" // memset
#شامل "util.h" // irqtimer_calc
#شامل "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
ایستا نویسه *
glob_prefix(ثابت نویسه *glob, ثابت نویسه *str)
{
    برای (;;) {
        اگر (!*glob && (!*str || *str == '/'))
            بازگشت (نویسه*)str;
        اگر (*glob == '*') {
            اگر (!*str || *str == '/' || *str == glob[1])
                glob++;
            وگرنه
                str++;
            ادامه;
        }
        اگر (*glob != *str)
            بازگشت پوچ;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

ایستا نویسه *
build_pci_path(نویسه *buf, صحیح max, ثابت نویسه *devname, ساختار pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    نویسه *p = buf;
    اگر (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } وگرنه {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        اگر (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    صحیح dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    اگر (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    بازگشت p;
}

ایستا نویسه *
build_scsi_path(نویسه *buf, صحیح max,
                ساختار pci_device *pci, صحیح target, صحیح lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    نویسه *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    بازگشت p;
}

ایستا نویسه *
build_ata_path(نویسه *buf, صحیح max,
               ساختار pci_device *pci, صحیح chanid, صحیح slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    نویسه *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    بازگشت p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

تعریفنوع ساختار BootDeviceLCHS {
    نویسه *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

ایستا BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
ایستا صحیح Bios_Geometry_شمار;

ایستا نویسه *
parse_u32(نویسه *cur, u32 *n)
{
    u32 m = 0;
    اگر (cur) {
        وقتی ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        اگر (*cur != '\0')
            cur++;
    }
    *n = m;
    بازگشت cur;
}

ایستا تهی
loadBiosGeometry(تهی)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        بازگشت;
    نویسه *f = romfile_loadfile("bios-geometry", پوچ);
    اگر (!f)
        بازگشت;

    صحیح i = 0;
    Bios_Geometry_شمار = 1;
    وقتی (f[i]) {
        اگر (f[i] == '\n')
            Bios_Geometry_شمار++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_شمار * اندازه(BootDeviceLCHS));
    اگر (!BiosGeometry) {
        warn_noalloc();
        آزاد(f);
        Bios_Geometry_شمار = 0;
        بازگشت;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    انجامبده {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        اگر (f)
            *(f++) = '\0';
        نویسه *chs_مقادیر = strchr(d->name, ' ');
        اگر (chs_مقادیر)
            *(chs_مقادیر++) = '\0';
        chs_مقادیر = parse_u32(chs_مقادیر, &d->lcyls);
        chs_مقادیر = parse_u32(chs_مقادیر, &d->lheads);
        chs_مقادیر = parse_u32(chs_مقادیر, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } وقتی (f);
}

// Search the bios-geometry list for the given glob pattern.
ایستا BootDeviceLCHS *
boot_lchs_find(ثابت نویسه *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    صحیح i;
    برای (i = 0; i < Bios_Geometry_شمار; i++)
        اگر (glob_prefix(glob, BiosGeometry[i].name))
            بازگشت &BiosGeometry[i];
    بازگشت پوچ;
}

صحیح boot_lchs_find_pci_device(ساختار pci_device *pci, ساختار chs_s *chs)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        بازگشت -1;
    نویسه desc[256];
    build_pci_path(desc, اندازه(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    اگر (!b)
        بازگشت -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    بازگشت 0;
}

صحیح boot_lchs_find_scsi_device(ساختار pci_device *pci, صحیح target, صحیح lun,
                               ساختار chs_s *chs)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        بازگشت -1;
    اگر (!pci)
        // support only pci machine for now
        بازگشت -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    نویسه desc[256];
    build_scsi_path(desc, اندازه(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    اگر (!b)
        بازگشت -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    بازگشت 0;
}

صحیح boot_lchs_find_ata_device(ساختار pci_device *pci, صحیح chanid, صحیح slave,
                              ساختار chs_s *chs)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        بازگشت -1;
    اگر (!pci)
        // support only pci machine for now
        بازگشت -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    نویسه desc[256];
    build_ata_path(desc, اندازه(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    اگر (!b)
        بازگشت -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    بازگشت 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

ایستا نویسه **Bootorder VARVERIFY32INIT;
ایستا صحیح Bootorder_شمار;

ایستا تهی
loadBootOrder(تهی)
{
    اگر (!CONFIG_BOOTORDER)
        بازگشت;

    نویسه *f = romfile_loadfile("bootorder", پوچ);
    اگر (!f)
        بازگشت;

    صحیح i = 0;
    Bootorder_شمار = 1;
    وقتی (f[i]) {
        اگر (f[i] == '\n')
            Bootorder_شمار++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_شمار*اندازه(نویسه*));
    اگر (!Bootorder) {
        warn_noalloc();
        آزاد(f);
        Bootorder_شمار = 0;
        بازگشت;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    انجامبده {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        اگر (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } وقتی (f);
}

// Search the bootorder list for the given glob pattern.
ایستا صحیح
find_prio(ثابت نویسه *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    صحیح i;
    برای (i = 0; i < Bootorder_شمار; i++)
        اگر (glob_prefix(glob, Bootorder[i]))
            بازگشت i+1;
    بازگشت -1;
}

u8 is_bootprio_strict(تهی)
{
    ایستا صحیح prio_halt = -2;

    اگر (prio_halt == -2)
        prio_halt = find_prio("HALT");
    بازگشت prio_halt >= 0;
}

صحیح bootprio_find_pci_device(ساختار pci_device *pci)
{
    اگر (CONFIG_CSM)
        بازگشت csm_bootprio_pci(pci);
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    نویسه desc[256];
    build_pci_path(desc, اندازه(desc), "*", pci);
    بازگشت find_prio(desc);
}

صحیح bootprio_find_mmio_device(تهی *mmio)
{
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    نویسه desc[256];
    snprintf(desc, اندازه(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    بازگشت find_prio(desc);
}

صحیح bootprio_find_scsi_device(ساختار pci_device *pci, صحیح target, صحیح lun)
{
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    اگر (!pci)
        // support only pci machine for now
        بازگشت -1;
    نویسه desc[256];
    build_scsi_path(desc, اندازه(desc), pci, target, lun);
    بازگشت find_prio(desc);
}

صحیح bootprio_find_scsi_mmio_device(تهی *mmio, صحیح target, صحیح lun)
{
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    نویسه desc[256];
    snprintf(desc, اندازه(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    بازگشت find_prio(desc);
}

صحیح bootprio_find_ata_device(ساختار pci_device *pci, صحیح chanid, صحیح slave)
{
    اگر (CONFIG_CSM)
        بازگشت csm_bootprio_ata(pci, chanid, slave);
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    اگر (!pci)
        // support only pci machine for now
        بازگشت -1;
    نویسه desc[256];
    build_ata_path(desc, اندازه(desc), pci, chanid, slave);
    بازگشت find_prio(desc);
}

صحیح bootprio_find_fdc_device(ساختار pci_device *pci, صحیح port, صحیح fdid)
{
    اگر (CONFIG_CSM)
        بازگشت csm_bootprio_fdc(pci, port, fdid);
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    اگر (!pci)
        // support only pci machine for now
        بازگشت -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    نویسه desc[256], *p;
    p = build_pci_path(desc, اندازه(desc), "isa", pci);
    snprintf(p, desc+اندازه(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    بازگشت find_prio(desc);
}

صحیح bootprio_find_pci_rom(ساختار pci_device *pci, صحیح instance)
{
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    نویسه desc[256], *p;
    p = build_pci_path(desc, اندازه(desc), "*", pci);
    اگر (instance)
        snprintf(p, desc+اندازه(desc)-p, ":rom%x", instance);
    بازگشت find_prio(desc);
}

صحیح bootprio_find_named_rom(ثابت نویسه *name, صحیح instance)
{
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    نویسه desc[256], *p;
    p = desc + snprintf(desc, اندازه(desc), "/rom@%s", name);
    اگر (instance)
        snprintf(p, desc+اندازه(desc)-p, ":rom%x", instance);
    بازگشت find_prio(desc);
}

ایستا صحیح usb_portmap(ساختار usbdevice_s *usbdev)
{
    اگر (usbdev->hub->op->portmap)
        بازگشت usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    بازگشت usbdev->port + 1;
}

ایستا نویسه *
build_usb_path(نویسه *buf, صحیح max, ساختار usbhub_s *hub)
{
    اگر (!hub->usbdev)
        // Root hub - nothing to add.
        بازگشت buf;
    نویسه *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    بازگشت p;
}

صحیح bootprio_find_usb(ساختار usbdevice_s *usbdev, صحیح lun)
{
    اگر (!CONFIG_BOOTORDER)
        بازگشت -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    نویسه desc[256], *p;

    اگر (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, اندازه(desc), "usb", usbdev->hub->cntl->pci);
    وگرنه اگر (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, اندازه(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    وگرنه
        بازگشت -1;

    p = build_usb_path(p, desc+اندازه(desc)-p, usbdev->hub);
    snprintf(p, desc+اندازه(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    صحیح ret = find_prio(desc);
    اگر (ret >= 0)
        بازگشت ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+اندازه(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    بازگشت find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

ایستا صحیح BootRetryTime;
ایستا صحیح CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

ایستا صحیح DefaultFloppyPrio = 101;
ایستا صحیح DefaultCDPrio     = 102;
ایستا صحیح DefaultHDPrio     = 103;
ایستا صحیح DefaultBEVPrio    = 104;

تهی
boot_init(تهی)
{
    اگر (! CONFIG_BOOT)
        بازگشت;

    اگر (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        اگر (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        صحیح i;
        برای (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            انتخاب (val) {
            حالت 1: DefaultFloppyPrio = i; بشکن;
            حالت 2: DefaultHDPrio = i;     بشکن;
            حالت 3: DefaultCDPrio = i;     بشکن;
            حالت 4: DefaultBEVPrio = i;    بشکن;
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

ساختار bootentry_s {
    صحیح type;
    اجتماع {
        u32 data;
        ساختار segoff_s vector;
        ساختار drive_s *drive;
    };
    صحیح priority;
    ثابت نویسه *description;
    ساختار hlist_گره گره;
};
ایستا ساختار hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

ایستا تهی
bootentry_add(صحیح type, صحیح prio, u32 data, ثابت نویسه *desc)
{
    اگر (! CONFIG_BOOT)
        بازگشت;
    ساختار bootentry_s *be = malloc_tmp(اندازه(*be));
    اگر (!be) {
        warn_noalloc();
        بازگشت;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    ساختار hlist_گره **pprev;
    ساختار bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, گره) {
        اگر (be->priority < pos->priority)
            بشکن;
        اگر (be->priority > pos->priority)
            ادامه;
        اگر (be->type < pos->type)
            بشکن;
        اگر (be->type > pos->type)
            ادامه;
        اگر (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            بشکن;
    }
    hlist_add(&be->گره, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
ایستا درونخطی صحیح defPrio(صحیح priority, صحیح defaultprio) {
    بازگشت (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
تهی
boot_add_bev(u16 seg, u16 bev, u16 desc, صحیح prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
تهی
boot_add_bcv(u16 seg, u16 ip, u16 desc, صحیح prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

تهی
boot_add_floppy(ساختار drive_s *drive, ثابت نویسه *desc, صحیح prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

تهی
boot_add_hd(ساختار drive_s *drive, ثابت نویسه *desc, صحیح prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

تهی
boot_add_cd(ساختار drive_s *drive, ثابت نویسه *desc, صحیح prio)
{
    اگر (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        نویسه *extra = cdrom_media_info(drive);
        اگر (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            آزاد(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
تهی
boot_add_cbfs(تهی *data, ثابت نویسه *desc, صحیح prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
ایستا صحیح
check_for_keystroke(تهی)
{
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    بازگشت !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
ایستا صحیح
get_raw_keystroke(تهی)
{
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    بازگشت br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
صحیح
get_keystroke_full(صحیح msec)
{
    u32 end = irqtimer_calc(msec);
    برای (;;) {
        اگر (check_for_keystroke())
            بازگشت get_raw_keystroke();
        اگر (irqtimer_check(end))
            بازگشت -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
صحیح
get_keystroke(صحیح msec)
{
    صحیح keystroke = get_keystroke_full(msec);

    اگر (keystroke < 0)
        بازگشت keystroke;
    بازگشت keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

ایستا ثابت نویسه menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
تهی
interactive_bootmenu(تهی)
{
    اگر (! CONFIG_BOOTMENU)
        بازگشت;
    صحیح show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    اگر (!show_boot_menu)
        بازگشت;

    // skip menu if only one boot device and no TPM
    اگر (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        بازگشت;
    }

    صحیح menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    صحیح menukey = romfile_loadint("etc/boot-menu-key", 1);
    صحیح scan_code;
    اگر (menutime >= 0) {
        وقتی (get_keystroke(0) >= 0)
            ;

        نویسه *bootmsg = romfile_loadfile("etc/boot-menu-message", پوچ);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        آزاد(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        اگر (scan_code != menukey)
            بازگشت;
    }

    وقتی (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    صحیح maxmenu = 0;
    ساختار bootentry_s *pos, *boot = پوچ;
    hlist_for_each_entry(pos, &BootList, گره) {
        نویسه desc[77];
        اگر (maxmenu >= ARRAY_SIZE(menuchars)) {
            بشکن;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    اگر (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    صحیح esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    برای (;;) {
        صحیح keystroke = get_keystroke_full(1000);
        اگر (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            ادامه;
        اگر (keystroke < 0) // timeout
            ادامه;

        scan_code = keystroke >> 8;
        صحیح کلید_ascii = keystroke & 0xff;
        اگر (tpm_can_show_menu() && کلید_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        اگر (scan_code == 1) {
            // ESC
            printf("\n");
            بازگشت;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, گره) {
            اگر (maxmenu >= ARRAY_SIZE(menuchars))
                بشکن;
            اگر (کلید_ascii == menuchars[maxmenu]) {
                boot = pos;
                بشکن;
            }
            maxmenu++;
        }
        اگر (boot)
            بشکن;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->گره);
    boot->priority = 0;
    hlist_add_head(&boot->گره, &BootList);
}

// BEV (Boot Execution Vector) list
ساختار bev_s {
    صحیح type;
    u32 vector;
};
ایستا ساختار bev_s BEV[20];
ایستا صحیح BEVCount;
ایستا صحیح HaveHDBoot, HaveFDBoot;

ایستا تهی
add_bev(صحیح type, u32 vector)
{
    اگر (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        بازگشت;
    اگر (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        بازگشت;
    اگر (BEVCount >= ARRAY_SIZE(BEV))
        بازگشت;
    ساختار bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
تهی
bcv_prepboot(تهی)
{
    اگر (! CONFIG_BOOT)
        بازگشت;

    صحیح haltprio = find_prio("HALT");
    اگر (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    ساختار bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, گره) {
        انتخاب (pos->type) {
        حالت IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            بشکن;
        حالت IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            بشکن;
        حالت IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            بشکن;
        حالت IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        پیشفرض:
            add_bev(pos->type, pos->data);
            بشکن;
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
ایستا تهی
call_boot_entry(ساختار segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
ایستا تهی
boot_disk(u8 bootdrv, صحیح checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    اگر (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        بازگشت;
    }

    اگر (checksig) {
        ساختار mbr_s *mbr = (تهی*)0;
        اگر (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            بازگشت;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
ایستا تهی
boot_cdrom(ساختار drive_s *drive)
{
    اگر (! CONFIG_CDROM_BOOT)
        بازگشت;
    printf("Booting from DVD/CD...\n");

    صحیح status = cdrom_boot(drive);
    اگر (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        بازگشت;
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
ایستا تهی
boot_cbfs(ساختار cbfs_file *file)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        بازگشت;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
ایستا تهی
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    ساختار segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
ایستا تهی
boot_fail(تهی)
{
    اگر (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    وگرنه
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    برای (;;) {
        اگر (BootRetryTime != (u32)-1 && irqtimer_check(end))
            بشکن;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
ایستا تهی
do_boot(صحیح seq_nr)
{
    اگر (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    اگر (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    ساختار bev_s *ie = &BEV[seq_nr];
    انتخاب (ie->type) {
    حالت IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        بشکن;
    حالت IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        بشکن;
    حالت IPL_TYPE_CDROM:
        boot_cdrom((تهی*)ie->vector);
        بشکن;
    حالت IPL_TYPE_CBFS:
        boot_cbfs((تهی*)ie->vector);
        بشکن;
    حالت IPL_TYPE_BEV:
        boot_rom(ie->vector);
        بشکن;
    حالت IPL_TYPE_HALT:
        boot_fail();
        بشکن;
    }

    // Boot failed: invoke the boot recovery function
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

صحیح BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
تهی VISIBLE32FLAT
handle_18(تهی)
{
    debug_enter(پوچ, DEBUG_HDL_18);
    صحیح seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
تهی VISIBLE32FLAT
handle_19(تهی)
{
    debug_enter(پوچ, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
