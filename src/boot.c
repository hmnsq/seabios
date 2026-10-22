// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "fw/paravirt.h" // qemu_cfg_show_boot_menu
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_bdf_to_*
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // struct pci_device
#ଅନ୍ତର୍ଭାଵ "hw/rtc.h" // rtc_read
#ଅନ୍ତର୍ଭାଵ "hw/usb.h" // struct usbdevice_s
#ଅନ୍ତର୍ଭାଵ "list.h" // hlist_node
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // struct mbr_s
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // irqtimer_calc
#ଅନ୍ତର୍ଭାଵ "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
ସ୍ଥିର ଅକ୍ଷର *
glob_prefix(ଅଚଳ ଅକ୍ଷର *glob, ଅଚଳ ଅକ୍ଷର *str)
{
    ପାଇଁ (;;) {
        ଯଦି (!*glob && (!*str || *str == '/'))
            ଫେରନ୍ତୁ (ଅକ୍ଷର*)str;
        ଯଦି (*glob == '*') {
            ଯଦି (!*str || *str == '/' || *str == glob[1])
                glob++;
            ଅନ୍ୟଥା
                str++;
            ଜାରି;
        }
        ଯଦି (*glob != *str)
            ଫେରନ୍ତୁ ଖାଲି;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

ସ୍ଥିର ଅକ୍ଷର *
build_pci_path(ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max, ଅଚଳ ଅକ୍ଷର *devname, ସଂରଚନା pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    ଅକ୍ଷର *p = buf;
    ଯଦି (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } ଅନ୍ୟଥା {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        ଯଦି (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    ଯଦି (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    ଫେରନ୍ତୁ p;
}

ସ୍ଥିର ଅକ୍ଷର *
build_scsi_path(ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max,
                ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ଅକ୍ଷର *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    ଫେରନ୍ତୁ p;
}

ସ୍ଥିର ଅକ୍ଷର *
build_ata_path(ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max,
               ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chanid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    ଅକ୍ଷର *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    ଫେରନ୍ତୁ p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା BootDeviceLCHS {
    ଅକ୍ଷର *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

ସ୍ଥିର BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା Bios_Geometry_ଗଣନା;

ସ୍ଥିର ଅକ୍ଷର *
parse_u32(ଅକ୍ଷର *cur, u32 *n)
{
    u32 m = 0;
    ଯଦି (cur) {
        ଯେତେବେଳେ ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        ଯଦି (*cur != '\0')
            cur++;
    }
    *n = m;
    ଫେରନ୍ତୁ cur;
}

ସ୍ଥିର ଶୂନ୍ୟ
loadBiosGeometry(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_HOST_BIOS_GEOMETRY)
        ଫେରନ୍ତୁ;
    ଅକ୍ଷର *f = romfile_loadfile("bios-geometry", ଖାଲି);
    ଯଦି (!f)
        ଫେରନ୍ତୁ;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
    Bios_Geometry_ଗଣନା = 1;
    ଯେତେବେଳେ (f[i]) {
        ଯଦି (f[i] == '\n')
            Bios_Geometry_ଗଣନା++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(Bios_Geometry_ଗଣନା * ଆକାର(BootDeviceLCHS));
    ଯଦି (!BiosGeometry) {
        warn_noalloc();
        ମୁକ୍ତ(f);
        Bios_Geometry_ଗଣନା = 0;
        ଫେରନ୍ତୁ;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    କର {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        ଯଦି (f)
            *(f++) = '\0';
        ଅକ୍ଷର *chs_ମୂଲ୍ୟଗୁଡ଼ିକ = strchr(d->name, ' ');
        ଯଦି (chs_ମୂଲ୍ୟଗୁଡ଼ିକ)
            *(chs_ମୂଲ୍ୟଗୁଡ଼ିକ++) = '\0';
        chs_ମୂଲ୍ୟଗୁଡ଼ିକ = parse_u32(chs_ମୂଲ୍ୟଗୁଡ଼ିକ, &d->lcyls);
        chs_ମୂଲ୍ୟଗୁଡ଼ିକ = parse_u32(chs_ମୂଲ୍ୟଗୁଡ଼ିକ, &d->lheads);
        chs_ମୂଲ୍ୟଗୁଡ଼ିକ = parse_u32(chs_ମୂଲ୍ୟଗୁଡ଼ିକ, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } ଯେତେବେଳେ (f);
}

// Search the bios-geometry list for the given glob pattern.
ସ୍ଥିର BootDeviceLCHS *
boot_lchs_find(ଅଚଳ ଅକ୍ଷର *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < Bios_Geometry_ଗଣନା; i++)
        ଯଦି (glob_prefix(glob, BiosGeometry[i].name))
            ଫେରନ୍ତୁ &BiosGeometry[i];
    ଫେରନ୍ତୁ ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା boot_lchs_find_pci_device(ସଂରଚନା pci_device *pci, ସଂରଚନା chs_s *chs)
{
    ଯଦି (!CONFIG_HOST_BIOS_GEOMETRY)
        ଫେରନ୍ତୁ -1;
    ଅକ୍ଷର desc[256];
    build_pci_path(desc, ଆକାର(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ଯଦି (!b)
        ଫେରନ୍ତୁ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା boot_lchs_find_scsi_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun,
                               ସଂରଚନା chs_s *chs)
{
    ଯଦି (!CONFIG_HOST_BIOS_GEOMETRY)
        ଫେରନ୍ତୁ -1;
    ଯଦି (!pci)
        // support only pci machine for now
        ଫେରନ୍ତୁ -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    ଅକ୍ଷର desc[256];
    build_scsi_path(desc, ଆକାର(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ଯଦି (!b)
        ଫେରନ୍ତୁ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା boot_lchs_find_ata_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chanid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave,
                              ସଂରଚନା chs_s *chs)
{
    ଯଦି (!CONFIG_HOST_BIOS_GEOMETRY)
        ଫେରନ୍ତୁ -1;
    ଯଦି (!pci)
        // support only pci machine for now
        ଫେରନ୍ତୁ -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    ଅକ୍ଷର desc[256];
    build_ata_path(desc, ଆକାର(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    ଯଦି (!b)
        ଫେରନ୍ତୁ -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    ଫେରନ୍ତୁ 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

ସ୍ଥିର ଅକ୍ଷର **Bootorder VARVERIFY32INIT;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା Bootorder_ଗଣନା;

ସ୍ଥିର ଶୂନ୍ୟ
loadBootOrder(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ;

    ଅକ୍ଷର *f = romfile_loadfile("bootorder", ଖାଲି);
    ଯଦି (!f)
        ଫେରନ୍ତୁ;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
    Bootorder_ଗଣନା = 1;
    ଯେତେବେଳେ (f[i]) {
        ଯଦି (f[i] == '\n')
            Bootorder_ଗଣନା++;
        i++;
    }
    Bootorder = malloc_tmphigh(Bootorder_ଗଣନା*ଆକାର(ଅକ୍ଷର*));
    ଯଦି (!Bootorder) {
        warn_noalloc();
        ମୁକ୍ତ(f);
        Bootorder_ଗଣନା = 0;
        ଫେରନ୍ତୁ;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    କର {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        ଯଦି (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } ଯେତେବେଳେ (f);
}

// Search the bootorder list for the given glob pattern.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
find_prio(ଅଚଳ ଅକ୍ଷର *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < Bootorder_ଗଣନା; i++)
        ଯଦି (glob_prefix(glob, Bootorder[i]))
            ଫେରନ୍ତୁ i+1;
    ଫେରନ୍ତୁ -1;
}

u8 is_bootprio_strict(ଶୂନ୍ୟ)
{
    ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio_halt = -2;

    ଯଦି (prio_halt == -2)
        prio_halt = find_prio("HALT");
    ଫେରନ୍ତୁ prio_halt >= 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_pci_device(ସଂରଚନା pci_device *pci)
{
    ଯଦି (CONFIG_CSM)
        ଫେରନ୍ତୁ csm_bootprio_pci(pci);
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    ଅକ୍ଷର desc[256];
    build_pci_path(desc, ଆକାର(desc), "*", pci);
    ଫେରନ୍ତୁ find_prio(desc);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_mmio_device(ଶୂନ୍ୟ *mmio)
{
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    ଅକ୍ଷର desc[256];
    snprintf(desc, ଆକାର(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    ଫେରନ୍ତୁ find_prio(desc);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_scsi_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun)
{
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    ଯଦି (!pci)
        // support only pci machine for now
        ଫେରନ୍ତୁ -1;
    ଅକ୍ଷର desc[256];
    build_scsi_path(desc, ଆକାର(desc), pci, target, lun);
    ଫେରନ୍ତୁ find_prio(desc);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_scsi_mmio_device(ଶୂନ୍ୟ *mmio, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun)
{
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    ଅକ୍ଷର desc[256];
    snprintf(desc, ଆକାର(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    ଫେରନ୍ତୁ find_prio(desc);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_ata_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chanid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave)
{
    ଯଦି (CONFIG_CSM)
        ଫେରନ୍ତୁ csm_bootprio_ata(pci, chanid, slave);
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    ଯଦି (!pci)
        // support only pci machine for now
        ଫେରନ୍ତୁ -1;
    ଅକ୍ଷର desc[256];
    build_ata_path(desc, ଆକାର(desc), pci, chanid, slave);
    ଫେରନ୍ତୁ find_prio(desc);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_fdc_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fdid)
{
    ଯଦି (CONFIG_CSM)
        ଫେରନ୍ତୁ csm_bootprio_fdc(pci, port, fdid);
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    ଯଦି (!pci)
        // support only pci machine for now
        ଫେରନ୍ତୁ -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    ଅକ୍ଷର desc[256], *p;
    p = build_pci_path(desc, ଆକାର(desc), "isa", pci);
    snprintf(p, desc+ଆକାର(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    ଫେରନ୍ତୁ find_prio(desc);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_pci_rom(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance)
{
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    ଅକ୍ଷର desc[256], *p;
    p = build_pci_path(desc, ଆକାର(desc), "*", pci);
    ଯଦି (instance)
        snprintf(p, desc+ଆକାର(desc)-p, ":rom%x", instance);
    ଫେରନ୍ତୁ find_prio(desc);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_named_rom(ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance)
{
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    ଅକ୍ଷର desc[256], *p;
    p = desc + snprintf(desc, ଆକାର(desc), "/rom@%s", name);
    ଯଦି (instance)
        snprintf(p, desc+ଆକାର(desc)-p, ":rom%x", instance);
    ଫେରନ୍ତୁ find_prio(desc);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_portmap(ସଂରଚନା usbdevice_s *usbdev)
{
    ଯଦି (usbdev->hub->op->portmap)
        ଫେରନ୍ତୁ usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    ଫେରନ୍ତୁ usbdev->port + 1;
}

ସ୍ଥିର ଅକ୍ଷର *
build_usb_path(ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max, ସଂରଚନା usbhub_s *hub)
{
    ଯଦି (!hub->usbdev)
        // Root hub - nothing to add.
        ଫେରନ୍ତୁ buf;
    ଅକ୍ଷର *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    ଫେରନ୍ତୁ p;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_usb(ସଂରଚନା usbdevice_s *usbdev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun)
{
    ଯଦି (!CONFIG_BOOTORDER)
        ଫେରନ୍ତୁ -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    ଅକ୍ଷର desc[256], *p;

    ଯଦି (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, ଆକାର(desc), "usb", usbdev->hub->cntl->pci);
    ଅନ୍ୟଥା ଯଦି (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, ଆକାର(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    ଅନ୍ୟଥା
        ଫେରନ୍ତୁ -1;

    p = build_usb_path(p, desc+ଆକାର(desc)-p, usbdev->hub);
    snprintf(p, desc+ଆକାର(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = find_prio(desc);
    ଯଦି (ret >= 0)
        ଫେରନ୍ତୁ ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+ଆକାର(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    ଫେରନ୍ତୁ find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା BootRetryTime;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DefaultFloppyPrio = 101;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DefaultCDPrio     = 102;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DefaultHDPrio     = 103;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DefaultBEVPrio    = 104;

ଶୂନ୍ୟ
boot_init(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_BOOT)
        ଫେରନ୍ତୁ;

    ଯଦି (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        ଯଦି (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            ବାଛ (val) {
            ସ୍ଥିତି 1: DefaultFloppyPrio = i; ଭାଙ୍ଗ;
            ସ୍ଥିତି 2: DefaultHDPrio = i;     ଭାଙ୍ଗ;
            ସ୍ଥିତି 3: DefaultCDPrio = i;     ଭାଙ୍ଗ;
            ସ୍ଥିତି 4: DefaultBEVPrio = i;    ଭାଙ୍ଗ;
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

ସଂରଚନା bootentry_s {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା type;
    ସଂଘ {
        u32 data;
        ସଂରଚନା segoff_s vector;
        ସଂରଚନା drive_s *drive;
    };
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା priority;
    ଅଚଳ ଅକ୍ଷର *description;
    ସଂରଚନା hlist_ଗଣ୍ଠି ଗଣ୍ଠି;
};
ସ୍ଥିର ସଂରଚନା hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

ସ୍ଥିର ଶୂନ୍ୟ
bootentry_add(ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio, u32 data, ଅଚଳ ଅକ୍ଷର *desc)
{
    ଯଦି (! CONFIG_BOOT)
        ଫେରନ୍ତୁ;
    ସଂରଚନା bootentry_s *be = malloc_tmp(ଆକାର(*be));
    ଯଦି (!be) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    ସଂରଚନା hlist_ଗଣ୍ଠି **pprev;
    ସଂରଚନା bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, ଗଣ୍ଠି) {
        ଯଦି (be->priority < pos->priority)
            ଭାଙ୍ଗ;
        ଯଦି (be->priority > pos->priority)
            ଜାରି;
        ଯଦି (be->type < pos->type)
            ଭାଙ୍ଗ;
        ଯଦି (be->type > pos->type)
            ଜାରି;
        ଯଦି (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            ଭାଙ୍ଗ;
    }
    hlist_add(&be->ଗଣ୍ଠି, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା defPrio(ପୂର୍ଣ୍ଣସଂଖ୍ୟା priority, ପୂର୍ଣ୍ଣସଂଖ୍ୟା defaultprio) {
    ଫେରନ୍ତୁ (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
ଶୂନ୍ୟ
boot_add_bev(u16 seg, u16 bev, u16 desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
ଶୂନ୍ୟ
boot_add_bcv(u16 seg, u16 ip, u16 desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

ଶୂନ୍ୟ
boot_add_floppy(ସଂରଚନା drive_s *drive, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

ଶୂନ୍ୟ
boot_add_hd(ସଂରଚନା drive_s *drive, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

ଶୂନ୍ୟ
boot_add_cd(ସଂରଚନା drive_s *drive, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
    ଯଦି (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        ଅକ୍ଷର *extra = cdrom_media_info(drive);
        ଯଦି (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            ମୁକ୍ତ(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
ଶୂନ୍ୟ
boot_add_cbfs(ଶୂନ୍ୟ *data, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
check_for_keystroke(ଶୂନ୍ୟ)
{
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    ଫେରନ୍ତୁ !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_raw_keystroke(ଶୂନ୍ୟ)
{
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    ଫେରନ୍ତୁ br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_keystroke_full(ପୂର୍ଣ୍ଣସଂଖ୍ୟା msec)
{
    u32 end = irqtimer_calc(msec);
    ପାଇଁ (;;) {
        ଯଦି (check_for_keystroke())
            ଫେରନ୍ତୁ get_raw_keystroke();
        ଯଦି (irqtimer_check(end))
            ଫେରନ୍ତୁ -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_keystroke(ପୂର୍ଣ୍ଣସଂଖ୍ୟା msec)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା keystroke = get_keystroke_full(msec);

    ଯଦି (keystroke < 0)
        ଫେରନ୍ତୁ keystroke;
    ଫେରନ୍ତୁ keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
ଶୂନ୍ୟ
interactive_bootmenu(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_BOOTMENU)
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    ଯଦି (!show_boot_menu)
        ଫେରନ୍ତୁ;

    // skip menu if only one boot device and no TPM
    ଯଦି (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        ଫେରନ୍ତୁ;
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା menukey = romfile_loadint("etc/boot-menu-key", 1);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା scan_code;
    ଯଦି (menutime >= 0) {
        ଯେତେବେଳେ (get_keystroke(0) >= 0)
            ;

        ଅକ୍ଷର *bootmsg = romfile_loadfile("etc/boot-menu-message", ଖାଲି);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        ମୁକ୍ତ(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        ଯଦି (scan_code != menukey)
            ଫେରନ୍ତୁ;
    }

    ଯେତେବେଳେ (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxmenu = 0;
    ସଂରଚନା bootentry_s *pos, *boot = ଖାଲି;
    hlist_for_each_entry(pos, &BootList, ଗଣ୍ଠି) {
        ଅକ୍ଷର desc[77];
        ଯଦି (maxmenu >= ARRAY_SIZE(menuchars)) {
            ଭାଙ୍ଗ;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    ଯଦି (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    ପାଇଁ (;;) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା keystroke = get_keystroke_full(1000);
        ଯଦି (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            ଜାରି;
        ଯଦି (keystroke < 0) // timeout
            ଜାରି;

        scan_code = keystroke >> 8;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି_ascii = keystroke & 0xff;
        ଯଦି (tpm_can_show_menu() && ଚାବି_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        ଯଦି (scan_code == 1) {
            // ESC
            printf("\n");
            ଫେରନ୍ତୁ;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, ଗଣ୍ଠି) {
            ଯଦି (maxmenu >= ARRAY_SIZE(menuchars))
                ଭାଙ୍ଗ;
            ଯଦି (ଚାବି_ascii == menuchars[maxmenu]) {
                boot = pos;
                ଭାଙ୍ଗ;
            }
            maxmenu++;
        }
        ଯଦି (boot)
            ଭାଙ୍ଗ;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->ଗଣ୍ଠି);
    boot->priority = 0;
    hlist_add_head(&boot->ଗଣ୍ଠି, &BootList);
}

// BEV (Boot Execution Vector) list
ସଂରଚନା bev_s {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା type;
    u32 vector;
};
ସ୍ଥିର ସଂରଚନା bev_s BEV[20];
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା BEVCount;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା HaveHDBoot, HaveFDBoot;

ସ୍ଥିର ଶୂନ୍ୟ
add_bev(ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, u32 vector)
{
    ଯଦି (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        ଫେରନ୍ତୁ;
    ଯଦି (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        ଫେରନ୍ତୁ;
    ଯଦି (BEVCount >= ARRAY_SIZE(BEV))
        ଫେରନ୍ତୁ;
    ସଂରଚନା bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
ଶୂନ୍ୟ
bcv_prepboot(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_BOOT)
        ଫେରନ୍ତୁ;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା haltprio = find_prio("HALT");
    ଯଦି (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    ସଂରଚନା bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, ଗଣ୍ଠି) {
        ବାଛ (pos->type) {
        ସ୍ଥିତି IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            add_bev(pos->type, pos->data);
            ଭାଙ୍ଗ;
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
ସ୍ଥିର ଶୂନ୍ୟ
call_boot_entry(ସଂରଚନା segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
ସ୍ଥିର ଶୂନ୍ୟ
boot_disk(u8 bootdrv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    ଯଦି (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        ଫେରନ୍ତୁ;
    }

    ଯଦି (checksig) {
        ସଂରଚନା mbr_s *mbr = (ଶୂନ୍ୟ*)0;
        ଯଦି (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            ଫେରନ୍ତୁ;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
ସ୍ଥିର ଶୂନ୍ୟ
boot_cdrom(ସଂରଚନା drive_s *drive)
{
    ଯଦି (! CONFIG_CDROM_BOOT)
        ଫେରନ୍ତୁ;
    printf("Booting from DVD/CD...\n");

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = cdrom_boot(drive);
    ଯଦି (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        ଫେରନ୍ତୁ;
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
ସ୍ଥିର ଶୂନ୍ୟ
boot_cbfs(ସଂରଚନା cbfs_file *file)
{
    ଯଦି (!CONFIG_COREBOOT_FLASH)
        ଫେରନ୍ତୁ;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
ସ୍ଥିର ଶୂନ୍ୟ
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    ସଂରଚନା segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
ସ୍ଥିର ଶୂନ୍ୟ
boot_fail(ଶୂନ୍ୟ)
{
    ଯଦି (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    ଅନ୍ୟଥା
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    ପାଇଁ (;;) {
        ଯଦି (BootRetryTime != (u32)-1 && irqtimer_check(end))
            ଭାଙ୍ଗ;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
ସ୍ଥିର ଶୂନ୍ୟ
do_boot(ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq_nr)
{
    ଯଦି (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    ଯଦି (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    ସଂରଚନା bev_s *ie = &BEV[seq_nr];
    ବାଛ (ie->type) {
    ସ୍ଥିତି IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି IPL_TYPE_CDROM:
        boot_cdrom((ଶୂନ୍ୟ*)ie->vector);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି IPL_TYPE_CBFS:
        boot_cbfs((ଶୂନ୍ୟ*)ie->vector);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି IPL_TYPE_BEV:
        boot_rom(ie->vector);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି IPL_TYPE_HALT:
        boot_fail();
        ଭାଙ୍ଗ;
    }

    // Boot failed: invoke the boot recovery function
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
ଶୂନ୍ୟ VISIBLE32FLAT
handle_18(ଶୂନ୍ୟ)
{
    debug_enter(ଖାଲି, DEBUG_HDL_18);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
ଶୂନ୍ୟ VISIBLE32FLAT
handle_19(ଶୂନ୍ୟ)
{
    debug_enter(ଖାଲି, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
