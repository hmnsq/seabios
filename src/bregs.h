// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __BREGS_H
#ವ್ಯಾಖ್ಯಾನಿಸು __BREGS_H

#ಅನ್ತರ್ಭಾವ "types.h" // u16
#ಅನ್ತರ್ಭಾವ "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು UREG(ER, R, RH, RL) ಒಕ್ಕೂಟ { u32 ER; ರಚನೆ { u16 R; u16 R ## _hi; }; ರಚನೆ { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
ರಚನೆ bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    ರಚನೆ segoff_s ಸಂಕೇತ_2;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ಗಣ_cf(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ cond)
{
    ಆದರೆ (cond)
        regs->flags |= F_CF;
    ಇಲ್ಲದಿದ್ದರೆ
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#ವ್ಯಾಖ್ಯಾನಿಸು RET_EUNSUPPORTED 0x86

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ಗಣ_success(ರಚನೆ bregs *regs)
{
    ಗಣ_cf(regs, 0);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ಗಣ_ಸಂಕೇತ_success(ರಚನೆ bregs *regs)
{
    regs->ah = 0;
    ಗಣ_cf(regs, 0);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ಗಣ_invalid_silent(ರಚನೆ bregs *regs)
{
    ಗಣ_cf(regs, 1);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ಗಣ_ಸಂಕೇತ_invalid_silent(ರಚನೆ bregs *regs, u8 ಸಂಕೇತ_2)
{
    regs->ah = ಸಂಕೇತ_2;
    ಗಣ_cf(regs, 1);
}

#ಆದರೆ_ಅಂತ್ಯ // bregs.h
