// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#اگر_غیر_تعریف_شدہ __BREGS_H
#تعریف_کرو __BREGS_H

#شامل "types.h" // u16
#شامل "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#تعریف_کرو UREG(ER, R, RH, RL) اتحاد { u32 ER; ساخت { u16 R; u16 R ## _hi; }; ساخت { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
ساخت bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    ساخت segoff_s رمز;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

ساکن اندرونی خالی
سیٹ_cf(ساخت bregs *regs, صحیح cond)
{
    اگر (cond)
        regs->flags |= F_CF;
    ورنہ
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#تعریف_کرو RET_EUNSUPPORTED 0x86

ساکن اندرونی خالی
سیٹ_success(ساخت bregs *regs)
{
    سیٹ_cf(regs, 0);
}

ساکن اندرونی خالی
سیٹ_رمز_success(ساخت bregs *regs)
{
    regs->ah = 0;
    سیٹ_cf(regs, 0);
}

ساکن اندرونی خالی
سیٹ_invalid_silent(ساخت bregs *regs)
{
    سیٹ_cf(regs, 1);
}

ساکن اندرونی خالی
سیٹ_رمز_invalid_silent(ساخت bregs *regs, u8 رمز)
{
    regs->ah = رمز;
    سیٹ_cf(regs, 1);
}

#اگر_ختم // bregs.h
