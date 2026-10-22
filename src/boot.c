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
        якщо (!*glob && (!*str || *str == '/'))
            повернути (char*)str;
        якщо (*glob == '*') {
            якщо (!*str || *str == '/' || *str == glob[1])
                glob++;
            інакше
                str++;
            continue;
        }
        якщо (*glob != *str)
            повернути NULL;
        glob++;
        str++;
    }
}

#define FW_PCI_DOMAIN "/pci@i0cf8"

static char *
build_pci_path(char *buf, ціле max, const char *devname, struct pci_device *pci)
{
    // Build the string path of a bdf - for example: /pci@i0cf8/isa@1,2
    char *p = buf;
    якщо (pci->parent) {
        p = build_pci_path(p, max, "pci-bridge", pci->parent);
    } інакше {
        p += snprintf(p, buf+max-p, "%s", FW_PCI_DOMAIN);
        якщо (pci->rootbus)
            p += snprintf(p, buf+max-p, ",%x", pci->rootbus);
    }

    ціле dev = pci_bdf_to_dev(pci->bdf), fn = pci_bdf_to_fn(pci->bdf);
    p += snprintf(p, buf+max-p, "/%s@%x", devname, dev);
    якщо (fn)
        p += snprintf(p, buf+max-p, ",%x", fn);
    повернути p;
}

static char *
build_scsi_path(char *buf, ціле max,
                struct pci_device *pci, ціле target, ціле lun)
{
    // Build the string path of a scsi drive - for example:
    // /pci@i0cf8/scsi@5/channel@0/disk@1,0
    char *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/*@0/*@%x,%x", target, lun);
    повернути p;
}

static char *
build_ata_path(char *buf, ціле max,
               struct pci_device *pci, ціле chanid, ціле slave)
{
    // Build the string path of an ata drive - for example:
    // /pci@i0cf8/ide@1,1/drive@1/disk@0
    char *p;
    p = build_pci_path(buf, max, "*", pci);
    p += snprintf(p, buf+max-p, "/drive@%x/disk@%x", chanid, slave);
    повернути p;
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
static ціле BiosGeometryCount;

static char *
parse_u32(char *cur, u32 *n)
{
    u32 m = 0;
    якщо (cur) {
        поки ('0' <= *cur && *cur <= '9') {
            m = 10 * m + (*cur - '0');
            cur++;
        }
        якщо (*cur != '\0')
            cur++;
    }
    *n = m;
    повернути cur;
}

static void
loadBiosGeometry(void)
{
    якщо (!CONFIG_HOST_BIOS_GEOMETRY)
        повернути;
    char *f = romfile_loadfile("bios-geometry", NULL);
    якщо (!f)
        повернути;

    ціле i = 0;
    BiosGeometryCount = 1;
    поки (f[i]) {
        якщо (f[i] == '\n')
            BiosGeometryCount++;
        i++;
    }
    BiosGeometry = malloc_tmphigh(BiosGeometryCount * sizeof(BootDeviceLCHS));
    якщо (!BiosGeometry) {
        warn_noalloc();
        free(f);
        BiosGeometryCount = 0;
        повернути;
    }

    dprintf(1, "bios geometry:\n");
    i = 0;
    do {
        BootDeviceLCHS *d = &BiosGeometry[i];
        d->name = f;
        f = strchr(f, '\n');
        якщо (f)
            *(f++) = '\0';
        char *chs_values = strchr(d->name, ' ');
        якщо (chs_values)
            *(chs_values++) = '\0';
        chs_values = parse_u32(chs_values, &d->lcyls);
        chs_values = parse_u32(chs_values, &d->lheads);
        chs_values = parse_u32(chs_values, &d->lsecs);
        dprintf(1, "%s: (%u, %u, %u)\n",
                d->name, d->lcyls, d->lheads, d->lsecs);
        i++;
    } поки (f);
}

// Search the bios-geometry list for the given glob pattern.
static BootDeviceLCHS *
boot_lchs_find(const char *glob)
{
    dprintf(1, "Searching bios-geometry for: %s\n", glob);
    ціле i;
    для (i = 0; i < BiosGeometryCount; i++)
        якщо (glob_prefix(glob, BiosGeometry[i].name))
            повернути &BiosGeometry[i];
    повернути NULL;
}

ціле boot_lchs_find_pci_device(struct pci_device *pci, struct chs_s *chs)
{
    якщо (!CONFIG_HOST_BIOS_GEOMETRY)
        повернути -1;
    char desc[256];
    build_pci_path(desc, sizeof(desc), "*", pci);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    якщо (!b)
        повернути -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    повернути 0;
}

ціле boot_lchs_find_scsi_device(struct pci_device *pci, ціле target, ціле lun,
                               struct chs_s *chs)
{
    якщо (!CONFIG_HOST_BIOS_GEOMETRY)
        повернути -1;
    якщо (!pci)
        // support only pci machine for now
        повернути -1;
    // Find scsi drive - for example: /pci@i0cf8/scsi@5/channel@0/disk@1,0
    char desc[256];
    build_scsi_path(desc, sizeof(desc), pci, target, lun);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    якщо (!b)
        повернути -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    повернути 0;
}

ціле boot_lchs_find_ata_device(struct pci_device *pci, ціле chanid, ціле slave,
                              struct chs_s *chs)
{
    якщо (!CONFIG_HOST_BIOS_GEOMETRY)
        повернути -1;
    якщо (!pci)
        // support only pci machine for now
        повернути -1;
    // Find ata drive - for example: /pci@i0cf8/ide@1,1/drive@1/disk@0
    char desc[256];
    build_ata_path(desc, sizeof(desc), pci, chanid, slave);
    BootDeviceLCHS *b = boot_lchs_find(desc);
    якщо (!b)
        повернути -1;
    chs->cylinder = (u16)b->lcyls;
    chs->head = (u16)b->lheads;
    chs->sector = (u16)b->lsecs;
    повернути 0;
}


/****************************************************************
 * Boot priority ordering
 ****************************************************************/

static char **Bootorder VARVERIFY32INIT;
static ціле BootorderCount;

static void
loadBootOrder(void)
{
    якщо (!CONFIG_BOOTORDER)
        повернути;

    char *f = romfile_loadfile("bootorder", NULL);
    якщо (!f)
        повернути;

    ціле i = 0;
    BootorderCount = 1;
    поки (f[i]) {
        якщо (f[i] == '\n')
            BootorderCount++;
        i++;
    }
    Bootorder = malloc_tmphigh(BootorderCount*sizeof(char*));
    якщо (!Bootorder) {
        warn_noalloc();
        free(f);
        BootorderCount = 0;
        повернути;
    }

    dprintf(1, "boot order:\n");
    i = 0;
    do {
        Bootorder[i] = f;
        f = strchr(f, '\n');
        якщо (f)
            *(f++) = '\0';
        Bootorder[i] = nullTrailingSpace(Bootorder[i]);
        dprintf(1, "%d: %s\n", i+1, Bootorder[i]);
        i++;
    } поки (f);
}

// Search the bootorder list for the given glob pattern.
static ціле
find_prio(const char *glob)
{
    dprintf(1, "Searching bootorder for: %s\n", glob);
    ціле i;
    для (i = 0; i < BootorderCount; i++)
        якщо (glob_prefix(glob, Bootorder[i]))
            повернути i+1;
    повернути -1;
}

u8 is_bootprio_strict(void)
{
    static ціле prio_halt = -2;

    якщо (prio_halt == -2)
        prio_halt = find_prio("HALT");
    повернути prio_halt >= 0;
}

ціле bootprio_find_pci_device(struct pci_device *pci)
{
    якщо (CONFIG_CSM)
        повернути csm_bootprio_pci(pci);
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    // Find pci device - for example: /pci@i0cf8/ethernet@5
    char desc[256];
    build_pci_path(desc, sizeof(desc), "*", pci);
    повернути find_prio(desc);
}

ціле bootprio_find_mmio_device(void *mmio)
{
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    char desc[256];
    snprintf(desc, sizeof(desc), "/virtio-mmio@%016x/*", (u32)mmio);
    повернути find_prio(desc);
}

ціле bootprio_find_scsi_device(struct pci_device *pci, ціле target, ціле lun)
{
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    якщо (!pci)
        // support only pci machine for now
        повернути -1;
    char desc[256];
    build_scsi_path(desc, sizeof(desc), pci, target, lun);
    повернути find_prio(desc);
}

ціле bootprio_find_scsi_mmio_device(void *mmio, ціле target, ціле lun)
{
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    char desc[256];
    snprintf(desc, sizeof(desc), "/virtio-mmio@%016x/*@0/*@%x,%x",
             (u32)mmio, target, lun);
    повернути find_prio(desc);
}

ціле bootprio_find_ata_device(struct pci_device *pci, ціле chanid, ціле slave)
{
    якщо (CONFIG_CSM)
        повернути csm_bootprio_ata(pci, chanid, slave);
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    якщо (!pci)
        // support only pci machine for now
        повернути -1;
    char desc[256];
    build_ata_path(desc, sizeof(desc), pci, chanid, slave);
    повернути find_prio(desc);
}

ціле bootprio_find_fdc_device(struct pci_device *pci, ціле port, ціле fdid)
{
    якщо (CONFIG_CSM)
        повернути csm_bootprio_fdc(pci, port, fdid);
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    якщо (!pci)
        // support only pci machine for now
        повернути -1;
    // Find floppy - for example: /pci@i0cf8/isa@1/fdc@03f1/floppy@0
    char desc[256], *p;
    p = build_pci_path(desc, sizeof(desc), "isa", pci);
    snprintf(p, desc+sizeof(desc)-p, "/fdc@%04x/floppy@%x", port, fdid);
    повернути find_prio(desc);
}

ціле bootprio_find_pci_rom(struct pci_device *pci, ціле instance)
{
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    // Find pci rom - for example: /pci@i0cf8/scsi@3:rom2
    char desc[256], *p;
    p = build_pci_path(desc, sizeof(desc), "*", pci);
    якщо (instance)
        snprintf(p, desc+sizeof(desc)-p, ":rom%x", instance);
    повернути find_prio(desc);
}

ціле bootprio_find_named_rom(const char *name, ціле instance)
{
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    // Find named rom - for example: /rom@genroms/linuxboot.bin
    char desc[256], *p;
    p = desc + snprintf(desc, sizeof(desc), "/rom@%s", name);
    якщо (instance)
        snprintf(p, desc+sizeof(desc)-p, ":rom%x", instance);
    повернути find_prio(desc);
}

static ціле usb_portmap(struct usbdevice_s *usbdev)
{
    якщо (usbdev->hub->op->portmap)
        повернути usbdev->hub->op->portmap(usbdev->hub, usbdev->port);
    повернути usbdev->port + 1;
}

static char *
build_usb_path(char *buf, ціле max, struct usbhub_s *hub)
{
    якщо (!hub->usbdev)
        // Root hub - nothing to add.
        повернути buf;
    char *p = build_usb_path(buf, max, hub->usbdev->hub);
    p += snprintf(p, buf+max-p, "/hub@%x", usb_portmap(hub->usbdev));
    повернути p;
}

ціле bootprio_find_usb(struct usbdevice_s *usbdev, ціле lun)
{
    якщо (!CONFIG_BOOTORDER)
        повернути -1;
    // Find usb - examples:
    //   pci:  /pci@i0cf8/usb@1,2/storage@1/channel@0/disk@0,0
    //   mmio: /sysbus-xhci@00000000fe900000/storage@1/channel@0/disk@0,0
    char desc[256], *p;

    якщо (usbdev->hub->cntl->pci)
        p = build_pci_path(desc, sizeof(desc), "usb", usbdev->hub->cntl->pci);
    інакше якщо (usbdev->hub->cntl->mmio)
        p = desc + snprintf(desc, sizeof(desc), "/*@%016x"
                            , (u32)usbdev->hub->cntl->mmio);
    інакше
        повернути -1;

    p = build_usb_path(p, desc+sizeof(desc)-p, usbdev->hub);
    snprintf(p, desc+sizeof(desc)-p, "/storage@%x/*@0/*@0,%x"
             , usb_portmap(usbdev), lun);
    ціле ret = find_prio(desc);
    якщо (ret >= 0)
        повернути ret;
    // Try usb-host/redir - for example: /pci@i0cf8/usb@1,2/usb-host@1
    snprintf(p, desc+sizeof(desc)-p, "/usb-*@%x", usb_portmap(usbdev));
    повернути find_prio(desc);
}


/****************************************************************
 * Boot setup
 ****************************************************************/

static ціле BootRetryTime;
static ціле CheckFloppySig = 1;

#define DEFAULT_PRIO           9999

static ціле DefaultFloppyPrio = 101;
static ціле DefaultCDPrio     = 102;
static ціле DefaultHDPrio     = 103;
static ціле DefaultBEVPrio    = 104;

void
boot_init(void)
{
    якщо (! CONFIG_BOOT)
        повернути;

    якщо (CONFIG_QEMU) {
        // On emulators, get boot order from nvram.
        якщо (rtc_read(CMOS_BIOS_BOOTFLAG1) & 1)
            CheckFloppySig = 0;
        u32 bootorder = (rtc_read(CMOS_BIOS_BOOTFLAG2)
                         | ((rtc_read(CMOS_BIOS_BOOTFLAG1) & 0xf0) << 4));
        DefaultFloppyPrio = DefaultCDPrio = DefaultHDPrio
            = DefaultBEVPrio = DEFAULT_PRIO;
        ціле i;
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
    ціле type;
    union {
        u32 data;
        struct segoff_s vector;
        struct drive_s *drive;
    };
    ціле priority;
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
bootentry_add(ціле type, ціле prio, u32 data, const char *desc)
{
    якщо (! CONFIG_BOOT)
        повернути;
    struct bootentry_s *be = malloc_tmp(sizeof(*be));
    якщо (!be) {
        warn_noalloc();
        повернути;
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
        якщо (be->priority < pos->priority)
            break;
        якщо (be->priority > pos->priority)
            continue;
        якщо (be->type < pos->type)
            break;
        якщо (be->type > pos->type)
            continue;
        якщо (be->type <= IPL_TYPE_CDROM
            && (be->drive->type < pos->drive->type
                || (be->drive->type == pos->drive->type
                    && be->drive->cntl_id < pos->drive->cntl_id)))
            break;
    }
    hlist_add(&be->node, pprev);
}

// Return the given priority if it's set - defaultprio otherwise.
static inline ціле defPrio(ціле priority, ціле defaultprio) {
    повернути (priority < 0) ? defaultprio : priority;
}

// Add a BEV vector for a given pnp compatible option rom.
void
boot_add_bev(u16 seg, u16 bev, u16 desc, ціле prio)
{
    bootentry_add(IPL_TYPE_BEV, defPrio(prio, DefaultBEVPrio)
                  , SEGOFF(seg, bev).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Unknown");
    DefaultBEVPrio = DEFAULT_PRIO;
}

// Add a bcv entry for an expansion card harddrive or legacy option rom
void
boot_add_bcv(u16 seg, u16 ip, u16 desc, ціле prio)
{
    bootentry_add(IPL_TYPE_BCV, defPrio(prio, DefaultHDPrio)
                  , SEGOFF(seg, ip).segoff
                  , desc ? MAKE_FLATPTR(seg, desc) : "Legacy option rom");
}

void
boot_add_floppy(struct drive_s *drive, const char *desc, ціле prio)
{
    bootentry_add(IPL_TYPE_FLOPPY, defPrio(prio, DefaultFloppyPrio)
                  , (u32)drive, desc);
}

void
boot_add_hd(struct drive_s *drive, const char *desc, ціле prio)
{
    bootentry_add(IPL_TYPE_HARDDISK, defPrio(prio, DefaultHDPrio)
                  , (u32)drive, desc);
}

void
boot_add_cd(struct drive_s *drive, const char *desc, ціле prio)
{
    якщо (GET_GLOBAL(PlatformRunningOn) & PF_QEMU) {
        // We want short boot times.  But on physical hardware even
        // the test unit ready can take several seconds.  So do media
        // access on qemu only, where we know it will be fast.
        char *extra = cdrom_media_info(drive);
        якщо (extra) {
            desc = znprintf(MAXDESCSIZE, "%s (%s)", desc, extra);
            free(extra);
        }
    }
    bootentry_add(IPL_TYPE_CDROM, defPrio(prio, DefaultCDPrio)
                  , (u32)drive, desc);
}

// Add a CBFS payload entry
void
boot_add_cbfs(void *data, const char *desc, ціле prio)
{
    bootentry_add(IPL_TYPE_CBFS, defPrio(prio, DEFAULT_PRIO), (u32)data, desc);
}


/****************************************************************
 * Keyboard calls
 ****************************************************************/

// See if a keystroke is pending in the keyboard buffer.
static ціле
check_for_keystroke(void)
{
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF|F_ZF;
    br.ah = 1;
    call16_int(0x16, &br);
    повернути !(br.flags & F_ZF);
}

// Return a keystroke - waiting forever if necessary.
static ціле
get_raw_keystroke(void)
{
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF;
    call16_int(0x16, &br);
    повернути br.ax;
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns both scancode and ascii code.
ціле
get_keystroke_full(ціле msec)
{
    u32 end = irqtimer_calc(msec);
    для (;;) {
        якщо (check_for_keystroke())
            повернути get_raw_keystroke();
        якщо (irqtimer_check(end))
            повернути -1;
        yield_toirq();
    }
}

// Read a keystroke - waiting up to 'msec' milliseconds.
// returns scancode only.
ціле
get_keystroke(ціле msec)
{
    ціле keystroke = get_keystroke_full(msec);

    якщо (keystroke < 0)
        повернути keystroke;
    повернути keystroke >> 8;
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
    якщо (! CONFIG_BOOTMENU)
        повернути;
    ціле show_boot_menu = romfile_loadint("etc/show-boot-menu", 1);
    якщо (!show_boot_menu)
        повернути;

    // skip menu if only one boot device and no TPM
    якщо (show_boot_menu == 2 && !tpm_can_show_menu()
        && !hlist_empty(&BootList) && !BootList.first->next) {
        dprintf(1, "Only one boot device present. Skip boot menu.\n");
        printf("\n");
        повернути;
    }

    ціле menutime = romfile_loadint("etc/boot-menu-wait", DEFAULT_BOOTMENU_WAIT);
    ціле menukey = romfile_loadint("etc/boot-menu-key", 1);
    ціле scan_code;
    якщо (menutime >= 0) {
        поки (get_keystroke(0) >= 0)
            ;

        char *bootmsg = romfile_loadfile("etc/boot-menu-message", NULL);
        printf("%s", bootmsg ?: "\nPress ESC for boot menu.\n\n");
        free(bootmsg);

        enable_bootsplash();
        scan_code = get_keystroke(menutime);
        disable_bootsplash();
        якщо (scan_code != menukey)
            повернути;
    }

    поки (get_keystroke(0) >= 0)
        ;

    printf("Select boot device:\n\n");
    wait_threads();

    // Show menu items
    ціле maxmenu = 0;
    struct bootentry_s *pos, *boot = NULL;
    hlist_for_each_entry(pos, &BootList, node) {
        char desc[77];
        якщо (maxmenu >= ARRAY_SIZE(menuchars)) {
            break;
        }
        printf("%c. %s\n", menuchars[maxmenu]
               , strtcpy(desc, pos->description, ARRAY_SIZE(desc)));
        maxmenu++;
    }
    якщо (tpm_can_show_menu()) {
        printf("\nt. TPM Configuration\n");
    }

    // Get key press.  If the menu key is ESC, do not restart boot unless
    // 1.5 seconds have passed.  Otherwise users (trained by years of
    // repeatedly hitting keys to enter the BIOS) will end up hitting ESC
    // multiple times and immediately booting the primary boot device.
    ціле esc_accepted_time = irqtimer_calc(menukey == 1 ? 1500 : 0);
    для (;;) {
        ціле keystroke = get_keystroke_full(1000);
        якщо (keystroke == 0x011b && !irqtimer_check(esc_accepted_time))
            continue;
        якщо (keystroke < 0) // timeout
            continue;

        scan_code = keystroke >> 8;
        ціле key_ascii = keystroke & 0xff;
        якщо (tpm_can_show_menu() && key_ascii == 't') {
            printf("\n");
            tpm_menu();
        }
        якщо (scan_code == 1) {
            // ESC
            printf("\n");
            повернути;
        }

        maxmenu = 0;
        hlist_for_each_entry(pos, &BootList, node) {
            якщо (maxmenu >= ARRAY_SIZE(menuchars))
                break;
            якщо (key_ascii == menuchars[maxmenu]) {
                boot = pos;
                break;
            }
            maxmenu++;
        }
        якщо (boot)
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
    ціле type;
    u32 vector;
};
static struct bev_s BEV[20];
static ціле BEVCount;
static ціле HaveHDBoot, HaveFDBoot;

static void
add_bev(ціле type, u32 vector)
{
    якщо (type == IPL_TYPE_HARDDISK && HaveHDBoot++)
        повернути;
    якщо (type == IPL_TYPE_FLOPPY && HaveFDBoot++)
        повернути;
    якщо (BEVCount >= ARRAY_SIZE(BEV))
        повернути;
    struct bev_s *bev = &BEV[BEVCount++];
    bev->type = type;
    bev->vector = vector;
}

// Prepare for boot - show menu and run bcvs.
void
bcv_prepboot(void)
{
    якщо (! CONFIG_BOOT)
        повернути;

    ціле haltprio = find_prio("HALT");
    якщо (haltprio >= 0)
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
boot_disk(u8 bootdrv, ціле checksig)
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

    якщо (br.flags & F_CF) {
        printf("Boot failed: could not read the boot disk\n\n");
        повернути;
    }

    якщо (checksig) {
        struct mbr_s *mbr = (void*)0;
        якщо (GET_FARVAR(bootseg, mbr->signature) != MBR_SIGNATURE) {
            printf("Boot failed: not a bootable disk\n\n");
            повернути;
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
    якщо (! CONFIG_CDROM_BOOT)
        повернути;
    printf("Booting from DVD/CD...\n");

    ціле status = cdrom_boot(drive);
    якщо (status) {
        printf("Boot failed: Could not read from CDROM (code %04x)\n", status);
        повернути;
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
    якщо (!CONFIG_COREBOOT_FLASH)
        повернути;
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
    якщо (BootRetryTime == (u32)-1)
        printf("No bootable device.\n");
    інакше
        printf("No bootable device.  Retrying in %d seconds.\n"
               , BootRetryTime/1000);
    // Wait for 'BootRetryTime' milliseconds and then reboot.
    u32 end = irqtimer_calc(BootRetryTime);
    для (;;) {
        якщо (BootRetryTime != (u32)-1 && irqtimer_check(end))
            break;
        yield_toirq();
    }
    printf("Rebooting.\n");
    reset();
}

// Determine next boot method and attempt a boot using it.
static void
do_boot(ціле seq_nr)
{
    якщо (! CONFIG_BOOT)
        panic("Boot support not compiled in.\n");

    якщо (seq_nr >= BEVCount)
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

ціле BootSequence VARLOW = -1;

// Boot Failure recovery: try the next device.
void VISIBLE32FLAT
handle_18(void)
{
    debug_enter(NULL, DEBUG_HDL_18);
    ціле seq = BootSequence + 1;
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
