// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "block.h" // struct drive_s
#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "fw/paravirt.h" // qemu_cfg_show_boot_menu
#ضمن "hw/pci.h" // pci_bdf_to_*
#ضمن "hw/pcidevice.h" // struct pci_device
#ضمن "hw/rtc.h" // rtc_read
#ضمن "hw/usb.h" // struct usbdevice_s
#ضمن "list.h" // hlist_node
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadint
#ضمن "std/disk.h" // struct mbr_s
#ضمن "string.h" // memset
#ضمن "util.h" // irqtimer_calc
#ضمن "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
ساكن حرف *
glob_اڳياڙي(ثابت حرف *glob, ثابت حرف *str)
{
    لكل (;;) {
        إذا (!*glob && (!*str || *str == '/'))
            أرجع (حرف*)str;
        إذا (*glob == '*') {
            إذا (!*str || *str == '/' || *str == glob[1])
                glob++;
            وإلا
                str++;
            استمر;
        }
        إذا (*glob != *str)
            أرجع عدم;
        glob++;
        str++;
    }
}

#وصف_ڪريو FW_PCI_DOMAIN "/pci@i0cf8"

ساكن حرف *
build_pci_رستو(حرف *buf, صحيح max, ثابت حرف *devname, بنية pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    حرف *p = buf;
    إذا (pci->والد) {
        p = build_pci_رستو(p, max, "pci-bridge", pci->والد);
    } وإلا {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        إذا (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    صحيح dev = pci_bdf_منزل_dev(pci->bdf), fn = pci_bdf_منزل_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    إذا (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    أرجع p;
}

ساكن حرف *
build_scsi_رستو(حرف *buf, صحيح max,
                بنية pci_device *pci, صحيح هدف, صحيح lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    حرف *p;
    p = build_pci_رستو(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", هدف, lun);
    أرجع p;
}

ساكن حرف *
build_ata_رستو(حرف *buf, صحيح max,
               بنية pci_device *pci, صحيح chanid, صحيح slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    حرف *p;
    p = build_pci_رستو(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    أرجع p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

تعريفنوع بنية BootDeviceLCHS {
    حرف *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

ساكن BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
ساكن صحيح Bios_Geometry_ڳڻپ;

ساكن حرف *
parse_u32(حرف *cur, u32 *n)
{
    u32 m = 0;
    إذا (cur) {
        طالما ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        إذا (*cur != '\0')
            cur++;
    }
    *n = m;
    أرجع cur;
}

ساكن فراغ
ڀريو_Bios_Geometry(فراغ)
{
    إذا (!CONFIG_HOST_BIOS_GEOMETRY)
        أرجع;
    حرف *f = romfile_loadfile("bios-geometry", عدم);
    إذا (!f)
        أرجع;

    صحيح i = 0;
    Bios_Geometry_ڳڻپ = 1;
    طالما (f[i]) {
        إذا (f[i] == '\n')
            Bios_Geometry_ڳڻپ++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_ڳڻپ * حجم(BootDeviceLCHS));
    إذا (!BiosGeometry) {
        warn_noalloc();
        حرر(f);
        Bios_Geometry_ڳڻپ = 0;
        أرجع;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    افعل {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        إذا (f)
            *(f++) = '\0';
        حرف *chs_قدرون = strchr(d->name, ' ');
        إذا (chs_قدرون)
            *(chs_قدرون++) = '\0';
        chs_قدرون = parse_u32(chs_قدرون, &d->lcyls);
        chs_قدرون = parse_u32(chs_قدرون, &d->lheads);
        chs_قدرون = parse_u32(chs_قدرون, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } طالما (f);
}

// Search the bios-geometry list for the given glob pattern.
ساكن BootDeviceLCHS *
boot_lchs_ڳوليو(ثابت حرف *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    صحيح i;
    لكل (i = 0; i < Bios_Geometry_ڳڻپ; i++)
        إذا (glob_اڳياڙي(glob, BiosGeometry[i].name))
            أرجع &BiosGeometry[i];
    أرجع عدم;
}

صحيح boot_lchs_ڳوليو_pci_device(بنية pci_device *pci, بنية chs_s *chs)
{
    إذا (!CONFIG_HOST_BIOS_GEOMETRY)
        أرجع -1;
    حرف desc[256];
    build_pci_رستو(desc, حجم(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_ڳوليو(desc);
    إذا (!b)
        أرجع -1;
    chs->cylinder = (u16)b->lcyls;
    chs->سر = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    أرجع 0;
}

صحيح boot_lchs_ڳوليو_scsi_device(بنية pci_device *pci, صحيح هدف, صحيح lun,
                               بنية chs_s *chs)
{
    إذا (!CONFIG_HOST_BIOS_GEOMETRY)
        أرجع -1;
    إذا (!pci)
        // support only pci machine for now
        أرجع -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    حرف desc[256];
    build_scsi_رستو(desc, حجم(desc), pci, هدف, lun);
    BootDeviceLCHS *b = boot_lchs_ڳوليو(desc);
    إذا (!b)
        أرجع -1;
    chs->cylinder = (u16)b->lcyls;
    chs->سر = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    أرجع 0;
}

صحيح boot_lchs_ڳوليو_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave,
                              بنية chs_s *chs)
{
    إذا (!CONFIG_HOST_BIOS_GEOMETRY)
        أرجع -1;
    إذا (!pci)
        // support only pci machine for now
        أرجع -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    حرف desc[256];
    build_ata_رستو(desc, حجم(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_ڳوليو(desc);
    إذا (!b)
        أرجع -1;
    chs->cylinder = (u16)b->lcyls;
    chs->سر = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    أرجع 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

ساكن حرف **Bootorder VARVERIFY32INIT;
ساكن صحيح Bootorder_ڳڻپ;

ساكن فراغ
ڀريو_Boot_ترتيب(فراغ)
{
    إذا (!CONFIG_BOOTORDER)
        أرجع;

    حرف *f = romfile_loadfile("bootorder", عدم);
    إذا (!f)
        أرجع;

    صحيح i = 0;
    Bootorder_ڳڻپ = 1;
    طالما (f[i]) {
        إذا (f[i] == '\n')
            Bootorder_ڳڻپ++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_ڳڻپ*حجم(حرف*));
    إذا (!Bootorder) {
        warn_noalloc();
        حرر(f);
        Bootorder_ڳڻپ = 0;
        أرجع;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    افعل {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        إذا (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } طالما (f);
}

// Search the bootorder list for the given glob pattern.
ساكن صحيح
ڳوليو_prio(ثابت حرف *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    صحيح i;
    لكل (i = 0; i < Bootorder_ڳڻپ; i++)
        إذا (glob_اڳياڙي(glob, Bootorder[i]))
            أرجع i+1;
    أرجع -1;
}

u8 is_bootprio_strict(فراغ)
{
    ساكن صحيح prio_halt = -2;

    إذا (prio_halt == -2)
        prio_halt = ڳوليو_prio("HALT");
    أرجع prio_halt >= 0;
}

صحيح bootprio_ڳوليو_pci_device(بنية pci_device *pci)
{
    إذا (CONFIG_CSM)
        أرجع csm_bootprio_pci(pci);
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    حرف desc[256];
    build_pci_رستو(desc, حجم(desc), "*", pci);
    أرجع ڳوليو_prio(desc);
}

صحيح bootprio_ڳوليو_mmio_device(فراغ *mmio)
{
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    حرف desc[256];
    snprintf(desc, حجم(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    أرجع ڳوليو_prio(desc);
}

صحيح bootprio_ڳوليو_scsi_device(بنية pci_device *pci, صحيح هدف, صحيح lun)
{
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    إذا (!pci)
        // support only pci machine for now
        أرجع -1;
    حرف desc[256];
    build_scsi_رستو(desc, حجم(desc), pci, هدف, lun);
    أرجع ڳوليو_prio(desc);
}

صحيح bootprio_ڳوليو_scsi_mmio_device(فراغ *mmio, صحيح هدف, صحيح lun)
{
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    حرف desc[256];
    snprintf(desc, حجم(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, هدف, lun);
    أرجع ڳوليو_prio(desc);
}

صحيح bootprio_ڳوليو_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave)
{
    إذا (CONFIG_CSM)
        أرجع csm_bootprio_ata(pci, chanid, slave);
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    إذا (!pci)
        // support only pci machine for now
        أرجع -1;
    حرف desc[256];
    build_ata_رستو(desc, حجم(desc), pci, chanid, slave);
    أرجع ڳوليو_prio(desc);
}

صحيح bootprio_ڳوليو_fdc_device(بنية pci_device *pci, صحيح پورٽ, صحيح fdid)
{
    إذا (CONFIG_CSM)
        أرجع csm_bootprio_fdc(pci, پورٽ, fdid);
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    إذا (!pci)
        // support only pci machine for now
        أرجع -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    حرف desc[256], *p;
    p = build_pci_رستو(desc, حجم(desc), "isa", pci);
    snprintf(p, desc+حجم(desc)-p, "/fdc@%04x/floppy@%x", پورٽ, fdid);
    أرجع ڳوليو_prio(desc);
}

صحيح bootprio_ڳوليو_pci_rom(بنية pci_device *pci, صحيح instance)
{
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    حرف desc[256], *p;
    p = build_pci_رستو(desc, حجم(desc), "*", pci);
    إذا (instance)
        snprintf(p, desc+حجم(desc)-p, ":rom%x", instance);
    أرجع ڳوليو_prio(desc);
}

صحيح bootprio_ڳوليو_named_rom(ثابت حرف *name, صحيح instance)
{
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    حرف desc[256], *p;
    p = desc + snprintf(desc, حجم(desc), "/rom@%s", name);
    إذا (instance)
        snprintf(p, desc+حجم(desc)-p, ":rom%x", instance);
    أرجع ڳوليو_prio(desc);
}

ساكن صحيح usb_portmap(بنية usbdevice_s *usbdev)
{
    إذا (usbdev->hub->op->portmap)
        أرجع usbdev->hub->op->portmap(usbdev->hub, usbdev->پورٽ);
    أرجع usbdev->پورٽ + 1;
}

ساكن حرف *
build_usb_رستو(حرف *buf, صحيح max, بنية usbhub_s *hub)
{
    إذا (!hub->usbdev)
        // Root hub - nothing to add.
        أرجع buf;
    حرف *p = build_usb_رستو(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    أرجع p;
}

صحيح bootprio_ڳوليو_usb(بنية usbdevice_s *usbdev, صحيح lun)
{
    إذا (!CONFIG_BOOTORDER)
        أرجع -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    حرف desc[256], *p;

    إذا (usbdev->hub->cntl->pci)
        p = build_pci_رستو(desc, حجم(desc), "usb", usbdev->hub->cntl->pci);
    وإلا إذا (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, حجم(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    وإلا
        أرجع -1;

    p = build_usb_رستو(p, desc+حجم(desc)-p, usbdev->hub);
    snprintf(p, desc+حجم(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    صحيح ret = ڳوليو_prio(desc);
    إذا (ret >= 0)
        أرجع ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+حجم(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    أرجع ڳوليو_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

ساكن صحيح Boot_Retry_وقت;
ساكن صحيح CheckFloppySig = 1;

#وصف_ڪريو DEFAULT_PRIO           9999

ساكن صحيح DefaultFloppyPrio = 101;
ساكن صحيح DefaultCDPrio     = 102;
ساكن صحيح DefaultHDPrio     = 103;
ساكن صحيح DefaultBEVPrio    = 104;

فراغ
boot_init(فراغ)
{
    إذا (! CONFIG_BOOT)
        أرجع;

    إذا (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        إذا (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        صحيح i;
        لكل (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            بدل (val) {
            حالة 1: DefaultFloppyPrio = i; اكسر;
            حالة 2: DefaultHDPrio = i;     اكسر;
            حالة 3: DefaultCDPrio = i;     اكسر;
            حالة 4: DefaultBEVPrio = i;    اكسر;
            }
        }
    }

    Boot_Retry_وقت = romfile_loadint("etc/boot-fail-wait", 60*1000);

    ڀريو_Boot_ترتيب();
    ڀريو_Bios_Geometry();
}


/****************************************************************
 * BootList handling
 ****************************************************************/

بنية bootentry_s {
    صحيح type;
    اتحاد {
        u32 معلومات;
        بنية segoff_s ويڪٽر;
        بنية drive_s *drive;
    };
    صحيح ترجيح;
    ثابت حرف *description;
    بنية hlist_ڳنڍ ڳنڍ;
};
ساكن بنية hlist_سر BootList VARVERIFY32INIT;

#وصف_ڪريو IPL_TYPE_FLOPPY      0x01
#وصف_ڪريو IPL_TYPE_HARDDISK    0x02
#وصف_ڪريو IPL_TYPE_CDROM       0x03
#وصف_ڪريو IPL_TYPE_CBFS        0x20
#وصف_ڪريو IPL_TYPE_BEV         0x80
#وصف_ڪريو IPL_TYPE_BCV         0x81
#وصف_ڪريو IPL_TYPE_HALT        0xf0

ساكن فراغ
bootentry_جوڙيو(صحيح type, صحيح prio, u32 معلومات, ثابت حرف *desc)
{
    إذا (! CONFIG_BOOT)
        أرجع;
    بنية bootentry_s *be = malloc_tmp(حجم(*be));
    إذا (!be) {
        warn_noalloc();
        أرجع;
    }
    be->type = type;
    be->ترجيح = prio;
    be->معلومات = معلومات;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, معلومات);

    // Add entry in sorted order.
    بنية hlist_ڳنڍ **pprev;
    بنية bootentry_s *pos;
    hlist_for_each_داخلا_pprev(pos, pprev, &BootList, ڳنڍ) {
        إذا (be->ترجيح < pos->ترجيح)
            اكسر;
        إذا (be->ترجيح > pos->ترجيح)
            استمر;
        إذا (be->type < pos->type)
            اكسر;
        إذا (be->type > pos->type)
            استمر;
        إذا (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            اكسر;
    }
    hlist_جوڙيو(&be->ڳنڍ, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
ساكن مضمن صحيح defPrio(صحيح ترجيح, صحيح defaultprio) {
    أرجع (ترجيح < 0) ? defaultprio : ترجيح;
}

// Add a BEV vector for a given pnp compatible option rom.
فراغ
boot_جوڙيو_bev(u16 seg, u16 bev, u16 desc, صحيح prio)
{
    bootentry_جوڙيو(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
فراغ
boot_جوڙيو_bcv(u16 seg, u16 ip, u16 desc, صحيح prio)
{
    bootentry_جوڙيو(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

فراغ
boot_جوڙيو_floppy(بنية drive_s *drive, ثابت حرف *desc, صحيح prio)
{
    bootentry_جوڙيو(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

فراغ
boot_جوڙيو_hd(بنية drive_s *drive, ثابت حرف *desc, صحيح prio)
{
    bootentry_جوڙيو(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

فراغ
boot_جوڙيو_cd(بنية drive_s *drive, ثابت حرف *desc, صحيح prio)
{
    إذا (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        حرف *extra = cdrom_media_info(drive);
        إذا (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            حرر(extra);
        }
    }
    bootentry_جوڙيو(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
فراغ
boot_جوڙيو_cbfs(فراغ *معلومات, ثابت حرف *desc, صحيح prio)
{
    bootentry_جوڙيو(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)معلومات, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
ساكن صحيح
check_for_keystroke(فراغ)
{
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    أرجع !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
ساكن صحيح
get_raw_keystroke(فراغ)
{
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    أرجع br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
صحيح
get_keystroke_ڀريل(صحيح msec)
{
    u32 پڇاڙي = irqtimer_calc(msec);
    لكل (;;) {
        إذا (check_for_keystroke())
            أرجع get_raw_keystroke();
        إذا (irqtimer_check(پڇاڙي))
            أرجع -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
صحيح
get_keystroke(صحيح msec)
{
    صحيح keystroke = get_keystroke_ڀريل(msec);

    إذا (keystroke < 0)
        أرجع keystroke;
    أرجع keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#وصف_ڪريو DEFAULT_BOOTMENU_WAIT 2500

ساكن ثابت حرف menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
فراغ
interactive_bootmenu(فراغ)
{
    إذا (! CONFIG_BOOTMENU)
        أرجع;
    صحيح show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    إذا (!show_boot_menu)
        أرجع;

    // skip menu if only one boot device and no TPM
    إذا (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_خالي_جايون(&BootList) && !BootList.پهريون->ايندڙ) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        لکو("\n");
        أرجع;
    }

    صحيح menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    صحيح menukey = romfile_loadint("etc/boot-menu-key", 1);
    صحيح جاچ_ڪوڊ;
    إذا (menutime >= 0) {
        طالما (get_keystroke(0) >= 0)
            ;

        حرف *bootmsg = romfile_loadfile("etc/boot-menu-message", عدم);
        لکو("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        حرر(bootmsg);

        enable_bootsplash();
        جاچ_ڪوڊ = get_keystroke(menutime);
        disable_bootsplash();
        إذا (جاچ_ڪوڊ != menukey)
            أرجع;
    }

    طالما (get_keystroke(0) >= 0)
        ;

    لکو("Select boot device:\n\n");
    wait_ڌاڳا();

    // Show menu items
    صحيح maxmenu = 0;
    بنية bootentry_s *pos, *boot = عدم;
    hlist_for_each_داخلا(pos, &BootList, ڳنڍ) {
        حرف desc[77];
        إذا (maxmenu >= ARRAY_سائيز(menuchars)) {
            اكسر;
        }
        لکو("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_سائيز(desc)));
        maxmenu++;
    }
    إذا (tpm_can_show_menu()) {
        لکو("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    صحيح esc_accepted_وقت = irqtimer_calc(menukey == 1 ? 1500 : 0);
    لكل (;;) {
        صحيح keystroke = get_keystroke_ڀريل(1000);
        إذا (keystroke == 0x011b && !irqtimer_check(esc_accepted_وقت))
            استمر;
        إذا (keystroke < 0) // timeout
            استمر;

        جاچ_ڪوڊ = keystroke >> 8;
        صحيح ڪنجي_ascii = keystroke & 0xff;
        إذا (tpm_can_show_menu() && ڪنجي_ascii == 't') {
            لکو("\n");
            tpm_menu();
        }
        إذا (جاچ_ڪوڊ == 1) {
            // ESC
            لکو("\n");
            أرجع;
        }

        maxmenu = 0;
        hlist_for_each_داخلا(pos, &BootList, ڳنڍ) {
            إذا (maxmenu >= ARRAY_سائيز(menuchars))
                اكسر;
            إذا (ڪنجي_ascii == menuchars[maxmenu]) {
                boot = pos;
                اكسر;
            }
            maxmenu++;
        }
        إذا (boot)
            اكسر;
    }
    لکو("\n");

    // Find entry and make top priority.
    hlist_del(&boot->ڳنڍ);
    boot->ترجيح = 0;
    hlist_جوڙيو_سر(&boot->ڳنڍ, &BootList);
}

// BEV (Boot Execution Vector) list
بنية bev_s {
    صحيح type;
    u32 ويڪٽر;
};
ساكن بنية bev_s BEV[20];
ساكن صحيح BEVCount;
ساكن صحيح HaveHDBoot, HaveFDBoot;

ساكن فراغ
جوڙيو_bev(صحيح type, u32 ويڪٽر)
{
    إذا (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        أرجع;
    إذا (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        أرجع;
    إذا (BEVCount >= ARRAY_سائيز(BEV))
        أرجع;
    بنية bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->ويڪٽر = ويڪٽر;
}

// Prepare for boot - show menu and run bcvs.
فراغ
bcv_prepboot(فراغ)
{
    إذا (! CONFIG_BOOT)
        أرجع;

    صحيح haltprio = ڳوليو_prio("HALT");
    إذا (haltprio >= 0)
        bootentry_جوڙيو(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    بنية bootentry_s *pos;
    hlist_for_each_داخلا(pos, &BootList, ڳنڍ) {
        بدل (pos->type) {
        حالة IPL_TYPE_BCV:
            call_bcv(pos->ويڪٽر.seg, pos->ويڪٽر.وٿي_2);
            جوڙيو_bev(IPL_TYPE_HARDDISK, 0);
            اكسر;
        حالة IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            جوڙيو_bev(IPL_TYPE_FLOPPY, 0);
            اكسر;
        حالة IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            جوڙيو_bev(IPL_TYPE_HARDDISK, 0);
            اكسر;
        حالة IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        افتراضي:
            جوڙيو_bev(pos->type, pos->معلومات);
            اكسر;
        }
    }

    // If nothing added a floppy/hd boot - add it manually.
    جوڙيو_bev(IPL_TYPE_FLOPPY, 0);
    جوڙيو_bev(IPL_TYPE_HARDDISK, 0);
}


/****************************************************************
 * Boot code (int 18/19)
 ****************************************************************/

// Jump to a bootup entry point.
ساكن فراغ
call_boot_داخلا(بنية segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.وٿي_2);
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.ڪوڊ = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
ساكن فراغ
boot_disk(u8 bootdrv, صحيح checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    إذا (br.flags & F_CF) {
        لکو("Boot failed: could not read the boot disk\n\n");
        أرجع;
    }

    إذا (checksig) {
        بنية mbr_s *mbr = (فراغ*)0;
        إذا (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            لکو("Boot failed: not a bootable disk\n\n");
            أرجع;
        }
    }

    tpm_جوڙيو_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_داخلا(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
ساكن فراغ
boot_cdrom(بنية drive_s *drive)
{
    إذا (! CONFIG_CDROM_BOOT)
        أرجع;
    لکو("Booting from DVD/CD...\n");

    صحيح حالت = cdrom_boot(drive);
    إذا (حالت) {
        لکو("Boot failed: Could not read from CDROM (code %04x)\n", حالت);
        أرجع;
    }

    u8 bootdrv = CDEmu.emulated_drive;
    u16 bootseg = CDEmu.ڀريو_segment;

    tpm_جوڙيو_cdrom(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_داخلا(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CBFS payload
ساكن فراغ
boot_cbfs(بنية cbfs_file *file)
{
    إذا (!CONFIG_COREBOOT_FLASH)
        أرجع;
    لکو("Booting from CBFS...\n");
    cbfs_لاڳيتو_payload(file);
}

// Boot from a BEV entry on an optionrom.
ساكن فراغ
boot_rom(u32 ويڪٽر)
{
    لکو("Booting from ROM...\n");
    بنية segoff_s so;
    so.segoff = ويڪٽر;
    call_boot_داخلا(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
ساكن فراغ
boot_fail(فراغ)
{
    إذا (Boot_Retry_وقت == (u32)-1)
        لکو("No bootable device.\n");
    وإلا
        لکو("No bootable device.  Retrying in %d seconds.\n"
               , Boot_Retry_وقت/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 پڇاڙي = irqtimer_calc(Boot_Retry_وقت);
    لكل (;;) {
        إذا (Boot_Retry_وقت != (u32)-1 && irqtimer_check(پڇاڙي))
            اكسر;
        yield_toirq();
    }
    لکو("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
ساكن فراغ
do_boot(صحيح seq_nr)
{
    إذا (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    إذا (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    بنية bev_s *ie = &BEV[seq_nr];
    بدل (ie->type) {
    حالة IPL_TYPE_FLOPPY:
        لکو("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        اكسر;
    حالة IPL_TYPE_HARDDISK:
        لکو("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        اكسر;
    حالة IPL_TYPE_CDROM:
        boot_cdrom((فراغ*)ie->ويڪٽر);
        اكسر;
    حالة IPL_TYPE_CBFS:
        boot_cbfs((فراغ*)ie->ويڪٽر);
        اكسر;
    حالة IPL_TYPE_BEV:
        boot_rom(ie->ويڪٽر);
        اكسر;
    حالة IPL_TYPE_HALT:
        boot_fail();
        اكسر;
    }

    // Boot failed: invoke the boot recovery function
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

صحيح BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
فراغ VISIBLE32FLAT
handle_18(فراغ)
{
    debug_enter(عدم, DEBUG_HDL_18);
    صحيح seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
فراغ VISIBLE32FLAT
handle_19(فراغ)
{
    debug_enter(عدم, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
