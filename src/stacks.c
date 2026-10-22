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

#ವ್ಯಾಖ್ಯಾನಿಸು MAIN_ರಾಶಿ_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

ರಚನೆ {
    u8 ವಿಧಾನ_2;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    ರಚನೆ descloc_s gdt;
} Call16_ದತ್ತಾಂಶ VARLOW;

#ವ್ಯಾಖ್ಯಾನಿಸು C16_BIG 1
#ವ್ಯಾಖ್ಯಾನಿಸು C16_SMM 2

ಪೂರ್ಣಾಂಕ HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
call32_prep(u8 ವಿಧಾನ_2)
{
    ಆದರೆ (!CONFIG_CALL32_SMM || ವಿಧಾನ_2 != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        ಆದರೆ (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            ಹಿಂತಿರುಗಿ -1;
        ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.cr0, cr0);

        // Backup fs/gs and gdt
        ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.fs, GET_SEG(FS));
        ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.gs, GET_SEG(GS));
        ರಚನೆ descloc_s gdt;
        sgdt(&gdt);
        ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.gdt.ಉದ್ದಳತೆ, gdt.ಉದ್ದಳತೆ);
        ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.a20, ಗಣ_a20(1));
    }

    // Backup ss
    ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(ಪೋರ್ಟ್_CMOS_ಸೂಚ್ಯಂಕ);
    ಆದರೆ (!(cmosindex & NMI_DISABLE_ಬಿಟ್)) {
        outb(cmosindex | NMI_DISABLE_ಬಿಟ್, ಪೋರ್ಟ್_CMOS_ಸೂಚ್ಯಂಕ);
        inb(ಪೋರ್ಟ್_CMOS_ದತ್ತಾಂಶ);
    }
    ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.cmosindex, cmosindex);

    ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.ವಿಧಾನ_2, ವಿಧಾನ_2);
    ಹಿಂತಿರುಗಿ 0;
}

// Restore state backed up during call32
ಸ್ಥಿರ u8
call32_post(ಶೂನ್ಯ)
{
    u8 ವಿಧಾನ_2 = GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.ವಿಧಾನ_2);
    ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.ವಿಧಾನ_2, 0);
    ಗಣ_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.ss, 0);

    ಆದರೆ (!CONFIG_CALL32_SMM || ವಿಧಾನ_2 != C16_SMM) {
        // Restore a20
        u8 a20 = GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.a20);
        ಆದರೆ (!a20)
            ಗಣ_a20(0);

        // Restore gdt and fs/gs
        ರಚನೆ descloc_s gdt;
        gdt.ಉದ್ದಳತೆ = GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.gdt.ಉದ್ದಳತೆ);
        gdt.addr = GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.gdt.addr);
        lgdt(&gdt);
        ಗಣ_SEG(FS, GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.fs));
        ಗಣ_SEG(GS, GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.gs));

        // Restore cr0
        u32 cr0_caching = GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.cr0) & (CR0_CD|CR0_NW);
        ಆದರೆ (cr0_caching)
            cr0_ಮುಖವಾಡ(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_ಕೆಳಗಿನ(Call16_ದತ್ತಾಂಶ.cmosindex);
    ಆದರೆ (!(cmosindex & NMI_DISABLE_ಬಿಟ್)) {
        outb(cmosindex, ಪೋರ್ಟ್_CMOS_ಸೂಚ್ಯಂಕ);
        inb(ಪೋರ್ಟ್_CMOS_ದತ್ತಾಂಶ);
    }
    ಹಿಂತಿರುಗಿ ವಿಧಾನ_2;
}

// Force next call16() to restore to a pristine cpu environment state
ಸ್ಥಿರ ಶೂನ್ಯ
call16_override(ಪೂರ್ಣಾಂಕ big)
{
    ASSERT32FLAT();
    ಆದರೆ (getesp() > BUILD_ರಾಶಿ_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16_ದತ್ತಾಂಶ, 0, ಗಾತ್ರ(Call16_ದತ್ತಾಂಶ));
    ಆದರೆ (big) {
        Call16_ದತ್ತಾಂಶ.ವಿಧಾನ_2 = C16_BIG;
        Call16_ದತ್ತಾಂಶ.a20 = 1;
    } ಇಲ್ಲದಿದ್ದರೆ {
        Call16_ದತ್ತಾಂಶ.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 ವಿಧಾನ_2 = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(ವಿಧಾನ_2);
    ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿಸು ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#ವ್ಯಾಖ್ಯಾನಿಸು ASM32_BACK32   "  .popsection\n  .code32\n"
#ವ್ಯಾಖ್ಯಾನಿಸು ASM16_SWITCH32 "  .code32\n"
#ವ್ಯಾಖ್ಯಾನಿಸು ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
ಸ್ಥಿರ u32
call32_smm(ಶೂನ್ಯ *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm ಅಸ್ಥಿರ(
        // Backup esp / set esp to flat stack location
        "  movl %%esp, %0\n"
        "  movl %%ss, %%eax\n"
        "  shll $4, %%eax\n"
        "  addl %%eax, %%esp\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%ebx\n"
        "  outb %%al, $" __stringify(ಪೋರ್ಟ್_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM16_SWITCH32
        "1:movl %1, %%eax\n"
        "  calll *%2\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(ಪೋರ್ಟ್_SMI_CMD) "\n"
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
    ಹಿಂತಿರುಗಿ eax;
}

ಸ್ಥಿರ u32
call16_smm(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_CALL32_SMM)
        ಹಿಂತಿರುಗಿ eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16_ದತ್ತಾಂಶ.ss << 4;
    asm ಅಸ್ಥಿರ(
        // Restore esp
        "  subl %0, %%esp\n"

        // Transition to 16bit mode, call func, return to 32bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%ebx\n"
        "  outb %%al, $" __stringify(ಪೋರ್ಟ್_SMI_CMD) "\n"
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
        "  outb %%al, $" __stringify(ಪೋರ್ಟ್_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Set esp to flat stack location
        ASM32_BACK32
        "2:addl %0, %%esp\n"
        : "+r" (stackoffset), "+r" (eax), "+d" (edx)
        : "r" (func)
        : "eax", "ecx", "ebx", "cc", "memory");
    ಹಿಂತಿರುಗಿ eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(ಶೂನ್ಯ *func, u32 eax, u32 errret)
{
    ASSERT16();
    ಆದರೆ (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        ಹಿಂತಿರುಗಿ call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    ಪೂರ್ಣಾಂಕ ret = call32_prep(C16_BIG);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ errret;
    u32 bkup_ss, bkup_esp;
    asm ಅಸ್ಥಿರ(
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
    ಹಿಂತಿರುಗಿ eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
ಸ್ಥಿರ u32
call16(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ASSERT32FLAT();
    ಆದರೆ (getesp() > MAIN_ರಾಶಿ_MAX)
        panic("call16 with invalid stack\n");
    ಆದರೆ (CONFIG_CALL32_SMM && Call16_ದತ್ತಾಂಶ.ವಿಧಾನ_2 == C16_SMM)
        ಹಿಂತಿರುಗಿ call16_smm(eax, edx, func);

    ಬಾಹ್ಯ ಶೂನ್ಯ transition16big(ಶೂನ್ಯ);
    ಬಾಹ್ಯ ಶೂನ್ಯ transition16(ಶೂನ್ಯ);
    ಶೂನ್ಯ *thunk = transition16;
    ಆದರೆ (Call16_ದತ್ತಾಂಶ.ವಿಧಾನ_2 == C16_BIG || ಒಳ_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16_ದತ್ತಾಂಶ.ss;
    asm ಅಸ್ಥಿರ(
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
    ಹಿಂತಿರುಗಿ eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_ರಾಶಿ[BUILD_EXTRA_ರಾಶಿ_ಗಾತ್ರ+1] VARLOW __aligned(8);
u8 *ರಾಶಿ_Pos VARLOW;

// Test if currently on the extra stack
ಪೂರ್ಣಾಂಕ
on_extra_ರಾಶಿ(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ MODE16 && GET_SEG(SS) == SEG_ಕೆಳಗಿನ && getesp() > (u32)Extra_ರಾಶಿ;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ಆದರೆ (on_extra_ರಾಶಿ())
        ಹಿಂತಿರುಗಿ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 ರಾಶಿ_seg = SEG_ಕೆಳಗಿನ;
    u32 bkup_ss, bkup_esp;
    asm ಅಸ್ಥಿರ(
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
    ಹಿಂತಿರುಗಿ eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, ಶೂನ್ಯ *func)
{
    ಆದರೆ (!MODESEGMENT)
        ಹಿಂತಿರುಗಿ call16(eax, edx, func);
    ಆದರೆ (!MODE16 || !on_extra_ರಾಶಿ())
        ಹಿಂತಿರುಗಿ ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_ರಾಶಿ_pos, ತಾತ್ಕಾಲಿಕ;
    asm ಅಸ್ಥಿರ(
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
          , "=&r" (bkup_ರಾಶಿ_pos), "=&r" (ತಾತ್ಕಾಲಿಕ), "+m" (ರಾಶಿ_Pos)
        :
        : "cc", "memory");
    ಹಿಂತಿರುಗಿ eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
ಶೂನ್ಯ VISIBLE16
_farcall16(ರಚನೆ bregs *callregs, u16 callregseg)
{
    ಆದರೆ (need_hop_ಹಿಂಭಾಗ()) {
        ರಾಶಿ_hop_ಹಿಂಭಾಗ(_farcall16, callregs, callregseg);
        ಹಿಂತಿರುಗಿ;
    }
    ASSERT16();
    asm ಅಸ್ಥಿರ(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
ಶೂನ್ಯ
farcall16(ರಚನೆ bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
ಶೂನ್ಯ
farcall16big(ರಚನೆ bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
ಶೂನ್ಯ
__call16_int(ರಚನೆ bregs *callregs, u16 ಸ್ಥಾನಾಂತರ_2)
{
    callregs->ಸಂಕೇತ_2.ಸ್ಥಾನಾಂತರ_2 = ಸ್ಥಾನಾಂತರ_2;
    ಆದರೆ (!MODESEGMENT) {
        callregs->ಸಂಕೇತ_2.seg = SEG_BIOS;
        _farcall16((ಶೂನ್ಯ*)callregs - Call16_ದತ್ತಾಂಶ.ss * 16, Call16_ದತ್ತಾಂಶ.ss);
        ಹಿಂತಿರುಗಿ;
    }
    callregs->ಸಂಕೇತ_2.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
ಶೂನ್ಯ
reset(ಶೂನ್ಯ)
{
    ಬಾಹ್ಯ ಶೂನ್ಯ reset_ಸದಿಶ(ಶೂನ್ಯ) __noreturn;
    ಆದರೆ (!MODE16)
        call16(0, 0, reset_ಸದಿಶ);
    reset_ಸದಿಶ();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
ರಚನೆ thread_info {
    ಶೂನ್ಯ *stackpos;
    ರಚನೆ hlist_ಗಂಟು ಗಂಟು;
};
ರಚನೆ thread_info MainThread VARFSEG = {
    ಖಾಲಿ, { &MainThread.ಗಂಟು, &MainThread.ಗಂಟು.ಮುಂದಿನ }
};
#ವ್ಯಾಖ್ಯಾನಿಸು THREADSTACKSIZE 4096

// Check if any threads are running.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
have_ಎಳೆಗಳು(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ (CONFIG_ಎಳೆಗಳು
            && GET_FLATPTR(MainThread.ಗಂಟು.ಮುಂದಿನ) != &MainThread.ಗಂಟು);
}

// Return the 'struct thread_info' for the currently running thread.
ರಚನೆ thread_info *
getCurThread(ಶೂನ್ಯ)
{
    u32 esp = getesp();
    ಆದರೆ (esp <= MAIN_ರಾಶಿ_MAX)
        ಹಿಂತಿರುಗಿ &MainThread;
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)ALIGN_ಕೆಳಕ್ಕೆ(esp, THREADSTACKSIZE);
}

ಸ್ಥಿರ u8 Can_ಅಡಚಣೆ, ThreadControl;

// Initialize the support for internal threads.
ಶೂನ್ಯ
thread_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    Can_ಅಡಚಣೆ = 1;
    call16_override(1);
    ಆದರೆ (! CONFIG_ಎಳೆಗಳು)
        ಹಿಂತಿರುಗಿ;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
ಪೂರ್ಣಾಂಕ
ಎಳೆಗಳು_during_optionroms(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ CONFIG_ಎಳೆಗಳು && CONFIG_RTC_TIMER && ThreadControl == 2 && ಒಳ_post();
}

// Switch to next thread stack.
ಸ್ಥಿರ ಶೂನ್ಯ
switch_ಮುಂದಿನ(ರಚನೆ thread_info *cur)
{
    ರಚನೆ thread_info *ಮುಂದಿನ = container_ದ(
        cur->ಗಂಟು.ಮುಂದಿನ, ರಚನೆ thread_info, ಗಂಟು);
    ಆದರೆ (cur == ಮುಂದಿನ)
        // Nothing to do.
        ಹಿಂತಿರುಗಿ;
    asm ಅಸ್ಥಿರ(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(ಮುಂದಿನ)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
ಸ್ಥಿರ ಶೂನ್ಯ
__end_thread(ರಚನೆ thread_info *old)
{
    hlist_del(&old->ಗಂಟು);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    ಬಿಡುಗಡೆ(old);
    ಆದರೆ (!have_ಎಳೆಗಳು())
        dprintf(1, "All threads complete.\n");
}

ಶೂನ್ಯ VISIBLE16 check_irqs(ಶೂನ್ಯ);

// Create a new thread and start executing 'func' in it.
ಶೂನ್ಯ
ಸತತ_thread(ಶೂನ್ಯ (*func)(ಶೂನ್ಯ*), ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_ಎಳೆಗಳು || ! ThreadControl)
        ಹೋಗು fail;
    ರಚನೆ thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    ಆದರೆ (!thread)
        ಹೋಗು fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (ಶೂನ್ಯ*)thread + THREADSTACKSIZE;
    ರಚನೆ thread_info *cur = getCurThread();
    ರಚನೆ thread_info *edx = cur;
    hlist_ಕೂಡಿಸು_after(&thread->ಗಂಟು, &cur->ಗಂಟು);
    asm ಅಸ್ಥಿರ(
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
        : "+a"(ದತ್ತಾಂಶ), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    ಆದರೆ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    ಹಿಂತಿರುಗಿ;

fail:
    func(ದತ್ತಾಂಶ);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
ಶೂನ್ಯ VISIBLE16
check_irqs(ಶೂನ್ಯ)
{
    ಆದರೆ (!MODESEGMENT && !Can_ಅಡಚಣೆ) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (need_hop_ಹಿಂಭಾಗ()) {
        ರಾಶಿ_hop_ಹಿಂಭಾಗ(check_irqs, 0, 0);
        ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (MODE16)
        clock_poll_irq();
    asm ಅಸ್ಥಿರ("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
ಶೂನ್ಯ
yield(ಶೂನ್ಯ)
{
    ಆದರೆ (MODESEGMENT || !CONFIG_ಎಳೆಗಳು) {
        check_irqs();
        ಹಿಂತಿರುಗಿ;
    }
    ರಚನೆ thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_ಮುಂದಿನ(cur);
    ಆದರೆ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

ಶೂನ್ಯ VISIBLE16
wait_irq(ಶೂನ್ಯ)
{
    ಆದರೆ (need_hop_ಹಿಂಭಾಗ()) {
        ರಾಶಿ_hop_ಹಿಂಭಾಗ(wait_irq, 0, 0);
        ಹಿಂತಿರುಗಿ;
    }
    asm ಅಸ್ಥಿರ("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
ಶೂನ್ಯ
yield_toirq(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_ಎಳೆಗಳು() || !Can_ಅಡಚಣೆ))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        ಹಿಂತಿರುಗಿ;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
ಶೂನ್ಯ
wait_ಎಳೆಗಳು(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆಗ (have_ಎಳೆಗಳು())
        yield();
}

ಶೂನ್ಯ
mutex_ಬೀಗ(ರಚನೆ mutex_s *mutex)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_ಎಳೆಗಳು)
        ಹಿಂತಿರುಗಿ;
    ಆಗ (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

ಶೂನ್ಯ
mutex_unlock(ರಚನೆ mutex_s *mutex)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_ಎಳೆಗಳು)
        ಹಿಂತಿರುಗಿ;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

ಪೂರ್ಣಾಂಕ CanPreempt VARFSEG;
ಸ್ಥಿರ u32 Preempt_ಎಣಿಕೆ;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
ಶೂನ್ಯ
ಆರಂಭ_preempt(ಶೂನ್ಯ)
{
    ಆದರೆ (! ಎಳೆಗಳು_during_optionroms())
        ಹಿಂತಿರುಗಿ;
    CanPreempt = 1;
    Preempt_ಎಣಿಕೆ = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
ಶೂನ್ಯ
ಮುಕ್ತಾಯ_preempt(ಶೂನ್ಯ)
{
    ಆದರೆ (! ಎಳೆಗಳು_during_optionroms()) {
        yield();
        ಹಿಂತಿರುಗಿ;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_ಎಣಿಕೆ);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
ಪೂರ್ಣಾಂಕ
wait_preempt(ಶೂನ್ಯ)
{
    ಆದರೆ (MODESEGMENT || !CONFIG_ಎಳೆಗಳು || !CanPreempt
        || getesp() < MAIN_ರಾಶಿ_MAX)
        ಹಿಂತಿರುಗಿ 0;
    ಆಗ (CanPreempt)
        yield();
    ಹಿಂತಿರುಗಿ 1;
}

// Try to execute 32bit threads.
ಶೂನ್ಯ VISIBLE32INIT
yield_preempt(ಶೂನ್ಯ)
{
    Preempt_ಎಣಿಕೆ++;
    switch_ಮುಂದಿನ(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
ಶೂನ್ಯ
check_preempt(ಶೂನ್ಯ)
{
    ಆದರೆ (CONFIG_ಎಳೆಗಳು && GET_GLOBAL(CanPreempt) && have_ಎಳೆಗಳು())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

ರಚನೆ call32_params_s {
    ಶೂನ್ಯ *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(ರಚನೆ call32_params_s *params)
{
    ಹಿಂತಿರುಗಿ ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(ಶೂನ್ಯ *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    ರಚನೆ call32_params_s params = {func, eax, edx, ecx};
    ಹಿಂತಿರುಗಿ call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
