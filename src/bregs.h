// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#جے_بنا_تعریف __BREGS_H
#تعریف_کرو __BREGS_H

#ضمن "types.h" // u16
#ضمن "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#تعریف_کرو UREG(ER, R, RH, RL) اتحاد { u32 ER; بنية { u16 R; u16 R ## _hi; }; بنية { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
بنية bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    بنية segoff_s کوڈ;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

ساكن مضمن فراغ
سیٹ_cf(بنية bregs *regs, صحيح cond)
{
    إذا (cond)
        regs->flags |= F_CF;
    وإلا
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#تعریف_کرو RET_EUNSUPPORTED 0x86

ساكن مضمن فراغ
سیٹ_success(بنية bregs *regs)
{
    سیٹ_cf(regs, 0);
}

ساكن مضمن فراغ
سیٹ_کوڈ_success(بنية bregs *regs)
{
    regs->ah = 0;
    سیٹ_cf(regs, 0);
}

ساكن مضمن فراغ
سیٹ_invalid_silent(بنية bregs *regs)
{
    سیٹ_cf(regs, 1);
}

ساكن مضمن فراغ
سیٹ_کوڈ_invalid_silent(بنية bregs *regs, u8 کوڈ)
{
    regs->ah = کوڈ;
    سیٹ_cf(regs, 1);
}

#جے_انت // bregs.h
