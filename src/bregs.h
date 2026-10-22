// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __BREGS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __BREGS_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u16
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UREG(ER, R, RH, RL) ਸੰਘ { u32 ER; ਬਣਤਰ { u16 R; u16 R ## _hi; }; ਬਣਤਰ { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
ਬਣਤਰ bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    ਬਣਤਰ segoff_s ਕੋਡ;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ਸੈੱਟ_cf(ਬਣਤਰ bregs *regs, ਪੂਰਨ cond)
{
    ਜੇ (cond)
        regs->flags |= F_CF;
    ਨਹੀਂਤਾਂ
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_EUNSUPPORTED 0x86

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ਸੈੱਟ_success(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_cf(regs, 0);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ਸੈੱਟ_ਕੋਡ_success(ਬਣਤਰ bregs *regs)
{
    regs->ah = 0;
    ਸੈੱਟ_cf(regs, 0);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ਸੈੱਟ_invalid_silent(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_cf(regs, 1);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ਸੈੱਟ_ਕੋਡ_invalid_silent(ਬਣਤਰ bregs *regs, u8 ਕੋਡ)
{
    regs->ah = ਕੋਡ;
    ਸੈੱਟ_cf(regs, 1);
}

#ਜੇ_ਅੰਤ // bregs.h
