// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "bregs.h" // CR0_PE
#अन्तर्भाव "fw/paravirt.h" // PORT_SMI_CMD
#अन्तर्भाव "hw/rtc.h" // rtc_use
#अन्तर्भाव "list.h" // hlist_node
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "stacks.h" // struct mutex_s
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // useRTC

#define MAIN_चळत_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

संरचना {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    संरचना descloc_s gdt;
} Call16Data VARLOW;

#define C16_BIG 1
#define C16_SMM 2

पूर्णांक HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
स्थिर पूर्णांक
call32_prep(u8 method)
{
    जर (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        जर (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            परत -1;
        SET_खालचा(Call16Data.cr0, cr0);

        // Backup fs/gs and gdt
        SET_खालचा(Call16Data.fs, GET_SEG(FS));
        SET_खालचा(Call16Data.gs, GET_SEG(GS));
        संरचना descloc_s gdt;
        sgdt(&gdt);
        SET_खालचा(Call16Data.gdt.length, gdt.length);
        SET_खालचा(Call16Data.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_खालचा(Call16Data.a20, set_a20(1));
    }

    // Backup ss
    SET_खालचा(Call16Data.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_INDEX);
    जर (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    SET_खालचा(Call16Data.cmosindex, cmosindex);

    SET_खालचा(Call16Data.method, method);
    परत 0;
}

// Restore state backed up during call32
स्थिर u8
call32_post(शून्य)
{
    u8 method = GET_खालचा(Call16Data.method);
    SET_खालचा(Call16Data.method, 0);
    SET_खालचा(Call16Data.ss, 0);

    जर (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_खालचा(Call16Data.a20);
        जर (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        संरचना descloc_s gdt;
        gdt.length = GET_खालचा(Call16Data.gdt.length);
        gdt.addr = GET_खालचा(Call16Data.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_खालचा(Call16Data.fs));
        SET_SEG(GS, GET_खालचा(Call16Data.gs));

        // Restore cr0
        u32 cr0_caching = GET_खालचा(Call16Data.cr0) & (CR0_CD|CR0_NW);
        जर (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_खालचा(Call16Data.cmosindex);
    जर (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    परत method;
}

// Force next call16() to restore to a pristine cpu environment state
स्थिर शून्य
call16_override(पूर्णांक big)
{
    ASSERT32FLAT();
    जर (getesp() > BUILD_चळत_ADDR)
        panic("call16_override with invalid stack\n");
    स_रखो(&Call16Data, 0, आकार(Call16Data));
    जर (big) {
        Call16Data.method = C16_BIG;
        Call16Data.a20 = 1;
    } अन्यथा {
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
    परत ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
स्थिर u32
call32_smm(शून्य *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    यंत्र अस्थिर(
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
    परत eax;
}

स्थिर u32
call16_smm(u32 eax, u32 edx, शून्य *func)
{
    ASSERT32FLAT();
    जर (!CONFIG_CALL32_SMM)
        परत eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16Data.ss << 4;
    यंत्र अस्थिर(
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
    परत eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(शून्य *func, u32 eax, u32 errret)
{
    ASSERT16();
    जर (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        परत call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    पूर्णांक ret = call32_prep(C16_BIG);
    जर (ret)
        परत errret;
    u32 bkup_ss, bkup_esp;
    यंत्र अस्थिर(
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
    परत eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
स्थिर u32
call16(u32 eax, u32 edx, शून्य *func)
{
    ASSERT32FLAT();
    जर (getesp() > MAIN_चळत_MAX)
        panic("call16 with invalid stack\n");
    जर (CONFIG_CALL32_SMM && Call16Data.method == C16_SMM)
        परत call16_smm(eax, edx, func);

    बाह्य शून्य transition16big(शून्य);
    बाह्य शून्य transition16(शून्य);
    शून्य *thunk = transition16;
    जर (Call16Data.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16Data.ss;
    यंत्र अस्थिर(
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
    परत eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_चळत[BUILD_EXTRA_चळत_SIZE+1] VARLOW __aligned(8);
u8 *चळत_Pos VARLOW;

// Test if currently on the extra stack
पूर्णांक
on_extra_चळत(शून्य)
{
    परत MODE16 && GET_SEG(SS) == SEG_खालचा && getesp() > (u32)Extra_चळत;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, शून्य *func)
{
    जर (on_extra_चळत())
        परत ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 चळत_seg = SEG_खालचा;
    u32 bkup_ss, bkup_esp;
    यंत्र अस्थिर(
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
        : "m" (चळत_Pos), "r" (चळत_seg)
        : "cc", "memory");
    परत eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, शून्य *func)
{
    जर (!MODESEGMENT)
        परत call16(eax, edx, func);
    जर (!MODE16 || !on_extra_चळत())
        परत ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_चळत_pos, temp;
    यंत्र अस्थिर(
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
          , "=&r" (bkup_चळत_pos), "=&r" (temp), "+m" (चळत_Pos)
        :
        : "cc", "memory");
    परत eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
शून्य VISIBLE16
_farcall16(संरचना bregs *callregs, u16 callregseg)
{
    जर (need_hop_back()) {
        चळत_hop_back(_farcall16, callregs, callregseg);
        परत;
    }
    ASSERT16();
    यंत्र अस्थिर(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
शून्य
farcall16(संरचना bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
शून्य
farcall16big(संरचना bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
शून्य
__call16_int(संरचना bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    जर (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((शून्य*)callregs - Call16Data.ss * 16, Call16Data.ss);
        परत;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
शून्य
reset(शून्य)
{
    बाह्य शून्य reset_vector(शून्य) __noreturn;
    जर (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
संरचना thread_info {
    शून्य *stackpos;
    संरचना hlist_गाठ गाठ;
};
संरचना thread_info MainThread VARFSEG = {
    रिक्त, { &MainThread.गाठ, &MainThread.गाठ.next }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
स्थिर पूर्णांक
have_threads(शून्य)
{
    परत (CONFIG_THREADS
            && GET_FLATPTR(MainThread.गाठ.next) != &MainThread.गाठ);
}

// Return the 'struct thread_info' for the currently running thread.
संरचना thread_info *
getCurThread(शून्य)
{
    u32 esp = getesp();
    जर (esp <= MAIN_चळत_MAX)
        परत &MainThread;
    परत (शून्य*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

स्थिर u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
शून्य
thread_setup(शून्य)
{
    CanInterrupt = 1;
    call16_override(1);
    जर (! CONFIG_THREADS)
        परत;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
पूर्णांक
threads_during_optionroms(शून्य)
{
    परत CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
स्थिर शून्य
switch_next(संरचना thread_info *cur)
{
    संरचना thread_info *next = container_of(
        cur->गाठ.next, संरचना thread_info, गाठ);
    जर (cur == next)
        // Nothing to do.
        परत;
    यंत्र अस्थिर(
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
स्थिर शून्य
__end_thread(संरचना thread_info *old)
{
    hlist_del(&old->गाठ);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    मुक्त(old);
    जर (!have_threads())
        dprintf(1, "All threads complete.\n");
}

शून्य VISIBLE16 check_irqs(शून्य);

// Create a new thread and start executing 'func' in it.
शून्य
run_thread(शून्य (*func)(शून्य*), शून्य *data)
{
    ASSERT32FLAT();
    जर (! CONFIG_THREADS || ! ThreadControl)
        जा fail;
    संरचना thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    जर (!thread)
        जा fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (शून्य*)thread + THREADSTACKSIZE;
    संरचना thread_info *cur = getCurThread();
    संरचना thread_info *edx = cur;
    hlist_add_after(&thread->गाठ, &cur->गाठ);
    यंत्र अस्थिर(
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
    जर (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    परत;

fail:
    func(data);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
शून्य VISIBLE16
check_irqs(शून्य)
{
    जर (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        परत;
    }
    जर (need_hop_back()) {
        चळत_hop_back(check_irqs, 0, 0);
        परत;
    }
    जर (MODE16)
        clock_poll_irq();
    यंत्र अस्थिर("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
शून्य
yield(शून्य)
{
    जर (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        परत;
    }
    संरचना thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_next(cur);
    जर (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

शून्य VISIBLE16
wait_irq(शून्य)
{
    जर (need_hop_back()) {
        चळत_hop_back(wait_irq, 0, 0);
        परत;
    }
    यंत्र अस्थिर("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
शून्य
yield_toirq(शून्य)
{
    जर (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        परत;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
शून्य
wait_threads(शून्य)
{
    ASSERT32FLAT();
    पर्यंत (have_threads())
        yield();
}

शून्य
mutex_lock(संरचना mutex_s *mutex)
{
    ASSERT32FLAT();
    जर (! CONFIG_THREADS)
        परत;
    पर्यंत (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

शून्य
mutex_unlock(संरचना mutex_s *mutex)
{
    ASSERT32FLAT();
    जर (! CONFIG_THREADS)
        परत;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

पूर्णांक CanPreempt VARFSEG;
स्थिर u32 Preempt_मोजणी;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
शून्य
start_preempt(शून्य)
{
    जर (! threads_during_optionroms())
        परत;
    CanPreempt = 1;
    Preempt_मोजणी = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
शून्य
finish_preempt(शून्य)
{
    जर (! threads_during_optionroms()) {
        yield();
        परत;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_मोजणी);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
पूर्णांक
wait_preempt(शून्य)
{
    जर (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_चळत_MAX)
        परत 0;
    पर्यंत (CanPreempt)
        yield();
    परत 1;
}

// Try to execute 32bit threads.
शून्य VISIBLE32INIT
yield_preempt(शून्य)
{
    Preempt_मोजणी++;
    switch_next(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
शून्य
check_preempt(शून्य)
{
    जर (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

संरचना call32_params_s {
    शून्य *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(संरचना call32_params_s *params)
{
    परत ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(शून्य *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    संरचना call32_params_s params = {func, eax, edx, ecx};
    परत call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
