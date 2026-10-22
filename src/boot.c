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
ಸ್ಥಿರ ಅಕ್ಷರ *
glob_ಪೂರ್ವಪ್ರತ್ಯಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *glob, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
    ಗಾಗಿ (;;) {
        ಆದರೆ (!*glob && (!*str || *str == '/'))
            ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)str;
        ಆದರೆ (*glob == '*') {
            ಆದರೆ (!*str || *str == '/' || *str == glob[1])
                glob++;
            ಇಲ್ಲದಿದ್ದರೆ
                str++;
            ಮುಂದುವರಿ;
        }
        ಆದರೆ (*glob != *str)
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        glob++;
        str++;
    }
}

#ವ್ಯಾಖ್ಯಾನಿಸು FW_PCI_DOMAIN "/pci@i0cf8"

ಸ್ಥಿರ ಅಕ್ಷರ *
build_pci_ಮಾರ್ಗ(ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ max, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ರಚನೆ pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    ಅಕ್ಷರ *p = buf;
    ಆದರೆ (pci->ಪೋಷಕ) {
        p = build_pci_ಮಾರ್ಗ(p, max, "pci-bridge", pci->ಪೋಷಕ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        ಆದರೆ (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    ಪೂರ್ಣಾಂಕ dev = pci_bdf_ಗಮ್ಯ_dev(pci->bdf), fn = pci_bdf_ಗಮ್ಯ_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    ಆದರೆ (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    ಹಿಂತಿರುಗಿ p;
}

ಸ್ಥಿರ ಅಕ್ಷರ *
build_scsi_ಮಾರ್ಗ(ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ max,
                ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಗುರಿ, ಪೂರ್ಣಾಂಕ lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ಅಕ್ಷರ *p;
    p = build_pci_ಮಾರ್ಗ(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", ಗುರಿ, lun);
    ಹಿಂತಿರುಗಿ p;
}

ಸ್ಥಿರ ಅಕ್ಷರ *
build_ata_ಮಾರ್ಗ(ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ max,
               ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    ಅಕ್ಷರ *p;
    p = build_pci_ಮಾರ್ಗ(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    ಹಿಂತಿರುಗಿ p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ BootDeviceLCHS {
    ಅಕ್ಷರ *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

ಸ್ಥಿರ BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Bios_Geometry_ಎಣಿಕೆ;

ಸ್ಥಿರ ಅಕ್ಷರ *
parse_u32(ಅಕ್ಷರ *cur, u32 *n)
{
    u32 m = 0;
    ಆದರೆ (cur) {
        ಆಗ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        ಆದರೆ (*cur != '\0')
            cur++;
    }
    *n = m;
    ಹಿಂತಿರುಗಿ cur;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ತುಂಬು_Bios_Geometry(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಹಿಂತಿರುಗಿ;
    ಅಕ್ಷರ *f = romfile_loadfile("bios-geometry", ಖಾಲಿ);
    ಆದರೆ (!f)
        ಹಿಂತಿರುಗಿ;

    ಪೂರ್ಣಾಂಕ i = 0;
    Bios_Geometry_ಎಣಿಕೆ = 1;
    ಆಗ (f[i]) {
        ಆದರೆ (f[i] == '\n')
            Bios_Geometry_ಎಣಿಕೆ++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_ಎಣಿಕೆ * ಗಾತ್ರ(BootDeviceLCHS));
    ಆದರೆ (!BiosGeometry) {
        warn_noalloc();
        ಬಿಡುಗಡೆ(f);
        Bios_Geometry_ಎಣಿಕೆ = 0;
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    ಮಾಡು {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        ಆದರೆ (f)
            *(f++) = '\0';
        ಅಕ್ಷರ *chs_ಮೌಲ್ಯಗಳು = strchr(d->name, ' ');
        ಆದರೆ (chs_ಮೌಲ್ಯಗಳು)
            *(chs_ಮೌಲ್ಯಗಳು++) = '\0';
        chs_ಮೌಲ್ಯಗಳು = parse_u32(chs_ಮೌಲ್ಯಗಳು, &d->lcyls);
        chs_ಮೌಲ್ಯಗಳು = parse_u32(chs_ಮೌಲ್ಯಗಳು, &d->lheads);
        chs_ಮೌಲ್ಯಗಳು = parse_u32(chs_ಮೌಲ್ಯಗಳು, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } ಆಗ (f);
}

// Search the bios-geometry list for the given glob pattern.
ಸ್ಥಿರ BootDeviceLCHS *
boot_lchs_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < Bios_Geometry_ಎಣಿಕೆ; i++)
        ಆದರೆ (glob_ಪೂರ್ವಪ್ರತ್ಯಯ(glob, BiosGeometry[i].name))
            ಹಿಂತಿರುಗಿ &BiosGeometry[i];
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ boot_lchs_ಕಂಡುಹಿಡಿ_pci_device(ರಚನೆ pci_device *pci, ರಚನೆ chs_s *chs)
{
    ಆದರೆ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಹಿಂತಿರುಗಿ -1;
    ಅಕ್ಷರ desc[256];
    build_pci_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_ಕಂಡುಹಿಡಿ(desc);
    ಆದರೆ (!b)
        ಹಿಂತಿರುಗಿ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->ತಲೆ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಗುರಿ, ಪೂರ್ಣಾಂಕ lun,
                               ರಚನೆ chs_s *chs)
{
    ಆದರೆ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!pci)
        // support only pci machine for now
        ಹಿಂತಿರುಗಿ -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ಅಕ್ಷರ desc[256];
    build_scsi_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), pci, ಗುರಿ, lun);
    BootDeviceLCHS *b = boot_lchs_ಕಂಡುಹಿಡಿ(desc);
    ಆದರೆ (!b)
        ಹಿಂತಿರುಗಿ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->ತಲೆ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ boot_lchs_ಕಂಡುಹಿಡಿ_ata_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ slave,
                              ರಚನೆ chs_s *chs)
{
    ಆದರೆ (!CONFIG_HOST_BIOS_GEOMETRY)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!pci)
        // support only pci machine for now
        ಹಿಂತಿರುಗಿ -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    ಅಕ್ಷರ desc[256];
    build_ata_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_ಕಂಡುಹಿಡಿ(desc);
    ಆದರೆ (!b)
        ಹಿಂತಿರುಗಿ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->ತಲೆ = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ಹಿಂತಿರುಗಿ 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

ಸ್ಥಿರ ಅಕ್ಷರ **Bootorder VARVERIFY32INIT;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Bootorder_ಎಣಿಕೆ;

ಸ್ಥಿರ ಶೂನ್ಯ
ತುಂಬು_Boot_ಕ್ರಮ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ;

    ಅಕ್ಷರ *f = romfile_loadfile("bootorder", ಖಾಲಿ);
    ಆದರೆ (!f)
        ಹಿಂತಿರುಗಿ;

    ಪೂರ್ಣಾಂಕ i = 0;
    Bootorder_ಎಣಿಕೆ = 1;
    ಆಗ (f[i]) {
        ಆದರೆ (f[i] == '\n')
            Bootorder_ಎಣಿಕೆ++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_ಎಣಿಕೆ*ಗಾತ್ರ(ಅಕ್ಷರ*));
    ಆದರೆ (!Bootorder) {
        warn_noalloc();
        ಬಿಡುಗಡೆ(f);
        Bootorder_ಎಣಿಕೆ = 0;
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    ಮಾಡು {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        ಆದರೆ (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } ಆಗ (f);
}

// Search the bootorder list for the given glob pattern.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಕಂಡುಹಿಡಿ_prio(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < Bootorder_ಎಣಿಕೆ; i++)
        ಆದರೆ (glob_ಪೂರ್ವಪ್ರತ್ಯಯ(glob, Bootorder[i]))
            ಹಿಂತಿರುಗಿ i+1;
    ಹಿಂತಿರುಗಿ -1;
}

u8 is_bootprio_strict(ಶೂನ್ಯ)
{
    ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ prio_halt = -2;

    ಆದರೆ (prio_halt == -2)
        prio_halt = ಕಂಡುಹಿಡಿ_prio("HALT");
    ಹಿಂತಿರುಗಿ prio_halt >= 0;
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_pci_device(ರಚನೆ pci_device *pci)
{
    ಆದರೆ (CONFIG_CSM)
        ಹಿಂತಿರುಗಿ csm_bootprio_pci(pci);
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    ಅಕ್ಷರ desc[256];
    build_pci_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), "*", pci);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_mmio_device(ಶೂನ್ಯ *mmio)
{
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    ಅಕ್ಷರ desc[256];
    snprintf(desc, ಗಾತ್ರ(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_scsi_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಗುರಿ, ಪೂರ್ಣಾಂಕ lun)
{
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!pci)
        // support only pci machine for now
        ಹಿಂತಿರುಗಿ -1;
    ಅಕ್ಷರ desc[256];
    build_scsi_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), pci, ಗುರಿ, lun);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_scsi_mmio_device(ಶೂನ್ಯ *mmio, ಪೂರ್ಣಾಂಕ ಗುರಿ, ಪೂರ್ಣಾಂಕ lun)
{
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    ಅಕ್ಷರ desc[256];
    snprintf(desc, ಗಾತ್ರ(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, ಗುರಿ, lun);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_ata_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ slave)
{
    ಆದರೆ (CONFIG_CSM)
        ಹಿಂತಿರುಗಿ csm_bootprio_ata(pci, chanid, slave);
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!pci)
        // support only pci machine for now
        ಹಿಂತಿರುಗಿ -1;
    ಅಕ್ಷರ desc[256];
    build_ata_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), pci, chanid, slave);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_fdc_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ fdid)
{
    ಆದರೆ (CONFIG_CSM)
        ಹಿಂತಿರುಗಿ csm_bootprio_fdc(pci, ಪೋರ್ಟ್, fdid);
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!pci)
        // support only pci machine for now
        ಹಿಂತಿರುಗಿ -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    ಅಕ್ಷರ desc[256], *p;
    p = build_pci_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), "isa", pci);
    snprintf(p, desc+ಗಾತ್ರ(desc)-p, "/fdc@%04x/floppy@%x", ಪೋರ್ಟ್, fdid);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_pci_rom(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ instance)
{
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    ಅಕ್ಷರ desc[256], *p;
    p = build_pci_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), "*", pci);
    ಆದರೆ (instance)
        snprintf(p, desc+ಗಾತ್ರ(desc)-p, ":rom%x", instance);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_named_rom(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ instance)
{
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    ಅಕ್ಷರ desc[256], *p;
    p = desc + snprintf(desc, ಗಾತ್ರ(desc), "/rom@%s", name);
    ಆದರೆ (instance)
        snprintf(p, desc+ಗಾತ್ರ(desc)-p, ":rom%x", instance);
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_portmap(ರಚನೆ usbdevice_s *usbdev)
{
    ಆದರೆ (usbdev->hub->op->portmap)
        ಹಿಂತಿರುಗಿ usbdev->hub->op->portmap(usbdev->hub, usbdev->ಪೋರ್ಟ್);
    ಹಿಂತಿರುಗಿ usbdev->ಪೋರ್ಟ್ + 1;
}

ಸ್ಥಿರ ಅಕ್ಷರ *
build_usb_ಮಾರ್ಗ(ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ max, ರಚನೆ usbhub_s *hub)
{
    ಆದರೆ (!hub->usbdev)
        // Root hub - nothing to add.
        ಹಿಂತಿರುಗಿ buf;
    ಅಕ್ಷರ *p = build_usb_ಮಾರ್ಗ(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    ಹಿಂತಿರುಗಿ p;
}

ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_usb(ರಚನೆ usbdevice_s *usbdev, ಪೂರ್ಣಾಂಕ lun)
{
    ಆದರೆ (!CONFIG_BOOTORDER)
        ಹಿಂತಿರುಗಿ -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    ಅಕ್ಷರ desc[256], *p;

    ಆದರೆ (usbdev->hub->cntl->pci)
        p = build_pci_ಮಾರ್ಗ(desc, ಗಾತ್ರ(desc), "usb", usbdev->hub->cntl->pci);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, ಗಾತ್ರ(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ -1;

    p = build_usb_ಮಾರ್ಗ(p, desc+ಗಾತ್ರ(desc)-p, usbdev->hub);
    snprintf(p, desc+ಗಾತ್ರ(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    ಪೂರ್ಣಾಂಕ ret = ಕಂಡುಹಿಡಿ_prio(desc);
    ಆದರೆ (ret >= 0)
        ಹಿಂತಿರುಗಿ ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+ಗಾತ್ರ(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Boot_Retry_ಸಮಯ;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ CheckFloppySig = 1;

#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_PRIO           9999

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DefaultFloppyPrio = 101;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DefaultCDPrio     = 102;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DefaultHDPrio     = 103;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DefaultBEVPrio    = 104;

ಶೂನ್ಯ
boot_init(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_BOOT)
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        ಆದರೆ (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            ಆಯ್ಕೆ (val) {
            ಸ್ಥಿತಿ 1: DefaultFloppyPrio = i; ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 2: DefaultHDPrio = i;     ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 3: DefaultCDPrio = i;     ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 4: DefaultBEVPrio = i;    ನಿಲ್ಲಿಸು;
            }
        }
    }

    Boot_Retry_ಸಮಯ = romfile_loadint("etc/boot-fail-wait", 60*1000);

    ತುಂಬು_Boot_ಕ್ರಮ();
    ತುಂಬು_Bios_Geometry();
}


/****************************************************************
 * BootList handling
 ****************************************************************/

ರಚನೆ bootentry_s {
    ಪೂರ್ಣಾಂಕ type;
    ಒಕ್ಕೂಟ {
        u32 ದತ್ತಾಂಶ;
        ರಚನೆ segoff_s ಸದಿಶ;
        ರಚನೆ drive_s *drive;
    };
    ಪೂರ್ಣಾಂಕ ಆದ್ಯತೆ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *description;
    ರಚನೆ hlist_ಗಂಟು ಗಂಟು;
};
ಸ್ಥಿರ ರಚನೆ hlist_ತಲೆ BootList VARVERIFY32INIT;

#ವ್ಯಾಖ್ಯಾನಿಸು IPL_TYPE_FLOPPY      0x01
#ವ್ಯಾಖ್ಯಾನಿಸು IPL_TYPE_HARDDISK    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು IPL_TYPE_CDROM       0x03
#ವ್ಯಾಖ್ಯಾನಿಸು IPL_TYPE_CBFS        0x20
#ವ್ಯಾಖ್ಯಾನಿಸು IPL_TYPE_BEV         0x80
#ವ್ಯಾಖ್ಯಾನಿಸು IPL_TYPE_BCV         0x81
#ವ್ಯಾಖ್ಯಾನಿಸು IPL_TYPE_HALT        0xf0

ಸ್ಥಿರ ಶೂನ್ಯ
bootentry_ಕೂಡಿಸು(ಪೂರ್ಣಾಂಕ type, ಪೂರ್ಣಾಂಕ prio, u32 ದತ್ತಾಂಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc)
{
    ಆದರೆ (! CONFIG_BOOT)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ bootentry_s *be = malloc_tmp(ಗಾತ್ರ(*be));
    ಆದರೆ (!be) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    be->type = type;
    be->ಆದ್ಯತೆ = prio;
    be->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, ದತ್ತಾಂಶ);

    // Add entry in sorted order.
    ರಚನೆ hlist_ಗಂಟು **pprev;
    ರಚನೆ bootentry_s *pos;
    hlist_for_each_ನಮೂದು_pprev(pos, pprev, &BootList, ಗಂಟು) {
        ಆದರೆ (be->ಆದ್ಯತೆ < pos->ಆದ್ಯತೆ)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (be->ಆದ್ಯತೆ > pos->ಆದ್ಯತೆ)
            ಮುಂದುವರಿ;
        ಆದರೆ (be->type < pos->type)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (be->type > pos->type)
            ಮುಂದುವರಿ;
        ಆದರೆ (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            ನಿಲ್ಲಿಸು;
    }
    hlist_ಕೂಡಿಸು(&be->ಗಂಟು, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ defPrio(ಪೂರ್ಣಾಂಕ ಆದ್ಯತೆ, ಪೂರ್ಣಾಂಕ defaultprio) {
    ಹಿಂತಿರುಗಿ (ಆದ್ಯತೆ < 0) ? defaultprio : ಆದ್ಯತೆ;
}

// Add a BEV vector for a given pnp compatible option rom.
ಶೂನ್ಯ
boot_ಕೂಡಿಸು_bev(u16 seg, u16 bev, u16 desc, ಪೂರ್ಣಾಂಕ prio)
{
    bootentry_ಕೂಡಿಸು(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
ಶೂನ್ಯ
boot_ಕೂಡಿಸು_bcv(u16 seg, u16 ip, u16 desc, ಪೂರ್ಣಾಂಕ prio)
{
    bootentry_ಕೂಡಿಸು(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

ಶೂನ್ಯ
boot_ಕೂಡಿಸು_floppy(ರಚನೆ drive_s *drive, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio)
{
    bootentry_ಕೂಡಿಸು(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

ಶೂನ್ಯ
boot_ಕೂಡಿಸು_hd(ರಚನೆ drive_s *drive, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio)
{
    bootentry_ಕೂಡಿಸು(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

ಶೂನ್ಯ
boot_ಕೂಡಿಸು_cd(ರಚನೆ drive_s *drive, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio)
{
    ಆದರೆ (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        ಅಕ್ಷರ *extra = cdrom_media_info(drive);
        ಆದರೆ (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            ಬಿಡುಗಡೆ(extra);
        }
    }
    bootentry_ಕೂಡಿಸು(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
ಶೂನ್ಯ
boot_ಕೂಡಿಸು_cbfs(ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio)
{
    bootentry_ಕೂಡಿಸು(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)ದತ್ತಾಂಶ, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
check_for_keystroke(ಶೂನ್ಯ)
{
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    ಹಿಂತಿರುಗಿ !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_raw_keystroke(ಶೂನ್ಯ)
{
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    ಹಿಂತಿರುಗಿ br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
ಪೂರ್ಣಾಂಕ
get_keystroke_ತುಂಬಿದ(ಪೂರ್ಣಾಂಕ msec)
{
    u32 ಕೊನೆ = irqtimer_calc(msec);
    ಗಾಗಿ (;;) {
        ಆದರೆ (check_for_keystroke())
            ಹಿಂತಿರುಗಿ get_raw_keystroke();
        ಆದರೆ (irqtimer_check(ಕೊನೆ))
            ಹಿಂತಿರುಗಿ -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
ಪೂರ್ಣಾಂಕ
get_keystroke(ಪೂರ್ಣಾಂಕ msec)
{
    ಪೂರ್ಣಾಂಕ keystroke = get_keystroke_ತುಂಬಿದ(msec);

    ಆದರೆ (keystroke < 0)
        ಹಿಂತಿರುಗಿ keystroke;
    ಹಿಂತಿರುಗಿ keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_BOOTMENU_WAIT 2500

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ menuchars[] = {
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
    ಆದರೆ (! CONFIG_BOOTMENU)
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    ಆದರೆ (!show_boot_menu)
        ಹಿಂತಿರುಗಿ;

    // skip menu if only one boot device and no TPM
    ಆದರೆ (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_ಖಾಲಿಗಳು(&BootList) && !BootList.ಮೊದಲ->ಮುಂದಿನ) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        ಬರೆ("\n");
        ಹಿಂತಿರುಗಿ;
    }

    ಪೂರ್ಣಾಂಕ menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    ಪೂರ್ಣಾಂಕ menukey = romfile_loadint("etc/boot-menu-key", 1);
    ಪೂರ್ಣಾಂಕ ಪರಿಶೀಲನೆ_ಸಂಕೇತ;
    ಆದರೆ (menutime >= 0) {
        ಆಗ (get_keystroke(0) >= 0)
            ;

        ಅಕ್ಷರ *bootmsg = romfile_loadfile("etc/boot-menu-message", ಖಾಲಿ);
        ಬರೆ("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        ಬಿಡುಗಡೆ(bootmsg);

        enable_bootsplash();
        ಪರಿಶೀಲನೆ_ಸಂಕೇತ = get_keystroke(menutime);
        disable_bootsplash();
        ಆದರೆ (ಪರಿಶೀಲನೆ_ಸಂಕೇತ != menukey)
            ಹಿಂತಿರುಗಿ;
    }

    ಆಗ (get_keystroke(0) >= 0)
        ;

    ಬರೆ("Select boot device:\n\n");
    wait_ಎಳೆಗಳು();

    // Show menu items
    ಪೂರ್ಣಾಂಕ maxmenu = 0;
    ರಚನೆ bootentry_s *pos, *boot = ಖಾಲಿ;
    hlist_for_each_ನಮೂದು(pos, &BootList, ಗಂಟು) {
        ಅಕ್ಷರ desc[77];
        ಆದರೆ (maxmenu >= ARRAY_ಗಾತ್ರ(menuchars)) {
            ನಿಲ್ಲಿಸು;
        }
        ಬರೆ("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_ಗಾತ್ರ(desc)));
        maxmenu++;
    }
    ಆದರೆ (tpm_can_show_menu()) {
        ಬರೆ("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    ಪೂರ್ಣಾಂಕ esc_accepted_ಸಮಯ = irqtimer_calc(menukey == 1 ? 1500 : 0);
    ಗಾಗಿ (;;) {
        ಪೂರ್ಣಾಂಕ keystroke = get_keystroke_ತುಂಬಿದ(1000);
        ಆದರೆ (keystroke == 0x011b && !irqtimer_check(esc_accepted_ಸಮಯ))
            ಮುಂದುವರಿ;
        ಆದರೆ (keystroke < 0) // timeout
            ಮುಂದುವರಿ;

        ಪರಿಶೀಲನೆ_ಸಂಕೇತ = keystroke >> 8;
        ಪೂರ್ಣಾಂಕ ಕೀಲಿ_ascii = keystroke & 0xff;
        ಆದರೆ (tpm_can_show_menu() && ಕೀಲಿ_ascii == 't') {
            ಬರೆ("\n");
            tpm_menu();
        }
        ಆದರೆ (ಪರಿಶೀಲನೆ_ಸಂಕೇತ == 1) {
            // ESC
            ಬರೆ("\n");
            ಹಿಂತಿರುಗಿ;
        }

        maxmenu = 0;
        hlist_for_each_ನಮೂದು(pos, &BootList, ಗಂಟು) {
            ಆದರೆ (maxmenu >= ARRAY_ಗಾತ್ರ(menuchars))
                ನಿಲ್ಲಿಸು;
            ಆದರೆ (ಕೀಲಿ_ascii == menuchars[maxmenu]) {
                boot = pos;
                ನಿಲ್ಲಿಸು;
            }
            maxmenu++;
        }
        ಆದರೆ (boot)
            ನಿಲ್ಲಿಸು;
    }
    ಬರೆ("\n");

    // Find entry and make top priority.
    hlist_del(&boot->ಗಂಟು);
    boot->ಆದ್ಯತೆ = 0;
    hlist_ಕೂಡಿಸು_ತಲೆ(&boot->ಗಂಟು, &BootList);
}

// BEV (Boot Execution Vector) list
ರಚನೆ bev_s {
    ಪೂರ್ಣಾಂಕ type;
    u32 ಸದಿಶ;
};
ಸ್ಥಿರ ರಚನೆ bev_s BEV[20];
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ BEVCount;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ HaveHDBoot, HaveFDBoot;

ಸ್ಥಿರ ಶೂನ್ಯ
ಕೂಡಿಸು_bev(ಪೂರ್ಣಾಂಕ type, u32 ಸದಿಶ)
{
    ಆದರೆ (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (BEVCount >= ARRAY_ಗಾತ್ರ(BEV))
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->ಸದಿಶ = ಸದಿಶ;
}

// Prepare for boot - show menu and run bcvs.
ಶೂನ್ಯ
bcv_prepboot(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_BOOT)
        ಹಿಂತಿರುಗಿ;

    ಪೂರ್ಣಾಂಕ haltprio = ಕಂಡುಹಿಡಿ_prio("HALT");
    ಆದರೆ (haltprio >= 0)
        bootentry_ಕೂಡಿಸು(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    ರಚನೆ bootentry_s *pos;
    hlist_for_each_ನಮೂದು(pos, &BootList, ಗಂಟು) {
        ಆಯ್ಕೆ (pos->type) {
        ಸ್ಥಿತಿ IPL_TYPE_BCV:
            call_bcv(pos->ಸದಿಶ.seg, pos->ಸದಿಶ.ಸ್ಥಾನಾಂತರ_2);
            ಕೂಡಿಸು_bev(IPL_TYPE_HARDDISK, 0);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            ಕೂಡಿಸು_bev(IPL_TYPE_FLOPPY, 0);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            ಕೂಡಿಸು_bev(IPL_TYPE_HARDDISK, 0);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        ಪೂರ್ವನಿಯೋಜಿತ:
            ಕೂಡಿಸು_bev(pos->type, pos->ದತ್ತಾಂಶ);
            ನಿಲ್ಲಿಸು;
        }
    }

    // If nothing added a floppy/hd boot - add it manually.
    ಕೂಡಿಸು_bev(IPL_TYPE_FLOPPY, 0);
    ಕೂಡಿಸು_bev(IPL_TYPE_HARDDISK, 0);
}


/****************************************************************
 * Boot code (int 18/19)
 ****************************************************************/

// Jump to a bootup entry point.
ಸ್ಥಿರ ಶೂನ್ಯ
call_boot_ನಮೂದು(ರಚನೆ segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.ಸ್ಥಾನಾಂತರ_2);
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    br.ಸಂಕೇತ_2 = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
ಸ್ಥಿರ ಶೂನ್ಯ
boot_disk(u8 bootdrv, ಪೂರ್ಣಾಂಕ checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    ಆದರೆ (br.flags & F_CF) {
        ಬರೆ("Boot failed: could not read the boot disk\n\n");
        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (checksig) {
        ರಚನೆ mbr_s *mbr = (ಶೂನ್ಯ*)0;
        ಆದರೆ (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            ಬರೆ("Boot failed: not a bootable disk\n\n");
            ಹಿಂತಿರುಗಿ;
        }
    }

    tpm_ಕೂಡಿಸು_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_ನಮೂದು(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
ಸ್ಥಿರ ಶೂನ್ಯ
boot_cdrom(ರಚನೆ drive_s *drive)
{
    ಆದರೆ (! CONFIG_CDROM_BOOT)
        ಹಿಂತಿರುಗಿ;
    ಬರೆ("Booting from DVD/CD...\n");

    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = cdrom_boot(drive);
    ಆದರೆ (ಸ್ಥಿತಿ_2) {
        ಬರೆ("Boot failed: Could not read from CDROM (code %04x)\n", ಸ್ಥಿತಿ_2);
        ಹಿಂತಿರುಗಿ;
    }

    u8 bootdrv = CDEmu.emulated_drive;
    u16 bootseg = CDEmu.ತುಂಬು_segment;

    tpm_ಕೂಡಿಸು_cdrom(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_ನಮೂದು(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CBFS payload
ಸ್ಥಿರ ಶೂನ್ಯ
boot_cbfs(ರಚನೆ cbfs_file *file)
{
    ಆದರೆ (!CONFIG_COREBOOT_FLASH)
        ಹಿಂತಿರುಗಿ;
    ಬರೆ("Booting from CBFS...\n");
    cbfs_ಸತತ_payload(file);
}

// Boot from a BEV entry on an optionrom.
ಸ್ಥಿರ ಶೂನ್ಯ
boot_rom(u32 ಸದಿಶ)
{
    ಬರೆ("Booting from ROM...\n");
    ರಚನೆ segoff_s so;
    so.segoff = ಸದಿಶ;
    call_boot_ನಮೂದು(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
ಸ್ಥಿರ ಶೂನ್ಯ
boot_fail(ಶೂನ್ಯ)
{
    ಆದರೆ (Boot_Retry_ಸಮಯ == (u32)-1)
        ಬರೆ("No bootable device.\n");
    ಇಲ್ಲದಿದ್ದರೆ
        ಬರೆ("No bootable device.  Retrying in %d seconds.\n"
               , Boot_Retry_ಸಮಯ/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 ಕೊನೆ = irqtimer_calc(Boot_Retry_ಸಮಯ);
    ಗಾಗಿ (;;) {
        ಆದರೆ (Boot_Retry_ಸಮಯ != (u32)-1 && irqtimer_check(ಕೊನೆ))
            ನಿಲ್ಲಿಸು;
        yield_toirq();
    }
    ಬರೆ("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
ಸ್ಥಿರ ಶೂನ್ಯ
do_boot(ಪೂರ್ಣಾಂಕ seq_nr)
{
    ಆದರೆ (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    ಆದರೆ (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    ರಚನೆ bev_s *ie = &BEV[seq_nr];
    ಆಯ್ಕೆ (ie->type) {
    ಸ್ಥಿತಿ IPL_TYPE_FLOPPY:
        ಬರೆ("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IPL_TYPE_HARDDISK:
        ಬರೆ("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IPL_TYPE_CDROM:
        boot_cdrom((ಶೂನ್ಯ*)ie->ಸದಿಶ);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IPL_TYPE_CBFS:
        boot_cbfs((ಶೂನ್ಯ*)ie->ಸದಿಶ);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IPL_TYPE_BEV:
        boot_rom(ie->ಸದಿಶ);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ IPL_TYPE_HALT:
        boot_fail();
        ನಿಲ್ಲಿಸು;
    }

    // Boot failed: invoke the boot recovery function
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

ಪೂರ್ಣಾಂಕ BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
ಶೂನ್ಯ VISIBLE32FLAT
handle_18(ಶೂನ್ಯ)
{
    debug_enter(ಖಾಲಿ, DEBUG_HDL_18);
    ಪೂರ್ಣಾಂಕ seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
ಶೂನ್ಯ VISIBLE32FLAT
handle_19(ಶೂನ್ಯ)
{
    debug_enter(ಖಾಲಿ, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
