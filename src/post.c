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
#ضمن "memmap.h" //علامت
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
        سیٹ_IVT(i, FUNC16(اندراج_iret_official));

    // Initialize all hw vectors to a default hw handler.
    لكل (i=BIOS_HWIRQ0_سمتیہ; i<BIOS_HWIRQ0_سمتیہ+8; i++)
        سیٹ_IVT(i, FUNC16(اندراج_hwpic1));
    لكل (i=BIOS_HWIRQ8_سمتیہ; i<BIOS_HWIRQ8_سمتیہ+8; i++)
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
    لكل (i=0x60; i<=0x66; i++)
        سیٹ_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    سیٹ_IVT(0x79, SEGOFF(0, 0));
}

ساكن فراغ
bda_init(فراغ)
{
    dprintf(3, "init bda\n");

    بنية bios_انکڑے_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, حجم(*bda));

    صحيح esize = EBDA_ناپ_شروع;
    u16 ebda_seg = EBDA_SEGMENT_شروع;
    إذا (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_منزل_SEG(ALIGN_تھلے(علامت(final_varlow_شروع), 1024)
                                  - EBDA_ناپ_شروع*1024);
    سیٹ_BDA(ebda_seg, ebda_seg);

    سیٹ_BDA(mem_ناپ_kb, ebda_seg / (1024/16));

    // Init ebda
    بنية extended_bios_انکڑے_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, حجم(*ebda));
    ebda->ناپ = esize;

    e820_جوڑو((u32)ebda, BUILD_LOWRAM_سرا-(u32)ebda, E820_RESERVED);

    // Init extra stack
    ڈھیر_Pos = &Extra_ڈھیر[BUILD_EXTRA_ڈھیر_ناپ] - علامت(zonelow_ادھار);
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

    Have_لگاتار_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_ناپ);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
فراغ VISIBLE32FLAT
شروع_Boot(فراغ)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((فراغ*)BUILD_ڈھیر_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_ڈھیر_ADDR);

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
    إذا (دھاگے_during_optionroms())
        device_hardware_تیاری();

    // Run vga option rom
    vgarom_تیاری();
    sercon_تیاری();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    إذا (!دھاگے_during_optionroms()) {
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
    شروع_Boot();
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
    u32 initsize = علامت(code32init_سرا) - علامت(code32init_شروع);
    u32 codealign = علامت(_reloc_min_align);
    فراغ *codedest = memalign_tmp(codealign, initsize);
    فراغ *codesrc = VSYMBOL(code32init_شروع);
    إذا (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    حافظہ_نقل(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(reloc_abs_شروع), VSYMBOL(reloc_abs_سرا)
                 , delta);
    updateRelocs(codedest, VSYMBOL(reloc_rel_شروع), VSYMBOL(reloc_rel_سرا)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_شروع), VSYMBOL(reloc_init_شروع)
                 , VSYMBOL(reloc_init_سرا), delta);
    إذا (f >= codesrc && f < VSYMBOL(code32init_سرا))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
فراغ
کوڈ_mutable_preinit(فراغ)
{
    إذا (Have_لگاتار_Post)
        // Already run
        أرجع;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_کوڈ, 0);
    barrier();
    Have_لگاتار_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
فراغ VISIBLE32INIT
dopost(فراغ)
{
    کوڈ_mutable_preinit();

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
