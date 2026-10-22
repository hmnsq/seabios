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
ساکن حرف *
glob_سابقہ(ثابت حرف *glob, ثابت حرف *str)
{
    برائے (;;) {
        اگر (!*glob && (!*str || *str == '/'))
            واپس (حرف*)str;
        اگر (*glob == '*') {
            اگر (!*str || *str == '/' || *str == glob[1])
                glob++;
            ورنہ
                str++;
            جاری;
        }
        اگر (*glob != *str)
            واپس تہی;
        glob++;
        str++;
    }
}

#تعریف_کرو FW_PCI_DOMAIN "/pci@i0cf8"

ساکن حرف *
build_pci_راستہ(حرف *buf, صحیح max, ثابت حرف *devname, ساخت pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    حرف *p = buf;
    اگر (pci->والد) {
        p = build_pci_راستہ(p, max, "pci-bridge", pci->والد);
    } ورنہ {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        اگر (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    صحیح dev = pci_bdf_مقصد_dev(pci->bdf), fn = pci_bdf_مقصد_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    اگر (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    واپس p;
}

ساکن حرف *
build_scsi_راستہ(حرف *buf, صحیح max,
                ساخت pci_device *pci, صحیح ہدف, صحیح lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    حرف *p;
    p = build_pci_راستہ(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", ہدف, lun);
    واپس p;
}

ساکن حرف *
build_ata_راستہ(حرف *buf, صحیح max,
               ساخت pci_device *pci, صحیح chanid, صحیح slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    حرف *p;
    p = build_pci_راستہ(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    واپس p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

قسمتعریف ساخت BootDeviceLCHS {
    حرف *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

ساکن BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
ساکن صحیح Bios_Geometry_گنتی;

ساکن حرف *
parse_u32(حرف *cur, u32 *n)
{
    u32 m = 0;
    اگر (cur) {
        جب ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        اگر (*cur != '\0')
            cur++;
    }
    *n = m;
    واپس cur;
}

ساکن خالی
بھرو_Bios_Geometry(خالی)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        واپس;
    حرف *f = romfile_loadfile("bios-geometry", تہی);
    اگر (!f)
        واپس;

    صحیح i = 0;
    Bios_Geometry_گنتی = 1;
    جب (f[i]) {
        اگر (f[i] == '\n')
            Bios_Geometry_گنتی++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_گنتی * حجم(BootDeviceLCHS));
    اگر (!BiosGeometry) {
        warn_noalloc();
        آزاد(f);
        Bios_Geometry_گنتی = 0;
        واپس;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    کرو {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        اگر (f)
            *(f++) = '\0';
        حرف *chs_اقدار = strchr(d->name, ' ');
        اگر (chs_اقدار)
            *(chs_اقدار++) = '\0';
        chs_اقدار = parse_u32(chs_اقدار, &d->lcyls);
        chs_اقدار = parse_u32(chs_اقدار, &d->lheads);
        chs_اقدار = parse_u32(chs_اقدار, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } جب (f);
}

// Search the bios-geometry list for the given glob pattern.
ساکن BootDeviceLCHS *
boot_lchs_ڈھونڈو(ثابت حرف *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    صحیح i;
    برائے (i = 0; i < Bios_Geometry_گنتی; i++)
        اگر (glob_سابقہ(glob, BiosGeometry[i].name))
            واپس &BiosGeometry[i];
    واپس تہی;
}

صحیح boot_lchs_ڈھونڈو_pci_device(ساخت pci_device *pci, ساخت chs_s *chs)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        واپس -1;
    حرف desc[256];
    build_pci_راستہ(desc, حجم(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_ڈھونڈو(desc);
    اگر (!b)
        واپس -1;
    chs->cylinder = (u16)b->lcyls;
    chs->سر = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    واپس 0;
}

صحیح boot_lchs_ڈھونڈو_scsi_device(ساخت pci_device *pci, صحیح ہدف, صحیح lun,
                               ساخت chs_s *chs)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        واپس -1;
    اگر (!pci)
        // support only pci machine for now
        واپس -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    حرف desc[256];
    build_scsi_راستہ(desc, حجم(desc), pci, ہدف, lun);
    BootDeviceLCHS *b = boot_lchs_ڈھونڈو(desc);
    اگر (!b)
        واپس -1;
    chs->cylinder = (u16)b->lcyls;
    chs->سر = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    واپس 0;
}

صحیح boot_lchs_ڈھونڈو_ata_device(ساخت pci_device *pci, صحیح chanid, صحیح slave,
                              ساخت chs_s *chs)
{
    اگر (!CONFIG_HOST_BIOS_GEOMETRY)
        واپس -1;
    اگر (!pci)
        // support only pci machine for now
        واپس -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    حرف desc[256];
    build_ata_راستہ(desc, حجم(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_ڈھونڈو(desc);
    اگر (!b)
        واپس -1;
    chs->cylinder = (u16)b->lcyls;
    chs->سر = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    واپس 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

ساکن حرف **Bootorder VARVERIFY32INIT;
ساکن صحیح Bootorder_گنتی;

ساکن خالی
بھرو_Boot_ترتیب(خالی)
{
    اگر (!CONFIG_BOOTORDER)
        واپس;

    حرف *f = romfile_loadfile("bootorder", تہی);
    اگر (!f)
        واپس;

    صحیح i = 0;
    Bootorder_گنتی = 1;
    جب (f[i]) {
        اگر (f[i] == '\n')
            Bootorder_گنتی++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_گنتی*حجم(حرف*));
    اگر (!Bootorder) {
        warn_noalloc();
        آزاد(f);
        Bootorder_گنتی = 0;
        واپس;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    کرو {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        اگر (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } جب (f);
}

// Search the bootorder list for the given glob pattern.
ساکن صحیح
ڈھونڈو_prio(ثابت حرف *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    صحیح i;
    برائے (i = 0; i < Bootorder_گنتی; i++)
        اگر (glob_سابقہ(glob, Bootorder[i]))
            واپس i+1;
    واپس -1;
}

u8 is_bootprio_strict(خالی)
{
    ساکن صحیح prio_halt = -2;

    اگر (prio_halt == -2)
        prio_halt = ڈھونڈو_prio("HALT");
    واپس prio_halt >= 0;
}

صحیح bootprio_ڈھونڈو_pci_device(ساخت pci_device *pci)
{
    اگر (CONFIG_CSM)
        واپس csm_bootprio_pci(pci);
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    حرف desc[256];
    build_pci_راستہ(desc, حجم(desc), "*", pci);
    واپس ڈھونڈو_prio(desc);
}

صحیح bootprio_ڈھونڈو_mmio_device(خالی *mmio)
{
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    حرف desc[256];
    snprintf(desc, حجم(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    واپس ڈھونڈو_prio(desc);
}

صحیح bootprio_ڈھونڈو_scsi_device(ساخت pci_device *pci, صحیح ہدف, صحیح lun)
{
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    اگر (!pci)
        // support only pci machine for now
        واپس -1;
    حرف desc[256];
    build_scsi_راستہ(desc, حجم(desc), pci, ہدف, lun);
    واپس ڈھونڈو_prio(desc);
}

صحیح bootprio_ڈھونڈو_scsi_mmio_device(خالی *mmio, صحیح ہدف, صحیح lun)
{
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    حرف desc[256];
    snprintf(desc, حجم(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, ہدف, lun);
    واپس ڈھونڈو_prio(desc);
}

صحیح bootprio_ڈھونڈو_ata_device(ساخت pci_device *pci, صحیح chanid, صحیح slave)
{
    اگر (CONFIG_CSM)
        واپس csm_bootprio_ata(pci, chanid, slave);
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    اگر (!pci)
        // support only pci machine for now
        واپس -1;
    حرف desc[256];
    build_ata_راستہ(desc, حجم(desc), pci, chanid, slave);
    واپس ڈھونڈو_prio(desc);
}

صحیح bootprio_ڈھونڈو_fdc_device(ساخت pci_device *pci, صحیح پورٹ, صحیح fdid)
{
    اگر (CONFIG_CSM)
        واپس csm_bootprio_fdc(pci, پورٹ, fdid);
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    اگر (!pci)
        // support only pci machine for now
        واپس -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    حرف desc[256], *p;
    p = build_pci_راستہ(desc, حجم(desc), "isa", pci);
    snprintf(p, desc+حجم(desc)-p, "/fdc@%04x/floppy@%x", پورٹ, fdid);
    واپس ڈھونڈو_prio(desc);
}

صحیح bootprio_ڈھونڈو_pci_rom(ساخت pci_device *pci, صحیح instance)
{
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    حرف desc[256], *p;
    p = build_pci_راستہ(desc, حجم(desc), "*", pci);
    اگر (instance)
        snprintf(p, desc+حجم(desc)-p, ":rom%x", instance);
    واپس ڈھونڈو_prio(desc);
}

صحیح bootprio_ڈھونڈو_named_rom(ثابت حرف *name, صحیح instance)
{
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    حرف desc[256], *p;
    p = desc + snprintf(desc, حجم(desc), "/rom@%s", name);
    اگر (instance)
        snprintf(p, desc+حجم(desc)-p, ":rom%x", instance);
    واپس ڈھونڈو_prio(desc);
}

ساکن صحیح usb_portmap(ساخت usbdevice_s *usbdev)
{
    اگر (usbdev->hub->op->portmap)
        واپس usbdev->hub->op->portmap(usbdev->hub, usbdev->پورٹ);
    واپس usbdev->پورٹ + 1;
}

ساکن حرف *
build_usb_راستہ(حرف *buf, صحیح max, ساخت usbhub_s *hub)
{
    اگر (!hub->usbdev)
        // Root hub - nothing to add.
        واپس buf;
    حرف *p = build_usb_راستہ(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    واپس p;
}

صحیح bootprio_ڈھونڈو_usb(ساخت usbdevice_s *usbdev, صحیح lun)
{
    اگر (!CONFIG_BOOTORDER)
        واپس -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    حرف desc[256], *p;

    اگر (usbdev->hub->cntl->pci)
        p = build_pci_راستہ(desc, حجم(desc), "usb", usbdev->hub->cntl->pci);
    ورنہ اگر (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, حجم(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    ورنہ
        واپس -1;

    p = build_usb_راستہ(p, desc+حجم(desc)-p, usbdev->hub);
    snprintf(p, desc+حجم(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    صحیح ret = ڈھونڈو_prio(desc);
    اگر (ret >= 0)
        واپس ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+حجم(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    واپس ڈھونڈو_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

ساکن صحیح Boot_Retry_وقت;
ساکن صحیح CheckFloppySig = 1;

#تعریف_کرو DEFAULT_PRIO           9999

ساکن صحیح DefaultFloppyPrio = 101;
ساکن صحیح DefaultCDPrio     = 102;
ساکن صحیح DefaultHDPrio     = 103;
ساکن صحیح DefaultBEVPrio    = 104;

خالی
boot_init(خالی)
{
    اگر (! CONFIG_BOOT)
        واپس;

    اگر (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        اگر (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        صحیح i;
        برائے (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            انتخاب (val) {
            حالت 1: DefaultFloppyPrio = i; توڑو;
            حالت 2: DefaultHDPrio = i;     توڑو;
            حالت 3: DefaultCDPrio = i;     توڑو;
            حالت 4: DefaultBEVPrio = i;    توڑو;
            }
        }
    }

    Boot_Retry_وقت = romfile_loadint("etc/boot-fail-wait", 60*1000);

    بھرو_Boot_ترتیب();
    بھرو_Bios_Geometry();
}


/****************************************************************
 * BootList handling
 ****************************************************************/

ساخت bootentry_s {
    صحیح type;
    اتحاد {
        u32 ڈیٹا;
        ساخت segoff_s سمتیہ;
        ساخت drive_s *drive;
    };
    صحیح ترجیح;
    ثابت حرف *description;
    ساخت hlist_گرہ گرہ;
};
ساکن ساخت hlist_سر BootList VARVERIFY32INIT;

#تعریف_کرو IPL_TYPE_FLOPPY      0x01
#تعریف_کرو IPL_TYPE_HARDDISK    0x02
#تعریف_کرو IPL_TYPE_CDROM       0x03
#تعریف_کرو IPL_TYPE_CBFS        0x20
#تعریف_کرو IPL_TYPE_BEV         0x80
#تعریف_کرو IPL_TYPE_BCV         0x81
#تعریف_کرو IPL_TYPE_HALT        0xf0

ساکن خالی
bootentry_جوڑو(صحیح type, صحیح prio, u32 ڈیٹا, ثابت حرف *desc)
{
    اگر (! CONFIG_BOOT)
        واپس;
    ساخت bootentry_s *be = malloc_tmp(حجم(*be));
    اگر (!be) {
        warn_noalloc();
        واپس;
    }
    be->type = type;
    be->ترجیح = prio;
    be->ڈیٹا = ڈیٹا;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, ڈیٹا);

    // Add entry in sorted order.
    ساخت hlist_گرہ **pprev;
    ساخت bootentry_s *pos;
    hlist_for_each_اندراج_pprev(pos, pprev, &BootList, گرہ) {
        اگر (be->ترجیح < pos->ترجیح)
            توڑو;
        اگر (be->ترجیح > pos->ترجیح)
            جاری;
        اگر (be->type < pos->type)
            توڑو;
        اگر (be->type > pos->type)
            جاری;
        اگر (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            توڑو;
    }
    hlist_جوڑو(&be->گرہ, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
ساکن اندرونی صحیح defPrio(صحیح ترجیح, صحیح defaultprio) {
    واپس (ترجیح < 0) ? defaultprio : ترجیح;
}

// Add a BEV vector for a given pnp compatible option rom.
خالی
boot_جوڑو_bev(u16 seg, u16 bev, u16 desc, صحیح prio)
{
    bootentry_جوڑو(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
خالی
boot_جوڑو_bcv(u16 seg, u16 ip, u16 desc, صحیح prio)
{
    bootentry_جوڑو(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

خالی
boot_جوڑو_floppy(ساخت drive_s *drive, ثابت حرف *desc, صحیح prio)
{
    bootentry_جوڑو(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

خالی
boot_جوڑو_hd(ساخت drive_s *drive, ثابت حرف *desc, صحیح prio)
{
    bootentry_جوڑو(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

خالی
boot_جوڑو_cd(ساخت drive_s *drive, ثابت حرف *desc, صحیح prio)
{
    اگر (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        حرف *extra = cdrom_media_info(drive);
        اگر (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            آزاد(extra);
        }
    }
    bootentry_جوڑو(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
خالی
boot_جوڑو_cbfs(خالی *ڈیٹا, ثابت حرف *desc, صحیح prio)
{
    bootentry_جوڑو(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)ڈیٹا, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
ساکن صحیح
check_for_keystroke(خالی)
{
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    واپس !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
ساکن صحیح
get_raw_keystroke(خالی)
{
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    واپس br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
صحیح
get_keystroke_بھرا(صحیح msec)
{
    u32 انتہا = irqtimer_calc(msec);
    برائے (;;) {
        اگر (check_for_keystroke())
            واپس get_raw_keystroke();
        اگر (irqtimer_check(انتہا))
            واپس -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
صحیح
get_keystroke(صحیح msec)
{
    صحیح keystroke = get_keystroke_بھرا(msec);

    اگر (keystroke < 0)
        واپس keystroke;
    واپس keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#تعریف_کرو DEFAULT_BOOTMENU_WAIT 2500

ساکن ثابت حرف menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
خالی
interactive_bootmenu(خالی)
{
    اگر (! CONFIG_BOOTMENU)
        واپس;
    صحیح show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    اگر (!show_boot_menu)
        واپس;

    // skip menu if only one boot device and no TPM
    اگر (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_خالی_خانے(&BootList) && !BootList.پہلا->اگلا) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        لکھو("\n");
        واپس;
    }

    صحیح menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    صحیح menukey = romfile_loadint("etc/boot-menu-key", 1);
    صحیح جائزہ_رمز;
    اگر (menutime >= 0) {
        جب (get_keystroke(0) >= 0)
            ;

        حرف *bootmsg = romfile_loadfile("etc/boot-menu-message", تہی);
        لکھو("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        آزاد(bootmsg);

        enable_bootsplash();
        جائزہ_رمز = get_keystroke(menutime);
        disable_bootsplash();
        اگر (جائزہ_رمز != menukey)
            واپس;
    }

    جب (get_keystroke(0) >= 0)
        ;

    لکھو("Select boot device:\n\n");
    wait_دھاگے();

    // Show menu items
    صحیح maxmenu = 0;
    ساخت bootentry_s *pos, *boot = تہی;
    hlist_for_each_اندراج(pos, &BootList, گرہ) {
        حرف desc[77];
        اگر (maxmenu >= ARRAY_سائز(menuchars)) {
            توڑو;
        }
        لکھو("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_سائز(desc)));
        maxmenu++;
    }
    اگر (tpm_can_show_menu()) {
        لکھو("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    صحیح esc_accepted_وقت = irqtimer_calc(menukey == 1 ? 1500 : 0);
    برائے (;;) {
        صحیح keystroke = get_keystroke_بھرا(1000);
        اگر (keystroke == 0x011b && !irqtimer_check(esc_accepted_وقت))
            جاری;
        اگر (keystroke < 0) // timeout
            جاری;

        جائزہ_رمز = keystroke >> 8;
        صحیح کلید_ascii = keystroke & 0xff;
        اگر (tpm_can_show_menu() && کلید_ascii == 't') {
            لکھو("\n");
            tpm_menu();
        }
        اگر (جائزہ_رمز == 1) {
            // ESC
            لکھو("\n");
            واپس;
        }

        maxmenu = 0;
        hlist_for_each_اندراج(pos, &BootList, گرہ) {
            اگر (maxmenu >= ARRAY_سائز(menuchars))
                توڑو;
            اگر (کلید_ascii == menuchars[maxmenu]) {
                boot = pos;
                توڑو;
            }
            maxmenu++;
        }
        اگر (boot)
            توڑو;
    }
    لکھو("\n");

    // Find entry and make top priority.
    hlist_del(&boot->گرہ);
    boot->ترجیح = 0;
    hlist_جوڑو_سر(&boot->گرہ, &BootList);
}

// BEV (Boot Execution Vector) list
ساخت bev_s {
    صحیح type;
    u32 سمتیہ;
};
ساکن ساخت bev_s BEV[20];
ساکن صحیح BEVCount;
ساکن صحیح HaveHDBoot, HaveFDBoot;

ساکن خالی
جوڑو_bev(صحیح type, u32 سمتیہ)
{
    اگر (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        واپس;
    اگر (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        واپس;
    اگر (BEVCount >= ARRAY_سائز(BEV))
        واپس;
    ساخت bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->سمتیہ = سمتیہ;
}

// Prepare for boot - show menu and run bcvs.
خالی
bcv_prepboot(خالی)
{
    اگر (! CONFIG_BOOT)
        واپس;

    صحیح haltprio = ڈھونڈو_prio("HALT");
    اگر (haltprio >= 0)
        bootentry_جوڑو(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    ساخت bootentry_s *pos;
    hlist_for_each_اندراج(pos, &BootList, گرہ) {
        انتخاب (pos->type) {
        حالت IPL_TYPE_BCV:
            call_bcv(pos->سمتیہ.seg, pos->سمتیہ.انحراف_2);
            جوڑو_bev(IPL_TYPE_HARDDISK, 0);
            توڑو;
        حالت IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            جوڑو_bev(IPL_TYPE_FLOPPY, 0);
            توڑو;
        حالت IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            جوڑو_bev(IPL_TYPE_HARDDISK, 0);
            توڑو;
        حالت IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        طےشدہ:
            جوڑو_bev(pos->type, pos->ڈیٹا);
            توڑو;
        }
    }

    // If nothing added a floppy/hd boot - add it manually.
    جوڑو_bev(IPL_TYPE_FLOPPY, 0);
    جوڑو_bev(IPL_TYPE_HARDDISK, 0);
}


/****************************************************************
 * Boot code (int 18/19)
 ****************************************************************/

// Jump to a bootup entry point.
ساکن خالی
call_boot_اندراج(ساخت segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.انحراف_2);
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.رمز = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
ساکن خالی
boot_disk(u8 bootdrv, صحیح checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    اگر (br.flags & F_CF) {
        لکھو("Boot failed: could not read the boot disk\n\n");
        واپس;
    }

    اگر (checksig) {
        ساخت mbr_s *mbr = (خالی*)0;
        اگر (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            لکھو("Boot failed: not a bootable disk\n\n");
            واپس;
        }
    }

    tpm_جوڑو_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_اندراج(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
ساکن خالی
boot_cdrom(ساخت drive_s *drive)
{
    اگر (! CONFIG_CDROM_BOOT)
        واپس;
    لکھو("Booting from DVD/CD...\n");

    صحیح حالت_2 = cdrom_boot(drive);
    اگر (حالت_2) {
        لکھو("Boot failed: Could not read from CDROM (code %04x)\n", حالت_2);
        واپس;
    }

    u8 bootdrv = CDEmu.emulated_drive;
    u16 bootseg = CDEmu.بھرو_segment;

    tpm_جوڑو_cdrom(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_اندراج(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CBFS payload
ساکن خالی
boot_cbfs(ساخت cbfs_file *file)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        واپس;
    لکھو("Booting from CBFS...\n");
    cbfs_تسلسل_payload(file);
}

// Boot from a BEV entry on an optionrom.
ساکن خالی
boot_rom(u32 سمتیہ)
{
    لکھو("Booting from ROM...\n");
    ساخت segoff_s so;
    so.segoff = سمتیہ;
    call_boot_اندراج(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
ساکن خالی
boot_fail(خالی)
{
    اگر (Boot_Retry_وقت == (u32)-1)
        لکھو("No bootable device.\n");
    ورنہ
        لکھو("No bootable device.  Retrying in %d seconds.\n"
               , Boot_Retry_وقت/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 انتہا = irqtimer_calc(Boot_Retry_وقت);
    برائے (;;) {
        اگر (Boot_Retry_وقت != (u32)-1 && irqtimer_check(انتہا))
            توڑو;
        yield_toirq();
    }
    لکھو("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
ساکن خالی
do_boot(صحیح seq_nr)
{
    اگر (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    اگر (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    ساخت bev_s *ie = &BEV[seq_nr];
    انتخاب (ie->type) {
    حالت IPL_TYPE_FLOPPY:
        لکھو("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        توڑو;
    حالت IPL_TYPE_HARDDISK:
        لکھو("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        توڑو;
    حالت IPL_TYPE_CDROM:
        boot_cdrom((خالی*)ie->سمتیہ);
        توڑو;
    حالت IPL_TYPE_CBFS:
        boot_cbfs((خالی*)ie->سمتیہ);
        توڑو;
    حالت IPL_TYPE_BEV:
        boot_rom(ie->سمتیہ);
        توڑو;
    حالت IPL_TYPE_HALT:
        boot_fail();
        توڑو;
    }

    // Boot failed: invoke the boot recovery function
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

صحیح BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
خالی VISIBLE32FLAT
handle_18(خالی)
{
    debug_enter(تہی, DEBUG_HDL_18);
    صحیح seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
خالی VISIBLE32FLAT
handle_19(خالی)
{
    debug_enter(تہی, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
