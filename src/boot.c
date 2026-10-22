// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "fw/paravirt.h" // qemu_cfg_show_boot_menu
#അന്തര്ഭാവ "hw/pci.h" // pci_bdf_to_*
#അന്തര്ഭാവ "hw/pcidevice.h" // struct pci_device
#അന്തര്ഭാവ "hw/rtc.h" // rtc_read
#അന്തര്ഭാവ "hw/usb.h" // struct usbdevice_s
#അന്തര്ഭാവ "list.h" // hlist_node
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "std/disk.h" // struct mbr_s
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // irqtimer_calc
#അന്തര്ഭാവ "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
സ്ഥിതം അക്ഷരം *
glob_prefix(സ്ഥിരം അക്ഷരം *glob, സ്ഥിരം അക്ഷരം *str)
{
    വേണ്ടി (;;) {
        എങ്കിൽ (!*glob && (!*str || *str == '/'))
            തിരികെ (അക്ഷരം*)str;
        എങ്കിൽ (*glob == '*') {
            എങ്കിൽ (!*str || *str == '/' || *str == glob[1])
                glob++;
            അല്ലെങ്കിൽ
                str++;
            തുടരുക;
        }
        എങ്കിൽ (*glob != *str)
            തിരികെ ശൂന്യ;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

സ്ഥിതം അക്ഷരം *
build_pci_path(അക്ഷരം *buf, പൂർണ്ണസംഖ്യ max, സ്ഥിരം അക്ഷരം *devname, ഘടന pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    അക്ഷരം *p = buf;
    എങ്കിൽ (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } അല്ലെങ്കിൽ {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        എങ്കിൽ (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    പൂർണ്ണസംഖ്യ dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    എങ്കിൽ (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    തിരികെ p;
}

സ്ഥിതം അക്ഷരം *
build_scsi_path(അക്ഷരം *buf, പൂർണ്ണസംഖ്യ max,
                ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ target, പൂർണ്ണസംഖ്യ lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    അക്ഷരം *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    തിരികെ p;
}

സ്ഥിതം അക്ഷരം *
build_ata_path(അക്ഷരം *buf, പൂർണ്ണസംഖ്യ max,
               ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ chanid, പൂർണ്ണസംഖ്യ slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    അക്ഷരം *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    തിരികെ p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

തരനിർവചനം ഘടന BootDeviceLCHS {
    അക്ഷരം *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

സ്ഥിതം BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
സ്ഥിതം പൂർണ്ണസംഖ്യ Bios_Geometry_എണ്ണം;

സ്ഥിതം അക്ഷരം *
parse_u32(അക്ഷരം *cur, u32 *n)
{
    u32 m = 0;
    എങ്കിൽ (cur) {
        ആകുമ്പോൾ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        എങ്കിൽ (*cur != '\0')
            cur++;
    }
    *n = m;
    തിരികെ cur;
}

സ്ഥിതം ശൂന്യം
loadBiosGeometry(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_HOST_BIOS_GEOMETRY)
        തിരികെ;
    അക്ഷരം *f = romfile_loadfile("bios-geometry", ശൂന്യ);
    എങ്കിൽ (!f)
        തിരികെ;

    പൂർണ്ണസംഖ്യ i = 0;
    Bios_Geometry_എണ്ണം = 1;
    ആകുമ്പോൾ (f[i]) {
        എങ്കിൽ (f[i] == '\n')
            Bios_Geometry_എണ്ണം++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_എണ്ണം * വലുപ്പം(BootDeviceLCHS));
    എങ്കിൽ (!BiosGeometry) {
        warn_noalloc();
        സ്വതന്ത്രം(f);
        Bios_Geometry_എണ്ണം = 0;
        തിരികെ;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    ചെയ്യുക {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        എങ്കിൽ (f)
            *(f++) = '\0';
        അക്ഷരം *chs_മൂല്യങ്ങൾ = strchr(d->name, ' ');
        എങ്കിൽ (chs_മൂല്യങ്ങൾ)
            *(chs_മൂല്യങ്ങൾ++) = '\0';
        chs_മൂല്യങ്ങൾ = parse_u32(chs_മൂല്യങ്ങൾ, &d->lcyls);
        chs_മൂല്യങ്ങൾ = parse_u32(chs_മൂല്യങ്ങൾ, &d->lheads);
        chs_മൂല്യങ്ങൾ = parse_u32(chs_മൂല്യങ്ങൾ, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } ആകുമ്പോൾ (f);
}

// Search the bios-geometry list for the given glob pattern.
സ്ഥിതം BootDeviceLCHS *
boot_lchs_find(സ്ഥിരം അക്ഷരം *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < Bios_Geometry_എണ്ണം; i++)
        എങ്കിൽ (glob_prefix(glob, BiosGeometry[i].name))
            തിരികെ &BiosGeometry[i];
    തിരികെ ശൂന്യ;
}

പൂർണ്ണസംഖ്യ boot_lchs_find_pci_device(ഘടന pci_device *pci, ഘടന chs_s *chs)
{
    എങ്കിൽ (!CONFIG_HOST_BIOS_GEOMETRY)
        തിരികെ -1;
    അക്ഷരം desc[256];
    build_pci_path(desc, വലുപ്പം(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    എങ്കിൽ (!b)
        തിരികെ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    തിരികെ 0;
}

പൂർണ്ണസംഖ്യ boot_lchs_find_scsi_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ target, പൂർണ്ണസംഖ്യ lun,
                               ഘടന chs_s *chs)
{
    എങ്കിൽ (!CONFIG_HOST_BIOS_GEOMETRY)
        തിരികെ -1;
    എങ്കിൽ (!pci)
        // support only pci machine for now
        തിരികെ -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    അക്ഷരം desc[256];
    build_scsi_path(desc, വലുപ്പം(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    എങ്കിൽ (!b)
        തിരികെ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    തിരികെ 0;
}

പൂർണ്ണസംഖ്യ boot_lchs_find_ata_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ chanid, പൂർണ്ണസംഖ്യ slave,
                              ഘടന chs_s *chs)
{
    എങ്കിൽ (!CONFIG_HOST_BIOS_GEOMETRY)
        തിരികെ -1;
    എങ്കിൽ (!pci)
        // support only pci machine for now
        തിരികെ -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    അക്ഷരം desc[256];
    build_ata_path(desc, വലുപ്പം(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    എങ്കിൽ (!b)
        തിരികെ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    തിരികെ 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

സ്ഥിതം അക്ഷരം **Bootorder VARVERIFY32INIT;
സ്ഥിതം പൂർണ്ണസംഖ്യ Bootorder_എണ്ണം;

സ്ഥിതം ശൂന്യം
loadBootOrder(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ;

    അക്ഷരം *f = romfile_loadfile("bootorder", ശൂന്യ);
    എങ്കിൽ (!f)
        തിരികെ;

    പൂർണ്ണസംഖ്യ i = 0;
    Bootorder_എണ്ണം = 1;
    ആകുമ്പോൾ (f[i]) {
        എങ്കിൽ (f[i] == '\n')
            Bootorder_എണ്ണം++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_എണ്ണം*വലുപ്പം(അക്ഷരം*));
    എങ്കിൽ (!Bootorder) {
        warn_noalloc();
        സ്വതന്ത്രം(f);
        Bootorder_എണ്ണം = 0;
        തിരികെ;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    ചെയ്യുക {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        എങ്കിൽ (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } ആകുമ്പോൾ (f);
}

// Search the bootorder list for the given glob pattern.
സ്ഥിതം പൂർണ്ണസംഖ്യ
find_prio(സ്ഥിരം അക്ഷരം *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < Bootorder_എണ്ണം; i++)
        എങ്കിൽ (glob_prefix(glob, Bootorder[i]))
            തിരികെ i+1;
    തിരികെ -1;
}

u8 is_bootprio_strict(ശൂന്യം)
{
    സ്ഥിതം പൂർണ്ണസംഖ്യ prio_halt = -2;

    എങ്കിൽ (prio_halt == -2)
        prio_halt = find_prio("HALT");
    തിരികെ prio_halt >= 0;
}

പൂർണ്ണസംഖ്യ bootprio_find_pci_device(ഘടന pci_device *pci)
{
    എങ്കിൽ (CONFIG_CSM)
        തിരികെ csm_bootprio_pci(pci);
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    അക്ഷരം desc[256];
    build_pci_path(desc, വലുപ്പം(desc), "*", pci);
    തിരികെ find_prio(desc);
}

പൂർണ്ണസംഖ്യ bootprio_find_mmio_device(ശൂന്യം *mmio)
{
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    അക്ഷരം desc[256];
    snprintf(desc, വലുപ്പം(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    തിരികെ find_prio(desc);
}

പൂർണ്ണസംഖ്യ bootprio_find_scsi_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ target, പൂർണ്ണസംഖ്യ lun)
{
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    എങ്കിൽ (!pci)
        // support only pci machine for now
        തിരികെ -1;
    അക്ഷരം desc[256];
    build_scsi_path(desc, വലുപ്പം(desc), pci, target, lun);
    തിരികെ find_prio(desc);
}

പൂർണ്ണസംഖ്യ bootprio_find_scsi_mmio_device(ശൂന്യം *mmio, പൂർണ്ണസംഖ്യ target, പൂർണ്ണസംഖ്യ lun)
{
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    അക്ഷരം desc[256];
    snprintf(desc, വലുപ്പം(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    തിരികെ find_prio(desc);
}

പൂർണ്ണസംഖ്യ bootprio_find_ata_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ chanid, പൂർണ്ണസംഖ്യ slave)
{
    എങ്കിൽ (CONFIG_CSM)
        തിരികെ csm_bootprio_ata(pci, chanid, slave);
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    എങ്കിൽ (!pci)
        // support only pci machine for now
        തിരികെ -1;
    അക്ഷരം desc[256];
    build_ata_path(desc, വലുപ്പം(desc), pci, chanid, slave);
    തിരികെ find_prio(desc);
}

പൂർണ്ണസംഖ്യ bootprio_find_fdc_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ port, പൂർണ്ണസംഖ്യ fdid)
{
    എങ്കിൽ (CONFIG_CSM)
        തിരികെ csm_bootprio_fdc(pci, port, fdid);
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    എങ്കിൽ (!pci)
        // support only pci machine for now
        തിരികെ -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    അക്ഷരം desc[256], *p;
    p = build_pci_path(desc, വലുപ്പം(desc), "isa", pci);
    snprintf(p, desc+വലുപ്പം(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    തിരികെ find_prio(desc);
}

പൂർണ്ണസംഖ്യ bootprio_find_pci_rom(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ instance)
{
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    അക്ഷരം desc[256], *p;
    p = build_pci_path(desc, വലുപ്പം(desc), "*", pci);
    എങ്കിൽ (instance)
        snprintf(p, desc+വലുപ്പം(desc)-p, ":rom%x", instance);
    തിരികെ find_prio(desc);
}

പൂർണ്ണസംഖ്യ bootprio_find_named_rom(സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ instance)
{
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    അക്ഷരം desc[256], *p;
    p = desc + snprintf(desc, വലുപ്പം(desc), "/rom@%s", name);
    എങ്കിൽ (instance)
        snprintf(p, desc+വലുപ്പം(desc)-p, ":rom%x", instance);
    തിരികെ find_prio(desc);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_portmap(ഘടന usbdevice_s *usbdev)
{
    എങ്കിൽ (usbdev->hub->op->portmap)
        തിരികെ usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    തിരികെ usbdev->port + 1;
}

സ്ഥിതം അക്ഷരം *
build_usb_path(അക്ഷരം *buf, പൂർണ്ണസംഖ്യ max, ഘടന usbhub_s *hub)
{
    എങ്കിൽ (!hub->usbdev)
        // Root hub - nothing to add.
        തിരികെ buf;
    അക്ഷരം *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    തിരികെ p;
}

പൂർണ്ണസംഖ്യ bootprio_find_usb(ഘടന usbdevice_s *usbdev, പൂർണ്ണസംഖ്യ lun)
{
    എങ്കിൽ (!CONFIG_BOOTORDER)
        തിരികെ -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    അക്ഷരം desc[256], *p;

    എങ്കിൽ (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, വലുപ്പം(desc), "usb", usbdev->hub->cntl->pci);
    അല്ലെങ്കിൽ എങ്കിൽ (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, വലുപ്പം(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    അല്ലെങ്കിൽ
        തിരികെ -1;

    p = build_usb_path(p, desc+വലുപ്പം(desc)-p, usbdev->hub);
    snprintf(p, desc+വലുപ്പം(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    പൂർണ്ണസംഖ്യ ret = find_prio(desc);
    എങ്കിൽ (ret >= 0)
        തിരികെ ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+വലുപ്പം(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    തിരികെ find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

സ്ഥിതം പൂർണ്ണസംഖ്യ BootRetryTime;
സ്ഥിതം പൂർണ്ണസംഖ്യ CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

സ്ഥിതം പൂർണ്ണസംഖ്യ DefaultFloppyPrio = 101;
സ്ഥിതം പൂർണ്ണസംഖ്യ DefaultCDPrio     = 102;
സ്ഥിതം പൂർണ്ണസംഖ്യ DefaultHDPrio     = 103;
സ്ഥിതം പൂർണ്ണസംഖ്യ DefaultBEVPrio    = 104;

ശൂന്യം
boot_init(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_BOOT)
        തിരികെ;

    എങ്കിൽ (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        എങ്കിൽ (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            തിരഞ്ഞെടുപ്പ് (val) {
            അവസ്ഥ 1: DefaultFloppyPrio = i; നിർത്തുക;
            അവസ്ഥ 2: DefaultHDPrio = i;     നിർത്തുക;
            അവസ്ഥ 3: DefaultCDPrio = i;     നിർത്തുക;
            അവസ്ഥ 4: DefaultBEVPrio = i;    നിർത്തുക;
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

ഘടന bootentry_s {
    പൂർണ്ണസംഖ്യ type;
    സംഘം {
        u32 data;
        ഘടന segoff_s vector;
        ഘടന drive_s *drive;
    };
    പൂർണ്ണസംഖ്യ priority;
    സ്ഥിരം അക്ഷരം *description;
    ഘടന hlist_കണ്ണി കണ്ണി;
};
സ്ഥിതം ഘടന hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

സ്ഥിതം ശൂന്യം
bootentry_add(പൂർണ്ണസംഖ്യ type, പൂർണ്ണസംഖ്യ prio, u32 data, സ്ഥിരം അക്ഷരം *desc)
{
    എങ്കിൽ (! CONFIG_BOOT)
        തിരികെ;
    ഘടന bootentry_s *be = malloc_tmp(വലുപ്പം(*be));
    എങ്കിൽ (!be) {
        warn_noalloc();
        തിരികെ;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    ഘടന hlist_കണ്ണി **pprev;
    ഘടന bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, കണ്ണി) {
        എങ്കിൽ (be->priority < pos->priority)
            നിർത്തുക;
        എങ്കിൽ (be->priority > pos->priority)
            തുടരുക;
        എങ്കിൽ (be->type < pos->type)
            നിർത്തുക;
        എങ്കിൽ (be->type > pos->type)
            തുടരുക;
        എങ്കിൽ (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            നിർത്തുക;
    }
    hlist_add(&be->കണ്ണി, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ defPrio(പൂർണ്ണസംഖ്യ priority, പൂർണ്ണസംഖ്യ defaultprio) {
    തിരികെ (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
ശൂന്യം
boot_add_bev(u16 seg, u16 bev, u16 desc, പൂർണ്ണസംഖ്യ prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
ശൂന്യം
boot_add_bcv(u16 seg, u16 ip, u16 desc, പൂർണ്ണസംഖ്യ prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

ശൂന്യം
boot_add_floppy(ഘടന drive_s *drive, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

ശൂന്യം
boot_add_hd(ഘടന drive_s *drive, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

ശൂന്യം
boot_add_cd(ഘടന drive_s *drive, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio)
{
    എങ്കിൽ (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        അക്ഷരം *extra = cdrom_media_info(drive);
        എങ്കിൽ (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            സ്വതന്ത്രം(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
ശൂന്യം
boot_add_cbfs(ശൂന്യം *data, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
സ്ഥിതം പൂർണ്ണസംഖ്യ
check_for_keystroke(ശൂന്യം)
{
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    തിരികെ !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
സ്ഥിതം പൂർണ്ണസംഖ്യ
get_raw_keystroke(ശൂന്യം)
{
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    തിരികെ br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
പൂർണ്ണസംഖ്യ
get_keystroke_full(പൂർണ്ണസംഖ്യ msec)
{
    u32 end = irqtimer_calc(msec);
    വേണ്ടി (;;) {
        എങ്കിൽ (check_for_keystroke())
            തിരികെ get_raw_keystroke();
        എങ്കിൽ (irqtimer_check(end))
            തിരികെ -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
പൂർണ്ണസംഖ്യ
get_keystroke(പൂർണ്ണസംഖ്യ msec)
{
    പൂർണ്ണസംഖ്യ keystroke = get_keystroke_full(msec);

    എങ്കിൽ (keystroke < 0)
        തിരികെ keystroke;
    തിരികെ keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

സ്ഥിതം സ്ഥിരം അക്ഷരം menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
ശൂന്യം
interactive_bootmenu(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_BOOTMENU)
        തിരികെ;
    പൂർണ്ണസംഖ്യ show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    എങ്കിൽ (!show_boot_menu)
        തിരികെ;

    // skip menu if only one boot device and no TPM
    എങ്കിൽ (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        തിരികെ;
    }

    പൂർണ്ണസംഖ്യ menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    പൂർണ്ണസംഖ്യ menukey = romfile_loadint("etc/boot-menu-key", 1);
    പൂർണ്ണസംഖ്യ scan_code;
    എങ്കിൽ (menutime >= 0) {
        ആകുമ്പോൾ (get_keystroke(0) >= 0)
            ;

        അക്ഷരം *bootmsg = romfile_loadfile("etc/boot-menu-message", ശൂന്യ);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        സ്വതന്ത്രം(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        എങ്കിൽ (scan_code != menukey)
            തിരികെ;
    }

    ആകുമ്പോൾ (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    പൂർണ്ണസംഖ്യ maxmenu = 0;
    ഘടന bootentry_s *pos, *boot = ശൂന്യ;
    hlist_for_each_entry(pos, &BootList, കണ്ണി) {
        അക്ഷരം desc[77];
        എങ്കിൽ (maxmenu >= ARRAY_SIZE(menuchars)) {
            നിർത്തുക;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    എങ്കിൽ (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    പൂർണ്ണസംഖ്യ esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    വേണ്ടി (;;) {
        പൂർണ്ണസംഖ്യ keystroke = get_keystroke_full(1000);
        എങ്കിൽ (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            തുടരുക;
        എങ്കിൽ (keystroke < 0) // timeout
            തുടരുക;

        scan_code = keystroke >> 8;
        പൂർണ്ണസംഖ്യ താക്കോൽ_ascii = keystroke & 0xff;
        എങ്കിൽ (tpm_can_show_menu() && താക്കോൽ_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        എങ്കിൽ (scan_code == 1) {
            // ESC
            printf("\n");
            തിരികെ;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, കണ്ണി) {
            എങ്കിൽ (maxmenu >= ARRAY_SIZE(menuchars))
                നിർത്തുക;
            എങ്കിൽ (താക്കോൽ_ascii == menuchars[maxmenu]) {
                boot = pos;
                നിർത്തുക;
            }
            maxmenu++;
        }
        എങ്കിൽ (boot)
            നിർത്തുക;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->കണ്ണി);
    boot->priority = 0;
    hlist_add_head(&boot->കണ്ണി, &BootList);
}

// BEV (Boot Execution Vector) list
ഘടന bev_s {
    പൂർണ്ണസംഖ്യ type;
    u32 vector;
};
സ്ഥിതം ഘടന bev_s BEV[20];
സ്ഥിതം പൂർണ്ണസംഖ്യ BEVCount;
സ്ഥിതം പൂർണ്ണസംഖ്യ HaveHDBoot, HaveFDBoot;

സ്ഥിതം ശൂന്യം
add_bev(പൂർണ്ണസംഖ്യ type, u32 vector)
{
    എങ്കിൽ (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        തിരികെ;
    എങ്കിൽ (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        തിരികെ;
    എങ്കിൽ (BEVCount >= ARRAY_SIZE(BEV))
        തിരികെ;
    ഘടന bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
ശൂന്യം
bcv_prepboot(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_BOOT)
        തിരികെ;

    പൂർണ്ണസംഖ്യ haltprio = find_prio("HALT");
    എങ്കിൽ (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    ഘടന bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, കണ്ണി) {
        തിരഞ്ഞെടുപ്പ് (pos->type) {
        അവസ്ഥ IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            നിർത്തുക;
        അവസ്ഥ IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            നിർത്തുക;
        അവസ്ഥ IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            നിർത്തുക;
        അവസ്ഥ IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        സ്ഥിരസ്ഥിതി:
            add_bev(pos->type, pos->data);
            നിർത്തുക;
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
സ്ഥിതം ശൂന്യം
call_boot_entry(ഘടന segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
സ്ഥിതം ശൂന്യം
boot_disk(u8 bootdrv, പൂർണ്ണസംഖ്യ checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    എങ്കിൽ (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        തിരികെ;
    }

    എങ്കിൽ (checksig) {
        ഘടന mbr_s *mbr = (ശൂന്യം*)0;
        എങ്കിൽ (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            തിരികെ;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
സ്ഥിതം ശൂന്യം
boot_cdrom(ഘടന drive_s *drive)
{
    എങ്കിൽ (! CONFIG_CDROM_BOOT)
        തിരികെ;
    printf("Booting from DVD/CD...\n");

    പൂർണ്ണസംഖ്യ status = cdrom_boot(drive);
    എങ്കിൽ (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        തിരികെ;
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
സ്ഥിതം ശൂന്യം
boot_cbfs(ഘടന cbfs_file *file)
{
    എങ്കിൽ (!CONFIG_COREBOOT_FLASH)
        തിരികെ;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
സ്ഥിതം ശൂന്യം
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    ഘടന segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
സ്ഥിതം ശൂന്യം
boot_fail(ശൂന്യം)
{
    എങ്കിൽ (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    അല്ലെങ്കിൽ
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    വേണ്ടി (;;) {
        എങ്കിൽ (BootRetryTime != (u32)-1 && irqtimer_check(end))
            നിർത്തുക;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
സ്ഥിതം ശൂന്യം
do_boot(പൂർണ്ണസംഖ്യ seq_nr)
{
    എങ്കിൽ (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    എങ്കിൽ (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    ഘടന bev_s *ie = &BEV[seq_nr];
    തിരഞ്ഞെടുപ്പ് (ie->type) {
    അവസ്ഥ IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        നിർത്തുക;
    അവസ്ഥ IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        നിർത്തുക;
    അവസ്ഥ IPL_TYPE_CDROM:
        boot_cdrom((ശൂന്യം*)ie->vector);
        നിർത്തുക;
    അവസ്ഥ IPL_TYPE_CBFS:
        boot_cbfs((ശൂന്യം*)ie->vector);
        നിർത്തുക;
    അവസ്ഥ IPL_TYPE_BEV:
        boot_rom(ie->vector);
        നിർത്തുക;
    അവസ്ഥ IPL_TYPE_HALT:
        boot_fail();
        നിർത്തുക;
    }

    // Boot failed: invoke the boot recovery function
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

പൂർണ്ണസംഖ്യ BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
ശൂന്യം VISIBLE32FLAT
handle_18(ശൂന്യം)
{
    debug_enter(ശൂന്യ, DEBUG_HDL_18);
    പൂർണ്ണസംഖ്യ seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
ശൂന്യം VISIBLE32FLAT
handle_19(ശൂന്യം)
{
    debug_enter(ശൂന്യ, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
