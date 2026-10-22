// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "e820map.h" // e820_add
#অন্তর্ভাব "farptr.h" // MAKE_FLATPTR
#অন্তর্ভাব "hw/pci.h" // pci_to_bdf
#অন্তর্ভাব "hw/pcidevice.h" // pci_probe_devices
#অন্তর্ভাব "hw/pic.h" // pic_irqmask_read
#অন্তর্ভাব "malloc.h" // malloc_csm_preinit
#অন্তর্ভাব "memmap.h" //প্রতীক
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "paravirt.h" // qemu_preinit
#অন্তর্ভাব "stacks.h" // wait_threads
#অন্তর্ভাব "std/acpi.h" // RSDP_SIGNATURE
#অন্তর্ভাব "std/bda.h" // struct bios_data_area_s
#অন্তর্ভাব "std/optionrom.h" // struct rom_header
#অন্তর্ভাব "util.h" // copy_smbios_21

#define UINT8 u8
#define UINT16 u16
#define UINT32 u32
#অন্তর্ভাব "std/LegacyBios.h"

সংরচনা rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_TABLE csm_compat_table VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .TableChecksum = 0 /* Filled in by checkrom.py */,
    .TableLength = পরিমাণ(csm_compat_table),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16CallOffset = 0 /* Filled in by checkrom.py */,
    .OemIdStringPointer = (u32)"SeaBIOS",
    .AcpiRsdPtrPointer = (u32)&csm_rsdp,
};

EFI_TO_COMPATIBILITY16_INIT_TABLE *csm_init_table;
EFI_TO_COMPATIBILITY16_BOOT_TABLE *csm_boot_table;

স্থির u16 PICMask = PIC_IRQMASK_DEFAULT;

বাহ্য শূন্য __csm_return(সংরচনা bregs *regs) __noreturn;

স্থির শূন্য
csm_return(সংরচনা bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_table.UmaAddress = rommax;
    csm_compat_table.UmaSize = প্রতীক(final_readonly_start) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

স্থির শূন্য
csm_maininit(সংরচনা bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_table.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_table.PnPInstallationCheckOffset = get_pnp_offset();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
স্থির শূন্য
handle_csm_0000(সংরচনা bregs *regs)
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
    dprintf(3, "NumE820Entries  %08x\n", csm_init_table->সংখ্যা_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_table->OsMemoryAbove1Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_table->ThunkStart);
    dprintf(3, "ThunkSize       %08x\n", csm_init_table->ThunkSizeInBytes);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_table->নিচের_Pmm_Memory);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_table->নিচের_Pmm_Memory_Size_In_Bytes);

    malloc_csm_preinit(csm_init_table->নিচের_Pmm_Memory,
                       csm_init_table->নিচের_Pmm_Memory_Size_In_Bytes,
                       csm_init_table->HiPmmMemory,
                       csm_init_table->HiPmmMemorySizeInBytes);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
স্থির শূন্য
handle_csm_0001(সংরচনা bregs *regs)
{
    যদি (!CONFIG_BOOT) {
        regs->ax = 1;
        প্রত্যাগম;
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
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_table->সংখ্যা_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_table->সংখ্যা_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_table->BbsTable);
    dprintf(3, "SmmTable %08x\n", csm_boot_table->SmmTable);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_table->OsMemoryAbove1Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_table->UnconventionalDeviceTable);

    regs->ax = 0;
}

/* PrepareToBoot */
স্থির শূন্য
handle_csm_0002(সংরচনা bregs *regs)
{
    যদি (!CONFIG_BOOT) {
        regs->ax = 1;
        প্রত্যাগম;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    সংরচনা e820entry *p = (শূন্য *)csm_compat_table.E820Pointer;
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i < csm_compat_table.E820Length / পরিমাণ(সংরচনা e820entry); i++)
        e820_add(p[i].start, p[i].size, p[i].type);

    যদি (csm_init_table->HiPmmMemorySizeInBytes > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = csm_init_table->HiPmmMemory + csm_init_table->HiPmmMemorySizeInBytes;
        e820_add(hi_pmm_end - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    যদি (csm_compat_table.IrqRoutingTablePointer &&
        csm_compat_table.IrqRoutingTableLength) {
        PirAddr = (শূন্য *)csm_compat_table.IrqRoutingTablePointer;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    যদি (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        find_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    যদি (csm_boot_table->SmbiosTable)
        copy_smbios_21((শূন্য *)csm_boot_table->SmbiosTable);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    সংরচনা bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
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
স্থির শূন্য
handle_csm_0003(সংরচনা bregs *regs)
{
    যদি (!CONFIG_BOOT) {
        regs->ax = 1;
        প্রত্যাগম;
    }

    dprintf(3, "Boot\n");

    startBoot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
স্থির শূন্য
handle_csm_0005(সংরচনা bregs *regs)
{
    EFI_DISPATCH_OPROM_TABLE *table = MAKE_FLATPTR(regs->es, regs->bx);
    সংরচনা rom_header *rom;
    u16 bdf;

    যদি (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        প্রত্যাগম;
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
স্থির শূন্য
handle_csm_0006(সংরচনা bregs *regs)
{
    u16 size = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    শূন্য *chunk = রিক্ত;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        size, align, region);

    যদি (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    যদি (align) {
        align = 1 << __ffs(align);
        যদি (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } অন্যথা {
        align = MALLOC_MIN_ALIGN;
    }

    যদি (region & 2)
        chunk = _malloc(&Zone_নিচের, size, align);
    যদি (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, size, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        size, align, region, chunk);
    যদি (chunk) {
        regs->ds = FLATPTR_TO_SEG(chunk);
        regs->bx = FLATPTR_TO_OFFSET(chunk);
        regs->ax = 0;
    } অন্যথা {
        regs->ax = 1;
    }
}

শূন্য VISIBLE32INIT
handle_csm(সংরচনা bregs *regs)
{
    ASSERT32FLAT();

    যদি (!CONFIG_CSM)
        প্রত্যাগম;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    code_mutable_preinit();
    pic_irqmask_write(PICMask);

    বিকল্প(regs->ax) {
    পক্ষ 0000: handle_csm_0000(regs); ভঙ্গ;
    পক্ষ 0001: handle_csm_0001(regs); ভঙ্গ;
    পক্ষ 0002: handle_csm_0002(regs); ভঙ্গ;
    পক্ষ 0003: handle_csm_0003(regs); ভঙ্গ;
//    case 0004: handle_csm_0004(regs); break;
    পক্ষ 0005: handle_csm_0005(regs); ভঙ্গ;
    পক্ষ 0006: handle_csm_0006(regs); ভঙ্গ;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    স্বভাব: regs->al = 1;
    }

    csm_return(regs);
}

স্থির পূর্ণাঙ্ক csm_prio_to_seabios(u16 csm_prio)
{
    বিকল্প (csm_prio) {
    পক্ষ BBS_DO_NOT_BOOT_FROM:
    পক্ষ BBS_IGNORE_ENTRY:
        প্রত্যাগম -1;

    পক্ষ BBS_LOWEST_PRIORITY:
    পক্ষ BBS_UNPRIORITIZED_ENTRY:
    স্বভাব:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        প্রত্যাগম csm_prio + 1;
    }
}

পূর্ণাঙ্ক csm_bootprio_ata(সংরচনা pci_device *pci, পূর্ণাঙ্ক chanid, পূর্ণাঙ্ক slave)
{
    যদি (!csm_boot_table)
        প্রত্যাগম -1;
    BBS_TABLE *bbs = (শূন্য *)csm_boot_table->BbsTable;
    পূর্ণাঙ্ক index = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            index, bbs[index].BootPriority);
    প্রত্যাগম csm_prio_to_seabios(bbs[index].BootPriority);
}

পূর্ণাঙ্ক csm_bootprio_fdc(সংরচনা pci_device *pci, পূর্ণাঙ্ক port, পূর্ণাঙ্ক fdid)
{
    যদি (!csm_boot_table)
        প্রত্যাগম -1;
    BBS_TABLE *bbs = (শূন্য *)csm_boot_table->BbsTable;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].BootPriority);
    প্রত্যাগম csm_prio_to_seabios(bbs[0].BootPriority);
}

পূর্ণাঙ্ক csm_bootprio_pci(সংরচনা pci_device *pci)
{
    যদি (!csm_boot_table)
        প্রত্যাগম -1;
    BBS_TABLE *bbs = (শূন্য *)csm_boot_table->BbsTable;
    পূর্ণাঙ্ক i;

    প্রতি (i = 5; i < csm_boot_table->সংখ্যা_Bbs_Entries; i++) {
        যদি (pci->bdf == pci_to_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].BootPriority);
            প্রত্যাগম csm_prio_to_seabios(bbs[i].BootPriority);
        }
    }
    প্রত্যাগম -1;
}
