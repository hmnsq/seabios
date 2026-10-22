// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "bregs.h" // CR0_PE
#অন্তর্ভাব "fw/paravirt.h" // PORT_SMI_CMD
#অন্তর্ভাব "hw/rtc.h" // rtc_use
#অন্তর্ভাব "list.h" // hlist_node
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "stacks.h" // struct mutex_s
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // useRTC

#define MAIN_স্তূপ_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

কাঠামো {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    কাঠামো descloc_s gdt;
} Call16_তথ্য VARLOW;

#define C16_BIG 1
#define C16_SMM 2

পূর্ণসংখ্যা HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
স্থির পূর্ণসংখ্যা
call32_prep(u8 method)
{
    যদি (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        যদি (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            ফেরত -1;
        SET_নিচের(Call16_তথ্য.cr0, cr0);

        // Backup fs/gs and gdt
        SET_নিচের(Call16_তথ্য.fs, GET_SEG(FS));
        SET_নিচের(Call16_তথ্য.gs, GET_SEG(GS));
        কাঠামো descloc_s gdt;
        sgdt(&gdt);
        SET_নিচের(Call16_তথ্য.gdt.দৈর্ঘ্য, gdt.দৈর্ঘ্য);
        SET_নিচের(Call16_তথ্য.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_নিচের(Call16_তথ্য.a20, set_a20(1));
    }

    // Backup ss
    SET_নিচের(Call16_তথ্য.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_সূচক);
    যদি (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_সূচক);
        inb(PORT_CMOS_তথ্য);
    }
    SET_নিচের(Call16_তথ্য.cmosindex, cmosindex);

    SET_নিচের(Call16_তথ্য.method, method);
    ফেরত 0;
}

// Restore state backed up during call32
স্থির u8
call32_post(শূন্য)
{
    u8 method = GET_নিচের(Call16_তথ্য.method);
    SET_নিচের(Call16_তথ্য.method, 0);
    SET_নিচের(Call16_তথ্য.ss, 0);

    যদি (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_নিচের(Call16_তথ্য.a20);
        যদি (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        কাঠামো descloc_s gdt;
        gdt.দৈর্ঘ্য = GET_নিচের(Call16_তথ্য.gdt.দৈর্ঘ্য);
        gdt.addr = GET_নিচের(Call16_তথ্য.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_নিচের(Call16_তথ্য.fs));
        SET_SEG(GS, GET_নিচের(Call16_তথ্য.gs));

        // Restore cr0
        u32 cr0_caching = GET_নিচের(Call16_তথ্য.cr0) & (CR0_CD|CR0_NW);
        যদি (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_নিচের(Call16_তথ্য.cmosindex);
    যদি (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_সূচক);
        inb(PORT_CMOS_তথ্য);
    }
    ফেরত method;
}

// Force next call16() to restore to a pristine cpu environment state
স্থির শূন্য
call16_override(পূর্ণসংখ্যা big)
{
    ASSERT32FLAT();
    যদি (getesp() > BUILD_স্তূপ_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16_তথ্য, 0, আকার(Call16_তথ্য));
    যদি (big) {
        Call16_তথ্য.method = C16_BIG;
        Call16_তথ্য.a20 = 1;
    } অন্যথা {
        Call16_তথ্য.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 method = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(method);
    ফেরত ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
স্থির u32
call32_smm(শূন্য *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm অস্থির(
        // Backup esp / set esp to flat stack location
        "  movl %%esp, %0\n"
        "  movl %%ss, %%eax\n"
        "  shll $4, %%eax\n"
        "  addl %%eax, %%esp\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%ebx\n"
        "  outb %%al, $" __stringify(PORT_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM16_SWITCH32
        "1:movl %1, %%eax\n"
        "  calll *%2\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(PORT_SMI_CMD) "\n"
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
    ফেরত eax;
}

স্থির u32
call16_smm(u32 eax, u32 edx, শূন্য *func)
{
    ASSERT32FLAT();
    যদি (!CONFIG_CALL32_SMM)
        ফেরত eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16_তথ্য.ss << 4;
    asm অস্থির(
        // Restore esp
        "  subl %0, %%esp\n"

        // Transition to 16bit mode, call func, return to 32bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%ebx\n"
        "  outb %%al, $" __stringify(PORT_SMI_CMD) "\n"
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
        "  outb %%al, $" __stringify(PORT_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Set esp to flat stack location
        ASM32_BACK32
        "2:addl %0, %%esp\n"
        : "+r" (stackoffset), "+r" (eax), "+d" (edx)
        : "r" (func)
        : "eax", "ecx", "ebx", "cc", "memory");
    ফেরত eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(শূন্য *func, u32 eax, u32 errret)
{
    ASSERT16();
    যদি (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        ফেরত call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    পূর্ণসংখ্যা ret = call32_prep(C16_BIG);
    যদি (ret)
        ফেরত errret;
    u32 bkup_ss, bkup_esp;
    asm অস্থির(
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
    ফেরত eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
স্থির u32
call16(u32 eax, u32 edx, শূন্য *func)
{
    ASSERT32FLAT();
    যদি (getesp() > MAIN_স্তূপ_MAX)
        panic("call16 with invalid stack\n");
    যদি (CONFIG_CALL32_SMM && Call16_তথ্য.method == C16_SMM)
        ফেরত call16_smm(eax, edx, func);

    বাহ্যিক শূন্য transition16big(শূন্য);
    বাহ্যিক শূন্য transition16(শূন্য);
    শূন্য *thunk = transition16;
    যদি (Call16_তথ্য.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16_তথ্য.ss;
    asm অস্থির(
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
    ফেরত eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_স্তূপ[BUILD_EXTRA_স্তূপ_আকার+1] VARLOW __aligned(8);
u8 *স্তূপ_Pos VARLOW;

// Test if currently on the extra stack
পূর্ণসংখ্যা
on_extra_স্তূপ(শূন্য)
{
    ফেরত MODE16 && GET_SEG(SS) == SEG_নিচের && getesp() > (u32)Extra_স্তূপ;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, শূন্য *func)
{
    যদি (on_extra_স্তূপ())
        ফেরত ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 স্তূপ_seg = SEG_নিচের;
    u32 bkup_ss, bkup_esp;
    asm অস্থির(
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
        : "m" (স্তূপ_Pos), "r" (স্তূপ_seg)
        : "cc", "memory");
    ফেরত eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, শূন্য *func)
{
    যদি (!MODESEGMENT)
        ফেরত call16(eax, edx, func);
    যদি (!MODE16 || !on_extra_স্তূপ())
        ফেরত ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_স্তূপ_pos, অস্থায়ী;
    asm অস্থির(
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
          , "=&r" (bkup_স্তূপ_pos), "=&r" (অস্থায়ী), "+m" (স্তূপ_Pos)
        :
        : "cc", "memory");
    ফেরত eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
শূন্য VISIBLE16
_farcall16(কাঠামো bregs *callregs, u16 callregseg)
{
    যদি (need_hop_back()) {
        স্তূপ_hop_back(_farcall16, callregs, callregseg);
        ফেরত;
    }
    ASSERT16();
    asm অস্থির(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
শূন্য
farcall16(কাঠামো bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
শূন্য
farcall16big(কাঠামো bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
শূন্য
__call16_int(কাঠামো bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    যদি (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((শূন্য*)callregs - Call16_তথ্য.ss * 16, Call16_তথ্য.ss);
        ফেরত;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
শূন্য
reset(শূন্য)
{
    বাহ্যিক শূন্য reset_vector(শূন্য) __noreturn;
    যদি (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
কাঠামো thread_info {
    শূন্য *stackpos;
    কাঠামো hlist_গ্রন্থি গ্রন্থি;
};
কাঠামো thread_info MainThread VARFSEG = {
    ফাঁকা, { &MainThread.গ্রন্থি, &MainThread.গ্রন্থি.পরবর্তী }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
স্থির পূর্ণসংখ্যা
have_threads(শূন্য)
{
    ফেরত (CONFIG_THREADS
            && GET_FLATPTR(MainThread.গ্রন্থি.পরবর্তী) != &MainThread.গ্রন্থি);
}

// Return the 'struct thread_info' for the currently running thread.
কাঠামো thread_info *
getCurThread(শূন্য)
{
    u32 esp = getesp();
    যদি (esp <= MAIN_স্তূপ_MAX)
        ফেরত &MainThread;
    ফেরত (শূন্য*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

স্থির u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
শূন্য
thread_setup(শূন্য)
{
    CanInterrupt = 1;
    call16_override(1);
    যদি (! CONFIG_THREADS)
        ফেরত;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
পূর্ণসংখ্যা
threads_during_optionroms(শূন্য)
{
    ফেরত CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
স্থির শূন্য
switch_পরবর্তী(কাঠামো thread_info *cur)
{
    কাঠামো thread_info *পরবর্তী = container_of(
        cur->গ্রন্থি.পরবর্তী, কাঠামো thread_info, গ্রন্থি);
    যদি (cur == পরবর্তী)
        // Nothing to do.
        ফেরত;
    asm অস্থির(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(পরবর্তী)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
স্থির শূন্য
__end_thread(কাঠামো thread_info *old)
{
    hlist_del(&old->গ্রন্থি);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    মুক্ত(old);
    যদি (!have_threads())
        dprintf(1, "All threads complete.\n");
}

শূন্য VISIBLE16 check_irqs(শূন্য);

// Create a new thread and start executing 'func' in it.
শূন্য
run_thread(শূন্য (*func)(শূন্য*), শূন্য *তথ্য)
{
    ASSERT32FLAT();
    যদি (! CONFIG_THREADS || ! ThreadControl)
        যাও fail;
    কাঠামো thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    যদি (!thread)
        যাও fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (শূন্য*)thread + THREADSTACKSIZE;
    কাঠামো thread_info *cur = getCurThread();
    কাঠামো thread_info *edx = cur;
    hlist_add_after(&thread->গ্রন্থি, &cur->গ্রন্থি);
    asm অস্থির(
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
        : "+a"(তথ্য), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    যদি (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    ফেরত;

fail:
    func(তথ্য);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
শূন্য VISIBLE16
check_irqs(শূন্য)
{
    যদি (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        ফেরত;
    }
    যদি (need_hop_back()) {
        স্তূপ_hop_back(check_irqs, 0, 0);
        ফেরত;
    }
    যদি (MODE16)
        clock_poll_irq();
    asm অস্থির("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
শূন্য
yield(শূন্য)
{
    যদি (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        ফেরত;
    }
    কাঠামো thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_পরবর্তী(cur);
    যদি (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

শূন্য VISIBLE16
wait_irq(শূন্য)
{
    যদি (need_hop_back()) {
        স্তূপ_hop_back(wait_irq, 0, 0);
        ফেরত;
    }
    asm অস্থির("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
শূন্য
yield_toirq(শূন্য)
{
    যদি (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        ফেরত;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
শূন্য
wait_threads(শূন্য)
{
    ASSERT32FLAT();
    যখন (have_threads())
        yield();
}

শূন্য
mutex_lock(কাঠামো mutex_s *mutex)
{
    ASSERT32FLAT();
    যদি (! CONFIG_THREADS)
        ফেরত;
    যখন (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

শূন্য
mutex_unlock(কাঠামো mutex_s *mutex)
{
    ASSERT32FLAT();
    যদি (! CONFIG_THREADS)
        ফেরত;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

পূর্ণসংখ্যা CanPreempt VARFSEG;
স্থির u32 Preempt_গুনতি;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
শূন্য
start_preempt(শূন্য)
{
    যদি (! threads_during_optionroms())
        ফেরত;
    CanPreempt = 1;
    Preempt_গুনতি = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
শূন্য
finish_preempt(শূন্য)
{
    যদি (! threads_during_optionroms()) {
        yield();
        ফেরত;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_গুনতি);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
পূর্ণসংখ্যা
wait_preempt(শূন্য)
{
    যদি (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_স্তূপ_MAX)
        ফেরত 0;
    যখন (CanPreempt)
        yield();
    ফেরত 1;
}

// Try to execute 32bit threads.
শূন্য VISIBLE32INIT
yield_preempt(শূন্য)
{
    Preempt_গুনতি++;
    switch_পরবর্তী(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
শূন্য
check_preempt(শূন্য)
{
    যদি (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

কাঠামো call32_params_s {
    শূন্য *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(কাঠামো call32_params_s *params)
{
    ফেরত ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(শূন্য *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    কাঠামো call32_params_s params = {func, eax, edx, ecx};
    ফেরত call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
