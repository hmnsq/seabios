// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "e820map.h" // e820_add
#అన్తర్భావ "farptr.h" // MAKE_FLATPTR
#అన్తర్భావ "hw/pci.h" // pci_to_bdf
#అన్తర్భావ "hw/pcidevice.h" // pci_probe_devices
#అన్తర్భావ "hw/pic.h" // pic_irqmask_read
#అన్తర్భావ "malloc.h" // malloc_csm_preinit
#అన్తర్భావ "memmap.h" //చిహ్నం
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // qemu_preinit
#అన్తర్భావ "stacks.h" // wait_threads
#అన్తర్భావ "std/acpi.h" // RSDP_SIGNATURE
#అన్తర్భావ "std/bda.h" // struct bios_data_area_s
#అన్తర్భావ "std/optionrom.h" // struct rom_header
#అన్తర్భావ "util.h" // copy_smbios_21

#నిర్వచించు UINT8 u8
#నిర్వచించు UINT16 u16
#నిర్వచించు UINT32 u32
#అన్తర్భావ "std/LegacyBios.h"

నిర్మాణం rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_పట్టిక csm_compat_పట్టిక VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .పట్టిక_Checksum = 0 /* Filled in by checkrom.py */,
    .పట్టిక_నిడివి = పరిమాణం(csm_compat_పట్టిక),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16_Call_స్థానభ్రంశం = 0 /* Filled in by checkrom.py */,
    .Oem_Id_String_సూచిక = (u32)"SeaBIOS",
    .Acpi_Rsd_Ptr_సూచిక = (u32)&csm_rsdp,
};

EFI_గమ్యం_COMPATIBILITY16_INIT_పట్టిక *csm_init_పట్టిక;
EFI_గమ్యం_COMPATIBILITY16_BOOT_పట్టిక *csm_boot_పట్టిక;

స్థిర u16 PICMask = PIC_IRQMASK_DEFAULT;

బాహ్య శూన్య __csm_return(నిర్మాణం bregs *regs) __noreturn;

స్థిర శూన్య
csm_return(నిర్మాణం bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_పట్టిక.Uma_చిరునామా = rommax;
    csm_compat_పట్టిక.Uma_కొలత = చిహ్నం(final_readonly_ప్రారంభం) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

స్థిర శూన్య
csm_maininit(నిర్మాణం bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_పట్టిక.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_పట్టిక.Pn_PInstallation_Check_స్థానభ్రంశం = get_pnp_స్థానభ్రంశం();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
స్థిర శూన్య
handle_csm_0000(నిర్మాణం bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_పట్టిక = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_పట్టిక->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_పట్టిక->Hi_Pmm_జ్ఞాపకం);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_పట్టిక->Hi_Pmm_జ్ఞాపకం_కొలత_లోపల_బైట్లు);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_పట్టిక->తిరగేయి_Thunk_Call_Segment,
            csm_init_పట్టిక->తిరగేయి_Thunk_Call_స్థానభ్రంశం);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_పట్టిక->సంఖ్య_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_పట్టిక->Os_జ్ఞాపకం_Above1_Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_పట్టిక->Thunk_ప్రారంభం);
    dprintf(3, "ThunkSize       %08x\n", csm_init_పట్టిక->Thunk_కొలత_లోపల_బైట్లు);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_పట్టిక->దిగువ_Pmm_జ్ఞాపకం);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_పట్టిక->దిగువ_Pmm_జ్ఞాపకం_కొలత_లోపల_బైట్లు);

    malloc_csm_preinit(csm_init_పట్టిక->దిగువ_Pmm_జ్ఞాపకం,
                       csm_init_పట్టిక->దిగువ_Pmm_జ్ఞాపకం_కొలత_లోపల_బైట్లు,
                       csm_init_పట్టిక->Hi_Pmm_జ్ఞాపకం,
                       csm_init_పట్టిక->Hi_Pmm_జ్ఞాపకం_కొలత_లోపల_బైట్లు);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
స్థిర శూన్య
handle_csm_0001(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_BOOT) {
        regs->ax = 1;
        తిరిగి;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_పట్టిక = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_పట్టిక->Major_సంస్కరణ);
    dprintf(3, "MinorVersion %04x\n", csm_boot_పట్టిక->Minor_సంస్కరణ);
    dprintf(3, "AcpiTable %08x\n", csm_boot_పట్టిక->Acpi_పట్టిక);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_పట్టిక->Smbios_పట్టిక);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_పట్టిక->Smbios_పట్టిక_నిడివి);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_పట్టిక->Device_మార్గం_Type);
    dprintf(3, "PciIrqMask %04x\n", csm_boot_పట్టిక->Pci_Irq_ముసుగు);
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_పట్టిక->సంఖ్య_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_పట్టిక->సంఖ్య_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_పట్టిక->Bbs_పట్టిక);
    dprintf(3, "SmmTable %08x\n", csm_boot_పట్టిక->Smm_పట్టిక);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_పట్టిక->Os_జ్ఞాపకం_Above1_Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_పట్టిక->Unconventional_Device_పట్టిక);

    regs->ax = 0;
}

/* PrepareToBoot */
స్థిర శూన్య
handle_csm_0002(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_BOOT) {
        regs->ax = 1;
        తిరిగి;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    నిర్మాణం e820entry *p = (శూన్య *)csm_compat_పట్టిక.E820_సూచిక;
    పూర్ణాంకం i;
    కొరకు (i=0; i < csm_compat_పట్టిక.E820_నిడివి / పరిమాణం(నిర్మాణం e820entry); i++)
        e820_కూడు(p[i].ప్రారంభం, p[i].కొలత, p[i].type);

    ఒకవేళ (csm_init_పట్టిక->Hi_Pmm_జ్ఞాపకం_కొలత_లోపల_బైట్లు > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_చివర = csm_init_పట్టిక->Hi_Pmm_జ్ఞాపకం + csm_init_పట్టిక->Hi_Pmm_జ్ఞాపకం_కొలత_లోపల_బైట్లు;
        e820_కూడు(hi_pmm_చివర - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    ఒకవేళ (csm_compat_పట్టిక.Irq_Routing_పట్టిక_సూచిక &&
        csm_compat_పట్టిక.Irq_Routing_పట్టిక_నిడివి) {
        PirAddr = (శూన్య *)csm_compat_పట్టిక.Irq_Routing_పట్టిక_సూచిక;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    ఒకవేళ (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        కనుగొను_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    ఒకవేళ (csm_boot_పట్టిక->Smbios_పట్టిక)
        copy_smbios_21((శూన్య *)csm_boot_పట్టిక->Smbios_పట్టిక);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    నిర్మాణం bios_దత్తాంశం_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    bda->hdcount = 0;

    thread_అమరిక();
    mathcp_అమరిక();
    timer_అమరిక();
    clock_అమరిక();
    device_hardware_అమరిక();
    wait_దారాలు();
    interactive_bootmenu();

    prepareboot();

    regs->ax = 0;
}

/* Boot */
స్థిర శూన్య
handle_csm_0003(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_BOOT) {
        regs->ax = 1;
        తిరిగి;
    }

    dprintf(3, "Boot\n");

    ప్రారంభం_Boot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
స్థిర శూన్య
handle_csm_0005(నిర్మాణం bregs *regs)
{
    EFI_DISPATCH_OPROM_పట్టిక *పట్టిక = MAKE_FLATPTR(regs->es, regs->bx);
    నిర్మాణం rom_శీర్షిక *rom;
    u16 bdf;

    ఒకవేళ (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        తిరిగి;
    }

    dprintf(3, "Legacy16DispatchOprom rom %p\n", పట్టిక);

    dprintf(3, "OpromSegment   %04x\n", పట్టిక->OpromSegment);
    dprintf(3, "RuntimeSegment %04x\n", పట్టిక->RuntimeSegment);
    dprintf(3, "PnPInstallationCheck %04x:%04x\n",
            పట్టిక->PnPInstallationCheckSegment,
            పట్టిక->Pn_PInstallation_Check_స్థానభ్రంశం);
    dprintf(3, "RuntimeSegment %04x\n", పట్టిక->RuntimeSegment);

    rom = MAKE_FLATPTR(పట్టిక->OpromSegment, 0);
    bdf = pci_bus_devfn_గమ్యం_bdf(పట్టిక->PciBus, పట్టిక->PciDeviceFunction);

    rom_reserve(rom->కొలత * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->కొలత * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
స్థిర శూన్య
handle_csm_0006(నిర్మాణం bregs *regs)
{
    u16 కొలత = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    శూన్య *chunk = ఖాళీ;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        కొలత, align, region);

    ఒకవేళ (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    ఒకవేళ (align) {
        align = 1 << __ffs(align);
        ఒకవేళ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } లేకపోతే {
        align = MALLOC_MIN_ALIGN;
    }

    ఒకవేళ (region & 2)
        chunk = _malloc(&Zone_దిగువ, కొలత, align);
    ఒకవేళ (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, కొలత, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        కొలత, align, region, chunk);
    ఒకవేళ (chunk) {
        regs->ds = FLATPTR_గమ్యం_SEG(chunk);
        regs->bx = FLATPTR_గమ్యం_స్థానభ్రంశం(chunk);
        regs->ax = 0;
    } లేకపోతే {
        regs->ax = 1;
    }
}

శూన్య VISIBLE32INIT
handle_csm(నిర్మాణం bregs *regs)
{
    ASSERT32FLAT();

    ఒకవేళ (!CONFIG_CSM)
        తిరిగి;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    సంకేతం_mutable_preinit();
    pic_irqmask_write(PICMask);

    ఎంపిక(regs->ax) {
    స్థితి 0000: handle_csm_0000(regs); ఆపు;
    స్థితి 0001: handle_csm_0001(regs); ఆపు;
    స్థితి 0002: handle_csm_0002(regs); ఆపు;
    స్థితి 0003: handle_csm_0003(regs); ఆపు;
//    case 0004: handle_csm_0004(regs); break;
    స్థితి 0005: handle_csm_0005(regs); ఆపు;
    స్థితి 0006: handle_csm_0006(regs); ఆపు;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    అప్రమేయ: regs->al = 1;
    }

    csm_return(regs);
}

స్థిర పూర్ణాంకం csm_prio_గమ్యం_seabios(u16 csm_prio)
{
    ఎంపిక (csm_prio) {
    స్థితి BBS_DO_NOT_BOOT_FROM:
    స్థితి BBS_IGNORE_నమోదు:
        తిరిగి -1;

    స్థితి BBS_అత్యల్ప_ప్రాధాన్యత:
    స్థితి BBS_UNPRIORITIZED_నమోదు:
    అప్రమేయ:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        తిరిగి csm_prio + 1;
    }
}

పూర్ణాంకం csm_bootprio_ata(నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం slave)
{
    ఒకవేళ (!csm_boot_పట్టిక)
        తిరిగి -1;
    BBS_పట్టిక *bbs = (శూన్య *)csm_boot_పట్టిక->Bbs_పట్టిక;
    పూర్ణాంకం సూచీ = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            సూచీ, bbs[సూచీ].Boot_ప్రాధాన్యత);
    తిరిగి csm_prio_గమ్యం_seabios(bbs[సూచీ].Boot_ప్రాధాన్యత);
}

పూర్ణాంకం csm_bootprio_fdc(నిర్మాణం pci_device *pci, పూర్ణాంకం పోర్టు, పూర్ణాంకం fdid)
{
    ఒకవేళ (!csm_boot_పట్టిక)
        తిరిగి -1;
    BBS_పట్టిక *bbs = (శూన్య *)csm_boot_పట్టిక->Bbs_పట్టిక;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].Boot_ప్రాధాన్యత);
    తిరిగి csm_prio_గమ్యం_seabios(bbs[0].Boot_ప్రాధాన్యత);
}

పూర్ణాంకం csm_bootprio_pci(నిర్మాణం pci_device *pci)
{
    ఒకవేళ (!csm_boot_పట్టిక)
        తిరిగి -1;
    BBS_పట్టిక *bbs = (శూన్య *)csm_boot_పట్టిక->Bbs_పట్టిక;
    పూర్ణాంకం i;

    కొరకు (i = 5; i < csm_boot_పట్టిక->సంఖ్య_Bbs_Entries; i++) {
        ఒకవేళ (pci->bdf == pci_గమ్యం_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].Boot_ప్రాధాన్యత);
            తిరిగి csm_prio_గమ్యం_seabios(bbs[i].Boot_ప్రాధాన్యత);
        }
    }
    తిరిగి -1;
}
