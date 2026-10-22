// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#నిర్వచితం_కాకపోతే __BREGS_H
#నిర్వచించు __BREGS_H

#అన్తర్భావ "types.h" // u16
#అన్తర్భావ "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#నిర్వచించు UREG(ER, R, RH, RL) సంఘం { u32 ER; నిర్మాణం { u16 R; u16 R ## _hi; }; నిర్మాణం { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
నిర్మాణం bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    నిర్మాణం segoff_s సంకేతం_2;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

స్థిర అంతఃవరుస శూన్య
సమితి_cf(నిర్మాణం bregs *regs, పూర్ణాంకం cond)
{
    ఒకవేళ (cond)
        regs->flags |= F_CF;
    లేకపోతే
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#నిర్వచించు RET_EUNSUPPORTED 0x86

స్థిర అంతఃవరుస శూన్య
సమితి_success(నిర్మాణం bregs *regs)
{
    సమితి_cf(regs, 0);
}

స్థిర అంతఃవరుస శూన్య
సమితి_సంకేతం_success(నిర్మాణం bregs *regs)
{
    regs->ah = 0;
    సమితి_cf(regs, 0);
}

స్థిర అంతఃవరుస శూన్య
సమితి_invalid_silent(నిర్మాణం bregs *regs)
{
    సమితి_cf(regs, 1);
}

స్థిర అంతఃవరుస శూన్య
సమితి_సంకేతం_invalid_silent(నిర్మాణం bregs *regs, u8 సంకేతం_2)
{
    regs->ah = సంకేతం_2;
    సమితి_cf(regs, 1);
}

#అయితే_ముగింపు // bregs.h
