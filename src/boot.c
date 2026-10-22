// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "fw/paravirt.h" // qemu_cfg_show_boot_menu
#అన్తర్భావ "hw/pci.h" // pci_bdf_to_*
#అన్తర్భావ "hw/pcidevice.h" // struct pci_device
#అన్తర్భావ "hw/rtc.h" // rtc_read
#అన్తర్భావ "hw/usb.h" // struct usbdevice_s
#అన్తర్భావ "list.h" // hlist_node
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "std/disk.h" // struct mbr_s
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // irqtimer_calc
#అన్తర్భావ "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
స్థిర అక్షరం *
glob_ఉపసర్గ(స్థిరరాశి అక్షరం *glob, స్థిరరాశి అక్షరం *str)
{
    కొరకు (;;) {
        ఒకవేళ (!*glob && (!*str || *str == '/'))
            తిరిగి (అక్షరం*)str;
        ఒకవేళ (*glob == '*') {
            ఒకవేళ (!*str || *str == '/' || *str == glob[1])
                glob++;
            లేకపోతే
                str++;
            కొనసాగు;
        }
        ఒకవేళ (*glob != *str)
            తిరిగి ఖాళీ;
        glob++;
        str++;
    }
}

#నిర్వచించు FW_PCI_DOMAIN "/pci@i0cf8"

స్థిర అక్షరం *
build_pci_మార్గం(అక్షరం *buf, పూర్ణాంకం max, స్థిరరాశి అక్షరం *devname, నిర్మాణం pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    అక్షరం *p = buf;
    ఒకవేళ (pci->మాతృ) {
        p = build_pci_మార్గం(p, max, "pci-bridge", pci->మాతృ);
    } లేకపోతే {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        ఒకవేళ (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    పూర్ణాంకం dev = pci_bdf_గమ్యం_dev(pci->bdf), fn = pci_bdf_గమ్యం_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    ఒకవేళ (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    తిరిగి p;
}

స్థిర అక్షరం *
build_scsi_మార్గం(అక్షరం *buf, పూర్ణాంకం max,
                నిర్మాణం pci_device *pci, పూర్ణాంకం లక్ష్యం, పూర్ణాంకం lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    అక్షరం *p;
    p = build_pci_మార్గం(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", లక్ష్యం, lun);
    తిరిగి p;
}

స్థిర అక్షరం *
build_ata_మార్గం(అక్షరం *buf, పూర్ణాంకం max,
               నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    అక్షరం *p;
    p = build_pci_మార్గం(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    తిరిగి p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

రకనిర్వచనం నిర్మాణం BootDeviceLCHS {
    అక్షరం *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

స్థిర BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
స్థిర పూర్ణాంకం Bios_Geometry_లెక్క;

స్థిర అక్షరం *
parse_u32(అక్షరం *cur, u32 *n)
{
    u32 m = 0;
    ఒకవేళ (cur) {
        అయితే ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        ఒకవేళ (*cur != '\0')
            cur++;
    }
    *n = m;
    తిరిగి cur;
}

స్థిర శూన్య
ఎక్కించు_Bios_Geometry(శూన్య)
{
    ఒకవేళ (!CONFIG_HOST_BIOS_GEOMETRY)
        తిరిగి;
    అక్షరం *f = romfile_loadfile("bios-geometry", ఖాళీ);
    ఒకవేళ (!f)
        తిరిగి;

    పూర్ణాంకం i = 0;
    Bios_Geometry_లెక్క = 1;
    అయితే (f[i]) {
        ఒకవేళ (f[i] == '\n')
            Bios_Geometry_లెక్క++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_లెక్క * పరిమాణం(BootDeviceLCHS));
    ఒకవేళ (!BiosGeometry) {
        warn_noalloc();
        విడుదల(f);
        Bios_Geometry_లెక్క = 0;
        తిరిగి;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    చేయి {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        ఒకవేళ (f)
            *(f++) = '\0';
        అక్షరం *chs_విలువలు = strchr(d->name, ' ');
        ఒకవేళ (chs_విలువలు)
            *(chs_విలువలు++) = '\0';
        chs_విలువలు = parse_u32(chs_విలువలు, &d->lcyls);
        chs_విలువలు = parse_u32(chs_విలువలు, &d->lheads);
        chs_విలువలు = parse_u32(chs_విలువలు, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } అయితే (f);
}

// Search the bios-geometry list for the given glob pattern.
స్థిర BootDeviceLCHS *
boot_lchs_కనుగొను(స్థిరరాశి అక్షరం *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    పూర్ణాంకం i;
    కొరకు (i = 0; i < Bios_Geometry_లెక్క; i++)
        ఒకవేళ (glob_ఉపసర్గ(glob, BiosGeometry[i].name))
            తిరిగి &BiosGeometry[i];
    తిరిగి ఖాళీ;
}

పూర్ణాంకం boot_lchs_కనుగొను_pci_device(నిర్మాణం pci_device *pci, నిర్మాణం chs_s *chs)
{
    ఒకవేళ (!CONFIG_HOST_BIOS_GEOMETRY)
        తిరిగి -1;
    అక్షరం desc[256];
    build_pci_మార్గం(desc, పరిమాణం(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_కనుగొను(desc);
    ఒకవేళ (!b)
        తిరిగి -1;
    chs->cylinder = (u16)b->lcyls;
    chs->తల = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    తిరిగి 0;
}

పూర్ణాంకం boot_lchs_కనుగొను_scsi_device(నిర్మాణం pci_device *pci, పూర్ణాంకం లక్ష్యం, పూర్ణాంకం lun,
                               నిర్మాణం chs_s *chs)
{
    ఒకవేళ (!CONFIG_HOST_BIOS_GEOMETRY)
        తిరిగి -1;
    ఒకవేళ (!pci)
        // support only pci machine for now
        తిరిగి -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    అక్షరం desc[256];
    build_scsi_మార్గం(desc, పరిమాణం(desc), pci, లక్ష్యం, lun);
    BootDeviceLCHS *b = boot_lchs_కనుగొను(desc);
    ఒకవేళ (!b)
        తిరిగి -1;
    chs->cylinder = (u16)b->lcyls;
    chs->తల = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    తిరిగి 0;
}

పూర్ణాంకం boot_lchs_కనుగొను_ata_device(నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం slave,
                              నిర్మాణం chs_s *chs)
{
    ఒకవేళ (!CONFIG_HOST_BIOS_GEOMETRY)
        తిరిగి -1;
    ఒకవేళ (!pci)
        // support only pci machine for now
        తిరిగి -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    అక్షరం desc[256];
    build_ata_మార్గం(desc, పరిమాణం(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_కనుగొను(desc);
    ఒకవేళ (!b)
        తిరిగి -1;
    chs->cylinder = (u16)b->lcyls;
    chs->తల = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    తిరిగి 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

స్థిర అక్షరం **Bootorder VARVERIFY32INIT;
స్థిర పూర్ణాంకం Bootorder_లెక్క;

స్థిర శూన్య
ఎక్కించు_Boot_క్రమం(శూన్య)
{
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి;

    అక్షరం *f = romfile_loadfile("bootorder", ఖాళీ);
    ఒకవేళ (!f)
        తిరిగి;

    పూర్ణాంకం i = 0;
    Bootorder_లెక్క = 1;
    అయితే (f[i]) {
        ఒకవేళ (f[i] == '\n')
            Bootorder_లెక్క++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_లెక్క*పరిమాణం(అక్షరం*));
    ఒకవేళ (!Bootorder) {
        warn_noalloc();
        విడుదల(f);
        Bootorder_లెక్క = 0;
        తిరిగి;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    చేయి {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        ఒకవేళ (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } అయితే (f);
}

// Search the bootorder list for the given glob pattern.
స్థిర పూర్ణాంకం
కనుగొను_prio(స్థిరరాశి అక్షరం *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    పూర్ణాంకం i;
    కొరకు (i = 0; i < Bootorder_లెక్క; i++)
        ఒకవేళ (glob_ఉపసర్గ(glob, Bootorder[i]))
            తిరిగి i+1;
    తిరిగి -1;
}

u8 is_bootprio_strict(శూన్య)
{
    స్థిర పూర్ణాంకం prio_halt = -2;

    ఒకవేళ (prio_halt == -2)
        prio_halt = కనుగొను_prio("HALT");
    తిరిగి prio_halt >= 0;
}

పూర్ణాంకం bootprio_కనుగొను_pci_device(నిర్మాణం pci_device *pci)
{
    ఒకవేళ (CONFIG_CSM)
        తిరిగి csm_bootprio_pci(pci);
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    అక్షరం desc[256];
    build_pci_మార్గం(desc, పరిమాణం(desc), "*", pci);
    తిరిగి కనుగొను_prio(desc);
}

పూర్ణాంకం bootprio_కనుగొను_mmio_device(శూన్య *mmio)
{
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    అక్షరం desc[256];
    snprintf(desc, పరిమాణం(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    తిరిగి కనుగొను_prio(desc);
}

పూర్ణాంకం bootprio_కనుగొను_scsi_device(నిర్మాణం pci_device *pci, పూర్ణాంకం లక్ష్యం, పూర్ణాంకం lun)
{
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    ఒకవేళ (!pci)
        // support only pci machine for now
        తిరిగి -1;
    అక్షరం desc[256];
    build_scsi_మార్గం(desc, పరిమాణం(desc), pci, లక్ష్యం, lun);
    తిరిగి కనుగొను_prio(desc);
}

పూర్ణాంకం bootprio_కనుగొను_scsi_mmio_device(శూన్య *mmio, పూర్ణాంకం లక్ష్యం, పూర్ణాంకం lun)
{
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    అక్షరం desc[256];
    snprintf(desc, పరిమాణం(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, లక్ష్యం, lun);
    తిరిగి కనుగొను_prio(desc);
}

పూర్ణాంకం bootprio_కనుగొను_ata_device(నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం slave)
{
    ఒకవేళ (CONFIG_CSM)
        తిరిగి csm_bootprio_ata(pci, chanid, slave);
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    ఒకవేళ (!pci)
        // support only pci machine for now
        తిరిగి -1;
    అక్షరం desc[256];
    build_ata_మార్గం(desc, పరిమాణం(desc), pci, chanid, slave);
    తిరిగి కనుగొను_prio(desc);
}

పూర్ణాంకం bootprio_కనుగొను_fdc_device(నిర్మాణం pci_device *pci, పూర్ణాంకం పోర్టు, పూర్ణాంకం fdid)
{
    ఒకవేళ (CONFIG_CSM)
        తిరిగి csm_bootprio_fdc(pci, పోర్టు, fdid);
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    ఒకవేళ (!pci)
        // support only pci machine for now
        తిరిగి -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    అక్షరం desc[256], *p;
    p = build_pci_మార్గం(desc, పరిమాణం(desc), "isa", pci);
    snprintf(p, desc+పరిమాణం(desc)-p, "/fdc@%04x/floppy@%x", పోర్టు, fdid);
    తిరిగి కనుగొను_prio(desc);
}

పూర్ణాంకం bootprio_కనుగొను_pci_rom(నిర్మాణం pci_device *pci, పూర్ణాంకం instance)
{
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    అక్షరం desc[256], *p;
    p = build_pci_మార్గం(desc, పరిమాణం(desc), "*", pci);
    ఒకవేళ (instance)
        snprintf(p, desc+పరిమాణం(desc)-p, ":rom%x", instance);
    తిరిగి కనుగొను_prio(desc);
}

పూర్ణాంకం bootprio_కనుగొను_named_rom(స్థిరరాశి అక్షరం *name, పూర్ణాంకం instance)
{
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    అక్షరం desc[256], *p;
    p = desc + snprintf(desc, పరిమాణం(desc), "/rom@%s", name);
    ఒకవేళ (instance)
        snprintf(p, desc+పరిమాణం(desc)-p, ":rom%x", instance);
    తిరిగి కనుగొను_prio(desc);
}

స్థిర పూర్ణాంకం usb_portmap(నిర్మాణం usbdevice_s *usbdev)
{
    ఒకవేళ (usbdev->hub->op->portmap)
        తిరిగి usbdev->hub->op->portmap(usbdev->hub, usbdev->పోర్టు);
    తిరిగి usbdev->పోర్టు + 1;
}

స్థిర అక్షరం *
build_usb_మార్గం(అక్షరం *buf, పూర్ణాంకం max, నిర్మాణం usbhub_s *hub)
{
    ఒకవేళ (!hub->usbdev)
        // Root hub - nothing to add.
        తిరిగి buf;
    అక్షరం *p = build_usb_మార్గం(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    తిరిగి p;
}

పూర్ణాంకం bootprio_కనుగొను_usb(నిర్మాణం usbdevice_s *usbdev, పూర్ణాంకం lun)
{
    ఒకవేళ (!CONFIG_BOOTORDER)
        తిరిగి -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    అక్షరం desc[256], *p;

    ఒకవేళ (usbdev->hub->cntl->pci)
        p = build_pci_మార్గం(desc, పరిమాణం(desc), "usb", usbdev->hub->cntl->pci);
    లేకపోతే ఒకవేళ (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, పరిమాణం(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    లేకపోతే
        తిరిగి -1;

    p = build_usb_మార్గం(p, desc+పరిమాణం(desc)-p, usbdev->hub);
    snprintf(p, desc+పరిమాణం(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    పూర్ణాంకం ret = కనుగొను_prio(desc);
    ఒకవేళ (ret >= 0)
        తిరిగి ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+పరిమాణం(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    తిరిగి కనుగొను_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

స్థిర పూర్ణాంకం Boot_Retry_సమయం;
స్థిర పూర్ణాంకం CheckFloppySig = 1;

#నిర్వచించు DEFAULT_PRIO           9999

స్థిర పూర్ణాంకం DefaultFloppyPrio = 101;
స్థిర పూర్ణాంకం DefaultCDPrio     = 102;
స్థిర పూర్ణాంకం DefaultHDPrio     = 103;
స్థిర పూర్ణాంకం DefaultBEVPrio    = 104;

శూన్య
boot_init(శూన్య)
{
    ఒకవేళ (! CONFIG_BOOT)
        తిరిగి;

    ఒకవేళ (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        ఒకవేళ (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        పూర్ణాంకం i;
        కొరకు (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            ఎంపిక (val) {
            స్థితి 1: DefaultFloppyPrio = i; ఆపు;
            స్థితి 2: DefaultHDPrio = i;     ఆపు;
            స్థితి 3: DefaultCDPrio = i;     ఆపు;
            స్థితి 4: DefaultBEVPrio = i;    ఆపు;
            }
        }
    }

    Boot_Retry_సమయం = romfile_loadint("etc/boot-fail-wait", 60*1000);

    ఎక్కించు_Boot_క్రమం();
    ఎక్కించు_Bios_Geometry();
}


/****************************************************************
 * BootList handling
 ****************************************************************/

నిర్మాణం bootentry_s {
    పూర్ణాంకం type;
    సంఘం {
        u32 దత్తాంశం;
        నిర్మాణం segoff_s సదిశ;
        నిర్మాణం drive_s *drive;
    };
    పూర్ణాంకం ప్రాధాన్యత;
    స్థిరరాశి అక్షరం *description;
    నిర్మాణం hlist_కణుపు కణుపు;
};
స్థిర నిర్మాణం hlist_తల BootList VARVERIFY32INIT;

#నిర్వచించు IPL_TYPE_FLOPPY      0x01
#నిర్వచించు IPL_TYPE_HARDDISK    0x02
#నిర్వచించు IPL_TYPE_CDROM       0x03
#నిర్వచించు IPL_TYPE_CBFS        0x20
#నిర్వచించు IPL_TYPE_BEV         0x80
#నిర్వచించు IPL_TYPE_BCV         0x81
#నిర్వచించు IPL_TYPE_HALT        0xf0

స్థిర శూన్య
bootentry_కూడు(పూర్ణాంకం type, పూర్ణాంకం prio, u32 దత్తాంశం, స్థిరరాశి అక్షరం *desc)
{
    ఒకవేళ (! CONFIG_BOOT)
        తిరిగి;
    నిర్మాణం bootentry_s *be = malloc_tmp(పరిమాణం(*be));
    ఒకవేళ (!be) {
        warn_noalloc();
        తిరిగి;
    }
    be->type = type;
    be->ప్రాధాన్యత = prio;
    be->దత్తాంశం = దత్తాంశం;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, దత్తాంశం);

    // Add entry in sorted order.
    నిర్మాణం hlist_కణుపు **pprev;
    నిర్మాణం bootentry_s *pos;
    hlist_for_each_నమోదు_pprev(pos, pprev, &BootList, కణుపు) {
        ఒకవేళ (be->ప్రాధాన్యత < pos->ప్రాధాన్యత)
            ఆపు;
        ఒకవేళ (be->ప్రాధాన్యత > pos->ప్రాధాన్యత)
            కొనసాగు;
        ఒకవేళ (be->type < pos->type)
            ఆపు;
        ఒకవేళ (be->type > pos->type)
            కొనసాగు;
        ఒకవేళ (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            ఆపు;
    }
    hlist_కూడు(&be->కణుపు, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
స్థిర అంతఃవరుస పూర్ణాంకం defPrio(పూర్ణాంకం ప్రాధాన్యత, పూర్ణాంకం defaultprio) {
    తిరిగి (ప్రాధాన్యత < 0) ? defaultprio : ప్రాధాన్యత;
}

// Add a BEV vector for a given pnp compatible option rom.
శూన్య
boot_కూడు_bev(u16 seg, u16 bev, u16 desc, పూర్ణాంకం prio)
{
    bootentry_కూడు(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
శూన్య
boot_కూడు_bcv(u16 seg, u16 ip, u16 desc, పూర్ణాంకం prio)
{
    bootentry_కూడు(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

శూన్య
boot_కూడు_floppy(నిర్మాణం drive_s *drive, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio)
{
    bootentry_కూడు(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

శూన్య
boot_కూడు_hd(నిర్మాణం drive_s *drive, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio)
{
    bootentry_కూడు(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

శూన్య
boot_కూడు_cd(నిర్మాణం drive_s *drive, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio)
{
    ఒకవేళ (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        అక్షరం *extra = cdrom_media_info(drive);
        ఒకవేళ (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            విడుదల(extra);
        }
    }
    bootentry_కూడు(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
శూన్య
boot_కూడు_cbfs(శూన్య *దత్తాంశం, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio)
{
    bootentry_కూడు(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)దత్తాంశం, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
స్థిర పూర్ణాంకం
check_for_keystroke(శూన్య)
{
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    తిరిగి !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
స్థిర పూర్ణాంకం
get_raw_keystroke(శూన్య)
{
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    తిరిగి br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
పూర్ణాంకం
get_keystroke_నిండిన(పూర్ణాంకం msec)
{
    u32 చివర = irqtimer_calc(msec);
    కొరకు (;;) {
        ఒకవేళ (check_for_keystroke())
            తిరిగి get_raw_keystroke();
        ఒకవేళ (irqtimer_check(చివర))
            తిరిగి -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
పూర్ణాంకం
get_keystroke(పూర్ణాంకం msec)
{
    పూర్ణాంకం keystroke = get_keystroke_నిండిన(msec);

    ఒకవేళ (keystroke < 0)
        తిరిగి keystroke;
    తిరిగి keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#నిర్వచించు DEFAULT_BOOTMENU_WAIT 2500

స్థిర స్థిరరాశి అక్షరం menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
శూన్య
interactive_bootmenu(శూన్య)
{
    ఒకవేళ (! CONFIG_BOOTMENU)
        తిరిగి;
    పూర్ణాంకం show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    ఒకవేళ (!show_boot_menu)
        తిరిగి;

    // skip menu if only one boot device and no TPM
    ఒకవేళ (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_ఖాళీలు(&BootList) && !BootList.మొదటి->తదుపరి) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        రాయి("\n");
        తిరిగి;
    }

    పూర్ణాంకం menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    పూర్ణాంకం menukey = romfile_loadint("etc/boot-menu-key", 1);
    పూర్ణాంకం పరిశీలన_సంకేతం;
    ఒకవేళ (menutime >= 0) {
        అయితే (get_keystroke(0) >= 0)
            ;

        అక్షరం *bootmsg = romfile_loadfile("etc/boot-menu-message", ఖాళీ);
        రాయి("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        విడుదల(bootmsg);

        enable_bootsplash();
        పరిశీలన_సంకేతం = get_keystroke(menutime);
        disable_bootsplash();
        ఒకవేళ (పరిశీలన_సంకేతం != menukey)
            తిరిగి;
    }

    అయితే (get_keystroke(0) >= 0)
        ;

    రాయి("Select boot device:\n\n");
    wait_దారాలు();

    // Show menu items
    పూర్ణాంకం maxmenu = 0;
    నిర్మాణం bootentry_s *pos, *boot = ఖాళీ;
    hlist_for_each_నమోదు(pos, &BootList, కణుపు) {
        అక్షరం desc[77];
        ఒకవేళ (maxmenu >= ARRAY_కొలత(menuchars)) {
            ఆపు;
        }
        రాయి("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_కొలత(desc)));
        maxmenu++;
    }
    ఒకవేళ (tpm_can_show_menu()) {
        రాయి("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    పూర్ణాంకం esc_accepted_సమయం = irqtimer_calc(menukey == 1 ? 1500 : 0);
    కొరకు (;;) {
        పూర్ణాంకం keystroke = get_keystroke_నిండిన(1000);
        ఒకవేళ (keystroke == 0x011b && !irqtimer_check(esc_accepted_సమయం))
            కొనసాగు;
        ఒకవేళ (keystroke < 0) // timeout
            కొనసాగు;

        పరిశీలన_సంకేతం = keystroke >> 8;
        పూర్ణాంకం కీలకం_ascii = keystroke & 0xff;
        ఒకవేళ (tpm_can_show_menu() && కీలకం_ascii == 't') {
            రాయి("\n");
            tpm_menu();
        }
        ఒకవేళ (పరిశీలన_సంకేతం == 1) {
            // ESC
            రాయి("\n");
            తిరిగి;
        }

        maxmenu = 0;
        hlist_for_each_నమోదు(pos, &BootList, కణుపు) {
            ఒకవేళ (maxmenu >= ARRAY_కొలత(menuchars))
                ఆపు;
            ఒకవేళ (కీలకం_ascii == menuchars[maxmenu]) {
                boot = pos;
                ఆపు;
            }
            maxmenu++;
        }
        ఒకవేళ (boot)
            ఆపు;
    }
    రాయి("\n");

    // Find entry and make top priority.
    hlist_del(&boot->కణుపు);
    boot->ప్రాధాన్యత = 0;
    hlist_కూడు_తల(&boot->కణుపు, &BootList);
}

// BEV (Boot Execution Vector) list
నిర్మాణం bev_s {
    పూర్ణాంకం type;
    u32 సదిశ;
};
స్థిర నిర్మాణం bev_s BEV[20];
స్థిర పూర్ణాంకం BEVCount;
స్థిర పూర్ణాంకం HaveHDBoot, HaveFDBoot;

స్థిర శూన్య
కూడు_bev(పూర్ణాంకం type, u32 సదిశ)
{
    ఒకవేళ (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        తిరిగి;
    ఒకవేళ (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        తిరిగి;
    ఒకవేళ (BEVCount >= ARRAY_కొలత(BEV))
        తిరిగి;
    నిర్మాణం bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->సదిశ = సదిశ;
}

// Prepare for boot - show menu and run bcvs.
శూన్య
bcv_prepboot(శూన్య)
{
    ఒకవేళ (! CONFIG_BOOT)
        తిరిగి;

    పూర్ణాంకం haltprio = కనుగొను_prio("HALT");
    ఒకవేళ (haltprio >= 0)
        bootentry_కూడు(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    నిర్మాణం bootentry_s *pos;
    hlist_for_each_నమోదు(pos, &BootList, కణుపు) {
        ఎంపిక (pos->type) {
        స్థితి IPL_TYPE_BCV:
            call_bcv(pos->సదిశ.seg, pos->సదిశ.స్థానభ్రంశం_2);
            కూడు_bev(IPL_TYPE_HARDDISK, 0);
            ఆపు;
        స్థితి IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            కూడు_bev(IPL_TYPE_FLOPPY, 0);
            ఆపు;
        స్థితి IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            కూడు_bev(IPL_TYPE_HARDDISK, 0);
            ఆపు;
        స్థితి IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        అప్రమేయ:
            కూడు_bev(pos->type, pos->దత్తాంశం);
            ఆపు;
        }
    }

    // If nothing added a floppy/hd boot - add it manually.
    కూడు_bev(IPL_TYPE_FLOPPY, 0);
    కూడు_bev(IPL_TYPE_HARDDISK, 0);
}


/****************************************************************
 * Boot code (int 18/19)
 ****************************************************************/

// Jump to a bootup entry point.
స్థిర శూన్య
call_boot_నమోదు(నిర్మాణం segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.స్థానభ్రంశం_2);
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    br.సంకేతం_2 = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
స్థిర శూన్య
boot_disk(u8 bootdrv, పూర్ణాంకం checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    ఒకవేళ (br.flags & F_CF) {
        రాయి("Boot failed: could not read the boot disk\n\n");
        తిరిగి;
    }

    ఒకవేళ (checksig) {
        నిర్మాణం mbr_s *mbr = (శూన్య*)0;
        ఒకవేళ (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            రాయి("Boot failed: not a bootable disk\n\n");
            తిరిగి;
        }
    }

    tpm_కూడు_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_నమోదు(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
స్థిర శూన్య
boot_cdrom(నిర్మాణం drive_s *drive)
{
    ఒకవేళ (! CONFIG_CDROM_BOOT)
        తిరిగి;
    రాయి("Booting from DVD/CD...\n");

    పూర్ణాంకం స్థితి_2 = cdrom_boot(drive);
    ఒకవేళ (స్థితి_2) {
        రాయి("Boot failed: Could not read from CDROM (code %04x)\n", స్థితి_2);
        తిరిగి;
    }

    u8 bootdrv = CDEmu.emulated_drive;
    u16 bootseg = CDEmu.ఎక్కించు_segment;

    tpm_కూడు_cdrom(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_నమోదు(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CBFS payload
స్థిర శూన్య
boot_cbfs(నిర్మాణం cbfs_file *file)
{
    ఒకవేళ (!CONFIG_COREBOOT_FLASH)
        తిరిగి;
    రాయి("Booting from CBFS...\n");
    cbfs_నిరంతరం_payload(file);
}

// Boot from a BEV entry on an optionrom.
స్థిర శూన్య
boot_rom(u32 సదిశ)
{
    రాయి("Booting from ROM...\n");
    నిర్మాణం segoff_s so;
    so.segoff = సదిశ;
    call_boot_నమోదు(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
స్థిర శూన్య
boot_fail(శూన్య)
{
    ఒకవేళ (Boot_Retry_సమయం == (u32)-1)
        రాయి("No bootable device.\n");
    లేకపోతే
        రాయి("No bootable device.  Retrying in %d seconds.\n"
               , Boot_Retry_సమయం/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 చివర = irqtimer_calc(Boot_Retry_సమయం);
    కొరకు (;;) {
        ఒకవేళ (Boot_Retry_సమయం != (u32)-1 && irqtimer_check(చివర))
            ఆపు;
        yield_toirq();
    }
    రాయి("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
స్థిర శూన్య
do_boot(పూర్ణాంకం seq_nr)
{
    ఒకవేళ (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    ఒకవేళ (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    నిర్మాణం bev_s *ie = &BEV[seq_nr];
    ఎంపిక (ie->type) {
    స్థితి IPL_TYPE_FLOPPY:
        రాయి("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        ఆపు;
    స్థితి IPL_TYPE_HARDDISK:
        రాయి("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        ఆపు;
    స్థితి IPL_TYPE_CDROM:
        boot_cdrom((శూన్య*)ie->సదిశ);
        ఆపు;
    స్థితి IPL_TYPE_CBFS:
        boot_cbfs((శూన్య*)ie->సదిశ);
        ఆపు;
    స్థితి IPL_TYPE_BEV:
        boot_rom(ie->సదిశ);
        ఆపు;
    స్థితి IPL_TYPE_HALT:
        boot_fail();
        ఆపు;
    }

    // Boot failed: invoke the boot recovery function
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

పూర్ణాంకం BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
శూన్య VISIBLE32FLAT
handle_18(శూన్య)
{
    debug_enter(ఖాళీ, DEBUG_HDL_18);
    పూర్ణాంకం seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
శూన్య VISIBLE32FLAT
handle_19(శూన్య)
{
    debug_enter(ఖాళీ, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
