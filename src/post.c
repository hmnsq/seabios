// 32bit code to Power On Self Test (POST) a machine.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // SET_BDA
#అన్తర్భావ "block.h" // block_setup
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "e820map.h" // e820_add
#అన్తర్భావ "fw/paravirt.h" // qemu_cfg_preinit
#అన్తర్భావ "fw/xen.h" // xen_preinit
#అన్తర్భావ "hw/pic.h" // pic_setup
#అన్తర్భావ "hw/ps2port.h" // ps2port_setup
#అన్తర్భావ "hw/rtc.h" // rtc_write
#అన్తర్భావ "hw/serialio.h" // serial_debug_preinit
#అన్తర్భావ "hw/usb.h" // usb_setup
#అన్తర్భావ "malloc.h" // malloc_init
#అన్తర్భావ "memmap.h" //చిహ్నం
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // kbd_init
#అన్తర్భావ "tcgbios.h" // tpm_*


/****************************************************************
 * BIOS initialization and hardware setup
 ****************************************************************/

స్థిర శూన్య
ivt_init(శూన్య)
{
    dprintf(3, "init ivt\n");

    // Initialize all vectors to the default handler.
    పూర్ణాంకం i;
    కొరకు (i=0; i<256; i++)
        సమితి_IVT(i, FUNC16(నమోదు_iret_official));

    // Initialize all hw vectors to a default hw handler.
    కొరకు (i=BIOS_HWIRQ0_సదిశ; i<BIOS_HWIRQ0_సదిశ+8; i++)
        సమితి_IVT(i, FUNC16(నమోదు_hwpic1));
    కొరకు (i=BIOS_HWIRQ8_సదిశ; i<BIOS_HWIRQ8_సదిశ+8; i++)
        సమితి_IVT(i, FUNC16(నమోదు_hwpic2));

    // Initialize software handlers.
    సమితి_IVT(0x02, FUNC16(నమోదు_02));
    సమితి_IVT(0x05, FUNC16(నమోదు_05));
    సమితి_IVT(0x10, FUNC16(నమోదు_10));
    సమితి_IVT(0x11, FUNC16(నమోదు_11));
    సమితి_IVT(0x12, FUNC16(నమోదు_12));
    సమితి_IVT(0x13, FUNC16(నమోదు_13_official));
    సమితి_IVT(0x14, FUNC16(నమోదు_14));
    సమితి_IVT(0x15, FUNC16(నమోదు_15_official));
    సమితి_IVT(0x16, FUNC16(నమోదు_16));
    సమితి_IVT(0x17, FUNC16(నమోదు_17));
    సమితి_IVT(0x18, FUNC16(నమోదు_18));
    సమితి_IVT(0x19, FUNC16(నమోదు_19_official));
    సమితి_IVT(0x1a, FUNC16(నమోదు_1a_official));
    సమితి_IVT(0x40, FUNC16(నమోదు_40));

    // INT 60h-66h reserved for user interrupt
    కొరకు (i=0x60; i<=0x66; i++)
        సమితి_IVT(i, SEGOFF(0, 0));

    // set vector 0x79 to zero
    // this is used by 'gardian angel' protection system
    సమితి_IVT(0x79, SEGOFF(0, 0));
}

స్థిర శూన్య
bda_init(శూన్య)
{
    dprintf(3, "init bda\n");

    నిర్మాణం bios_దత్తాంశం_area_s *bda = MAKE_FLATPTR(SEG_BDA, 0);
    memset(bda, 0, పరిమాణం(*bda));

    పూర్ణాంకం esize = EBDA_కొలత_ప్రారంభం;
    u16 ebda_seg = EBDA_SEGMENT_ప్రారంభం;
    ఒకవేళ (!CONFIG_MALLOC_UPPERMEMORY)
        ebda_seg = FLATPTR_గమ్యం_SEG(ALIGN_కిందికి(చిహ్నం(final_varlow_ప్రారంభం), 1024)
                                  - EBDA_కొలత_ప్రారంభం*1024);
    సమితి_BDA(ebda_seg, ebda_seg);

    సమితి_BDA(mem_కొలత_kb, ebda_seg / (1024/16));

    // Init ebda
    నిర్మాణం extended_bios_దత్తాంశం_area_s *ebda = get_ebda_ptr();
    memset(ebda, 0, పరిమాణం(*ebda));
    ebda->కొలత = esize;

    e820_కూడు((u32)ebda, BUILD_LOWRAM_చివర-(u32)ebda, E820_RESERVED);

    // Init extra stack
    దొంతర_Pos = &Extra_దొంతర[BUILD_EXTRA_దొంతర_కొలత] - చిహ్నం(zonelow_పునాది);
}

శూన్య
interface_init(శూన్య)
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
శూన్య
device_hardware_అమరిక(శూన్య)
{
    usb_అమరిక();
    ps2port_అమరిక();
    ఖండం_అమరిక();
    lpt_అమరిక();
    serial_అమరిక();
    cbfs_payload_అమరిక();
}

స్థిర శూన్య
platform_hardware_అమరిక(శూన్య)
{
    // Make sure legacy DMA isn't running.
    dma_అమరిక();

    // Init base pc hardware.
    pic_అమరిక();
    thread_అమరిక();
    mathcp_అమరిక();

    // Platform specific setup
    qemu_platform_అమరిక();
    coreboot_platform_అమరిక();

    // Setup timers and periodic clock interrupt
    timer_అమరిక();
    clock_అమరిక();

    // Initialize TPM
    tpm_అమరిక();
}

శూన్య
prepareboot(శూన్య)
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

    Have_నిరంతరం_Post = 2;

    // Setup bios checksum.
    BiosChecksum -= checksum((u8*)BUILD_BIOS_ADDR, BUILD_BIOS_కొలత);
}

// Begin the boot process by invoking an int0x19 in 16bit mode.
శూన్య VISIBLE32FLAT
ప్రారంభం_Boot(శూన్య)
{
    // Clear low-memory allocations (required by PMM spec).
    memset((శూన్య*)BUILD_దొంతర_ADDR, 0, BUILD_EBDA_MINIMUM - BUILD_దొంతర_ADDR);

    dprintf(3, "Jump to int19\n");
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    call16_int(0x19, &br);
}

// Main setup code.
స్థిర శూన్య
maininit(శూన్య)
{
    // Initialize internal interfaces.
    interface_init();

    // Setup platform devices.
    platform_hardware_అమరిక();

    // Start hardware initialization (if threads allowed during optionroms)
    ఒకవేళ (దారాలు_during_optionroms())
        device_hardware_అమరిక();

    // Run vga option rom
    vgarom_అమరిక();
    sercon_అమరిక();
    enable_vga_console();

    // Do hardware initialization (if running synchronously)
    ఒకవేళ (!దారాలు_during_optionroms()) {
        device_hardware_అమరిక();
        wait_దారాలు();
    }

    // Run option roms
    optionrom_అమరిక();

    // Allow user to modify overall boot order.
    interactive_bootmenu();
    wait_దారాలు();

    // Prepare for boot.
    prepareboot();

    // Write protect bios memory.
    make_bios_readonly();

    // Invoke int 19 to start boot process.
    ప్రారంభం_Boot();
}


/****************************************************************
 * POST entry and code relocation
 ****************************************************************/

// Update given relocs for the code at 'dest' with a given 'delta'
స్థిర శూన్య
updateRelocs(శూన్య *dest, u32 *rstart, u32 *rend, u32 delta)
{
    u32 *reloc;
    కొరకు (reloc = rstart; reloc < rend; reloc++)
        *((u32*)(dest + *reloc)) += delta;
}

// Relocate init code and then call a function at its new address.
// The passed function should be in the "init" section and must not
// return.
శూన్య __noreturn
reloc_preinit(శూన్య *f, శూన్య *arg)
{
    శూన్య (*func)(శూన్య *) __noreturn = f;
    ఒకవేళ (!CONFIG_RELOCATE_INIT)
        func(arg);

    // Allocate space for init code.
    u32 initsize = చిహ్నం(code32init_చివర) - చిహ్నం(code32init_ప్రారంభం);
    u32 codealign = చిహ్నం(_reloc_min_align);
    శూన్య *codedest = memalign_tmp(codealign, initsize);
    శూన్య *codesrc = VSYMBOL(code32init_ప్రారంభం);
    ఒకవేళ (!codedest)
        panic("No space for init relocation.\n");

    // Copy code and update relocs (init absolute, init relative, and runtime)
    dprintf(1, "Relocating init from %p to %p (size %d)\n"
            , codesrc, codedest, initsize);
    s32 delta = codedest - codesrc;
    జ్ఞాపకం_నకలు(codedest, codesrc, initsize);
    updateRelocs(codedest, VSYMBOL(reloc_abs_ప్రారంభం), VSYMBOL(reloc_abs_చివర)
                 , delta);
    updateRelocs(codedest, VSYMBOL(reloc_rel_ప్రారంభం), VSYMBOL(reloc_rel_చివర)
                 , -delta);
    updateRelocs(VSYMBOL(code32flat_ప్రారంభం), VSYMBOL(reloc_init_ప్రారంభం)
                 , VSYMBOL(reloc_init_చివర), delta);
    ఒకవేళ (f >= codesrc && f < VSYMBOL(code32init_చివర))
        func = f + delta;

    // Call function in relocated code.
    barrier();
    func(arg);
}

// Runs after all code is present and prior to any modifications
శూన్య
సంకేతం_mutable_preinit(శూన్య)
{
    ఒకవేళ (Have_నిరంతరం_Post)
        // Already run
        తిరిగి;
    // Setup reset-vector entry point (controls legacy reboots).
    rtc_write(CMOS_RESET_సంకేతం, 0);
    barrier();
    Have_నిరంతరం_Post = 1;
    barrier();
}

// Setup for code relocation and then relocate.
శూన్య VISIBLE32INIT
dopost(శూన్య)
{
    సంకేతం_mutable_preinit();

    // Detect ram and setup internal malloc.
    qemu_preinit();
    coreboot_preinit();
    malloc_preinit();

    // Relocate initialization code and call maininit().
    reloc_preinit(maininit, ఖాళీ);
}

// Entry point for Power On Self Test (POST) - the BIOS initilization
// phase.  This function makes the memory at 0xc0000-0xfffff
// read/writable and then calls dopost().
శూన్య VISIBLE32FLAT
handle_post(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU && !CONFIG_COREBOOT)
        తిరిగి;

    serial_debug_preinit();
    debug_banner();

    // Check if we are running under Xen.
    xen_preinit();

    // Allow writes to modify bios area (0xf0000)
    make_bios_writable();

    // Now that memory is read/writable - start post process.
    dopost();
}
