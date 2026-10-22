// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "e820map.h" // e820_add
#ಅನ್ತರ್ಭಾವ "farptr.h" // MAKE_FLATPTR
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_to_bdf
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_probe_devices
#ಅನ್ತರ್ಭಾವ "hw/pic.h" // pic_irqmask_read
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_csm_preinit
#ಅನ್ತರ್ಭಾವ "memmap.h" //ಚಿಹ್ನೆ
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // qemu_preinit
#ಅನ್ತರ್ಭಾವ "stacks.h" // wait_threads
#ಅನ್ತರ್ಭಾವ "std/acpi.h" // RSDP_SIGNATURE
#ಅನ್ತರ್ಭಾವ "std/bda.h" // struct bios_data_area_s
#ಅನ್ತರ್ಭಾವ "std/optionrom.h" // struct rom_header
#ಅನ್ತರ್ಭಾವ "util.h" // copy_smbios_21

#define UINT8 u8
#define UINT16 u16
#define UINT32 u32
#ಅನ್ತರ್ಭಾವ "std/LegacyBios.h"

ಸಂರಚನಾ rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_TABLE csm_compat_table VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .TableChecksum = 0 /* Filled in by checkrom.py */,
    .TableLength = ಪರಿಮಾಣ(csm_compat_table),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16CallOffset = 0 /* Filled in by checkrom.py */,
    .OemIdStringPointer = (u32)"SeaBIOS",
    .AcpiRsdPtrPointer = (u32)&csm_rsdp,
};

EFI_TO_COMPATIBILITY16_INIT_TABLE *csm_init_table;
EFI_TO_COMPATIBILITY16_BOOT_TABLE *csm_boot_table;

ಸ್ಥಿರ u16 PICMask = PIC_IRQMASK_DEFAULT;

ಬಾಹ್ಯ ಶೂನ್ಯ __csm_return(ಸಂರಚನಾ bregs *regs) __noreturn;

ಸ್ಥಿರ ಶೂನ್ಯ
csm_return(ಸಂರಚನಾ bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_table.UmaAddress = rommax;
    csm_compat_table.UmaSize = ಚಿಹ್ನೆ(final_readonly_start) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
csm_maininit(ಸಂರಚನಾ bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_table.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_table.PnPInstallationCheckOffset = get_pnp_offset();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0000(ಸಂರಚನಾ bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_table = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_table->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_table->HiPmmMemory);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_table->HiPmmMemorySizeInBytes);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_table->ReverseThunkCallSegment,
            csm_init_table->ReverseThunkCallOffset);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_table->ಸಂಖ್ಯೆ_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_table->OsMemoryAbove1Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_table->ThunkStart);
    dprintf(3, "ThunkSize       %08x\n", csm_init_table->ThunkSizeInBytes);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_table->ಕೆಳಗಿನ_Pmm_Memory);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_table->ಕೆಳಗಿನ_Pmm_Memory_Size_In_Bytes);

    malloc_csm_preinit(csm_init_table->ಕೆಳಗಿನ_Pmm_Memory,
                       csm_init_table->ಕೆಳಗಿನ_Pmm_Memory_Size_In_Bytes,
                       csm_init_table->HiPmmMemory,
                       csm_init_table->HiPmmMemorySizeInBytes);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0001(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_BOOT) {
        regs->ax = 1;
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_table = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_table->MajorVersion);
    dprintf(3, "MinorVersion %04x\n", csm_boot_table->MinorVersion);
    dprintf(3, "AcpiTable %08x\n", csm_boot_table->AcpiTable);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_table->SmbiosTable);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_table->SmbiosTableLength);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_table->DevicePathType);
    dprintf(3, "PciIrqMask %04x\n", csm_boot_table->PciIrqMask);
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_table->ಸಂಖ್ಯೆ_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_table->ಸಂಖ್ಯೆ_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_table->BbsTable);
    dprintf(3, "SmmTable %08x\n", csm_boot_table->SmmTable);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_table->OsMemoryAbove1Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_table->UnconventionalDeviceTable);

    regs->ax = 0;
}

/* PrepareToBoot */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0002(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_BOOT) {
        regs->ax = 1;
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    ಸಂರಚನಾ e820entry *p = (ಶೂನ್ಯ *)csm_compat_table.E820Pointer;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i < csm_compat_table.E820Length / ಪರಿಮಾಣ(ಸಂರಚನಾ e820entry); i++)
        e820_add(p[i].start, p[i].size, p[i].type);

    ಯದಿ (csm_init_table->HiPmmMemorySizeInBytes > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = csm_init_table->HiPmmMemory + csm_init_table->HiPmmMemorySizeInBytes;
        e820_add(hi_pmm_end - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    ಯದಿ (csm_compat_table.IrqRoutingTablePointer &&
        csm_compat_table.IrqRoutingTableLength) {
        PirAddr = (ಶೂನ್ಯ *)csm_compat_table.IrqRoutingTablePointer;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    ಯದಿ (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        find_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    ಯದಿ (csm_boot_table->SmbiosTable)
        copy_smbios_21((ಶೂನ್ಯ *)csm_boot_table->SmbiosTable);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    ಸಂರಚನಾ bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    bda->hdcount = 0;

    thread_setup();
    mathcp_setup();
    timer_setup();
    clock_setup();
    device_hardware_setup();
    wait_threads();
    interactive_bootmenu();

    prepareboot();

    regs->ax = 0;
}

/* Boot */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0003(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_BOOT) {
        regs->ax = 1;
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(3, "Boot\n");

    startBoot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0005(ಸಂರಚನಾ bregs *regs)
{
    EFI_DISPATCH_OPROM_TABLE *table = MAKE_FLATPTR(regs->es, regs->bx);
    ಸಂರಚನಾ rom_header *rom;
    u16 bdf;

    ಯದಿ (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(3, "Legacy16DispatchOprom rom %p\n", table);

    dprintf(3, "OpromSegment   %04x\n", table->OpromSegment);
    dprintf(3, "RuntimeSegment %04x\n", table->RuntimeSegment);
    dprintf(3, "PnPInstallationCheck %04x:%04x\n",
            table->PnPInstallationCheckSegment,
            table->PnPInstallationCheckOffset);
    dprintf(3, "RuntimeSegment %04x\n", table->RuntimeSegment);

    rom = MAKE_FLATPTR(table->OpromSegment, 0);
    bdf = pci_bus_devfn_to_bdf(table->PciBus, table->PciDeviceFunction);

    rom_reserve(rom->size * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->size * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0006(ಸಂರಚನಾ bregs *regs)
{
    u16 size = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    ಶೂನ್ಯ *chunk = ರಿಕ್ತ;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        size, align, region);

    ಯದಿ (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    ಯದಿ (align) {
        align = 1 << __ffs(align);
        ಯದಿ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } ಅನ್ಯಥಾ {
        align = MALLOC_MIN_ALIGN;
    }

    ಯದಿ (region & 2)
        chunk = _malloc(&Zone_ಕೆಳಗಿನ, size, align);
    ಯದಿ (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, size, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        size, align, region, chunk);
    ಯದಿ (chunk) {
        regs->ds = FLATPTR_TO_SEG(chunk);
        regs->bx = FLATPTR_TO_OFFSET(chunk);
        regs->ax = 0;
    } ಅನ್ಯಥಾ {
        regs->ax = 1;
    }
}

ಶೂನ್ಯ VISIBLE32INIT
handle_csm(ಸಂರಚನಾ bregs *regs)
{
    ASSERT32FLAT();

    ಯದಿ (!CONFIG_CSM)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    code_mutable_preinit();
    pic_irqmask_write(PICMask);

    ವಿಕಲ್ಪ(regs->ax) {
    ಪಕ್ಷ 0000: handle_csm_0000(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0001: handle_csm_0001(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0002: handle_csm_0002(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0003: handle_csm_0003(regs); ಭಙ್ಗ;
//    case 0004: handle_csm_0004(regs); break;
    ಪಕ್ಷ 0005: handle_csm_0005(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0006: handle_csm_0006(regs); ಭಙ್ಗ;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    ಸ್ವಭಾವ: regs->al = 1;
    }

    csm_return(regs);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ csm_prio_to_seabios(u16 csm_prio)
{
    ವಿಕಲ್ಪ (csm_prio) {
    ಪಕ್ಷ BBS_DO_NOT_BOOT_FROM:
    ಪಕ್ಷ BBS_IGNORE_ENTRY:
        ಪ್ರತ್ಯಾಗಮ -1;

    ಪಕ್ಷ BBS_LOWEST_PRIORITY:
    ಪಕ್ಷ BBS_UNPRIORITIZED_ENTRY:
    ಸ್ವಭಾವ:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        ಪ್ರತ್ಯಾಗಮ csm_prio + 1;
    }
}

ಪೂರ್ಣಾಙ್ಕ csm_bootprio_ata(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ slave)
{
    ಯದಿ (!csm_boot_table)
        ಪ್ರತ್ಯಾಗಮ -1;
    BBS_TABLE *bbs = (ಶೂನ್ಯ *)csm_boot_table->BbsTable;
    ಪೂರ್ಣಾಙ್ಕ index = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            index, bbs[index].BootPriority);
    ಪ್ರತ್ಯಾಗಮ csm_prio_to_seabios(bbs[index].BootPriority);
}

ಪೂರ್ಣಾಙ್ಕ csm_bootprio_fdc(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ port, ಪೂರ್ಣಾಙ್ಕ fdid)
{
    ಯದಿ (!csm_boot_table)
        ಪ್ರತ್ಯಾಗಮ -1;
    BBS_TABLE *bbs = (ಶೂನ್ಯ *)csm_boot_table->BbsTable;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].BootPriority);
    ಪ್ರತ್ಯಾಗಮ csm_prio_to_seabios(bbs[0].BootPriority);
}

ಪೂರ್ಣಾಙ್ಕ csm_bootprio_pci(ಸಂರಚನಾ pci_device *pci)
{
    ಯದಿ (!csm_boot_table)
        ಪ್ರತ್ಯಾಗಮ -1;
    BBS_TABLE *bbs = (ಶೂನ್ಯ *)csm_boot_table->BbsTable;
    ಪೂರ್ಣಾಙ್ಕ i;

    ಪ್ರತಿ (i = 5; i < csm_boot_table->ಸಂಖ್ಯೆ_Bbs_Entries; i++) {
        ಯದಿ (pci->bdf == pci_to_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].BootPriority);
            ಪ್ರತ್ಯಾಗಮ csm_prio_to_seabios(bbs[i].BootPriority);
        }
    }
    ಪ್ರತ್ಯಾಗಮ -1;
}
