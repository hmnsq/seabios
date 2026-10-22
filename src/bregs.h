// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __BREGS_H
#define __BREGS_H

#ಅನ್ತರ್ಭಾವ "types.h" // u16
#ಅನ್ತರ್ಭಾವ "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#define UREG(ER, R, RH, RL) ಸಙ್ಘ { u32 ER; ಸಂರಚನಾ { u16 R; u16 R ## _hi; }; ಸಂರಚನಾ { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
ಸಂರಚನಾ bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    ಸಂರಚನಾ segoff_s code;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
set_cf(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ cond)
{
    ಯದಿ (cond)
        regs->flags |= F_CF;
    ಅನ್ಯಥಾ
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#define RET_EUNSUPPORTED 0x86

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
set_success(ಸಂರಚನಾ bregs *regs)
{
    set_cf(regs, 0);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
set_code_success(ಸಂರಚನಾ bregs *regs)
{
    regs->ah = 0;
    set_cf(regs, 0);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
set_invalid_silent(ಸಂರಚನಾ bregs *regs)
{
    set_cf(regs, 1);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
set_code_invalid_silent(ಸಂರಚನಾ bregs *regs, u8 code)
{
    regs->ah = code;
    set_cf(regs, 1);
}

#endif // bregs.h
