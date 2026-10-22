// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "bregs.h" // CR0_PE
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // PORT_SMI_CMD
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // rtc_use
#ಅನ್ತರ್ಭಾವ "list.h" // hlist_node
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "stacks.h" // struct mutex_s
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // useRTC

#define MAIN_ರಾಶಿ_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

ಸಂರಚನಾ {
    u8 method;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    ಸಂರಚನಾ descloc_s gdt;
} Call16Data VARLOW;

#define C16_BIG 1
#define C16_SMM 2

ಪೂರ್ಣಾಙ್ಕ HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
call32_prep(u8 method)
{
    ಯದಿ (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        ಯದಿ (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            ಪ್ರತ್ಯಾಗಮ -1;
        SET_ಕೆಳಗಿನ(Call16Data.cr0, cr0);

        // Backup fs/gs and gdt
        SET_ಕೆಳಗಿನ(Call16Data.fs, GET_SEG(FS));
        SET_ಕೆಳಗಿನ(Call16Data.gs, GET_SEG(GS));
        ಸಂರಚನಾ descloc_s gdt;
        sgdt(&gdt);
        SET_ಕೆಳಗಿನ(Call16Data.gdt.length, gdt.length);
        SET_ಕೆಳಗಿನ(Call16Data.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        SET_ಕೆಳಗಿನ(Call16Data.a20, set_a20(1));
    }

    // Backup ss
    SET_ಕೆಳಗಿನ(Call16Data.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(PORT_CMOS_INDEX);
    ಯದಿ (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    SET_ಕೆಳಗಿನ(Call16Data.cmosindex, cmosindex);

    SET_ಕೆಳಗಿನ(Call16Data.method, method);
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Restore state backed up during call32
ಸ್ಥಿರ u8
call32_post(ಶೂನ್ಯ)
{
    u8 method = GET_ಕೆಳಗಿನ(Call16Data.method);
    SET_ಕೆಳಗಿನ(Call16Data.method, 0);
    SET_ಕೆಳಗಿನ(Call16Data.ss, 0);

    ಯದಿ (!CONFIG_CALL32_SMM || method != C16_SMM) {
        // Restore a20
        u8 a20 = GET_ಕೆಳಗಿನ(Call16Data.a20);
        ಯದಿ (!a20)
            set_a20(0);

        // Restore gdt and fs/gs
        ಸಂರಚನಾ descloc_s gdt;
        gdt.length = GET_ಕೆಳಗಿನ(Call16Data.gdt.length);
        gdt.addr = GET_ಕೆಳಗಿನ(Call16Data.gdt.addr);
        lgdt(&gdt);
        SET_SEG(FS, GET_ಕೆಳಗಿನ(Call16Data.fs));
        SET_SEG(GS, GET_ಕೆಳಗಿನ(Call16Data.gs));

        // Restore cr0
        u32 cr0_caching = GET_ಕೆಳಗಿನ(Call16Data.cr0) & (CR0_CD|CR0_NW);
        ಯದಿ (cr0_caching)
            cr0_mask(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_ಕೆಳಗಿನ(Call16Data.cmosindex);
    ಯದಿ (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, PORT_CMOS_INDEX);
        inb(PORT_CMOS_DATA);
    }
    ಪ್ರತ್ಯಾಗಮ method;
}

// Force next call16() to restore to a pristine cpu environment state
ಸ್ಥಿರ ಶೂನ್ಯ
call16_override(ಪೂರ್ಣಾಙ್ಕ big)
{
    ASSERT32FLAT();
    ಯದಿ (getesp() > BUILD_ರಾಶಿ_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16Data, 0, ಪರಿಮಾಣ(Call16Data));
    ಯದಿ (big) {
        Call16Data.method = C16_BIG;
        Call16Data.a20 = 1;
    } ಅನ್ಯಥಾ {
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
    ಪ್ರತ್ಯಾಗಮ ret;
}

#define ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#define ASM32_BACK32   "  .popsection\n  .code32\n"
#define ASM16_SWITCH32 "  .code32\n"
#define ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
ಸ್ಥಿರ u32
call32_smm(ಶೂನ್ಯ *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm ಚಞ್ಚಲ(
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
    ಪ್ರತ್ಯಾಗಮ eax;
}

ಸ್ಥಿರ u32
call16_smm(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ASSERT32FLAT();
    ಯದಿ (!CONFIG_CALL32_SMM)
        ಪ್ರತ್ಯಾಗಮ eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16Data.ss << 4;
    asm ಚಞ್ಚಲ(
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
    ಪ್ರತ್ಯಾಗಮ eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(ಶೂನ್ಯ *func, u32 eax, u32 errret)
{
    ASSERT16();
    ಯದಿ (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        ಪ್ರತ್ಯಾಗಮ call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    ಪೂರ್ಣಾಙ್ಕ ret = call32_prep(C16_BIG);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ errret;
    u32 bkup_ss, bkup_esp;
    asm ಚಞ್ಚಲ(
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
    ಪ್ರತ್ಯಾಗಮ eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
ಸ್ಥಿರ u32
call16(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ASSERT32FLAT();
    ಯದಿ (getesp() > MAIN_ರಾಶಿ_MAX)
        panic("call16 with invalid stack\n");
    ಯದಿ (CONFIG_CALL32_SMM && Call16Data.method == C16_SMM)
        ಪ್ರತ್ಯಾಗಮ call16_smm(eax, edx, func);

    ಬಾಹ್ಯ ಶೂನ್ಯ transition16big(ಶೂನ್ಯ);
    ಬಾಹ್ಯ ಶೂನ್ಯ transition16(ಶೂನ್ಯ);
    ಶೂನ್ಯ *thunk = transition16;
    ಯದಿ (Call16Data.method == C16_BIG || in_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16Data.ss;
    asm ಚಞ್ಚಲ(
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
    ಪ್ರತ್ಯಾಗಮ eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_ರಾಶಿ[BUILD_EXTRA_ರಾಶಿ_SIZE+1] VARLOW __aligned(8);
u8 *ರಾಶಿ_Pos VARLOW;

// Test if currently on the extra stack
ಪೂರ್ಣಾಙ್ಕ
on_extra_ರಾಶಿ(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ MODE16 && GET_SEG(SS) == SEG_ಕೆಳಗಿನ && getesp() > (u32)Extra_ರಾಶಿ;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ಯದಿ (on_extra_ರಾಶಿ())
        ಪ್ರತ್ಯಾಗಮ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 ರಾಶಿ_seg = SEG_ಕೆಳಗಿನ;
    u32 bkup_ss, bkup_esp;
    asm ಚಞ್ಚಲ(
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
        : "m" (ರಾಶಿ_Pos), "r" (ರಾಶಿ_seg)
        : "cc", "memory");
    ಪ್ರತ್ಯಾಗಮ eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ಯದಿ (!MODESEGMENT)
        ಪ್ರತ್ಯಾಗಮ call16(eax, edx, func);
    ಯದಿ (!MODE16 || !on_extra_ರಾಶಿ())
        ಪ್ರತ್ಯಾಗಮ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_ರಾಶಿ_pos, temp;
    asm ಚಞ್ಚಲ(
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
          , "=&r" (bkup_ರಾಶಿ_pos), "=&r" (temp), "+m" (ರಾಶಿ_Pos)
        :
        : "cc", "memory");
    ಪ್ರತ್ಯಾಗಮ eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
ಶೂನ್ಯ VISIBLE16
_farcall16(ಸಂರಚನಾ bregs *callregs, u16 callregseg)
{
    ಯದಿ (need_hop_back()) {
        ರಾಶಿ_hop_back(_farcall16, callregs, callregseg);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ASSERT16();
    asm ಚಞ್ಚಲ(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
ಶೂನ್ಯ
farcall16(ಸಂರಚನಾ bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
ಶೂನ್ಯ
farcall16big(ಸಂರಚನಾ bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
ಶೂನ್ಯ
__call16_int(ಸಂರಚನಾ bregs *callregs, u16 offset)
{
    callregs->code.offset = offset;
    ಯದಿ (!MODESEGMENT) {
        callregs->code.seg = SEG_BIOS;
        _farcall16((ಶೂನ್ಯ*)callregs - Call16Data.ss * 16, Call16Data.ss);
        ಪ್ರತ್ಯಾಗಮ;
    }
    callregs->code.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
ಶೂನ್ಯ
reset(ಶೂನ್ಯ)
{
    ಬಾಹ್ಯ ಶೂನ್ಯ reset_vector(ಶೂನ್ಯ) __noreturn;
    ಯದಿ (!MODE16)
        call16(0, 0, reset_vector);
    reset_vector();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
ಸಂರಚನಾ thread_info {
    ಶೂನ್ಯ *stackpos;
    ಸಂರಚನಾ hlist_ಗಂಟು ಗಂಟು;
};
ಸಂರಚನಾ thread_info MainThread VARFSEG = {
    ರಿಕ್ತ, { &MainThread.ಗಂಟು, &MainThread.ಗಂಟು.next }
};
#define THREADSTACKSIZE 4096

// Check if any threads are running.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
have_threads(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ (CONFIG_THREADS
            && GET_FLATPTR(MainThread.ಗಂಟು.next) != &MainThread.ಗಂಟು);
}

// Return the 'struct thread_info' for the currently running thread.
ಸಂರಚನಾ thread_info *
getCurThread(ಶೂನ್ಯ)
{
    u32 esp = getesp();
    ಯದಿ (esp <= MAIN_ರಾಶಿ_MAX)
        ಪ್ರತ್ಯಾಗಮ &MainThread;
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)ALIGN_DOWN(esp, THREADSTACKSIZE);
}

ಸ್ಥಿರ u8 CanInterrupt, ThreadControl;

// Initialize the support for internal threads.
ಶೂನ್ಯ
thread_setup(ಶೂನ್ಯ)
{
    CanInterrupt = 1;
    call16_override(1);
    ಯದಿ (! CONFIG_THREADS)
        ಪ್ರತ್ಯಾಗಮ;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
ಪೂರ್ಣಾಙ್ಕ
threads_during_optionroms(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ CONFIG_THREADS && CONFIG_RTC_TIMER && ThreadControl == 2 && in_post();
}

// Switch to next thread stack.
ಸ್ಥಿರ ಶೂನ್ಯ
switch_next(ಸಂರಚನಾ thread_info *cur)
{
    ಸಂರಚನಾ thread_info *next = container_of(
        cur->ಗಂಟು.next, ಸಂರಚನಾ thread_info, ಗಂಟು);
    ಯದಿ (cur == next)
        // Nothing to do.
        ಪ್ರತ್ಯಾಗಮ;
    asm ಚಞ್ಚಲ(
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
ಸ್ಥಿರ ಶೂನ್ಯ
__end_thread(ಸಂರಚನಾ thread_info *old)
{
    hlist_del(&old->ಗಂಟು);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    ಮುಕ್ತ(old);
    ಯದಿ (!have_threads())
        dprintf(1, "All threads complete.\n");
}

ಶೂನ್ಯ VISIBLE16 check_irqs(ಶೂನ್ಯ);

// Create a new thread and start executing 'func' in it.
ಶೂನ್ಯ
run_thread(ಶೂನ್ಯ (*func)(ಶೂನ್ಯ*), ಶೂನ್ಯ *data)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_THREADS || ! ThreadControl)
        ಗಚ್ಛ fail;
    ಸಂರಚನಾ thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    ಯದಿ (!thread)
        ಗಚ್ಛ fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (ಶೂನ್ಯ*)thread + THREADSTACKSIZE;
    ಸಂರಚನಾ thread_info *cur = getCurThread();
    ಸಂರಚನಾ thread_info *edx = cur;
    hlist_add_after(&thread->ಗಂಟು, &cur->ಗಂಟು);
    asm ಚಞ್ಚಲ(
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
    ಯದಿ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    ಪ್ರತ್ಯಾಗಮ;

fail:
    func(data);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
ಶೂನ್ಯ VISIBLE16
check_irqs(ಶೂನ್ಯ)
{
    ಯದಿ (!MODESEGMENT && !CanInterrupt) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (need_hop_back()) {
        ರಾಶಿ_hop_back(check_irqs, 0, 0);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (MODE16)
        clock_poll_irq();
    asm ಚಞ್ಚಲ("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
ಶೂನ್ಯ
yield(ಶೂನ್ಯ)
{
    ಯದಿ (MODESEGMENT || !CONFIG_THREADS) {
        check_irqs();
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಸಂರಚನಾ thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_next(cur);
    ಯದಿ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

ಶೂನ್ಯ VISIBLE16
wait_irq(ಶೂನ್ಯ)
{
    ಯದಿ (need_hop_back()) {
        ರಾಶಿ_hop_back(wait_irq, 0, 0);
        ಪ್ರತ್ಯಾಗಮ;
    }
    asm ಚಞ್ಚಲ("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
ಶೂನ್ಯ
yield_toirq(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_threads() || !CanInterrupt))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        ಪ್ರತ್ಯಾಗಮ;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
ಶೂನ್ಯ
wait_threads(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯಾವತ್ (have_threads())
        yield();
}

ಶೂನ್ಯ
mutex_lock(ಸಂರಚನಾ mutex_s *mutex)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_THREADS)
        ಪ್ರತ್ಯಾಗಮ;
    ಯಾವತ್ (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

ಶೂನ್ಯ
mutex_unlock(ಸಂರಚನಾ mutex_s *mutex)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_THREADS)
        ಪ್ರತ್ಯಾಗಮ;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

ಪೂರ್ಣಾಙ್ಕ CanPreempt VARFSEG;
ಸ್ಥಿರ u32 Preempt_ಎಣಿಕೆ;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
ಶೂನ್ಯ
start_preempt(ಶೂನ್ಯ)
{
    ಯದಿ (! threads_during_optionroms())
        ಪ್ರತ್ಯಾಗಮ;
    CanPreempt = 1;
    Preempt_ಎಣಿಕೆ = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
ಶೂನ್ಯ
finish_preempt(ಶೂನ್ಯ)
{
    ಯದಿ (! threads_during_optionroms()) {
        yield();
        ಪ್ರತ್ಯಾಗಮ;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_ಎಣಿಕೆ);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
ಪೂರ್ಣಾಙ್ಕ
wait_preempt(ಶೂನ್ಯ)
{
    ಯದಿ (MODESEGMENT || !CONFIG_THREADS || !CanPreempt
        || getesp() < MAIN_ರಾಶಿ_MAX)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಯಾವತ್ (CanPreempt)
        yield();
    ಪ್ರತ್ಯಾಗಮ 1;
}

// Try to execute 32bit threads.
ಶೂನ್ಯ VISIBLE32INIT
yield_preempt(ಶೂನ್ಯ)
{
    Preempt_ಎಣಿಕೆ++;
    switch_next(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
ಶೂನ್ಯ
check_preempt(ಶೂನ್ಯ)
{
    ಯದಿ (CONFIG_THREADS && GET_GLOBAL(CanPreempt) && have_threads())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

ಸಂರಚನಾ call32_params_s {
    ಶೂನ್ಯ *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(ಸಂರಚನಾ call32_params_s *params)
{
    ಪ್ರತ್ಯಾಗಮ ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(ಶೂನ್ಯ *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    ಸಂರಚನಾ call32_params_s params = {func, eax, edx, ecx};
    ಪ್ರತ್ಯಾಗಮ call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
