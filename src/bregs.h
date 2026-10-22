// Structure layout of cpu registers that the bios uses.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#यदि_अपरिभाषितम् __BREGS_H
#परिभाषय __BREGS_H

#अन्तर्भाव "types.h" // u16
#अन्तर्भाव "x86.h" // F_CF


/****************************************************************
 * Registers saved/restored in romlayout.S
 ****************************************************************/

#परिभाषय UREG(ER, R, RH, RL) सङ्घ { u32 ER; संरचना { u16 R; u16 R ## _hi; }; संरचना { u8 RL; u8 RH; u8 R ## _hilo; u8 R ## _hihi; }; }

// Layout of registers passed in to irq handlers.  Note that this
// layout corresponds to code in romlayout.S - don't change it here
// without also updating the assembler code.
संरचना bregs {
    u16 ds;
    u16 es;
    UREG(edi, di, di8u, di8l);
    UREG(esi, si, si8u, si8l);
    UREG(ebp, bp, bp8u, bp8l);
    UREG(ebx, bx, bh, bl);
    UREG(edx, dx, dh, dl);
    UREG(ecx, cx, ch, cl);
    UREG(eax, ax, ah, al);
    संरचना segoff_s संहिता;
    u16 flags;
} PACKED;


/****************************************************************
 * Helper functions
 ****************************************************************/

स्थिर अन्तःस्थ शून्य
समुच्चयः_cf(संरचना bregs *regs, पूर्णाङ्क cond)
{
    यदि (cond)
        regs->flags |= F_CF;
    अन्यथा
        regs->flags &= ~F_CF;
}

// Frequently used return codes
#परिभाषय RET_EUNSUPPORTED 0x86

स्थिर अन्तःस्थ शून्य
समुच्चयः_success(संरचना bregs *regs)
{
    समुच्चयः_cf(regs, 0);
}

स्थिर अन्तःस्थ शून्य
समुच्चयः_संहिता_success(संरचना bregs *regs)
{
    regs->ah = 0;
    समुच्चयः_cf(regs, 0);
}

स्थिर अन्तःस्थ शून्य
समुच्चयः_invalid_silent(संरचना bregs *regs)
{
    समुच्चयः_cf(regs, 1);
}

स्थिर अन्तःस्थ शून्य
समुच्चयः_संहिता_invalid_silent(संरचना bregs *regs, u8 संहिता)
{
    regs->ah = संहिता;
    समुच्चयः_cf(regs, 1);
}

#यदि_अन्तः // bregs.h
