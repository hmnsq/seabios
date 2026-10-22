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
#ضمن "memmap.h" //نشان
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
        سيٽ_IVT(i, FUNC16(داخلا_iret_official));

    // Initialize all hw vectors to a default hw handler.
    لكل (i=BIOS_HWIRQ0_ويڪٽر; i<BIOS_HWIRQ0_ويڪٽر+8; i++)
        سيٽ_IVT(i, FUNC16(داخلا_hwpic1));
    لكل (i=BIOS_HWIRQ8_ويڪٽر; i<BIOS_HWIRQ8_ويڪٽر+8; i++)
        سيٽ_IVT(i, FUNC16(داخلا_hwpic2));

    // Initialize software handlers.
    سيٽ_IVT(0x02, FUNC16(داخلا_02));
    سيٽ_IVT(0x05, FUNC16(داخلا_05));
    سيٽ_IVT(0x10, FUNC16(داخلا_10));
    سيٽ_IVT(0x11, FUNC16(داخلا_11));
    سيٽ_IVT(0x12, FUNC16(داخلا_12));
    سيٽ_IVT(0x13, FUNC16(داخلا_13_official));
    سيٽ_IVT(0x14, FUNC16(داخلا_14));
    سيٽ_IVT(0x15, FUNC16(داخلا_15_official));
    سيٽ_IVT(0x16, FUNC16(داخلا_16));
    سيٽ_IVT(0x17, FUNC16(داخلا_17));
    سيٽ_IVT(0x18, FUNC16(داخلا_18));
    سيٽ_IVT(0x19, FUNC16(داخلا_19_official));
    سيٽ_IVT(0x1a, FUNC16(داخلا_1a_official));
    سيٽ_IVT(0x40, FUNC16(داخلا_40));

    // INT 60h-66h reserved for user interrupt
    لكل (i=0x60; i<=0x66; i++)
        سيٽ_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    سيٽ_IVT(0x79, SEGOFF(0, 0));
}

ساكن فراغ
bda_init(فراغ)
{
    dprintf(3, "init bda\n");

    بنية bios_معلومات_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, حجم(*bda));

    صحيح esize = EBDA_سائيز_شروعات;
    u16 ebda_seg = EBDA_SEGMENT_شروعات;
    إذا (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_منزل_SEG(ALIGN_هيٺ_ڏانهن(نشان(final_varlow_شروعات), 1024)
                                  - EBDA_سائيز_شروعات*1024);
    سيٽ_BDA(ebda_seg, ebda_seg);

    سيٽ_BDA(mem_سائيز_kb, ebda_seg / (1024/16));

    // Init ebda
    بنية extended_bios_معلومات_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, حجم(*ebda));
    ebda->سائيز = esize;

    e820_جوڙيو((u32)ebda, BUILD_LOWRAM_پڇاڙي-(u32)ebda, E820_RESERVED);

    // Init extra stack
    ڍير_Pos = &Extra_ڍير[BUILD_EXTRA_ڍير_سائيز] - نشان(zonelow_بنياد);
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
device_hardware_تياري(فراغ)
{
    usb_تياري();
    ps2port_تياري();
    بلاڪ_تياري();
    lpt_تياري();
    serial_تياري();
    cbfs_payload_تياري();
}

ساكن فراغ
platform_hardware_تياري(فراغ)
{
    // Make sure legacy DMA isn't running.
    dma_تياري();

    // Init base pc hardware.
    pic_تياري();
    thread_تياري();
    mathcp_تياري();

    // Platform specific setup
    qemu_platform_تياري();
    coreboot_platform_تياري();

    // Setup timers and periodic clock interrupt
    timer_تياري();
    clock_تياري();

    // Initialize TPM
    tpm_تياري();
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

    Have_لاڳيتو_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_سائيز);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
فراغ VISIBLE32FLAT
شروعات_Boot(فراغ)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((فراغ*)BUILD_ڍير_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_ڍير_ADDR);

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
    platform_hardware_تياري();

    // Start hardware initialization (if threads allowed during optionroms)
    إذا (ڌاڳا_during_optionroms())
        device_hardware_تياري();

    // Run vga option rom
    vgarom_تياري();
    sercon_تياري();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    إذا (!ڌاڳا_during_optionroms()) {
        device_hardware_تياري();
        wait_ڌاڳا();
    }

    // Run option roms
    optionrom_تياري();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_ڌاڳا();

    // Prepare for boot.
    prepareboot();

    // Write protect bios memory.
    make_bios_readonly();

    // Invoke int 19 to start boot process.
    شروعات_Boot();
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
    u32 initsize = نشان(code32init_پڇاڙي) - نشان(code32init_شروعات);
    u32 codealign = نشان(_reloc_min_align);
    فراغ *codedest = memalign_tmp(codealign, initsize);
    فراغ *codesrc = VSYMBOL(code32init_شروعات);
    إذا (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    يادگيري_نقل(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(reloc_abs_شروعات), VSYMBOL(reloc_abs_پڇاڙي)
                 , delta);
    updateRelocs(codedest, VSYMBOL(reloc_rel_شروعات), VSYMBOL(reloc_rel_پڇاڙي)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_شروعات), VSYMBOL(reloc_init_شروعات)
                 , VSYMBOL(reloc_init_پڇاڙي), delta);
    إذا (f >= codesrc && f < VSYMBOL(code32init_پڇاڙي))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
فراغ
ڪوڊ_mutable_preinit(فراغ)
{
    إذا (Have_لاڳيتو_Post)
        // Already run
        أرجع;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_ڪوڊ, 0);
    barrier();
    Have_لاڳيتو_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
فراغ VISIBLE32INIT
dopost(فراغ)
{
    ڪوڊ_mutable_preinit();

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
