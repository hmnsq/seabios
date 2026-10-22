// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अगर_अघोषित __BREGS_H
#घोषणा __BREGS_H

#समावेश "types.h" // u16
#समावेश "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#घोषणा UREG(ER, R, RH, RL) जोड़ { u32 ER; काष्ठा { u16 R; u16 R ## _hi; }; काष्ठा { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
काष्ठा bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    काष्ठा segoff_s code;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

अटल अंतरभूत व्योम
set_cf(काष्ठा bregs *regs, पूर्णांक cond)
{
    अगर (cond)
        regs->flags |= F_CF;
    अन्यथा
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#घोषणा RET_EUNSUPPORTED 0x86

अटल अंतरभूत व्योम
set_success(काष्ठा bregs *regs)
{
    set_cf(regs, 0);
}

अटल अंतरभूत व्योम
set_code_success(काष्ठा bregs *regs)
{
    regs->ah = 0;
    set_cf(regs, 0);
}

अटल अंतरभूत व्योम
set_invalid_silent(काष्ठा bregs *regs)
{
    set_cf(regs, 1);
}

अटल अंतरभूत व्योम
set_code_invalid_silent(काष्ठा bregs *regs, u8 code)
{
    regs->ah = code;
    set_cf(regs, 1);
}

#पूर्ण_अगर // bregs.h
