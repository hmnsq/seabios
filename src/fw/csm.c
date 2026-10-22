// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "e820map.h" // e820_add
#شامل "farptr.h" // MAKE_FLATPTR
#شامل "hw/pci.h" // pci_to_bdf
#شامل "hw/pcidevice.h" // pci_probe_devices
#شامل "hw/pic.h" // pic_irqmask_read
#شامل "malloc.h" // malloc_csm_preinit
#شامل "memmap.h" //نماد
#شامل "output.h" // dprintf
#شامل "paravirt.h" // qemu_preinit
#شامل "stacks.h" // wait_threads
#شامل "std/acpi.h" // RSDP_SIGNATURE
#شامل "std/bda.h" // struct bios_data_area_s
#شامل "std/optionrom.h" // struct rom_header
#شامل "util.h" // copy_smbios_21

#define UINT8 u8
#define UINT16 u16
#define UINT32 u32
#شامل "std/LegacyBios.h"

ساختار rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_TABLE csm_compat_table VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .TableChecksum = 0 /* Filled in by checkrom.py */,
    .TableLength = اندازه(csm_compat_table),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16CallOffset = 0 /* Filled in by checkrom.py */,
    .OemIdStringPointer = (u32)"SeaBIOS",
    .AcpiRsdPtrPointer = (u32)&csm_rsdp,
};

EFI_TO_COMPATIBILITY16_INIT_TABLE *csm_init_table;
EFI_TO_COMPATIBILITY16_BOOT_TABLE *csm_boot_table;

ایستا u16 PICMask = PIC_IRQMASK_DEFAULT;

بیرونی تهی __csm_return(ساختار bregs *regs) __noreturn;

ایستا تهی
csm_return(ساختار bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_table.UmaAddress = rommax;
    csm_compat_table.UmaSize = نماد(final_readonly_start) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

ایستا تهی
csm_maininit(ساختار bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_table.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_table.PnPInstallationCheckOffset = get_pnp_offset();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
ایستا تهی
handle_csm_0000(ساختار bregs *regs)
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
    dprintf(3, "NumE820Entries  %08x\n", csm_init_table->عدد_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_table->OsMemoryAbove1Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_table->ThunkStart);
    dprintf(3, "ThunkSize       %08x\n", csm_init_table->ThunkSizeInBytes);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_table->کم_Pmm_Memory);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_table->کم_Pmm_Memory_Size_In_Bytes);

    malloc_csm_preinit(csm_init_table->کم_Pmm_Memory,
                       csm_init_table->کم_Pmm_Memory_Size_In_Bytes,
                       csm_init_table->HiPmmMemory,
                       csm_init_table->HiPmmMemorySizeInBytes);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
ایستا تهی
handle_csm_0001(ساختار bregs *regs)
{
    اگر (!CONFIG_BOOT) {
        regs->ax = 1;
        بازگشت;
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
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_table->عدد_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_table->عدد_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_table->BbsTable);
    dprintf(3, "SmmTable %08x\n", csm_boot_table->SmmTable);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_table->OsMemoryAbove1Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_table->UnconventionalDeviceTable);

    regs->ax = 0;
}

/* PrepareToBoot */
ایستا تهی
handle_csm_0002(ساختار bregs *regs)
{
    اگر (!CONFIG_BOOT) {
        regs->ax = 1;
        بازگشت;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    ساختار e820entry *p = (تهی *)csm_compat_table.E820Pointer;
    صحیح i;
    برای (i=0; i < csm_compat_table.E820Length / اندازه(ساختار e820entry); i++)
        e820_add(p[i].start, p[i].size, p[i].type);

    اگر (csm_init_table->HiPmmMemorySizeInBytes > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = csm_init_table->HiPmmMemory + csm_init_table->HiPmmMemorySizeInBytes;
        e820_add(hi_pmm_end - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    اگر (csm_compat_table.IrqRoutingTablePointer &&
        csm_compat_table.IrqRoutingTableLength) {
        PirAddr = (تهی *)csm_compat_table.IrqRoutingTablePointer;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    اگر (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        find_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    اگر (csm_boot_table->SmbiosTable)
        copy_smbios_21((تهی *)csm_boot_table->SmbiosTable);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    ساختار bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
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
ایستا تهی
handle_csm_0003(ساختار bregs *regs)
{
    اگر (!CONFIG_BOOT) {
        regs->ax = 1;
        بازگشت;
    }

    dprintf(3, "Boot\n");

    startBoot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
ایستا تهی
handle_csm_0005(ساختار bregs *regs)
{
    EFI_DISPATCH_OPROM_TABLE *table = MAKE_FLATPTR(regs->es, regs->bx);
    ساختار rom_header *rom;
    u16 bdf;

    اگر (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        بازگشت;
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
ایستا تهی
handle_csm_0006(ساختار bregs *regs)
{
    u16 size = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    تهی *chunk = پوچ;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        size, align, region);

    اگر (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    اگر (align) {
        align = 1 << __ffs(align);
        اگر (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } وگرنه {
        align = MALLOC_MIN_ALIGN;
    }

    اگر (region & 2)
        chunk = _malloc(&Zone_کم, size, align);
    اگر (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, size, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        size, align, region, chunk);
    اگر (chunk) {
        regs->ds = FLATPTR_TO_SEG(chunk);
        regs->bx = FLATPTR_TO_OFFSET(chunk);
        regs->ax = 0;
    } وگرنه {
        regs->ax = 1;
    }
}

تهی VISIBLE32INIT
handle_csm(ساختار bregs *regs)
{
    ASSERT32FLAT();

    اگر (!CONFIG_CSM)
        بازگشت;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    code_mutable_preinit();
    pic_irqmask_write(PICMask);

    انتخاب(regs->ax) {
    حالت 0000: handle_csm_0000(regs); بشکن;
    حالت 0001: handle_csm_0001(regs); بشکن;
    حالت 0002: handle_csm_0002(regs); بشکن;
    حالت 0003: handle_csm_0003(regs); بشکن;
//    case 0004: handle_csm_0004(regs); break;
    حالت 0005: handle_csm_0005(regs); بشکن;
    حالت 0006: handle_csm_0006(regs); بشکن;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    پیشفرض: regs->al = 1;
    }

    csm_return(regs);
}

ایستا صحیح csm_prio_to_seabios(u16 csm_prio)
{
    انتخاب (csm_prio) {
    حالت BBS_DO_NOT_BOOT_FROM:
    حالت BBS_IGNORE_ENTRY:
        بازگشت -1;

    حالت BBS_LOWEST_PRIORITY:
    حالت BBS_UNPRIORITIZED_ENTRY:
    پیشفرض:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        بازگشت csm_prio + 1;
    }
}

صحیح csm_bootprio_ata(ساختار pci_device *pci, صحیح chanid, صحیح slave)
{
    اگر (!csm_boot_table)
        بازگشت -1;
    BBS_TABLE *bbs = (تهی *)csm_boot_table->BbsTable;
    صحیح index = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            index, bbs[index].BootPriority);
    بازگشت csm_prio_to_seabios(bbs[index].BootPriority);
}

صحیح csm_bootprio_fdc(ساختار pci_device *pci, صحیح port, صحیح fdid)
{
    اگر (!csm_boot_table)
        بازگشت -1;
    BBS_TABLE *bbs = (تهی *)csm_boot_table->BbsTable;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].BootPriority);
    بازگشت csm_prio_to_seabios(bbs[0].BootPriority);
}

صحیح csm_bootprio_pci(ساختار pci_device *pci)
{
    اگر (!csm_boot_table)
        بازگشت -1;
    BBS_TABLE *bbs = (تهی *)csm_boot_table->BbsTable;
    صحیح i;

    برای (i = 5; i < csm_boot_table->عدد_Bbs_Entries; i++) {
        اگر (pci->bdf == pci_to_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].BootPriority);
            بازگشت csm_prio_to_seabios(bbs[i].BootPriority);
        }
    }
    بازگشت -1;
}
