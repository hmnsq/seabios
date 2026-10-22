// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PIC_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PIC_H

#ಅನ್ತರ್ಭಾವ "x86.h" // outb

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIC1_CMD          0x0020
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ         0x0021
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIC2_CMD          0x00a0
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ         0x00a1

// PORT_PIC1 bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು PIC1_IRQ0  (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು PIC1_IRQ1  (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PIC1_IRQ2  (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು PIC1_IRQ5  (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು PIC2_IRQ8  (1<<8)
#ವ್ಯಾಖ್ಯಾನಿಸು PIC2_IRQ12 (1<<12)
#ವ್ಯಾಖ್ಯಾನಿಸು PIC2_IRQ13 (1<<13)
#ವ್ಯಾಖ್ಯಾನಿಸು PIC2_IRQ14 (1<<14)

#ವ್ಯಾಖ್ಯಾನಿಸು PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#ವ್ಯಾಖ್ಯಾನಿಸು BIOS_HWIRQ0_ಸದಿಶ 0x08
#ವ್ಯಾಖ್ಯಾನಿಸು BIOS_HWIRQ8_ಸದಿಶ 0x70

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
pic_eoi1(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, ಪೋರ್ಟ್_PIC1_CMD);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
pic_eoi2(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, ಪೋರ್ಟ್_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(ಶೂನ್ಯ);
ಶೂನ್ಯ pic_irqmask_write(u16 ಮುಖವಾಡ);
ಶೂನ್ಯ pic_irqmask_ಮುಖವಾಡ(u16 off, u16 on);
ಶೂನ್ಯ pic_reset(u8 irq0, u8 irq8);
ಶೂನ್ಯ pic_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ enable_hwirq(ಪೂರ್ಣಾಂಕ hwirq, ರಚನೆ segoff_s func);

#ಆದರೆ_ಅಂತ್ಯ // pic.h
