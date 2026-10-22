// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // SET_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // block_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // e820_add
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // qemu_cfg_preinit
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/xen.h" // xen_preinit
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pic.h" // pic_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ps2port.h" // ps2port_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // rtc_write
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/serialio.h" // serial_debug_preinit
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/usb.h" // usb_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_init
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" //ਚਿੰਨ੍ਹ
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // kbd_init
#ਅਨ੍ਤਰ੍ਭਾਵ "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ
ivt_init(ਖਾਲੀ)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    ਪੂਰਨ i;
    ਲਈ (i=0; i<256; i++)
        ਸੈੱਟ_IVT(i, FUNC16(ਇੰਦਰਾਜ_iret_official));

    // Initialize all hw vectors to a default hw handler.
    ਲਈ (i=BIOS_HWIRQ0_ਸਦਿਸ਼; i<BIOS_HWIRQ0_ਸਦਿਸ਼+8; i++)
        ਸੈੱਟ_IVT(i, FUNC16(ਇੰਦਰਾਜ_hwpic1));
    ਲਈ (i=BIOS_HWIRQ8_ਸਦਿਸ਼; i<BIOS_HWIRQ8_ਸਦਿਸ਼+8; i++)
        ਸੈੱਟ_IVT(i, FUNC16(ਇੰਦਰਾਜ_hwpic2));

    // Initialize software handlers.
    ਸੈੱਟ_IVT(0x02, FUNC16(ਇੰਦਰਾਜ_02));
    ਸੈੱਟ_IVT(0x05, FUNC16(ਇੰਦਰਾਜ_05));
    ਸੈੱਟ_IVT(0x10, FUNC16(ਇੰਦਰਾਜ_10));
    ਸੈੱਟ_IVT(0x11, FUNC16(ਇੰਦਰਾਜ_11));
    ਸੈੱਟ_IVT(0x12, FUNC16(ਇੰਦਰਾਜ_12));
    ਸੈੱਟ_IVT(0x13, FUNC16(ਇੰਦਰਾਜ_13_official));
    ਸੈੱਟ_IVT(0x14, FUNC16(ਇੰਦਰਾਜ_14));
    ਸੈੱਟ_IVT(0x15, FUNC16(ਇੰਦਰਾਜ_15_official));
    ਸੈੱਟ_IVT(0x16, FUNC16(ਇੰਦਰਾਜ_16));
    ਸੈੱਟ_IVT(0x17, FUNC16(ਇੰਦਰਾਜ_17));
    ਸੈੱਟ_IVT(0x18, FUNC16(ਇੰਦਰਾਜ_18));
    ਸੈੱਟ_IVT(0x19, FUNC16(ਇੰਦਰਾਜ_19_official));
    ਸੈੱਟ_IVT(0x1a, FUNC16(ਇੰਦਰਾਜ_1a_official));
    ਸੈੱਟ_IVT(0x40, FUNC16(ਇੰਦਰਾਜ_40));

    // INT 60h-66h reserved for user interrupt
    ਲਈ (i=0x60; i<=0x66; i++)
        ਸੈੱਟ_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    ਸੈੱਟ_IVT(0x79, SEGOFF(0, 0));
}

ਸਥਿਰ ਖਾਲੀ
bda_init(ਖਾਲੀ)
{
    dprintf(3, "init bda\n");

    ਬਣਤਰ bios_ਅੰਕੜੇ_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, ਆਕਾਰ(*bda));

    ਪੂਰਨ esize = EBDA_ਮਾਪ_ਸ਼ੁਰੂ;
    u16 ebda_seg = EBDA_SEGMENT_ਸ਼ੁਰੂ;
    ਜੇ (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_ਮੰਜ਼ਿਲ_SEG(ALIGN_ਥੱਲੇ(ਚਿੰਨ੍ਹ(final_varlow_ਸ਼ੁਰੂ), 1024)
                                  - EBDA_ਮਾਪ_ਸ਼ੁਰੂ*1024);
    ਸੈੱਟ_BDA(ebda_seg, ebda_seg);

    ਸੈੱਟ_BDA(mem_ਮਾਪ_kb, ebda_seg / (1024/16));

    // Init ebda
    ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, ਆਕਾਰ(*ebda));
    ebda->ਮਾਪ = esize;

    e820_ਜੋੜੋ((u32)ebda, BUILD_LOWRAM_ਸਿਰਾ-(u32)ebda, E820_RESERVED);

    // Init extra stack
    ਢੇਰ_Pos = &Extra_ਢੇਰ[BUILD_EXTRA_ਢੇਰ_ਮਾਪ] - ਚਿੰਨ੍ਹ(zonelow_ਅਧਾਰ);
}

ਖਾਲੀ
interface_init(ਖਾਲੀ)
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
ਖਾਲੀ
device_hardware_ਤਿਆਰੀ(ਖਾਲੀ)
{
    usb_ਤਿਆਰੀ();
    ps2port_ਤਿਆਰੀ();
    ਖੰਡ_ਤਿਆਰੀ();
    lpt_ਤਿਆਰੀ();
    serial_ਤਿਆਰੀ();
    cbfs_payload_ਤਿਆਰੀ();
}

ਸਥਿਰ ਖਾਲੀ
platform_hardware_ਤਿਆਰੀ(ਖਾਲੀ)
{
    // Make sure legacy DMA isn't running.
    dma_ਤਿਆਰੀ();

    // Init base pc hardware.
    pic_ਤਿਆਰੀ();
    thread_ਤਿਆਰੀ();
    mathcp_ਤਿਆਰੀ();

    // Platform specific setup
    qemu_platform_ਤਿਆਰੀ();
    coreboot_platform_ਤਿਆਰੀ();

    // Setup timers and periodic clock interrupt
    timer_ਤਿਆਰੀ();
    clock_ਤਿਆਰੀ();

    // Initialize TPM
    tpm_ਤਿਆਰੀ();
}

ਖਾਲੀ
prepareboot(ਖਾਲੀ)
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

    Have_ਲੜੀਵਾਰ_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_ਮਾਪ);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
ਖਾਲੀ VISIBLE32FLAT
ਸ਼ੁਰੂ_Boot(ਖਾਲੀ)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((ਖਾਲੀ*)BUILD_ਢੇਰ_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_ਢੇਰ_ADDR);

    dprintf(3, "Jump to int19\n");
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
ਸਥਿਰ ਖਾਲੀ
maininit(ਖਾਲੀ)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_ਤਿਆਰੀ();

    // Start hardware initialization (if threads allowed during optionroms)
    ਜੇ (ਧਾਗੇ_during_optionroms())
        device_hardware_ਤਿਆਰੀ();

    // Run vga option rom
    vgarom_ਤਿਆਰੀ();
    sercon_ਤਿਆਰੀ();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    ਜੇ (!ਧਾਗੇ_during_optionroms()) {
        device_hardware_ਤਿਆਰੀ();
        wait_ਧਾਗੇ();
    }

    // Run option roms
    optionrom_ਤਿਆਰੀ();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_ਧਾਗੇ();

    // Prepare for boot.
    prepareboot();

    // Write protect bios memory.
    make_bios_readonly();

    // Invoke int 19 to start boot process.
    ਸ਼ੁਰੂ_Boot();
}


/****************************************************************
 * POST entry and code relocation
 ****************************************************************/

// Update given relocs for the code at 'dest' with a given 'delta'
ਸਥਿਰ ਖਾਲੀ
updateRelocs(ਖਾਲੀ *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    ਲਈ (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
ਖਾਲੀ __noreturn
reloc_preinit(ਖਾਲੀ *f, ਖਾਲੀ *arg)
{
    ਖਾਲੀ (*func)(ਖਾਲੀ *) __noreturn = f;
    ਜੇ (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = ਚਿੰਨ੍ਹ(code32init_ਸਿਰਾ) - ਚਿੰਨ੍ਹ(code32init_ਸ਼ੁਰੂ);
    u32 codealign = ਚਿੰਨ੍ਹ(_reloc_min_align);
    ਖਾਲੀ *codedest = memalign_tmp(codealign, initsize);
    ਖਾਲੀ *codesrc = VSYMBOL(code32init_ਸ਼ੁਰੂ);
    ਜੇ (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(reloc_abs_ਸ਼ੁਰੂ), VSYMBOL(reloc_abs_ਸਿਰਾ)
                 , delta);
    updateRelocs(codedest, VSYMBOL(reloc_rel_ਸ਼ੁਰੂ), VSYMBOL(reloc_rel_ਸਿਰਾ)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_ਸ਼ੁਰੂ), VSYMBOL(reloc_init_ਸ਼ੁਰੂ)
                 , VSYMBOL(reloc_init_ਸਿਰਾ), delta);
    ਜੇ (f >= codesrc && f < VSYMBOL(code32init_ਸਿਰਾ))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
ਖਾਲੀ
ਕੋਡ_mutable_preinit(ਖਾਲੀ)
{
    ਜੇ (Have_ਲੜੀਵਾਰ_Post)
        // Already run
        ਵਾਪਸ;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_ਕੋਡ, 0);
    barrier();
    Have_ਲੜੀਵਾਰ_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
ਖਾਲੀ VISIBLE32INIT
dopost(ਖਾਲੀ)
{
    ਕੋਡ_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, ਨਿਰਾ);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
ਖਾਲੀ VISIBLE32FLAT
handle_post(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU && !CONFIG_COREBOOT)
        ਵਾਪਸ;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
