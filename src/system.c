// Handler for int 0x15 "system" calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "e820map.h" // E820_RAM
#ಅನ್ತರ್ಭಾವ "hw/pic.h" // pic_reset
#ಅನ್ತರ್ಭಾವ "malloc.h" // LegacyRamSize
#ಅನ್ತರ್ಭಾವ "output.h" // debug_enter
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy_far
#ಅನ್ತರ್ಭಾವ "util.h" // handle_1553
#ಅನ್ತರ್ಭಾವ "x86.h" // set_a20

ಸ್ಥಿರ ಶೂನ್ಯ
handle_152400(ರಚನೆ bregs *regs)
{
    ಗಣ_a20(0);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_152401(ರಚನೆ bregs *regs)
{
    ಗಣ_a20(1);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_152402(ರಚನೆ bregs *regs)
{
    regs->al = get_a20();
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_152403(ರಚನೆ bregs *regs)
{
    regs->bx = 3;
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1524XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EUNSUPPORTED);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1524(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x00: handle_152400(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: handle_152401(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: handle_152402(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: handle_152403(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_1524XX(regs); ನಿಲ್ಲಿಸು;
    }
}

// removable media eject
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1552(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1587(ರಚನೆ bregs *regs)
{
    // +++ should probably have descriptor checks
    // +++ should have exception handlers

    u8 prev_a20_enable = ಗಣ_a20(1); // enable A20 line

    // 128K max of transfer on 386+ ???
    // source == destination ???

    // ES:SI points to descriptor table
    // offset   use     initially  comments
    // ==============================================
    // 00..07   Unused  zeros      Null descriptor
    // 08..0f   GDT     zeros      filled in by BIOS
    // 10..17   source  ssssssss   source of data
    // 18..1f   dest    dddddddd   destination of data
    // 20..27   CS      zeros      filled in by BIOS
    // 28..2f   SS      zeros      filled in by BIOS

// check for access rights of source & dest here

    // Initialize GDT descriptor
    u64 *gdt_far = (ಶೂನ್ಯ*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    u32 loc = (u32)MAKE_FLATPTR(gdt_seg, gdt_far);
    ಗಣ_FARVAR(gdt_seg, gdt_far[1], GDT_ದತ್ತಾಂಶ | GDT_ಎಲ್ಲೆ((6*ಗಾತ್ರ(u64))-1)
               | GDT_ತಳ(loc));
    // Initialize CS descriptor
    u64 lim = GDT_ಎಲ್ಲೆ(0x0ffff);
    ಆದರೆ (ಒಳ_post())
        lim = GDT_GRANLIMIT(0xffffffff);
    ಗಣ_FARVAR(gdt_seg, gdt_far[4], GDT_ಸಂಕೇತ | lim | GDT_ತಳ(BUILD_BIOS_ADDR));
    // Initialize SS descriptor
    loc = (u32)MAKE_FLATPTR(GET_SEG(SS), 0);
    ಗಣ_FARVAR(gdt_seg, gdt_far[5], GDT_ದತ್ತಾಂಶ | lim | GDT_ತಳ(loc));

    ಗಣ_SEG(ES, gdt_seg);
    u16 ಎಣಿಕೆ_2 = regs->cx, si = 0, di = 0;
    asm ಅಸ್ಥಿರ(
        // Load new descriptor tables
        "  lgdtw %%es:(1<<3)(%%eax)\n"
        "  lidtw %%cs:pmode_IDT_info\n"

        // Enable protected mode
        "  movl %%cr0, %%eax\n"
        "  orl $" __stringify(CR0_PE) ", %%eax\n"
        "  movl %%eax, %%cr0\n"

        // far jump to flush CPU queue after transition to protected mode
        "  ljmpw $(4<<3), $1f\n"

        // GDT points to valid descriptor table, now load DS, ES
        "1:movw $(2<<3), %%ax\n" // 2nd descriptor in table, TI=GDT, RPL=00
        "  movw %%ax, %%ds\n"
        "  movw $(3<<3), %%ax\n" // 3rd descriptor in table, TI=GDT, RPL=00
        "  movw %%ax, %%es\n"

        // memcpy CX words using 32bit memcpy if applicable
        "  testw $1, %%cx\n"
        "  jnz 3f\n"
        "  shrw $1, %%cx\n"
        "  rep movsl %%ds:(%%si), %%es:(%%di)\n"

        // Restore DS and ES segment limits to 0xffff
        "2:movw $(5<<3), %%ax\n" // 5th descriptor in table (SS)
        "  movw %%ax, %%ds\n"
        "  movw %%ax, %%es\n"

        // Disable protected mode
        "  movl %%cr0, %%eax\n"
        "  andl $~" __stringify(CR0_PE) ", %%eax\n"
        "  movl %%eax, %%cr0\n"

        // far jump to flush CPU queue after transition to real mode
        "  ljmpw $" __stringify(SEG_BIOS) ", $4f\n"

        // Slower 16bit copy method
        "3:rep movsw %%ds:(%%si), %%es:(%%di)\n"
        "  jmp 2b\n"

        // restore IDT to normal real-mode defaults
        "4:lidtw %%cs:rmode_IDT_info\n"

        // Restore %ds (from %ss)
        "  movw %%ss, %%ax\n"
        "  movw %%ax, %%ds\n"
        : "+a" (gdt_far), "+c"(ಎಣಿಕೆ_2), "+m" (__segment_ES)
        : "S" (si), "D" (di)
        : "cc");

    ಗಣ_a20(prev_a20_enable);

    ಗಣ_ಸಂಕೇತ_success(regs);
}

// Get the amount of extended memory (above 1M)
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1588(ರಚನೆ bregs *regs)
{
    u32 rs = GET_GLOBAL(Legacy_Ram_ಗಾತ್ರ);

    // According to Ralf Brown's interrupt the limit should be 15M,
    // but real machines mostly return max. 63M.
    ಆದರೆ (rs > 64*1024*1024)
        regs->ax = 63 * 1024;
    ಇಲ್ಲದಿದ್ದರೆ
        regs->ax = (rs - 1*1024*1024) / 1024;
    ಗಣ_success(regs);
}

// Switch to protected mode
ಶೂನ್ಯ VISIBLE16
handle_1589(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    ಗಣ_a20(1);

    pic_reset(regs->bl, regs->bh);

    u64 *gdt_far = (ಶೂನ್ಯ*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    ಗಣ_FARVAR(gdt_seg, gdt_far[7], GDT_ಸಂಕೇತ | GDT_ಎಲ್ಲೆ(BUILD_BIOS_ಗಾತ್ರ-1)
               | GDT_ತಳ(BUILD_BIOS_ADDR));

    regs->ds = 3<<3; // 3rd gdt descriptor is %ds
    regs->es = 4<<3; // 4th gdt descriptor is %es
    regs->ಸಂಕೇತ_2.seg = 6<<3; // 6th gdt descriptor is %cs

    ಗಣ_ಸಂಕೇತ_success(regs);

    asm ಅಸ್ಥಿರ(
        // Load new descriptor tables
        "  lgdtw %%es:(1<<3)(%%si)\n"
        "  lidtw %%es:(2<<3)(%%si)\n"

        // Enable protected mode
        "  movl %%cr0, %%eax\n"
        "  orl $" __stringify(CR0_PE) ", %%eax\n"
        "  movl %%eax, %%cr0\n"

        // far jump to flush CPU queue after transition to protected mode
        "  ljmpw $(7<<3), $1f\n"

        // GDT points to valid descriptor table, now load SS
        "1:movw $(5<<3), %%ax\n"
        "  movw %%ax, %%ds\n"
        "  movw %%ax, %%ss\n"
        :
        : "S"(gdt_far), "m" (__segment_ES)
        : "eax", "cc");
}

// Device busy interrupt.  Called by Int 16h when no key available
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1590(ರಚನೆ bregs *regs)
{
}

// Interrupt complete.  Called by Int 16h when key becomes available
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1591(ರಚನೆ bregs *regs)
{
}

// keyboard intercept
ಸ್ಥಿರ ಶೂನ್ಯ
handle_154f(ರಚನೆ bregs *regs)
{
    ಗಣ_invalid_silent(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_15c0(ರಚನೆ bregs *regs)
{
    regs->es = SEG_BIOS;
    regs->bx = (u32)&BIOS_CONFIG_ಕೋಷ್ಟಕ;
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_15c1(ರಚನೆ bregs *regs)
{
    regs->es = get_ebda_seg();
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_15e801(ರಚನೆ bregs *regs)
{
    // my real system sets ax and bx to 0
    // this is confirmed by Ralph Brown list
    // but syslinux v1.48 is known to behave
    // strangely if ax is set to 0
    // regs.u.r16.ax = 0;
    // regs.u.r16.bx = 0;

    u32 rs = GET_GLOBAL(Legacy_Ram_ಗಾತ್ರ);

    // Get the amount of extended memory (above 1M)
    ಆದರೆ (rs > 16*1024*1024) {
        // limit to 15M
        regs->cx = 15*1024;
        // Get the amount of extended memory above 16M in 64k blocks
        regs->dx = (rs - 16*1024*1024) / (64*1024);
    } ಇಲ್ಲದಿದ್ದರೆ {
        regs->cx = (rs - 1*1024*1024) / 1024;
        regs->dx = 0;
    }

    // Set configured memory equal to extended memory
    regs->ax = regs->cx;
    regs->bx = regs->dx;

    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_15e820(ರಚನೆ bregs *regs)
{
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = GET_GLOBAL(e820_ಎಣಿಕೆ);
    ಆದರೆ (regs->edx != 0x534D4150 || regs->bx >= ಎಣಿಕೆ_2
        || regs->ecx < ಗಾತ್ರ(e820_list[0])) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EUNSUPPORTED);
        ಹಿಂತಿರುಗಿ;
    }

    memcpy_far(regs->es, (ಶೂನ್ಯ*)(regs->di+0)
               , get_global_seg(), &e820_list[regs->bx]
               , ಗಾತ್ರ(e820_list[0]));
    ಆದರೆ (regs->bx == ಎಣಿಕೆ_2-1)
        regs->ebx = 0;
    ಇಲ್ಲದಿದ್ದರೆ
        regs->ebx++;
    regs->eax = 0x534D4150;
    regs->ecx = ಗಾತ್ರ(e820_list[0]);
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_15e8XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EUNSUPPORTED);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_15e8(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x01: handle_15e801(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x20: handle_15e820(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_15e8XX(regs); ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_15XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EUNSUPPORTED);
}

// INT 15h System Services Entry Point
ಶೂನ್ಯ VISIBLE16
handle_15(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x24: handle_1524(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x4f: handle_154f(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x52: handle_1552(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x53: handle_1553(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x5f: handle_155f(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x7f: handle_157f(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x83: handle_1583(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x86: handle_1586(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x87: handle_1587(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x88: handle_1588(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x90: handle_1590(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x91: handle_1591(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0xc0: handle_15c0(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0xc1: handle_15c1(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0xc2: handle_15c2(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0xe8: handle_15e8(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_15XX(regs); ನಿಲ್ಲಿಸು;
    }
}
