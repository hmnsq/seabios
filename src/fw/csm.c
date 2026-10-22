// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "bregs.h" // struct bregs
#समावेश "config.h" // CONFIG_*
#समावेश "e820map.h" // e820_add
#समावेश "farptr.h" // MAKE_FLATPTR
#समावेश "hw/pci.h" // pci_to_bdf
#समावेश "hw/pcidevice.h" // pci_probe_devices
#समावेश "hw/pic.h" // pic_irqmask_read
#समावेश "malloc.h" // malloc_csm_preinit
#समावेश "memmap.h" //प्रतीक
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // qemu_preinit
#समावेश "stacks.h" // wait_threads
#समावेश "std/acpi.h" // RSDP_SIGNATURE
#समावेश "std/bda.h" // struct bios_data_area_s
#समावेश "std/optionrom.h" // struct rom_header
#समावेश "util.h" // copy_smbios_21

#घोषणा UINT8 u8
#घोषणा UINT16 u16
#घोषणा UINT32 u32
#समावेश "std/LegacyBios.h"

काष्ठा rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_TABLE csm_compat_table VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .TableChecksum = 0 /* Filled in by checkrom.py */,
    .Table_लंबाई = माप(csm_compat_table),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16CallOffset = 0 /* Filled in by checkrom.py */,
    .OemIdStringPointer = (u32)"SeaBIOS",
    .AcpiRsdPtrPointer = (u32)&csm_rsdp,
};

EFI_TO_COMPATIBILITY16_INIT_TABLE *csm_init_table;
EFI_TO_COMPATIBILITY16_BOOT_TABLE *csm_boot_table;

अटल u16 PICMask = PIC_IRQMASK_DEFAULT;

बाह्य व्योम __csm_return(काष्ठा bregs *regs) __noreturn;

अटल व्योम
csm_return(काष्ठा bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_table.UmaAddress = rommax;
    csm_compat_table.Uma_आकार = प्रतीक(final_readonly_start) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

अटल व्योम
csm_maininit(काष्ठा bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_table.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_table.PnPInstallationCheckOffset = get_pnp_offset();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
अटल व्योम
handle_csm_0000(काष्ठा bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_table = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_table->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_table->HiPmmMemory);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_table->Hi_Pmm_Memory_आकार_In_Bytes);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_table->ReverseThunkCallSegment,
            csm_init_table->ReverseThunkCallOffset);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_table->संख्या_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_table->OsMemoryAbove1Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_table->ThunkStart);
    dprintf(3, "ThunkSize       %08x\n", csm_init_table->Thunk_आकार_In_Bytes);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_table->निचला_Pmm_Memory);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_table->निचला_Pmm_Memory_आकार_In_Bytes);

    malloc_csm_preinit(csm_init_table->निचला_Pmm_Memory,
                       csm_init_table->निचला_Pmm_Memory_आकार_In_Bytes,
                       csm_init_table->HiPmmMemory,
                       csm_init_table->Hi_Pmm_Memory_आकार_In_Bytes);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
अटल व्योम
handle_csm_0001(काष्ठा bregs *regs)
{
    अगर (!CONFIG_BOOT) {
        regs->ax = 1;
        वापस;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_table = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_table->MajorVersion);
    dprintf(3, "MinorVersion %04x\n", csm_boot_table->MinorVersion);
    dprintf(3, "AcpiTable %08x\n", csm_boot_table->AcpiTable);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_table->SmbiosTable);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_table->Smbios_Table_लंबाई);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_table->DevicePathType);
    dprintf(3, "PciIrqMask %04x\n", csm_boot_table->PciIrqMask);
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_table->संख्या_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_table->संख्या_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_table->BbsTable);
    dprintf(3, "SmmTable %08x\n", csm_boot_table->SmmTable);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_table->OsMemoryAbove1Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_table->UnconventionalDeviceTable);

    regs->ax = 0;
}

/* PrepareToBoot */
अटल व्योम
handle_csm_0002(काष्ठा bregs *regs)
{
    अगर (!CONFIG_BOOT) {
        regs->ax = 1;
        वापस;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    काष्ठा e820entry *p = (व्योम *)csm_compat_table.E820Pointer;
    पूर्णांक i;
    क्रम (i=0; i < csm_compat_table.E820_लंबाई / माप(काष्ठा e820entry); i++)
        e820_add(p[i].start, p[i].आकार, p[i].type);

    अगर (csm_init_table->Hi_Pmm_Memory_आकार_In_Bytes > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = csm_init_table->HiPmmMemory + csm_init_table->Hi_Pmm_Memory_आकार_In_Bytes;
        e820_add(hi_pmm_end - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    अगर (csm_compat_table.IrqRoutingTablePointer &&
        csm_compat_table.Irq_Routing_Table_लंबाई) {
        PirAddr = (व्योम *)csm_compat_table.IrqRoutingTablePointer;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    अगर (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        find_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    अगर (csm_boot_table->SmbiosTable)
        copy_smbios_21((व्योम *)csm_boot_table->SmbiosTable);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    काष्ठा bios_तथ्य_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
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
अटल व्योम
handle_csm_0003(काष्ठा bregs *regs)
{
    अगर (!CONFIG_BOOT) {
        regs->ax = 1;
        वापस;
    }

    dprintf(3, "Boot\n");

    startBoot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
अटल व्योम
handle_csm_0005(काष्ठा bregs *regs)
{
    EFI_DISPATCH_OPROM_TABLE *table = MAKE_FLATPTR(regs->es, regs->bx);
    काष्ठा rom_header *rom;
    u16 bdf;

    अगर (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        वापस;
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

    rom_reserve(rom->आकार * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->आकार * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
अटल व्योम
handle_csm_0006(काष्ठा bregs *regs)
{
    u16 आकार = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    व्योम *chunk = शून्य;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        आकार, align, region);

    अगर (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    अगर (align) {
        align = 1 << __ffs(align);
        अगर (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } अन्यथा {
        align = MALLOC_MIN_ALIGN;
    }

    अगर (region & 2)
        chunk = _malloc(&Zone_निचला, आकार, align);
    अगर (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, आकार, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        आकार, align, region, chunk);
    अगर (chunk) {
        regs->ds = FLATPTR_TO_SEG(chunk);
        regs->bx = FLATPTR_TO_OFFSET(chunk);
        regs->ax = 0;
    } अन्यथा {
        regs->ax = 1;
    }
}

व्योम VISIBLE32INIT
handle_csm(काष्ठा bregs *regs)
{
    ASSERT32FLAT();

    अगर (!CONFIG_CSM)
        वापस;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    code_mutable_preinit();
    pic_irqmask_write(PICMask);

    चयन(regs->ax) {
    हाल 0000: handle_csm_0000(regs); अवरोध;
    हाल 0001: handle_csm_0001(regs); अवरोध;
    हाल 0002: handle_csm_0002(regs); अवरोध;
    हाल 0003: handle_csm_0003(regs); अवरोध;
//    case 0004: handle_csm_0004(regs); break;
    हाल 0005: handle_csm_0005(regs); अवरोध;
    हाल 0006: handle_csm_0006(regs); अवरोध;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    शेष: regs->al = 1;
    }

    csm_return(regs);
}

अटल पूर्णांक csm_prio_to_seabios(u16 csm_prio)
{
    चयन (csm_prio) {
    हाल BBS_DO_NOT_BOOT_FROM:
    हाल BBS_IGNORE_ENTRY:
        वापस -1;

    हाल BBS_LOWEST_PRIORITY:
    हाल BBS_UNPRIORITIZED_ENTRY:
    शेष:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        वापस csm_prio + 1;
    }
}

पूर्णांक csm_bootprio_ata(काष्ठा pci_device *pci, पूर्णांक chanid, पूर्णांक slave)
{
    अगर (!csm_boot_table)
        वापस -1;
    BBS_TABLE *bbs = (व्योम *)csm_boot_table->BbsTable;
    पूर्णांक सूचकांक = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            सूचकांक, bbs[सूचकांक].BootPriority);
    वापस csm_prio_to_seabios(bbs[सूचकांक].BootPriority);
}

पूर्णांक csm_bootprio_fdc(काष्ठा pci_device *pci, पूर्णांक port, पूर्णांक fdid)
{
    अगर (!csm_boot_table)
        वापस -1;
    BBS_TABLE *bbs = (व्योम *)csm_boot_table->BbsTable;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].BootPriority);
    वापस csm_prio_to_seabios(bbs[0].BootPriority);
}

पूर्णांक csm_bootprio_pci(काष्ठा pci_device *pci)
{
    अगर (!csm_boot_table)
        वापस -1;
    BBS_TABLE *bbs = (व्योम *)csm_boot_table->BbsTable;
    पूर्णांक i;

    क्रम (i = 5; i < csm_boot_table->संख्या_Bbs_Entries; i++) {
        अगर (pci->bdf == pci_to_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].BootPriority);
            वापस csm_prio_to_seabios(bbs[i].BootPriority);
        }
    }
    वापस -1;
}
