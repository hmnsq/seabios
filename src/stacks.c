// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "bregs.h" // CR0_PE
#ଅନ୍ତର୍ଭାଵ "fw/paravirt.h" // PORT_SMI_CMD
#ଅନ୍ତର୍ଭାଵ "hw/rtc.h" // rtc_use
#ଅନ୍ତର୍ଭାଵ "list.h" // hlist_node
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "stacks.h" // struct mutex_s
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // useRTC

#define MAIN_ଗଦା_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

ସଂରଚନା {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    ସଂରଚନା descloc_s gdt;
} Call16Data VARLOW;

#define C16_BIG 1
#define C16_SMM 2

ପୂର୍ଣ୍ଣସଂଖ୍ୟା HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
call32_prep(u8 method)
{
    ଯଦି (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        ଯଦି (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            ଫେରନ୍ତୁ -1;
        SET_ତଳ(Call16Data.cr0, cr0);

        // Backup fs/gs and gdt
        SET_ତଳ(Call16Data.fs, GET_SEG(FS));
        SET_ତଳ(Call16Data.gs, GET_SEG(GS));
        ସଂରଚନା descloc_s gdt;
        sgdt(&gdt);
        SET_ତଳ(Call16Data.gdt.length, gdt.length);
        SET_ତଳ(Call16Data.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_ତଳ(Call16Data.a20, set_a20(1));
    }

    // Backup ss
    SET_ତଳ(Call16Data.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_INDEX);
    ଯଦି (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    SET_ତଳ(Call16Data.cmosindex, cmosindex);

    SET_ତଳ(Call16Data.method, method);
    ଫେରନ୍ତୁ 0;
}

// Restore state backed up during call32
ସ୍ଥିର u8
call32_post(ଶୂନ୍ୟ)
{
    u8 method = GET_ତଳ(Call16Data.method);
    SET_ତଳ(Call16Data.method, 0);
    SET_ତଳ(Call16Data.ss, 0);

    ଯଦି (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_ତଳ(Call16Data.a20);
        ଯଦି (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        ସଂରଚନା descloc_s gdt;
        gdt.length = GET_ତଳ(Call16Data.gdt.length);
        gdt.addr = GET_ତଳ(Call16Data.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_ତଳ(Call16Data.fs));
        SET_SEG(GS, GET_ତଳ(Call16Data.gs));

        // Restore cr0
        u32 cr0_caching = GET_ତଳ(Call16Data.cr0) & (CR0_CD|CR0_NW);
        ଯଦି (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_ତଳ(Call16Data.cmosindex);
    ଯଦି (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    ଫେରନ୍ତୁ method;
}

// Force next call16() to restore to a pristine cpu environment state
ସ୍ଥିର ଶୂନ୍ୟ
call16_override(ପୂର୍ଣ୍ଣସଂଖ୍ୟା big)
{
    ASSERT32FLAT();
    ଯଦି (getesp() > BUILD_ଗଦା_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16Data, 0, ଆକାର(Call16Data));
    ଯଦି (big) {
        Call16Data.method = C16_BIG;
        Call16Data.a20 = 1;
    } ଅନ୍ୟଥା {
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
    ଫେରନ୍ତୁ ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
ସ୍ଥିର u32
call32_smm(ଶୂନ୍ୟ *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm ଅସ୍ଥିର(
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
    ଫେରନ୍ତୁ eax;
}

ସ୍ଥିର u32
call16_smm(u32 eax, u32 edx, ଶୂନ୍ୟ *func)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_CALL32_SMM)
        ଫେରନ୍ତୁ eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16Data.ss << 4;
    asm ଅସ୍ଥିର(
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
    ଫେରନ୍ତୁ eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(ଶୂନ୍ୟ *func, u32 eax, u32 errret)
{
    ASSERT16();
    ଯଦି (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        ଫେରନ୍ତୁ call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = call32_prep(C16_BIG);
    ଯଦି (ret)
        ଫେରନ୍ତୁ errret;
    u32 bkup_ss, bkup_esp;
    asm ଅସ୍ଥିର(
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
    ଫେରନ୍ତୁ eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
ସ୍ଥିର u32
call16(u32 eax, u32 edx, ଶୂନ୍ୟ *func)
{
    ASSERT32FLAT();
    ଯଦି (getesp() > MAIN_ଗଦା_MAX)
        panic("call16 with invalid stack\n");
    ଯଦି (CONFIG_CALL32_SMM && Call16Data.method == C16_SMM)
        ଫେରନ୍ତୁ call16_smm(eax, edx, func);

    ବାହ୍ୟ ଶୂନ୍ୟ transition16big(ଶୂନ୍ୟ);
    ବାହ୍ୟ ଶୂନ୍ୟ transition16(ଶୂନ୍ୟ);
    ଶୂନ୍ୟ *thunk = transition16;
    ଯଦି (Call16Data.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16Data.ss;
    asm ଅସ୍ଥିର(
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
    ଫେରନ୍ତୁ eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_ଗଦା[BUILD_EXTRA_ଗଦା_SIZE+1] VARLOW __aligned(8);
u8 *ଗଦା_Pos VARLOW;

// Test if currently on the extra stack
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
on_extra_ଗଦା(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ MODE16 && GET_SEG(SS) == SEG_ତଳ && getesp() > (u32)Extra_ଗଦା;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, ଶୂନ୍ୟ *func)
{
    ଯଦି (on_extra_ଗଦା())
        ଫେରନ୍ତୁ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 ଗଦା_seg = SEG_ତଳ;
    u32 bkup_ss, bkup_esp;
    asm ଅସ୍ଥିର(
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
        : "m" (ଗଦା_Pos), "r" (ଗଦା_seg)
        : "cc", "memory");
    ଫେରନ୍ତୁ eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, ଶୂନ୍ୟ *func)
{
    ଯଦି (!MODESEGMENT)
        ଫେରନ୍ତୁ call16(eax, edx, func);
    ଯଦି (!MODE16 || !on_extra_ଗଦା())
        ଫେରନ୍ତୁ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_ଗଦା_pos, temp;
    asm ଅସ୍ଥିର(
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
          , "=&r" (bkup_ଗଦା_pos), "=&r" (temp), "+m" (ଗଦା_Pos)
        :
        : "cc", "memory");
    ଫେରନ୍ତୁ eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
ଶୂନ୍ୟ VISIBLE16
_farcall16(ସଂରଚନା bregs *callregs, u16 callregseg)
{
    ଯଦି (need_hop_back()) {
        ଗଦା_hop_back(_farcall16, callregs, callregseg);
        ଫେରନ୍ତୁ;
    }
    ASSERT16();
    asm ଅସ୍ଥିର(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
ଶୂନ୍ୟ
farcall16(ସଂରଚନା bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
ଶୂନ୍ୟ
farcall16big(ସଂରଚନା bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
ଶୂନ୍ୟ
__call16_int(ସଂରଚନା bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    ଯଦି (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((ଶୂନ୍ୟ*)callregs - Call16Data.ss * 16, Call16Data.ss);
        ଫେରନ୍ତୁ;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
ଶୂନ୍ୟ
reset(ଶୂନ୍ୟ)
{
    ବାହ୍ୟ ଶୂନ୍ୟ reset_vector(ଶୂନ୍ୟ) __noreturn;
    ଯଦି (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
ସଂରଚନା thread_info {
    ଶୂନ୍ୟ *stackpos;
    ସଂରଚନା hlist_ଗଣ୍ଠି ଗଣ୍ଠି;
};
ସଂରଚନା thread_info MainThread VARFSEG = {
    ଖାଲି, { &MainThread.ଗଣ୍ଠି, &MainThread.ଗଣ୍ଠି.next }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
have_threads(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ (CONFIG_THREADS
            && GET_FLATPTR(MainThread.ଗଣ୍ଠି.next) != &MainThread.ଗଣ୍ଠି);
}

// Return the 'struct thread_info' for the currently running thread.
ସଂରଚନା thread_info *
getCurThread(ଶୂନ୍ୟ)
{
    u32 esp = getesp();
    ଯଦି (esp <= MAIN_ଗଦା_MAX)
        ଫେରନ୍ତୁ &MainThread;
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

ସ୍ଥିର u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
ଶୂନ୍ୟ
thread_setup(ଶୂନ୍ୟ)
{
    CanInterrupt = 1;
    call16_override(1);
    ଯଦି (! CONFIG_THREADS)
        ଫେରନ୍ତୁ;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
threads_during_optionroms(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
ସ୍ଥିର ଶୂନ୍ୟ
switch_next(ସଂରଚନା thread_info *cur)
{
    ସଂରଚନା thread_info *next = container_of(
        cur->ଗଣ୍ଠି.next, ସଂରଚନା thread_info, ଗଣ୍ଠି);
    ଯଦି (cur == next)
        // Nothing to do.
        ଫେରନ୍ତୁ;
    asm ଅସ୍ଥିର(
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
ସ୍ଥିର ଶୂନ୍ୟ
__end_thread(ସଂରଚନା thread_info *old)
{
    hlist_del(&old->ଗଣ୍ଠି);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    ମୁକ୍ତ(old);
    ଯଦି (!have_threads())
        dprintf(1, "All threads complete.\n");
}

ଶୂନ୍ୟ VISIBLE16 check_irqs(ଶୂନ୍ୟ);

// Create a new thread and start executing 'func' in it.
ଶୂନ୍ୟ
run_thread(ଶୂନ୍ୟ (*func)(ଶୂନ୍ୟ*), ଶୂନ୍ୟ *data)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_THREADS || ! ThreadControl)
        ଯାଅ fail;
    ସଂରଚନା thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    ଯଦି (!thread)
        ଯାଅ fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (ଶୂନ୍ୟ*)thread + THREADSTACKSIZE;
    ସଂରଚନା thread_info *cur = getCurThread();
    ସଂରଚନା thread_info *edx = cur;
    hlist_add_after(&thread->ଗଣ୍ଠି, &cur->ଗଣ୍ଠି);
    asm ଅସ୍ଥିର(
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
    ଯଦି (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    ଫେରନ୍ତୁ;

fail:
    func(data);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
ଶୂନ୍ୟ VISIBLE16
check_irqs(ଶୂନ୍ୟ)
{
    ଯଦି (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        ଫେରନ୍ତୁ;
    }
    ଯଦି (need_hop_back()) {
        ଗଦା_hop_back(check_irqs, 0, 0);
        ଫେରନ୍ତୁ;
    }
    ଯଦି (MODE16)
        clock_poll_irq();
    asm ଅସ୍ଥିର("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
ଶୂନ୍ୟ
yield(ଶୂନ୍ୟ)
{
    ଯଦି (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        ଫେରନ୍ତୁ;
    }
    ସଂରଚନା thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_next(cur);
    ଯଦି (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

ଶୂନ୍ୟ VISIBLE16
wait_irq(ଶୂନ୍ୟ)
{
    ଯଦି (need_hop_back()) {
        ଗଦା_hop_back(wait_irq, 0, 0);
        ଫେରନ୍ତୁ;
    }
    asm ଅସ୍ଥିର("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
ଶୂନ୍ୟ
yield_toirq(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        ଫେରନ୍ତୁ;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
ଶୂନ୍ୟ
wait_threads(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯେତେବେଳେ (have_threads())
        yield();
}

ଶୂନ୍ୟ
mutex_lock(ସଂରଚନା mutex_s *mutex)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_THREADS)
        ଫେରନ୍ତୁ;
    ଯେତେବେଳେ (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

ଶୂନ୍ୟ
mutex_unlock(ସଂରଚନା mutex_s *mutex)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_THREADS)
        ଫେରନ୍ତୁ;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା CanPreempt VARFSEG;
ସ୍ଥିର u32 Preempt_ଗଣନା;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
ଶୂନ୍ୟ
start_preempt(ଶୂନ୍ୟ)
{
    ଯଦି (! threads_during_optionroms())
        ଫେରନ୍ତୁ;
    CanPreempt = 1;
    Preempt_ଗଣନା = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
ଶୂନ୍ୟ
finish_preempt(ଶୂନ୍ୟ)
{
    ଯଦି (! threads_during_optionroms()) {
        yield();
        ଫେରନ୍ତୁ;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_ଗଣନା);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
wait_preempt(ଶୂନ୍ୟ)
{
    ଯଦି (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_ଗଦା_MAX)
        ଫେରନ୍ତୁ 0;
    ଯେତେବେଳେ (CanPreempt)
        yield();
    ଫେରନ୍ତୁ 1;
}

// Try to execute 32bit threads.
ଶୂନ୍ୟ VISIBLE32INIT
yield_preempt(ଶୂନ୍ୟ)
{
    Preempt_ଗଣନା++;
    switch_next(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
ଶୂନ୍ୟ
check_preempt(ଶୂନ୍ୟ)
{
    ଯଦି (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

ସଂରଚନା call32_params_s {
    ଶୂନ୍ୟ *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(ସଂରଚନା call32_params_s *params)
{
    ଫେରନ୍ତୁ ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(ଶୂନ୍ୟ *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    ସଂରଚନା call32_params_s params = {func, eax, edx, ecx};
    ଫେରନ୍ତୁ call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
