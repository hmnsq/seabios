// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "bregs.h" // CR0_PE
#شامل "fw/paravirt.h" // PORT_SMI_CMD
#شامل "hw/rtc.h" // rtc_use
#شامل "list.h" // hlist_node
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadint
#شامل "stacks.h" // struct mutex_s
#شامل "string.h" // memset
#شامل "util.h" // useRTC

#تعریف_کرو MAIN_ڈھیر_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

ساخت {
    u8 طریقہ_2;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    ساخت descloc_s gdt;
} Call16_ڈیٹا VARLOW;

#تعریف_کرو C16_BIG 1
#تعریف_کرو C16_SMM 2

صحیح HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
ساکن صحیح
call32_prep(u8 طریقہ_2)
{
    اگر (!CONFIG_CALL32_SMM || طریقہ_2 != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        اگر (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            واپس -1;
        سیٹ_کم(Call16_ڈیٹا.cr0, cr0);

        // Backup fs/gs and gdt
        سیٹ_کم(Call16_ڈیٹا.fs, GET_SEG(FS));
        سیٹ_کم(Call16_ڈیٹا.gs, GET_SEG(GS));
        ساخت descloc_s gdt;
        sgdt(&gdt);
        سیٹ_کم(Call16_ڈیٹا.gdt.لمبائی, gdt.لمبائی);
        سیٹ_کم(Call16_ڈیٹا.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        سیٹ_کم(Call16_ڈیٹا.a20, سیٹ_a20(1));
    }

    // Backup ss
    سیٹ_کم(Call16_ڈیٹا.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(پورٹ_CMOS_اشاریہ);
    اگر (!(cmosindex & NMI_DISABLE_ثنائی_ہندسہ)) {
        outb(cmosindex | NMI_DISABLE_ثنائی_ہندسہ, پورٹ_CMOS_اشاریہ);
        inb(پورٹ_CMOS_ڈیٹا);
    }
    سیٹ_کم(Call16_ڈیٹا.cmosindex, cmosindex);

    سیٹ_کم(Call16_ڈیٹا.طریقہ_2, طریقہ_2);
    واپس 0;
}

// Restore state backed up during call32
ساکن u8
call32_post(خالی)
{
    u8 طریقہ_2 = GET_کم(Call16_ڈیٹا.طریقہ_2);
    سیٹ_کم(Call16_ڈیٹا.طریقہ_2, 0);
    سیٹ_کم(Call16_ڈیٹا.ss, 0);

    اگر (!CONFIG_CALL32_SMM || طریقہ_2 != C16_SMM) {
        // Restore a20
        u8 a20 = GET_کم(Call16_ڈیٹا.a20);
        اگر (!a20)
            سیٹ_a20(0);

        // Restore gdt and fs/gs
        ساخت descloc_s gdt;
        gdt.لمبائی = GET_کم(Call16_ڈیٹا.gdt.لمبائی);
        gdt.addr = GET_کم(Call16_ڈیٹا.gdt.addr);
        lgdt(&gdt);
        سیٹ_SEG(FS, GET_کم(Call16_ڈیٹا.fs));
        سیٹ_SEG(GS, GET_کم(Call16_ڈیٹا.gs));

        // Restore cr0
        u32 cr0_caching = GET_کم(Call16_ڈیٹا.cr0) & (CR0_CD|CR0_NW);
        اگر (cr0_caching)
            cr0_نقاب(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_کم(Call16_ڈیٹا.cmosindex);
    اگر (!(cmosindex & NMI_DISABLE_ثنائی_ہندسہ)) {
        outb(cmosindex, پورٹ_CMOS_اشاریہ);
        inb(پورٹ_CMOS_ڈیٹا);
    }
    واپس طریقہ_2;
}

// Force next call16() to restore to a pristine cpu environment state
ساکن خالی
call16_override(صحیح big)
{
    ASSERT32FLAT();
    اگر (getesp() > BUILD_ڈھیر_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16_ڈیٹا, 0, حجم(Call16_ڈیٹا));
    اگر (big) {
        Call16_ڈیٹا.طریقہ_2 = C16_BIG;
        Call16_ڈیٹا.a20 = 1;
    } ورنہ {
        Call16_ڈیٹا.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 طریقہ_2 = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(طریقہ_2);
    واپس ret;
}

#تعریف_کرو ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#تعریف_کرو ASM32_BACK32   "  .popsection\n  .code32\n"
#تعریف_کرو ASM16_SWITCH32 "  .code32\n"
#تعریف_کرو ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
ساکن u32
call32_smm(خالی *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm متلون(
        // Backup esp / set esp to flat stack location
        "  movl %%esp, %0\n"
        "  movl %%ss, %%eax\n"
        "  shll $4, %%eax\n"
        "  addl %%eax, %%esp\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%ebx\n"
        "  outb %%al, $" __stringify(پورٹ_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM16_SWITCH32
        "1:movl %1, %%eax\n"
        "  calll *%2\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(پورٹ_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Restore esp
        ASM16_BACK16
        "2:movl %0, %%esp\n"
        : "=&r" (bkup_esp), "+r" (eax)
        : "r" (func)
        : "eax", "ecx", "edx", "ebx", "cc", "memory");
    call32_post();

    dprintf(9, "call32_smm done %p %x\n", func, eax);
    واپس eax;
}

ساکن u32
call16_smm(u32 eax, u32 edx, خالی *func)
{
    ASSERT32FLAT();
    اگر (!CONFIG_CALL32_SMM)
        واپس eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16_ڈیٹا.ss << 4;
    asm متلون(
        // Restore esp
        "  subl %0, %%esp\n"

        // Transition to 16bit mode, call func, return to 32bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%ebx\n"
        "  outb %%al, $" __stringify(پورٹ_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM32_SWITCH16
        "1:movl %1, %%eax\n"
        "  movl %3, %%ecx\n"
        "  calll _cfunc16_call16_helper\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(پورٹ_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Set esp to flat stack location
        ASM32_BACK32
        "2:addl %0, %%esp\n"
        : "+r" (stackoffset), "+r" (eax), "+d" (edx)
        : "r" (func)
        : "eax", "ecx", "ebx", "cc", "memory");
    واپس eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(خالی *func, u32 eax, u32 errret)
{
    ASSERT16();
    اگر (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        واپس call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    صحیح ret = call32_prep(C16_BIG);
    اگر (ret)
        واپس errret;
    u32 bkup_ss, bkup_esp;
    asm متلون(
        // Backup ss/esp / set esp to flat stack location
        "  movl %%ss, %0\n"
        "  movl %%esp, %1\n"
        "  shll $4, %0\n"
        "  addl %0, %%esp\n"
        "  shrl $4, %0\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%edx\n"
        "  jmp transition32_nmi_off\n"
        ASM16_SWITCH32
        "1:calll *%3\n"
        "  movl $2f, %%edx\n"
        "  jmp transition16big\n"

        // Restore ds/ss/esp
        ASM16_BACK16
        "2:movl %0, %%ds\n"
        "  movl %0, %%ss\n"
        "  movl %1, %%esp\n"
        : "=&r" (bkup_ss), "=&r" (bkup_esp), "+a" (eax)
        : "r" (func)
        : "ecx", "edx", "cc", "memory");
    call32_post();
    واپس eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
ساکن u32
call16(u32 eax, u32 edx, خالی *func)
{
    ASSERT32FLAT();
    اگر (getesp() > MAIN_ڈھیر_MAX)
        panic("call16 with invalid stack\n");
    اگر (CONFIG_CALL32_SMM && Call16_ڈیٹا.طریقہ_2 == C16_SMM)
        واپس call16_smm(eax, edx, func);

    خارجی خالی transition16big(خالی);
    خارجی خالی transition16(خالی);
    خالی *thunk = transition16;
    اگر (Call16_ڈیٹا.طریقہ_2 == C16_BIG || اندر_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16_ڈیٹا.ss;
    asm متلون(
        // Transition to 16bit mode
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%edx\n"
        "  jmp *%%ecx\n"
        // Setup ss/esp and call func
        ASM32_SWITCH16
        "1:movl %2, %%ecx\n"
        "  shll $4, %2\n"
        "  movw %%cx, %%ss\n"
        "  subl %2, %%esp\n"
        "  movw %%cx, %%ds\n"
        "  movl %4, %%edx\n"
        "  movl %3, %%ecx\n"
        "  calll _cfunc16_call16_helper\n"
        // Return to 32bit and restore esp
        "  movl $2f, %%edx\n"
        "  jmp transition32_nmi_off\n"
        ASM32_BACK32
        "2:addl %2, %%esp\n"
        : "+a" (eax), "+c"(thunk), "+r"(stackseg)
        : "r" (func), "r" (edx)
        : "edx", "cc", "memory");
    واپس eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_ڈھیر[BUILD_EXTRA_ڈھیر_سائز+1] VARLOW __aligned(8);
u8 *ڈھیر_Pos VARLOW;

// Test if currently on the extra stack
صحیح
on_extra_ڈھیر(خالی)
{
    واپس MODE16 && GET_SEG(SS) == SEG_کم && getesp() > (u32)Extra_ڈھیر;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, خالی *func)
{
    اگر (on_extra_ڈھیر())
        واپس ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 ڈھیر_seg = SEG_کم;
    u32 bkup_ss, bkup_esp;
    asm متلون(
        // Backup current %ss/%esp values.
        "movw %%ss, %w3\n"
        "movl %%esp, %4\n"
        // Copy stack seg to %ds/%ss and set %esp
        "movw %w6, %%ds\n"
        "movw %w6, %%ss\n"
        "movl %5, %%esp\n"
        "pushl %3\n"
        "pushl %4\n"
        // Call func
        "calll *%2\n"
        "popl %4\n"
        "popl %3\n"
        // Restore segments and stack
        "movw %w3, %%ds\n"
        "movw %w3, %%ss\n"
        "movl %4, %%esp"
        : "+a" (eax), "+d" (edx), "+c" (func), "=&r" (bkup_ss), "=&r" (bkup_esp)
        : "m" (ڈھیر_Pos), "r" (ڈھیر_seg)
        : "cc", "memory");
    واپس eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, خالی *func)
{
    اگر (!MODESEGMENT)
        واپس call16(eax, edx, func);
    اگر (!MODE16 || !on_extra_ڈھیر())
        واپس ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_ڈھیر_pos, عارضی;
    asm متلون(
        // Backup stack_pos and current %ss/%esp
        "movl %6, %4\n"
        "movw %%ss, %w3\n"
        "movl %%esp, %6\n"
        // Restore original callers' %ss/%esp
        "movl -4(%4), %5\n"
        "movl %5, %%ss\n"
        "movw %%ds:-8(%4), %%sp\n"
        "movl %5, %%ds\n"
        // Call func
        "calll *%2\n"
        // Restore %ss/%esp and stack_pos
        "movw %w3, %%ds\n"
        "movw %w3, %%ss\n"
        "movl %6, %%esp\n"
        "movl %4, %6"
        : "+a" (eax), "+d" (edx), "+c" (func), "=&r" (bkup_ss)
          , "=&r" (bkup_ڈھیر_pos), "=&r" (عارضی), "+m" (ڈھیر_Pos)
        :
        : "cc", "memory");
    واپس eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
خالی VISIBLE16
_farcall16(ساخت bregs *callregs, u16 callregseg)
{
    اگر (need_hop_عقب()) {
        ڈھیر_hop_عقب(_farcall16, callregs, callregseg);
        واپس;
    }
    ASSERT16();
    asm متلون(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
خالی
farcall16(ساخت bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
خالی
farcall16big(ساخت bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
خالی
__call16_int(ساخت bregs *callregs, u16 انحراف_2)
{
    callregs->رمز.انحراف_2 = انحراف_2;
    اگر (!MODESEGMENT) {
        callregs->رمز.seg = SEG_BIOS;
        _farcall16((خالی*)callregs - Call16_ڈیٹا.ss * 16, Call16_ڈیٹا.ss);
        واپس;
    }
    callregs->رمز.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
خالی
reset(خالی)
{
    خارجی خالی reset_سمتیہ(خالی) __noreturn;
    اگر (!MODE16)
        call16(0, 0, reset_سمتیہ);
    reset_سمتیہ();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
ساخت thread_info {
    خالی *stackpos;
    ساخت hlist_گرہ گرہ;
};
ساخت thread_info MainThread VARFSEG = {
    تہی, { &MainThread.گرہ, &MainThread.گرہ.اگلا }
};
#تعریف_کرو THREADSTACKSIZE 4096

// Check if any threads are running.
ساکن صحیح
have_دھاگے(خالی)
{
    واپس (CONFIG_دھاگے
            && GET_FLATPTR(MainThread.گرہ.اگلا) != &MainThread.گرہ);
}

// Return the 'struct thread_info' for the currently running thread.
ساخت thread_info *
getCurThread(خالی)
{
    u32 esp = getesp();
    اگر (esp <= MAIN_ڈھیر_MAX)
        واپس &MainThread;
    واپس (خالی*)ALIGN_نیچے_کو(esp, THREADSTACKSIZE);
}

ساکن u8 Can_مداخلت, ThreadControl;

// Initialize the support for internal threads.
خالی
thread_تیاری(خالی)
{
    Can_مداخلت = 1;
    call16_override(1);
    اگر (! CONFIG_دھاگے)
        واپس;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
صحیح
دھاگے_during_optionroms(خالی)
{
    واپس CONFIG_دھاگے && CONFIG_RTC_TIMER && ThreadControl == 2 && اندر_post();
}

// Switch to next thread stack.
ساکن خالی
switch_اگلا(ساخت thread_info *cur)
{
    ساخت thread_info *اگلا = container_کا(
        cur->گرہ.اگلا, ساخت thread_info, گرہ);
    اگر (cur == اگلا)
        // Nothing to do.
        واپس;
    asm متلون(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(اگلا)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
ساکن خالی
__end_thread(ساخت thread_info *old)
{
    hlist_del(&old->گرہ);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    آزاد(old);
    اگر (!have_دھاگے())
        dprintf(1, "All threads complete.\n");
}

خالی VISIBLE16 check_irqs(خالی);

// Create a new thread and start executing 'func' in it.
خالی
تسلسل_thread(خالی (*func)(خالی*), خالی *ڈیٹا)
{
    ASSERT32FLAT();
    اگر (! CONFIG_دھاگے || ! ThreadControl)
        جاؤ fail;
    ساخت thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    اگر (!thread)
        جاؤ fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (خالی*)thread + THREADSTACKSIZE;
    ساخت thread_info *cur = getCurThread();
    ساخت thread_info *edx = cur;
    hlist_جوڑو_after(&thread->گرہ, &cur->گرہ);
    asm متلون(
        // Start thread
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%edx)\n"       // cur->stackpos = %esp
        "  movl (%%ebx), %%esp\n"       // %esp = thread->stackpos
        "  calll *%%ecx\n"              // Call func

        // End thread
        "  movl %%ebx, %%eax\n"         // %eax = thread
        "  movl 4(%%ebx), %%ebx\n"      // %ebx = thread->node.next
        "  movl (%5), %%esp\n"          // %esp = MainThread.stackpos
        "  calll %4\n"                  // call __end_thread(thread)
        "  movl -4(%%ebx), %%esp\n"     // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(ڈیٹا), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    اگر (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    واپس;

fail:
    func(ڈیٹا);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
خالی VISIBLE16
check_irqs(خالی)
{
    اگر (!MODESEGMENT && !Can_مداخلت) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        واپس;
    }
    اگر (need_hop_عقب()) {
        ڈھیر_hop_عقب(check_irqs, 0, 0);
        واپس;
    }
    اگر (MODE16)
        clock_poll_irq();
    asm متلون("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
خالی
yield(خالی)
{
    اگر (MODESEGMENT || !CONFIG_دھاگے) {
        check_irqs();
        واپس;
    }
    ساخت thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_اگلا(cur);
    اگر (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

خالی VISIBLE16
wait_irq(خالی)
{
    اگر (need_hop_عقب()) {
        ڈھیر_hop_عقب(wait_irq, 0, 0);
        واپس;
    }
    asm متلون("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
خالی
yield_toirq(خالی)
{
    اگر (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_دھاگے() || !Can_مداخلت))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        واپس;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
خالی
wait_دھاگے(خالی)
{
    ASSERT32FLAT();
    جب (have_دھاگے())
        yield();
}

خالی
mutex_تالا(ساخت mutex_s *mutex)
{
    ASSERT32FLAT();
    اگر (! CONFIG_دھاگے)
        واپس;
    جب (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

خالی
mutex_unlock(ساخت mutex_s *mutex)
{
    ASSERT32FLAT();
    اگر (! CONFIG_دھاگے)
        واپس;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

صحیح CanPreempt VARFSEG;
ساکن u32 Preempt_گنتی;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
خالی
آغاز_preempt(خالی)
{
    اگر (! دھاگے_during_optionroms())
        واپس;
    CanPreempt = 1;
    Preempt_گنتی = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
خالی
اختتام_preempt(خالی)
{
    اگر (! دھاگے_during_optionroms()) {
        yield();
        واپس;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_گنتی);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
صحیح
wait_preempt(خالی)
{
    اگر (MODESEGMENT || !CONFIG_دھاگے || !CanPreempt
        || getesp() < MAIN_ڈھیر_MAX)
        واپس 0;
    جب (CanPreempt)
        yield();
    واپس 1;
}

// Try to execute 32bit threads.
خالی VISIBLE32INIT
yield_preempt(خالی)
{
    Preempt_گنتی++;
    switch_اگلا(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
خالی
check_preempt(خالی)
{
    اگر (CONFIG_دھاگے && GET_GLOBAL(CanPreempt) && have_دھاگے())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

ساخت call32_params_s {
    خالی *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(ساخت call32_params_s *params)
{
    واپس ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(خالی *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    ساخت call32_params_s params = {func, eax, edx, ecx};
    واپس call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
