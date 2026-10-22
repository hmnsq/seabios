// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __BREGS_H
#define __BREGS_H

#அந்தர்பாவ "types.h" // u16
#அந்தர்பாவ "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#define UREG(ER, R, RH, RL) ஒன்றிப்பு { u32 ER; கட்டமைப்பு { u16 R; u16 R ## _hi; }; கட்டமைப்பு { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
கட்டமைப்பு bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    கட்டமைப்பு segoff_s code;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

நிலையான உள்வரி வெறுமை
set_cf(கட்டமைப்பு bregs *regs, முழு cond)
{
    எனில் (cond)
        regs->flags |= F_CF;
    இல்லையெனில்
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#define RET_EUNSUPPORTED 0x86

நிலையான உள்வரி வெறுமை
set_success(கட்டமைப்பு bregs *regs)
{
    set_cf(regs, 0);
}

நிலையான உள்வரி வெறுமை
set_code_success(கட்டமைப்பு bregs *regs)
{
    regs->ah = 0;
    set_cf(regs, 0);
}

நிலையான உள்வரி வெறுமை
set_invalid_silent(கட்டமைப்பு bregs *regs)
{
    set_cf(regs, 1);
}

நிலையான உள்வரி வெறுமை
set_code_invalid_silent(கட்டமைப்பு bregs *regs, u8 code)
{
    regs->ah = code;
    set_cf(regs, 1);
}

#endif // bregs.h
