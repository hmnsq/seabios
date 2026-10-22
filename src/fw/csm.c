// Compatibility Support Module (CSM) for UEFI / EDK-II
//
// Copyright © 2013 Intel Corporation
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "e820map.h" // e820_add
#ضمن "farptr.h" // MAKE_FLATPTR
#ضمن "hw/pci.h" // pci_to_bdf
#ضمن "hw/pcidevice.h" // pci_probe_devices
#ضمن "hw/pic.h" // pic_irqmask_read
#ضمن "malloc.h" // malloc_csm_preinit
#ضمن "memmap.h" //علامت
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // qemu_preinit
#ضمن "stacks.h" // wait_threads
#ضمن "std/acpi.h" // RSDP_SIGNATURE
#ضمن "std/bda.h" // struct bios_data_area_s
#ضمن "std/optionrom.h" // struct rom_header
#ضمن "util.h" // copy_smbios_21

#تعریف_کرو UINT8 u8
#تعریف_کرو UINT16 u16
#تعریف_کرو UINT32 u32
#ضمن "std/LegacyBios.h"

بنية rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_جدول csm_compat_جدول VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .جدول_Checksum = 0 /* Filled in by checkrom.py */,
    .جدول_لمائی = حجم(csm_compat_جدول),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16_Call_وتھ = 0 /* Filled in by checkrom.py */,
    .Oem_Id_String_اشارہ_گر = (u32)"SeaBIOS",
    .Acpi_Rsd_Ptr_اشارہ_گر = (u32)&csm_rsdp,
};

EFI_منزل_COMPATIBILITY16_INIT_جدول *csm_init_جدول;
EFI_منزل_COMPATIBILITY16_BOOT_جدول *csm_boot_جدول;

ساكن u16 PICMask = PIC_IRQMASK_DEFAULT;

خارجي فراغ __csm_return(بنية bregs *regs) __noreturn;

ساكن فراغ
csm_return(بنية bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_جدول.Uma_پتہ = rommax;
    csm_compat_جدول.Uma_ناپ = علامت(final_readonly_شروع) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

ساكن فراغ
csm_maininit(بنية bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_جدول.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_جدول.Pn_PInstallation_Check_وتھ = get_pnp_وتھ();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
ساكن فراغ
handle_csm_0000(بنية bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_جدول = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_جدول->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_جدول->Hi_Pmm_حافظہ);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_جدول->Hi_Pmm_حافظہ_ناپ_وچ_بائٹاں);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_جدول->الٹ_Thunk_Call_Segment,
            csm_init_جدول->الٹ_Thunk_Call_وتھ);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_جدول->نمبر_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_جدول->Os_حافظہ_Above1_Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_جدول->Thunk_شروع);
    dprintf(3, "ThunkSize       %08x\n", csm_init_جدول->Thunk_ناپ_وچ_بائٹاں);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_جدول->کم_Pmm_حافظہ);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_جدول->کم_Pmm_حافظہ_ناپ_وچ_بائٹاں);

    malloc_csm_preinit(csm_init_جدول->کم_Pmm_حافظہ,
                       csm_init_جدول->کم_Pmm_حافظہ_ناپ_وچ_بائٹاں,
                       csm_init_جدول->Hi_Pmm_حافظہ,
                       csm_init_جدول->Hi_Pmm_حافظہ_ناپ_وچ_بائٹاں);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
ساكن فراغ
handle_csm_0001(بنية bregs *regs)
{
    إذا (!CONFIG_BOOT) {
        regs->ax = 1;
        أرجع;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_جدول = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_جدول->Major_نسخہ);
    dprintf(3, "MinorVersion %04x\n", csm_boot_جدول->Minor_نسخہ);
    dprintf(3, "AcpiTable %08x\n", csm_boot_جدول->Acpi_جدول);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_جدول->Smbios_جدول);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_جدول->Smbios_جدول_لمائی);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_جدول->Device_رستہ_Type);
    dprintf(3, "PciIrqMask %04x\n", csm_boot_جدول->Pci_Irq_نقاب);
    dprintf(3, "NumberE820Entries %08x\n", csm_boot_جدول->نمبر_E820_Entries);
//    dprintf(3, "HddInfo %08x\n", csm_boot_table->HddInfo);
    dprintf(3, "NumberBbsEntries %08x\n", csm_boot_جدول->نمبر_Bbs_Entries);
    dprintf(3, "BBsTable %08x\n", csm_boot_جدول->Bbs_جدول);
    dprintf(3, "SmmTable %08x\n", csm_boot_جدول->Smm_جدول);
    dprintf(3, "OsMemoryAbove1Mb %08x\n", csm_boot_جدول->Os_حافظہ_Above1_Mb);
    dprintf(3, "UnconventionalDeviceTable %08x\n", csm_boot_جدول->Unconventional_Device_جدول);

    regs->ax = 0;
}

/* PrepareToBoot */
ساكن فراغ
handle_csm_0002(بنية bregs *regs)
{
    إذا (!CONFIG_BOOT) {
        regs->ax = 1;
        أرجع;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    بنية e820entry *p = (فراغ *)csm_compat_جدول.E820_اشارہ_گر;
    صحيح i;
    لكل (i=0; i < csm_compat_جدول.E820_لمائی / حجم(بنية e820entry); i++)
        e820_جوڑو(p[i].شروع, p[i].ناپ, p[i].type);

    إذا (csm_init_جدول->Hi_Pmm_حافظہ_ناپ_وچ_بائٹاں > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_سرا = csm_init_جدول->Hi_Pmm_حافظہ + csm_init_جدول->Hi_Pmm_حافظہ_ناپ_وچ_بائٹاں;
        e820_جوڑو(hi_pmm_سرا - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    إذا (csm_compat_جدول.Irq_Routing_جدول_اشارہ_گر &&
        csm_compat_جدول.Irq_Routing_جدول_لمائی) {
        PirAddr = (فراغ *)csm_compat_جدول.Irq_Routing_جدول_اشارہ_گر;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    إذا (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        لبھو_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    إذا (csm_boot_جدول->Smbios_جدول)
        copy_smbios_21((فراغ *)csm_boot_جدول->Smbios_جدول);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    بنية bios_انکڑے_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    bda->hdcount = 0;

    thread_تیاری();
    mathcp_تیاری();
    timer_تیاری();
    clock_تیاری();
    device_hardware_تیاری();
    wait_دھاگے();
    interactive_bootmenu();

    prepareboot();

    regs->ax = 0;
}

/* Boot */
ساكن فراغ
handle_csm_0003(بنية bregs *regs)
{
    إذا (!CONFIG_BOOT) {
        regs->ax = 1;
        أرجع;
    }

    dprintf(3, "Boot\n");

    شروع_Boot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
ساكن فراغ
handle_csm_0005(بنية bregs *regs)
{
    EFI_DISPATCH_OPROM_جدول *جدول = MAKE_FLATPTR(regs->es, regs->bx);
    بنية rom_سرخی *rom;
    u16 bdf;

    إذا (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        أرجع;
    }

    dprintf(3, "Legacy16DispatchOprom rom %p\n", جدول);

    dprintf(3, "OpromSegment   %04x\n", جدول->OpromSegment);
    dprintf(3, "RuntimeSegment %04x\n", جدول->RuntimeSegment);
    dprintf(3, "PnPInstallationCheck %04x:%04x\n",
            جدول->PnPInstallationCheckSegment,
            جدول->Pn_PInstallation_Check_وتھ);
    dprintf(3, "RuntimeSegment %04x\n", جدول->RuntimeSegment);

    rom = MAKE_FLATPTR(جدول->OpromSegment, 0);
    bdf = pci_bus_devfn_منزل_bdf(جدول->PciBus, جدول->PciDeviceFunction);

    rom_reserve(rom->ناپ * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->ناپ * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
ساكن فراغ
handle_csm_0006(بنية bregs *regs)
{
    u16 ناپ = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    فراغ *chunk = عدم;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        ناپ, align, region);

    إذا (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    إذا (align) {
        align = 1 << __ffs(align);
        إذا (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } وإلا {
        align = MALLOC_MIN_ALIGN;
    }

    إذا (region & 2)
        chunk = _malloc(&Zone_کم, ناپ, align);
    إذا (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, ناپ, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        ناپ, align, region, chunk);
    إذا (chunk) {
        regs->ds = FLATPTR_منزل_SEG(chunk);
        regs->bx = FLATPTR_منزل_وتھ(chunk);
        regs->ax = 0;
    } وإلا {
        regs->ax = 1;
    }
}

فراغ VISIBLE32INIT
handle_csm(بنية bregs *regs)
{
    ASSERT32FLAT();

    إذا (!CONFIG_CSM)
        أرجع;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    کوڈ_mutable_preinit();
    pic_irqmask_write(PICMask);

    بدل(regs->ax) {
    حالة 0000: handle_csm_0000(regs); اكسر;
    حالة 0001: handle_csm_0001(regs); اكسر;
    حالة 0002: handle_csm_0002(regs); اكسر;
    حالة 0003: handle_csm_0003(regs); اكسر;
//    case 0004: handle_csm_0004(regs); break;
    حالة 0005: handle_csm_0005(regs); اكسر;
    حالة 0006: handle_csm_0006(regs); اكسر;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    افتراضي: regs->al = 1;
    }

    csm_return(regs);
}

ساكن صحيح csm_prio_منزل_seabios(u16 csm_prio)
{
    بدل (csm_prio) {
    حالة BBS_DO_NOT_BOOT_FROM:
    حالة BBS_IGNORE_اندراج:
        أرجع -1;

    حالة BBS_سبھ_توں_نیواں_ترجیح:
    حالة BBS_UNPRIORITIZED_اندراج:
    افتراضي:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        أرجع csm_prio + 1;
    }
}

صحيح csm_bootprio_ata(بنية pci_device *pci, صحيح chanid, صحيح slave)
{
    إذا (!csm_boot_جدول)
        أرجع -1;
    BBS_جدول *bbs = (فراغ *)csm_boot_جدول->Bbs_جدول;
    صحيح اشاریہ = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            اشاریہ, bbs[اشاریہ].Boot_ترجیح);
    أرجع csm_prio_منزل_seabios(bbs[اشاریہ].Boot_ترجیح);
}

صحيح csm_bootprio_fdc(بنية pci_device *pci, صحيح پورٹ, صحيح fdid)
{
    إذا (!csm_boot_جدول)
        أرجع -1;
    BBS_جدول *bbs = (فراغ *)csm_boot_جدول->Bbs_جدول;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].Boot_ترجیح);
    أرجع csm_prio_منزل_seabios(bbs[0].Boot_ترجیح);
}

صحيح csm_bootprio_pci(بنية pci_device *pci)
{
    إذا (!csm_boot_جدول)
        أرجع -1;
    BBS_جدول *bbs = (فراغ *)csm_boot_جدول->Bbs_جدول;
    صحيح i;

    لكل (i = 5; i < csm_boot_جدول->نمبر_Bbs_Entries; i++) {
        إذا (pci->bdf == pci_منزل_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].Boot_ترجیح);
            أرجع csm_prio_منزل_seabios(bbs[i].Boot_ترجیح);
        }
    }
    أرجع -1;
}
