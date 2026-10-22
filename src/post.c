// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // SET_BDA
#ضمن "block.h" // block_setup
#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "e820map.h" // e820_add
#ضمن "fw/paravirt.h" // qemu_cfg_preinit
#ضمن "fw/xen.h" // xen_preinit
#ضمن "hw/pic.h" // pic_setup
#ضمن "hw/ps2port.h" // ps2port_setup
#ضمن "hw/rtc.h" // rtc_write
#ضمن "hw/serialio.h" // serial_debug_preinit
#ضمن "hw/usb.h" // usb_setup
#ضمن "malloc.h" // malloc_init
#ضمن "memmap.h" //نښه
#ضمن "output.h" // dprintf
#ضمن "string.h" // memset
#ضمن "util.h" // kbd_init
#ضمن "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

ساكن فراغ
ivt_init(فراغ)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    صحيح i;
    لكل (i=0; i<256; i++)
        سېټ_IVT(i, FUNC16(ثبت_iret_official));

    // Initialize all hw vectors to a default hw handler.
    لكل (i=BIOS_HWIRQ0_ویکټور; i<BIOS_HWIRQ0_ویکټور+8; i++)
        سېټ_IVT(i, FUNC16(ثبت_hwpic1));
    لكل (i=BIOS_HWIRQ8_ویکټور; i<BIOS_HWIRQ8_ویکټور+8; i++)
        سېټ_IVT(i, FUNC16(ثبت_hwpic2));

    // Initialize software handlers.
    سېټ_IVT(0x02, FUNC16(ثبت_02));
    سېټ_IVT(0x05, FUNC16(ثبت_05));
    سېټ_IVT(0x10, FUNC16(ثبت_10));
    سېټ_IVT(0x11, FUNC16(ثبت_11));
    سېټ_IVT(0x12, FUNC16(ثبت_12));
    سېټ_IVT(0x13, FUNC16(ثبت_13_official));
    سېټ_IVT(0x14, FUNC16(ثبت_14));
    سېټ_IVT(0x15, FUNC16(ثبت_15_official));
    سېټ_IVT(0x16, FUNC16(ثبت_16));
    سېټ_IVT(0x17, FUNC16(ثبت_17));
    سېټ_IVT(0x18, FUNC16(ثبت_18));
    سېټ_IVT(0x19, FUNC16(ثبت_19_official));
    سېټ_IVT(0x1a, FUNC16(ثبت_1a_official));
    سېټ_IVT(0x40, FUNC16(ثبت_40));

    // INT 60h-66h reserved for user interrupt
    لكل (i=0x60; i<=0x66; i++)
        سېټ_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    سېټ_IVT(0x79, SEGOFF(0, 0));
}

ساكن فراغ
bda_init(فراغ)
{
    dprintf(3, "init bda\n");

    بنية bios_معلومات_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, حجم(*bda));

    صحيح esize = EBDA_اندازه_پیل;
    u16 ebda_seg = EBDA_SEGMENT_پیل;
    إذا (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_منزل_SEG(ALIGN_ښکته(نښه(final_varlow_پیل), 1024)
                                  - EBDA_اندازه_پیل*1024);
    سېټ_BDA(ebda_seg, ebda_seg);

    سېټ_BDA(mem_اندازه_kb, ebda_seg / (1024/16));

    // Init ebda
    بنية extended_bios_معلومات_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, حجم(*ebda));
    ebda->اندازه = esize;

    e820_جمع_کړه((u32)ebda, BUILD_LOWRAM_پای_ټکی-(u32)ebda, E820_RESERVED);

    // Init extra stack
    ډېری_Pos = &Extra_ډېری[BUILD_EXTRA_ډېری_اندازه] - نښه(zonelow_بنسټ);
}

فراغ
interface_init(فراغ)
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
فراغ
device_hardware_تیاری(فراغ)
{
    usb_تیاری();
    ps2port_تیاری();
    بلاک_تیاری();
    lpt_تیاری();
    serial_تیاری();
    cbfs_payload_تیاری();
}

ساكن فراغ
platform_hardware_تیاری(فراغ)
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

فراغ
prepareboot(فراغ)
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

    Have_پرلپسې_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_اندازه);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
فراغ VISIBLE32FLAT
پیل_Boot(فراغ)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((فراغ*)BUILD_ډېری_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_ډېری_ADDR);

    dprintf(3, "Jump to int19\n");
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
ساكن فراغ
maininit(فراغ)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_تیاری();

    // Start hardware initialization (if threads allowed during optionroms)
    إذا (تارونه_during_optionroms())
        device_hardware_تیاری();

    // Run vga option rom
    vgarom_تیاری();
    sercon_تیاری();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    إذا (!تارونه_during_optionroms()) {
        device_hardware_تیاری();
        wait_تارونه();
    }

    // Run option roms
    optionrom_تیاری();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_تارونه();

    // Prepare for boot.
    prepareboot();

    // Write protect bios memory.
    make_bios_readonly();

    // Invoke int 19 to start boot process.
    پیل_Boot();
}


/****************************************************************
 * POST entry and code relocation
 ****************************************************************/

// Update given relocs for the code at 'dest' with a given 'delta'
ساكن فراغ
updateRelocs(فراغ *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    لكل (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
فراغ __noreturn
reloc_preinit(فراغ *f, فراغ *arg)
{
    فراغ (*func)(فراغ *) __noreturn = f;
    إذا (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = نښه(code32init_پای_ټکی) - نښه(code32init_پیل);
    u32 codealign = نښه(_reloc_min_align);
    فراغ *codedest = memalign_tmp(codealign, initsize);
    فراغ *codesrc = VSYMBOL(code32init_پیل);
    إذا (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    حافظه_نقل(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(reloc_abs_پیل), VSYMBOL(reloc_abs_پای_ټکی)
                 , delta);
    updateRelocs(codedest, VSYMBOL(reloc_rel_پیل), VSYMBOL(reloc_rel_پای_ټکی)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_پیل), VSYMBOL(reloc_init_پیل)
                 , VSYMBOL(reloc_init_پای_ټکی), delta);
    إذا (f >= codesrc && f < VSYMBOL(code32init_پای_ټکی))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
فراغ
کوډ_mutable_preinit(فراغ)
{
    إذا (Have_پرلپسې_Post)
        // Already run
        أرجع;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_کوډ, 0);
    barrier();
    Have_پرلپسې_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
فراغ VISIBLE32INIT
dopost(فراغ)
{
    کوډ_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, عدم);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
فراغ VISIBLE32FLAT
handle_post(فراغ)
{
    إذا (!CONFIG_QEMU && !CONFIG_COREBOOT)
        أرجع;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
