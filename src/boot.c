// Code to load disk image and start system boot.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "block.h" // struct drive_s
#include "bregs.h" // struct bregs
#include "config.h" // CONFIG_*
#include "fw/paravirt.h" // qemu_cfg_show_boot_menu
#include "hw/pci.h" // pci_bdf_to_*
#include "hw/pcidevice.h" // struct pci_device
#include "hw/rtc.h" // rtc_read
#include "hw/usb.h" // struct usbdevice_s
#include "list.h" // hlist_node
#include "malloc.h" // free
#include "output.h" // dprintf
#include "romfile.h" // romfile_loadint
#include "std/disk.h" // struct mbr_s
#include "string.h" // memset
#include "util.h" // irqtimer_calc
#include "tcgbios.h" // tpm_*

/****************************************************************
 * Helper search functions
 ****************************************************************/

// See if 'str' starts with 'glob' - if glob contains an '*' character
// it will match any number of characters in str that aren't a '/' or
// the next glob character.
static char *
glob_prefix(const char *glob, const char *str)
{
    для (;;) {
        если (!*glob && (!*str || *str == '/'))
            вернуть (char*)str;
        если (*glob == '*') {
            если (!*str || *str == '/' || *str == glob[1])
                glob++;
            иначе
                str++;
            continue;
        }
        если (*glob != *str)
            вернуть NULL;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

static char *
build_pci_path(char *buf, целое max, const char *devname, struct pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    char *p = buf;
    если (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } иначе {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        если (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    целое dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    если (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    вернуть p;
}

static char *
build_scsi_path(char *buf, целое max,
                struct pci_device *pci, целое target, целое lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    char *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    вернуть p;
}

static char *
build_ata_path(char *buf, целое max,
               struct pci_device *pci, целое chanid, целое slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    char *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    вернуть p;
}


/****************************************************************
 * Boot device logical geometry
 ****************************************************************/

typedef struct BootDeviceLCHS {
    char *name;
    u32 lcyls;
    u32 lheads;
    u32 lsecs;
} BootDeviceLCHS;

static BootDeviceLCHS *BiosGeometry VARVERIFY32INIT;
static целое BiosGeometryCount;

static char *
parse_u32(char *cur, u32 *n)
{
    u32 m = 0;
    если (cur) {
        пока ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        если (*cur != '\0')
            cur++;
    }
    *n = m;
    вернуть cur;
}

static void
loadBiosGeometry(void)
{
    если (!CONFIG_HOST_BIOS_GEOMETRY)
        вернуть;
    char *f = romfile_loadfile("bios-geometry", NULL);
    если (!f)
        вернуть;

    целое i = 0;
    BiosGeometryCount = 1;
    пока (f[i]) {
        если (f[i] == '\n')
            BiosGeometryCount++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(BiosGeometryCount * sizeof(BootDeviceLCHS));
    если (!BiosGeometry) {
        warn_noalloc();
        free(f);
        BiosGeometryCount = 0;
        вернуть;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    do {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        если (f)
            *(f++) = '\0';
        char *chs_values = strchr(d->name, ' ');
        если (chs_values)
            *(chs_values++) = '\0';
        chs_values = parse_u32(chs_values, &d->lcyls);
        chs_values = parse_u32(chs_values, &d->lheads);
        chs_values = parse_u32(chs_values, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } пока (f);
}

// Search the bios-geometry list for the given glob pattern.
static BootDeviceLCHS *
boot_lchs_find(const char *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    целое i;
    для (i = 0; i < BiosGeometryCount; i++)
        если (glob_prefix(glob, BiosGeometry[i].name))
            вернуть &BiosGeometry[i];
    вернуть NULL;
}

целое boot_lchs_find_pci_device(struct pci_device *pci, struct chs_s *chs)
{
    если (!CONFIG_HOST_BIOS_GEOMETRY)
        вернуть -1;
    char desc[256];
    build_pci_path(desc, sizeof(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    если (!b)
        вернуть -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    вернуть 0;
}

целое boot_lchs_find_scsi_device(struct pci_device *pci, целое target, целое lun,
                               struct chs_s *chs)
{
    если (!CONFIG_HOST_BIOS_GEOMETRY)
        вернуть -1;
    если (!pci)
        // support only pci machine for now
        вернуть -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    char desc[256];
    build_scsi_path(desc, sizeof(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    если (!b)
        вернуть -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    вернуть 0;
}

целое boot_lchs_find_ata_device(struct pci_device *pci, целое chanid, целое slave,
                              struct chs_s *chs)
{
    если (!CONFIG_HOST_BIOS_GEOMETRY)
        вернуть -1;
    если (!pci)
        // support only pci machine for now
        вернуть -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    char desc[256];
    build_ata_path(desc, sizeof(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    если (!b)
        вернуть -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    вернуть 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

static char **Bootorder VARVERIFY32INIT;
static целое BootorderCount;

static void
loadBootOrder(void)
{
    если (!CONFIG_BOOTORDER)
        вернуть;

    char *f = romfile_loadfile("bootorder", NULL);
    если (!f)
        вернуть;

    целое i = 0;
    BootorderCount = 1;
    пока (f[i]) {
        если (f[i] == '\n')
            BootorderCount++;
        i++;
    }
    Bootorder = malloc_tmphigh(BootorderCount*sizeof(char*));
    если (!Bootorder) {
        warn_noalloc();
        free(f);
        BootorderCount = 0;
        вернуть;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    do {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        если (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } пока (f);
}

// Search the bootorder list for the given glob pattern.
static целое
find_prio(const char *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    целое i;
    для (i = 0; i < BootorderCount; i++)
        если (glob_prefix(glob, Bootorder[i]))
            вернуть i+1;
    вернуть -1;
}

u8 is_bootprio_strict(void)
{
    static целое prio_halt = -2;

    если (prio_halt == -2)
        prio_halt = find_prio("HALT");
    вернуть prio_halt >= 0;
}

целое bootprio_find_pci_device(struct pci_device *pci)
{
    если (CONFIG_CSM)
        вернуть csm_bootprio_pci(pci);
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    char desc[256];
    build_pci_path(desc, sizeof(desc), "*", pci);
    вернуть find_prio(desc);
}

целое bootprio_find_mmio_device(void *mmio)
{
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    char desc[256];
    snprintf(desc, sizeof(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    вернуть find_prio(desc);
}

целое bootprio_find_scsi_device(struct pci_device *pci, целое target, целое lun)
{
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    если (!pci)
        // support only pci machine for now
        вернуть -1;
    char desc[256];
    build_scsi_path(desc, sizeof(desc), pci, target, lun);
    вернуть find_prio(desc);
}

целое bootprio_find_scsi_mmio_device(void *mmio, целое target, целое lun)
{
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    char desc[256];
    snprintf(desc, sizeof(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    вернуть find_prio(desc);
}

целое bootprio_find_ata_device(struct pci_device *pci, целое chanid, целое slave)
{
    если (CONFIG_CSM)
        вернуть csm_bootprio_ata(pci, chanid, slave);
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    если (!pci)
        // support only pci machine for now
        вернуть -1;
    char desc[256];
    build_ata_path(desc, sizeof(desc), pci, chanid, slave);
    вернуть find_prio(desc);
}

целое bootprio_find_fdc_device(struct pci_device *pci, целое port, целое fdid)
{
    если (CONFIG_CSM)
        вернуть csm_bootprio_fdc(pci, port, fdid);
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    если (!pci)
        // support only pci machine for now
        вернуть -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    char desc[256], *p;
    p = build_pci_path(desc, sizeof(desc), "isa", pci);
    snprintf(p, desc+sizeof(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    вернуть find_prio(desc);
}

целое bootprio_find_pci_rom(struct pci_device *pci, целое instance)
{
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    char desc[256], *p;
    p = build_pci_path(desc, sizeof(desc), "*", pci);
    если (instance)
        snprintf(p, desc+sizeof(desc)-p, ":rom%x", instance);
    вернуть find_prio(desc);
}

целое bootprio_find_named_rom(const char *name, целое instance)
{
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    char desc[256], *p;
    p = desc + snprintf(desc, sizeof(desc), "/rom@%s", name);
    если (instance)
        snprintf(p, desc+sizeof(desc)-p, ":rom%x", instance);
    вернуть find_prio(desc);
}

static целое usb_portmap(struct usbdevice_s *usbdev)
{
    если (usbdev->hub->op->portmap)
        вернуть usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    вернуть usbdev->port + 1;
}

static char *
build_usb_path(char *buf, целое max, struct usbhub_s *hub)
{
    если (!hub->usbdev)
        // Root hub - nothing to add.
        вернуть buf;
    char *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    вернуть p;
}

целое bootprio_find_usb(struct usbdevice_s *usbdev, целое lun)
{
    если (!CONFIG_BOOTORDER)
        вернуть -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    char desc[256], *p;

    если (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, sizeof(desc), "usb", usbdev->hub->cntl->pci);
    иначе если (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, sizeof(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    иначе
        вернуть -1;

    p = build_usb_path(p, desc+sizeof(desc)-p, usbdev->hub);
    snprintf(p, desc+sizeof(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    целое ret = find_prio(desc);
    если (ret >= 0)
        вернуть ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+sizeof(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    вернуть find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

static целое BootRetryTime;
static целое CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

static целое DefaultFloppyPrio = 101;
static целое DefaultCDPrio     = 102;
static целое DefaultHDPrio     = 103;
static целое DefaultBEVPrio    = 104;

void
boot_init(void)
{
    если (! CONFIG_BOOT)
        вернуть;

    если (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        если (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        целое i;
        для (i=101; i<104; i++) {
            u32 val = bootorder & 0x0f;
            bootorder >>= 4;
            switch (val) {
            case 1: DefaultFloppyPrio = i; break;
            case 2: DefaultHDPrio = i;     break;
            case 3: DefaultCDPrio = i;     break;
            case 4: DefaultBEVPrio = i;    break;
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

struct bootentry_s {
    целое type;
    union {
        u32 data;
        struct segoff_s vector;
        struct drive_s *drive;
    };
    целое priority;
    const char *description;
    struct hlist_node node;
};
static struct hlist_head BootList VARVERIFY32INIT;

#define IPL_TYPE_FLOPPY      0x01
#define IPL_TYPE_HARDDISK    0x02
#define IPL_TYPE_CDROM       0x03
#define IPL_TYPE_CBFS        0x20
#define IPL_TYPE_BEV         0x80
#define IPL_TYPE_BCV         0x81
#define IPL_TYPE_HALT        0xf0

static void
bootentry_add(целое type, целое prio, u32 data, const char *desc)
{
    если (! CONFIG_BOOT)
        вернуть;
    struct bootentry_s *be = malloc_tmp(sizeof(*be));
    если (!be) {
        warn_noalloc();
        вернуть;
    }
    be->type = type;
    be->priority = prio;
    be->data = data;
    be->description = desc ?: "?";
    dprintf(3, "Registering bootable: %s (type:%d prio:%d data:%x)\n"
            , be->description, type, prio, data);

    // Add entry in sorted order.
    struct hlist_node **pprev;
    struct bootentry_s *pos;
    hlist_for_each_entry_pprev(pos, pprev, &BootList, node) {
        если (be->priority < pos->priority)
            break;
        если (be->priority > pos->priority)
            continue;
        если (be->type < pos->type)
            break;
        если (be->type > pos->type)
            continue;
        если (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            break;
    }
    hlist_add(&be->node, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
static inline целое defPrio(целое priority, целое defaultprio) {
    вернуть (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
void
boot_add_bev(u16 seg, u16 bev, u16 desc, целое prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
void
boot_add_bcv(u16 seg, u16 ip, u16 desc, целое prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

void
boot_add_floppy(struct drive_s *drive, const char *desc, целое prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

void
boot_add_hd(struct drive_s *drive, const char *desc, целое prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

void
boot_add_cd(struct drive_s *drive, const char *desc, целое prio)
{
    если (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        char *extra = cdrom_media_info(drive);
        если (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            free(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
void
boot_add_cbfs(void *data, const char *desc, целое prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
static целое
check_for_keystroke(void)
{
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    вернуть !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
static целое
get_raw_keystroke(void)
{
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    вернуть br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
целое
get_keystroke_full(целое msec)
{
    u32 end = irqtimer_calc(msec);
    для (;;) {
        если (check_for_keystroke())
            вернуть get_raw_keystroke();
        если (irqtimer_check(end))
            вернуть -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
целое
get_keystroke(целое msec)
{
    целое keystroke = get_keystroke_full(msec);

    если (keystroke < 0)
        вернуть keystroke;
    вернуть keystroke >> 8;
}

/****************************************************************
 * Boot menu and BCV execution
 ****************************************************************/

#define DEFAULT_BOOTMENU_WAIT 2500

static const char menuchars[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', /* skip t (tpm menu) */
    'u', 'v', 'w', 'x', 'y', 'z'
};

// Show IPL option menu.
void
interactive_bootmenu(void)
{
    если (! CONFIG_BOOTMENU)
        вернуть;
    целое show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    если (!show_boot_menu)
        вернуть;

    // skip menu if only one boot device and no TPM
    если (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        вернуть;
    }

    целое menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    целое menukey = romfile_loadint("etc/boot-menu-key", 1);
    целое scan_code;
    если (menutime >= 0) {
        пока (get_keystroke(0) >= 0)
            ;

        char *bootmsg = romfile_loadfile("etc/boot-menu-message", NULL);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        free(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        если (scan_code != menukey)
            вернуть;
    }

    пока (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    целое maxmenu = 0;
    struct bootentry_s *pos, *boot = NULL;
    hlist_for_each_entry(pos, &BootList, node) {
        char desc[77];
        если (maxmenu >= ARRAY_SIZE(menuchars)) {
            break;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    если (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    целое esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    для (;;) {
        целое keystroke = get_keystroke_full(1000);
        если (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            continue;
        если (keystroke < 0) // timeout
            continue;

        scan_code = keystroke >> 8;
        целое key_ascii = keystroke & 0xff;
        если (tpm_can_show_menu() && key_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        если (scan_code == 1) {
            // ESC
            printf("\n");
            вернуть;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, node) {
            если (maxmenu >= ARRAY_SIZE(menuchars))
                break;
            если (key_ascii == menuchars[maxmenu]) {
                boot = pos;
                break;
            }
            maxmenu++;
        }
        если (boot)
            break;
    }
    printf("\n");

    // Find entry and make top priority.
    hlist_del(&boot->node);
    boot->priority = 0;
    hlist_add_head(&boot->node, &BootList);
}

// BEV (Boot Execution Vector) list
struct bev_s {
    целое type;
    u32 vector;
};
static struct bev_s BEV[20];
static целое BEVCount;
static целое HaveHDBoot, HaveFDBoot;

static void
add_bev(целое type, u32 vector)
{
    если (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        вернуть;
    если (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        вернуть;
    если (BEVCount >= ARRAY_SIZE(BEV))
        вернуть;
    struct bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
void
bcv_prepboot(void)
{
    если (! CONFIG_BOOT)
        вернуть;

    целое haltprio = find_prio("HALT");
    если (haltprio >= 0)
        bootentry_add(IPL_TYPE_HALT, haltprio, 0, "HALT");

    // Map drives and populate BEV list
    struct bootentry_s *pos;
    hlist_for_each_entry(pos, &BootList, node) {
        switch (pos->type) {
        case IPL_TYPE_BCV:
            call_bcv(pos->vector.seg, pos->vector.offset);
            add_bev(IPL_TYPE_HARDDISK, 0);
            break;
        case IPL_TYPE_FLOPPY:
            map_floppy_drive(pos->drive);
            add_bev(IPL_TYPE_FLOPPY, 0);
            break;
        case IPL_TYPE_HARDDISK:
            map_hd_drive(pos->drive);
            add_bev(IPL_TYPE_HARDDISK, 0);
            break;
        case IPL_TYPE_CDROM:
            map_cd_drive(pos->drive);
            // NO BREAK
        default:
            add_bev(pos->type, pos->data);
            break;
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
static void
call_boot_entry(struct segoff_s bootsegip, u8 bootdrv)
{
    dprintf(1, "Booting from %04x:%04x\n", bootsegip.seg, bootsegip.offset);
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF;
    br.code = bootsegip;
    // Set the magic number in ax and the boot drive in dl.
    br.dl = bootdrv;
    br.ax = 0xaa55;
    farcall16(&br);
}

// Boot from a disk (either floppy or harddrive)
static void
boot_disk(u8 bootdrv, целое checksig)
{
    u16 bootseg = 0x07c0;

    // Read sector
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF;
    br.dl = bootdrv;
    br.es = bootseg;
    br.ah = 2;
    br.al = 1;
    br.cl = 1;
    call16_int(0x13, &br);

    если (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        вернуть;
    }

    если (checksig) {
        struct mbr_s *mbr = (void*)0;
        если (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            вернуть;
        }
    }

    tpm_add_bcv(bootdrv, MAKE_FLATPTR(bootseg, 0), 512);

    /* Canonicalize bootseg:bootip */
    u16 bootip = (bootseg & 0x0fff) << 4;
    bootseg &= 0xf000;

    call_boot_entry(SEGOFF(bootseg, bootip), bootdrv);
}

// Boot from a CD-ROM
static void
boot_cdrom(struct drive_s *drive)
{
    если (! CONFIG_CDROM_BOOT)
        вернуть;
    printf("Booting from DVD/CD...\n");

    целое status = cdrom_boot(drive);
    если (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        вернуть;
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
static void
boot_cbfs(struct cbfs_file *file)
{
    если (!CONFIG_COREBOOT_FLASH)
        вернуть;
    printf("Booting from CBFS...\n");
    cbfs_run_payload(file);
}

// Boot from a BEV entry on an optionrom.
static void
boot_rom(u32 vector)
{
    printf("Booting from ROM...\n");
    struct segoff_s so;
    so.segoff = vector;
    call_boot_entry(so, 0);
}

// Unable to find bootable device - warn user and eventually retry.
static void
boot_fail(void)
{
    если (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    иначе
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    для (;;) {
        если (BootRetryTime != (u32)-1 && irqtimer_check(end))
            break;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
static void
do_boot(целое seq_nr)
{
    если (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    если (seq_nr >= BEVCount)
        boot_fail();

    // Boot the given BEV type.
    struct bev_s *ie = &BEV[seq_nr];
    switch (ie->type) {
    case IPL_TYPE_FLOPPY:
        printf("Booting from Floppy...\n");
        boot_disk(0x00, CheckFloppySig);
        break;
    case IPL_TYPE_HARDDISK:
        printf("Booting from Hard Disk...\n");
        boot_disk(0x80, 1);
        break;
    case IPL_TYPE_CDROM:
        boot_cdrom((void*)ie->vector);
        break;
    case IPL_TYPE_CBFS:
        boot_cbfs((void*)ie->vector);
        break;
    case IPL_TYPE_BEV:
        boot_rom(ie->vector);
        break;
    case IPL_TYPE_HALT:
        boot_fail();
        break;
    }

    // Boot failed: invoke the boot recovery function
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF;
    call16_int(0x18, &br);
}

целое BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
void VISIBLE32FLAT
handle_18(void)
{
    debug_enter(NULL, DEBUG_HDL_18);
    целое seq = BootSequence + 1;
    BootSequence = seq;
    do_boot(seq);
}

// INT 19h Boot Load Service Entry Point
void VISIBLE32FLAT
handle_19(void)
{
    debug_enter(NULL, DEBUG_HDL_19);
    BootSequence = 0;
    do_boot(0);
}
