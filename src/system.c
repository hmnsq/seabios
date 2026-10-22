// Handler for int 0x15 "system" calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "e820map.h" // E820_RAM
#అన్తర్భావ "hw/pic.h" // pic_reset
#అన్తర్భావ "malloc.h" // LegacyRamSize
#అన్తర్భావ "output.h" // debug_enter
#అన్తర్భావ "string.h" // memcpy_far
#అన్తర్భావ "util.h" // handle_1553
#అన్తర్భావ "x86.h" // set_a20

స్థిర శూన్య
handle_152400(నిర్మాణం bregs *regs)
{
    సమితి_a20(0);
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
handle_152401(నిర్మాణం bregs *regs)
{
    సమితి_a20(1);
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
handle_152402(నిర్మాణం bregs *regs)
{
    regs->al = get_a20();
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
handle_152403(నిర్మాణం bregs *regs)
{
    regs->bx = 3;
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
handle_1524XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EUNSUPPORTED);
}

స్థిర శూన్య
handle_1524(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->al) {
    స్థితి 0x00: handle_152400(regs); ఆపు;
    స్థితి 0x01: handle_152401(regs); ఆపు;
    స్థితి 0x02: handle_152402(regs); ఆపు;
    స్థితి 0x03: handle_152403(regs); ఆపు;
    అప్రమేయ:   handle_1524XX(regs); ఆపు;
    }
}

// removable media eject
స్థిర శూన్య
handle_1552(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
handle_1587(నిర్మాణం bregs *regs)
{
    // +++ should probably have descriptor checks
    // +++ should have exception handlers

    u8 prev_a20_enable = సమితి_a20(1); // enable A20 line

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
    u64 *gdt_far = (శూన్య*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    u32 loc = (u32)MAKE_FLATPTR(gdt_seg, gdt_far);
    సమితి_FARVAR(gdt_seg, gdt_far[1], GDT_దత్తాంశం | GDT_హద్దు((6*పరిమాణం(u64))-1)
               | GDT_పునాది(loc));
    // Initialize CS descriptor
    u64 lim = GDT_హద్దు(0x0ffff);
    ఒకవేళ (లోపల_post())
        lim = GDT_GRANLIMIT(0xffffffff);
    సమితి_FARVAR(gdt_seg, gdt_far[4], GDT_సంకేతం | lim | GDT_పునాది(BUILD_BIOS_ADDR));
    // Initialize SS descriptor
    loc = (u32)MAKE_FLATPTR(GET_SEG(SS), 0);
    సమితి_FARVAR(gdt_seg, gdt_far[5], GDT_దత్తాంశం | lim | GDT_పునాది(loc));

    సమితి_SEG(ES, gdt_seg);
    u16 లెక్క = regs->cx, si = 0, di = 0;
    asm అస్థిర(
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
        : "+a" (gdt_far), "+c"(లెక్క), "+m" (__segment_ES)
        : "S" (si), "D" (di)
        : "cc");

    సమితి_a20(prev_a20_enable);

    సమితి_సంకేతం_success(regs);
}

// Get the amount of extended memory (above 1M)
స్థిర శూన్య
handle_1588(నిర్మాణం bregs *regs)
{
    u32 rs = GET_GLOBAL(Legacy_Ram_కొలత);

    // According to Ralf Brown's interrupt the limit should be 15M,
    // but real machines mostly return max. 63M.
    ఒకవేళ (rs > 64*1024*1024)
        regs->ax = 63 * 1024;
    లేకపోతే
        regs->ax = (rs - 1*1024*1024) / 1024;
    సమితి_success(regs);
}

// Switch to protected mode
శూన్య VISIBLE16
handle_1589(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    సమితి_a20(1);

    pic_reset(regs->bl, regs->bh);

    u64 *gdt_far = (శూన్య*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    సమితి_FARVAR(gdt_seg, gdt_far[7], GDT_సంకేతం | GDT_హద్దు(BUILD_BIOS_కొలత-1)
               | GDT_పునాది(BUILD_BIOS_ADDR));

    regs->ds = 3<<3; // 3rd gdt descriptor is %ds
    regs->es = 4<<3; // 4th gdt descriptor is %es
    regs->సంకేతం_2.seg = 6<<3; // 6th gdt descriptor is %cs

    సమితి_సంకేతం_success(regs);

    asm అస్థిర(
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
స్థిర శూన్య
handle_1590(నిర్మాణం bregs *regs)
{
}

// Interrupt complete.  Called by Int 16h when key becomes available
స్థిర శూన్య
handle_1591(నిర్మాణం bregs *regs)
{
}

// keyboard intercept
స్థిర శూన్య
handle_154f(నిర్మాణం bregs *regs)
{
    సమితి_invalid_silent(regs);
}

స్థిర శూన్య
handle_15c0(నిర్మాణం bregs *regs)
{
    regs->es = SEG_BIOS;
    regs->bx = (u32)&BIOS_CONFIG_పట్టిక;
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
handle_15c1(నిర్మాణం bregs *regs)
{
    regs->es = get_ebda_seg();
    సమితి_success(regs);
}

స్థిర శూన్య
handle_15e801(నిర్మాణం bregs *regs)
{
    // my real system sets ax and bx to 0
    // this is confirmed by Ralph Brown list
    // but syslinux v1.48 is known to behave
    // strangely if ax is set to 0
    // regs.u.r16.ax = 0;
    // regs.u.r16.bx = 0;

    u32 rs = GET_GLOBAL(Legacy_Ram_కొలత);

    // Get the amount of extended memory (above 1M)
    ఒకవేళ (rs > 16*1024*1024) {
        // limit to 15M
        regs->cx = 15*1024;
        // Get the amount of extended memory above 16M in 64k blocks
        regs->dx = (rs - 16*1024*1024) / (64*1024);
    } లేకపోతే {
        regs->cx = (rs - 1*1024*1024) / 1024;
        regs->dx = 0;
    }

    // Set configured memory equal to extended memory
    regs->ax = regs->cx;
    regs->bx = regs->dx;

    సమితి_success(regs);
}

స్థిర శూన్య
handle_15e820(నిర్మాణం bregs *regs)
{
    పూర్ణాంకం లెక్క = GET_GLOBAL(e820_లెక్క);
    ఒకవేళ (regs->edx != 0x534D4150 || regs->bx >= లెక్క
        || regs->ecx < పరిమాణం(e820_list[0])) {
        సమితి_సంకేతం_invalid(regs, RET_EUNSUPPORTED);
        తిరిగి;
    }

    memcpy_far(regs->es, (శూన్య*)(regs->di+0)
               , get_global_seg(), &e820_list[regs->bx]
               , పరిమాణం(e820_list[0]));
    ఒకవేళ (regs->bx == లెక్క-1)
        regs->ebx = 0;
    లేకపోతే
        regs->ebx++;
    regs->eax = 0x534D4150;
    regs->ecx = పరిమాణం(e820_list[0]);
    సమితి_success(regs);
}

స్థిర శూన్య
handle_15e8XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EUNSUPPORTED);
}

స్థిర శూన్య
handle_15e8(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->al) {
    స్థితి 0x01: handle_15e801(regs); ఆపు;
    స్థితి 0x20: handle_15e820(regs); ఆపు;
    అప్రమేయ:   handle_15e8XX(regs); ఆపు;
    }
}

స్థిర శూన్య
handle_15XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EUNSUPPORTED);
}

// INT 15h System Services Entry Point
శూన్య VISIBLE16
handle_15(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    ఎంపిక (regs->ah) {
    స్థితి 0x24: handle_1524(regs); ఆపు;
    స్థితి 0x4f: handle_154f(regs); ఆపు;
    స్థితి 0x52: handle_1552(regs); ఆపు;
    స్థితి 0x53: handle_1553(regs); ఆపు;
    స్థితి 0x5f: handle_155f(regs); ఆపు;
    స్థితి 0x7f: handle_157f(regs); ఆపు;
    స్థితి 0x83: handle_1583(regs); ఆపు;
    స్థితి 0x86: handle_1586(regs); ఆపు;
    స్థితి 0x87: handle_1587(regs); ఆపు;
    స్థితి 0x88: handle_1588(regs); ఆపు;
    స్థితి 0x90: handle_1590(regs); ఆపు;
    స్థితి 0x91: handle_1591(regs); ఆపు;
    స్థితి 0xc0: handle_15c0(regs); ఆపు;
    స్థితి 0xc1: handle_15c1(regs); ఆపు;
    స్థితి 0xc2: handle_15c2(regs); ఆపు;
    స్థితి 0xe8: handle_15e8(regs); ఆపు;
    అప్రమేయ:   handle_15XX(regs); ఆపు;
    }
}
