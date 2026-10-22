// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // e820_add
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // MAKE_FLATPTR
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_to_bdf
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // pci_probe_devices
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pic.h" // pic_irqmask_read
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_csm_preinit
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" //ਚਿੰਨ੍ਹ
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // qemu_preinit
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // wait_threads
#ਅਨ੍ਤਰ੍ਭਾਵ "std/acpi.h" // RSDP_SIGNATURE
#ਅਨ੍ਤਰ੍ਭਾਵ "std/bda.h" // struct bios_data_area_s
#ਅਨ੍ਤਰ੍ਭਾਵ "std/optionrom.h" // struct rom_header
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // copy_smbios_21

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UINT8 u8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UINT16 u16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UINT32 u32
#ਅਨ੍ਤਰ੍ਭਾਵ "std/LegacyBios.h"

ਬਣਤਰ rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_ਸਾਰਣੀ csm_compat_ਸਾਰਣੀ VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .ਸਾਰਣੀ_Checksum = 0 /* Filled in by checkrom.py */,
    .ਸਾਰਣੀ_ਲੰਮਾਈ = ਆਕਾਰ(csm_compat_ਸਾਰਣੀ),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16_Call_ਵਿੱਥ = 0 /* Filled in by checkrom.py */,
    .Oem_Id_String_ਸੂਚਕ = (u32)"SeaBIOS",
    .Acpi_Rsd_Ptr_ਸੂਚਕ = (u32)&csm_rsdp,
};

EFI_ਮੰਜ਼ਿਲ_COMPATIBILITY16_INIT_ਸਾਰਣੀ *csm_init_ਸਾਰਣੀ;
EFI_ਮੰਜ਼ਿਲ_COMPATIBILITY16_BOOT_ਸਾਰਣੀ *csm_boot_ਸਾਰਣੀ;

ਸਥਿਰ u16 PICMask = PIC_IRQMASK_DEFAULT;

ਬਾਹਰੀ ਖਾਲੀ __csm_return(ਬਣਤਰ bregs *regs) __noreturn;

ਸਥਿਰ ਖਾਲੀ
csm_return(ਬਣਤਰ bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_ਸਾਰਣੀ.Uma_ਪਤਾ = rommax;
    csm_compat_ਸਾਰਣੀ.Uma_ਮਾਪ = ਚਿੰਨ੍ਹ(final_readonly_ਸ਼ੁਰੂ) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

ਸਥਿਰ ਖਾਲੀ
csm_maininit(ਬਣਤਰ bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_ਸਾਰਣੀ.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_ਸਾਰਣੀ.Pn_PInstallation_Check_ਵਿੱਥ = get_pnp_ਵਿੱਥ();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
ਸਥਿਰ ਖਾਲੀ
handle_csm_0000(ਬਣਤਰ bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_ਸਾਰਣੀ = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_ਸਾਰਣੀ->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_ਸਾਰਣੀ->Hi_Pmm_ਸਮ੍ਰਿਤੀ);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_ਸਾਰਣੀ->Hi_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_ਸਾਰਣੀ->ਉਲਟ_Thunk_Call_Segment,
            csm_init_ਸਾਰਣੀ->ਉਲਟ_Thunk_Call_ਵਿੱਥ);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_ਸਾਰਣੀ->ਸੰਖਿਆ_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_ਸਾਰਣੀ->Os_ਸਮ੍ਰਿਤੀ_Above1_Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_ਸਾਰਣੀ->Thunk_ਸ਼ੁਰੂ);
    dprintf(3, "ThunkSize       %08x\n", csm_init_ਸਾਰਣੀ->Thunk_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_ਸਾਰਣੀ->ਨੀਵਾਂ_Pmm_ਸਮ੍ਰਿਤੀ);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_ਸਾਰਣੀ->ਨੀਵਾਂ_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ);

    malloc_csm_preinit(csm_init_ਸਾਰਣੀ->ਨੀਵਾਂ_Pmm_ਸਮ੍ਰਿਤੀ,
                       csm_init_ਸਾਰਣੀ->ਨੀਵਾਂ_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ,
                       csm_init_ਸਾਰਣੀ->Hi_Pmm_ਸਮ੍ਰਿਤੀ,
                       csm_init_ਸਾਰਣੀ->Hi_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
ਸਥਿਰ ਖਾਲੀ
handle_csm_0001(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_BOOT) {
        regs->ax = 1;
        ਵਾਪਸ;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_ਸਾਰਣੀ = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_ਸਾਰਣੀ->Major_ਸੰਸਕਰਣ);
    dprintf(3, "MinorVersion %04x\n", csm_boot_ਸਾਰਣੀ->Minor_ਸੰਸਕਰਣ);
    dprintf(3, "AcpiTable %08x\n", csm_boot_ਸਾਰਣੀ->Acpi_ਸਾਰਣੀ);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_ਸਾਰਣੀ->Smbios_ਸਾਰਣੀ);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_ਸਾਰਣੀ->Smbios_ਸਾਰਣੀ_ਲੰਮਾਈ);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_ਸਾਰਣੀ->Device_ਰਸਤਾ_Type);
    dprintf(3, "PciIrqMask %04x\n", csm_boot_ਸਾਰਣੀ->Pci_Irq_ਨਕਾਬ);
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_ਸਾਰਣੀ->ਸੰਖਿਆ_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_ਸਾਰਣੀ->ਸੰਖਿਆ_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_ਸਾਰਣੀ->Bbs_ਸਾਰਣੀ);
    dprintf(3, "SmmTable %08x\n", csm_boot_ਸਾਰਣੀ->Smm_ਸਾਰਣੀ);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_ਸਾਰਣੀ->Os_ਸਮ੍ਰਿਤੀ_Above1_Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_ਸਾਰਣੀ->Unconventional_Device_ਸਾਰਣੀ);

    regs->ax = 0;
}

/* PrepareToBoot */
ਸਥਿਰ ਖਾਲੀ
handle_csm_0002(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_BOOT) {
        regs->ax = 1;
        ਵਾਪਸ;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    ਬਣਤਰ e820entry *p = (ਖਾਲੀ *)csm_compat_ਸਾਰਣੀ.E820_ਸੂਚਕ;
    ਪੂਰਨ i;
    ਲਈ (i=0; i < csm_compat_ਸਾਰਣੀ.E820_ਲੰਮਾਈ / ਆਕਾਰ(ਬਣਤਰ e820entry); i++)
        e820_ਜੋੜੋ(p[i].ਸ਼ੁਰੂ, p[i].ਮਾਪ, p[i].type);

    ਜੇ (csm_init_ਸਾਰਣੀ->Hi_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_ਸਿਰਾ = csm_init_ਸਾਰਣੀ->Hi_Pmm_ਸਮ੍ਰਿਤੀ + csm_init_ਸਾਰਣੀ->Hi_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ;
        e820_ਜੋੜੋ(hi_pmm_ਸਿਰਾ - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    ਜੇ (csm_compat_ਸਾਰਣੀ.Irq_Routing_ਸਾਰਣੀ_ਸੂਚਕ &&
        csm_compat_ਸਾਰਣੀ.Irq_Routing_ਸਾਰਣੀ_ਲੰਮਾਈ) {
        PirAddr = (ਖਾਲੀ *)csm_compat_ਸਾਰਣੀ.Irq_Routing_ਸਾਰਣੀ_ਸੂਚਕ;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    ਜੇ (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        ਲੱਭੋ_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    ਜੇ (csm_boot_ਸਾਰਣੀ->Smbios_ਸਾਰਣੀ)
        copy_smbios_21((ਖਾਲੀ *)csm_boot_ਸਾਰਣੀ->Smbios_ਸਾਰਣੀ);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    ਬਣਤਰ bios_ਅੰਕੜੇ_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    bda->hdcount = 0;

    thread_ਤਿਆਰੀ();
    mathcp_ਤਿਆਰੀ();
    timer_ਤਿਆਰੀ();
    clock_ਤਿਆਰੀ();
    device_hardware_ਤਿਆਰੀ();
    wait_ਧਾਗੇ();
    interactive_bootmenu();

    prepareboot();

    regs->ax = 0;
}

/* Boot */
ਸਥਿਰ ਖਾਲੀ
handle_csm_0003(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_BOOT) {
        regs->ax = 1;
        ਵਾਪਸ;
    }

    dprintf(3, "Boot\n");

    ਸ਼ੁਰੂ_Boot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
ਸਥਿਰ ਖਾਲੀ
handle_csm_0005(ਬਣਤਰ bregs *regs)
{
    EFI_DISPATCH_OPROM_ਸਾਰਣੀ *ਸਾਰਣੀ = MAKE_FLATPTR(regs->es, regs->bx);
    ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom;
    u16 bdf;

    ਜੇ (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        ਵਾਪਸ;
    }

    dprintf(3, "Legacy16DispatchOprom rom %p\n", ਸਾਰਣੀ);

    dprintf(3, "OpromSegment   %04x\n", ਸਾਰਣੀ->OpromSegment);
    dprintf(3, "RuntimeSegment %04x\n", ਸਾਰਣੀ->RuntimeSegment);
    dprintf(3, "PnPInstallationCheck %04x:%04x\n",
            ਸਾਰਣੀ->PnPInstallationCheckSegment,
            ਸਾਰਣੀ->Pn_PInstallation_Check_ਵਿੱਥ);
    dprintf(3, "RuntimeSegment %04x\n", ਸਾਰਣੀ->RuntimeSegment);

    rom = MAKE_FLATPTR(ਸਾਰਣੀ->OpromSegment, 0);
    bdf = pci_bus_devfn_ਮੰਜ਼ਿਲ_bdf(ਸਾਰਣੀ->PciBus, ਸਾਰਣੀ->PciDeviceFunction);

    rom_reserve(rom->ਮਾਪ * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->ਮਾਪ * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
ਸਥਿਰ ਖਾਲੀ
handle_csm_0006(ਬਣਤਰ bregs *regs)
{
    u16 ਮਾਪ = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    ਖਾਲੀ *chunk = ਨਿਰਾ;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        ਮਾਪ, align, region);

    ਜੇ (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    ਜੇ (align) {
        align = 1 << __ffs(align);
        ਜੇ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } ਨਹੀਂਤਾਂ {
        align = MALLOC_MIN_ALIGN;
    }

    ਜੇ (region & 2)
        chunk = _malloc(&Zone_ਨੀਵਾਂ, ਮਾਪ, align);
    ਜੇ (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, ਮਾਪ, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        ਮਾਪ, align, region, chunk);
    ਜੇ (chunk) {
        regs->ds = FLATPTR_ਮੰਜ਼ਿਲ_SEG(chunk);
        regs->bx = FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(chunk);
        regs->ax = 0;
    } ਨਹੀਂਤਾਂ {
        regs->ax = 1;
    }
}

ਖਾਲੀ VISIBLE32INIT
handle_csm(ਬਣਤਰ bregs *regs)
{
    ASSERT32FLAT();

    ਜੇ (!CONFIG_CSM)
        ਵਾਪਸ;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    ਕੋਡ_mutable_preinit();
    pic_irqmask_write(PICMask);

    ਚੋਣ(regs->ax) {
    ਹਾਲਤ 0000: handle_csm_0000(regs); ਰੋਕੋ;
    ਹਾਲਤ 0001: handle_csm_0001(regs); ਰੋਕੋ;
    ਹਾਲਤ 0002: handle_csm_0002(regs); ਰੋਕੋ;
    ਹਾਲਤ 0003: handle_csm_0003(regs); ਰੋਕੋ;
//    case 0004: handle_csm_0004(regs); break;
    ਹਾਲਤ 0005: handle_csm_0005(regs); ਰੋਕੋ;
    ਹਾਲਤ 0006: handle_csm_0006(regs); ਰੋਕੋ;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    ਮੂਲ: regs->al = 1;
    }

    csm_return(regs);
}

ਸਥਿਰ ਪੂਰਨ csm_prio_ਮੰਜ਼ਿਲ_seabios(u16 csm_prio)
{
    ਚੋਣ (csm_prio) {
    ਹਾਲਤ BBS_DO_NOT_BOOT_FROM:
    ਹਾਲਤ BBS_IGNORE_ਇੰਦਰਾਜ:
        ਵਾਪਸ -1;

    ਹਾਲਤ BBS_ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਤਰਜੀਹ:
    ਹਾਲਤ BBS_UNPRIORITIZED_ਇੰਦਰਾਜ:
    ਮੂਲ:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        ਵਾਪਸ csm_prio + 1;
    }
}

ਪੂਰਨ csm_bootprio_ata(ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ slave)
{
    ਜੇ (!csm_boot_ਸਾਰਣੀ)
        ਵਾਪਸ -1;
    BBS_ਸਾਰਣੀ *bbs = (ਖਾਲੀ *)csm_boot_ਸਾਰਣੀ->Bbs_ਸਾਰਣੀ;
    ਪੂਰਨ ਸੂਚਕਾਂਕ = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            ਸੂਚਕਾਂਕ, bbs[ਸੂਚਕਾਂਕ].Boot_ਤਰਜੀਹ);
    ਵਾਪਸ csm_prio_ਮੰਜ਼ਿਲ_seabios(bbs[ਸੂਚਕਾਂਕ].Boot_ਤਰਜੀਹ);
}

ਪੂਰਨ csm_bootprio_fdc(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ fdid)
{
    ਜੇ (!csm_boot_ਸਾਰਣੀ)
        ਵਾਪਸ -1;
    BBS_ਸਾਰਣੀ *bbs = (ਖਾਲੀ *)csm_boot_ਸਾਰਣੀ->Bbs_ਸਾਰਣੀ;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].Boot_ਤਰਜੀਹ);
    ਵਾਪਸ csm_prio_ਮੰਜ਼ਿਲ_seabios(bbs[0].Boot_ਤਰਜੀਹ);
}

ਪੂਰਨ csm_bootprio_pci(ਬਣਤਰ pci_device *pci)
{
    ਜੇ (!csm_boot_ਸਾਰਣੀ)
        ਵਾਪਸ -1;
    BBS_ਸਾਰਣੀ *bbs = (ਖਾਲੀ *)csm_boot_ਸਾਰਣੀ->Bbs_ਸਾਰਣੀ;
    ਪੂਰਨ i;

    ਲਈ (i = 5; i < csm_boot_ਸਾਰਣੀ->ਸੰਖਿਆ_Bbs_Entries; i++) {
        ਜੇ (pci->bdf == pci_ਮੰਜ਼ਿਲ_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].Boot_ਤਰਜੀਹ);
            ਵਾਪਸ csm_prio_ਮੰਜ਼ਿਲ_seabios(bbs[i].Boot_ਤਰਜੀਹ);
        }
    }
    ਵਾਪਸ -1;
}
