// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "bregs.h" // CR0_PE
#समावेश "fw/paravirt.h" // PORT_SMI_CMD
#समावेश "hw/rtc.h" // rtc_use
#समावेश "list.h" // hlist_node
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_loadint
#समावेश "stacks.h" // struct mutex_s
#समावेश "string.h" // memset
#समावेश "util.h" // useRTC

#घोषणा MAIN_ढेर_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

काष्ठा {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    काष्ठा descloc_s gdt;
} Call16_तथ्य VARLOW;

#घोषणा C16_BIG 1
#घोषणा C16_SMM 2

पूर्णांक HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
अटल पूर्णांक
call32_prep(u8 method)
{
    अगर (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        अगर (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            वापस -1;
        SET_निचला(Call16_तथ्य.cr0, cr0);

        // Backup fs/gs and gdt
        SET_निचला(Call16_तथ्य.fs, GET_SEG(FS));
        SET_निचला(Call16_तथ्य.gs, GET_SEG(GS));
        काष्ठा descloc_s gdt;
        sgdt(&gdt);
        SET_निचला(Call16_तथ्य.gdt.लंबाई, gdt.लंबाई);
        SET_निचला(Call16_तथ्य.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_निचला(Call16_तथ्य.a20, set_a20(1));
    }

    // Backup ss
    SET_निचला(Call16_तथ्य.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_सूचकांक);
    अगर (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_सूचकांक);
        inb(PORT_CMOS_तथ्य);
    }
    SET_निचला(Call16_तथ्य.cmosindex, cmosindex);

    SET_निचला(Call16_तथ्य.method, method);
    वापस 0;
}

// Restore state backed up during call32
अटल u8
call32_post(व्योम)
{
    u8 method = GET_निचला(Call16_तथ्य.method);
    SET_निचला(Call16_तथ्य.method, 0);
    SET_निचला(Call16_तथ्य.ss, 0);

    अगर (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_निचला(Call16_तथ्य.a20);
        अगर (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        काष्ठा descloc_s gdt;
        gdt.लंबाई = GET_निचला(Call16_तथ्य.gdt.लंबाई);
        gdt.addr = GET_निचला(Call16_तथ्य.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_निचला(Call16_तथ्य.fs));
        SET_SEG(GS, GET_निचला(Call16_तथ्य.gs));

        // Restore cr0
        u32 cr0_caching = GET_निचला(Call16_तथ्य.cr0) & (CR0_CD|CR0_NW);
        अगर (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_निचला(Call16_तथ्य.cmosindex);
    अगर (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_सूचकांक);
        inb(PORT_CMOS_तथ्य);
    }
    वापस method;
}

// Force next call16() to restore to a pristine cpu environment state
अटल व्योम
call16_override(पूर्णांक big)
{
    ASSERT32FLAT();
    अगर (getesp() > BUILD_ढेर_ADDR)
        panic("call16_override with invalid stack\n");
    स_रखो(&Call16_तथ्य, 0, माप(Call16_तथ्य));
    अगर (big) {
        Call16_तथ्य.method = C16_BIG;
        Call16_तथ्य.a20 = 1;
    } अन्यथा {
        Call16_तथ्य.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 method = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(method);
    वापस ret;
}

#घोषणा ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#घोषणा ASM32_BACK32   "  .popsection\n  .code32\n"
#घोषणा ASM16_SWITCH32 "  .code32\n"
#घोषणा ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
अटल u32
call32_smm(व्योम *func, u32 eax)
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
    वापस eax;
}

अटल u32
call16_smm(u32 eax, u32 edx, व्योम *func)
{
    ASSERT32FLAT();
    अगर (!CONFIG_CALL32_SMM)
        वापस eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16_तथ्य.ss << 4;
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
    वापस eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(व्योम *func, u32 eax, u32 errret)
{
    ASSERT16();
    अगर (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        वापस call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    पूर्णांक ret = call32_prep(C16_BIG);
    अगर (ret)
        वापस errret;
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
    वापस eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
अटल u32
call16(u32 eax, u32 edx, व्योम *func)
{
    ASSERT32FLAT();
    अगर (getesp() > MAIN_ढेर_MAX)
        panic("call16 with invalid stack\n");
    अगर (CONFIG_CALL32_SMM && Call16_तथ्य.method == C16_SMM)
        वापस call16_smm(eax, edx, func);

    बाह्य व्योम transition16big(व्योम);
    बाह्य व्योम transition16(व्योम);
    व्योम *thunk = transition16;
    अगर (Call16_तथ्य.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16_तथ्य.ss;
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
    वापस eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_ढेर[BUILD_EXTRA_ढेर_आकार+1] VARLOW __aligned(8);
u8 *ढेर_Pos VARLOW;

// Test if currently on the extra stack
पूर्णांक
on_extra_ढेर(व्योम)
{
    वापस MODE16 && GET_SEG(SS) == SEG_निचला && getesp() > (u32)Extra_ढेर;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, व्योम *func)
{
    अगर (on_extra_ढेर())
        वापस ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 ढेर_seg = SEG_निचला;
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
        : "m" (ढेर_Pos), "r" (ढेर_seg)
        : "cc", "memory");
    वापस eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, व्योम *func)
{
    अगर (!MODESEGMENT)
        वापस call16(eax, edx, func);
    अगर (!MODE16 || !on_extra_ढेर())
        वापस ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_ढेर_pos, अस्थायी;
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
          , "=&r" (bkup_ढेर_pos), "=&r" (अस्थायी), "+m" (ढेर_Pos)
        :
        : "cc", "memory");
    वापस eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
व्योम VISIBLE16
_farcall16(काष्ठा bregs *callregs, u16 callregseg)
{
    अगर (need_hop_back()) {
        ढेर_hop_back(_farcall16, callregs, callregseg);
        वापस;
    }
    ASSERT16();
    यंत्र अस्थिर(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
व्योम
farcall16(काष्ठा bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
व्योम
farcall16big(काष्ठा bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
व्योम
__call16_int(काष्ठा bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    अगर (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((व्योम*)callregs - Call16_तथ्य.ss * 16, Call16_तथ्य.ss);
        वापस;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
व्योम
reset(व्योम)
{
    बाह्य व्योम reset_vector(व्योम) __noreturn;
    अगर (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
काष्ठा thread_info {
    व्योम *stackpos;
    काष्ठा hlist_गाँठ गाँठ;
};
काष्ठा thread_info MainThread VARFSEG = {
    शून्य, { &MainThread.गाँठ, &MainThread.गाँठ.अग्र }
};
#घोषणा THREADSTACKSIZE 4096

// Check if any threads are running.
अटल पूर्णांक
have_threads(व्योम)
{
    वापस (CONFIG_THREADS
            && GET_FLATPTR(MainThread.गाँठ.अग्र) != &MainThread.गाँठ);
}

// Return the 'struct thread_info' for the currently running thread.
काष्ठा thread_info *
getCurThread(व्योम)
{
    u32 esp = getesp();
    अगर (esp <= MAIN_ढेर_MAX)
        वापस &MainThread;
    वापस (व्योम*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

अटल u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
व्योम
thread_setup(व्योम)
{
    CanInterrupt = 1;
    call16_override(1);
    अगर (! CONFIG_THREADS)
        वापस;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
पूर्णांक
threads_during_optionroms(व्योम)
{
    वापस CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
अटल व्योम
switch_अग्र(काष्ठा thread_info *cur)
{
    काष्ठा thread_info *अग्र = container_of(
        cur->गाँठ.अग्र, काष्ठा thread_info, गाँठ);
    अगर (cur == अग्र)
        // Nothing to do.
        वापस;
    यंत्र अस्थिर(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(अग्र)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
अटल व्योम
__end_thread(काष्ठा thread_info *old)
{
    hlist_del(&old->गाँठ);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    मुक्त(old);
    अगर (!have_threads())
        dprintf(1, "All threads complete.\n");
}

व्योम VISIBLE16 check_irqs(व्योम);

// Create a new thread and start executing 'func' in it.
व्योम
run_thread(व्योम (*func)(व्योम*), व्योम *तथ्य)
{
    ASSERT32FLAT();
    अगर (! CONFIG_THREADS || ! ThreadControl)
        जाओ fail;
    काष्ठा thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    अगर (!thread)
        जाओ fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (व्योम*)thread + THREADSTACKSIZE;
    काष्ठा thread_info *cur = getCurThread();
    काष्ठा thread_info *edx = cur;
    hlist_add_after(&thread->गाँठ, &cur->गाँठ);
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
        : "+a"(तथ्य), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    अगर (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    वापस;

fail:
    func(तथ्य);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
व्योम VISIBLE16
check_irqs(व्योम)
{
    अगर (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        वापस;
    }
    अगर (need_hop_back()) {
        ढेर_hop_back(check_irqs, 0, 0);
        वापस;
    }
    अगर (MODE16)
        clock_poll_irq();
    यंत्र अस्थिर("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
व्योम
yield(व्योम)
{
    अगर (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        वापस;
    }
    काष्ठा thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_अग्र(cur);
    अगर (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

व्योम VISIBLE16
wait_irq(व्योम)
{
    अगर (need_hop_back()) {
        ढेर_hop_back(wait_irq, 0, 0);
        वापस;
    }
    यंत्र अस्थिर("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
व्योम
yield_toirq(व्योम)
{
    अगर (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        वापस;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
व्योम
wait_threads(व्योम)
{
    ASSERT32FLAT();
    जबतक (have_threads())
        yield();
}

व्योम
mutex_lock(काष्ठा mutex_s *mutex)
{
    ASSERT32FLAT();
    अगर (! CONFIG_THREADS)
        वापस;
    जबतक (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

व्योम
mutex_unlock(काष्ठा mutex_s *mutex)
{
    ASSERT32FLAT();
    अगर (! CONFIG_THREADS)
        वापस;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

पूर्णांक CanPreempt VARFSEG;
अटल u32 Preempt_गिनती;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
व्योम
start_preempt(व्योम)
{
    अगर (! threads_during_optionroms())
        वापस;
    CanPreempt = 1;
    Preempt_गिनती = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
व्योम
finish_preempt(व्योम)
{
    अगर (! threads_during_optionroms()) {
        yield();
        वापस;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_गिनती);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
पूर्णांक
wait_preempt(व्योम)
{
    अगर (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_ढेर_MAX)
        वापस 0;
    जबतक (CanPreempt)
        yield();
    वापस 1;
}

// Try to execute 32bit threads.
व्योम VISIBLE32INIT
yield_preempt(व्योम)
{
    Preempt_गिनती++;
    switch_अग्र(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
व्योम
check_preempt(व्योम)
{
    अगर (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

काष्ठा call32_params_s {
    व्योम *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(काष्ठा call32_params_s *params)
{
    वापस ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(व्योम *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    काष्ठा call32_params_s params = {func, eax, edx, ecx};
    वापस call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
