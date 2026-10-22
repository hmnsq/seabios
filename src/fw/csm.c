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

#ವ್ಯಾಖ್ಯಾನಿಸು UINT8 u8
#ವ್ಯಾಖ್ಯಾನಿಸು UINT16 u16
#ವ್ಯಾಖ್ಯಾನಿಸು UINT32 u32
#ಅನ್ತರ್ಭಾವ "std/LegacyBios.h"

ರಚನೆ rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_ಕೋಷ್ಟಕ csm_compat_ಕೋಷ್ಟಕ VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .ಕೋಷ್ಟಕ_Checksum = 0 /* Filled in by checkrom.py */,
    .ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ = ಗಾತ್ರ(csm_compat_ಕೋಷ್ಟಕ),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16_Call_ಸ್ಥಾನಾಂತರ = 0 /* Filled in by checkrom.py */,
    .Oem_Id_String_ಸೂಚಕ = (u32)"SeaBIOS",
    .Acpi_Rsd_Ptr_ಸೂಚಕ = (u32)&csm_rsdp,
};

EFI_ಗಮ್ಯ_COMPATIBILITY16_INIT_ಕೋಷ್ಟಕ *csm_init_ಕೋಷ್ಟಕ;
EFI_ಗಮ್ಯ_COMPATIBILITY16_BOOT_ಕೋಷ್ಟಕ *csm_boot_ಕೋಷ್ಟಕ;

ಸ್ಥಿರ u16 PICMask = PIC_IRQMASK_DEFAULT;

ಬಾಹ್ಯ ಶೂನ್ಯ __csm_return(ರಚನೆ bregs *regs) __noreturn;

ಸ್ಥಿರ ಶೂನ್ಯ
csm_return(ರಚನೆ bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_ಕೋಷ್ಟಕ.Uma_ವಿಳಾಸ = rommax;
    csm_compat_ಕೋಷ್ಟಕ.Uma_ಗಾತ್ರ = ಚಿಹ್ನೆ(final_readonly_ಆರಂಭ) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
csm_maininit(ರಚನೆ bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_ಕೋಷ್ಟಕ.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_ಕೋಷ್ಟಕ.Pn_PInstallation_Check_ಸ್ಥಾನಾಂತರ = get_pnp_ಸ್ಥಾನಾಂತರ();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0000(ರಚನೆ bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_ಕೋಷ್ಟಕ = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_ಕೋಷ್ಟಕ->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_ಕೋಷ್ಟಕ->Hi_Pmm_ಸ್ಮರಣೆ);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_ಕೋಷ್ಟಕ->Hi_Pmm_ಸ್ಮರಣೆ_ಗಾತ್ರ_ಒಳ_ಬೈಟುಗಳು);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_ಕೋಷ್ಟಕ->ಹಿಮ್ಮುಖ_Thunk_Call_Segment,
            csm_init_ಕೋಷ್ಟಕ->ಹಿಮ್ಮುಖ_Thunk_Call_ಸ್ಥಾನಾಂತರ);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_ಕೋಷ್ಟಕ->ಸಂಖ್ಯೆ_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_ಕೋಷ್ಟಕ->Os_ಸ್ಮರಣೆ_Above1_Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_ಕೋಷ್ಟಕ->Thunk_ಆರಂಭ);
    dprintf(3, "ThunkSize       %08x\n", csm_init_ಕೋಷ್ಟಕ->Thunk_ಗಾತ್ರ_ಒಳ_ಬೈಟುಗಳು);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_ಕೋಷ್ಟಕ->ಕೆಳಗಿನ_Pmm_ಸ್ಮರಣೆ);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_ಕೋಷ್ಟಕ->ಕೆಳಗಿನ_Pmm_ಸ್ಮರಣೆ_ಗಾತ್ರ_ಒಳ_ಬೈಟುಗಳು);

    malloc_csm_preinit(csm_init_ಕೋಷ್ಟಕ->ಕೆಳಗಿನ_Pmm_ಸ್ಮರಣೆ,
                       csm_init_ಕೋಷ್ಟಕ->ಕೆಳಗಿನ_Pmm_ಸ್ಮರಣೆ_ಗಾತ್ರ_ಒಳ_ಬೈಟುಗಳು,
                       csm_init_ಕೋಷ್ಟಕ->Hi_Pmm_ಸ್ಮರಣೆ,
                       csm_init_ಕೋಷ್ಟಕ->Hi_Pmm_ಸ್ಮರಣೆ_ಗಾತ್ರ_ಒಳ_ಬೈಟುಗಳು);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0001(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_BOOT) {
        regs->ax = 1;
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_ಕೋಷ್ಟಕ = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_ಕೋಷ್ಟಕ->Major_ಆವೃತ್ತಿ);
    dprintf(3, "MinorVersion %04x\n", csm_boot_ಕೋಷ್ಟಕ->Minor_ಆವೃತ್ತಿ);
    dprintf(3, "AcpiTable %08x\n", csm_boot_ಕೋಷ್ಟಕ->Acpi_ಕೋಷ್ಟಕ);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_ಕೋಷ್ಟಕ->Smbios_ಕೋಷ್ಟಕ);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_ಕೋಷ್ಟಕ->Smbios_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_ಕೋಷ್ಟಕ->Device_ಮಾರ್ಗ_Type);
    dprintf(3, "PciIrqMask %04x\n", csm_boot_ಕೋಷ್ಟಕ->Pci_Irq_ಮುಖವಾಡ);
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_ಕೋಷ್ಟಕ->ಸಂಖ್ಯೆ_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_ಕೋಷ್ಟಕ->ಸಂಖ್ಯೆ_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_ಕೋಷ್ಟಕ->Bbs_ಕೋಷ್ಟಕ);
    dprintf(3, "SmmTable %08x\n", csm_boot_ಕೋಷ್ಟಕ->Smm_ಕೋಷ್ಟಕ);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_ಕೋಷ್ಟಕ->Os_ಸ್ಮರಣೆ_Above1_Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_ಕೋಷ್ಟಕ->Unconventional_Device_ಕೋಷ್ಟಕ);

    regs->ax = 0;
}

/* PrepareToBoot */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0002(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_BOOT) {
        regs->ax = 1;
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    ರಚನೆ e820entry *p = (ಶೂನ್ಯ *)csm_compat_ಕೋಷ್ಟಕ.E820_ಸೂಚಕ;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i < csm_compat_ಕೋಷ್ಟಕ.E820_ಉದ್ದಳತೆ / ಗಾತ್ರ(ರಚನೆ e820entry); i++)
        e820_ಕೂಡಿಸು(p[i].ಆರಂಭ, p[i].ಗಾತ್ರ_2, p[i].type);

    ಆದರೆ (csm_init_ಕೋಷ್ಟಕ->Hi_Pmm_ಸ್ಮರಣೆ_ಗಾತ್ರ_ಒಳ_ಬೈಟುಗಳು > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_ಕೊನೆ = csm_init_ಕೋಷ್ಟಕ->Hi_Pmm_ಸ್ಮರಣೆ + csm_init_ಕೋಷ್ಟಕ->Hi_Pmm_ಸ್ಮರಣೆ_ಗಾತ್ರ_ಒಳ_ಬೈಟುಗಳು;
        e820_ಕೂಡಿಸು(hi_pmm_ಕೊನೆ - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    ಆದರೆ (csm_compat_ಕೋಷ್ಟಕ.Irq_Routing_ಕೋಷ್ಟಕ_ಸೂಚಕ &&
        csm_compat_ಕೋಷ್ಟಕ.Irq_Routing_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ) {
        PirAddr = (ಶೂನ್ಯ *)csm_compat_ಕೋಷ್ಟಕ.Irq_Routing_ಕೋಷ್ಟಕ_ಸೂಚಕ;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    ಆದರೆ (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        ಕಂಡುಹಿಡಿ_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    ಆದರೆ (csm_boot_ಕೋಷ್ಟಕ->Smbios_ಕೋಷ್ಟಕ)
        copy_smbios_21((ಶೂನ್ಯ *)csm_boot_ಕೋಷ್ಟಕ->Smbios_ಕೋಷ್ಟಕ);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    ರಚನೆ bios_ದತ್ತಾಂಶ_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    bda->hdcount = 0;

    thread_ಸಿದ್ಧತೆ();
    mathcp_ಸಿದ್ಧತೆ();
    timer_ಸಿದ್ಧತೆ();
    clock_ಸಿದ್ಧತೆ();
    device_hardware_ಸಿದ್ಧತೆ();
    wait_ಎಳೆಗಳು();
    interactive_bootmenu();

    prepareboot();

    regs->ax = 0;
}

/* Boot */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0003(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_BOOT) {
        regs->ax = 1;
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(3, "Boot\n");

    ಆರಂಭ_Boot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0005(ರಚನೆ bregs *regs)
{
    EFI_DISPATCH_OPROM_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ = MAKE_FLATPTR(regs->es, regs->bx);
    ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom;
    u16 bdf;

    ಆದರೆ (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(3, "Legacy16DispatchOprom rom %p\n", ಕೋಷ್ಟಕ);

    dprintf(3, "OpromSegment   %04x\n", ಕೋಷ್ಟಕ->OpromSegment);
    dprintf(3, "RuntimeSegment %04x\n", ಕೋಷ್ಟಕ->RuntimeSegment);
    dprintf(3, "PnPInstallationCheck %04x:%04x\n",
            ಕೋಷ್ಟಕ->PnPInstallationCheckSegment,
            ಕೋಷ್ಟಕ->Pn_PInstallation_Check_ಸ್ಥಾನಾಂತರ);
    dprintf(3, "RuntimeSegment %04x\n", ಕೋಷ್ಟಕ->RuntimeSegment);

    rom = MAKE_FLATPTR(ಕೋಷ್ಟಕ->OpromSegment, 0);
    bdf = pci_bus_devfn_ಗಮ್ಯ_bdf(ಕೋಷ್ಟಕ->PciBus, ಕೋಷ್ಟಕ->PciDeviceFunction);

    rom_reserve(rom->ಗಾತ್ರ_2 * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->ಗಾತ್ರ_2 * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
ಸ್ಥಿರ ಶೂನ್ಯ
handle_csm_0006(ರಚನೆ bregs *regs)
{
    u16 ಗಾತ್ರ_2 = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    ಶೂನ್ಯ *chunk = ಖಾಲಿ;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        ಗಾತ್ರ_2, align, region);

    ಆದರೆ (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    ಆದರೆ (align) {
        align = 1 << __ffs(align);
        ಆದರೆ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } ಇಲ್ಲದಿದ್ದರೆ {
        align = MALLOC_MIN_ALIGN;
    }

    ಆದರೆ (region & 2)
        chunk = _malloc(&Zone_ಕೆಳಗಿನ, ಗಾತ್ರ_2, align);
    ಆದರೆ (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, ಗಾತ್ರ_2, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        ಗಾತ್ರ_2, align, region, chunk);
    ಆದರೆ (chunk) {
        regs->ds = FLATPTR_ಗಮ್ಯ_SEG(chunk);
        regs->bx = FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(chunk);
        regs->ax = 0;
    } ಇಲ್ಲದಿದ್ದರೆ {
        regs->ax = 1;
    }
}

ಶೂನ್ಯ VISIBLE32INIT
handle_csm(ರಚನೆ bregs *regs)
{
    ASSERT32FLAT();

    ಆದರೆ (!CONFIG_CSM)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    ಸಂಕೇತ_mutable_preinit();
    pic_irqmask_write(PICMask);

    ಆಯ್ಕೆ(regs->ax) {
    ಸ್ಥಿತಿ 0000: handle_csm_0000(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0001: handle_csm_0001(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0002: handle_csm_0002(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0003: handle_csm_0003(regs); ನಿಲ್ಲಿಸು;
//    case 0004: handle_csm_0004(regs); break;
    ಸ್ಥಿತಿ 0005: handle_csm_0005(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0006: handle_csm_0006(regs); ನಿಲ್ಲಿಸು;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    ಪೂರ್ವನಿಯೋಜಿತ: regs->al = 1;
    }

    csm_return(regs);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ csm_prio_ಗಮ್ಯ_seabios(u16 csm_prio)
{
    ಆಯ್ಕೆ (csm_prio) {
    ಸ್ಥಿತಿ BBS_DO_NOT_BOOT_FROM:
    ಸ್ಥಿತಿ BBS_IGNORE_ನಮೂದು:
        ಹಿಂತಿರುಗಿ -1;

    ಸ್ಥಿತಿ BBS_ಅತಿಕಡಿಮೆ_ಆದ್ಯತೆ:
    ಸ್ಥಿತಿ BBS_UNPRIORITIZED_ನಮೂದು:
    ಪೂರ್ವನಿಯೋಜಿತ:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        ಹಿಂತಿರುಗಿ csm_prio + 1;
    }
}

ಪೂರ್ಣಾಂಕ csm_bootprio_ata(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ slave)
{
    ಆದರೆ (!csm_boot_ಕೋಷ್ಟಕ)
        ಹಿಂತಿರುಗಿ -1;
    BBS_ಕೋಷ್ಟಕ *bbs = (ಶೂನ್ಯ *)csm_boot_ಕೋಷ್ಟಕ->Bbs_ಕೋಷ್ಟಕ;
    ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            ಸೂಚ್ಯಂಕ, bbs[ಸೂಚ್ಯಂಕ].Boot_ಆದ್ಯತೆ);
    ಹಿಂತಿರುಗಿ csm_prio_ಗಮ್ಯ_seabios(bbs[ಸೂಚ್ಯಂಕ].Boot_ಆದ್ಯತೆ);
}

ಪೂರ್ಣಾಂಕ csm_bootprio_fdc(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ fdid)
{
    ಆದರೆ (!csm_boot_ಕೋಷ್ಟಕ)
        ಹಿಂತಿರುಗಿ -1;
    BBS_ಕೋಷ್ಟಕ *bbs = (ಶೂನ್ಯ *)csm_boot_ಕೋಷ್ಟಕ->Bbs_ಕೋಷ್ಟಕ;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].Boot_ಆದ್ಯತೆ);
    ಹಿಂತಿರುಗಿ csm_prio_ಗಮ್ಯ_seabios(bbs[0].Boot_ಆದ್ಯತೆ);
}

ಪೂರ್ಣಾಂಕ csm_bootprio_pci(ರಚನೆ pci_device *pci)
{
    ಆದರೆ (!csm_boot_ಕೋಷ್ಟಕ)
        ಹಿಂತಿರುಗಿ -1;
    BBS_ಕೋಷ್ಟಕ *bbs = (ಶೂನ್ಯ *)csm_boot_ಕೋಷ್ಟಕ->Bbs_ಕೋಷ್ಟಕ;
    ಪೂರ್ಣಾಂಕ i;

    ಗಾಗಿ (i = 5; i < csm_boot_ಕೋಷ್ಟಕ->ಸಂಖ್ಯೆ_Bbs_Entries; i++) {
        ಆದರೆ (pci->bdf == pci_ಗಮ್ಯ_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].Boot_ಆದ್ಯತೆ);
            ಹಿಂತಿರುಗಿ csm_prio_ಗಮ್ಯ_seabios(bbs[i].Boot_ಆದ್ಯತೆ);
        }
    }
    ಹಿಂತಿರುಗಿ -1;
}
