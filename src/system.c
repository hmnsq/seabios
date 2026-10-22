// Handler for int 0x15 "system" calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "e820map.h" // E820_RAM
#അന്തര്ഭാവ "hw/pic.h" // pic_reset
#അന്തര്ഭാവ "malloc.h" // LegacyRamSize
#അന്തര്ഭാവ "output.h" // debug_enter
#അന്തര്ഭാവ "string.h" // memcpy_far
#അന്തര്ഭാവ "util.h" // handle_1553
#അന്തര്ഭാവ "x86.h" // set_a20

സ്ഥിതം ശൂന്യം
handle_152400(ഘടന bregs *regs)
{
    set_a20(0);
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_152401(ഘടന bregs *regs)
{
    set_a20(1);
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_152402(ഘടന bregs *regs)
{
    regs->al = get_a20();
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_152403(ഘടന bregs *regs)
{
    regs->bx = 3;
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_1524XX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

സ്ഥിതം ശൂന്യം
handle_1524(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x00: handle_152400(regs); നിർത്തുക;
    അവസ്ഥ 0x01: handle_152401(regs); നിർത്തുക;
    അവസ്ഥ 0x02: handle_152402(regs); നിർത്തുക;
    അവസ്ഥ 0x03: handle_152403(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_1524XX(regs); നിർത്തുക;
    }
}

// removable media eject
സ്ഥിതം ശൂന്യം
handle_1552(ഘടന bregs *regs)
{
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_1587(ഘടന bregs *regs)
{
    // +++ should probably have descriptor checks
    // +++ should have exception handlers

    u8 prev_a20_enable = set_a20(1); // enable A20 line

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
    u64 *gdt_far = (ശൂന്യം*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    u32 loc = (u32)MAKE_FLATPTR(gdt_seg, gdt_far);
    SET_FARVAR(gdt_seg, gdt_far[1], GDT_DATA | GDT_LIMIT((6*വലുപ്പം(u64))-1)
               | GDT_BASE(loc));
    // Initialize CS descriptor
    u64 lim = GDT_LIMIT(0x0ffff);
    എങ്കിൽ (in_post())
        lim = GDT_GRANLIMIT(0xffffffff);
    SET_FARVAR(gdt_seg, gdt_far[4], GDT_CODE | lim | GDT_BASE(BUILD_BIOS_ADDR));
    // Initialize SS descriptor
    loc = (u32)MAKE_FLATPTR(GET_SEG(SS), 0);
    SET_FARVAR(gdt_seg, gdt_far[5], GDT_DATA | lim | GDT_BASE(loc));

    SET_SEG(ES, gdt_seg);
    u16 എണ്ണം = regs->cx, si = 0, di = 0;
    asm അസ്ഥിര(
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
        : "+a" (gdt_far), "+c"(എണ്ണം), "+m" (__segment_ES)
        : "S" (si), "D" (di)
        : "cc");

    set_a20(prev_a20_enable);

    set_code_success(regs);
}

// Get the amount of extended memory (above 1M)
സ്ഥിതം ശൂന്യം
handle_1588(ഘടന bregs *regs)
{
    u32 rs = GET_GLOBAL(LegacyRamSize);

    // According to Ralf Brown's interrupt the limit should be 15M,
    // but real machines mostly return max. 63M.
    എങ്കിൽ (rs > 64*1024*1024)
        regs->ax = 63 * 1024;
    അല്ലെങ്കിൽ
        regs->ax = (rs - 1*1024*1024) / 1024;
    set_success(regs);
}

// Switch to protected mode
ശൂന്യം VISIBLE16
handle_1589(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    set_a20(1);

    pic_reset(regs->bl, regs->bh);

    u64 *gdt_far = (ശൂന്യം*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    SET_FARVAR(gdt_seg, gdt_far[7], GDT_CODE | GDT_LIMIT(BUILD_BIOS_SIZE-1)
               | GDT_BASE(BUILD_BIOS_ADDR));

    regs->ds = 3<<3; // 3rd gdt descriptor is %ds
    regs->es = 4<<3; // 4th gdt descriptor is %es
    regs->code.seg = 6<<3; // 6th gdt descriptor is %cs

    set_code_success(regs);

    asm അസ്ഥിര(
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
സ്ഥിതം ശൂന്യം
handle_1590(ഘടന bregs *regs)
{
}

// Interrupt complete.  Called by Int 16h when key becomes available
സ്ഥിതം ശൂന്യം
handle_1591(ഘടന bregs *regs)
{
}

// keyboard intercept
സ്ഥിതം ശൂന്യം
handle_154f(ഘടന bregs *regs)
{
    set_invalid_silent(regs);
}

സ്ഥിതം ശൂന്യം
handle_15c0(ഘടന bregs *regs)
{
    regs->es = SEG_BIOS;
    regs->bx = (u32)&BIOS_CONFIG_TABLE;
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_15c1(ഘടന bregs *regs)
{
    regs->es = get_ebda_seg();
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_15e801(ഘടന bregs *regs)
{
    // my real system sets ax and bx to 0
    // this is confirmed by Ralph Brown list
    // but syslinux v1.48 is known to behave
    // strangely if ax is set to 0
    // regs.u.r16.ax = 0;
    // regs.u.r16.bx = 0;

    u32 rs = GET_GLOBAL(LegacyRamSize);

    // Get the amount of extended memory (above 1M)
    എങ്കിൽ (rs > 16*1024*1024) {
        // limit to 15M
        regs->cx = 15*1024;
        // Get the amount of extended memory above 16M in 64k blocks
        regs->dx = (rs - 16*1024*1024) / (64*1024);
    } അല്ലെങ്കിൽ {
        regs->cx = (rs - 1*1024*1024) / 1024;
        regs->dx = 0;
    }

    // Set configured memory equal to extended memory
    regs->ax = regs->cx;
    regs->bx = regs->dx;

    set_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_15e820(ഘടന bregs *regs)
{
    പൂർണ്ണസംഖ്യ എണ്ണം = GET_GLOBAL(e820_എണ്ണം);
    എങ്കിൽ (regs->edx != 0x534D4150 || regs->bx >= എണ്ണം
        || regs->ecx < വലുപ്പം(e820_list[0])) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        തിരികെ;
    }

    memcpy_far(regs->es, (ശൂന്യം*)(regs->di+0)
               , get_global_seg(), &e820_list[regs->bx]
               , വലുപ്പം(e820_list[0]));
    എങ്കിൽ (regs->bx == എണ്ണം-1)
        regs->ebx = 0;
    അല്ലെങ്കിൽ
        regs->ebx++;
    regs->eax = 0x534D4150;
    regs->ecx = വലുപ്പം(e820_list[0]);
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_15e8XX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

സ്ഥിതം ശൂന്യം
handle_15e8(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x01: handle_15e801(regs); നിർത്തുക;
    അവസ്ഥ 0x20: handle_15e820(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_15e8XX(regs); നിർത്തുക;
    }
}

സ്ഥിതം ശൂന്യം
handle_15XX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

// INT 15h System Services Entry Point
ശൂന്യം VISIBLE16
handle_15(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    തിരഞ്ഞെടുപ്പ് (regs->ah) {
    അവസ്ഥ 0x24: handle_1524(regs); നിർത്തുക;
    അവസ്ഥ 0x4f: handle_154f(regs); നിർത്തുക;
    അവസ്ഥ 0x52: handle_1552(regs); നിർത്തുക;
    അവസ്ഥ 0x53: handle_1553(regs); നിർത്തുക;
    അവസ്ഥ 0x5f: handle_155f(regs); നിർത്തുക;
    അവസ്ഥ 0x7f: handle_157f(regs); നിർത്തുക;
    അവസ്ഥ 0x83: handle_1583(regs); നിർത്തുക;
    അവസ്ഥ 0x86: handle_1586(regs); നിർത്തുക;
    അവസ്ഥ 0x87: handle_1587(regs); നിർത്തുക;
    അവസ്ഥ 0x88: handle_1588(regs); നിർത്തുക;
    അവസ്ഥ 0x90: handle_1590(regs); നിർത്തുക;
    അവസ്ഥ 0x91: handle_1591(regs); നിർത്തുക;
    അവസ്ഥ 0xc0: handle_15c0(regs); നിർത്തുക;
    അവസ്ഥ 0xc1: handle_15c1(regs); നിർത്തുക;
    അവസ്ഥ 0xc2: handle_15c2(regs); നിർത്തുക;
    അവസ്ഥ 0xe8: handle_15e8(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_15XX(regs); നിർത്തുക;
    }
}
