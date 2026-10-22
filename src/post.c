// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // SET_BDA
#അന്തര്ഭാവ "block.h" // block_setup
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "e820map.h" // e820_add
#അന്തര്ഭാവ "fw/paravirt.h" // qemu_cfg_preinit
#അന്തര്ഭാവ "fw/xen.h" // xen_preinit
#അന്തര്ഭാവ "hw/pic.h" // pic_setup
#അന്തര്ഭാവ "hw/ps2port.h" // ps2port_setup
#അന്തര്ഭാവ "hw/rtc.h" // rtc_write
#അന്തര്ഭാവ "hw/serialio.h" // serial_debug_preinit
#അന്തര്ഭാവ "hw/usb.h" // usb_setup
#അന്തര്ഭാവ "malloc.h" // malloc_init
#അന്തര്ഭാവ "memmap.h" //ചിഹ്നം
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // kbd_init
#അന്തര്ഭാവ "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

സ്ഥിതം ശൂന്യം
ivt_init(ശൂന്യം)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<256; i++)
        SET_IVT(i, FUNC16(entry_iret_official));

    // Initialize all hw vectors to a default hw handler.
    വേണ്ടി (i=BIOS_HWIRQ0_VECTOR; i<BIOS_HWIRQ0_VECTOR+8; i++)
        SET_IVT(i, FUNC16(entry_hwpic1));
    വേണ്ടി (i=BIOS_HWIRQ8_VECTOR; i<BIOS_HWIRQ8_VECTOR+8; i++)
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
    വേണ്ടി (i=0x60; i<=0x66; i++)
        SET_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    SET_IVT(0x79, SEGOFF(0, 0));
}

സ്ഥിതം ശൂന്യം
bda_init(ശൂന്യം)
{
    dprintf(3, "init bda\n");

    ഘടന bios_data_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, വലുപ്പം(*bda));

    പൂർണ്ണസംഖ്യ esize = EBDA_SIZE_START;
    u16 ebda_seg = EBDA_SEGMENT_START;
    എങ്കിൽ (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_TO_SEG(ALIGN_DOWN(ചിഹ്നം(final_varlow_start), 1024)
                                  - EBDA_SIZE_START*1024);
    SET_BDA(ebda_seg, ebda_seg);

    SET_BDA(mem_size_kb, ebda_seg / (1024/16));

    // Init ebda
    ഘടന extended_bios_data_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, വലുപ്പം(*ebda));
    ebda->size = esize;

    e820_add((u32)ebda, BUILD_LOWRAM_END-(u32)ebda, E820_RESERVED);

    // Init extra stack
    അട്ടി_Pos = &Extra_അട്ടി[BUILD_EXTRA_അട്ടി_SIZE] - ചിഹ്നം(zonelow_base);
}

ശൂന്യം
interface_init(ശൂന്യം)
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
ശൂന്യം
device_hardware_setup(ശൂന്യം)
{
    usb_setup();
    ps2port_setup();
    block_setup();
    lpt_setup();
    serial_setup();
    cbfs_payload_setup();
}

സ്ഥിതം ശൂന്യം
platform_hardware_setup(ശൂന്യം)
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

ശൂന്യം
prepareboot(ശൂന്യം)
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
ശൂന്യം VISIBLE32FLAT
startBoot(ശൂന്യം)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((ശൂന്യം*)BUILD_അട്ടി_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_അട്ടി_ADDR);

    dprintf(3, "Jump to int19\n");
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
സ്ഥിതം ശൂന്യം
maininit(ശൂന്യം)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_setup();

    // Start hardware initialization (if threads allowed during optionroms)
    എങ്കിൽ (threads_during_optionroms())
        device_hardware_setup();

    // Run vga option rom
    vgarom_setup();
    sercon_setup();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    എങ്കിൽ (!threads_during_optionroms()) {
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
സ്ഥിതം ശൂന്യം
updateRelocs(ശൂന്യം *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    വേണ്ടി (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
ശൂന്യം __noreturn
reloc_preinit(ശൂന്യം *f, ശൂന്യം *arg)
{
    ശൂന്യം (*func)(ശൂന്യം *) __noreturn = f;
    എങ്കിൽ (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = ചിഹ്നം(code32init_end) - ചിഹ്നം(code32init_start);
    u32 codealign = ചിഹ്നം(_reloc_min_align);
    ശൂന്യം *codedest = memalign_tmp(codealign, initsize);
    ശൂന്യം *codesrc = VSYMBOL(code32init_start);
    എങ്കിൽ (!codedest)
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
    എങ്കിൽ (f >= codesrc && f < VSYMBOL(code32init_end))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
ശൂന്യം
code_mutable_preinit(ശൂന്യം)
{
    എങ്കിൽ (HaveRunPost)
        // Already run
        തിരികെ;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_CODE, 0);
    barrier();
    HaveRunPost = 1;
    barrier();
}

// Setup for code relocation and then relocate.
ശൂന്യം VISIBLE32INIT
dopost(ശൂന്യം)
{
    code_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, ശൂന്യ);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
ശൂന്യം VISIBLE32FLAT
handle_post(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU && !CONFIG_COREBOOT)
        തിരികെ;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
