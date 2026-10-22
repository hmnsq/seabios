// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "bregs.h" // CR0_PE
#அந்தர்பாவ "fw/paravirt.h" // PORT_SMI_CMD
#அந்தர்பாவ "hw/rtc.h" // rtc_use
#அந்தர்பாவ "list.h" // hlist_node
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "stacks.h" // struct mutex_s
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // useRTC

#define MAIN_அடுக்கு_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

கட்டமைப்பு {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    கட்டமைப்பு descloc_s gdt;
} Call16Data VARLOW;

#define C16_BIG 1
#define C16_SMM 2

முழு HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
நிலையான முழு
call32_prep(u8 method)
{
    எனில் (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        எனில் (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            திரும்பு -1;
        SET_கீழ்(Call16Data.cr0, cr0);

        // Backup fs/gs and gdt
        SET_கீழ்(Call16Data.fs, GET_SEG(FS));
        SET_கீழ்(Call16Data.gs, GET_SEG(GS));
        கட்டமைப்பு descloc_s gdt;
        sgdt(&gdt);
        SET_கீழ்(Call16Data.gdt.length, gdt.length);
        SET_கீழ்(Call16Data.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_கீழ்(Call16Data.a20, set_a20(1));
    }

    // Backup ss
    SET_கீழ்(Call16Data.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_INDEX);
    எனில் (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    SET_கீழ்(Call16Data.cmosindex, cmosindex);

    SET_கீழ்(Call16Data.method, method);
    திரும்பு 0;
}

// Restore state backed up during call32
நிலையான u8
call32_post(வெறுமை)
{
    u8 method = GET_கீழ்(Call16Data.method);
    SET_கீழ்(Call16Data.method, 0);
    SET_கீழ்(Call16Data.ss, 0);

    எனில் (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_கீழ்(Call16Data.a20);
        எனில் (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        கட்டமைப்பு descloc_s gdt;
        gdt.length = GET_கீழ்(Call16Data.gdt.length);
        gdt.addr = GET_கீழ்(Call16Data.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_கீழ்(Call16Data.fs));
        SET_SEG(GS, GET_கீழ்(Call16Data.gs));

        // Restore cr0
        u32 cr0_caching = GET_கீழ்(Call16Data.cr0) & (CR0_CD|CR0_NW);
        எனில் (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_கீழ்(Call16Data.cmosindex);
    எனில் (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    திரும்பு method;
}

// Force next call16() to restore to a pristine cpu environment state
நிலையான வெறுமை
call16_override(முழு big)
{
    ASSERT32FLAT();
    எனில் (getesp() > BUILD_அடுக்கு_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16Data, 0, அளவு(Call16Data));
    எனில் (big) {
        Call16Data.method = C16_BIG;
        Call16Data.a20 = 1;
    } இல்லையெனில் {
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
    திரும்பு ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
நிலையான u32
call32_smm(வெறுமை *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm நிலையற்ற(
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
    திரும்பு eax;
}

நிலையான u32
call16_smm(u32 eax, u32 edx, வெறுமை *func)
{
    ASSERT32FLAT();
    எனில் (!CONFIG_CALL32_SMM)
        திரும்பு eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16Data.ss << 4;
    asm நிலையற்ற(
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
    திரும்பு eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(வெறுமை *func, u32 eax, u32 errret)
{
    ASSERT16();
    எனில் (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        திரும்பு call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    முழு ret = call32_prep(C16_BIG);
    எனில் (ret)
        திரும்பு errret;
    u32 bkup_ss, bkup_esp;
    asm நிலையற்ற(
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
    திரும்பு eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
நிலையான u32
call16(u32 eax, u32 edx, வெறுமை *func)
{
    ASSERT32FLAT();
    எனில் (getesp() > MAIN_அடுக்கு_MAX)
        panic("call16 with invalid stack\n");
    எனில் (CONFIG_CALL32_SMM && Call16Data.method == C16_SMM)
        திரும்பு call16_smm(eax, edx, func);

    வெளி வெறுமை transition16big(வெறுமை);
    வெளி வெறுமை transition16(வெறுமை);
    வெறுமை *thunk = transition16;
    எனில் (Call16Data.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16Data.ss;
    asm நிலையற்ற(
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
    திரும்பு eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_அடுக்கு[BUILD_EXTRA_அடுக்கு_SIZE+1] VARLOW __aligned(8);
u8 *அடுக்கு_Pos VARLOW;

// Test if currently on the extra stack
முழு
on_extra_அடுக்கு(வெறுமை)
{
    திரும்பு MODE16 && GET_SEG(SS) == SEG_கீழ் && getesp() > (u32)Extra_அடுக்கு;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, வெறுமை *func)
{
    எனில் (on_extra_அடுக்கு())
        திரும்பு ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 அடுக்கு_seg = SEG_கீழ்;
    u32 bkup_ss, bkup_esp;
    asm நிலையற்ற(
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
        : "m" (அடுக்கு_Pos), "r" (அடுக்கு_seg)
        : "cc", "memory");
    திரும்பு eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, வெறுமை *func)
{
    எனில் (!MODESEGMENT)
        திரும்பு call16(eax, edx, func);
    எனில் (!MODE16 || !on_extra_அடுக்கு())
        திரும்பு ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_அடுக்கு_pos, temp;
    asm நிலையற்ற(
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
          , "=&r" (bkup_அடுக்கு_pos), "=&r" (temp), "+m" (அடுக்கு_Pos)
        :
        : "cc", "memory");
    திரும்பு eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
வெறுமை VISIBLE16
_farcall16(கட்டமைப்பு bregs *callregs, u16 callregseg)
{
    எனில் (need_hop_back()) {
        அடுக்கு_hop_back(_farcall16, callregs, callregseg);
        திரும்பு;
    }
    ASSERT16();
    asm நிலையற்ற(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
வெறுமை
farcall16(கட்டமைப்பு bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
வெறுமை
farcall16big(கட்டமைப்பு bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
வெறுமை
__call16_int(கட்டமைப்பு bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    எனில் (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((வெறுமை*)callregs - Call16Data.ss * 16, Call16Data.ss);
        திரும்பு;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
வெறுமை
reset(வெறுமை)
{
    வெளி வெறுமை reset_vector(வெறுமை) __noreturn;
    எனில் (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
கட்டமைப்பு thread_info {
    வெறுமை *stackpos;
    கட்டமைப்பு hlist_முனை முனை;
};
கட்டமைப்பு thread_info MainThread VARFSEG = {
    வெற்று, { &MainThread.முனை, &MainThread.முனை.next }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
நிலையான முழு
have_threads(வெறுமை)
{
    திரும்பு (CONFIG_THREADS
            && GET_FLATPTR(MainThread.முனை.next) != &MainThread.முனை);
}

// Return the 'struct thread_info' for the currently running thread.
கட்டமைப்பு thread_info *
getCurThread(வெறுமை)
{
    u32 esp = getesp();
    எனில் (esp <= MAIN_அடுக்கு_MAX)
        திரும்பு &MainThread;
    திரும்பு (வெறுமை*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

நிலையான u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
வெறுமை
thread_setup(வெறுமை)
{
    CanInterrupt = 1;
    call16_override(1);
    எனில் (! CONFIG_THREADS)
        திரும்பு;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
முழு
threads_during_optionroms(வெறுமை)
{
    திரும்பு CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
நிலையான வெறுமை
switch_next(கட்டமைப்பு thread_info *cur)
{
    கட்டமைப்பு thread_info *next = container_of(
        cur->முனை.next, கட்டமைப்பு thread_info, முனை);
    எனில் (cur == next)
        // Nothing to do.
        திரும்பு;
    asm நிலையற்ற(
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
நிலையான வெறுமை
__end_thread(கட்டமைப்பு thread_info *old)
{
    hlist_del(&old->முனை);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    விடுவி(old);
    எனில் (!have_threads())
        dprintf(1, "All threads complete.\n");
}

வெறுமை VISIBLE16 check_irqs(வெறுமை);

// Create a new thread and start executing 'func' in it.
வெறுமை
run_thread(வெறுமை (*func)(வெறுமை*), வெறுமை *data)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_THREADS || ! ThreadControl)
        செல் fail;
    கட்டமைப்பு thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    எனில் (!thread)
        செல் fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (வெறுமை*)thread + THREADSTACKSIZE;
    கட்டமைப்பு thread_info *cur = getCurThread();
    கட்டமைப்பு thread_info *edx = cur;
    hlist_add_after(&thread->முனை, &cur->முனை);
    asm நிலையற்ற(
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
    எனில் (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    திரும்பு;

fail:
    func(data);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
வெறுமை VISIBLE16
check_irqs(வெறுமை)
{
    எனில் (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        திரும்பு;
    }
    எனில் (need_hop_back()) {
        அடுக்கு_hop_back(check_irqs, 0, 0);
        திரும்பு;
    }
    எனில் (MODE16)
        clock_poll_irq();
    asm நிலையற்ற("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
வெறுமை
yield(வெறுமை)
{
    எனில் (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        திரும்பு;
    }
    கட்டமைப்பு thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_next(cur);
    எனில் (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

வெறுமை VISIBLE16
wait_irq(வெறுமை)
{
    எனில் (need_hop_back()) {
        அடுக்கு_hop_back(wait_irq, 0, 0);
        திரும்பு;
    }
    asm நிலையற்ற("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
வெறுமை
yield_toirq(வெறுமை)
{
    எனில் (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        திரும்பு;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
வெறுமை
wait_threads(வெறுமை)
{
    ASSERT32FLAT();
    போது (have_threads())
        yield();
}

வெறுமை
mutex_lock(கட்டமைப்பு mutex_s *mutex)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_THREADS)
        திரும்பு;
    போது (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

வெறுமை
mutex_unlock(கட்டமைப்பு mutex_s *mutex)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_THREADS)
        திரும்பு;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

முழு CanPreempt VARFSEG;
நிலையான u32 Preempt_மொத்தம்;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
வெறுமை
start_preempt(வெறுமை)
{
    எனில் (! threads_during_optionroms())
        திரும்பு;
    CanPreempt = 1;
    Preempt_மொத்தம் = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
வெறுமை
finish_preempt(வெறுமை)
{
    எனில் (! threads_during_optionroms()) {
        yield();
        திரும்பு;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_மொத்தம்);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
முழு
wait_preempt(வெறுமை)
{
    எனில் (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_அடுக்கு_MAX)
        திரும்பு 0;
    போது (CanPreempt)
        yield();
    திரும்பு 1;
}

// Try to execute 32bit threads.
வெறுமை VISIBLE32INIT
yield_preempt(வெறுமை)
{
    Preempt_மொத்தம்++;
    switch_next(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
வெறுமை
check_preempt(வெறுமை)
{
    எனில் (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

கட்டமைப்பு call32_params_s {
    வெறுமை *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(கட்டமைப்பு call32_params_s *params)
{
    திரும்பு ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(வெறுமை *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    கட்டமைப்பு call32_params_s params = {func, eax, edx, ecx};
    திரும்பு call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
