// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "bregs.h" // CR0_PE
#ضمن "fw/paravirt.h" // PORT_SMI_CMD
#ضمن "hw/rtc.h" // rtc_use
#ضمن "list.h" // hlist_node
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadint
#ضمن "stacks.h" // struct mutex_s
#ضمن "string.h" // memset
#ضمن "util.h" // useRTC

#define MAIN_STACK_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

بنية {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    بنية descloc_s gdt;
} Call16Data VARLOW;

#define C16_BIG 1
#define C16_SMM 2

صحيح HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
ساكن صحيح
call32_prep(u8 method)
{
    𐡄𐡍 (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        𐡄𐡍 (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            أرجع -1;
        SET_LOW(Call16Data.cr0, cr0);

        // Backup fs/gs and gdt
        SET_LOW(Call16Data.fs, GET_SEG(FS));
        SET_LOW(Call16Data.gs, GET_SEG(GS));
        بنية descloc_s gdt;
        sgdt(&gdt);
        SET_LOW(Call16Data.gdt.length, gdt.length);
        SET_LOW(Call16Data.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_LOW(Call16Data.a20, set_a20(1));
    }

    // Backup ss
    SET_LOW(Call16Data.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_INDEX);
    𐡄𐡍 (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    SET_LOW(Call16Data.cmosindex, cmosindex);

    SET_LOW(Call16Data.method, method);
    أرجع 0;
}

// Restore state backed up during call32
ساكن u8
call32_post(فراغ)
{
    u8 method = GET_LOW(Call16Data.method);
    SET_LOW(Call16Data.method, 0);
    SET_LOW(Call16Data.ss, 0);

    𐡄𐡍 (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_LOW(Call16Data.a20);
        𐡄𐡍 (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        بنية descloc_s gdt;
        gdt.length = GET_LOW(Call16Data.gdt.length);
        gdt.addr = GET_LOW(Call16Data.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_LOW(Call16Data.fs));
        SET_SEG(GS, GET_LOW(Call16Data.gs));

        // Restore cr0
        u32 cr0_caching = GET_LOW(Call16Data.cr0) & (CR0_CD|CR0_NW);
        𐡄𐡍 (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_LOW(Call16Data.cmosindex);
    𐡄𐡍 (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    أرجع method;
}

// Force next call16() to restore to a pristine cpu environment state
ساكن فراغ
call16_override(صحيح big)
{
    ASSERT32FLAT();
    𐡄𐡍 (getesp() > BUILD_STACK_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16Data, 0, حجم(Call16Data));
    𐡄𐡍 (big) {
        Call16Data.method = C16_BIG;
        Call16Data.a20 = 1;
    } وإلا {
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
    أرجع ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
ساكن u32
call32_smm(فراغ *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm متطاير(
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
    أرجع eax;
}

ساكن u32
call16_smm(u32 eax, u32 edx, فراغ *func)
{
    ASSERT32FLAT();
    𐡄𐡍 (!CONFIG_CALL32_SMM)
        أرجع eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16Data.ss << 4;
    asm متطاير(
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
    أرجع eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(فراغ *func, u32 eax, u32 errret)
{
    ASSERT16();
    𐡄𐡍 (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        أرجع call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    صحيح ret = call32_prep(C16_BIG);
    𐡄𐡍 (ret)
        أرجع errret;
    u32 bkup_ss, bkup_esp;
    asm متطاير(
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
    أرجع eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
ساكن u32
call16(u32 eax, u32 edx, فراغ *func)
{
    ASSERT32FLAT();
    𐡄𐡍 (getesp() > MAIN_STACK_MAX)
        panic("call16 with invalid stack\n");
    𐡄𐡍 (CONFIG_CALL32_SMM && Call16Data.method == C16_SMM)
        أرجع call16_smm(eax, edx, func);

    خارجي فراغ transition16big(فراغ);
    خارجي فراغ transition16(فراغ);
    فراغ *thunk = transition16;
    𐡄𐡍 (Call16Data.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16Data.ss;
    asm متطاير(
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
    أرجع eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 ExtraStack[BUILD_EXTRA_STACK_SIZE+1] VARLOW __aligned(8);
u8 *StackPos VARLOW;

// Test if currently on the extra stack
صحيح
on_extra_stack(فراغ)
{
    أرجع MODE16 && GET_SEG(SS) == SEG_LOW && getesp() > (u32)ExtraStack;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, فراغ *func)
{
    𐡄𐡍 (on_extra_stack())
        أرجع ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 stack_seg = SEG_LOW;
    u32 bkup_ss, bkup_esp;
    asm متطاير(
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
        : "m" (StackPos), "r" (stack_seg)
        : "cc", "memory");
    أرجع eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, فراغ *func)
{
    𐡄𐡍 (!MODESEGMENT)
        أرجع call16(eax, edx, func);
    𐡄𐡍 (!MODE16 || !on_extra_stack())
        أرجع ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_stack_pos, temp;
    asm متطاير(
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
          , "=&r" (bkup_stack_pos), "=&r" (temp), "+m" (StackPos)
        :
        : "cc", "memory");
    أرجع eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
فراغ VISIBLE16
_farcall16(بنية bregs *callregs, u16 callregseg)
{
    𐡄𐡍 (need_hop_back()) {
        stack_hop_back(_farcall16, callregs, callregseg);
        أرجع;
    }
    ASSERT16();
    asm متطاير(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
فراغ
farcall16(بنية bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
فراغ
farcall16big(بنية bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
فراغ
__call16_int(بنية bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    𐡄𐡍 (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((فراغ*)callregs - Call16Data.ss * 16, Call16Data.ss);
        أرجع;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
فراغ
reset(فراغ)
{
    خارجي فراغ reset_vector(فراغ) __noreturn;
    𐡄𐡍 (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
بنية thread_info {
    فراغ *stackpos;
    بنية hlist_node node;
};
بنية thread_info MainThread VARFSEG = {
    عدم, { &MainThread.node, &MainThread.node.next }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
ساكن صحيح
have_threads(فراغ)
{
    أرجع (CONFIG_THREADS
            && GET_FLATPTR(MainThread.node.next) != &MainThread.node);
}

// Return the 'struct thread_info' for the currently running thread.
بنية thread_info *
getCurThread(فراغ)
{
    u32 esp = getesp();
    𐡄𐡍 (esp <= MAIN_STACK_MAX)
        أرجع &MainThread;
    أرجع (فراغ*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

ساكن u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
فراغ
thread_setup(فراغ)
{
    CanInterrupt = 1;
    call16_override(1);
    𐡄𐡍 (! CONFIG_THREADS)
        أرجع;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
صحيح
threads_during_optionroms(فراغ)
{
    أرجع CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
ساكن فراغ
switch_next(بنية thread_info *cur)
{
    بنية thread_info *next = container_of(
        cur->node.next, بنية thread_info, node);
    𐡄𐡍 (cur == next)
        // Nothing to do.
        أرجع;
    asm متطاير(
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
ساكن فراغ
__end_thread(بنية thread_info *old)
{
    hlist_del(&old->node);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    حرر(old);
    𐡄𐡍 (!have_threads())
        dprintf(1, "All threads complete.\n");
}

فراغ VISIBLE16 check_irqs(فراغ);

// Create a new thread and start executing 'func' in it.
فراغ
run_thread(فراغ (*func)(فراغ*), فراغ *data)
{
    ASSERT32FLAT();
    𐡄𐡍 (! CONFIG_THREADS || ! ThreadControl)
        اذهب fail;
    بنية thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    𐡄𐡍 (!thread)
        اذهب fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (فراغ*)thread + THREADSTACKSIZE;
    بنية thread_info *cur = getCurThread();
    بنية thread_info *edx = cur;
    hlist_add_after(&thread->node, &cur->node);
    asm متطاير(
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
    𐡄𐡍 (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    أرجع;

fail:
    func(data);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
فراغ VISIBLE16
check_irqs(فراغ)
{
    𐡄𐡍 (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        أرجع;
    }
    𐡄𐡍 (need_hop_back()) {
        stack_hop_back(check_irqs, 0, 0);
        أرجع;
    }
    𐡄𐡍 (MODE16)
        clock_poll_irq();
    asm متطاير("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
فراغ
yield(فراغ)
{
    𐡄𐡍 (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        أرجع;
    }
    بنية thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_next(cur);
    𐡄𐡍 (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

فراغ VISIBLE16
wait_irq(فراغ)
{
    𐡄𐡍 (need_hop_back()) {
        stack_hop_back(wait_irq, 0, 0);
        أرجع;
    }
    asm متطاير("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
فراغ
yield_toirq(فراغ)
{
    𐡄𐡍 (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        أرجع;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
فراغ
wait_threads(فراغ)
{
    ASSERT32FLAT();
    طالما (have_threads())
        yield();
}

فراغ
mutex_lock(بنية mutex_s *mutex)
{
    ASSERT32FLAT();
    𐡄𐡍 (! CONFIG_THREADS)
        أرجع;
    طالما (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

فراغ
mutex_unlock(بنية mutex_s *mutex)
{
    ASSERT32FLAT();
    𐡄𐡍 (! CONFIG_THREADS)
        أرجع;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

صحيح CanPreempt VARFSEG;
ساكن u32 PreemptCount;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
فراغ
start_preempt(فراغ)
{
    𐡄𐡍 (! threads_during_optionroms())
        أرجع;
    CanPreempt = 1;
    PreemptCount = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
فراغ
finish_preempt(فراغ)
{
    𐡄𐡍 (! threads_during_optionroms()) {
        yield();
        أرجع;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", PreemptCount);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
صحيح
wait_preempt(فراغ)
{
    𐡄𐡍 (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_STACK_MAX)
        أرجع 0;
    طالما (CanPreempt)
        yield();
    أرجع 1;
}

// Try to execute 32bit threads.
فراغ VISIBLE32INIT
yield_preempt(فراغ)
{
    PreemptCount++;
    switch_next(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
فراغ
check_preempt(فراغ)
{
    𐡄𐡍 (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

بنية call32_params_s {
    فراغ *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(بنية call32_params_s *params)
{
    أرجع ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(فراغ *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    بنية call32_params_s params = {func, eax, edx, ecx};
    أرجع call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
