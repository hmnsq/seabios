// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "e820map.h" // e820_add
#अन्तर्भाव "farptr.h" // MAKE_FLATPTR
#अन्तर्भाव "hw/pci.h" // pci_to_bdf
#अन्तर्भाव "hw/pcidevice.h" // pci_probe_devices
#अन्तर्भाव "hw/pic.h" // pic_irqmask_read
#अन्तर्भाव "malloc.h" // malloc_csm_preinit
#अन्तर्भाव "memmap.h" //चिह्नम्
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // qemu_preinit
#अन्तर्भाव "stacks.h" // wait_threads
#अन्तर्भाव "std/acpi.h" // RSDP_SIGNATURE
#अन्तर्भाव "std/bda.h" // struct bios_data_area_s
#अन्तर्भाव "std/optionrom.h" // struct rom_header
#अन्तर्भाव "util.h" // copy_smbios_21

#परिभाषय UINT8 u8
#परिभाषय UINT16 u16
#परिभाषय UINT32 u32
#अन्तर्भाव "std/LegacyBios.h"

संरचना rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_सारणी csm_compat_सारणी VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .सारणी_Checksum = 0 /* Filled in by checkrom.py */,
    .सारणी_दीर्घता = परिमाण(csm_compat_सारणी),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16_Call_विस्थापनम् = 0 /* Filled in by checkrom.py */,
    .OemIdStringPointer = (u32)"SeaBIOS",
    .AcpiRsdPtrPointer = (u32)&csm_rsdp,
};

EFI_गन्तव्यम्_COMPATIBILITY16_INIT_सारणी *csm_init_सारणी;
EFI_गन्तव्यम्_COMPATIBILITY16_BOOT_सारणी *csm_boot_सारणी;

स्थिर u16 PICMask = PIC_IRQMASK_DEFAULT;

बाह्य शून्य __csm_return(संरचना bregs *regs) __noreturn;

स्थिर शून्य
csm_return(संरचना bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_सारणी.Uma_स्थानाङ्कः = rommax;
    csm_compat_सारणी.Uma_आकार = चिह्नम्(final_readonly_start) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

स्थिर शून्य
csm_maininit(संरचना bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_सारणी.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_सारणी.Pn_PInstallation_Check_विस्थापनम् = get_pnp_विस्थापनम्();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
स्थिर शून्य
handle_csm_0000(संरचना bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_सारणी = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_सारणी->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_सारणी->Hi_Pmm_स्मृतिः);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_सारणी->Hi_Pmm_स्मृतिः_आकार_अन्तर्_अष्टकानि);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_सारणी->ReverseThunkCallSegment,
            csm_init_सारणी->Reverse_Thunk_Call_विस्थापनम्);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_सारणी->सङ्ख्या_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_सारणी->Os_स्मृतिः_Above1_Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_सारणी->ThunkStart);
    dprintf(3, "ThunkSize       %08x\n", csm_init_सारणी->Thunk_आकार_अन्तर्_अष्टकानि);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_सारणी->नीचः_Pmm_स्मृतिः);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_सारणी->नीचः_Pmm_स्मृतिः_आकार_अन्तर्_अष्टकानि);

    malloc_csm_preinit(csm_init_सारणी->नीचः_Pmm_स्मृतिः,
                       csm_init_सारणी->नीचः_Pmm_स्मृतिः_आकार_अन्तर्_अष्टकानि,
                       csm_init_सारणी->Hi_Pmm_स्मृतिः,
                       csm_init_सारणी->Hi_Pmm_स्मृतिः_आकार_अन्तर्_अष्टकानि);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
स्थिर शून्य
handle_csm_0001(संरचना bregs *regs)
{
    यदि (!CONFIG_BOOT) {
        regs->ax = 1;
        प्रत्यागम;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_सारणी = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_सारणी->Major_संस्करणम्);
    dprintf(3, "MinorVersion %04x\n", csm_boot_सारणी->Minor_संस्करणम्);
    dprintf(3, "AcpiTable %08x\n", csm_boot_सारणी->Acpi_सारणी);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_सारणी->Smbios_सारणी);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_सारणी->Smbios_सारणी_दीर्घता);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_सारणी->Device_मार्गः_Type);
    dprintf(3, "PciIrqMask %04x\n", csm_boot_सारणी->Pci_Irq_आवरणम्);
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_सारणी->सङ्ख्या_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_सारणी->सङ्ख्या_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_सारणी->Bbs_सारणी);
    dprintf(3, "SmmTable %08x\n", csm_boot_सारणी->Smm_सारणी);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_सारणी->Os_स्मृतिः_Above1_Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_सारणी->Unconventional_Device_सारणी);

    regs->ax = 0;
}

/* PrepareToBoot */
स्थिर शून्य
handle_csm_0002(संरचना bregs *regs)
{
    यदि (!CONFIG_BOOT) {
        regs->ax = 1;
        प्रत्यागम;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    संरचना e820entry *p = (शून्य *)csm_compat_सारणी.E820Pointer;
    पूर्णाङ्क i;
    प्रति (i=0; i < csm_compat_सारणी.E820_दीर्घता / परिमाण(संरचना e820entry); i++)
        e820_सङ्कलय(p[i].start, p[i].आकार, p[i].type);

    यदि (csm_init_सारणी->Hi_Pmm_स्मृतिः_आकार_अन्तर्_अष्टकानि > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_अन्तः = csm_init_सारणी->Hi_Pmm_स्मृतिः + csm_init_सारणी->Hi_Pmm_स्मृतिः_आकार_अन्तर्_अष्टकानि;
        e820_सङ्कलय(hi_pmm_अन्तः - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    यदि (csm_compat_सारणी.Irq_Routing_सारणी_Pointer &&
        csm_compat_सारणी.Irq_Routing_सारणी_दीर्घता) {
        PirAddr = (शून्य *)csm_compat_सारणी.Irq_Routing_सारणी_Pointer;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    यदि (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        find_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    यदि (csm_boot_सारणी->Smbios_सारणी)
        copy_smbios_21((शून्य *)csm_boot_सारणी->Smbios_सारणी);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    संरचना bios_दत्त_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    bda->hdcount = 0;

    thread_सज्जता();
    mathcp_सज्जता();
    timer_सज्जता();
    clock_सज्जता();
    device_hardware_सज्जता();
    wait_तन्तवः();
    interactive_bootmenu();

    prepareboot();

    regs->ax = 0;
}

/* Boot */
स्थिर शून्य
handle_csm_0003(संरचना bregs *regs)
{
    यदि (!CONFIG_BOOT) {
        regs->ax = 1;
        प्रत्यागम;
    }

    dprintf(3, "Boot\n");

    startBoot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
स्थिर शून्य
handle_csm_0005(संरचना bregs *regs)
{
    EFI_DISPATCH_OPROM_सारणी *सारणी = MAKE_FLATPTR(regs->es, regs->bx);
    संरचना rom_शीर्षकम् *rom;
    u16 bdf;

    यदि (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        प्रत्यागम;
    }

    dprintf(3, "Legacy16DispatchOprom rom %p\n", सारणी);

    dprintf(3, "OpromSegment   %04x\n", सारणी->OpromSegment);
    dprintf(3, "RuntimeSegment %04x\n", सारणी->RuntimeSegment);
    dprintf(3, "PnPInstallationCheck %04x:%04x\n",
            सारणी->PnPInstallationCheckSegment,
            सारणी->Pn_PInstallation_Check_विस्थापनम्);
    dprintf(3, "RuntimeSegment %04x\n", सारणी->RuntimeSegment);

    rom = MAKE_FLATPTR(सारणी->OpromSegment, 0);
    bdf = pci_bus_devfn_गन्तव्यम्_bdf(सारणी->PciBus, सारणी->PciDeviceFunction);

    rom_reserve(rom->आकार * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->आकार * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
स्थिर शून्य
handle_csm_0006(संरचना bregs *regs)
{
    u16 आकार = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    शून्य *chunk = रिक्त;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        आकार, align, region);

    यदि (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    यदि (align) {
        align = 1 << __ffs(align);
        यदि (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } अन्यथा {
        align = MALLOC_MIN_ALIGN;
    }

    यदि (region & 2)
        chunk = _malloc(&Zone_नीचः, आकार, align);
    यदि (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, आकार, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        आकार, align, region, chunk);
    यदि (chunk) {
        regs->ds = FLATPTR_गन्तव्यम्_SEG(chunk);
        regs->bx = FLATPTR_गन्तव्यम्_विस्थापनम्(chunk);
        regs->ax = 0;
    } अन्यथा {
        regs->ax = 1;
    }
}

शून्य VISIBLE32INIT
handle_csm(संरचना bregs *regs)
{
    ASSERT32FLAT();

    यदि (!CONFIG_CSM)
        प्रत्यागम;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    संहिता_mutable_preinit();
    pic_irqmask_write(PICMask);

    विकल्प(regs->ax) {
    पक्ष 0000: handle_csm_0000(regs); भङ्ग;
    पक्ष 0001: handle_csm_0001(regs); भङ्ग;
    पक्ष 0002: handle_csm_0002(regs); भङ्ग;
    पक्ष 0003: handle_csm_0003(regs); भङ्ग;
//    case 0004: handle_csm_0004(regs); break;
    पक्ष 0005: handle_csm_0005(regs); भङ्ग;
    पक्ष 0006: handle_csm_0006(regs); भङ्ग;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    स्वभाव: regs->al = 1;
    }

    csm_return(regs);
}

स्थिर पूर्णाङ्क csm_prio_गन्तव्यम्_seabios(u16 csm_prio)
{
    विकल्प (csm_prio) {
    पक्ष BBS_DO_NOT_BOOT_FROM:
    पक्ष BBS_IGNORE_प्रविष्टिः:
        प्रत्यागम -1;

    पक्ष BBS_निम्नतमम्_प्राथमिकता:
    पक्ष BBS_UNPRIORITIZED_प्रविष्टिः:
    स्वभाव:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        प्रत्यागम csm_prio + 1;
    }
}

पूर्णाङ्क csm_bootprio_ata(संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क slave)
{
    यदि (!csm_boot_सारणी)
        प्रत्यागम -1;
    BBS_सारणी *bbs = (शून्य *)csm_boot_सारणी->Bbs_सारणी;
    पूर्णाङ्क सूचकांक = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            सूचकांक, bbs[सूचकांक].Boot_प्राथमिकता);
    प्रत्यागम csm_prio_गन्तव्यम्_seabios(bbs[सूचकांक].Boot_प्राथमिकता);
}

पूर्णाङ्क csm_bootprio_fdc(संरचना pci_device *pci, पूर्णाङ्क पत्तनम्, पूर्णाङ्क fdid)
{
    यदि (!csm_boot_सारणी)
        प्रत्यागम -1;
    BBS_सारणी *bbs = (शून्य *)csm_boot_सारणी->Bbs_सारणी;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].Boot_प्राथमिकता);
    प्रत्यागम csm_prio_गन्तव्यम्_seabios(bbs[0].Boot_प्राथमिकता);
}

पूर्णाङ्क csm_bootprio_pci(संरचना pci_device *pci)
{
    यदि (!csm_boot_सारणी)
        प्रत्यागम -1;
    BBS_सारणी *bbs = (शून्य *)csm_boot_सारणी->Bbs_सारणी;
    पूर्णाङ्क i;

    प्रति (i = 5; i < csm_boot_सारणी->सङ्ख्या_Bbs_Entries; i++) {
        यदि (pci->bdf == pci_गन्तव्यम्_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].Boot_प्राथमिकता);
            प्रत्यागम csm_prio_गन्तव्यम्_seabios(bbs[i].Boot_प्राथमिकता);
        }
    }
    प्रत्यागम -1;
}
