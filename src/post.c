// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // SET_BDA
#شامل "block.h" // block_setup
#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "e820map.h" // e820_add
#شامل "fw/paravirt.h" // qemu_cfg_preinit
#شامل "fw/xen.h" // xen_preinit
#شامل "hw/pic.h" // pic_setup
#شامل "hw/ps2port.h" // ps2port_setup
#شامل "hw/rtc.h" // rtc_write
#شامل "hw/serialio.h" // serial_debug_preinit
#شامل "hw/usb.h" // usb_setup
#شامل "malloc.h" // malloc_init
#شامل "memmap.h" //علامت
#شامل "output.h" // dprintf
#شامل "string.h" // memset
#شامل "util.h" // kbd_init
#شامل "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

ساکن خالی
ivt_init(خالی)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    صحیح i;
    برائے (i=0; i<256; i++)
        سیٹ_IVT(i, FUNC16(اندراج_iret_official));

    // Initialize all hw vectors to a default hw handler.
    برائے (i=BIOS_HWIRQ0_سمتیہ; i<BIOS_HWIRQ0_سمتیہ+8; i++)
        سیٹ_IVT(i, FUNC16(اندراج_hwpic1));
    برائے (i=BIOS_HWIRQ8_سمتیہ; i<BIOS_HWIRQ8_سمتیہ+8; i++)
        سیٹ_IVT(i, FUNC16(اندراج_hwpic2));

    // Initialize software handlers.
    سیٹ_IVT(0x02, FUNC16(اندراج_02));
    سیٹ_IVT(0x05, FUNC16(اندراج_05));
    سیٹ_IVT(0x10, FUNC16(اندراج_10));
    سیٹ_IVT(0x11, FUNC16(اندراج_11));
    سیٹ_IVT(0x12, FUNC16(اندراج_12));
    سیٹ_IVT(0x13, FUNC16(اندراج_13_official));
    سیٹ_IVT(0x14, FUNC16(اندراج_14));
    سیٹ_IVT(0x15, FUNC16(اندراج_15_official));
    سیٹ_IVT(0x16, FUNC16(اندراج_16));
    سیٹ_IVT(0x17, FUNC16(اندراج_17));
    سیٹ_IVT(0x18, FUNC16(اندراج_18));
    سیٹ_IVT(0x19, FUNC16(اندراج_19_official));
    سیٹ_IVT(0x1a, FUNC16(اندراج_1a_official));
    سیٹ_IVT(0x40, FUNC16(اندراج_40));

    // INT 60h-66h reserved for user interrupt
    برائے (i=0x60; i<=0x66; i++)
        سیٹ_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    سیٹ_IVT(0x79, SEGOFF(0, 0));
}

ساکن خالی
bda_init(خالی)
{
    dprintf(3, "init bda\n");

    ساخت bios_ڈیٹا_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, حجم(*bda));

    صحیح esize = EBDA_سائز_آغاز;
    u16 ebda_seg = EBDA_SEGMENT_آغاز;
    اگر (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_مقصد_SEG(ALIGN_نیچے_کو(علامت(final_varlow_آغاز), 1024)
                                  - EBDA_سائز_آغاز*1024);
    سیٹ_BDA(ebda_seg, ebda_seg);

    سیٹ_BDA(mem_سائز_kb, ebda_seg / (1024/16));

    // Init ebda
    ساخت extended_bios_ڈیٹا_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, حجم(*ebda));
    ebda->سائز = esize;

    e820_جوڑو((u32)ebda, BUILD_LOWRAM_انتہا-(u32)ebda, E820_RESERVED);

    // Init extra stack
    ڈھیر_Pos = &Extra_ڈھیر[BUILD_EXTRA_ڈھیر_سائز] - علامت(zonelow_بنیاد);
}

خالی
interface_init(خالی)
{
    // Running at new code address - do code relocation fixups
    malloc_init();

    // Setup romfile items.
    qemu_cfg_init();
    coreboot_cbfs_init();
    multiboot_init();

    // Setup ivt/bda/ebda
    ivt_init();
    bda_init();

    // Other interfaces
    boot_init();
    bios32_init();
    pmm_init();
    pnp_init();
    kbd_init();
    mouse_init();
}

// Initialize hardware devices
خالی
device_hardware_تیاری(خالی)
{
    usb_تیاری();
    ps2port_تیاری();
    بلاک_تیاری();
    lpt_تیاری();
    serial_تیاری();
    cbfs_payload_تیاری();
}

ساکن خالی
platform_hardware_تیاری(خالی)
{
    // Make sure legacy DMA isn't running.
    dma_تیاری();

    // Init base pc hardware.
    pic_تیاری();
    thread_تیاری();
    mathcp_تیاری();

    // Platform specific setup
    qemu_platform_تیاری();
    coreboot_platform_تیاری();

    // Setup timers and periodic clock interrupt
    timer_تیاری();
    clock_تیاری();

    // Initialize TPM
    tpm_تیاری();
}

خالی
prepareboot(خالی)
{
    // Change TPM phys. presence state befor leaving BIOS
    tpm_prepboot();

    // Run BCVs
    bcv_prepboot();

    // Finalize data structures before boot
    cdrom_prepboot();
    pmm_prepboot();
    malloc_prepboot();
    e820_prepboot();

    Have_تسلسل_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_سائز);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
خالی VISIBLE32FLAT
آغاز_Boot(خالی)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((خالی*)BUILD_ڈھیر_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_ڈھیر_ADDR);

    dprintf(3, "Jump to int19\n");
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
ساکن خالی
maininit(خالی)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_تیاری();

    // Start hardware initialization (if threads allowed during optionroms)
    اگر (دھاگے_during_optionroms())
        device_hardware_تیاری();

    // Run vga option rom
    vgarom_تیاری();
    sercon_تیاری();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    اگر (!دھاگے_during_optionroms()) {
        device_hardware_تیاری();
        wait_دھاگے();
    }

    // Run option roms
    optionrom_تیاری();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_دھاگے();

    // Prepare for boot.
    prepareboot();

    // Write protect bios memory.
    make_bios_readonly();

    // Invoke int 19 to start boot process.
    آغاز_Boot();
}


/****************************************************************
 * POST entry and code relocation
 ****************************************************************/

// Update given relocs for the code at 'dest' with a given 'delta'
ساکن خالی
updateRelocs(خالی *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    برائے (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
خالی __noreturn
reloc_preinit(خالی *f, خالی *arg)
{
    خالی (*func)(خالی *) __noreturn = f;
    اگر (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = علامت(code32init_انتہا) - علامت(code32init_آغاز);
    u32 codealign = علامت(_reloc_min_align);
    خالی *codedest = memalign_tmp(codealign, initsize);
    خالی *codesrc = VSYMBOL(code32init_آغاز);
    اگر (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    حافظہ_نقل(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(reloc_abs_آغاز), VSYMBOL(reloc_abs_انتہا)
                 , delta);
    updateRelocs(codedest, VSYMBOL(reloc_rel_آغاز), VSYMBOL(reloc_rel_انتہا)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_آغاز), VSYMBOL(reloc_init_آغاز)
                 , VSYMBOL(reloc_init_انتہا), delta);
    اگر (f >= codesrc && f < VSYMBOL(code32init_انتہا))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
خالی
رمز_mutable_preinit(خالی)
{
    اگر (Have_تسلسل_Post)
        // Already run
        واپس;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_رمز, 0);
    barrier();
    Have_تسلسل_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
خالی VISIBLE32INIT
dopost(خالی)
{
    رمز_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, تہی);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
خالی VISIBLE32FLAT
handle_post(خالی)
{
    اگر (!CONFIG_QEMU && !CONFIG_COREBOOT)
        واپس;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
