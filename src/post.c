// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // SET_BDA
#अन्तर्भाव "block.h" // block_setup
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "e820map.h" // e820_add
#अन्तर्भाव "fw/paravirt.h" // qemu_cfg_preinit
#अन्तर्भाव "fw/xen.h" // xen_preinit
#अन्तर्भाव "hw/pic.h" // pic_setup
#अन्तर्भाव "hw/ps2port.h" // ps2port_setup
#अन्तर्भाव "hw/rtc.h" // rtc_write
#अन्तर्भाव "hw/serialio.h" // serial_debug_preinit
#अन्तर्भाव "hw/usb.h" // usb_setup
#अन्तर्भाव "malloc.h" // malloc_init
#अन्तर्भाव "memmap.h" //चिह्नम्
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // kbd_init
#अन्तर्भाव "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

स्थिर शून्य
ivt_init(शून्य)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    पूर्णाङ्क i;
    प्रति (i=0; i<256; i++)
        समुच्चयः_IVT(i, FUNC16(प्रविष्टिः_iret_official));

    // Initialize all hw vectors to a default hw handler.
    प्रति (i=BIOS_HWIRQ0_सदिशम्; i<BIOS_HWIRQ0_सदिशम्+8; i++)
        समुच्चयः_IVT(i, FUNC16(प्रविष्टिः_hwpic1));
    प्रति (i=BIOS_HWIRQ8_सदिशम्; i<BIOS_HWIRQ8_सदिशम्+8; i++)
        समुच्चयः_IVT(i, FUNC16(प्रविष्टिः_hwpic2));

    // Initialize software handlers.
    समुच्चयः_IVT(0x02, FUNC16(प्रविष्टिः_02));
    समुच्चयः_IVT(0x05, FUNC16(प्रविष्टिः_05));
    समुच्चयः_IVT(0x10, FUNC16(प्रविष्टिः_10));
    समुच्चयः_IVT(0x11, FUNC16(प्रविष्टिः_11));
    समुच्चयः_IVT(0x12, FUNC16(प्रविष्टिः_12));
    समुच्चयः_IVT(0x13, FUNC16(प्रविष्टिः_13_official));
    समुच्चयः_IVT(0x14, FUNC16(प्रविष्टिः_14));
    समुच्चयः_IVT(0x15, FUNC16(प्रविष्टिः_15_official));
    समुच्चयः_IVT(0x16, FUNC16(प्रविष्टिः_16));
    समुच्चयः_IVT(0x17, FUNC16(प्रविष्टिः_17));
    समुच्चयः_IVT(0x18, FUNC16(प्रविष्टिः_18));
    समुच्चयः_IVT(0x19, FUNC16(प्रविष्टिः_19_official));
    समुच्चयः_IVT(0x1a, FUNC16(प्रविष्टिः_1a_official));
    समुच्चयः_IVT(0x40, FUNC16(प्रविष्टिः_40));

    // INT 60h-66h reserved for user interrupt
    प्रति (i=0x60; i<=0x66; i++)
        समुच्चयः_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    समुच्चयः_IVT(0x79, SEGOFF(0, 0));
}

स्थिर शून्य
bda_init(शून्य)
{
    dprintf(3, "init bda\n");

    संरचना bios_दत्त_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    स्मृतिं_पूरय(bda, 0, परिमाण(*bda));

    पूर्णाङ्क esize = EBDA_आकार_START;
    u16 ebda_seg = EBDA_SEGMENT_START;
    यदि (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_गन्तव्यम्_SEG(ALIGN_अधस्तात्(चिह्नम्(final_varlow_start), 1024)
                                  - EBDA_आकार_START*1024);
    समुच्चयः_BDA(ebda_seg, ebda_seg);

    समुच्चयः_BDA(mem_आकार_kb, ebda_seg / (1024/16));

    // Init ebda
    संरचना extended_bios_दत्त_area_s *ebda = get_ebda_ptr();
    स्मृतिं_पूरय(ebda, 0, परिमाण(*ebda));
    ebda->आकार = esize;

    e820_सङ्कलय((u32)ebda, BUILD_LOWRAM_अन्तः-(u32)ebda, E820_RESERVED);

    // Init extra stack
    राशिः_Pos = &Extra_राशिः[BUILD_EXTRA_राशिः_आकार] - चिह्नम्(zonelow_आधारः);
}

शून्य
interface_init(शून्य)
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
शून्य
device_hardware_सज्जता(शून्य)
{
    usb_सज्जता();
    ps2port_सज्जता();
    खण्डम्_सज्जता();
    lpt_सज्जता();
    serial_सज्जता();
    cbfs_payload_सज्जता();
}

स्थिर शून्य
platform_hardware_सज्जता(शून्य)
{
    // Make sure legacy DMA isn't running.
    dma_सज्जता();

    // Init base pc hardware.
    pic_सज्जता();
    thread_सज्जता();
    mathcp_सज्जता();

    // Platform specific setup
    qemu_platform_सज्जता();
    coreboot_platform_सज्जता();

    // Setup timers and periodic clock interrupt
    timer_सज्जता();
    clock_सज्जता();

    // Initialize TPM
    tpm_सज्जता();
}

शून्य
prepareboot(शून्य)
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

    Have_सततम्_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_आकार);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
शून्य VISIBLE32FLAT
startBoot(शून्य)
{
    // Clear low-memory allocations (required by PMM spec).
    स्मृतिं_पूरय((शून्य*)BUILD_राशिः_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_राशिः_ADDR);

    dprintf(3, "Jump to int19\n");
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
स्थिर शून्य
maininit(शून्य)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_सज्जता();

    // Start hardware initialization (if threads allowed during optionroms)
    यदि (तन्तवः_during_optionroms())
        device_hardware_सज्जता();

    // Run vga option rom
    vgarom_सज्जता();
    sercon_सज्जता();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    यदि (!तन्तवः_during_optionroms()) {
        device_hardware_सज्जता();
        wait_तन्तवः();
    }

    // Run option roms
    optionrom_सज्जता();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_तन्तवः();

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
स्थिर शून्य
updateRelocs(शून्य *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    प्रति (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
शून्य __noreturn
reloc_preinit(शून्य *f, शून्य *arg)
{
    शून्य (*func)(शून्य *) __noreturn = f;
    यदि (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = चिह्नम्(code32init_अन्तः) - चिह्नम्(code32init_start);
    u32 codealign = चिह्नम्(_reloc_min_align);
    शून्य *codedest = memalign_tmp(codealign, initsize);
    शून्य *codesrc = VSYMBOL(code32init_start);
    यदि (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    स्मृतिं_प्रतिलिख(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(_reloc_abs_start), VSYMBOL(reloc_abs_अन्तः)
                 , delta);
    updateRelocs(codedest, VSYMBOL(_reloc_rel_start), VSYMBOL(reloc_rel_अन्तः)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_start), VSYMBOL(_reloc_init_start)
                 , VSYMBOL(reloc_init_अन्तः), delta);
    यदि (f >= codesrc && f < VSYMBOL(code32init_अन्तः))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
शून्य
संहिता_mutable_preinit(शून्य)
{
    यदि (Have_सततम्_Post)
        // Already run
        प्रत्यागम;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_संहिता, 0);
    barrier();
    Have_सततम्_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
शून्य VISIBLE32INIT
dopost(शून्य)
{
    संहिता_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, रिक्त);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
शून्य VISIBLE32FLAT
handle_post(शून्य)
{
    यदि (!CONFIG_QEMU && !CONFIG_COREBOOT)
        प्रत्यागम;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
