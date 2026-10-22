// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // SET_BDA
#অন্তর্ভাৱ "block.h" // block_setup
#অন্তর্ভাৱ "bregs.h" // struct bregs
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "e820map.h" // e820_add
#অন্তর্ভাৱ "fw/paravirt.h" // qemu_cfg_preinit
#অন্তর্ভাৱ "fw/xen.h" // xen_preinit
#অন্তর্ভাৱ "hw/pic.h" // pic_setup
#অন্তর্ভাৱ "hw/ps2port.h" // ps2port_setup
#অন্তর্ভাৱ "hw/rtc.h" // rtc_write
#অন্তর্ভাৱ "hw/serialio.h" // serial_debug_preinit
#অন্তর্ভাৱ "hw/usb.h" // usb_setup
#অন্তর্ভাৱ "malloc.h" // malloc_init
#অন্তর্ভাৱ "memmap.h" //প্ৰতীক
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // kbd_init
#অন্তর্ভাৱ "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

স্থির শূন্য
ivt_init(শূন্য)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<256; i++)
        SET_IVT(i, FUNC16(entry_iret_official));

    // Initialize all hw vectors to a default hw handler.
    বাবে (i=BIOS_HWIRQ0_VECTOR; i<BIOS_HWIRQ0_VECTOR+8; i++)
        SET_IVT(i, FUNC16(entry_hwpic1));
    বাবে (i=BIOS_HWIRQ8_VECTOR; i<BIOS_HWIRQ8_VECTOR+8; i++)
        SET_IVT(i, FUNC16(entry_hwpic2));

    // Initialize software handlers.
    SET_IVT(0x02, FUNC16(entry_02));
    SET_IVT(0x05, FUNC16(entry_05));
    SET_IVT(0x10, FUNC16(entry_10));
    SET_IVT(0x11, FUNC16(entry_11));
    SET_IVT(0x12, FUNC16(entry_12));
    SET_IVT(0x13, FUNC16(entry_13_official));
    SET_IVT(0x14, FUNC16(entry_14));
    SET_IVT(0x15, FUNC16(entry_15_official));
    SET_IVT(0x16, FUNC16(entry_16));
    SET_IVT(0x17, FUNC16(entry_17));
    SET_IVT(0x18, FUNC16(entry_18));
    SET_IVT(0x19, FUNC16(entry_19_official));
    SET_IVT(0x1a, FUNC16(entry_1a_official));
    SET_IVT(0x40, FUNC16(entry_40));

    // INT 60h-66h reserved for user interrupt
    বাবে (i=0x60; i<=0x66; i++)
        SET_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    SET_IVT(0x79, SEGOFF(0, 0));
}

স্থির শূন্য
bda_init(শূন্য)
{
    dprintf(3, "init bda\n");

    সংরচনা bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, পরিমাণ(*bda));

    পূর্ণাঙ্ক esize = EBDA_SIZE_START;
    u16 ebda_seg = EBDA_SEGMENT_START;
    যদি (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_TO_SEG(ALIGN_DOWN(প্ৰতীক(final_varlow_start), 1024)
                                  - EBDA_SIZE_START*1024);
    SET_BDA(ebda_seg, ebda_seg);

    SET_BDA(mem_size_kb, ebda_seg / (1024/16));

    // Init ebda
    সংরচনা extended_bios_data_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, পরিমাণ(*ebda));
    ebda->size = esize;

    e820_add((u32)ebda, BUILD_LOWRAM_END-(u32)ebda, E820_RESERVED);

    // Init extra stack
    স্তূপ_Pos = &Extra_স্তূপ[BUILD_EXTRA_স্তূপ_SIZE] - প্ৰতীক(zonelow_base);
}

শূন্য
interface_init(শূন্য)
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
শূন্য
device_hardware_setup(শূন্য)
{
    usb_setup();
    ps2port_setup();
    block_setup();
    lpt_setup();
    serial_setup();
    cbfs_payload_setup();
}

স্থির শূন্য
platform_hardware_setup(শূন্য)
{
    // Make sure legacy DMA isn't running.
    dma_setup();

    // Init base pc hardware.
    pic_setup();
    thread_setup();
    mathcp_setup();

    // Platform specific setup
    qemu_platform_setup();
    coreboot_platform_setup();

    // Setup timers and periodic clock interrupt
    timer_setup();
    clock_setup();

    // Initialize TPM
    tpm_setup();
}

শূন্য
prepareboot(শূন্য)
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

    HaveRunPost = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_SIZE);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
শূন্য VISIBLE32FLAT
startBoot(শূন্য)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((শূন্য*)BUILD_স্তূপ_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_স্তূপ_ADDR);

    dprintf(3, "Jump to int19\n");
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
স্থির শূন্য
maininit(শূন্য)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_setup();

    // Start hardware initialization (if threads allowed during optionroms)
    যদি (threads_during_optionroms())
        device_hardware_setup();

    // Run vga option rom
    vgarom_setup();
    sercon_setup();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    যদি (!threads_during_optionroms()) {
        device_hardware_setup();
        wait_threads();
    }

    // Run option roms
    optionrom_setup();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_threads();

    // Prepare for boot.
    prepareboot();

    // Write protect bios memory.
    make_bios_readonly();

    // Invoke int 19 to start boot process.
    startBoot();
}


/****************************************************************
 * POST entry and code relocation
 ****************************************************************/

// Update given relocs for the code at 'dest' with a given 'delta'
স্থির শূন্য
updateRelocs(শূন্য *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    বাবে (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
শূন্য __noreturn
reloc_preinit(শূন্য *f, শূন্য *arg)
{
    শূন্য (*func)(শূন্য *) __noreturn = f;
    যদি (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = প্ৰতীক(code32init_end) - প্ৰতীক(code32init_start);
    u32 codealign = প্ৰতীক(_reloc_min_align);
    শূন্য *codedest = memalign_tmp(codealign, initsize);
    শূন্য *codesrc = VSYMBOL(code32init_start);
    যদি (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    memcpy(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(_reloc_abs_start), VSYMBOL(_reloc_abs_end)
                 , delta);
    updateRelocs(codedest, VSYMBOL(_reloc_rel_start), VSYMBOL(_reloc_rel_end)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_start), VSYMBOL(_reloc_init_start)
                 , VSYMBOL(_reloc_init_end), delta);
    যদি (f >= codesrc && f < VSYMBOL(code32init_end))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
শূন্য
code_mutable_preinit(শূন্য)
{
    যদি (HaveRunPost)
        // Already run
        ঘূৰাই;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_CODE, 0);
    barrier();
    HaveRunPost = 1;
    barrier();
}

// Setup for code relocation and then relocate.
শূন্য VISIBLE32INIT
dopost(শূন্য)
{
    code_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, রিক্ত);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
শূন্য VISIBLE32FLAT
handle_post(শূন্য)
{
    যদি (!CONFIG_QEMU && !CONFIG_COREBOOT)
        ঘূৰাই;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
