// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __BREGS_H
#define __BREGS_H

#অন্তর্ভাব "types.h" // u16
#অন্তর্ভাব "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#define UREG(ER, R, RH, RL) সঙ্ঘ { u32 ER; সংরচনা { u16 R; u16 R ## _hi; }; সংরচনা { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
সংরচনা bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    সংরচনা segoff_s code;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

স্থির অন্তঃস্থ শূন্য
set_cf(সংরচনা bregs *regs, পূর্ণাঙ্ক cond)
{
    যদি (cond)
        regs->flags |= F_CF;
    অন্যথা
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#define RET_EUNSUPPORTED 0x86

স্থির অন্তঃস্থ শূন্য
set_success(সংরচনা bregs *regs)
{
    set_cf(regs, 0);
}

স্থির অন্তঃস্থ শূন্য
set_code_success(সংরচনা bregs *regs)
{
    regs->ah = 0;
    set_cf(regs, 0);
}

স্থির অন্তঃস্থ শূন্য
set_invalid_silent(সংরচনা bregs *regs)
{
    set_cf(regs, 1);
}

স্থির অন্তঃস্থ শূন্য
set_code_invalid_silent(সংরচনা bregs *regs, u8 code)
{
    regs->ah = code;
    set_cf(regs, 1);
}

#endif // bregs.h
