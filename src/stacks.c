// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "bregs.h" // CR0_PE
#അന്തര്ഭാവ "fw/paravirt.h" // PORT_SMI_CMD
#അന്തര്ഭാവ "hw/rtc.h" // rtc_use
#അന്തര്ഭാവ "list.h" // hlist_node
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "stacks.h" // struct mutex_s
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // useRTC

#define MAIN_അട്ടി_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

ഘടന {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    ഘടന descloc_s gdt;
} Call16Data VARLOW;

#define C16_BIG 1
#define C16_SMM 2

പൂർണ്ണസംഖ്യ HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
സ്ഥിതം പൂർണ്ണസംഖ്യ
call32_prep(u8 method)
{
    എങ്കിൽ (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        എങ്കിൽ (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            തിരികെ -1;
        SET_താഴെയുള്ള(Call16Data.cr0, cr0);

        // Backup fs/gs and gdt
        SET_താഴെയുള്ള(Call16Data.fs, GET_SEG(FS));
        SET_താഴെയുള്ള(Call16Data.gs, GET_SEG(GS));
        ഘടന descloc_s gdt;
        sgdt(&gdt);
        SET_താഴെയുള്ള(Call16Data.gdt.length, gdt.length);
        SET_താഴെയുള്ള(Call16Data.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_താഴെയുള്ള(Call16Data.a20, set_a20(1));
    }

    // Backup ss
    SET_താഴെയുള്ള(Call16Data.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_INDEX);
    എങ്കിൽ (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    SET_താഴെയുള്ള(Call16Data.cmosindex, cmosindex);

    SET_താഴെയുള്ള(Call16Data.method, method);
    തിരികെ 0;
}

// Restore state backed up during call32
സ്ഥിതം u8
call32_post(ശൂന്യം)
{
    u8 method = GET_താഴെയുള്ള(Call16Data.method);
    SET_താഴെയുള്ള(Call16Data.method, 0);
    SET_താഴെയുള്ള(Call16Data.ss, 0);

    എങ്കിൽ (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_താഴെയുള്ള(Call16Data.a20);
        എങ്കിൽ (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        ഘടന descloc_s gdt;
        gdt.length = GET_താഴെയുള്ള(Call16Data.gdt.length);
        gdt.addr = GET_താഴെയുള്ള(Call16Data.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_താഴെയുള്ള(Call16Data.fs));
        SET_SEG(GS, GET_താഴെയുള്ള(Call16Data.gs));

        // Restore cr0
        u32 cr0_caching = GET_താഴെയുള്ള(Call16Data.cr0) & (CR0_CD|CR0_NW);
        എങ്കിൽ (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_താഴെയുള്ള(Call16Data.cmosindex);
    എങ്കിൽ (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    തിരികെ method;
}

// Force next call16() to restore to a pristine cpu environment state
സ്ഥിതം ശൂന്യം
call16_override(പൂർണ്ണസംഖ്യ big)
{
    ASSERT32FLAT();
    എങ്കിൽ (getesp() > BUILD_അട്ടി_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16Data, 0, വലുപ്പം(Call16Data));
    എങ്കിൽ (big) {
        Call16Data.method = C16_BIG;
        Call16Data.a20 = 1;
    } അല്ലെങ്കിൽ {
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
    തിരികെ ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
സ്ഥിതം u32
call32_smm(ശൂന്യം *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm അസ്ഥിര(
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
    തിരികെ eax;
}

സ്ഥിതം u32
call16_smm(u32 eax, u32 edx, ശൂന്യം *func)
{
    ASSERT32FLAT();
    എങ്കിൽ (!CONFIG_CALL32_SMM)
        തിരികെ eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16Data.ss << 4;
    asm അസ്ഥിര(
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
    തിരികെ eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(ശൂന്യം *func, u32 eax, u32 errret)
{
    ASSERT16();
    എങ്കിൽ (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        തിരികെ call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    പൂർണ്ണസംഖ്യ ret = call32_prep(C16_BIG);
    എങ്കിൽ (ret)
        തിരികെ errret;
    u32 bkup_ss, bkup_esp;
    asm അസ്ഥിര(
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
    തിരികെ eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
സ്ഥിതം u32
call16(u32 eax, u32 edx, ശൂന്യം *func)
{
    ASSERT32FLAT();
    എങ്കിൽ (getesp() > MAIN_അട്ടി_MAX)
        panic("call16 with invalid stack\n");
    എങ്കിൽ (CONFIG_CALL32_SMM && Call16Data.method == C16_SMM)
        തിരികെ call16_smm(eax, edx, func);

    ബാഹ്യ ശൂന്യം transition16big(ശൂന്യം);
    ബാഹ്യ ശൂന്യം transition16(ശൂന്യം);
    ശൂന്യം *thunk = transition16;
    എങ്കിൽ (Call16Data.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16Data.ss;
    asm അസ്ഥിര(
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
    തിരികെ eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_അട്ടി[BUILD_EXTRA_അട്ടി_SIZE+1] VARLOW __aligned(8);
u8 *അട്ടി_Pos VARLOW;

// Test if currently on the extra stack
പൂർണ്ണസംഖ്യ
on_extra_അട്ടി(ശൂന്യം)
{
    തിരികെ MODE16 && GET_SEG(SS) == SEG_താഴെയുള്ള && getesp() > (u32)Extra_അട്ടി;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, ശൂന്യം *func)
{
    എങ്കിൽ (on_extra_അട്ടി())
        തിരികെ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 അട്ടി_seg = SEG_താഴെയുള്ള;
    u32 bkup_ss, bkup_esp;
    asm അസ്ഥിര(
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
        : "m" (അട്ടി_Pos), "r" (അട്ടി_seg)
        : "cc", "memory");
    തിരികെ eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, ശൂന്യം *func)
{
    എങ്കിൽ (!MODESEGMENT)
        തിരികെ call16(eax, edx, func);
    എങ്കിൽ (!MODE16 || !on_extra_അട്ടി())
        തിരികെ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_അട്ടി_pos, temp;
    asm അസ്ഥിര(
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
          , "=&r" (bkup_അട്ടി_pos), "=&r" (temp), "+m" (അട്ടി_Pos)
        :
        : "cc", "memory");
    തിരികെ eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
ശൂന്യം VISIBLE16
_farcall16(ഘടന bregs *callregs, u16 callregseg)
{
    എങ്കിൽ (need_hop_back()) {
        അട്ടി_hop_back(_farcall16, callregs, callregseg);
        തിരികെ;
    }
    ASSERT16();
    asm അസ്ഥിര(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
ശൂന്യം
farcall16(ഘടന bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
ശൂന്യം
farcall16big(ഘടന bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
ശൂന്യം
__call16_int(ഘടന bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    എങ്കിൽ (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((ശൂന്യം*)callregs - Call16Data.ss * 16, Call16Data.ss);
        തിരികെ;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
ശൂന്യം
reset(ശൂന്യം)
{
    ബാഹ്യ ശൂന്യം reset_vector(ശൂന്യം) __noreturn;
    എങ്കിൽ (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
ഘടന thread_info {
    ശൂന്യം *stackpos;
    ഘടന hlist_കണ്ണി കണ്ണി;
};
ഘടന thread_info MainThread VARFSEG = {
    ശൂന്യ, { &MainThread.കണ്ണി, &MainThread.കണ്ണി.next }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
സ്ഥിതം പൂർണ്ണസംഖ്യ
have_threads(ശൂന്യം)
{
    തിരികെ (CONFIG_THREADS
            && GET_FLATPTR(MainThread.കണ്ണി.next) != &MainThread.കണ്ണി);
}

// Return the 'struct thread_info' for the currently running thread.
ഘടന thread_info *
getCurThread(ശൂന്യം)
{
    u32 esp = getesp();
    എങ്കിൽ (esp <= MAIN_അട്ടി_MAX)
        തിരികെ &MainThread;
    തിരികെ (ശൂന്യം*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

സ്ഥിതം u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
ശൂന്യം
thread_setup(ശൂന്യം)
{
    CanInterrupt = 1;
    call16_override(1);
    എങ്കിൽ (! CONFIG_THREADS)
        തിരികെ;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
പൂർണ്ണസംഖ്യ
threads_during_optionroms(ശൂന്യം)
{
    തിരികെ CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
സ്ഥിതം ശൂന്യം
switch_next(ഘടന thread_info *cur)
{
    ഘടന thread_info *next = container_of(
        cur->കണ്ണി.next, ഘടന thread_info, കണ്ണി);
    എങ്കിൽ (cur == next)
        // Nothing to do.
        തിരികെ;
    asm അസ്ഥിര(
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
സ്ഥിതം ശൂന്യം
__end_thread(ഘടന thread_info *old)
{
    hlist_del(&old->കണ്ണി);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    സ്വതന്ത്രം(old);
    എങ്കിൽ (!have_threads())
        dprintf(1, "All threads complete.\n");
}

ശൂന്യം VISIBLE16 check_irqs(ശൂന്യം);

// Create a new thread and start executing 'func' in it.
ശൂന്യം
run_thread(ശൂന്യം (*func)(ശൂന്യം*), ശൂന്യം *data)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_THREADS || ! ThreadControl)
        പോകുക fail;
    ഘടന thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    എങ്കിൽ (!thread)
        പോകുക fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (ശൂന്യം*)thread + THREADSTACKSIZE;
    ഘടന thread_info *cur = getCurThread();
    ഘടന thread_info *edx = cur;
    hlist_add_after(&thread->കണ്ണി, &cur->കണ്ണി);
    asm അസ്ഥിര(
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
    എങ്കിൽ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    തിരികെ;

fail:
    func(data);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
ശൂന്യം VISIBLE16
check_irqs(ശൂന്യം)
{
    എങ്കിൽ (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        തിരികെ;
    }
    എങ്കിൽ (need_hop_back()) {
        അട്ടി_hop_back(check_irqs, 0, 0);
        തിരികെ;
    }
    എങ്കിൽ (MODE16)
        clock_poll_irq();
    asm അസ്ഥിര("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
ശൂന്യം
yield(ശൂന്യം)
{
    എങ്കിൽ (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        തിരികെ;
    }
    ഘടന thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_next(cur);
    എങ്കിൽ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

ശൂന്യം VISIBLE16
wait_irq(ശൂന്യം)
{
    എങ്കിൽ (need_hop_back()) {
        അട്ടി_hop_back(wait_irq, 0, 0);
        തിരികെ;
    }
    asm അസ്ഥിര("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
ശൂന്യം
yield_toirq(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        തിരികെ;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
ശൂന്യം
wait_threads(ശൂന്യം)
{
    ASSERT32FLAT();
    ആകുമ്പോൾ (have_threads())
        yield();
}

ശൂന്യം
mutex_lock(ഘടന mutex_s *mutex)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_THREADS)
        തിരികെ;
    ആകുമ്പോൾ (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

ശൂന്യം
mutex_unlock(ഘടന mutex_s *mutex)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_THREADS)
        തിരികെ;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

പൂർണ്ണസംഖ്യ CanPreempt VARFSEG;
സ്ഥിതം u32 Preempt_എണ്ണം;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
ശൂന്യം
start_preempt(ശൂന്യം)
{
    എങ്കിൽ (! threads_during_optionroms())
        തിരികെ;
    CanPreempt = 1;
    Preempt_എണ്ണം = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
ശൂന്യം
finish_preempt(ശൂന്യം)
{
    എങ്കിൽ (! threads_during_optionroms()) {
        yield();
        തിരികെ;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_എണ്ണം);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
പൂർണ്ണസംഖ്യ
wait_preempt(ശൂന്യം)
{
    എങ്കിൽ (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_അട്ടി_MAX)
        തിരികെ 0;
    ആകുമ്പോൾ (CanPreempt)
        yield();
    തിരികെ 1;
}

// Try to execute 32bit threads.
ശൂന്യം VISIBLE32INIT
yield_preempt(ശൂന്യം)
{
    Preempt_എണ്ണം++;
    switch_next(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
ശൂന്യം
check_preempt(ശൂന്യം)
{
    എങ്കിൽ (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

ഘടന call32_params_s {
    ശൂന്യം *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(ഘടന call32_params_s *params)
{
    തിരികെ ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(ശൂന്യം *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    ഘടന call32_params_s params = {func, eax, edx, ecx};
    തിരികെ call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
