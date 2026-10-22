// Handler for int 0x15 "system" calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "bregs.h" // struct bregs
#شامل "e820map.h" // E820_RAM
#شامل "hw/pic.h" // pic_reset
#شامل "malloc.h" // LegacyRamSize
#شامل "output.h" // debug_enter
#شامل "string.h" // memcpy_far
#شامل "util.h" // handle_1553
#شامل "x86.h" // set_a20

ساکن خالی
handle_152400(ساخت bregs *regs)
{
    سیٹ_a20(0);
    سیٹ_رمز_success(regs);
}

ساکن خالی
handle_152401(ساخت bregs *regs)
{
    سیٹ_a20(1);
    سیٹ_رمز_success(regs);
}

ساکن خالی
handle_152402(ساخت bregs *regs)
{
    regs->al = get_a20();
    سیٹ_رمز_success(regs);
}

ساکن خالی
handle_152403(ساخت bregs *regs)
{
    regs->bx = 3;
    سیٹ_رمز_success(regs);
}

ساکن خالی
handle_1524XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EUNSUPPORTED);
}

ساکن خالی
handle_1524(ساخت bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x00: handle_152400(regs); توڑو;
    حالت 0x01: handle_152401(regs); توڑو;
    حالت 0x02: handle_152402(regs); توڑو;
    حالت 0x03: handle_152403(regs); توڑو;
    طےشدہ:   handle_1524XX(regs); توڑو;
    }
}

// removable media eject
ساکن خالی
handle_1552(ساخت bregs *regs)
{
    سیٹ_رمز_success(regs);
}

ساکن خالی
handle_1587(ساخت bregs *regs)
{
    // +++ should probably have descriptor checks
    // +++ should have exception handlers

    u8 پچھلا_a20_enable = سیٹ_a20(1); // enable A20 line

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
    u64 *gdt_far = (خالی*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    u32 loc = (u32)MAKE_FLATPTR(gdt_seg, gdt_far);
    سیٹ_FARVAR(gdt_seg, gdt_far[1], GDT_ڈیٹا | GDT_حد_بندی((6*حجم(u64))-1)
               | GDT_بنیاد(loc));
    // Initialize CS descriptor
    u64 lim = GDT_حد_بندی(0x0ffff);
    اگر (اندر_post())
        lim = GDT_GRANLIMIT(0xffffffff);
    سیٹ_FARVAR(gdt_seg, gdt_far[4], GDT_رمز | lim | GDT_بنیاد(BUILD_BIOS_ADDR));
    // Initialize SS descriptor
    loc = (u32)MAKE_FLATPTR(GET_SEG(SS), 0);
    سیٹ_FARVAR(gdt_seg, gdt_far[5], GDT_ڈیٹا | lim | GDT_بنیاد(loc));

    سیٹ_SEG(ES, gdt_seg);
    u16 گنتی = regs->cx, si = 0, di = 0;
    asm متلون(
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
        : "+a" (gdt_far), "+c"(گنتی), "+m" (__segment_ES)
        : "S" (si), "D" (di)
        : "cc");

    سیٹ_a20(پچھلا_a20_enable);

    سیٹ_رمز_success(regs);
}

// Get the amount of extended memory (above 1M)
ساکن خالی
handle_1588(ساخت bregs *regs)
{
    u32 rs = GET_GLOBAL(Legacy_Ram_سائز);

    // According to Ralf Brown's interrupt the limit should be 15M,
    // but real machines mostly return max. 63M.
    اگر (rs > 64*1024*1024)
        regs->ax = 63 * 1024;
    ورنہ
        regs->ax = (rs - 1*1024*1024) / 1024;
    سیٹ_success(regs);
}

// Switch to protected mode
خالی VISIBLE16
handle_1589(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    سیٹ_a20(1);

    pic_reset(regs->bl, regs->bh);

    u64 *gdt_far = (خالی*)(regs->si + 0);
    u16 gdt_seg = regs->es;
    سیٹ_FARVAR(gdt_seg, gdt_far[7], GDT_رمز | GDT_حد_بندی(BUILD_BIOS_سائز-1)
               | GDT_بنیاد(BUILD_BIOS_ADDR));

    regs->ds = 3<<3; // 3rd gdt descriptor is %ds
    regs->es = 4<<3; // 4th gdt descriptor is %es
    regs->رمز.seg = 6<<3; // 6th gdt descriptor is %cs

    سیٹ_رمز_success(regs);

    asm متلون(
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
ساکن خالی
handle_1590(ساخت bregs *regs)
{
}

// Interrupt complete.  Called by Int 16h when key becomes available
ساکن خالی
handle_1591(ساخت bregs *regs)
{
}

// keyboard intercept
ساکن خالی
handle_154f(ساخت bregs *regs)
{
    سیٹ_invalid_silent(regs);
}

ساکن خالی
handle_15c0(ساخت bregs *regs)
{
    regs->es = SEG_BIOS;
    regs->bx = (u32)&BIOS_CONFIG_جدول;
    سیٹ_رمز_success(regs);
}

ساکن خالی
handle_15c1(ساخت bregs *regs)
{
    regs->es = get_ebda_seg();
    سیٹ_success(regs);
}

ساکن خالی
handle_15e801(ساخت bregs *regs)
{
    // my real system sets ax and bx to 0
    // this is confirmed by Ralph Brown list
    // but syslinux v1.48 is known to behave
    // strangely if ax is set to 0
    // regs.u.r16.ax = 0;
    // regs.u.r16.bx = 0;

    u32 rs = GET_GLOBAL(Legacy_Ram_سائز);

    // Get the amount of extended memory (above 1M)
    اگر (rs > 16*1024*1024) {
        // limit to 15M
        regs->cx = 15*1024;
        // Get the amount of extended memory above 16M in 64k blocks
        regs->dx = (rs - 16*1024*1024) / (64*1024);
    } ورنہ {
        regs->cx = (rs - 1*1024*1024) / 1024;
        regs->dx = 0;
    }

    // Set configured memory equal to extended memory
    regs->ax = regs->cx;
    regs->bx = regs->dx;

    سیٹ_success(regs);
}

ساکن خالی
handle_15e820(ساخت bregs *regs)
{
    صحیح گنتی = GET_GLOBAL(e820_گنتی);
    اگر (regs->edx != 0x534D4150 || regs->bx >= گنتی
        || regs->ecx < حجم(e820_list[0])) {
        سیٹ_رمز_invalid(regs, RET_EUNSUPPORTED);
        واپس;
    }

    memcpy_far(regs->es, (خالی*)(regs->di+0)
               , get_global_seg(), &e820_list[regs->bx]
               , حجم(e820_list[0]));
    اگر (regs->bx == گنتی-1)
        regs->ebx = 0;
    ورنہ
        regs->ebx++;
    regs->eax = 0x534D4150;
    regs->ecx = حجم(e820_list[0]);
    سیٹ_success(regs);
}

ساکن خالی
handle_15e8XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EUNSUPPORTED);
}

ساکن خالی
handle_15e8(ساخت bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x01: handle_15e801(regs); توڑو;
    حالت 0x20: handle_15e820(regs); توڑو;
    طےشدہ:   handle_15e8XX(regs); توڑو;
    }
}

ساکن خالی
handle_15XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EUNSUPPORTED);
}

// INT 15h System Services Entry Point
خالی VISIBLE16
handle_15(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_15);
    انتخاب (regs->ah) {
    حالت 0x24: handle_1524(regs); توڑو;
    حالت 0x4f: handle_154f(regs); توڑو;
    حالت 0x52: handle_1552(regs); توڑو;
    حالت 0x53: handle_1553(regs); توڑو;
    حالت 0x5f: handle_155f(regs); توڑو;
    حالت 0x7f: handle_157f(regs); توڑو;
    حالت 0x83: handle_1583(regs); توڑو;
    حالت 0x86: handle_1586(regs); توڑو;
    حالت 0x87: handle_1587(regs); توڑو;
    حالت 0x88: handle_1588(regs); توڑو;
    حالت 0x90: handle_1590(regs); توڑو;
    حالت 0x91: handle_1591(regs); توڑو;
    حالت 0xc0: handle_15c0(regs); توڑو;
    حالت 0xc1: handle_15c1(regs); توڑو;
    حالت 0xc2: handle_15c2(regs); توڑو;
    حالت 0xe8: handle_15e8(regs); توڑو;
    طےشدہ:   handle_15XX(regs); توڑو;
    }
}
