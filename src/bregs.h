// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __BREGS_H
#define __BREGS_H

#شامل "types.h" // u16
#شامل "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#define UREG(ER, R, RH, RL) اجتماع { u32 ER; ساختار { u16 R; u16 R ## _hi; }; ساختار { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
ساختار bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    ساختار segoff_s code;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

ایستا درونخطی تهی
set_cf(ساختار bregs *regs, صحیح cond)
{
    اگر (cond)
        regs->flags |= F_CF;
    وگرنه
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#define RET_EUNSUPPORTED 0x86

ایستا درونخطی تهی
set_success(ساختار bregs *regs)
{
    set_cf(regs, 0);
}

ایستا درونخطی تهی
set_code_success(ساختار bregs *regs)
{
    regs->ah = 0;
    set_cf(regs, 0);
}

ایستا درونخطی تهی
set_invalid_silent(ساختار bregs *regs)
{
    set_cf(regs, 1);
}

ایستا درونخطی تهی
set_code_invalid_silent(ساختار bregs *regs, u8 code)
{
    regs->ah = code;
    set_cf(regs, 1);
}

#endif // bregs.h
