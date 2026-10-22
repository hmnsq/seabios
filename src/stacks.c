// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // CR0_PE
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // PORT_SMI_CMD
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // rtc_use
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h" // hlist_node
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // struct mutex_s
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // useRTC

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAIN_ਢੇਰ_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

ਬਣਤਰ {
    u8 ਵਿਧੀ_2;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    ਬਣਤਰ descloc_s gdt;
} Call16_ਅੰਕੜੇ VARLOW;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C16_BIG 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C16_SMM 2

ਪੂਰਨ HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
ਸਥਿਰ ਪੂਰਨ
call32_prep(u8 ਵਿਧੀ_2)
{
    ਜੇ (!CONFIG_CALL32_SMM || ਵਿਧੀ_2 != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        ਜੇ (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            ਵਾਪਸ -1;
        ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.cr0, cr0);

        // Backup fs/gs and gdt
        ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.fs, GET_SEG(FS));
        ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.gs, GET_SEG(GS));
        ਬਣਤਰ descloc_s gdt;
        sgdt(&gdt);
        ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.gdt.ਲੰਮਾਈ, gdt.ਲੰਮਾਈ);
        ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.a20, ਸੈੱਟ_a20(1));
    }

    // Backup ss
    ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(ਪੋਰਟ_CMOS_ਸੂਚਕਾਂਕ);
    ਜੇ (!(cmosindex & NMI_DISABLE_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
        outb(cmosindex | NMI_DISABLE_ਦੋਆਧਾਰੀ_ਅੰਕ, ਪੋਰਟ_CMOS_ਸੂਚਕਾਂਕ);
        inb(ਪੋਰਟ_CMOS_ਅੰਕੜੇ);
    }
    ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.cmosindex, cmosindex);

    ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.ਵਿਧੀ_2, ਵਿਧੀ_2);
    ਵਾਪਸ 0;
}

// Restore state backed up during call32
ਸਥਿਰ u8
call32_post(ਖਾਲੀ)
{
    u8 ਵਿਧੀ_2 = GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.ਵਿਧੀ_2);
    ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.ਵਿਧੀ_2, 0);
    ਸੈੱਟ_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.ss, 0);

    ਜੇ (!CONFIG_CALL32_SMM || ਵਿਧੀ_2 != C16_SMM) {
        // Restore a20
        u8 a20 = GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.a20);
        ਜੇ (!a20)
            ਸੈੱਟ_a20(0);

        // Restore gdt and fs/gs
        ਬਣਤਰ descloc_s gdt;
        gdt.ਲੰਮਾਈ = GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.gdt.ਲੰਮਾਈ);
        gdt.addr = GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.gdt.addr);
        lgdt(&gdt);
        ਸੈੱਟ_SEG(FS, GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.fs));
        ਸੈੱਟ_SEG(GS, GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.gs));

        // Restore cr0
        u32 cr0_caching = GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.cr0) & (CR0_CD|CR0_NW);
        ਜੇ (cr0_caching)
            cr0_ਨਕਾਬ(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_ਨੀਵਾਂ(Call16_ਅੰਕੜੇ.cmosindex);
    ਜੇ (!(cmosindex & NMI_DISABLE_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
        outb(cmosindex, ਪੋਰਟ_CMOS_ਸੂਚਕਾਂਕ);
        inb(ਪੋਰਟ_CMOS_ਅੰਕੜੇ);
    }
    ਵਾਪਸ ਵਿਧੀ_2;
}

// Force next call16() to restore to a pristine cpu environment state
ਸਥਿਰ ਖਾਲੀ
call16_override(ਪੂਰਨ big)
{
    ASSERT32FLAT();
    ਜੇ (getesp() > BUILD_ਢੇਰ_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16_ਅੰਕੜੇ, 0, ਆਕਾਰ(Call16_ਅੰਕੜੇ));
    ਜੇ (big) {
        Call16_ਅੰਕੜੇ.ਵਿਧੀ_2 = C16_BIG;
        Call16_ਅੰਕੜੇ.a20 = 1;
    } ਨਹੀਂਤਾਂ {
        Call16_ਅੰਕੜੇ.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 ਵਿਧੀ_2 = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(ਵਿਧੀ_2);
    ਵਾਪਸ ret;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM32_BACK32   "  .popsection\n  .code32\n"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM16_SWITCH32 "  .code32\n"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
ਸਥਿਰ u32
call32_smm(ਖਾਲੀ *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm ਅਸਥਿਰ(
        // Backup esp / set esp to flat stack location
        "  movl %%esp, %0\n"
        "  movl %%ss, %%eax\n"
        "  shll $4, %%eax\n"
        "  addl %%eax, %%esp\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%ebx\n"
        "  outb %%al, $" __stringify(ਪੋਰਟ_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM16_SWITCH32
        "1:movl %1, %%eax\n"
        "  calll *%2\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(ਪੋਰਟ_SMI_CMD) "\n"
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
    ਵਾਪਸ eax;
}

ਸਥਿਰ u32
call16_smm(u32 eax, u32 edx, ਖਾਲੀ *func)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_CALL32_SMM)
        ਵਾਪਸ eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16_ਅੰਕੜੇ.ss << 4;
    asm ਅਸਥਿਰ(
        // Restore esp
        "  subl %0, %%esp\n"

        // Transition to 16bit mode, call func, return to 32bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%ebx\n"
        "  outb %%al, $" __stringify(ਪੋਰਟ_SMI_CMD) "\n"
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
        "  outb %%al, $" __stringify(ਪੋਰਟ_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Set esp to flat stack location
        ASM32_BACK32
        "2:addl %0, %%esp\n"
        : "+r" (stackoffset), "+r" (eax), "+d" (edx)
        : "r" (func)
        : "eax", "ecx", "ebx", "cc", "memory");
    ਵਾਪਸ eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(ਖਾਲੀ *func, u32 eax, u32 errret)
{
    ASSERT16();
    ਜੇ (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        ਵਾਪਸ call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    ਪੂਰਨ ret = call32_prep(C16_BIG);
    ਜੇ (ret)
        ਵਾਪਸ errret;
    u32 bkup_ss, bkup_esp;
    asm ਅਸਥਿਰ(
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
    ਵਾਪਸ eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
ਸਥਿਰ u32
call16(u32 eax, u32 edx, ਖਾਲੀ *func)
{
    ASSERT32FLAT();
    ਜੇ (getesp() > MAIN_ਢੇਰ_MAX)
        panic("call16 with invalid stack\n");
    ਜੇ (CONFIG_CALL32_SMM && Call16_ਅੰਕੜੇ.ਵਿਧੀ_2 == C16_SMM)
        ਵਾਪਸ call16_smm(eax, edx, func);

    ਬਾਹਰੀ ਖਾਲੀ transition16big(ਖਾਲੀ);
    ਬਾਹਰੀ ਖਾਲੀ transition16(ਖਾਲੀ);
    ਖਾਲੀ *thunk = transition16;
    ਜੇ (Call16_ਅੰਕੜੇ.ਵਿਧੀ_2 == C16_BIG || ਵਿੱਚ_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16_ਅੰਕੜੇ.ss;
    asm ਅਸਥਿਰ(
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
    ਵਾਪਸ eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_ਢੇਰ[BUILD_EXTRA_ਢੇਰ_ਮਾਪ+1] VARLOW __aligned(8);
u8 *ਢੇਰ_Pos VARLOW;

// Test if currently on the extra stack
ਪੂਰਨ
on_extra_ਢੇਰ(ਖਾਲੀ)
{
    ਵਾਪਸ MODE16 && GET_SEG(SS) == SEG_ਨੀਵਾਂ && getesp() > (u32)Extra_ਢੇਰ;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, ਖਾਲੀ *func)
{
    ਜੇ (on_extra_ਢੇਰ())
        ਵਾਪਸ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 ਢੇਰ_seg = SEG_ਨੀਵਾਂ;
    u32 bkup_ss, bkup_esp;
    asm ਅਸਥਿਰ(
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
        : "m" (ਢੇਰ_Pos), "r" (ਢੇਰ_seg)
        : "cc", "memory");
    ਵਾਪਸ eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, ਖਾਲੀ *func)
{
    ਜੇ (!MODESEGMENT)
        ਵਾਪਸ call16(eax, edx, func);
    ਜੇ (!MODE16 || !on_extra_ਢੇਰ())
        ਵਾਪਸ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_ਢੇਰ_pos, ਆਰਜ਼ੀ;
    asm ਅਸਥਿਰ(
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
          , "=&r" (bkup_ਢੇਰ_pos), "=&r" (ਆਰਜ਼ੀ), "+m" (ਢੇਰ_Pos)
        :
        : "cc", "memory");
    ਵਾਪਸ eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
ਖਾਲੀ VISIBLE16
_farcall16(ਬਣਤਰ bregs *callregs, u16 callregseg)
{
    ਜੇ (need_hop_ਪਿੱਠ()) {
        ਢੇਰ_hop_ਪਿੱਠ(_farcall16, callregs, callregseg);
        ਵਾਪਸ;
    }
    ASSERT16();
    asm ਅਸਥਿਰ(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
ਖਾਲੀ
farcall16(ਬਣਤਰ bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
ਖਾਲੀ
farcall16big(ਬਣਤਰ bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
ਖਾਲੀ
__call16_int(ਬਣਤਰ bregs *callregs, u16 ਵਿੱਥ_2)
{
    callregs->ਕੋਡ.ਵਿੱਥ_2 = ਵਿੱਥ_2;
    ਜੇ (!MODESEGMENT) {
        callregs->ਕੋਡ.seg = SEG_BIOS;
        _farcall16((ਖਾਲੀ*)callregs - Call16_ਅੰਕੜੇ.ss * 16, Call16_ਅੰਕੜੇ.ss);
        ਵਾਪਸ;
    }
    callregs->ਕੋਡ.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
ਖਾਲੀ
reset(ਖਾਲੀ)
{
    ਬਾਹਰੀ ਖਾਲੀ reset_ਸਦਿਸ਼(ਖਾਲੀ) __noreturn;
    ਜੇ (!MODE16)
        call16(0, 0, reset_ਸਦਿਸ਼);
    reset_ਸਦਿਸ਼();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
ਬਣਤਰ thread_info {
    ਖਾਲੀ *stackpos;
    ਬਣਤਰ hlist_ਗੰਢ ਗੰਢ;
};
ਬਣਤਰ thread_info MainThread VARFSEG = {
    ਨਿਰਾ, { &MainThread.ਗੰਢ, &MainThread.ਗੰਢ.ਅਗਲਾ }
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THREADSTACKSIZE 4096

// Check if any threads are running.
ਸਥਿਰ ਪੂਰਨ
have_ਧਾਗੇ(ਖਾਲੀ)
{
    ਵਾਪਸ (CONFIG_ਧਾਗੇ
            && GET_FLATPTR(MainThread.ਗੰਢ.ਅਗਲਾ) != &MainThread.ਗੰਢ);
}

// Return the 'struct thread_info' for the currently running thread.
ਬਣਤਰ thread_info *
getCurThread(ਖਾਲੀ)
{
    u32 esp = getesp();
    ਜੇ (esp <= MAIN_ਢੇਰ_MAX)
        ਵਾਪਸ &MainThread;
    ਵਾਪਸ (ਖਾਲੀ*)ALIGN_ਥੱਲੇ(esp, THREADSTACKSIZE);
}

ਸਥਿਰ u8 Can_ਰੁਕਾਵਟ, ThreadControl;

// Initialize the support for internal threads.
ਖਾਲੀ
thread_ਤਿਆਰੀ(ਖਾਲੀ)
{
    Can_ਰੁਕਾਵਟ = 1;
    call16_override(1);
    ਜੇ (! CONFIG_ਧਾਗੇ)
        ਵਾਪਸ;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
ਪੂਰਨ
ਧਾਗੇ_during_optionroms(ਖਾਲੀ)
{
    ਵਾਪਸ CONFIG_ਧਾਗੇ && CONFIG_RTC_TIMER && ThreadControl == 2 && ਵਿੱਚ_post();
}

// Switch to next thread stack.
ਸਥਿਰ ਖਾਲੀ
switch_ਅਗਲਾ(ਬਣਤਰ thread_info *cur)
{
    ਬਣਤਰ thread_info *ਅਗਲਾ = container_ਦਾ(
        cur->ਗੰਢ.ਅਗਲਾ, ਬਣਤਰ thread_info, ਗੰਢ);
    ਜੇ (cur == ਅਗਲਾ)
        // Nothing to do.
        ਵਾਪਸ;
    asm ਅਸਥਿਰ(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(ਅਗਲਾ)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
ਸਥਿਰ ਖਾਲੀ
__end_thread(ਬਣਤਰ thread_info *old)
{
    hlist_del(&old->ਗੰਢ);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    ਮੁਕਤ(old);
    ਜੇ (!have_ਧਾਗੇ())
        dprintf(1, "All threads complete.\n");
}

ਖਾਲੀ VISIBLE16 check_irqs(ਖਾਲੀ);

// Create a new thread and start executing 'func' in it.
ਖਾਲੀ
ਲੜੀਵਾਰ_thread(ਖਾਲੀ (*func)(ਖਾਲੀ*), ਖਾਲੀ *ਅੰਕੜੇ)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_ਧਾਗੇ || ! ThreadControl)
        ਜਾਓ fail;
    ਬਣਤਰ thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    ਜੇ (!thread)
        ਜਾਓ fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (ਖਾਲੀ*)thread + THREADSTACKSIZE;
    ਬਣਤਰ thread_info *cur = getCurThread();
    ਬਣਤਰ thread_info *edx = cur;
    hlist_ਜੋੜੋ_after(&thread->ਗੰਢ, &cur->ਗੰਢ);
    asm ਅਸਥਿਰ(
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
        : "+a"(ਅੰਕੜੇ), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    ਜੇ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    ਵਾਪਸ;

fail:
    func(ਅੰਕੜੇ);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
ਖਾਲੀ VISIBLE16
check_irqs(ਖਾਲੀ)
{
    ਜੇ (!MODESEGMENT && !Can_ਰੁਕਾਵਟ) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        ਵਾਪਸ;
    }
    ਜੇ (need_hop_ਪਿੱਠ()) {
        ਢੇਰ_hop_ਪਿੱਠ(check_irqs, 0, 0);
        ਵਾਪਸ;
    }
    ਜੇ (MODE16)
        clock_poll_irq();
    asm ਅਸਥਿਰ("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
ਖਾਲੀ
yield(ਖਾਲੀ)
{
    ਜੇ (MODESEGMENT || !CONFIG_ਧਾਗੇ) {
        check_irqs();
        ਵਾਪਸ;
    }
    ਬਣਤਰ thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_ਅਗਲਾ(cur);
    ਜੇ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

ਖਾਲੀ VISIBLE16
wait_irq(ਖਾਲੀ)
{
    ਜੇ (need_hop_ਪਿੱਠ()) {
        ਢੇਰ_hop_ਪਿੱਠ(wait_irq, 0, 0);
        ਵਾਪਸ;
    }
    asm ਅਸਥਿਰ("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
ਖਾਲੀ
yield_toirq(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_ਧਾਗੇ() || !Can_ਰੁਕਾਵਟ))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        ਵਾਪਸ;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
ਖਾਲੀ
wait_ਧਾਗੇ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜਦੋਂ (have_ਧਾਗੇ())
        yield();
}

ਖਾਲੀ
mutex_ਤਾਲਾ(ਬਣਤਰ mutex_s *mutex)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_ਧਾਗੇ)
        ਵਾਪਸ;
    ਜਦੋਂ (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

ਖਾਲੀ
mutex_unlock(ਬਣਤਰ mutex_s *mutex)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_ਧਾਗੇ)
        ਵਾਪਸ;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

ਪੂਰਨ CanPreempt VARFSEG;
ਸਥਿਰ u32 Preempt_ਗਿਣਤੀ;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
ਖਾਲੀ
ਸ਼ੁਰੂ_preempt(ਖਾਲੀ)
{
    ਜੇ (! ਧਾਗੇ_during_optionroms())
        ਵਾਪਸ;
    CanPreempt = 1;
    Preempt_ਗਿਣਤੀ = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
ਖਾਲੀ
ਅੰਤ_preempt(ਖਾਲੀ)
{
    ਜੇ (! ਧਾਗੇ_during_optionroms()) {
        yield();
        ਵਾਪਸ;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_ਗਿਣਤੀ);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
ਪੂਰਨ
wait_preempt(ਖਾਲੀ)
{
    ਜੇ (MODESEGMENT || !CONFIG_ਧਾਗੇ || !CanPreempt
        || getesp() < MAIN_ਢੇਰ_MAX)
        ਵਾਪਸ 0;
    ਜਦੋਂ (CanPreempt)
        yield();
    ਵਾਪਸ 1;
}

// Try to execute 32bit threads.
ਖਾਲੀ VISIBLE32INIT
yield_preempt(ਖਾਲੀ)
{
    Preempt_ਗਿਣਤੀ++;
    switch_ਅਗਲਾ(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
ਖਾਲੀ
check_preempt(ਖਾਲੀ)
{
    ਜੇ (CONFIG_ਧਾਗੇ && GET_GLOBAL(CanPreempt) && have_ਧਾਗੇ())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

ਬਣਤਰ call32_params_s {
    ਖਾਲੀ *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(ਬਣਤਰ call32_params_s *params)
{
    ਵਾਪਸ ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(ਖਾਲੀ *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    ਬਣਤਰ call32_params_s params = {func, eax, edx, ecx};
    ਵਾਪਸ call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
