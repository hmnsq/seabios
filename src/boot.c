// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "block.h" // struct drive_s
#समावेश "bregs.h" // struct bregs
#समावेश "config.h" // CONFIG_*
#समावेश "fw/paravirt.h" // qemu_cfg_show_boot_menu
#समावेश "hw/pci.h" // pci_bdf_to_*
#समावेश "hw/pcidevice.h" // struct pci_device
#समावेश "hw/rtc.h" // rtc_read
#समावेश "hw/usb.h" // struct usbdevice_s
#समावेश "list.h" // hlist_node
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_loadint
#समावेश "std/disk.h" // struct mbr_s
#समावेश "string.h" // memset
#समावेश "util.h" // irqtimer_calc
#समावेश "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
अटल अक्षर *
glob_prefix(स्थिर अक्षर *glob, स्थिर अक्षर *str)
{
    क्रम (;;) {
        अगर (!*glob && (!*str || *str == '/'))
            वापस (अक्षर*)str;
        अगर (*glob == '*') {
            अगर (!*str || *str == '/' || *str == glob[1])
                glob++;
            अन्यथा
                str++;
            जारी;
        }
        अगर (*glob != *str)
            वापस शून्य;
        glob++;
        str++;
    }
}

#घोषणा FW_PCI_DOMAIN "/pci@i0cf8"

अटल अक्षर *
build_pci_path(अक्षर *buf, पूर्णांक max, स्थिर अक्षर *devname, काष्ठा pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    अक्षर *p = buf;
    अगर (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } अन्यथा {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        अगर (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    पूर्णांक dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    अगर (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    वापस p;
}

अटल अक्षर *
build_scsi_path(अक्षर *buf, पूर्णांक max,
                काष्ठा pci_device *pci, पूर्णांक target, पूर्णांक lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    अक्षर *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    वापस p;
}

अटल अक्षर *
build_ata_path(अक्षर *buf, पूर्णांक max,
               काष्ठा pci_device *pci, पूर्णांक chanid, पूर्णांक slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    अक्षर *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    वापस p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

प्रकार काष्ठा BootDeviceLCHS {
    अक्षर *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

अटल BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
अटल पूर्णांक Bios_Geometry_गिनती;

अटल अक्षर *
parse_u32(अक्षर *cur, u32 *n)
{
    u32 m = 0;
    अगर (cur) {
        जबतक ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        अगर (*cur != '\0')
            cur++;
    }
    *n = m;
    वापस cur;
}

अटल व्योम
loadBiosGeometry(व्योम)
{
    अगर (!CONFIG_HOST_BIOS_GEOMETRY)
        वापस;
    अक्षर *f = romfile_loadfile("bios-geometry", शून्य);
    अगर (!f)
        वापस;

    पूर्णांक i = 0;
    Bios_Geometry_गिनती = 1;
    जबतक (f[i]) {
        अगर (f[i] == '\n')
            Bios_Geometry_गिनती++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_गिनती * माप(BootDeviceLCHS));
    अगर (!BiosGeometry) {
        warn_noalloc();
        मुक्त(f);
        Bios_Geometry_गिनती = 0;
        वापस;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    करो {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = म_अक्षर(f, '\n');
        अगर (f)
            *(f++) = '\0';
        अक्षर *chs_मूल्यावली = म_अक्षर(d->name, ' ');
        अगर (chs_मूल्यावली)
            *(chs_मूल्यावली++) = '\0';
        chs_मूल्यावली = parse_u32(chs_मूल्यावली, &d->lcyls);
        chs_मूल्यावली = parse_u32(chs_मूल्यावली, &d->lheads);
        chs_मूल्यावली = parse_u32(chs_मूल्यावली, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } जबतक (f);
}

// Search the bios-geometry list for the given glob pattern.
अटल BootDeviceLCHS *
boot_lchs_find(स्थिर अक्षर *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    पूर्णांक i;
    क्रम (i = 0; i < Bios_Geometry_गिनती; i++)
        अगर (glob_prefix(glob, BiosGeometry[i].name))
            वापस &BiosGeometry[i];
    वापस शून्य;
}

पूर्णांक boot_lchs_find_pci_device(काष्ठा pci_device *pci, काष्ठा chs_s *chs)
{
    अगर (!CONFIG_HOST_BIOS_GEOMETRY)
        वापस -1;
    अक्षर desc[256];
    build_pci_path(desc, माप(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    अगर (!b)
        वापस -1;
    chs->cylinder = (u16)b->lcyls;
    chs->शीर्ष = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    वापस 0;
}

पूर्णांक boot_lchs_find_scsi_device(काष्ठा pci_device *pci, पूर्णांक target, पूर्णांक lun,
                               काष्ठा chs_s *chs)
{
    अगर (!CONFIG_HOST_BIOS_GEOMETRY)
        वापस -1;
    अगर (!pci)
        // support only pci machine for now
        वापस -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    अक्षर desc[256];
    build_scsi_path(desc, माप(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    अगर (!b)
        वापस -1;
    chs->cylinder = (u16)b->lcyls;
    chs->शीर्ष = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    वापस 0;
}

पूर्णांक boot_lchs_find_ata_device(काष्ठा pci_device *pci, पूर्णांक chanid, पूर्णांक slave,
                              काष्ठा chs_s *chs)
{
    अगर (!CONFIG_HOST_BIOS_GEOMETRY)
        वापस -1;
    अगर (!pci)
        // support only pci machine for now
        वापस -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    अक्षर desc[256];
    build_ata_path(desc, माप(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    अगर (!b)
        वापस -1;
    chs->cylinder = (u16)b->lcyls;
    chs->शीर्ष = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    वापस 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

अटल अक्षर **Bootorder VARVERIFY32INIT;
अटल पूर्णांक Bootorder_गिनती;

अटल व्योम
loadBootOrder(व्योम)
{
    अगर (!CONFIG_BOOTORDER)
        वापस;

    अक्षर *f = romfile_loadfile("bootorder", शून्य);
    अगर (!f)
        वापस;

    पूर्णांक i = 0;
    Bootorder_गिनती = 1;
    जबतक (f[i]) {
        अगर (f[i] == '\n')
            Bootorder_गिनती++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_गिनती*माप(अक्षर*));
    अगर (!Bootorder) {
        warn_noalloc();
        मुक्त(f);
        Bootorder_गिनती = 0;
        वापस;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    करो {
        Bootorder[i] = f;
        f = म_अक्षर(f, '\n');
        अगर (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } जबतक (f);
}

// Search the bootorder list for the given glob pattern.
अटल पूर्णांक
find_prio(स्थिर अक्षर *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    पूर्णांक i;
    क्रम (i = 0; i < Bootorder_गिनती; i++)
        अगर (glob_prefix(glob, Bootorder[i]))
            वापस i+1;
    वापस -1;
}

u8 is_bootprio_strict(व्योम)
{
    अटल पूर्णांक prio_halt = -2;

    अगर (prio_halt == -2)
        prio_halt = find_prio("HALT");
    वापस prio_halt >= 0;
}

पूर्णांक bootprio_find_pci_device(काष्ठा pci_device *pci)
{
    अगर (CONFIG_CSM)
        वापस csm_bootprio_pci(pci);
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    अक्षर desc[256];
    build_pci_path(desc, माप(desc), "*", pci);
    वापस find_prio(desc);
}

पूर्णांक bootprio_find_mmio_device(व्योम *mmio)
{
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    अक्षर desc[256];
    snprintf(desc, माप(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    वापस find_prio(desc);
}

पूर्णांक bootprio_find_scsi_device(काष्ठा pci_device *pci, पूर्णांक target, पूर्णांक lun)
{
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    अगर (!pci)
        // support only pci machine for now
        वापस -1;
    अक्षर desc[256];
    build_scsi_path(desc, माप(desc), pci, target, lun);
    वापस find_prio(desc);
}

पूर्णांक bootprio_find_scsi_mmio_device(व्योम *mmio, पूर्णांक target, पूर्णांक lun)
{
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    अक्षर desc[256];
    snprintf(desc, माप(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    वापस find_prio(desc);
}

पूर्णांक bootprio_find_ata_device(काष्ठा pci_device *pci, पूर्णांक chanid, पूर्णांक slave)
{
    अगर (CONFIG_CSM)
        वापस csm_bootprio_ata(pci, chanid, slave);
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    अगर (!pci)
        // support only pci machine for now
        वापस -1;
    अक्षर desc[256];
    build_ata_path(desc, माप(desc), pci, chanid, slave);
    वापस find_prio(desc);
}

पूर्णांक bootprio_find_fdc_device(काष्ठा pci_device *pci, पूर्णांक port, पूर्णांक fdid)
{
    अगर (CONFIG_CSM)
        वापस csm_bootprio_fdc(pci, port, fdid);
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    अगर (!pci)
        // support only pci machine for now
        वापस -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    अक्षर desc[256], *p;
    p = build_pci_path(desc, माप(desc), "isa", pci);
    snprintf(p, desc+माप(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    वापस find_prio(desc);
}

पूर्णांक bootprio_find_pci_rom(काष्ठा pci_device *pci, पूर्णांक instance)
{
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    अक्षर desc[256], *p;
    p = build_pci_path(desc, माप(desc), "*", pci);
    अगर (instance)
        snprintf(p, desc+माप(desc)-p, ":rom%x", instance);
    वापस find_prio(desc);
}

पूर्णांक bootprio_find_named_rom(स्थिर अक्षर *name, पूर्णांक instance)
{
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    अक्षर desc[256], *p;
    p = desc + snprintf(desc, माप(desc), "/rom@%s", name);
    अगर (instance)
        snprintf(p, desc+माप(desc)-p, ":rom%x", instance);
    वापस find_prio(desc);
}

अटल पूर्णांक usb_portmap(काष्ठा usbdevice_s *usbdev)
{
    अगर (usbdev->hub->op->portmap)
        वापस usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    वापस usbdev->port + 1;
}

अटल अक्षर *
build_usb_path(अक्षर *buf, पूर्णांक max, काष्ठा usbhub_s *hub)
{
    अगर (!hub->usbdev)
        // Root hub - nothing to add.
        वापस buf;
    अक्षर *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    वापस p;
}

पूर्णांक bootprio_find_usb(काष्ठा usbdevice_s *usbdev, पूर्णांक lun)
{
    अगर (!CONFIG_BOOTORDER)
        वापस -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    अक्षर desc[256], *p;

    अगर (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, माप(desc), "usb", usbdev->hub->cntl->pci);
    अन्यथा अगर (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, माप(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    अन्यथा
        वापस -1;

    p = build_usb_path(p, desc+माप(desc)-p, usbdev->hub);
    snprintf(p, desc+माप(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    पूर्णांक ret = find_prio(desc);
    अगर (ret >= 0)
        वापस ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+माप(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    वापस find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

अटल पूर्णांक BootRetryTime;
अटल पूर्णांक CheckFloppySig = 1;

#घोषणा DEFAULT_PRIO           9999

अटल पूर्णांक DefaultFloppyPrio = 101;
अटल पूर्णांक DefaultCDPrio     = 102;
अटल पूर्णांक DefaultHDPrio     = 103;
अटल पूर्णांक DefaultBEVPrio    = 104;

व्योम
boot_init(व्योम)
{
    अगर (! CONFIG_BOOT)
        वापस;

    अगर (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        अगर (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        पूर्णांक i;
        क्रम (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            चयन (val) {
            हाल 1: DefaultFloppyPrio = i; अवरोध;
            हाल 2: DefaultHDPrio = i;     अवरोध;
            हाल 3: DefaultCDPrio = i;     अवरोध;
            हाल 4: DefaultBEVPrio = i;    अवरोध;
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

काष्ठा bootentry_s {
    पूर्णांक type;
    जोड़ {
        u32 तथ्य;
        काष्ठा segoff_s vector;
        काष्ठा drive_s *drive;
    };
    पूर्णांक priority;
    स्थिर अक्षर *description;
    काष्ठा hlist_गाँठ गाँठ;
};
अटल काष्ठा hlist_शीर्ष BootList VARVERIFY32INIT;

#घोषणा IPL_TYPE_FLOPPY      0x01
#घोषणा IPL_TYPE_HARDDISK    0x02
#घोषणा IPL_TYPE_CDROM       0x03
#घोषणा IPL_TYPE_CBFS        0x20
#घोषणा IPL_TYPE_BEV         0x80
#घोषणा IPL_TYPE_BCV         0x81
#घोषणा IPL_TYPE_HALT        0xf0

अटल व्योम
bootentry_add(पूर्णांक type, पूर्णांक prio, u32 तथ्य, स्थिर अक्षर *desc)
{
    अगर (! CONFIG_BOOT)
        वापस;
    काष्ठा bootentry_s *be = malloc_tmp(माप(*be));
    अगर (!be) {
        warn_noalloc();
        वापस;
    }
    be->type = type;
    be->priority = prio;
    be->तथ्य = तथ्य;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, तथ्य);

    // Add entry in sorted order.
    काष्ठा hlist_गाँठ **pprev;
    काष्ठा bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, गाँठ) {
        अगर (be->priority < pos->priority)
            अवरोध;
        अगर (be->priority > pos->priority)
            जारी;
        अगर (be->type < pos->type)
            अवरोध;
        अगर (be->type > pos->type)
            जारी;
        अगर (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            अवरोध;
    }
    hlist_add(&be->गाँठ, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
अटल अंतरभूत पूर्णांक defPrio(पूर्णांक priority, पूर्णांक defaultprio) {
    वापस (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
व्योम
boot_add_bev(u16 seg, u16 bev, u16 desc, पूर्णांक prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
व्योम
boot_add_bcv(u16 seg, u16 ip, u16 desc, पूर्णांक prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

व्योम
boot_add_floppy(काष्ठा drive_s *drive, स्थिर अक्षर *desc, पूर्णांक prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

व्योम
boot_add_hd(काष्ठा drive_s *drive, स्थिर अक्षर *desc, पूर्णांक prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

व्योम
boot_add_cd(काष्ठा drive_s *drive, स्थिर अक्षर *desc, पूर्णांक prio)
{
    अगर (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        अक्षर *extra = cdrom_media_info(drive);
        अगर (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            मुक्त(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
व्योम
boot_add_cbfs(व्योम *तथ्य, स्थिर अक्षर *desc, पूर्णांक prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)तथ्य, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
अटल पूर्णांक
check_for_keystroke(व्योम)
{
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    वापस !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
अटल पूर्णांक
get_raw_keystroke(व्योम)
{
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    वापस br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
पूर्णांक
get_keystroke_full(पूर्णांक msec)
{
    u32 end = irqtimer_calc(msec);
    क्रम (;;) {
        अगर (check_for_keystroke())
            वापस get_raw_keystroke();
        अगर (irqtimer_check(end))
            वापस -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
पूर्णांक
get_keystroke(पूर्णांक msec)
{
    पूर्णांक keystroke = get_keystroke_full(msec);

    अगर (keystroke < 0)
        वापस keystroke;
    वापस keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#घोषणा DEFAULT_BOOTMENU_WAIT 2500

अटल स्थिर अक्षर menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
व्योम
interactive_bootmenu(व्योम)
{
    अगर (! CONFIG_BOOTMENU)
        वापस;
    पूर्णांक show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    अगर (!show_boot_menu)
        वापस;

    // skip menu if only one boot device and no TPM
    अगर (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->अग्र) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        म_लिखो("\n");
        वापस;
    }

    पूर्णांक menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    पूर्णांक menukey = romfile_loadint("etc/boot-menu-key", 1);
    पूर्णांक scan_code;
    अगर (menutime >= 0) {
        जबतक (get_keystroke(0) >= 0)
            ;

        अक्षर *bootmsg = romfile_loadfile("etc/boot-menu-message", शून्य);
        म_लिखो("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        मुक्त(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        अगर (scan_code != menukey)
            वापस;
    }

    जबतक (get_keystroke(0) >= 0)
        ;

    म_लिखो("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    पूर्णांक maxmenu = 0;
    काष्ठा bootentry_s *pos, *boot = शून्य;
    hlist_for_each_entry(pos, &BootList, गाँठ) {
        अक्षर desc[77];
        अगर (maxmenu >= ARRAY_आकार(menuchars)) {
            अवरोध;
        }
        म_लिखो("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_आकार(desc)));
        maxmenu++;
    }
    अगर (tpm_can_show_menu()) {
        म_लिखो("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    पूर्णांक esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    क्रम (;;) {
        पूर्णांक keystroke = get_keystroke_full(1000);
        अगर (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            जारी;
        अगर (keystroke < 0) // timeout
            जारी;

        scan_code = keystroke >> 8;
        पूर्णांक कुंजी_ascii = keystroke & 0xff;
        अगर (tpm_can_show_menu() && कुंजी_ascii == 't') {
            म_लिखो("\n");
            tpm_menu();
        }
        अगर (scan_code == 1) {
            // ESC
            म_लिखो("\n");
            वापस;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, गाँठ) {
            अगर (maxmenu >= ARRAY_आकार(menuchars))
                अवरोध;
            अगर (कुंजी_ascii == menuchars[maxmenu]) {
                boot = pos;
                अवरोध;
            }
            maxmenu++;
        }
        अगर (boot)
            अवरोध;
    }
    म_लिखो("\n");

    // Find entry and make top priority.
    hlist_del(&boot->गाँठ);
    boot->priority = 0;
    hlist_add_शीर्ष(&boot->गाँठ, &BootList);
}

// BEV (Boot Execution Vector) list
काष्ठा bev_s {
    पूर्णांक type;
    u32 vector;
};
अटल काष्ठा bev_s BEV[20];
अटल पूर्णांक BEVCount;
अटल पूर्णांक HaveHDBoot, HaveFDBoot;

अटल व्योम
add_bev(पूर्णांक type, u32 vector)
{
    अगर (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        वापस;
    अगर (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        वापस;
    अगर (BEVCount >= ARRAY_आकार(BEV))
        वापस;
    काष्ठा bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
व्योम
bcv_prepboot(व्योम)
{
    अगर (! CONFIG_BOOT)
        वापस;

    पूर्णांक haltprio = find_prio("HALT");
    अगर (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    काष्ठा bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, गाँठ) {
        चयन (pos->type) {
        हाल IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            अवरोध;
        हाल IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            अवरोध;
        हाल IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            अवरोध;
        हाल IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        शेष:
            add_bev(pos->type, pos->तथ्य);
            अवरोध;
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
अटल व्योम
call_boot_entry(काष्ठा segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
अटल व्योम
boot_disk(u8 bootdrv, पूर्णांक checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    अगर (br.flags & F_CF) {
        म_लिखो("Boot failed: could not read the boot disk\n\n");
        वापस;
    }

    अगर (checksig) {
        काष्ठा mbr_s *mbr = (व्योम*)0;
        अगर (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            म_लिखो("Boot failed: not a bootable disk\n\n");
            वापस;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
अटल व्योम
boot_cdrom(काष्ठा drive_s *drive)
{
    अगर (! CONFIG_CDROM_BOOT)
        वापस;
    म_लिखो("Booting from DVD/CD...\n");

    पूर्णांक status = cdrom_boot(drive);
    अगर (status) {
        म_लिखो("Boot failed: Could not read from CDROM (code %04x)\n", status);
        वापस;
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
अटल व्योम
boot_cbfs(काष्ठा cbfs_file *file)
{
    अगर (!CONFIG_COREBOOT_FLASH)
        वापस;
    म_लिखो("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
अटल व्योम
boot_rom(u32 vector)
{
    म_लिखो("Booting from ROM...\n");
    काष्ठा segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
अटल व्योम
boot_fail(व्योम)
{
    अगर (BootRetryTime == (u32)-1)
        म_लिखो("No bootable device.\n");
    अन्यथा
        म_लिखो("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    क्रम (;;) {
        अगर (BootRetryTime != (u32)-1 && irqtimer_check(end))
            अवरोध;
        yield_toirq();
    }
    म_लिखो("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
अटल व्योम
do_boot(पूर्णांक seq_nr)
{
    अगर (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    अगर (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    काष्ठा bev_s *ie = &BEV[seq_nr];
    चयन (ie->type) {
    हाल IPL_TYPE_FLOPPY:
        म_लिखो("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        अवरोध;
    हाल IPL_TYPE_HARDDISK:
        म_लिखो("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        अवरोध;
    हाल IPL_TYPE_CDROM:
        boot_cdrom((व्योम*)ie->vector);
        अवरोध;
    हाल IPL_TYPE_CBFS:
        boot_cbfs((व्योम*)ie->vector);
        अवरोध;
    हाल IPL_TYPE_BEV:
        boot_rom(ie->vector);
        अवरोध;
    हाल IPL_TYPE_HALT:
        boot_fail();
        अवरोध;
    }

    // Boot failed: invoke the boot recovery function
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

पूर्णांक BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
व्योम VISIBLE32FLAT
handle_18(व्योम)
{
    debug_enter(शून्य, DEBUG_HDL_18);
    पूर्णांक seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
व्योम VISIBLE32FLAT
handle_19(व्योम)
{
    debug_enter(शून्य, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
