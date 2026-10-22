// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "fw/paravirt.h" // qemu_cfg_show_boot_menu
#अन्तर्भाव "hw/pci.h" // pci_bdf_to_*
#अन्तर्भाव "hw/pcidevice.h" // struct pci_device
#अन्तर्भाव "hw/rtc.h" // rtc_read
#अन्तर्भाव "hw/usb.h" // struct usbdevice_s
#अन्तर्भाव "list.h" // hlist_node
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "std/disk.h" // struct mbr_s
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // irqtimer_calc
#अन्तर्भाव "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
स्थिर वर्ण *
glob_उपसर्गः(ध्रुव वर्ण *glob, ध्रुव वर्ण *str)
{
    प्रति (;;) {
        यदि (!*glob && (!*str || *str == '/'))
            प्रत्यागम (वर्ण*)str;
        यदि (*glob == '*') {
            यदि (!*str || *str == '/' || *str == glob[1])
                glob++;
            अन्यथा
                str++;
            अनुवर्त;
        }
        यदि (*glob != *str)
            प्रत्यागम रिक्त;
        glob++;
        str++;
    }
}

#परिभाषय FW_PCI_DOMAIN "/pci@i0cf8"

स्थिर वर्ण *
build_pci_मार्गः(वर्ण *buf, पूर्णाङ्क max, ध्रुव वर्ण *devname, संरचना pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    वर्ण *p = buf;
    यदि (pci->parent) {
        p = build_pci_मार्गः(p, max, "pci-bridge", pci->parent);
    } अन्यथा {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        यदि (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    पूर्णाङ्क dev = pci_bdf_गन्तव्यम्_dev(pci->bdf), fn = pci_bdf_गन्तव्यम्_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    यदि (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    प्रत्यागम p;
}

स्थिर वर्ण *
build_scsi_मार्गः(वर्ण *buf, पूर्णाङ्क max,
                संरचना pci_device *pci, पूर्णाङ्क target, पूर्णाङ्क lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    वर्ण *p;
    p = build_pci_मार्गः(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    प्रत्यागम p;
}

स्थिर वर्ण *
build_ata_मार्गः(वर्ण *buf, पूर्णाङ्क max,
               संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    वर्ण *p;
    p = build_pci_मार्गः(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    प्रत्यागम p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

प्रकारनिर्देश संरचना BootDeviceLCHS {
    वर्ण *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

स्थिर BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
स्थिर पूर्णाङ्क Bios_Geometry_गणनम्;

स्थिर वर्ण *
parse_u32(वर्ण *cur, u32 *n)
{
    u32 m = 0;
    यदि (cur) {
        यावत् ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        यदि (*cur != '\0')
            cur++;
    }
    *n = m;
    प्रत्यागम cur;
}

स्थिर शून्य
आरोपय_Bios_Geometry(शून्य)
{
    यदि (!CONFIG_HOST_BIOS_GEOMETRY)
        प्रत्यागम;
    वर्ण *f = romfile_loadfile("bios-geometry", रिक्त);
    यदि (!f)
        प्रत्यागम;

    पूर्णाङ्क i = 0;
    Bios_Geometry_गणनम् = 1;
    यावत् (f[i]) {
        यदि (f[i] == '\n')
            Bios_Geometry_गणनम्++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_गणनम् * परिमाण(BootDeviceLCHS));
    यदि (!BiosGeometry) {
        warn_noalloc();
        मुक्त(f);
        Bios_Geometry_गणनम् = 0;
        प्रत्यागम;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    कुरु {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        यदि (f)
            *(f++) = '\0';
        वर्ण *chs_मानानि = strchr(d->name, ' ');
        यदि (chs_मानानि)
            *(chs_मानानि++) = '\0';
        chs_मानानि = parse_u32(chs_मानानि, &d->lcyls);
        chs_मानानि = parse_u32(chs_मानानि, &d->lheads);
        chs_मानानि = parse_u32(chs_मानानि, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } यावत् (f);
}

// Search the bios-geometry list for the given glob pattern.
स्थिर BootDeviceLCHS *
boot_lchs_find(ध्रुव वर्ण *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    पूर्णाङ्क i;
    प्रति (i = 0; i < Bios_Geometry_गणनम्; i++)
        यदि (glob_उपसर्गः(glob, BiosGeometry[i].name))
            प्रत्यागम &BiosGeometry[i];
    प्रत्यागम रिक्त;
}

पूर्णाङ्क boot_lchs_find_pci_device(संरचना pci_device *pci, संरचना chs_s *chs)
{
    यदि (!CONFIG_HOST_BIOS_GEOMETRY)
        प्रत्यागम -1;
    वर्ण desc[256];
    build_pci_मार्गः(desc, परिमाण(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    यदि (!b)
        प्रत्यागम -1;
    chs->cylinder = (u16)b->lcyls;
    chs->शीर्ष = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    प्रत्यागम 0;
}

पूर्णाङ्क boot_lchs_find_scsi_device(संरचना pci_device *pci, पूर्णाङ्क target, पूर्णाङ्क lun,
                               संरचना chs_s *chs)
{
    यदि (!CONFIG_HOST_BIOS_GEOMETRY)
        प्रत्यागम -1;
    यदि (!pci)
        // support only pci machine for now
        प्रत्यागम -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    वर्ण desc[256];
    build_scsi_मार्गः(desc, परिमाण(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    यदि (!b)
        प्रत्यागम -1;
    chs->cylinder = (u16)b->lcyls;
    chs->शीर्ष = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    प्रत्यागम 0;
}

पूर्णाङ्क boot_lchs_find_ata_device(संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क slave,
                              संरचना chs_s *chs)
{
    यदि (!CONFIG_HOST_BIOS_GEOMETRY)
        प्रत्यागम -1;
    यदि (!pci)
        // support only pci machine for now
        प्रत्यागम -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    वर्ण desc[256];
    build_ata_मार्गः(desc, परिमाण(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    यदि (!b)
        प्रत्यागम -1;
    chs->cylinder = (u16)b->lcyls;
    chs->शीर्ष = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    प्रत्यागम 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

स्थिर वर्ण **Bootorder VARVERIFY32INIT;
स्थिर पूर्णाङ्क Bootorder_गणनम्;

स्थिर शून्य
आरोपय_Boot_क्रमः(शून्य)
{
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम;

    वर्ण *f = romfile_loadfile("bootorder", रिक्त);
    यदि (!f)
        प्रत्यागम;

    पूर्णाङ्क i = 0;
    Bootorder_गणनम् = 1;
    यावत् (f[i]) {
        यदि (f[i] == '\n')
            Bootorder_गणनम्++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_गणनम्*परिमाण(वर्ण*));
    यदि (!Bootorder) {
        warn_noalloc();
        मुक्त(f);
        Bootorder_गणनम् = 0;
        प्रत्यागम;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    कुरु {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        यदि (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } यावत् (f);
}

// Search the bootorder list for the given glob pattern.
स्थिर पूर्णाङ्क
find_prio(ध्रुव वर्ण *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    पूर्णाङ्क i;
    प्रति (i = 0; i < Bootorder_गणनम्; i++)
        यदि (glob_उपसर्गः(glob, Bootorder[i]))
            प्रत्यागम i+1;
    प्रत्यागम -1;
}

u8 is_bootprio_strict(शून्य)
{
    स्थिर पूर्णाङ्क prio_halt = -2;

    यदि (prio_halt == -2)
        prio_halt = find_prio("HALT");
    प्रत्यागम prio_halt >= 0;
}

पूर्णाङ्क bootprio_find_pci_device(संरचना pci_device *pci)
{
    यदि (CONFIG_CSM)
        प्रत्यागम csm_bootprio_pci(pci);
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    वर्ण desc[256];
    build_pci_मार्गः(desc, परिमाण(desc), "*", pci);
    प्रत्यागम find_prio(desc);
}

पूर्णाङ्क bootprio_find_mmio_device(शून्य *mmio)
{
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    वर्ण desc[256];
    snprintf(desc, परिमाण(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    प्रत्यागम find_prio(desc);
}

पूर्णाङ्क bootprio_find_scsi_device(संरचना pci_device *pci, पूर्णाङ्क target, पूर्णाङ्क lun)
{
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    यदि (!pci)
        // support only pci machine for now
        प्रत्यागम -1;
    वर्ण desc[256];
    build_scsi_मार्गः(desc, परिमाण(desc), pci, target, lun);
    प्रत्यागम find_prio(desc);
}

पूर्णाङ्क bootprio_find_scsi_mmio_device(शून्य *mmio, पूर्णाङ्क target, पूर्णाङ्क lun)
{
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    वर्ण desc[256];
    snprintf(desc, परिमाण(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    प्रत्यागम find_prio(desc);
}

पूर्णाङ्क bootprio_find_ata_device(संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क slave)
{
    यदि (CONFIG_CSM)
        प्रत्यागम csm_bootprio_ata(pci, chanid, slave);
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    यदि (!pci)
        // support only pci machine for now
        प्रत्यागम -1;
    वर्ण desc[256];
    build_ata_मार्गः(desc, परिमाण(desc), pci, chanid, slave);
    प्रत्यागम find_prio(desc);
}

पूर्णाङ्क bootprio_find_fdc_device(संरचना pci_device *pci, पूर्णाङ्क पत्तनम्, पूर्णाङ्क fdid)
{
    यदि (CONFIG_CSM)
        प्रत्यागम csm_bootprio_fdc(pci, पत्तनम्, fdid);
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    यदि (!pci)
        // support only pci machine for now
        प्रत्यागम -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    वर्ण desc[256], *p;
    p = build_pci_मार्गः(desc, परिमाण(desc), "isa", pci);
    snprintf(p, desc+परिमाण(desc)-p, "/fdc@%04x/floppy@%x", पत्तनम्, fdid);
    प्रत्यागम find_prio(desc);
}

पूर्णाङ्क bootprio_find_pci_rom(संरचना pci_device *pci, पूर्णाङ्क instance)
{
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    वर्ण desc[256], *p;
    p = build_pci_मार्गः(desc, परिमाण(desc), "*", pci);
    यदि (instance)
        snprintf(p, desc+परिमाण(desc)-p, ":rom%x", instance);
    प्रत्यागम find_prio(desc);
}

पूर्णाङ्क bootprio_find_named_rom(ध्रुव वर्ण *name, पूर्णाङ्क instance)
{
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    वर्ण desc[256], *p;
    p = desc + snprintf(desc, परिमाण(desc), "/rom@%s", name);
    यदि (instance)
        snprintf(p, desc+परिमाण(desc)-p, ":rom%x", instance);
    प्रत्यागम find_prio(desc);
}

स्थिर पूर्णाङ्क usb_portmap(संरचना usbdevice_s *usbdev)
{
    यदि (usbdev->hub->op->portmap)
        प्रत्यागम usbdev->hub->op->portmap(usbdev->hub, usbdev->पत्तनम्);
    प्रत्यागम usbdev->पत्तनम् + 1;
}

स्थिर वर्ण *
build_usb_मार्गः(वर्ण *buf, पूर्णाङ्क max, संरचना usbhub_s *hub)
{
    यदि (!hub->usbdev)
        // Root hub - nothing to add.
        प्रत्यागम buf;
    वर्ण *p = build_usb_मार्गः(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    प्रत्यागम p;
}

पूर्णाङ्क bootprio_find_usb(संरचना usbdevice_s *usbdev, पूर्णाङ्क lun)
{
    यदि (!CONFIG_BOOTORDER)
        प्रत्यागम -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    वर्ण desc[256], *p;

    यदि (usbdev->hub->cntl->pci)
        p = build_pci_मार्गः(desc, परिमाण(desc), "usb", usbdev->hub->cntl->pci);
    अन्यथा यदि (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, परिमाण(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    अन्यथा
        प्रत्यागम -1;

    p = build_usb_मार्गः(p, desc+परिमाण(desc)-p, usbdev->hub);
    snprintf(p, desc+परिमाण(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    पूर्णाङ्क ret = find_prio(desc);
    यदि (ret >= 0)
        प्रत्यागम ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+परिमाण(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    प्रत्यागम find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

स्थिर पूर्णाङ्क BootRetryTime;
स्थिर पूर्णाङ्क CheckFloppySig = 1;

#परिभाषय DEFAULT_PRIO           9999

स्थिर पूर्णाङ्क DefaultFloppyPrio = 101;
स्थिर पूर्णाङ्क DefaultCDPrio     = 102;
स्थिर पूर्णाङ्क DefaultHDPrio     = 103;
स्थिर पूर्णाङ्क DefaultBEVPrio    = 104;

शून्य
boot_init(शून्य)
{
    यदि (! CONFIG_BOOT)
        प्रत्यागम;

    यदि (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        यदि (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        पूर्णाङ्क i;
        प्रति (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            विकल्प (val) {
            पक्ष 1: DefaultFloppyPrio = i; भङ्ग;
            पक्ष 2: DefaultHDPrio = i;     भङ्ग;
            पक्ष 3: DefaultCDPrio = i;     भङ्ग;
            पक्ष 4: DefaultBEVPrio = i;    भङ्ग;
            }
        }
    }

    BootRetryTime = romfile_loadint("etc/boot-fail-wait", 60*1000);

    आरोपय_Boot_क्रमः();
    आरोपय_Bios_Geometry();
}


/****************************************************************
 * BootList handling
 ****************************************************************/

संरचना bootentry_s {
    पूर्णाङ्क type;
    सङ्घ {
        u32 दत्त;
        संरचना segoff_s सदिशम्;
        संरचना drive_s *drive;
    };
    पूर्णाङ्क प्राथमिकता;
    ध्रुव वर्ण *description;
    संरचना hlist_ग्रन्थिः ग्रन्थिः;
};
स्थिर संरचना hlist_शीर्ष BootList VARVERIFY32INIT;

#परिभाषय IPL_TYPE_FLOPPY      0x01
#परिभाषय IPL_TYPE_HARDDISK    0x02
#परिभाषय IPL_TYPE_CDROM       0x03
#परिभाषय IPL_TYPE_CBFS        0x20
#परिभाषय IPL_TYPE_BEV         0x80
#परिभाषय IPL_TYPE_BCV         0x81
#परिभाषय IPL_TYPE_HALT        0xf0

स्थिर शून्य
bootentry_सङ्कलय(पूर्णाङ्क type, पूर्णाङ्क prio, u32 दत्त, ध्रुव वर्ण *desc)
{
    यदि (! CONFIG_BOOT)
        प्रत्यागम;
    संरचना bootentry_s *be = malloc_tmp(परिमाण(*be));
    यदि (!be) {
        warn_noalloc();
        प्रत्यागम;
    }
    be->type = type;
    be->प्राथमिकता = prio;
    be->दत्त = दत्त;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, दत्त);

    // Add entry in sorted order.
    संरचना hlist_ग्रन्थिः **pprev;
    संरचना bootentry_s *pos;
    hlist_for_each_प्रविष्टिः_pprev(pos, pprev, &BootList, ग्रन्थिः) {
        यदि (be->प्राथमिकता < pos->प्राथमिकता)
            भङ्ग;
        यदि (be->प्राथमिकता > pos->प्राथमिकता)
            अनुवर्त;
        यदि (be->type < pos->type)
            भङ्ग;
        यदि (be->type > pos->type)
            अनुवर्त;
        यदि (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            भङ्ग;
    }
    hlist_सङ्कलय(&be->ग्रन्थिः, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
स्थिर अन्तःस्थ पूर्णाङ्क defPrio(पूर्णाङ्क प्राथमिकता, पूर्णाङ्क defaultprio) {
    प्रत्यागम (प्राथमिकता < 0) ? defaultprio : प्राथमिकता;
}

// Add a BEV vector for a given pnp compatible option rom.
शून्य
boot_सङ्कलय_bev(u16 seg, u16 bev, u16 desc, पूर्णाङ्क prio)
{
    bootentry_सङ्कलय(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
शून्य
boot_सङ्कलय_bcv(u16 seg, u16 ip, u16 desc, पूर्णाङ्क prio)
{
    bootentry_सङ्कलय(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

शून्य
boot_सङ्कलय_floppy(संरचना drive_s *drive, ध्रुव वर्ण *desc, पूर्णाङ्क prio)
{
    bootentry_सङ्कलय(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

शून्य
boot_सङ्कलय_hd(संरचना drive_s *drive, ध्रुव वर्ण *desc, पूर्णाङ्क prio)
{
    bootentry_सङ्कलय(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

शून्य
boot_सङ्कलय_cd(संरचना drive_s *drive, ध्रुव वर्ण *desc, पूर्णाङ्क prio)
{
    यदि (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        वर्ण *extra = cdrom_media_info(drive);
        यदि (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            मुक्त(extra);
        }
    }
    bootentry_सङ्कलय(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
शून्य
boot_सङ्कलय_cbfs(शून्य *दत्त, ध्रुव वर्ण *desc, पूर्णाङ्क prio)
{
    bootentry_सङ्कलय(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)दत्त, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
स्थिर पूर्णाङ्क
check_for_keystroke(शून्य)
{
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    प्रत्यागम !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
स्थिर पूर्णाङ्क
get_raw_keystroke(शून्य)
{
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    प्रत्यागम br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
पूर्णाङ्क
get_keystroke_पूर्णम्(पूर्णाङ्क msec)
{
    u32 अन्तः = irqtimer_calc(msec);
    प्रति (;;) {
        यदि (check_for_keystroke())
            प्रत्यागम get_raw_keystroke();
        यदि (irqtimer_check(अन्तः))
            प्रत्यागम -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
पूर्णाङ्क
get_keystroke(पूर्णाङ्क msec)
{
    पूर्णाङ्क keystroke = get_keystroke_पूर्णम्(msec);

    यदि (keystroke < 0)
        प्रत्यागम keystroke;
    प्रत्यागम keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#परिभाषय DEFAULT_BOOTMENU_WAIT 2500

स्थिर ध्रुव वर्ण menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
शून्य
interactive_bootmenu(शून्य)
{
    यदि (! CONFIG_BOOTMENU)
        प्रत्यागम;
    पूर्णाङ्क show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    यदि (!show_boot_menu)
        प्रत्यागम;

    // skip menu if only one boot device and no TPM
    यदि (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_रिक्तानि(&BootList) && !BootList.first->अग्र) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        प्रारूप_लिख("\n");
        प्रत्यागम;
    }

    पूर्णाङ्क menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    पूर्णाङ्क menukey = romfile_loadint("etc/boot-menu-key", 1);
    पूर्णाङ्क अवलोकनम्_संहिता;
    यदि (menutime >= 0) {
        यावत् (get_keystroke(0) >= 0)
            ;

        वर्ण *bootmsg = romfile_loadfile("etc/boot-menu-message", रिक्त);
        प्रारूप_लिख("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        मुक्त(bootmsg);

        enable_bootsplash();
        अवलोकनम्_संहिता = get_keystroke(menutime);
        disable_bootsplash();
        यदि (अवलोकनम्_संहिता != menukey)
            प्रत्यागम;
    }

    यावत् (get_keystroke(0) >= 0)
        ;

    प्रारूप_लिख("Select boot device:\n\n");
    wait_तन्तवः();

    // Show menu items
    पूर्णाङ्क maxmenu = 0;
    संरचना bootentry_s *pos, *boot = रिक्त;
    hlist_for_each_प्रविष्टिः(pos, &BootList, ग्रन्थिः) {
        वर्ण desc[77];
        यदि (maxmenu >= ARRAY_आकार(menuchars)) {
            भङ्ग;
        }
        प्रारूप_लिख("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_आकार(desc)));
        maxmenu++;
    }
    यदि (tpm_can_show_menu()) {
        प्रारूप_लिख("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    पूर्णाङ्क esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    प्रति (;;) {
        पूर्णाङ्क keystroke = get_keystroke_पूर्णम्(1000);
        यदि (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            अनुवर्त;
        यदि (keystroke < 0) // timeout
            अनुवर्त;

        अवलोकनम्_संहिता = keystroke >> 8;
        पूर्णाङ्क कुञ्चिका_ascii = keystroke & 0xff;
        यदि (tpm_can_show_menu() && कुञ्चिका_ascii == 't') {
            प्रारूप_लिख("\n");
            tpm_menu();
        }
        यदि (अवलोकनम्_संहिता == 1) {
            // ESC
            प्रारूप_लिख("\n");
            प्रत्यागम;
        }

        maxmenu = 0;
        hlist_for_each_प्रविष्टिः(pos, &BootList, ग्रन्थिः) {
            यदि (maxmenu >= ARRAY_आकार(menuchars))
                भङ्ग;
            यदि (कुञ्चिका_ascii == menuchars[maxmenu]) {
                boot = pos;
                भङ्ग;
            }
            maxmenu++;
        }
        यदि (boot)
            भङ्ग;
    }
    प्रारूप_लिख("\n");

    // Find entry and make top priority.
    hlist_del(&boot->ग्रन्थिः);
    boot->प्राथमिकता = 0;
    hlist_सङ्कलय_शीर्ष(&boot->ग्रन्थिः, &BootList);
}

// BEV (Boot Execution Vector) list
संरचना bev_s {
    पूर्णाङ्क type;
    u32 सदिशम्;
};
स्थिर संरचना bev_s BEV[20];
स्थिर पूर्णाङ्क BEVCount;
स्थिर पूर्णाङ्क HaveHDBoot, HaveFDBoot;

स्थिर शून्य
सङ्कलय_bev(पूर्णाङ्क type, u32 सदिशम्)
{
    यदि (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        प्रत्यागम;
    यदि (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        प्रत्यागम;
    यदि (BEVCount >= ARRAY_आकार(BEV))
        प्रत्यागम;
    संरचना bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->सदिशम् = सदिशम्;
}

// Prepare for boot - show menu and run bcvs.
शून्य
bcv_prepboot(शून्य)
{
    यदि (! CONFIG_BOOT)
        प्रत्यागम;

    पूर्णाङ्क haltprio = find_prio("HALT");
    यदि (haltprio >= 0)
        bootentry_सङ्कलय(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    संरचना bootentry_s *pos;
    hlist_for_each_प्रविष्टिः(pos, &BootList, ग्रन्थिः) {
        विकल्प (pos->type) {
        पक्ष IPL_TYPE_BCV:
            call_bcv(pos->सदिशम्.seg, pos->सदिशम्.विस्थापनम्_2);
            सङ्कलय_bev(IPL_TYPE_HARDDISK, 0);
            भङ्ग;
        पक्ष IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            सङ्कलय_bev(IPL_TYPE_FLOPPY, 0);
            भङ्ग;
        पक्ष IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            सङ्कलय_bev(IPL_TYPE_HARDDISK, 0);
            भङ्ग;
        पक्ष IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        स्वभाव:
            सङ्कलय_bev(pos->type, pos->दत्त);
            भङ्ग;
        }
    }

    // If nothing added a floppy/hd boot - add it manually.
    सङ्कलय_bev(IPL_TYPE_FLOPPY, 0);
    सङ्कलय_bev(IPL_TYPE_HARDDISK, 0);
}


/****************************************************************
 * Boot code (int 18/19)
 ****************************************************************/

// Jump to a bootup entry point.
स्थिर शून्य
call_boot_प्रविष्टिः(संरचना segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.विस्थापनम्_2);
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF;
    br.संहिता = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
स्थिर शून्य
boot_disk(u8 bootdrv, पूर्णाङ्क checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    यदि (br.flags & F_CF) {
        प्रारूप_लिख("Boot failed: could not read the boot disk\n\n");
        प्रत्यागम;
    }

    यदि (checksig) {
        संरचना mbr_s *mbr = (शून्य*)0;
        यदि (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            प्रारूप_लिख("Boot failed: not a bootable disk\n\n");
            प्रत्यागम;
        }
    }

    tpm_सङ्कलय_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_प्रविष्टिः(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
स्थिर शून्य
boot_cdrom(संरचना drive_s *drive)
{
    यदि (! CONFIG_CDROM_BOOT)
        प्रत्यागम;
    प्रारूप_लिख("Booting from DVD/CD...\n");

    पूर्णाङ्क स्थितिः = cdrom_boot(drive);
    यदि (स्थितिः) {
        प्रारूप_लिख("Boot failed: Could not read from CDROM (code %04x)\n", स्थितिः);
        प्रत्यागम;
    }

    u8 bootdrv = CDEmu.emulated_drive;
    u16 bootseg = CDEmu.आरोपय_segment;

    tpm_सङ्कलय_cdrom(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_प्रविष्टिः(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CBFS payload
स्थिर शून्य
boot_cbfs(संरचना cbfs_file *file)
{
    यदि (!CONFIG_COREBOOT_FLASH)
        प्रत्यागम;
    प्रारूप_लिख("Booting from CBFS...\n");
    cbfs_सततम्_payload(file);
}

// Boot from a BEV entry on an optionrom.
स्थिर शून्य
boot_rom(u32 सदिशम्)
{
    प्रारूप_लिख("Booting from ROM...\n");
    संरचना segoff_s so;
    so.segoff = सदिशम्;
    call_boot_प्रविष्टिः(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
स्थिर शून्य
boot_fail(शून्य)
{
    यदि (BootRetryTime == (u32)-1)
        प्रारूप_लिख("No bootable device.\n");
    अन्यथा
        प्रारूप_लिख("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 अन्तः = irqtimer_calc(BootRetryTime);
    प्रति (;;) {
        यदि (BootRetryTime != (u32)-1 && irqtimer_check(अन्तः))
            भङ्ग;
        yield_toirq();
    }
    प्रारूप_लिख("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
स्थिर शून्य
do_boot(पूर्णाङ्क seq_nr)
{
    यदि (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    यदि (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    संरचना bev_s *ie = &BEV[seq_nr];
    विकल्प (ie->type) {
    पक्ष IPL_TYPE_FLOPPY:
        प्रारूप_लिख("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        भङ्ग;
    पक्ष IPL_TYPE_HARDDISK:
        प्रारूप_लिख("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        भङ्ग;
    पक्ष IPL_TYPE_CDROM:
        boot_cdrom((शून्य*)ie->सदिशम्);
        भङ्ग;
    पक्ष IPL_TYPE_CBFS:
        boot_cbfs((शून्य*)ie->सदिशम्);
        भङ्ग;
    पक्ष IPL_TYPE_BEV:
        boot_rom(ie->सदिशम्);
        भङ्ग;
    पक्ष IPL_TYPE_HALT:
        boot_fail();
        भङ्ग;
    }

    // Boot failed: invoke the boot recovery function
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

पूर्णाङ्क BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
शून्य VISIBLE32FLAT
handle_18(शून्य)
{
    debug_enter(रिक्त, DEBUG_HDL_18);
    पूर्णाङ्क seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
शून्य VISIBLE32FLAT
handle_19(शून्य)
{
    debug_enter(रिक्त, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
