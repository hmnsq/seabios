// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // qemu_cfg_show_boot_menu
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_bdf_to_*
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // struct pci_device
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // rtc_read
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/usb.h" // struct usbdevice_s
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h" // hlist_node
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // struct mbr_s
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // irqtimer_calc
#ਅਨ੍ਤਰ੍ਭਾਵ "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
ਸਥਿਰ ਅੱਖਰ *
glob_ਅਗੇਤਰ(ਅਚਲ ਅੱਖਰ *glob, ਅਚਲ ਅੱਖਰ *str)
{
    ਲਈ (;;) {
        ਜੇ (!*glob && (!*str || *str == '/'))
            ਵਾਪਸ (ਅੱਖਰ*)str;
        ਜੇ (*glob == '*') {
            ਜੇ (!*str || *str == '/' || *str == glob[1])
                glob++;
            ਨਹੀਂਤਾਂ
                str++;
            ਜਾਰੀ;
        }
        ਜੇ (*glob != *str)
            ਵਾਪਸ ਨਿਰਾ;
        glob++;
        str++;
    }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FW_PCI_DOMAIN "/pci@i0cf8"

ਸਥਿਰ ਅੱਖਰ *
build_pci_ਰਸਤਾ(ਅੱਖਰ *buf, ਪੂਰਨ max, ਅਚਲ ਅੱਖਰ *devname, ਬਣਤਰ pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    ਅੱਖਰ *p = buf;
    ਜੇ (pci->ਮਾਪਾ) {
        p = build_pci_ਰਸਤਾ(p, max, "pci-bridge", pci->ਮਾਪਾ);
    } ਨਹੀਂਤਾਂ {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        ਜੇ (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    ਪੂਰਨ dev = pci_bdf_ਮੰਜ਼ਿਲ_dev(pci->bdf), fn = pci_bdf_ਮੰਜ਼ਿਲ_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    ਜੇ (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    ਵਾਪਸ p;
}

ਸਥਿਰ ਅੱਖਰ *
build_scsi_ਰਸਤਾ(ਅੱਖਰ *buf, ਪੂਰਨ max,
                ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਨਿਸ਼ਾਨਾ, ਪੂਰਨ lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ਅੱਖਰ *p;
    p = build_pci_ਰਸਤਾ(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", ਨਿਸ਼ਾਨਾ, lun);
    ਵਾਪਸ p;
}

ਸਥਿਰ ਅੱਖਰ *
build_ata_ਰਸਤਾ(ਅੱਖਰ *buf, ਪੂਰਨ max,
               ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    ਅੱਖਰ *p;
    p = build_pci_ਰਸਤਾ(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    ਵਾਪਸ p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ BootDeviceLCHS {
    ਅੱਖਰ *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

ਸਥਿਰ BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
ਸਥਿਰ ਪੂਰਨ Bios_Geometry_ਗਿਣਤੀ;

ਸਥਿਰ ਅੱਖਰ *
parse_u32(ਅੱਖਰ *cur, u32 *n)
{
    u32 m = 0;
    ਜੇ (cur) {
        ਜਦੋਂ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        ਜੇ (*cur != '\0')
            cur++;
    }
    *n = m;
    ਵਾਪਸ cur;
}

ਸਥਿਰ ਖਾਲੀ
ਲੱਦੋ_Bios_Geometry(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HOST_BIOS_GEOMETRY)
        ਵਾਪਸ;
    ਅੱਖਰ *f = romfile_loadfile("bios-geometry", ਨਿਰਾ);
    ਜੇ (!f)
        ਵਾਪਸ;

    ਪੂਰਨ i = 0;
    Bios_Geometry_ਗਿਣਤੀ = 1;
    ਜਦੋਂ (f[i]) {
        ਜੇ (f[i] == '\n')
            Bios_Geometry_ਗਿਣਤੀ++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_ਗਿਣਤੀ * ਆਕਾਰ(BootDeviceLCHS));
    ਜੇ (!BiosGeometry) {
        warn_noalloc();
        ਮੁਕਤ(f);
        Bios_Geometry_ਗਿਣਤੀ = 0;
        ਵਾਪਸ;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    ਕਰੋ {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        ਜੇ (f)
            *(f++) = '\0';
        ਅੱਖਰ *chs_ਮੁੱਲਾਂ = strchr(d->name, ' ');
        ਜੇ (chs_ਮੁੱਲਾਂ)
            *(chs_ਮੁੱਲਾਂ++) = '\0';
        chs_ਮੁੱਲਾਂ = parse_u32(chs_ਮੁੱਲਾਂ, &d->lcyls);
        chs_ਮੁੱਲਾਂ = parse_u32(chs_ਮੁੱਲਾਂ, &d->lheads);
        chs_ਮੁੱਲਾਂ = parse_u32(chs_ਮੁੱਲਾਂ, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } ਜਦੋਂ (f);
}

// Search the bios-geometry list for the given glob pattern.
ਸਥਿਰ BootDeviceLCHS *
boot_lchs_ਲੱਭੋ(ਅਚਲ ਅੱਖਰ *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < Bios_Geometry_ਗਿਣਤੀ; i++)
        ਜੇ (glob_ਅਗੇਤਰ(glob, BiosGeometry[i].name))
            ਵਾਪਸ &BiosGeometry[i];
    ਵਾਪਸ ਨਿਰਾ;
}

ਪੂਰਨ boot_lchs_ਲੱਭੋ_pci_device(ਬਣਤਰ pci_device *pci, ਬਣਤਰ chs_s *chs)
{
    ਜੇ (!CONFIG_HOST_BIOS_GEOMETRY)
        ਵਾਪਸ -1;
    ਅੱਖਰ desc[256];
    build_pci_ਰਸਤਾ(desc, ਆਕਾਰ(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_ਲੱਭੋ(desc);
    ਜੇ (!b)
        ਵਾਪਸ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->ਸਿਰ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ਵਾਪਸ 0;
}

ਪੂਰਨ boot_lchs_ਲੱਭੋ_scsi_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਨਿਸ਼ਾਨਾ, ਪੂਰਨ lun,
                               ਬਣਤਰ chs_s *chs)
{
    ਜੇ (!CONFIG_HOST_BIOS_GEOMETRY)
        ਵਾਪਸ -1;
    ਜੇ (!pci)
        // support only pci machine for now
        ਵਾਪਸ -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ਅੱਖਰ desc[256];
    build_scsi_ਰਸਤਾ(desc, ਆਕਾਰ(desc), pci, ਨਿਸ਼ਾਨਾ, lun);
    BootDeviceLCHS *b = boot_lchs_ਲੱਭੋ(desc);
    ਜੇ (!b)
        ਵਾਪਸ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->ਸਿਰ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ਵਾਪਸ 0;
}

ਪੂਰਨ boot_lchs_ਲੱਭੋ_ata_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ slave,
                              ਬਣਤਰ chs_s *chs)
{
    ਜੇ (!CONFIG_HOST_BIOS_GEOMETRY)
        ਵਾਪਸ -1;
    ਜੇ (!pci)
        // support only pci machine for now
        ਵਾਪਸ -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    ਅੱਖਰ desc[256];
    build_ata_ਰਸਤਾ(desc, ਆਕਾਰ(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_ਲੱਭੋ(desc);
    ਜੇ (!b)
        ਵਾਪਸ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->ਸਿਰ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ਵਾਪਸ 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

ਸਥਿਰ ਅੱਖਰ **Bootorder VARVERIFY32INIT;
ਸਥਿਰ ਪੂਰਨ Bootorder_ਗਿਣਤੀ;

ਸਥਿਰ ਖਾਲੀ
ਲੱਦੋ_Boot_ਕ੍ਰਮ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ;

    ਅੱਖਰ *f = romfile_loadfile("bootorder", ਨਿਰਾ);
    ਜੇ (!f)
        ਵਾਪਸ;

    ਪੂਰਨ i = 0;
    Bootorder_ਗਿਣਤੀ = 1;
    ਜਦੋਂ (f[i]) {
        ਜੇ (f[i] == '\n')
            Bootorder_ਗਿਣਤੀ++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_ਗਿਣਤੀ*ਆਕਾਰ(ਅੱਖਰ*));
    ਜੇ (!Bootorder) {
        warn_noalloc();
        ਮੁਕਤ(f);
        Bootorder_ਗਿਣਤੀ = 0;
        ਵਾਪਸ;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    ਕਰੋ {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        ਜੇ (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } ਜਦੋਂ (f);
}

// Search the bootorder list for the given glob pattern.
ਸਥਿਰ ਪੂਰਨ
ਲੱਭੋ_prio(ਅਚਲ ਅੱਖਰ *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < Bootorder_ਗਿਣਤੀ; i++)
        ਜੇ (glob_ਅਗੇਤਰ(glob, Bootorder[i]))
            ਵਾਪਸ i+1;
    ਵਾਪਸ -1;
}

u8 is_bootprio_strict(ਖਾਲੀ)
{
    ਸਥਿਰ ਪੂਰਨ prio_halt = -2;

    ਜੇ (prio_halt == -2)
        prio_halt = ਲੱਭੋ_prio("HALT");
    ਵਾਪਸ prio_halt >= 0;
}

ਪੂਰਨ bootprio_ਲੱਭੋ_pci_device(ਬਣਤਰ pci_device *pci)
{
    ਜੇ (CONFIG_CSM)
        ਵਾਪਸ csm_bootprio_pci(pci);
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    ਅੱਖਰ desc[256];
    build_pci_ਰਸਤਾ(desc, ਆਕਾਰ(desc), "*", pci);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਪੂਰਨ bootprio_ਲੱਭੋ_mmio_device(ਖਾਲੀ *mmio)
{
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    ਅੱਖਰ desc[256];
    snprintf(desc, ਆਕਾਰ(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਪੂਰਨ bootprio_ਲੱਭੋ_scsi_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਨਿਸ਼ਾਨਾ, ਪੂਰਨ lun)
{
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    ਜੇ (!pci)
        // support only pci machine for now
        ਵਾਪਸ -1;
    ਅੱਖਰ desc[256];
    build_scsi_ਰਸਤਾ(desc, ਆਕਾਰ(desc), pci, ਨਿਸ਼ਾਨਾ, lun);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਪੂਰਨ bootprio_ਲੱਭੋ_scsi_mmio_device(ਖਾਲੀ *mmio, ਪੂਰਨ ਨਿਸ਼ਾਨਾ, ਪੂਰਨ lun)
{
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    ਅੱਖਰ desc[256];
    snprintf(desc, ਆਕਾਰ(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, ਨਿਸ਼ਾਨਾ, lun);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਪੂਰਨ bootprio_ਲੱਭੋ_ata_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ slave)
{
    ਜੇ (CONFIG_CSM)
        ਵਾਪਸ csm_bootprio_ata(pci, chanid, slave);
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    ਜੇ (!pci)
        // support only pci machine for now
        ਵਾਪਸ -1;
    ਅੱਖਰ desc[256];
    build_ata_ਰਸਤਾ(desc, ਆਕਾਰ(desc), pci, chanid, slave);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਪੂਰਨ bootprio_ਲੱਭੋ_fdc_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ fdid)
{
    ਜੇ (CONFIG_CSM)
        ਵਾਪਸ csm_bootprio_fdc(pci, ਪੋਰਟ, fdid);
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    ਜੇ (!pci)
        // support only pci machine for now
        ਵਾਪਸ -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    ਅੱਖਰ desc[256], *p;
    p = build_pci_ਰਸਤਾ(desc, ਆਕਾਰ(desc), "isa", pci);
    snprintf(p, desc+ਆਕਾਰ(desc)-p, "/fdc@%04x/floppy@%x", ਪੋਰਟ, fdid);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਪੂਰਨ bootprio_ਲੱਭੋ_pci_rom(ਬਣਤਰ pci_device *pci, ਪੂਰਨ instance)
{
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    ਅੱਖਰ desc[256], *p;
    p = build_pci_ਰਸਤਾ(desc, ਆਕਾਰ(desc), "*", pci);
    ਜੇ (instance)
        snprintf(p, desc+ਆਕਾਰ(desc)-p, ":rom%x", instance);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਪੂਰਨ bootprio_ਲੱਭੋ_named_rom(ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ instance)
{
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    ਅੱਖਰ desc[256], *p;
    p = desc + snprintf(desc, ਆਕਾਰ(desc), "/rom@%s", name);
    ਜੇ (instance)
        snprintf(p, desc+ਆਕਾਰ(desc)-p, ":rom%x", instance);
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}

ਸਥਿਰ ਪੂਰਨ usb_portmap(ਬਣਤਰ usbdevice_s *usbdev)
{
    ਜੇ (usbdev->hub->op->portmap)
        ਵਾਪਸ usbdev->hub->op->portmap(usbdev->hub, usbdev->ਪੋਰਟ);
    ਵਾਪਸ usbdev->ਪੋਰਟ + 1;
}

ਸਥਿਰ ਅੱਖਰ *
build_usb_ਰਸਤਾ(ਅੱਖਰ *buf, ਪੂਰਨ max, ਬਣਤਰ usbhub_s *hub)
{
    ਜੇ (!hub->usbdev)
        // Root hub - nothing to add.
        ਵਾਪਸ buf;
    ਅੱਖਰ *p = build_usb_ਰਸਤਾ(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    ਵਾਪਸ p;
}

ਪੂਰਨ bootprio_ਲੱਭੋ_usb(ਬਣਤਰ usbdevice_s *usbdev, ਪੂਰਨ lun)
{
    ਜੇ (!CONFIG_BOOTORDER)
        ਵਾਪਸ -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    ਅੱਖਰ desc[256], *p;

    ਜੇ (usbdev->hub->cntl->pci)
        p = build_pci_ਰਸਤਾ(desc, ਆਕਾਰ(desc), "usb", usbdev->hub->cntl->pci);
    ਨਹੀਂਤਾਂ ਜੇ (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, ਆਕਾਰ(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ -1;

    p = build_usb_ਰਸਤਾ(p, desc+ਆਕਾਰ(desc)-p, usbdev->hub);
    snprintf(p, desc+ਆਕਾਰ(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    ਪੂਰਨ ret = ਲੱਭੋ_prio(desc);
    ਜੇ (ret >= 0)
        ਵਾਪਸ ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+ਆਕਾਰ(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    ਵਾਪਸ ਲੱਭੋ_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ Boot_Retry_ਸਮਾਂ;
ਸਥਿਰ ਪੂਰਨ CheckFloppySig = 1;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_PRIO           9999

ਸਥਿਰ ਪੂਰਨ DefaultFloppyPrio = 101;
ਸਥਿਰ ਪੂਰਨ DefaultCDPrio     = 102;
ਸਥਿਰ ਪੂਰਨ DefaultHDPrio     = 103;
ਸਥਿਰ ਪੂਰਨ DefaultBEVPrio    = 104;

ਖਾਲੀ
boot_init(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_BOOT)
        ਵਾਪਸ;

    ਜੇ (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        ਜੇ (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        ਪੂਰਨ i;
        ਲਈ (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            ਚੋਣ (val) {
            ਹਾਲਤ 1: DefaultFloppyPrio = i; ਰੋਕੋ;
            ਹਾਲਤ 2: DefaultHDPrio = i;     ਰੋਕੋ;
            ਹਾਲਤ 3: DefaultCDPrio = i;     ਰੋਕੋ;
            ਹਾਲਤ 4: DefaultBEVPrio = i;    ਰੋਕੋ;
            }
        }
    }

    Boot_Retry_ਸਮਾਂ = romfile_loadint("etc/boot-fail-wait", 60*1000);

    ਲੱਦੋ_Boot_ਕ੍ਰਮ();
    ਲੱਦੋ_Bios_Geometry();
}


/****************************************************************
 * BootList handling
 ****************************************************************/

ਬਣਤਰ bootentry_s {
    ਪੂਰਨ type;
    ਸੰਘ {
        u32 ਅੰਕੜੇ;
        ਬਣਤਰ segoff_s ਸਦਿਸ਼;
        ਬਣਤਰ drive_s *drive;
    };
    ਪੂਰਨ ਤਰਜੀਹ;
    ਅਚਲ ਅੱਖਰ *description;
    ਬਣਤਰ hlist_ਗੰਢ ਗੰਢ;
};
ਸਥਿਰ ਬਣਤਰ hlist_ਸਿਰ BootList VARVERIFY32INIT;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPL_TYPE_FLOPPY      0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPL_TYPE_HARDDISK    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPL_TYPE_CDROM       0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPL_TYPE_CBFS        0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPL_TYPE_BEV         0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPL_TYPE_BCV         0x81
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPL_TYPE_HALT        0xf0

ਸਥਿਰ ਖਾਲੀ
bootentry_ਜੋੜੋ(ਪੂਰਨ type, ਪੂਰਨ prio, u32 ਅੰਕੜੇ, ਅਚਲ ਅੱਖਰ *desc)
{
    ਜੇ (! CONFIG_BOOT)
        ਵਾਪਸ;
    ਬਣਤਰ bootentry_s *be = malloc_tmp(ਆਕਾਰ(*be));
    ਜੇ (!be) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    be->type = type;
    be->ਤਰਜੀਹ = prio;
    be->ਅੰਕੜੇ = ਅੰਕੜੇ;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, ਅੰਕੜੇ);

    // Add entry in sorted order.
    ਬਣਤਰ hlist_ਗੰਢ **pprev;
    ਬਣਤਰ bootentry_s *pos;
    hlist_for_each_ਇੰਦਰਾਜ_pprev(pos, pprev, &BootList, ਗੰਢ) {
        ਜੇ (be->ਤਰਜੀਹ < pos->ਤਰਜੀਹ)
            ਰੋਕੋ;
        ਜੇ (be->ਤਰਜੀਹ > pos->ਤਰਜੀਹ)
            ਜਾਰੀ;
        ਜੇ (be->type < pos->type)
            ਰੋਕੋ;
        ਜੇ (be->type > pos->type)
            ਜਾਰੀ;
        ਜੇ (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            ਰੋਕੋ;
    }
    hlist_ਜੋੜੋ(&be->ਗੰਢ, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ defPrio(ਪੂਰਨ ਤਰਜੀਹ, ਪੂਰਨ defaultprio) {
    ਵਾਪਸ (ਤਰਜੀਹ < 0) ? defaultprio : ਤਰਜੀਹ;
}

// Add a BEV vector for a given pnp compatible option rom.
ਖਾਲੀ
boot_ਜੋੜੋ_bev(u16 seg, u16 bev, u16 desc, ਪੂਰਨ prio)
{
    bootentry_ਜੋੜੋ(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
ਖਾਲੀ
boot_ਜੋੜੋ_bcv(u16 seg, u16 ip, u16 desc, ਪੂਰਨ prio)
{
    bootentry_ਜੋੜੋ(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

ਖਾਲੀ
boot_ਜੋੜੋ_floppy(ਬਣਤਰ drive_s *drive, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio)
{
    bootentry_ਜੋੜੋ(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

ਖਾਲੀ
boot_ਜੋੜੋ_hd(ਬਣਤਰ drive_s *drive, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio)
{
    bootentry_ਜੋੜੋ(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

ਖਾਲੀ
boot_ਜੋੜੋ_cd(ਬਣਤਰ drive_s *drive, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio)
{
    ਜੇ (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        ਅੱਖਰ *extra = cdrom_media_info(drive);
        ਜੇ (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            ਮੁਕਤ(extra);
        }
    }
    bootentry_ਜੋੜੋ(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
ਖਾਲੀ
boot_ਜੋੜੋ_cbfs(ਖਾਲੀ *ਅੰਕੜੇ, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio)
{
    bootentry_ਜੋੜੋ(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)ਅੰਕੜੇ, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
ਸਥਿਰ ਪੂਰਨ
check_for_keystroke(ਖਾਲੀ)
{
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    ਵਾਪਸ !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
ਸਥਿਰ ਪੂਰਨ
get_raw_keystroke(ਖਾਲੀ)
{
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    ਵਾਪਸ br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
ਪੂਰਨ
get_keystroke_ਭਰਿਆ(ਪੂਰਨ msec)
{
    u32 ਸਿਰਾ = irqtimer_calc(msec);
    ਲਈ (;;) {
        ਜੇ (check_for_keystroke())
            ਵਾਪਸ get_raw_keystroke();
        ਜੇ (irqtimer_check(ਸਿਰਾ))
            ਵਾਪਸ -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
ਪੂਰਨ
get_keystroke(ਪੂਰਨ msec)
{
    ਪੂਰਨ keystroke = get_keystroke_ਭਰਿਆ(msec);

    ਜੇ (keystroke < 0)
        ਵਾਪਸ keystroke;
    ਵਾਪਸ keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_BOOTMENU_WAIT 2500

ਸਥਿਰ ਅਚਲ ਅੱਖਰ menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
ਖਾਲੀ
interactive_bootmenu(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_BOOTMENU)
        ਵਾਪਸ;
    ਪੂਰਨ show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    ਜੇ (!show_boot_menu)
        ਵਾਪਸ;

    // skip menu if only one boot device and no TPM
    ਜੇ (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_ਖਾਲੀ_ਥਾਵਾਂ(&BootList) && !BootList.ਪਹਿਲਾ->ਅਗਲਾ) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        ਲਿਖੋ("\n");
        ਵਾਪਸ;
    }

    ਪੂਰਨ menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    ਪੂਰਨ menukey = romfile_loadint("etc/boot-menu-key", 1);
    ਪੂਰਨ ਜਾਂਚ_ਕੋਡ;
    ਜੇ (menutime >= 0) {
        ਜਦੋਂ (get_keystroke(0) >= 0)
            ;

        ਅੱਖਰ *bootmsg = romfile_loadfile("etc/boot-menu-message", ਨਿਰਾ);
        ਲਿਖੋ("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        ਮੁਕਤ(bootmsg);

        enable_bootsplash();
        ਜਾਂਚ_ਕੋਡ = get_keystroke(menutime);
        disable_bootsplash();
        ਜੇ (ਜਾਂਚ_ਕੋਡ != menukey)
            ਵਾਪਸ;
    }

    ਜਦੋਂ (get_keystroke(0) >= 0)
        ;

    ਲਿਖੋ("Select boot device:\n\n");
    wait_ਧਾਗੇ();

    // Show menu items
    ਪੂਰਨ maxmenu = 0;
    ਬਣਤਰ bootentry_s *pos, *boot = ਨਿਰਾ;
    hlist_for_each_ਇੰਦਰਾਜ(pos, &BootList, ਗੰਢ) {
        ਅੱਖਰ desc[77];
        ਜੇ (maxmenu >= ARRAY_ਮਾਪ(menuchars)) {
            ਰੋਕੋ;
        }
        ਲਿਖੋ("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_ਮਾਪ(desc)));
        maxmenu++;
    }
    ਜੇ (tpm_can_show_menu()) {
        ਲਿਖੋ("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    ਪੂਰਨ esc_accepted_ਸਮਾਂ = irqtimer_calc(menukey == 1 ? 1500 : 0);
    ਲਈ (;;) {
        ਪੂਰਨ keystroke = get_keystroke_ਭਰਿਆ(1000);
        ਜੇ (keystroke == 0x011b && !irqtimer_check(esc_accepted_ਸਮਾਂ))
            ਜਾਰੀ;
        ਜੇ (keystroke < 0) // timeout
            ਜਾਰੀ;

        ਜਾਂਚ_ਕੋਡ = keystroke >> 8;
        ਪੂਰਨ ਕੁੰਜੀ_ascii = keystroke & 0xff;
        ਜੇ (tpm_can_show_menu() && ਕੁੰਜੀ_ascii == 't') {
            ਲਿਖੋ("\n");
            tpm_menu();
        }
        ਜੇ (ਜਾਂਚ_ਕੋਡ == 1) {
            // ESC
            ਲਿਖੋ("\n");
            ਵਾਪਸ;
        }

        maxmenu = 0;
        hlist_for_each_ਇੰਦਰਾਜ(pos, &BootList, ਗੰਢ) {
            ਜੇ (maxmenu >= ARRAY_ਮਾਪ(menuchars))
                ਰੋਕੋ;
            ਜੇ (ਕੁੰਜੀ_ascii == menuchars[maxmenu]) {
                boot = pos;
                ਰੋਕੋ;
            }
            maxmenu++;
        }
        ਜੇ (boot)
            ਰੋਕੋ;
    }
    ਲਿਖੋ("\n");

    // Find entry and make top priority.
    hlist_del(&boot->ਗੰਢ);
    boot->ਤਰਜੀਹ = 0;
    hlist_ਜੋੜੋ_ਸਿਰ(&boot->ਗੰਢ, &BootList);
}

// BEV (Boot Execution Vector) list
ਬਣਤਰ bev_s {
    ਪੂਰਨ type;
    u32 ਸਦਿਸ਼;
};
ਸਥਿਰ ਬਣਤਰ bev_s BEV[20];
ਸਥਿਰ ਪੂਰਨ BEVCount;
ਸਥਿਰ ਪੂਰਨ HaveHDBoot, HaveFDBoot;

ਸਥਿਰ ਖਾਲੀ
ਜੋੜੋ_bev(ਪੂਰਨ type, u32 ਸਦਿਸ਼)
{
    ਜੇ (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        ਵਾਪਸ;
    ਜੇ (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        ਵਾਪਸ;
    ਜੇ (BEVCount >= ARRAY_ਮਾਪ(BEV))
        ਵਾਪਸ;
    ਬਣਤਰ bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->ਸਦਿਸ਼ = ਸਦਿਸ਼;
}

// Prepare for boot - show menu and run bcvs.
ਖਾਲੀ
bcv_prepboot(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_BOOT)
        ਵਾਪਸ;

    ਪੂਰਨ haltprio = ਲੱਭੋ_prio("HALT");
    ਜੇ (haltprio >= 0)
        bootentry_ਜੋੜੋ(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    ਬਣਤਰ bootentry_s *pos;
    hlist_for_each_ਇੰਦਰਾਜ(pos, &BootList, ਗੰਢ) {
        ਚੋਣ (pos->type) {
        ਹਾਲਤ IPL_TYPE_BCV:
            call_bcv(pos->ਸਦਿਸ਼.seg, pos->ਸਦਿਸ਼.ਵਿੱਥ_2);
            ਜੋੜੋ_bev(IPL_TYPE_HARDDISK, 0);
            ਰੋਕੋ;
        ਹਾਲਤ IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            ਜੋੜੋ_bev(IPL_TYPE_FLOPPY, 0);
            ਰੋਕੋ;
        ਹਾਲਤ IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            ਜੋੜੋ_bev(IPL_TYPE_HARDDISK, 0);
            ਰੋਕੋ;
        ਹਾਲਤ IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        ਮੂਲ:
            ਜੋੜੋ_bev(pos->type, pos->ਅੰਕੜੇ);
            ਰੋਕੋ;
        }
    }

    // If nothing added a floppy/hd boot - add it manually.
    ਜੋੜੋ_bev(IPL_TYPE_FLOPPY, 0);
    ਜੋੜੋ_bev(IPL_TYPE_HARDDISK, 0);
}


/****************************************************************
 * Boot code (int 18/19)
 ****************************************************************/

// Jump to a bootup entry point.
ਸਥਿਰ ਖਾਲੀ
call_boot_ਇੰਦਰਾਜ(ਬਣਤਰ segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.ਵਿੱਥ_2);
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    br.ਕੋਡ = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
ਸਥਿਰ ਖਾਲੀ
boot_disk(u8 bootdrv, ਪੂਰਨ checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    ਜੇ (br.flags & F_CF) {
        ਲਿਖੋ("Boot failed: could not read the boot disk\n\n");
        ਵਾਪਸ;
    }

    ਜੇ (checksig) {
        ਬਣਤਰ mbr_s *mbr = (ਖਾਲੀ*)0;
        ਜੇ (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            ਲਿਖੋ("Boot failed: not a bootable disk\n\n");
            ਵਾਪਸ;
        }
    }

    tpm_ਜੋੜੋ_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_ਇੰਦਰਾਜ(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
ਸਥਿਰ ਖਾਲੀ
boot_cdrom(ਬਣਤਰ drive_s *drive)
{
    ਜੇ (! CONFIG_CDROM_BOOT)
        ਵਾਪਸ;
    ਲਿਖੋ("Booting from DVD/CD...\n");

    ਪੂਰਨ ਸਥਿਤੀ = cdrom_boot(drive);
    ਜੇ (ਸਥਿਤੀ) {
        ਲਿਖੋ("Boot failed: Could not read from CDROM (code %04x)\n", ਸਥਿਤੀ);
        ਵਾਪਸ;
    }

    u8 bootdrv = CDEmu.emulated_drive;
    u16 bootseg = CDEmu.ਲੱਦੋ_segment;

    tpm_ਜੋੜੋ_cdrom(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_ਇੰਦਰਾਜ(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CBFS payload
ਸਥਿਰ ਖਾਲੀ
boot_cbfs(ਬਣਤਰ cbfs_file *file)
{
    ਜੇ (!CONFIG_COREBOOT_FLASH)
        ਵਾਪਸ;
    ਲਿਖੋ("Booting from CBFS...\n");
    cbfs_ਲੜੀਵਾਰ_payload(file);
}

// Boot from a BEV entry on an optionrom.
ਸਥਿਰ ਖਾਲੀ
boot_rom(u32 ਸਦਿਸ਼)
{
    ਲਿਖੋ("Booting from ROM...\n");
    ਬਣਤਰ segoff_s so;
    so.segoff = ਸਦਿਸ਼;
    call_boot_ਇੰਦਰਾਜ(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
ਸਥਿਰ ਖਾਲੀ
boot_fail(ਖਾਲੀ)
{
    ਜੇ (Boot_Retry_ਸਮਾਂ == (u32)-1)
        ਲਿਖੋ("No bootable device.\n");
    ਨਹੀਂਤਾਂ
        ਲਿਖੋ("No bootable device.  Retrying in %d seconds.\n"
               , Boot_Retry_ਸਮਾਂ/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 ਸਿਰਾ = irqtimer_calc(Boot_Retry_ਸਮਾਂ);
    ਲਈ (;;) {
        ਜੇ (Boot_Retry_ਸਮਾਂ != (u32)-1 && irqtimer_check(ਸਿਰਾ))
            ਰੋਕੋ;
        yield_toirq();
    }
    ਲਿਖੋ("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
ਸਥਿਰ ਖਾਲੀ
do_boot(ਪੂਰਨ seq_nr)
{
    ਜੇ (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    ਜੇ (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    ਬਣਤਰ bev_s *ie = &BEV[seq_nr];
    ਚੋਣ (ie->type) {
    ਹਾਲਤ IPL_TYPE_FLOPPY:
        ਲਿਖੋ("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        ਰੋਕੋ;
    ਹਾਲਤ IPL_TYPE_HARDDISK:
        ਲਿਖੋ("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        ਰੋਕੋ;
    ਹਾਲਤ IPL_TYPE_CDROM:
        boot_cdrom((ਖਾਲੀ*)ie->ਸਦਿਸ਼);
        ਰੋਕੋ;
    ਹਾਲਤ IPL_TYPE_CBFS:
        boot_cbfs((ਖਾਲੀ*)ie->ਸਦਿਸ਼);
        ਰੋਕੋ;
    ਹਾਲਤ IPL_TYPE_BEV:
        boot_rom(ie->ਸਦਿਸ਼);
        ਰੋਕੋ;
    ਹਾਲਤ IPL_TYPE_HALT:
        boot_fail();
        ਰੋਕੋ;
    }

    // Boot failed: invoke the boot recovery function
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

ਪੂਰਨ BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
ਖਾਲੀ VISIBLE32FLAT
handle_18(ਖਾਲੀ)
{
    debug_enter(ਨਿਰਾ, DEBUG_HDL_18);
    ਪੂਰਨ seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
ਖਾਲੀ VISIBLE32FLAT
handle_19(ਖਾਲੀ)
{
    debug_enter(ਨਿਰਾ, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
