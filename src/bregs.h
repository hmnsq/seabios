// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __BREGS_H
#define __BREGS_H

#અન્તર્ભાવ "types.h" // u16
#અન્તર્ભાવ "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#define UREG(ER, R, RH, RL) સંઘ { u32 ER; સંરચના { u16 R; u16 R ## _hi; }; સંરચના { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
સંરચના bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    સંરચના segoff_s code;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

સ્થિર અંતઃરેખ શૂન્ય
set_cf(સંરચના bregs *regs, પૂર્ણાંક cond)
{
    જો (cond)
        regs->flags |= F_CF;
    નહીંતર
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#define RET_EUNSUPPORTED 0x86

સ્થિર અંતઃરેખ શૂન્ય
set_success(સંરચના bregs *regs)
{
    set_cf(regs, 0);
}

સ્થિર અંતઃરેખ શૂન્ય
set_code_success(સંરચના bregs *regs)
{
    regs->ah = 0;
    set_cf(regs, 0);
}

સ્થિર અંતઃરેખ શૂન્ય
set_invalid_silent(સંરચના bregs *regs)
{
    set_cf(regs, 1);
}

સ્થિર અંતઃરેખ શૂન્ય
set_code_invalid_silent(સંરચના bregs *regs, u8 code)
{
    regs->ah = code;
    set_cf(regs, 1);
}

#endif // bregs.h
