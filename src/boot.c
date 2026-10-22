// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "fw/paravirt.h" // qemu_cfg_show_boot_menu
#અન્તર્ભાવ "hw/pci.h" // pci_bdf_to_*
#અન્તર્ભાવ "hw/pcidevice.h" // struct pci_device
#અન્તર્ભાવ "hw/rtc.h" // rtc_read
#અન્તર્ભાવ "hw/usb.h" // struct usbdevice_s
#અન્તર્ભાવ "list.h" // hlist_node
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "std/disk.h" // struct mbr_s
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // irqtimer_calc
#અન્તર્ભાવ "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
સ્થિર અક્ષર *
glob_prefix(અચલ અક્ષર *glob, અચલ અક્ષર *str)
{
    માટે (;;) {
        જો (!*glob && (!*str || *str == '/'))
            પાછા (અક્ષર*)str;
        જો (*glob == '*') {
            જો (!*str || *str == '/' || *str == glob[1])
                glob++;
            નહીંતર
                str++;
            ચાલુ;
        }
        જો (*glob != *str)
            પાછા ખાલી;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

સ્થિર અક્ષર *
build_pci_path(અક્ષર *buf, પૂર્ણાંક max, અચલ અક્ષર *devname, સંરચના pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    અક્ષર *p = buf;
    જો (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } નહીંતર {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        જો (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    પૂર્ણાંક dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    જો (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    પાછા p;
}

સ્થિર અક્ષર *
build_scsi_path(અક્ષર *buf, પૂર્ણાંક max,
                સંરચના pci_device *pci, પૂર્ણાંક target, પૂર્ણાંક lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    અક્ષર *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    પાછા p;
}

સ્થિર અક્ષર *
build_ata_path(અક્ષર *buf, પૂર્ણાંક max,
               સંરચના pci_device *pci, પૂર્ણાંક chanid, પૂર્ણાંક slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    અક્ષર *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    પાછા p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

પ્રકારવ્યાખ્યા સંરચના BootDeviceLCHS {
    અક્ષર *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

સ્થિર BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
સ્થિર પૂર્ણાંક Bios_Geometry_ગણતરી;

સ્થિર અક્ષર *
parse_u32(અક્ષર *cur, u32 *n)
{
    u32 m = 0;
    જો (cur) {
        જ્યારે ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        જો (*cur != '\0')
            cur++;
    }
    *n = m;
    પાછા cur;
}

સ્થિર શૂન્ય
loadBiosGeometry(શૂન્ય)
{
    જો (!CONFIG_HOST_BIOS_GEOMETRY)
        પાછા;
    અક્ષર *f = romfile_loadfile("bios-geometry", ખાલી);
    જો (!f)
        પાછા;

    પૂર્ણાંક i = 0;
    Bios_Geometry_ગણતરી = 1;
    જ્યારે (f[i]) {
        જો (f[i] == '\n')
            Bios_Geometry_ગણતરી++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_ગણતરી * કદ(BootDeviceLCHS));
    જો (!BiosGeometry) {
        warn_noalloc();
        મુક્ત(f);
        Bios_Geometry_ગણતરી = 0;
        પાછા;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    કરો {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        જો (f)
            *(f++) = '\0';
        અક્ષર *chs_કિંમતો = strchr(d->name, ' ');
        જો (chs_કિંમતો)
            *(chs_કિંમતો++) = '\0';
        chs_કિંમતો = parse_u32(chs_કિંમતો, &d->lcyls);
        chs_કિંમતો = parse_u32(chs_કિંમતો, &d->lheads);
        chs_કિંમતો = parse_u32(chs_કિંમતો, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } જ્યારે (f);
}

// Search the bios-geometry list for the given glob pattern.
સ્થિર BootDeviceLCHS *
boot_lchs_find(અચલ અક્ષર *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    પૂર્ણાંક i;
    માટે (i = 0; i < Bios_Geometry_ગણતરી; i++)
        જો (glob_prefix(glob, BiosGeometry[i].name))
            પાછા &BiosGeometry[i];
    પાછા ખાલી;
}

પૂર્ણાંક boot_lchs_find_pci_device(સંરચના pci_device *pci, સંરચના chs_s *chs)
{
    જો (!CONFIG_HOST_BIOS_GEOMETRY)
        પાછા -1;
    અક્ષર desc[256];
    build_pci_path(desc, કદ(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    જો (!b)
        પાછા -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    પાછા 0;
}

પૂર્ણાંક boot_lchs_find_scsi_device(સંરચના pci_device *pci, પૂર્ણાંક target, પૂર્ણાંક lun,
                               સંરચના chs_s *chs)
{
    જો (!CONFIG_HOST_BIOS_GEOMETRY)
        પાછા -1;
    જો (!pci)
        // support only pci machine for now
        પાછા -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    અક્ષર desc[256];
    build_scsi_path(desc, કદ(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    જો (!b)
        પાછા -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    પાછા 0;
}

પૂર્ણાંક boot_lchs_find_ata_device(સંરચના pci_device *pci, પૂર્ણાંક chanid, પૂર્ણાંક slave,
                              સંરચના chs_s *chs)
{
    જો (!CONFIG_HOST_BIOS_GEOMETRY)
        પાછા -1;
    જો (!pci)
        // support only pci machine for now
        પાછા -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    અક્ષર desc[256];
    build_ata_path(desc, કદ(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    જો (!b)
        પાછા -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    પાછા 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

સ્થિર અક્ષર **Bootorder VARVERIFY32INIT;
સ્થિર પૂર્ણાંક Bootorder_ગણતરી;

સ્થિર શૂન્ય
loadBootOrder(શૂન્ય)
{
    જો (!CONFIG_BOOTORDER)
        પાછા;

    અક્ષર *f = romfile_loadfile("bootorder", ખાલી);
    જો (!f)
        પાછા;

    પૂર્ણાંક i = 0;
    Bootorder_ગણતરી = 1;
    જ્યારે (f[i]) {
        જો (f[i] == '\n')
            Bootorder_ગણતરી++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_ગણતરી*કદ(અક્ષર*));
    જો (!Bootorder) {
        warn_noalloc();
        મુક્ત(f);
        Bootorder_ગણતરી = 0;
        પાછા;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    કરો {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        જો (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } જ્યારે (f);
}

// Search the bootorder list for the given glob pattern.
સ્થિર પૂર્ણાંક
find_prio(અચલ અક્ષર *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    પૂર્ણાંક i;
    માટે (i = 0; i < Bootorder_ગણતરી; i++)
        જો (glob_prefix(glob, Bootorder[i]))
            પાછા i+1;
    પાછા -1;
}

u8 is_bootprio_strict(શૂન્ય)
{
    સ્થિર પૂર્ણાંક prio_halt = -2;

    જો (prio_halt == -2)
        prio_halt = find_prio("HALT");
    પાછા prio_halt >= 0;
}

પૂર્ણાંક bootprio_find_pci_device(સંરચના pci_device *pci)
{
    જો (CONFIG_CSM)
        પાછા csm_bootprio_pci(pci);
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    અક્ષર desc[256];
    build_pci_path(desc, કદ(desc), "*", pci);
    પાછા find_prio(desc);
}

પૂર્ણાંક bootprio_find_mmio_device(શૂન્ય *mmio)
{
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    અક્ષર desc[256];
    snprintf(desc, કદ(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    પાછા find_prio(desc);
}

પૂર્ણાંક bootprio_find_scsi_device(સંરચના pci_device *pci, પૂર્ણાંક target, પૂર્ણાંક lun)
{
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    જો (!pci)
        // support only pci machine for now
        પાછા -1;
    અક્ષર desc[256];
    build_scsi_path(desc, કદ(desc), pci, target, lun);
    પાછા find_prio(desc);
}

પૂર્ણાંક bootprio_find_scsi_mmio_device(શૂન્ય *mmio, પૂર્ણાંક target, પૂર્ણાંક lun)
{
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    અક્ષર desc[256];
    snprintf(desc, કદ(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    પાછા find_prio(desc);
}

પૂર્ણાંક bootprio_find_ata_device(સંરચના pci_device *pci, પૂર્ણાંક chanid, પૂર્ણાંક slave)
{
    જો (CONFIG_CSM)
        પાછા csm_bootprio_ata(pci, chanid, slave);
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    જો (!pci)
        // support only pci machine for now
        પાછા -1;
    અક્ષર desc[256];
    build_ata_path(desc, કદ(desc), pci, chanid, slave);
    પાછા find_prio(desc);
}

પૂર્ણાંક bootprio_find_fdc_device(સંરચના pci_device *pci, પૂર્ણાંક port, પૂર્ણાંક fdid)
{
    જો (CONFIG_CSM)
        પાછા csm_bootprio_fdc(pci, port, fdid);
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    જો (!pci)
        // support only pci machine for now
        પાછા -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    અક્ષર desc[256], *p;
    p = build_pci_path(desc, કદ(desc), "isa", pci);
    snprintf(p, desc+કદ(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    પાછા find_prio(desc);
}

પૂર્ણાંક bootprio_find_pci_rom(સંરચના pci_device *pci, પૂર્ણાંક instance)
{
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    અક્ષર desc[256], *p;
    p = build_pci_path(desc, કદ(desc), "*", pci);
    જો (instance)
        snprintf(p, desc+કદ(desc)-p, ":rom%x", instance);
    પાછા find_prio(desc);
}

પૂર્ણાંક bootprio_find_named_rom(અચલ અક્ષર *name, પૂર્ણાંક instance)
{
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    અક્ષર desc[256], *p;
    p = desc + snprintf(desc, કદ(desc), "/rom@%s", name);
    જો (instance)
        snprintf(p, desc+કદ(desc)-p, ":rom%x", instance);
    પાછા find_prio(desc);
}

સ્થિર પૂર્ણાંક usb_portmap(સંરચના usbdevice_s *usbdev)
{
    જો (usbdev->hub->op->portmap)
        પાછા usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    પાછા usbdev->port + 1;
}

સ્થિર અક્ષર *
build_usb_path(અક્ષર *buf, પૂર્ણાંક max, સંરચના usbhub_s *hub)
{
    જો (!hub->usbdev)
        // Root hub - nothing to add.
        પાછા buf;
    અક્ષર *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    પાછા p;
}

પૂર્ણાંક bootprio_find_usb(સંરચના usbdevice_s *usbdev, પૂર્ણાંક lun)
{
    જો (!CONFIG_BOOTORDER)
        પાછા -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    અક્ષર desc[256], *p;

    જો (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, કદ(desc), "usb", usbdev->hub->cntl->pci);
    નહીંતર જો (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, કદ(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    નહીંતર
        પાછા -1;

    p = build_usb_path(p, desc+કદ(desc)-p, usbdev->hub);
    snprintf(p, desc+કદ(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    પૂર્ણાંક ret = find_prio(desc);
    જો (ret >= 0)
        પાછા ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+કદ(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    પાછા find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

સ્થિર પૂર્ણાંક BootRetryTime;
સ્થિર પૂર્ણાંક CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

સ્થિર પૂર્ણાંક DefaultFloppyPrio = 101;
સ્થિર પૂર્ણાંક DefaultCDPrio     = 102;
સ્થિર પૂર્ણાંક DefaultHDPrio     = 103;
સ્થિર પૂર્ણાંક DefaultBEVPrio    = 104;

શૂન્ય
boot_init(શૂન્ય)
{
    જો (! CONFIG_BOOT)
        પાછા;

    જો (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        જો (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        પૂર્ણાંક i;
        માટે (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            પસંદગી (val) {
            સ્થિતિ 1: DefaultFloppyPrio = i; તોડો;
            સ્થિતિ 2: DefaultHDPrio = i;     તોડો;
            સ્થિતિ 3: DefaultCDPrio = i;     તોડો;
            સ્થિતિ 4: DefaultBEVPrio = i;    તોડો;
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

સંરચના bootentry_s {
    પૂર્ણાંક type;
    સંઘ {
        u32 data;
        સંરચના segoff_s vector;
        સંરચના drive_s *drive;
    };
    પૂર્ણાંક priority;
    અચલ અક્ષર *description;
    સંરચના hlist_ગાંઠ ગાંઠ;
};
સ્થિર સંરચના hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

સ્થિર શૂન્ય
bootentry_add(પૂર્ણાંક type, પૂર્ણાંક prio, u32 data, અચલ અક્ષર *desc)
{
    જો (! CONFIG_BOOT)
        પાછા;
    સંરચના bootentry_s *be = malloc_tmp(કદ(*be));
    જો (!be) {
        warn_noalloc();
        પાછા;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    સંરચના hlist_ગાંઠ **pprev;
    સંરચના bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, ગાંઠ) {
        જો (be->priority < pos->priority)
            તોડો;
        જો (be->priority > pos->priority)
            ચાલુ;
        જો (be->type < pos->type)
            તોડો;
        જો (be->type > pos->type)
            ચાલુ;
        જો (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            તોડો;
    }
    hlist_add(&be->ગાંઠ, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
સ્થિર અંતઃરેખ પૂર્ણાંક defPrio(પૂર્ણાંક priority, પૂર્ણાંક defaultprio) {
    પાછા (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
શૂન્ય
boot_add_bev(u16 seg, u16 bev, u16 desc, પૂર્ણાંક prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
શૂન્ય
boot_add_bcv(u16 seg, u16 ip, u16 desc, પૂર્ણાંક prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

શૂન્ય
boot_add_floppy(સંરચના drive_s *drive, અચલ અક્ષર *desc, પૂર્ણાંક prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

શૂન્ય
boot_add_hd(સંરચના drive_s *drive, અચલ અક્ષર *desc, પૂર્ણાંક prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

શૂન્ય
boot_add_cd(સંરચના drive_s *drive, અચલ અક્ષર *desc, પૂર્ણાંક prio)
{
    જો (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        અક્ષર *extra = cdrom_media_info(drive);
        જો (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            મુક્ત(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
શૂન્ય
boot_add_cbfs(શૂન્ય *data, અચલ અક્ષર *desc, પૂર્ણાંક prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
સ્થિર પૂર્ણાંક
check_for_keystroke(શૂન્ય)
{
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    પાછા !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
સ્થિર પૂર્ણાંક
get_raw_keystroke(શૂન્ય)
{
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    પાછા br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
પૂર્ણાંક
get_keystroke_full(પૂર્ણાંક msec)
{
    u32 end = irqtimer_calc(msec);
    માટે (;;) {
        જો (check_for_keystroke())
            પાછા get_raw_keystroke();
        જો (irqtimer_check(end))
            પાછા -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
પૂર્ણાંક
get_keystroke(પૂર્ણાંક msec)
{
    પૂર્ણાંક keystroke = get_keystroke_full(msec);

    જો (keystroke < 0)
        પાછા keystroke;
    પાછા keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

સ્થિર અચલ અક્ષર menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
શૂન્ય
interactive_bootmenu(શૂન્ય)
{
    જો (! CONFIG_BOOTMENU)
        પાછા;
    પૂર્ણાંક show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    જો (!show_boot_menu)
        પાછા;

    // skip menu if only one boot device and no TPM
    જો (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        પાછા;
    }

    પૂર્ણાંક menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    પૂર્ણાંક menukey = romfile_loadint("etc/boot-menu-key", 1);
    પૂર્ણાંક scan_code;
    જો (menutime >= 0) {
        જ્યારે (get_keystroke(0) >= 0)
            ;

        અક્ષર *bootmsg = romfile_loadfile("etc/boot-menu-message", ખાલી);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        મુક્ત(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        જો (scan_code != menukey)
            પાછા;
    }

    જ્યારે (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    પૂર્ણાંક maxmenu = 0;
    સંરચના bootentry_s *pos, *boot = ખાલી;
    hlist_for_each_entry(pos, &BootList, ગાંઠ) {
        અક્ષર desc[77];
        જો (maxmenu >= ARRAY_SIZE(menuchars)) {
            તોડો;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    જો (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    પૂર્ણાંક esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    માટે (;;) {
        પૂર્ણાંક keystroke = get_keystroke_full(1000);
        જો (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            ચાલુ;
        જો (keystroke < 0) // timeout
            ચાલુ;

        scan_code = keystroke >> 8;
        પૂર્ણાંક ચાવી_ascii = keystroke & 0xff;
        જો (tpm_can_show_menu() && ચાવી_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        જો (scan_code == 1) {
            // ESC
            printf("\n");
            પાછા;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, ગાંઠ) {
            જો (maxmenu >= ARRAY_SIZE(menuchars))
                તોડો;
            જો (ચાવી_ascii == menuchars[maxmenu]) {
                boot = pos;
                તોડો;
            }
            maxmenu++;
        }
        જો (boot)
            તોડો;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->ગાંઠ);
    boot->priority = 0;
    hlist_add_head(&boot->ગાંઠ, &BootList);
}

// BEV (Boot Execution Vector) list
સંરચના bev_s {
    પૂર્ણાંક type;
    u32 vector;
};
સ્થિર સંરચના bev_s BEV[20];
સ્થિર પૂર્ણાંક BEVCount;
સ્થિર પૂર્ણાંક HaveHDBoot, HaveFDBoot;

સ્થિર શૂન્ય
add_bev(પૂર્ણાંક type, u32 vector)
{
    જો (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        પાછા;
    જો (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        પાછા;
    જો (BEVCount >= ARRAY_SIZE(BEV))
        પાછા;
    સંરચના bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
શૂન્ય
bcv_prepboot(શૂન્ય)
{
    જો (! CONFIG_BOOT)
        પાછા;

    પૂર્ણાંક haltprio = find_prio("HALT");
    જો (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    સંરચના bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, ગાંઠ) {
        પસંદગી (pos->type) {
        સ્થિતિ IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            તોડો;
        સ્થિતિ IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            તોડો;
        સ્થિતિ IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            તોડો;
        સ્થિતિ IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        મૂળભૂત:
            add_bev(pos->type, pos->data);
            તોડો;
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
સ્થિર શૂન્ય
call_boot_entry(સંરચના segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
સ્થિર શૂન્ય
boot_disk(u8 bootdrv, પૂર્ણાંક checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    જો (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        પાછા;
    }

    જો (checksig) {
        સંરચના mbr_s *mbr = (શૂન્ય*)0;
        જો (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            પાછા;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
સ્થિર શૂન્ય
boot_cdrom(સંરચના drive_s *drive)
{
    જો (! CONFIG_CDROM_BOOT)
        પાછા;
    printf("Booting from DVD/CD...\n");

    પૂર્ણાંક status = cdrom_boot(drive);
    જો (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        પાછા;
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
સ્થિર શૂન્ય
boot_cbfs(સંરચના cbfs_file *file)
{
    જો (!CONFIG_COREBOOT_FLASH)
        પાછા;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
સ્થિર શૂન્ય
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    સંરચના segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
સ્થિર શૂન્ય
boot_fail(શૂન્ય)
{
    જો (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    નહીંતર
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    માટે (;;) {
        જો (BootRetryTime != (u32)-1 && irqtimer_check(end))
            તોડો;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
સ્થિર શૂન્ય
do_boot(પૂર્ણાંક seq_nr)
{
    જો (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    જો (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    સંરચના bev_s *ie = &BEV[seq_nr];
    પસંદગી (ie->type) {
    સ્થિતિ IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        તોડો;
    સ્થિતિ IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        તોડો;
    સ્થિતિ IPL_TYPE_CDROM:
        boot_cdrom((શૂન્ય*)ie->vector);
        તોડો;
    સ્થિતિ IPL_TYPE_CBFS:
        boot_cbfs((શૂન્ય*)ie->vector);
        તોડો;
    સ્થિતિ IPL_TYPE_BEV:
        boot_rom(ie->vector);
        તોડો;
    સ્થિતિ IPL_TYPE_HALT:
        boot_fail();
        તોડો;
    }

    // Boot failed: invoke the boot recovery function
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

પૂર્ણાંક BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
શૂન્ય VISIBLE32FLAT
handle_18(શૂન્ય)
{
    debug_enter(ખાલી, DEBUG_HDL_18);
    પૂર્ણાંક seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
શૂન્ય VISIBLE32FLAT
handle_19(શૂન્ય)
{
    debug_enter(ખાલી, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
