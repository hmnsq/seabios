// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // SET_BDA
#அந்தர்பாவ "block.h" // block_setup
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "e820map.h" // e820_add
#அந்தர்பாவ "fw/paravirt.h" // qemu_cfg_preinit
#அந்தர்பாவ "fw/xen.h" // xen_preinit
#அந்தர்பாவ "hw/pic.h" // pic_setup
#அந்தர்பாவ "hw/ps2port.h" // ps2port_setup
#அந்தர்பாவ "hw/rtc.h" // rtc_write
#அந்தர்பாவ "hw/serialio.h" // serial_debug_preinit
#அந்தர்பாவ "hw/usb.h" // usb_setup
#அந்தர்பாவ "malloc.h" // malloc_init
#அந்தர்பாவ "memmap.h" //குறியீடு
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // kbd_init
#அந்தர்பாவ "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

நிலையான வெறுமை
ivt_init(வெறுமை)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    முழு i;
    க்கு (i=0; i<256; i++)
        SET_IVT(i, FUNC16(entry_iret_official));

    // Initialize all hw vectors to a default hw handler.
    க்கு (i=BIOS_HWIRQ0_VECTOR; i<BIOS_HWIRQ0_VECTOR+8; i++)
        SET_IVT(i, FUNC16(entry_hwpic1));
    க்கு (i=BIOS_HWIRQ8_VECTOR; i<BIOS_HWIRQ8_VECTOR+8; i++)
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
    க்கு (i=0x60; i<=0x66; i++)
        SET_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    SET_IVT(0x79, SEGOFF(0, 0));
}

நிலையான வெறுமை
bda_init(வெறுமை)
{
    dprintf(3, "init bda\n");

    கட்டமைப்பு bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, அளவு(*bda));

    முழு esize = EBDA_SIZE_START;
    u16 ebda_seg = EBDA_SEGMENT_START;
    எனில் (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_TO_SEG(ALIGN_DOWN(குறியீடு(final_varlow_start), 1024)
                                  - EBDA_SIZE_START*1024);
    SET_BDA(ebda_seg, ebda_seg);

    SET_BDA(mem_size_kb, ebda_seg / (1024/16));

    // Init ebda
    கட்டமைப்பு extended_bios_data_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, அளவு(*ebda));
    ebda->size = esize;

    e820_add((u32)ebda, BUILD_LOWRAM_END-(u32)ebda, E820_RESERVED);

    // Init extra stack
    அடுக்கு_Pos = &Extra_அடுக்கு[BUILD_EXTRA_அடுக்கு_SIZE] - குறியீடு(zonelow_base);
}

வெறுமை
interface_init(வெறுமை)
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
வெறுமை
device_hardware_setup(வெறுமை)
{
    usb_setup();
    ps2port_setup();
    block_setup();
    lpt_setup();
    serial_setup();
    cbfs_payload_setup();
}

நிலையான வெறுமை
platform_hardware_setup(வெறுமை)
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

வெறுமை
prepareboot(வெறுமை)
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
வெறுமை VISIBLE32FLAT
startBoot(வெறுமை)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((வெறுமை*)BUILD_அடுக்கு_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_அடுக்கு_ADDR);

    dprintf(3, "Jump to int19\n");
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
நிலையான வெறுமை
maininit(வெறுமை)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_setup();

    // Start hardware initialization (if threads allowed during optionroms)
    எனில் (threads_during_optionroms())
        device_hardware_setup();

    // Run vga option rom
    vgarom_setup();
    sercon_setup();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    எனில் (!threads_during_optionroms()) {
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
நிலையான வெறுமை
updateRelocs(வெறுமை *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    க்கு (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
வெறுமை __noreturn
reloc_preinit(வெறுமை *f, வெறுமை *arg)
{
    வெறுமை (*func)(வெறுமை *) __noreturn = f;
    எனில் (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = குறியீடு(code32init_end) - குறியீடு(code32init_start);
    u32 codealign = குறியீடு(_reloc_min_align);
    வெறுமை *codedest = memalign_tmp(codealign, initsize);
    வெறுமை *codesrc = VSYMBOL(code32init_start);
    எனில் (!codedest)
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
    எனில் (f >= codesrc && f < VSYMBOL(code32init_end))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
வெறுமை
code_mutable_preinit(வெறுமை)
{
    எனில் (HaveRunPost)
        // Already run
        திரும்பு;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_CODE, 0);
    barrier();
    HaveRunPost = 1;
    barrier();
}

// Setup for code relocation and then relocate.
வெறுமை VISIBLE32INIT
dopost(வெறுமை)
{
    code_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, வெற்று);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
வெறுமை VISIBLE32FLAT
handle_post(வெறுமை)
{
    எனில் (!CONFIG_QEMU && !CONFIG_COREBOOT)
        திரும்பு;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
