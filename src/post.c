// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // SET_BDA
#ಅನ್ತರ್ಭಾವ "block.h" // block_setup
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "e820map.h" // e820_add
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // qemu_cfg_preinit
#ಅನ್ತರ್ಭಾವ "fw/xen.h" // xen_preinit
#ಅನ್ತರ್ಭಾವ "hw/pic.h" // pic_setup
#ಅನ್ತರ್ಭಾವ "hw/ps2port.h" // ps2port_setup
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // rtc_write
#ಅನ್ತರ್ಭಾವ "hw/serialio.h" // serial_debug_preinit
#ಅನ್ತರ್ಭಾವ "hw/usb.h" // usb_setup
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_init
#ಅನ್ತರ್ಭಾವ "memmap.h" //ಚಿಹ್ನೆ
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // kbd_init
#ಅನ್ತರ್ಭಾವ "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
ivt_init(ಶೂನ್ಯ)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<256; i++)
        ಗಣ_IVT(i, FUNC16(ನಮೂದು_iret_official));

    // Initialize all hw vectors to a default hw handler.
    ಗಾಗಿ (i=BIOS_HWIRQ0_ಸದಿಶ; i<BIOS_HWIRQ0_ಸದಿಶ+8; i++)
        ಗಣ_IVT(i, FUNC16(ನಮೂದು_hwpic1));
    ಗಾಗಿ (i=BIOS_HWIRQ8_ಸದಿಶ; i<BIOS_HWIRQ8_ಸದಿಶ+8; i++)
        ಗಣ_IVT(i, FUNC16(ನಮೂದು_hwpic2));

    // Initialize software handlers.
    ಗಣ_IVT(0x02, FUNC16(ನಮೂದು_02));
    ಗಣ_IVT(0x05, FUNC16(ನಮೂದು_05));
    ಗಣ_IVT(0x10, FUNC16(ನಮೂದು_10));
    ಗಣ_IVT(0x11, FUNC16(ನಮೂದು_11));
    ಗಣ_IVT(0x12, FUNC16(ನಮೂದು_12));
    ಗಣ_IVT(0x13, FUNC16(ನಮೂದು_13_official));
    ಗಣ_IVT(0x14, FUNC16(ನಮೂದು_14));
    ಗಣ_IVT(0x15, FUNC16(ನಮೂದು_15_official));
    ಗಣ_IVT(0x16, FUNC16(ನಮೂದು_16));
    ಗಣ_IVT(0x17, FUNC16(ನಮೂದು_17));
    ಗಣ_IVT(0x18, FUNC16(ನಮೂದು_18));
    ಗಣ_IVT(0x19, FUNC16(ನಮೂದು_19_official));
    ಗಣ_IVT(0x1a, FUNC16(ನಮೂದು_1a_official));
    ಗಣ_IVT(0x40, FUNC16(ನಮೂದು_40));

    // INT 60h-66h reserved for user interrupt
    ಗಾಗಿ (i=0x60; i<=0x66; i++)
        ಗಣ_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    ಗಣ_IVT(0x79, SEGOFF(0, 0));
}

ಸ್ಥಿರ ಶೂನ್ಯ
bda_init(ಶೂನ್ಯ)
{
    dprintf(3, "init bda\n");

    ರಚನೆ bios_ದತ್ತಾಂಶ_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, ಗಾತ್ರ(*bda));

    ಪೂರ್ಣಾಂಕ esize = EBDA_ಗಾತ್ರ_ಆರಂಭ;
    u16 ebda_seg = EBDA_SEGMENT_ಆರಂಭ;
    ಆದರೆ (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_ಗಮ್ಯ_SEG(ALIGN_ಕೆಳಕ್ಕೆ(ಚಿಹ್ನೆ(final_varlow_ಆರಂಭ), 1024)
                                  - EBDA_ಗಾತ್ರ_ಆರಂಭ*1024);
    ಗಣ_BDA(ebda_seg, ebda_seg);

    ಗಣ_BDA(mem_ಗಾತ್ರ_kb, ebda_seg / (1024/16));

    // Init ebda
    ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, ಗಾತ್ರ(*ebda));
    ebda->ಗಾತ್ರ_2 = esize;

    e820_ಕೂಡಿಸು((u32)ebda, BUILD_LOWRAM_ಕೊನೆ-(u32)ebda, E820_RESERVED);

    // Init extra stack
    ರಾಶಿ_Pos = &Extra_ರಾಶಿ[BUILD_EXTRA_ರಾಶಿ_ಗಾತ್ರ] - ಚಿಹ್ನೆ(zonelow_ತಳ);
}

ಶೂನ್ಯ
interface_init(ಶೂನ್ಯ)
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
ಶೂನ್ಯ
device_hardware_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    usb_ಸಿದ್ಧತೆ();
    ps2port_ಸಿದ್ಧತೆ();
    ಖಂಡ_ಸಿದ್ಧತೆ();
    lpt_ಸಿದ್ಧತೆ();
    serial_ಸಿದ್ಧತೆ();
    cbfs_payload_ಸಿದ್ಧತೆ();
}

ಸ್ಥಿರ ಶೂನ್ಯ
platform_hardware_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    // Make sure legacy DMA isn't running.
    dma_ಸಿದ್ಧತೆ();

    // Init base pc hardware.
    pic_ಸಿದ್ಧತೆ();
    thread_ಸಿದ್ಧತೆ();
    mathcp_ಸಿದ್ಧತೆ();

    // Platform specific setup
    qemu_platform_ಸಿದ್ಧತೆ();
    coreboot_platform_ಸಿದ್ಧತೆ();

    // Setup timers and periodic clock interrupt
    timer_ಸಿದ್ಧತೆ();
    clock_ಸಿದ್ಧತೆ();

    // Initialize TPM
    tpm_ಸಿದ್ಧತೆ();
}

ಶೂನ್ಯ
prepareboot(ಶೂನ್ಯ)
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

    Have_ಸತತ_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_ಗಾತ್ರ);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
ಶೂನ್ಯ VISIBLE32FLAT
ಆರಂಭ_Boot(ಶೂನ್ಯ)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((ಶೂನ್ಯ*)BUILD_ರಾಶಿ_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_ರಾಶಿ_ADDR);

    dprintf(3, "Jump to int19\n");
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
ಸ್ಥಿರ ಶೂನ್ಯ
maininit(ಶೂನ್ಯ)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_ಸಿದ್ಧತೆ();

    // Start hardware initialization (if threads allowed during optionroms)
    ಆದರೆ (ಎಳೆಗಳು_during_optionroms())
        device_hardware_ಸಿದ್ಧತೆ();

    // Run vga option rom
    vgarom_ಸಿದ್ಧತೆ();
    sercon_ಸಿದ್ಧತೆ();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    ಆದರೆ (!ಎಳೆಗಳು_during_optionroms()) {
        device_hardware_ಸಿದ್ಧತೆ();
        wait_ಎಳೆಗಳು();
    }

    // Run option roms
    optionrom_ಸಿದ್ಧತೆ();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_ಎಳೆಗಳು();

    // Prepare for boot.
    prepareboot();

    // Write protect bios memory.
    make_bios_readonly();

    // Invoke int 19 to start boot process.
    ಆರಂಭ_Boot();
}


/****************************************************************
 * POST entry and code relocation
 ****************************************************************/

// Update given relocs for the code at 'dest' with a given 'delta'
ಸ್ಥಿರ ಶೂನ್ಯ
updateRelocs(ಶೂನ್ಯ *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    ಗಾಗಿ (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
ಶೂನ್ಯ __noreturn
reloc_preinit(ಶೂನ್ಯ *f, ಶೂನ್ಯ *arg)
{
    ಶೂನ್ಯ (*func)(ಶೂನ್ಯ *) __noreturn = f;
    ಆದರೆ (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = ಚಿಹ್ನೆ(code32init_ಕೊನೆ) - ಚಿಹ್ನೆ(code32init_ಆರಂಭ);
    u32 codealign = ಚಿಹ್ನೆ(_reloc_min_align);
    ಶೂನ್ಯ *codedest = memalign_tmp(codealign, initsize);
    ಶೂನ್ಯ *codesrc = VSYMBOL(code32init_ಆರಂಭ);
    ಆದರೆ (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    ಸ್ಮರಣೆ_ನಕಲು(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(reloc_abs_ಆರಂಭ), VSYMBOL(reloc_abs_ಕೊನೆ)
                 , delta);
    updateRelocs(codedest, VSYMBOL(reloc_rel_ಆರಂಭ), VSYMBOL(reloc_rel_ಕೊನೆ)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_ಆರಂಭ), VSYMBOL(reloc_init_ಆರಂಭ)
                 , VSYMBOL(reloc_init_ಕೊನೆ), delta);
    ಆದರೆ (f >= codesrc && f < VSYMBOL(code32init_ಕೊನೆ))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
ಶೂನ್ಯ
ಸಂಕೇತ_mutable_preinit(ಶೂನ್ಯ)
{
    ಆದರೆ (Have_ಸತತ_Post)
        // Already run
        ಹಿಂತಿರುಗಿ;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_ಸಂಕೇತ, 0);
    barrier();
    Have_ಸತತ_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
ಶೂನ್ಯ VISIBLE32INIT
dopost(ಶೂನ್ಯ)
{
    ಸಂಕೇತ_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, ಖಾಲಿ);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
ಶೂನ್ಯ VISIBLE32FLAT
handle_post(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU && !CONFIG_COREBOOT)
        ಹಿಂತಿರುಗಿ;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
