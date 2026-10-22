// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "fw/paravirt.h" // qemu_cfg_show_boot_menu
#அந்தர்பாவ "hw/pci.h" // pci_bdf_to_*
#அந்தர்பாவ "hw/pcidevice.h" // struct pci_device
#அந்தர்பாவ "hw/rtc.h" // rtc_read
#அந்தர்பாவ "hw/usb.h" // struct usbdevice_s
#அந்தர்பாவ "list.h" // hlist_node
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "std/disk.h" // struct mbr_s
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // irqtimer_calc
#அந்தர்பாவ "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
நிலையான எழுத்து *
glob_prefix(மாறிலி எழுத்து *glob, மாறிலி எழுத்து *str)
{
    க்கு (;;) {
        எனில் (!*glob && (!*str || *str == '/'))
            திரும்பு (எழுத்து*)str;
        எனில் (*glob == '*') {
            எனில் (!*str || *str == '/' || *str == glob[1])
                glob++;
            இல்லையெனில்
                str++;
            தொடர்;
        }
        எனில் (*glob != *str)
            திரும்பு வெற்று;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

நிலையான எழுத்து *
build_pci_path(எழுத்து *buf, முழு max, மாறிலி எழுத்து *devname, கட்டமைப்பு pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    எழுத்து *p = buf;
    எனில் (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } இல்லையெனில் {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        எனில் (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    முழு dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    எனில் (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    திரும்பு p;
}

நிலையான எழுத்து *
build_scsi_path(எழுத்து *buf, முழு max,
                கட்டமைப்பு pci_device *pci, முழு target, முழு lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    எழுத்து *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    திரும்பு p;
}

நிலையான எழுத்து *
build_ata_path(எழுத்து *buf, முழு max,
               கட்டமைப்பு pci_device *pci, முழு chanid, முழு slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    எழுத்து *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    திரும்பு p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

வகைவரை கட்டமைப்பு BootDeviceLCHS {
    எழுத்து *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

நிலையான BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
நிலையான முழு Bios_Geometry_மொத்தம்;

நிலையான எழுத்து *
parse_u32(எழுத்து *cur, u32 *n)
{
    u32 m = 0;
    எனில் (cur) {
        போது ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        எனில் (*cur != '\0')
            cur++;
    }
    *n = m;
    திரும்பு cur;
}

நிலையான வெறுமை
loadBiosGeometry(வெறுமை)
{
    எனில் (!CONFIG_HOST_BIOS_GEOMETRY)
        திரும்பு;
    எழுத்து *f = romfile_loadfile("bios-geometry", வெற்று);
    எனில் (!f)
        திரும்பு;

    முழு i = 0;
    Bios_Geometry_மொத்தம் = 1;
    போது (f[i]) {
        எனில் (f[i] == '\n')
            Bios_Geometry_மொத்தம்++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_மொத்தம் * அளவு(BootDeviceLCHS));
    எனில் (!BiosGeometry) {
        warn_noalloc();
        விடுவி(f);
        Bios_Geometry_மொத்தம் = 0;
        திரும்பு;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    செய் {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        எனில் (f)
            *(f++) = '\0';
        எழுத்து *chs_மதிப்புகள் = strchr(d->name, ' ');
        எனில் (chs_மதிப்புகள்)
            *(chs_மதிப்புகள்++) = '\0';
        chs_மதிப்புகள் = parse_u32(chs_மதிப்புகள், &d->lcyls);
        chs_மதிப்புகள் = parse_u32(chs_மதிப்புகள், &d->lheads);
        chs_மதிப்புகள் = parse_u32(chs_மதிப்புகள், &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } போது (f);
}

// Search the bios-geometry list for the given glob pattern.
நிலையான BootDeviceLCHS *
boot_lchs_find(மாறிலி எழுத்து *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    முழு i;
    க்கு (i = 0; i < Bios_Geometry_மொத்தம்; i++)
        எனில் (glob_prefix(glob, BiosGeometry[i].name))
            திரும்பு &BiosGeometry[i];
    திரும்பு வெற்று;
}

முழு boot_lchs_find_pci_device(கட்டமைப்பு pci_device *pci, கட்டமைப்பு chs_s *chs)
{
    எனில் (!CONFIG_HOST_BIOS_GEOMETRY)
        திரும்பு -1;
    எழுத்து desc[256];
    build_pci_path(desc, அளவு(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    எனில் (!b)
        திரும்பு -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    திரும்பு 0;
}

முழு boot_lchs_find_scsi_device(கட்டமைப்பு pci_device *pci, முழு target, முழு lun,
                               கட்டமைப்பு chs_s *chs)
{
    எனில் (!CONFIG_HOST_BIOS_GEOMETRY)
        திரும்பு -1;
    எனில் (!pci)
        // support only pci machine for now
        திரும்பு -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    எழுத்து desc[256];
    build_scsi_path(desc, அளவு(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    எனில் (!b)
        திரும்பு -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    திரும்பு 0;
}

முழு boot_lchs_find_ata_device(கட்டமைப்பு pci_device *pci, முழு chanid, முழு slave,
                              கட்டமைப்பு chs_s *chs)
{
    எனில் (!CONFIG_HOST_BIOS_GEOMETRY)
        திரும்பு -1;
    எனில் (!pci)
        // support only pci machine for now
        திரும்பு -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    எழுத்து desc[256];
    build_ata_path(desc, அளவு(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    எனில் (!b)
        திரும்பு -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    திரும்பு 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

நிலையான எழுத்து **Bootorder VARVERIFY32INIT;
நிலையான முழு Bootorder_மொத்தம்;

நிலையான வெறுமை
loadBootOrder(வெறுமை)
{
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு;

    எழுத்து *f = romfile_loadfile("bootorder", வெற்று);
    எனில் (!f)
        திரும்பு;

    முழு i = 0;
    Bootorder_மொத்தம் = 1;
    போது (f[i]) {
        எனில் (f[i] == '\n')
            Bootorder_மொத்தம்++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_மொத்தம்*அளவு(எழுத்து*));
    எனில் (!Bootorder) {
        warn_noalloc();
        விடுவி(f);
        Bootorder_மொத்தம் = 0;
        திரும்பு;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    செய் {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        எனில் (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } போது (f);
}

// Search the bootorder list for the given glob pattern.
நிலையான முழு
find_prio(மாறிலி எழுத்து *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    முழு i;
    க்கு (i = 0; i < Bootorder_மொத்தம்; i++)
        எனில் (glob_prefix(glob, Bootorder[i]))
            திரும்பு i+1;
    திரும்பு -1;
}

u8 is_bootprio_strict(வெறுமை)
{
    நிலையான முழு prio_halt = -2;

    எனில் (prio_halt == -2)
        prio_halt = find_prio("HALT");
    திரும்பு prio_halt >= 0;
}

முழு bootprio_find_pci_device(கட்டமைப்பு pci_device *pci)
{
    எனில் (CONFIG_CSM)
        திரும்பு csm_bootprio_pci(pci);
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    எழுத்து desc[256];
    build_pci_path(desc, அளவு(desc), "*", pci);
    திரும்பு find_prio(desc);
}

முழு bootprio_find_mmio_device(வெறுமை *mmio)
{
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    எழுத்து desc[256];
    snprintf(desc, அளவு(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    திரும்பு find_prio(desc);
}

முழு bootprio_find_scsi_device(கட்டமைப்பு pci_device *pci, முழு target, முழு lun)
{
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    எனில் (!pci)
        // support only pci machine for now
        திரும்பு -1;
    எழுத்து desc[256];
    build_scsi_path(desc, அளவு(desc), pci, target, lun);
    திரும்பு find_prio(desc);
}

முழு bootprio_find_scsi_mmio_device(வெறுமை *mmio, முழு target, முழு lun)
{
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    எழுத்து desc[256];
    snprintf(desc, அளவு(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    திரும்பு find_prio(desc);
}

முழு bootprio_find_ata_device(கட்டமைப்பு pci_device *pci, முழு chanid, முழு slave)
{
    எனில் (CONFIG_CSM)
        திரும்பு csm_bootprio_ata(pci, chanid, slave);
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    எனில் (!pci)
        // support only pci machine for now
        திரும்பு -1;
    எழுத்து desc[256];
    build_ata_path(desc, அளவு(desc), pci, chanid, slave);
    திரும்பு find_prio(desc);
}

முழு bootprio_find_fdc_device(கட்டமைப்பு pci_device *pci, முழு port, முழு fdid)
{
    எனில் (CONFIG_CSM)
        திரும்பு csm_bootprio_fdc(pci, port, fdid);
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    எனில் (!pci)
        // support only pci machine for now
        திரும்பு -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    எழுத்து desc[256], *p;
    p = build_pci_path(desc, அளவு(desc), "isa", pci);
    snprintf(p, desc+அளவு(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    திரும்பு find_prio(desc);
}

முழு bootprio_find_pci_rom(கட்டமைப்பு pci_device *pci, முழு instance)
{
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    எழுத்து desc[256], *p;
    p = build_pci_path(desc, அளவு(desc), "*", pci);
    எனில் (instance)
        snprintf(p, desc+அளவு(desc)-p, ":rom%x", instance);
    திரும்பு find_prio(desc);
}

முழு bootprio_find_named_rom(மாறிலி எழுத்து *name, முழு instance)
{
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    எழுத்து desc[256], *p;
    p = desc + snprintf(desc, அளவு(desc), "/rom@%s", name);
    எனில் (instance)
        snprintf(p, desc+அளவு(desc)-p, ":rom%x", instance);
    திரும்பு find_prio(desc);
}

நிலையான முழு usb_portmap(கட்டமைப்பு usbdevice_s *usbdev)
{
    எனில் (usbdev->hub->op->portmap)
        திரும்பு usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    திரும்பு usbdev->port + 1;
}

நிலையான எழுத்து *
build_usb_path(எழுத்து *buf, முழு max, கட்டமைப்பு usbhub_s *hub)
{
    எனில் (!hub->usbdev)
        // Root hub - nothing to add.
        திரும்பு buf;
    எழுத்து *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    திரும்பு p;
}

முழு bootprio_find_usb(கட்டமைப்பு usbdevice_s *usbdev, முழு lun)
{
    எனில் (!CONFIG_BOOTORDER)
        திரும்பு -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    எழுத்து desc[256], *p;

    எனில் (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, அளவு(desc), "usb", usbdev->hub->cntl->pci);
    இல்லையெனில் எனில் (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, அளவு(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    இல்லையெனில்
        திரும்பு -1;

    p = build_usb_path(p, desc+அளவு(desc)-p, usbdev->hub);
    snprintf(p, desc+அளவு(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    முழு ret = find_prio(desc);
    எனில் (ret >= 0)
        திரும்பு ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+அளவு(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    திரும்பு find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

நிலையான முழு BootRetryTime;
நிலையான முழு CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

நிலையான முழு DefaultFloppyPrio = 101;
நிலையான முழு DefaultCDPrio     = 102;
நிலையான முழு DefaultHDPrio     = 103;
நிலையான முழு DefaultBEVPrio    = 104;

வெறுமை
boot_init(வெறுமை)
{
    எனில் (! CONFIG_BOOT)
        திரும்பு;

    எனில் (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        எனில் (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        முழு i;
        க்கு (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            தேர்வு (val) {
            நிலை 1: DefaultFloppyPrio = i; நிறுத்து;
            நிலை 2: DefaultHDPrio = i;     நிறுத்து;
            நிலை 3: DefaultCDPrio = i;     நிறுத்து;
            நிலை 4: DefaultBEVPrio = i;    நிறுத்து;
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

கட்டமைப்பு bootentry_s {
    முழு type;
    ஒன்றிப்பு {
        u32 data;
        கட்டமைப்பு segoff_s vector;
        கட்டமைப்பு drive_s *drive;
    };
    முழு priority;
    மாறிலி எழுத்து *description;
    கட்டமைப்பு hlist_முனை முனை;
};
நிலையான கட்டமைப்பு hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

நிலையான வெறுமை
bootentry_add(முழு type, முழு prio, u32 data, மாறிலி எழுத்து *desc)
{
    எனில் (! CONFIG_BOOT)
        திரும்பு;
    கட்டமைப்பு bootentry_s *be = malloc_tmp(அளவு(*be));
    எனில் (!be) {
        warn_noalloc();
        திரும்பு;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    கட்டமைப்பு hlist_முனை **pprev;
    கட்டமைப்பு bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, முனை) {
        எனில் (be->priority < pos->priority)
            நிறுத்து;
        எனில் (be->priority > pos->priority)
            தொடர்;
        எனில் (be->type < pos->type)
            நிறுத்து;
        எனில் (be->type > pos->type)
            தொடர்;
        எனில் (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            நிறுத்து;
    }
    hlist_add(&be->முனை, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
நிலையான உள்வரி முழு defPrio(முழு priority, முழு defaultprio) {
    திரும்பு (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
வெறுமை
boot_add_bev(u16 seg, u16 bev, u16 desc, முழு prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
வெறுமை
boot_add_bcv(u16 seg, u16 ip, u16 desc, முழு prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

வெறுமை
boot_add_floppy(கட்டமைப்பு drive_s *drive, மாறிலி எழுத்து *desc, முழு prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

வெறுமை
boot_add_hd(கட்டமைப்பு drive_s *drive, மாறிலி எழுத்து *desc, முழு prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

வெறுமை
boot_add_cd(கட்டமைப்பு drive_s *drive, மாறிலி எழுத்து *desc, முழு prio)
{
    எனில் (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        எழுத்து *extra = cdrom_media_info(drive);
        எனில் (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            விடுவி(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
வெறுமை
boot_add_cbfs(வெறுமை *data, மாறிலி எழுத்து *desc, முழு prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
நிலையான முழு
check_for_keystroke(வெறுமை)
{
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    திரும்பு !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
நிலையான முழு
get_raw_keystroke(வெறுமை)
{
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    திரும்பு br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
முழு
get_keystroke_full(முழு msec)
{
    u32 end = irqtimer_calc(msec);
    க்கு (;;) {
        எனில் (check_for_keystroke())
            திரும்பு get_raw_keystroke();
        எனில் (irqtimer_check(end))
            திரும்பு -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
முழு
get_keystroke(முழு msec)
{
    முழு keystroke = get_keystroke_full(msec);

    எனில் (keystroke < 0)
        திரும்பு keystroke;
    திரும்பு keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

நிலையான மாறிலி எழுத்து menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
வெறுமை
interactive_bootmenu(வெறுமை)
{
    எனில் (! CONFIG_BOOTMENU)
        திரும்பு;
    முழு show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    எனில் (!show_boot_menu)
        திரும்பு;

    // skip menu if only one boot device and no TPM
    எனில் (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        திரும்பு;
    }

    முழு menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    முழு menukey = romfile_loadint("etc/boot-menu-key", 1);
    முழு scan_code;
    எனில் (menutime >= 0) {
        போது (get_keystroke(0) >= 0)
            ;

        எழுத்து *bootmsg = romfile_loadfile("etc/boot-menu-message", வெற்று);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        விடுவி(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        எனில் (scan_code != menukey)
            திரும்பு;
    }

    போது (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    முழு maxmenu = 0;
    கட்டமைப்பு bootentry_s *pos, *boot = வெற்று;
    hlist_for_each_entry(pos, &BootList, முனை) {
        எழுத்து desc[77];
        எனில் (maxmenu >= ARRAY_SIZE(menuchars)) {
            நிறுத்து;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    எனில் (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    முழு esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    க்கு (;;) {
        முழு keystroke = get_keystroke_full(1000);
        எனில் (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            தொடர்;
        எனில் (keystroke < 0) // timeout
            தொடர்;

        scan_code = keystroke >> 8;
        முழு சாவி_ascii = keystroke & 0xff;
        எனில் (tpm_can_show_menu() && சாவி_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        எனில் (scan_code == 1) {
            // ESC
            printf("\n");
            திரும்பு;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, முனை) {
            எனில் (maxmenu >= ARRAY_SIZE(menuchars))
                நிறுத்து;
            எனில் (சாவி_ascii == menuchars[maxmenu]) {
                boot = pos;
                நிறுத்து;
            }
            maxmenu++;
        }
        எனில் (boot)
            நிறுத்து;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->முனை);
    boot->priority = 0;
    hlist_add_head(&boot->முனை, &BootList);
}

// BEV (Boot Execution Vector) list
கட்டமைப்பு bev_s {
    முழு type;
    u32 vector;
};
நிலையான கட்டமைப்பு bev_s BEV[20];
நிலையான முழு BEVCount;
நிலையான முழு HaveHDBoot, HaveFDBoot;

நிலையான வெறுமை
add_bev(முழு type, u32 vector)
{
    எனில் (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        திரும்பு;
    எனில் (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        திரும்பு;
    எனில் (BEVCount >= ARRAY_SIZE(BEV))
        திரும்பு;
    கட்டமைப்பு bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
வெறுமை
bcv_prepboot(வெறுமை)
{
    எனில் (! CONFIG_BOOT)
        திரும்பு;

    முழு haltprio = find_prio("HALT");
    எனில் (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    கட்டமைப்பு bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, முனை) {
        தேர்வு (pos->type) {
        நிலை IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            நிறுத்து;
        நிலை IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            நிறுத்து;
        நிலை IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            நிறுத்து;
        நிலை IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        இயல்பு:
            add_bev(pos->type, pos->data);
            நிறுத்து;
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
நிலையான வெறுமை
call_boot_entry(கட்டமைப்பு segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
நிலையான வெறுமை
boot_disk(u8 bootdrv, முழு checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    எனில் (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        திரும்பு;
    }

    எனில் (checksig) {
        கட்டமைப்பு mbr_s *mbr = (வெறுமை*)0;
        எனில் (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            திரும்பு;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
நிலையான வெறுமை
boot_cdrom(கட்டமைப்பு drive_s *drive)
{
    எனில் (! CONFIG_CDROM_BOOT)
        திரும்பு;
    printf("Booting from DVD/CD...\n");

    முழு status = cdrom_boot(drive);
    எனில் (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        திரும்பு;
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
நிலையான வெறுமை
boot_cbfs(கட்டமைப்பு cbfs_file *file)
{
    எனில் (!CONFIG_COREBOOT_FLASH)
        திரும்பு;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
நிலையான வெறுமை
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    கட்டமைப்பு segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
நிலையான வெறுமை
boot_fail(வெறுமை)
{
    எனில் (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    இல்லையெனில்
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    க்கு (;;) {
        எனில் (BootRetryTime != (u32)-1 && irqtimer_check(end))
            நிறுத்து;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
நிலையான வெறுமை
do_boot(முழு seq_nr)
{
    எனில் (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    எனில் (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    கட்டமைப்பு bev_s *ie = &BEV[seq_nr];
    தேர்வு (ie->type) {
    நிலை IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        நிறுத்து;
    நிலை IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        நிறுத்து;
    நிலை IPL_TYPE_CDROM:
        boot_cdrom((வெறுமை*)ie->vector);
        நிறுத்து;
    நிலை IPL_TYPE_CBFS:
        boot_cbfs((வெறுமை*)ie->vector);
        நிறுத்து;
    நிலை IPL_TYPE_BEV:
        boot_rom(ie->vector);
        நிறுத்து;
    நிலை IPL_TYPE_HALT:
        boot_fail();
        நிறுத்து;
    }

    // Boot failed: invoke the boot recovery function
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

முழு BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
வெறுமை VISIBLE32FLAT
handle_18(வெறுமை)
{
    debug_enter(வெற்று, DEBUG_HDL_18);
    முழு seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
வெறுமை VISIBLE32FLAT
handle_19(வெறுமை)
{
    debug_enter(வெற்று, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
