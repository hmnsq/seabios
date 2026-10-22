// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // qemu_cfg_show_boot_menu
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_bdf_to_*
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // struct pci_device
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // rtc_read
#ಅನ್ತರ್ಭಾವ "hw/usb.h" // struct usbdevice_s
#ಅನ್ತರ್ಭಾವ "list.h" // hlist_node
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "std/disk.h" // struct mbr_s
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // irqtimer_calc
#ಅನ್ತರ್ಭಾವ "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
ಸ್ಥಿರ ವರ್ಣ *
glob_prefix(ಧ್ರುವ ವರ್ಣ *glob, ಧ್ರುವ ವರ್ಣ *str)
{
    ಪ್ರತಿ (;;) {
        ಯದಿ (!*glob && (!*str || *str == '/'))
            ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ*)str;
        ಯದಿ (*glob == '*') {
            ಯದಿ (!*str || *str == '/' || *str == glob[1])
                glob++;
            ಅನ್ಯಥಾ
                str++;
            ಅನುವರ್ತ;
        }
        ಯದಿ (*glob != *str)
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

ಸ್ಥಿರ ವರ್ಣ *
build_pci_path(ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ max, ಧ್ರುವ ವರ್ಣ *devname, ಸಂರಚನಾ pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    ವರ್ಣ *p = buf;
    ಯದಿ (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } ಅನ್ಯಥಾ {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        ಯದಿ (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    ಪೂರ್ಣಾಙ್ಕ dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    ಯದಿ (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    ಪ್ರತ್ಯಾಗಮ p;
}

ಸ್ಥಿರ ವರ್ಣ *
build_scsi_path(ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ max,
                ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ target, ಪೂರ್ಣಾಙ್ಕ lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ವರ್ಣ *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    ಪ್ರತ್ಯಾಗಮ p;
}

ಸ್ಥಿರ ವರ್ಣ *
build_ata_path(ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ max,
               ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    ವರ್ಣ *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    ಪ್ರತ್ಯಾಗಮ p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ BootDeviceLCHS {
    ವರ್ಣ *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

ಸ್ಥಿರ BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ Bios_Geometry_ಎಣಿಕೆ;

ಸ್ಥಿರ ವರ್ಣ *
parse_u32(ವರ್ಣ *cur, u32 *n)
{
    u32 m = 0;
    ಯದಿ (cur) {
        ಯಾವತ್ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        ಯದಿ (*cur != '\0')
            cur++;
    }
    *n = m;
    ಪ್ರತ್ಯಾಗಮ cur;
}

ಸ್ಥಿರ ಶೂನ್ಯ
loadBiosGeometry(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಪ್ರತ್ಯಾಗಮ;
    ವರ್ಣ *f = romfile_loadfile("bios-geometry", ರಿಕ್ತ);
    ಯದಿ (!f)
        ಪ್ರತ್ಯಾಗಮ;

    ಪೂರ್ಣಾಙ್ಕ i = 0;
    Bios_Geometry_ಎಣಿಕೆ = 1;
    ಯಾವತ್ (f[i]) {
        ಯದಿ (f[i] == '\n')
            Bios_Geometry_ಎಣಿಕೆ++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_ಎಣಿಕೆ * ಪರಿಮಾಣ(BootDeviceLCHS));
    ಯದಿ (!BiosGeometry) {
        warn_noalloc();
        ಮುಕ್ತ(f);
        Bios_Geometry_ಎಣಿಕೆ = 0;
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    ಕುರು {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        ಯದಿ (f)
            *(f++) = '\0';
        ವರ್ಣ *chs_ಮೌಲ್ಯಗಳು = strchr(d->name, ' ');
        ಯದಿ (chs_ಮೌಲ್ಯಗಳು)
            *(chs_ಮೌಲ್ಯಗಳು++) = '\0';
        chs_ಮೌಲ್ಯಗಳು = parse_u32(chs_ಮೌಲ್ಯಗಳು, &d->lcyls);
        chs_ಮೌಲ್ಯಗಳು = parse_u32(chs_ಮೌಲ್ಯಗಳು, &d->lheads);
        chs_ಮೌಲ್ಯಗಳು = parse_u32(chs_ಮೌಲ್ಯಗಳು, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } ಯಾವತ್ (f);
}

// Search the bios-geometry list for the given glob pattern.
ಸ್ಥಿರ BootDeviceLCHS *
boot_lchs_find(ಧ್ರುವ ವರ್ಣ *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < Bios_Geometry_ಎಣಿಕೆ; i++)
        ಯದಿ (glob_prefix(glob, BiosGeometry[i].name))
            ಪ್ರತ್ಯಾಗಮ &BiosGeometry[i];
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ boot_lchs_find_pci_device(ಸಂರಚನಾ pci_device *pci, ಸಂರಚನಾ chs_s *chs)
{
    ಯದಿ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಪ್ರತ್ಯಾಗಮ -1;
    ವರ್ಣ desc[256];
    build_pci_path(desc, ಪರಿಮಾಣ(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ಯದಿ (!b)
        ಪ್ರತ್ಯಾಗಮ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ boot_lchs_find_scsi_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ target, ಪೂರ್ಣಾಙ್ಕ lun,
                               ಸಂರಚನಾ chs_s *chs)
{
    ಯದಿ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!pci)
        // support only pci machine for now
        ಪ್ರತ್ಯಾಗಮ -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ವರ್ಣ desc[256];
    build_scsi_path(desc, ಪರಿಮಾಣ(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ಯದಿ (!b)
        ಪ್ರತ್ಯಾಗಮ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ boot_lchs_find_ata_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ slave,
                              ಸಂರಚನಾ chs_s *chs)
{
    ಯದಿ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!pci)
        // support only pci machine for now
        ಪ್ರತ್ಯಾಗಮ -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    ವರ್ಣ desc[256];
    build_ata_path(desc, ಪರಿಮಾಣ(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ಯದಿ (!b)
        ಪ್ರತ್ಯಾಗಮ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ಪ್ರತ್ಯಾಗಮ 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

ಸ್ಥಿರ ವರ್ಣ **Bootorder VARVERIFY32INIT;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ Bootorder_ಎಣಿಕೆ;

ಸ್ಥಿರ ಶೂನ್ಯ
loadBootOrder(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ;

    ವರ್ಣ *f = romfile_loadfile("bootorder", ರಿಕ್ತ);
    ಯದಿ (!f)
        ಪ್ರತ್ಯಾಗಮ;

    ಪೂರ್ಣಾಙ್ಕ i = 0;
    Bootorder_ಎಣಿಕೆ = 1;
    ಯಾವತ್ (f[i]) {
        ಯದಿ (f[i] == '\n')
            Bootorder_ಎಣಿಕೆ++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_ಎಣಿಕೆ*ಪರಿಮಾಣ(ವರ್ಣ*));
    ಯದಿ (!Bootorder) {
        warn_noalloc();
        ಮುಕ್ತ(f);
        Bootorder_ಎಣಿಕೆ = 0;
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    ಕುರು {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        ಯದಿ (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } ಯಾವತ್ (f);
}

// Search the bootorder list for the given glob pattern.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
find_prio(ಧ್ರುವ ವರ್ಣ *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < Bootorder_ಎಣಿಕೆ; i++)
        ಯದಿ (glob_prefix(glob, Bootorder[i]))
            ಪ್ರತ್ಯಾಗಮ i+1;
    ಪ್ರತ್ಯಾಗಮ -1;
}

u8 is_bootprio_strict(ಶೂನ್ಯ)
{
    ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ prio_halt = -2;

    ಯದಿ (prio_halt == -2)
        prio_halt = find_prio("HALT");
    ಪ್ರತ್ಯಾಗಮ prio_halt >= 0;
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_pci_device(ಸಂರಚನಾ pci_device *pci)
{
    ಯದಿ (CONFIG_CSM)
        ಪ್ರತ್ಯಾಗಮ csm_bootprio_pci(pci);
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    ವರ್ಣ desc[256];
    build_pci_path(desc, ಪರಿಮಾಣ(desc), "*", pci);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_mmio_device(ಶೂನ್ಯ *mmio)
{
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    ವರ್ಣ desc[256];
    snprintf(desc, ಪರಿಮಾಣ(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_scsi_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ target, ಪೂರ್ಣಾಙ್ಕ lun)
{
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!pci)
        // support only pci machine for now
        ಪ್ರತ್ಯಾಗಮ -1;
    ವರ್ಣ desc[256];
    build_scsi_path(desc, ಪರಿಮಾಣ(desc), pci, target, lun);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_scsi_mmio_device(ಶೂನ್ಯ *mmio, ಪೂರ್ಣಾಙ್ಕ target, ಪೂರ್ಣಾಙ್ಕ lun)
{
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    ವರ್ಣ desc[256];
    snprintf(desc, ಪರಿಮಾಣ(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_ata_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ slave)
{
    ಯದಿ (CONFIG_CSM)
        ಪ್ರತ್ಯಾಗಮ csm_bootprio_ata(pci, chanid, slave);
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!pci)
        // support only pci machine for now
        ಪ್ರತ್ಯಾಗಮ -1;
    ವರ್ಣ desc[256];
    build_ata_path(desc, ಪರಿಮಾಣ(desc), pci, chanid, slave);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_fdc_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ port, ಪೂರ್ಣಾಙ್ಕ fdid)
{
    ಯದಿ (CONFIG_CSM)
        ಪ್ರತ್ಯಾಗಮ csm_bootprio_fdc(pci, port, fdid);
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!pci)
        // support only pci machine for now
        ಪ್ರತ್ಯಾಗಮ -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    ವರ್ಣ desc[256], *p;
    p = build_pci_path(desc, ಪರಿಮಾಣ(desc), "isa", pci);
    snprintf(p, desc+ಪರಿಮಾಣ(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_pci_rom(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ instance)
{
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    ವರ್ಣ desc[256], *p;
    p = build_pci_path(desc, ಪರಿಮಾಣ(desc), "*", pci);
    ಯದಿ (instance)
        snprintf(p, desc+ಪರಿಮಾಣ(desc)-p, ":rom%x", instance);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_named_rom(ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ instance)
{
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    ವರ್ಣ desc[256], *p;
    p = desc + snprintf(desc, ಪರಿಮಾಣ(desc), "/rom@%s", name);
    ಯದಿ (instance)
        snprintf(p, desc+ಪರಿಮಾಣ(desc)-p, ":rom%x", instance);
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_portmap(ಸಂರಚನಾ usbdevice_s *usbdev)
{
    ಯದಿ (usbdev->hub->op->portmap)
        ಪ್ರತ್ಯಾಗಮ usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    ಪ್ರತ್ಯಾಗಮ usbdev->port + 1;
}

ಸ್ಥಿರ ವರ್ಣ *
build_usb_path(ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ max, ಸಂರಚನಾ usbhub_s *hub)
{
    ಯದಿ (!hub->usbdev)
        // Root hub - nothing to add.
        ಪ್ರತ್ಯಾಗಮ buf;
    ವರ್ಣ *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    ಪ್ರತ್ಯಾಗಮ p;
}

ಪೂರ್ಣಾಙ್ಕ bootprio_find_usb(ಸಂರಚನಾ usbdevice_s *usbdev, ಪೂರ್ಣಾಙ್ಕ lun)
{
    ಯದಿ (!CONFIG_BOOTORDER)
        ಪ್ರತ್ಯಾಗಮ -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    ವರ್ಣ desc[256], *p;

    ಯದಿ (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, ಪರಿಮಾಣ(desc), "usb", usbdev->hub->cntl->pci);
    ಅನ್ಯಥಾ ಯದಿ (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, ಪರಿಮಾಣ(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    ಅನ್ಯಥಾ
        ಪ್ರತ್ಯಾಗಮ -1;

    p = build_usb_path(p, desc+ಪರಿಮಾಣ(desc)-p, usbdev->hub);
    snprintf(p, desc+ಪರಿಮಾಣ(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    ಪೂರ್ಣಾಙ್ಕ ret = find_prio(desc);
    ಯದಿ (ret >= 0)
        ಪ್ರತ್ಯಾಗಮ ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+ಪರಿಮಾಣ(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    ಪ್ರತ್ಯಾಗಮ find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ BootRetryTime;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ DefaultFloppyPrio = 101;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ DefaultCDPrio     = 102;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ DefaultHDPrio     = 103;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ DefaultBEVPrio    = 104;

ಶೂನ್ಯ
boot_init(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_BOOT)
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        ಯದಿ (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            ವಿಕಲ್ಪ (val) {
            ಪಕ್ಷ 1: DefaultFloppyPrio = i; ಭಙ್ಗ;
            ಪಕ್ಷ 2: DefaultHDPrio = i;     ಭಙ್ಗ;
            ಪಕ್ಷ 3: DefaultCDPrio = i;     ಭಙ್ಗ;
            ಪಕ್ಷ 4: DefaultBEVPrio = i;    ಭಙ್ಗ;
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

ಸಂರಚನಾ bootentry_s {
    ಪೂರ್ಣಾಙ್ಕ type;
    ಸಙ್ಘ {
        u32 data;
        ಸಂರಚನಾ segoff_s vector;
        ಸಂರಚನಾ drive_s *drive;
    };
    ಪೂರ್ಣಾಙ್ಕ priority;
    ಧ್ರುವ ವರ್ಣ *description;
    ಸಂರಚನಾ hlist_ಗಂಟು ಗಂಟು;
};
ಸ್ಥಿರ ಸಂರಚನಾ hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

ಸ್ಥಿರ ಶೂನ್ಯ
bootentry_add(ಪೂರ್ಣಾಙ್ಕ type, ಪೂರ್ಣಾಙ್ಕ prio, u32 data, ಧ್ರುವ ವರ್ಣ *desc)
{
    ಯದಿ (! CONFIG_BOOT)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ bootentry_s *be = malloc_tmp(ಪರಿಮಾಣ(*be));
    ಯದಿ (!be) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    ಸಂರಚನಾ hlist_ಗಂಟು **pprev;
    ಸಂರಚನಾ bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, ಗಂಟು) {
        ಯದಿ (be->priority < pos->priority)
            ಭಙ್ಗ;
        ಯದಿ (be->priority > pos->priority)
            ಅನುವರ್ತ;
        ಯದಿ (be->type < pos->type)
            ಭಙ್ಗ;
        ಯದಿ (be->type > pos->type)
            ಅನುವರ್ತ;
        ಯದಿ (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            ಭಙ್ಗ;
    }
    hlist_add(&be->ಗಂಟು, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ defPrio(ಪೂರ್ಣಾಙ್ಕ priority, ಪೂರ್ಣಾಙ್ಕ defaultprio) {
    ಪ್ರತ್ಯಾಗಮ (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
ಶೂನ್ಯ
boot_add_bev(u16 seg, u16 bev, u16 desc, ಪೂರ್ಣಾಙ್ಕ prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
ಶೂನ್ಯ
boot_add_bcv(u16 seg, u16 ip, u16 desc, ಪೂರ್ಣಾಙ್ಕ prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

ಶೂನ್ಯ
boot_add_floppy(ಸಂರಚನಾ drive_s *drive, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

ಶೂನ್ಯ
boot_add_hd(ಸಂರಚನಾ drive_s *drive, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

ಶೂನ್ಯ
boot_add_cd(ಸಂರಚನಾ drive_s *drive, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio)
{
    ಯದಿ (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        ವರ್ಣ *extra = cdrom_media_info(drive);
        ಯದಿ (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            ಮುಕ್ತ(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
ಶೂನ್ಯ
boot_add_cbfs(ಶೂನ್ಯ *data, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
check_for_keystroke(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    ಪ್ರತ್ಯಾಗಮ !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
get_raw_keystroke(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    ಪ್ರತ್ಯಾಗಮ br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
ಪೂರ್ಣಾಙ್ಕ
get_keystroke_full(ಪೂರ್ಣಾಙ್ಕ msec)
{
    u32 end = irqtimer_calc(msec);
    ಪ್ರತಿ (;;) {
        ಯದಿ (check_for_keystroke())
            ಪ್ರತ್ಯಾಗಮ get_raw_keystroke();
        ಯದಿ (irqtimer_check(end))
            ಪ್ರತ್ಯಾಗಮ -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
ಪೂರ್ಣಾಙ್ಕ
get_keystroke(ಪೂರ್ಣಾಙ್ಕ msec)
{
    ಪೂರ್ಣಾಙ್ಕ keystroke = get_keystroke_full(msec);

    ಯದಿ (keystroke < 0)
        ಪ್ರತ್ಯಾಗಮ keystroke;
    ಪ್ರತ್ಯಾಗಮ keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
ಶೂನ್ಯ
interactive_bootmenu(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_BOOTMENU)
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    ಯದಿ (!show_boot_menu)
        ಪ್ರತ್ಯಾಗಮ;

    // skip menu if only one boot device and no TPM
    ಯದಿ (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಪೂರ್ಣಾಙ್ಕ menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    ಪೂರ್ಣಾಙ್ಕ menukey = romfile_loadint("etc/boot-menu-key", 1);
    ಪೂರ್ಣಾಙ್ಕ scan_code;
    ಯದಿ (menutime >= 0) {
        ಯಾವತ್ (get_keystroke(0) >= 0)
            ;

        ವರ್ಣ *bootmsg = romfile_loadfile("etc/boot-menu-message", ರಿಕ್ತ);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        ಮುಕ್ತ(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        ಯದಿ (scan_code != menukey)
            ಪ್ರತ್ಯಾಗಮ;
    }

    ಯಾವತ್ (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    ಪೂರ್ಣಾಙ್ಕ maxmenu = 0;
    ಸಂರಚನಾ bootentry_s *pos, *boot = ರಿಕ್ತ;
    hlist_for_each_entry(pos, &BootList, ಗಂಟು) {
        ವರ್ಣ desc[77];
        ಯದಿ (maxmenu >= ARRAY_SIZE(menuchars)) {
            ಭಙ್ಗ;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    ಯದಿ (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    ಪೂರ್ಣಾಙ್ಕ esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    ಪ್ರತಿ (;;) {
        ಪೂರ್ಣಾಙ್ಕ keystroke = get_keystroke_full(1000);
        ಯದಿ (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            ಅನುವರ್ತ;
        ಯದಿ (keystroke < 0) // timeout
            ಅನುವರ್ತ;

        scan_code = keystroke >> 8;
        ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ_ascii = keystroke & 0xff;
        ಯದಿ (tpm_can_show_menu() && ಕೀಲಿ_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        ಯದಿ (scan_code == 1) {
            // ESC
            printf("\n");
            ಪ್ರತ್ಯಾಗಮ;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, ಗಂಟು) {
            ಯದಿ (maxmenu >= ARRAY_SIZE(menuchars))
                ಭಙ್ಗ;
            ಯದಿ (ಕೀಲಿ_ascii == menuchars[maxmenu]) {
                boot = pos;
                ಭಙ್ಗ;
            }
            maxmenu++;
        }
        ಯದಿ (boot)
            ಭಙ್ಗ;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->ಗಂಟು);
    boot->priority = 0;
    hlist_add_head(&boot->ಗಂಟು, &BootList);
}

// BEV (Boot Execution Vector) list
ಸಂರಚನಾ bev_s {
    ಪೂರ್ಣಾಙ್ಕ type;
    u32 vector;
};
ಸ್ಥಿರ ಸಂರಚನಾ bev_s BEV[20];
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ BEVCount;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ HaveHDBoot, HaveFDBoot;

ಸ್ಥಿರ ಶೂನ್ಯ
add_bev(ಪೂರ್ಣಾಙ್ಕ type, u32 vector)
{
    ಯದಿ (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (BEVCount >= ARRAY_SIZE(BEV))
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
ಶೂನ್ಯ
bcv_prepboot(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_BOOT)
        ಪ್ರತ್ಯಾಗಮ;

    ಪೂರ್ಣಾಙ್ಕ haltprio = find_prio("HALT");
    ಯದಿ (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    ಸಂರಚನಾ bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, ಗಂಟು) {
        ವಿಕಲ್ಪ (pos->type) {
        ಪಕ್ಷ IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            ಭಙ್ಗ;
        ಪಕ್ಷ IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            ಭಙ್ಗ;
        ಪಕ್ಷ IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            ಭಙ್ಗ;
        ಪಕ್ಷ IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        ಸ್ವಭಾವ:
            add_bev(pos->type, pos->data);
            ಭಙ್ಗ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
call_boot_entry(ಸಂರಚನಾ segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
ಸ್ಥಿರ ಶೂನ್ಯ
boot_disk(u8 bootdrv, ಪೂರ್ಣಾಙ್ಕ checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    ಯದಿ (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (checksig) {
        ಸಂರಚನಾ mbr_s *mbr = (ಶೂನ್ಯ*)0;
        ಯದಿ (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            ಪ್ರತ್ಯಾಗಮ;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
ಸ್ಥಿರ ಶೂನ್ಯ
boot_cdrom(ಸಂರಚನಾ drive_s *drive)
{
    ಯದಿ (! CONFIG_CDROM_BOOT)
        ಪ್ರತ್ಯಾಗಮ;
    printf("Booting from DVD/CD...\n");

    ಪೂರ್ಣಾಙ್ಕ status = cdrom_boot(drive);
    ಯದಿ (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
boot_cbfs(ಸಂರಚನಾ cbfs_file *file)
{
    ಯದಿ (!CONFIG_COREBOOT_FLASH)
        ಪ್ರತ್ಯಾಗಮ;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
ಸ್ಥಿರ ಶೂನ್ಯ
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    ಸಂರಚನಾ segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
ಸ್ಥಿರ ಶೂನ್ಯ
boot_fail(ಶೂನ್ಯ)
{
    ಯದಿ (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    ಅನ್ಯಥಾ
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    ಪ್ರತಿ (;;) {
        ಯದಿ (BootRetryTime != (u32)-1 && irqtimer_check(end))
            ಭಙ್ಗ;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
ಸ್ಥಿರ ಶೂನ್ಯ
do_boot(ಪೂರ್ಣಾಙ್ಕ seq_nr)
{
    ಯದಿ (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    ಯದಿ (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    ಸಂರಚನಾ bev_s *ie = &BEV[seq_nr];
    ವಿಕಲ್ಪ (ie->type) {
    ಪಕ್ಷ IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        ಭಙ್ಗ;
    ಪಕ್ಷ IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        ಭಙ್ಗ;
    ಪಕ್ಷ IPL_TYPE_CDROM:
        boot_cdrom((ಶೂನ್ಯ*)ie->vector);
        ಭಙ್ಗ;
    ಪಕ್ಷ IPL_TYPE_CBFS:
        boot_cbfs((ಶೂನ್ಯ*)ie->vector);
        ಭಙ್ಗ;
    ಪಕ್ಷ IPL_TYPE_BEV:
        boot_rom(ie->vector);
        ಭಙ್ಗ;
    ಪಕ್ಷ IPL_TYPE_HALT:
        boot_fail();
        ಭಙ್ಗ;
    }

    // Boot failed: invoke the boot recovery function
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

ಪೂರ್ಣಾಙ್ಕ BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
ಶೂನ್ಯ VISIBLE32FLAT
handle_18(ಶೂನ್ಯ)
{
    debug_enter(ರಿಕ್ತ, DEBUG_HDL_18);
    ಪೂರ್ಣಾಙ್ಕ seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
ಶೂನ್ಯ VISIBLE32FLAT
handle_19(ಶೂನ್ಯ)
{
    debug_enter(ರಿಕ್ತ, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
