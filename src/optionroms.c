// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "bregs.h" // struct bregs
#include "config.h" // CONFIG_*
#include "farptr.h" // FLATPTR_TO_SEG
#include "biosvar.h" // GET_IVT
#include "hw/pci.h" // pci_config_readl
#include "hw/pcidevice.h" // foreachpci
#include "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#include "hw/pci_regs.h" // PCI_ROM_ADDRESS
#include "malloc.h" // rom_confirm
#include "output.h" // dprintf
#include "romfile.h" // romfile_loadint
#include "stacks.h" // farcall16big
#include "std/optionrom.h" // struct rom_header
#include "std/pnpbios.h" // PNP_SIGNATURE
#include "string.h" // memset
#include "util.h" // get_pnp_offset
#include "tcgbios.h" // tpm_*

static целое EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
static void
__callrom(struct rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_offset();
    br.code = SEGOFF(seg, offset);
    start_preempt();
    farcall16big(&br);
    finish_preempt();
}

// Execute a given option rom at the standard entry vector.
void
callrom(struct rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
void
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
static целое
is_valid_rom(struct rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    если (rom->signature != OPTION_ROM_SIGNATURE)
        вернуть 0;
    если (! rom->size)
        вернуть 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    если (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        если (EnforceChecksum)
            вернуть 0;
    }
    вернуть 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
static struct pnp_data *
get_pnp_rom(struct rom_header *rom)
{
    struct pnp_data *pnp = (void*)((u8*)rom + rom->pnpoffset);
    если (pnp->signature != PNP_SIGNATURE)
        вернуть NULL;
    вернуть pnp;
}

// Check for multiple pnp option rom headers.
static struct pnp_data *
get_pnp_next(struct rom_header *rom, struct pnp_data *pnp)
{
    если (! pnp->nextoffset)
        вернуть NULL;
    pnp = (void*)((u8*)rom + pnp->nextoffset);
    если (pnp->signature != PNP_SIGNATURE)
        вернуть NULL;
    вернуть pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
static struct pci_data *
get_pci_rom(struct rom_header *rom)
{
    struct pci_data *pd = (void*)((u32)rom + rom->pcioffset);
    если (pd->signature != PCI_ROM_SIGNATURE)
        вернуть NULL;
    если (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    вернуть pd;
}

// Run rom init code and note rom size.
static целое
init_optionrom(struct rom_header *rom, u16 bdf, целое isvga)
{
    если (! is_valid_rom(rom))
        вернуть -1;
    struct rom_header *newrom = rom_reserve(rom->size * 512);
    если (!newrom) {
        warn_noalloc();
        вернуть -1;
    }
    если (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    если (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    вернуть rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

static void
setRomSource(u64 *sources, struct rom_header *rom, u64 source)
{
    если (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

static целое
getRomPriority(u64 *sources, struct rom_header *rom, целое instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    если (!source)
        вернуть -1;
    если (source & RS_PCIROM)
        вернуть bootprio_find_pci_rom((void*)(u32)source, instance);
    struct romfile_s *file = (void*)(u32)source;
    вернуть bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

static struct rom_header *
deploy_romfile(struct romfile_s *file)
{
    u32 size = file->size;
    struct rom_header *rom = rom_reserve(size);
    если (!rom) {
        warn_noalloc();
        вернуть NULL;
    }
    целое ret = file->copy(file, rom, size);
    если (ret <= 0)
        вернуть NULL;
    вернуть rom;
}

// Run all roms in a given CBFS directory.
static void
run_file_roms(const char *prefix, целое isvga, u64 *sources)
{
    struct romfile_s *file = NULL;
    для (;;) {
        file = romfile_findprefix(prefix, file);
        если (!file)
            break;
        struct rom_header *rom = deploy_romfile(file);
        если (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
целое
is_pci_vga(struct pci_device *pci)
{
    если (pci->class != PCI_CLASS_DISPLAY_VGA)
        вернуть 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    если (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        вернуть 0;
    пока (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        если (!(ctrl & PCI_BRIDGE_CTL_VGA))
            вернуть 0;
    }
    вернуть 1;
}

// Copy a rom to its permanent location below 1MiB
static struct rom_header *
copy_rom(struct rom_header *rom)
{
    u32 romsize = rom->size * 512;
    struct rom_header *newrom = rom_reserve(romsize);
    если (!newrom) {
        warn_noalloc();
        вернуть NULL;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    вернуть newrom;
}

// Map the option rom of a given PCI device.
static struct rom_header *
map_pcirom(struct pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    если ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        вернуть NULL;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    если (!sz || sz == 0xffffffff)
        goto fail;

    если (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        goto fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    struct rom_header *rom = (void*)orig;
    для (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        если (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            goto fail;
        }
        struct pci_data *pd = get_pci_rom(rom);
        если (! pd) {
            dprintf(6, "No valid pci signature found\n");
            goto fail;
        }

        если (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            break;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        если (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            goto fail;
        }
        rom = (void*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    вернуть rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    вернуть NULL;
}

static целое boot_irq_captured(void)
{
    вернуть GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

static void boot_irq_restore(void)
{
    struct segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
static void
init_pcirom(struct pci_device *pci, целое isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    char fname[17];
    snprintf(fname, sizeof(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    struct romfile_s *file = romfile_find(fname);
    struct rom_header *rom = NULL;
    если (file)
        rom = deploy_romfile(file);
    иначе если (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    если (! rom)
        // No ROM present.
        вернуть;
    целое irq_was_captured = boot_irq_captured();
    struct pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    если (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        char *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

void
optionrom_setup(void)
{
    если (! CONFIG_OPTIONROMS)
        вернуть;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, sizeof(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    struct pci_device *pci;
    foreachpci(pci) {
        если (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            continue;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    пока (pos < rom_get_last()) {
        struct rom_header *rom = (void*)pos;
        если (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            continue;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        struct pnp_data *pnp = get_pnp_rom(rom);
        если (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            continue;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        целое instance = 0;
        пока (pnp) {
            если (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            иначе если (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            иначе
                break;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

целое ScreenAndDebug;
struct rom_header *VgaROM;

static void try_setup_display_other(void)
{
    struct pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        если (pci->class != PCI_CLASS_DISPLAY_OTHER)
            continue;
        struct rom_header *rom = map_pcirom(pci);
        если (!rom)
            continue;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        вернуть;
    }
}

// Call into vga code to turn on console.
void
vgarom_setup(void)
{
    целое have_vga = 0;

    если (! CONFIG_OPTIONROMS)
        вернуть;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((void*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    struct pci_device *pci;
    foreachpci(pci) {
        если (!is_pci_vga(pci))
            continue;
        vgahook_setup(pci);
        init_pcirom(pci, 1, NULL);
        have_vga = 1;
        break;
    }
    если (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, NULL);
    rom_reserve(0);

    если (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (void*)BUILD_ROM_START;
}

void
s3_resume_vga(void)
{
    если (!S3ResumeVga)
        вернуть;
    если (!VgaROM || ! is_valid_rom(VgaROM))
        вернуть;
    callrom(VgaROM, 0);
}
