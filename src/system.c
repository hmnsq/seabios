// Handler for int 0x15 "system" calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // E820_RAM
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pic.h" // pic_reset
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // LegacyRamSize
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // debug_enter
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy_far
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // handle_1553
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // set_a20

ਸਥਿਰ ਖਾਲੀ
handle_152400(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_a20(0);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_152401(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_a20(1);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_152402(ਬਣਤਰ bregs *regs)
{
    regs->al = get_a20();
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_152403(ਬਣਤਰ bregs *regs)
{
    regs->bx = 3;
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_1524XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EUNSUPPORTED);
}

ਸਥਿਰ ਖਾਲੀ
handle_1524(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x00: handle_152400(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: handle_152401(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: handle_152402(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x03: handle_152403(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_1524XX(regs); ਰੋਕੋ;
    }
}

// removable media eject
ਸਥਿਰ ਖਾਲੀ
handle_1552(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_1587(ਬਣਤਰ bregs *regs)
{
    // +++ should probably have descriptor checks
    // +++ should have exception handlers

    u8 prev_a20_enable = ਸੈੱਟ_a20(1); // enable A20 line

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
    u64 *gdt_far = (ਖਾਲੀ*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    u32 loc = (u32)MAKE_FLATPTR(gdt_seg, gdt_far);
    ਸੈੱਟ_FARVAR(gdt_seg, gdt_far[1], GDT_ਅੰਕੜੇ | GDT_ਹੱਦਬੰਦੀ((6*ਆਕਾਰ(u64))-1)
               | GDT_ਅਧਾਰ(loc));
    // Initialize CS descriptor
    u64 lim = GDT_ਹੱਦਬੰਦੀ(0x0ffff);
    ਜੇ (ਵਿੱਚ_post())
        lim = GDT_GRANLIMIT(0xffffffff);
    ਸੈੱਟ_FARVAR(gdt_seg, gdt_far[4], GDT_ਕੋਡ | lim | GDT_ਅਧਾਰ(BUILD_BIOS_ADDR));
    // Initialize SS descriptor
    loc = (u32)MAKE_FLATPTR(GET_SEG(SS), 0);
    ਸੈੱਟ_FARVAR(gdt_seg, gdt_far[5], GDT_ਅੰਕੜੇ | lim | GDT_ਅਧਾਰ(loc));

    ਸੈੱਟ_SEG(ES, gdt_seg);
    u16 ਗਿਣਤੀ_2 = regs->cx, si = 0, di = 0;
    asm ਅਸਥਿਰ(
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
        : "+a" (gdt_far), "+c"(ਗਿਣਤੀ_2), "+m" (__segment_ES)
        : "S" (si), "D" (di)
        : "cc");

    ਸੈੱਟ_a20(prev_a20_enable);

    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Get the amount of extended memory (above 1M)
ਸਥਿਰ ਖਾਲੀ
handle_1588(ਬਣਤਰ bregs *regs)
{
    u32 rs = GET_GLOBAL(Legacy_Ram_ਮਾਪ);

    // According to Ralf Brown's interrupt the limit should be 15M,
    // but real machines mostly return max. 63M.
    ਜੇ (rs > 64*1024*1024)
        regs->ax = 63 * 1024;
    ਨਹੀਂਤਾਂ
        regs->ax = (rs - 1*1024*1024) / 1024;
    ਸੈੱਟ_success(regs);
}

// Switch to protected mode
ਖਾਲੀ VISIBLE16
handle_1589(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    ਸੈੱਟ_a20(1);

    pic_reset(regs->bl, regs->bh);

    u64 *gdt_far = (ਖਾਲੀ*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    ਸੈੱਟ_FARVAR(gdt_seg, gdt_far[7], GDT_ਕੋਡ | GDT_ਹੱਦਬੰਦੀ(BUILD_BIOS_ਮਾਪ-1)
               | GDT_ਅਧਾਰ(BUILD_BIOS_ADDR));

    regs->ds = 3<<3; // 3rd gdt descriptor is %ds
    regs->es = 4<<3; // 4th gdt descriptor is %es
    regs->ਕੋਡ.seg = 6<<3; // 6th gdt descriptor is %cs

    ਸੈੱਟ_ਕੋਡ_success(regs);

    asm ਅਸਥਿਰ(
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
ਸਥਿਰ ਖਾਲੀ
handle_1590(ਬਣਤਰ bregs *regs)
{
}

// Interrupt complete.  Called by Int 16h when key becomes available
ਸਥਿਰ ਖਾਲੀ
handle_1591(ਬਣਤਰ bregs *regs)
{
}

// keyboard intercept
ਸਥਿਰ ਖਾਲੀ
handle_154f(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_invalid_silent(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_15c0(ਬਣਤਰ bregs *regs)
{
    regs->es = SEG_BIOS;
    regs->bx = (u32)&BIOS_CONFIG_ਸਾਰਣੀ;
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_15c1(ਬਣਤਰ bregs *regs)
{
    regs->es = get_ebda_seg();
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_15e801(ਬਣਤਰ bregs *regs)
{
    // my real system sets ax and bx to 0
    // this is confirmed by Ralph Brown list
    // but syslinux v1.48 is known to behave
    // strangely if ax is set to 0
    // regs.u.r16.ax = 0;
    // regs.u.r16.bx = 0;

    u32 rs = GET_GLOBAL(Legacy_Ram_ਮਾਪ);

    // Get the amount of extended memory (above 1M)
    ਜੇ (rs > 16*1024*1024) {
        // limit to 15M
        regs->cx = 15*1024;
        // Get the amount of extended memory above 16M in 64k blocks
        regs->dx = (rs - 16*1024*1024) / (64*1024);
    } ਨਹੀਂਤਾਂ {
        regs->cx = (rs - 1*1024*1024) / 1024;
        regs->dx = 0;
    }

    // Set configured memory equal to extended memory
    regs->ax = regs->cx;
    regs->bx = regs->dx;

    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_15e820(ਬਣਤਰ bregs *regs)
{
    ਪੂਰਨ ਗਿਣਤੀ_2 = GET_GLOBAL(e820_ਗਿਣਤੀ);
    ਜੇ (regs->edx != 0x534D4150 || regs->bx >= ਗਿਣਤੀ_2
        || regs->ecx < ਆਕਾਰ(e820_list[0])) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EUNSUPPORTED);
        ਵਾਪਸ;
    }

    memcpy_far(regs->es, (ਖਾਲੀ*)(regs->di+0)
               , get_global_seg(), &e820_list[regs->bx]
               , ਆਕਾਰ(e820_list[0]));
    ਜੇ (regs->bx == ਗਿਣਤੀ_2-1)
        regs->ebx = 0;
    ਨਹੀਂਤਾਂ
        regs->ebx++;
    regs->eax = 0x534D4150;
    regs->ecx = ਆਕਾਰ(e820_list[0]);
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_15e8XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EUNSUPPORTED);
}

ਸਥਿਰ ਖਾਲੀ
handle_15e8(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x01: handle_15e801(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x20: handle_15e820(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_15e8XX(regs); ਰੋਕੋ;
    }
}

ਸਥਿਰ ਖਾਲੀ
handle_15XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EUNSUPPORTED);
}

// INT 15h System Services Entry Point
ਖਾਲੀ VISIBLE16
handle_15(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x24: handle_1524(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x4f: handle_154f(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x52: handle_1552(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x53: handle_1553(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x5f: handle_155f(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x7f: handle_157f(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x83: handle_1583(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x86: handle_1586(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x87: handle_1587(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x88: handle_1588(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x90: handle_1590(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x91: handle_1591(regs); ਰੋਕੋ;
    ਹਾਲਤ 0xc0: handle_15c0(regs); ਰੋਕੋ;
    ਹਾਲਤ 0xc1: handle_15c1(regs); ਰੋਕੋ;
    ਹਾਲਤ 0xc2: handle_15c2(regs); ਰੋਕੋ;
    ਹਾਲਤ 0xe8: handle_15e8(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_15XX(regs); ਰੋਕੋ;
    }
}
