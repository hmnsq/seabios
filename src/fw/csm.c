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
#شامل "memmap.h" //علامت
#شامل "output.h" // dprintf
#شامل "paravirt.h" // qemu_preinit
#شامل "stacks.h" // wait_threads
#شامل "std/acpi.h" // RSDP_SIGNATURE
#شامل "std/bda.h" // struct bios_data_area_s
#شامل "std/optionrom.h" // struct rom_header
#شامل "util.h" // copy_smbios_21

#تعریف_کرو UINT8 u8
#تعریف_کرو UINT16 u16
#تعریف_کرو UINT32 u32
#شامل "std/LegacyBios.h"

ساخت rsdp_descriptor csm_rsdp VARFSEG __aligned(16);

EFI_COMPATIBILITY16_جدول csm_compat_جدول VARFSEG __aligned(16) = {
    .Signature = 0x24454649,
    .جدول_Checksum = 0 /* Filled in by checkrom.py */,
    .جدول_لمبائی = حجم(csm_compat_جدول),
    .Compatibility16CallSegment = SEG_BIOS,
    .Compatibility16_Call_انحراف = 0 /* Filled in by checkrom.py */,
    .Oem_Id_String_اشارہ_گر = (u32)"SeaBIOS",
    .Acpi_Rsd_Ptr_اشارہ_گر = (u32)&csm_rsdp,
};

EFI_مقصد_COMPATIBILITY16_INIT_جدول *csm_init_جدول;
EFI_مقصد_COMPATIBILITY16_BOOT_جدول *csm_boot_جدول;

ساکن u16 PICMask = PIC_IRQMASK_DEFAULT;

خارجی خالی __csm_return(ساخت bregs *regs) __noreturn;

ساکن خالی
csm_return(ساخت bregs *regs)
{
    u32 rommax = rom_get_max();

    dprintf(3, "handle_csm returning AX=%04x\n", regs->ax);

    csm_compat_جدول.Uma_پتہ = rommax;
    csm_compat_جدول.Uma_سائز = علامت(final_readonly_آغاز) - rommax;

    PICMask = pic_irqmask_read();
    __csm_return(regs);
}

ساکن خالی
csm_maininit(ساخت bregs *regs)
{
    interface_init();
    pci_probe_devices();

    csm_compat_جدول.PnPInstallationCheckSegment = SEG_BIOS;
    csm_compat_جدول.Pn_PInstallation_Check_انحراف = get_pnp_انحراف();

    regs->ax = 0;

    csm_return(regs);
}

/* Legacy16InitializeYourself */
ساکن خالی
handle_csm_0000(ساخت bregs *regs)
{
    qemu_preinit();

    dprintf(3, "Legacy16InitializeYourself table %04x:%04x\n", regs->es,
            regs->bx);

    csm_init_جدول = MAKE_FLATPTR(regs->es, regs->bx);

    dprintf(3, "BiosLessThan1MB %08x\n", csm_init_جدول->BiosLessThan1MB);
    dprintf(3, "HiPmmMemory     %08x\n", csm_init_جدول->Hi_Pmm_حافظہ);
    dprintf(3, "HiPmmMemorySize %08x\n", csm_init_جدول->Hi_Pmm_حافظہ_سائز_اندر_بائٹیں);
    dprintf(3, "ReverseThunk    %04x:%04x\n", csm_init_جدول->الٹ_Thunk_Call_Segment,
            csm_init_جدول->الٹ_Thunk_Call_انحراف);
    dprintf(3, "NumE820Entries  %08x\n", csm_init_جدول->نمبر_E820_Entries);
    dprintf(3, "OsMemoryAbove1M %08x\n", csm_init_جدول->Os_حافظہ_Above1_Mb);
    dprintf(3, "ThunkStart      %08x\n", csm_init_جدول->Thunk_آغاز);
    dprintf(3, "ThunkSize       %08x\n", csm_init_جدول->Thunk_سائز_اندر_بائٹیں);
    dprintf(3, "LoPmmMemory     %08x\n", csm_init_جدول->کم_Pmm_حافظہ);
    dprintf(3, "LoPmmMemorySize %08x\n", csm_init_جدول->کم_Pmm_حافظہ_سائز_اندر_بائٹیں);

    malloc_csm_preinit(csm_init_جدول->کم_Pmm_حافظہ,
                       csm_init_جدول->کم_Pmm_حافظہ_سائز_اندر_بائٹیں,
                       csm_init_جدول->Hi_Pmm_حافظہ,
                       csm_init_جدول->Hi_Pmm_حافظہ_سائز_اندر_بائٹیں);
    reloc_preinit(csm_maininit, regs);
}

/* Legacy16UpdateBbs */
ساکن خالی
handle_csm_0001(ساخت bregs *regs)
{
    اگر (!CONFIG_BOOT) {
        regs->ax = 1;
        واپس;
    }

    dprintf(3, "Legacy16UpdateBbs table %04x:%04x\n", regs->es, regs->bx);

    csm_boot_جدول = MAKE_FLATPTR(regs->es, regs->bx);
    dprintf(3, "MajorVersion %04x\n", csm_boot_جدول->Major_نسخہ);
    dprintf(3, "MinorVersion %04x\n", csm_boot_جدول->Minor_نسخہ);
    dprintf(3, "AcpiTable %08x\n", csm_boot_جدول->Acpi_جدول);
    dprintf(3, "SmbiosTable %08x\n", csm_boot_جدول->Smbios_جدول);
    dprintf(3, "SmbiosTableLength %08x\n", csm_boot_جدول->Smbios_جدول_لمبائی);
//    dprintf(3, "SioData %08x\n", csm_boot_table->SioData);
    dprintf(3, "DevicePathType %04x\n", csm_boot_جدول->Device_راستہ_Type);
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
ساکن خالی
handle_csm_0002(ساخت bregs *regs)
{
    اگر (!CONFIG_BOOT) {
        regs->ax = 1;
        واپس;
    }

    dprintf(3, "PrepareToBoot table %04x:%04x\n", regs->es, regs->bx);

    ساخت e820entry *p = (خالی *)csm_compat_جدول.E820_اشارہ_گر;
    صحیح i;
    برائے (i=0; i < csm_compat_جدول.E820_لمبائی / حجم(ساخت e820entry); i++)
        e820_جوڑو(p[i].آغاز, p[i].سائز, p[i].type);

    اگر (csm_init_جدول->Hi_Pmm_حافظہ_سائز_اندر_بائٹیں > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_انتہا = csm_init_جدول->Hi_Pmm_حافظہ + csm_init_جدول->Hi_Pmm_حافظہ_سائز_اندر_بائٹیں;
        e820_جوڑو(hi_pmm_انتہا - BUILD_MIN_HIGHTABLE, BUILD_MIN_HIGHTABLE, E820_RESERVED);
    }

    // For PCIBIOS 1ab10e
    اگر (csm_compat_جدول.Irq_Routing_جدول_اشارہ_گر &&
        csm_compat_جدول.Irq_Routing_جدول_لمبائی) {
        PirAddr = (خالی *)csm_compat_جدول.Irq_Routing_جدول_اشارہ_گر;
        dprintf(3, "CSM PIRQ table at %p\n", PirAddr);
    }

    // For find_resume_vector()... and find_acpi_features()
    اگر (csm_rsdp.signature == RSDP_SIGNATURE) {
        RsdpAddr = &csm_rsdp;
        dprintf(3, "CSM ACPI RSDP at %p\n", RsdpAddr);

        ڈھونڈو_acpi_features();
    }

    // SMBIOS table needs to be copied into the f-seg
    // XX: OVMF doesn't seem to set SmbiosTableLength so don't check it
    اگر (csm_boot_جدول->Smbios_جدول)
        copy_smbios_21((خالی *)csm_boot_جدول->Smbios_جدول);

    // MPTABLE is just there; we don't care where.

    // EFI may have reinitialised the video using its *own* driver.
    enable_vga_console();

    // EFI fills this in for us. Zero it for now...
    ساخت bios_ڈیٹا_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
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
ساکن خالی
handle_csm_0003(ساخت bregs *regs)
{
    اگر (!CONFIG_BOOT) {
        regs->ax = 1;
        واپس;
    }

    dprintf(3, "Boot\n");

    آغاز_Boot();

    regs->ax = 1;
}

/* Legacy16DispatchOprom */
ساکن خالی
handle_csm_0005(ساخت bregs *regs)
{
    EFI_DISPATCH_OPROM_جدول *جدول = MAKE_FLATPTR(regs->es, regs->bx);
    ساخت rom_سرخی *rom;
    u16 bdf;

    اگر (!CONFIG_OPTIONROMS) {
        regs->ax = 1;
        واپس;
    }

    dprintf(3, "Legacy16DispatchOprom rom %p\n", جدول);

    dprintf(3, "OpromSegment   %04x\n", جدول->OpromSegment);
    dprintf(3, "RuntimeSegment %04x\n", جدول->RuntimeSegment);
    dprintf(3, "PnPInstallationCheck %04x:%04x\n",
            جدول->PnPInstallationCheckSegment,
            جدول->Pn_PInstallation_Check_انحراف);
    dprintf(3, "RuntimeSegment %04x\n", جدول->RuntimeSegment);

    rom = MAKE_FLATPTR(جدول->OpromSegment, 0);
    bdf = pci_bus_devfn_مقصد_bdf(جدول->PciBus, جدول->PciDeviceFunction);

    rom_reserve(rom->سائز * 512);

    // XX PnP seg/ofs should never be other than default
    callrom(rom, bdf);

    rom_confirm(rom->سائز * 512);

    regs->bx = 0; // FIXME
    regs->ax = 0;
}

/* Legacy16GetTableAddress */
ساکن خالی
handle_csm_0006(ساخت bregs *regs)
{
    u16 سائز = regs->cx;
    u16 align = regs->dx;
    u16 region = regs->bx; // (1 for F000 seg, 2 for E000 seg, 0 for either)
    خالی *chunk = تہی;

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d\n",
        سائز, align, region);

    اگر (!region)
        region = 3;

    // DX = Required address alignment. Bit mapped.
    // First non-zero bit from the right is the alignment.*/
    اگر (align) {
        align = 1 << __ffs(align);
        اگر (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    } ورنہ {
        align = MALLOC_MIN_ALIGN;
    }

    اگر (region & 2)
        chunk = _malloc(&Zone_کم, سائز, align);
    اگر (!chunk && (region & 1))
        chunk = _malloc(&ZoneFSeg, سائز, align);

    dprintf(3, "Legacy16GetTableAddress size %x align %x region %d yields %p\n",
        سائز, align, region, chunk);
    اگر (chunk) {
        regs->ds = FLATPTR_مقصد_SEG(chunk);
        regs->bx = FLATPTR_مقصد_انحراف(chunk);
        regs->ax = 0;
    } ورنہ {
        regs->ax = 1;
    }
}

خالی VISIBLE32INIT
handle_csm(ساخت bregs *regs)
{
    ASSERT32FLAT();

    اگر (!CONFIG_CSM)
        واپس;

    dprintf(3, "handle_csm regs %p AX=%04x\n", regs, regs->ax);

    رمز_mutable_preinit();
    pic_irqmask_write(PICMask);

    انتخاب(regs->ax) {
    حالت 0000: handle_csm_0000(regs); توڑو;
    حالت 0001: handle_csm_0001(regs); توڑو;
    حالت 0002: handle_csm_0002(regs); توڑو;
    حالت 0003: handle_csm_0003(regs); توڑو;
//    case 0004: handle_csm_0004(regs); break;
    حالت 0005: handle_csm_0005(regs); توڑو;
    حالت 0006: handle_csm_0006(regs); توڑو;
//    case 0007: handle_csm_0007(regs); break;
//    case 0008: hamdle_csm_0008(regs); break;
    طےشدہ: regs->al = 1;
    }

    csm_return(regs);
}

ساکن صحیح csm_prio_مقصد_seabios(u16 csm_prio)
{
    انتخاب (csm_prio) {
    حالت BBS_DO_NOT_BOOT_FROM:
    حالت BBS_IGNORE_اندراج:
        واپس -1;

    حالت BBS_سب_سے_نیچا_ترجیح:
    حالت BBS_UNPRIORITIZED_اندراج:
    طےشدہ:
        // SeaBIOS default priorities start at 1, with 0 being used for
        // an item explicitly selected from interactive_bootmenu().
        // As in find_prio(), add 1 to the value being returned.
        واپس csm_prio + 1;
    }
}

صحیح csm_bootprio_ata(ساخت pci_device *pci, صحیح chanid, صحیح slave)
{
    اگر (!csm_boot_جدول)
        واپس -1;
    BBS_جدول *bbs = (خالی *)csm_boot_جدول->Bbs_جدول;
    صحیح اشاریہ = 1 + (chanid * 2) + slave;
    dprintf(3, "CSM bootprio for ATA%d,%d (index %d) is %d\n", chanid, slave,
            اشاریہ, bbs[اشاریہ].Boot_ترجیح);
    واپس csm_prio_مقصد_seabios(bbs[اشاریہ].Boot_ترجیح);
}

صحیح csm_bootprio_fdc(ساخت pci_device *pci, صحیح پورٹ, صحیح fdid)
{
    اگر (!csm_boot_جدول)
        واپس -1;
    BBS_جدول *bbs = (خالی *)csm_boot_جدول->Bbs_جدول;
    dprintf(3, "CSM bootprio for FDC is %d\n", bbs[0].Boot_ترجیح);
    واپس csm_prio_مقصد_seabios(bbs[0].Boot_ترجیح);
}

صحیح csm_bootprio_pci(ساخت pci_device *pci)
{
    اگر (!csm_boot_جدول)
        واپس -1;
    BBS_جدول *bbs = (خالی *)csm_boot_جدول->Bbs_جدول;
    صحیح i;

    برائے (i = 5; i < csm_boot_جدول->نمبر_Bbs_Entries; i++) {
        اگر (pci->bdf == pci_مقصد_bdf(bbs[i].Bus, bbs[i].Device, bbs[i].Function)) {
            dprintf(3, "CSM bootprio for PCI(%d,%d,%d) is %d\n", bbs[i].Bus,
                    bbs[i].Device, bbs[i].Function, bbs[i].Boot_ترجیح);
            واپس csm_prio_مقصد_seabios(bbs[i].Boot_ترجیح);
        }
    }
    واپس -1;
}
