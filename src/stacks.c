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

#define MAIN_پشته_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

ساختار {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    ساختار descloc_s gdt;
} Call16Data VARLOW;

#define C16_BIG 1
#define C16_SMM 2

صحیح HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
ایستا صحیح
call32_prep(u8 method)
{
    اگر (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        اگر (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            بازگشت -1;
        SET_کم(Call16Data.cr0, cr0);

        // Backup fs/gs and gdt
        SET_کم(Call16Data.fs, GET_SEG(FS));
        SET_کم(Call16Data.gs, GET_SEG(GS));
        ساختار descloc_s gdt;
        sgdt(&gdt);
        SET_کم(Call16Data.gdt.length, gdt.length);
        SET_کم(Call16Data.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_کم(Call16Data.a20, set_a20(1));
    }

    // Backup ss
    SET_کم(Call16Data.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_INDEX);
    اگر (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    SET_کم(Call16Data.cmosindex, cmosindex);

    SET_کم(Call16Data.method, method);
    بازگشت 0;
}

// Restore state backed up during call32
ایستا u8
call32_post(تهی)
{
    u8 method = GET_کم(Call16Data.method);
    SET_کم(Call16Data.method, 0);
    SET_کم(Call16Data.ss, 0);

    اگر (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_کم(Call16Data.a20);
        اگر (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        ساختار descloc_s gdt;
        gdt.length = GET_کم(Call16Data.gdt.length);
        gdt.addr = GET_کم(Call16Data.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_کم(Call16Data.fs));
        SET_SEG(GS, GET_کم(Call16Data.gs));

        // Restore cr0
        u32 cr0_caching = GET_کم(Call16Data.cr0) & (CR0_CD|CR0_NW);
        اگر (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_کم(Call16Data.cmosindex);
    اگر (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    بازگشت method;
}

// Force next call16() to restore to a pristine cpu environment state
ایستا تهی
call16_override(صحیح big)
{
    ASSERT32FLAT();
    اگر (getesp() > BUILD_پشته_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16Data, 0, اندازه(Call16Data));
    اگر (big) {
        Call16Data.method = C16_BIG;
        Call16Data.a20 = 1;
    } وگرنه {
        Call16Data.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 method = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(method);
    بازگشت ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
ایستا u32
call32_smm(تهی *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm فرار(
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
    بازگشت eax;
}

ایستا u32
call16_smm(u32 eax, u32 edx, تهی *func)
{
    ASSERT32FLAT();
    اگر (!CONFIG_CALL32_SMM)
        بازگشت eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16Data.ss << 4;
    asm فرار(
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
    بازگشت eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(تهی *func, u32 eax, u32 errret)
{
    ASSERT16();
    اگر (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        بازگشت call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    صحیح ret = call32_prep(C16_BIG);
    اگر (ret)
        بازگشت errret;
    u32 bkup_ss, bkup_esp;
    asm فرار(
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
    بازگشت eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
ایستا u32
call16(u32 eax, u32 edx, تهی *func)
{
    ASSERT32FLAT();
    اگر (getesp() > MAIN_پشته_MAX)
        panic("call16 with invalid stack\n");
    اگر (CONFIG_CALL32_SMM && Call16Data.method == C16_SMM)
        بازگشت call16_smm(eax, edx, func);

    بیرونی تهی transition16big(تهی);
    بیرونی تهی transition16(تهی);
    تهی *thunk = transition16;
    اگر (Call16Data.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16Data.ss;
    asm فرار(
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
    بازگشت eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_پشته[BUILD_EXTRA_پشته_SIZE+1] VARLOW __aligned(8);
u8 *پشته_Pos VARLOW;

// Test if currently on the extra stack
صحیح
on_extra_پشته(تهی)
{
    بازگشت MODE16 && GET_SEG(SS) == SEG_کم && getesp() > (u32)Extra_پشته;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, تهی *func)
{
    اگر (on_extra_پشته())
        بازگشت ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 پشته_seg = SEG_کم;
    u32 bkup_ss, bkup_esp;
    asm فرار(
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
        : "m" (پشته_Pos), "r" (پشته_seg)
        : "cc", "memory");
    بازگشت eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, تهی *func)
{
    اگر (!MODESEGMENT)
        بازگشت call16(eax, edx, func);
    اگر (!MODE16 || !on_extra_پشته())
        بازگشت ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_پشته_pos, temp;
    asm فرار(
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
          , "=&r" (bkup_پشته_pos), "=&r" (temp), "+m" (پشته_Pos)
        :
        : "cc", "memory");
    بازگشت eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
تهی VISIBLE16
_farcall16(ساختار bregs *callregs, u16 callregseg)
{
    اگر (need_hop_back()) {
        پشته_hop_back(_farcall16, callregs, callregseg);
        بازگشت;
    }
    ASSERT16();
    asm فرار(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
تهی
farcall16(ساختار bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
تهی
farcall16big(ساختار bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
تهی
__call16_int(ساختار bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    اگر (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((تهی*)callregs - Call16Data.ss * 16, Call16Data.ss);
        بازگشت;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
تهی
reset(تهی)
{
    بیرونی تهی reset_vector(تهی) __noreturn;
    اگر (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
ساختار thread_info {
    تهی *stackpos;
    ساختار hlist_گره گره;
};
ساختار thread_info MainThread VARFSEG = {
    پوچ, { &MainThread.گره, &MainThread.گره.next }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
ایستا صحیح
have_threads(تهی)
{
    بازگشت (CONFIG_THREADS
            && GET_FLATPTR(MainThread.گره.next) != &MainThread.گره);
}

// Return the 'struct thread_info' for the currently running thread.
ساختار thread_info *
getCurThread(تهی)
{
    u32 esp = getesp();
    اگر (esp <= MAIN_پشته_MAX)
        بازگشت &MainThread;
    بازگشت (تهی*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

ایستا u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
تهی
thread_setup(تهی)
{
    CanInterrupt = 1;
    call16_override(1);
    اگر (! CONFIG_THREADS)
        بازگشت;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
صحیح
threads_during_optionroms(تهی)
{
    بازگشت CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
ایستا تهی
switch_next(ساختار thread_info *cur)
{
    ساختار thread_info *next = container_of(
        cur->گره.next, ساختار thread_info, گره);
    اگر (cur == next)
        // Nothing to do.
        بازگشت;
    asm فرار(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(next)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
ایستا تهی
__end_thread(ساختار thread_info *old)
{
    hlist_del(&old->گره);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    آزاد(old);
    اگر (!have_threads())
        dprintf(1, "All threads complete.\n");
}

تهی VISIBLE16 check_irqs(تهی);

// Create a new thread and start executing 'func' in it.
تهی
run_thread(تهی (*func)(تهی*), تهی *data)
{
    ASSERT32FLAT();
    اگر (! CONFIG_THREADS || ! ThreadControl)
        برو fail;
    ساختار thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    اگر (!thread)
        برو fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (تهی*)thread + THREADSTACKSIZE;
    ساختار thread_info *cur = getCurThread();
    ساختار thread_info *edx = cur;
    hlist_add_after(&thread->گره, &cur->گره);
    asm فرار(
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
        : "+a"(data), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    اگر (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    بازگشت;

fail:
    func(data);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
تهی VISIBLE16
check_irqs(تهی)
{
    اگر (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        بازگشت;
    }
    اگر (need_hop_back()) {
        پشته_hop_back(check_irqs, 0, 0);
        بازگشت;
    }
    اگر (MODE16)
        clock_poll_irq();
    asm فرار("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
تهی
yield(تهی)
{
    اگر (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        بازگشت;
    }
    ساختار thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_next(cur);
    اگر (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

تهی VISIBLE16
wait_irq(تهی)
{
    اگر (need_hop_back()) {
        پشته_hop_back(wait_irq, 0, 0);
        بازگشت;
    }
    asm فرار("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
تهی
yield_toirq(تهی)
{
    اگر (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        بازگشت;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
تهی
wait_threads(تهی)
{
    ASSERT32FLAT();
    وقتی (have_threads())
        yield();
}

تهی
mutex_lock(ساختار mutex_s *mutex)
{
    ASSERT32FLAT();
    اگر (! CONFIG_THREADS)
        بازگشت;
    وقتی (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

تهی
mutex_unlock(ساختار mutex_s *mutex)
{
    ASSERT32FLAT();
    اگر (! CONFIG_THREADS)
        بازگشت;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

صحیح CanPreempt VARFSEG;
ایستا u32 Preempt_شمار;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
تهی
start_preempt(تهی)
{
    اگر (! threads_during_optionroms())
        بازگشت;
    CanPreempt = 1;
    Preempt_شمار = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
تهی
finish_preempt(تهی)
{
    اگر (! threads_during_optionroms()) {
        yield();
        بازگشت;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_شمار);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
صحیح
wait_preempt(تهی)
{
    اگر (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_پشته_MAX)
        بازگشت 0;
    وقتی (CanPreempt)
        yield();
    بازگشت 1;
}

// Try to execute 32bit threads.
تهی VISIBLE32INIT
yield_preempt(تهی)
{
    Preempt_شمار++;
    switch_next(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
تهی
check_preempt(تهی)
{
    اگر (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

ساختار call32_params_s {
    تهی *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(ساختار call32_params_s *params)
{
    بازگشت ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(تهی *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    ساختار call32_params_s params = {func, eax, edx, ecx};
    بازگشت call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
