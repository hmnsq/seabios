// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __PIC_H
#define __PIC_H

#अन्तर्भाव "x86.h" // outb

#define PORT_PIC1_CMD          0x0020
#define PORT_PIC1_DATA         0x0021
#define PORT_PIC2_CMD          0x00a0
#define PORT_PIC2_DATA         0x00a1

// PORT_PIC1 bitdefs
#define PIC1_IRQ0  (1<<0)
#define PIC1_IRQ1  (1<<1)
#define PIC1_IRQ2  (1<<2)
#define PIC1_IRQ5  (1<<5)
#define PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#define PIC2_IRQ8  (1<<8)
#define PIC2_IRQ12 (1<<12)
#define PIC2_IRQ13 (1<<13)
#define PIC2_IRQ14 (1<<14)

#define PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#define BIOS_HWIRQ0_VECTOR 0x08
#define BIOS_HWIRQ8_VECTOR 0x70

स्थिर अन्तःस्थ शून्य
pic_eoi1(शून्य)
{
    सचे (!CONFIG_HARDWARE_IRQ)
        पटिनिवत्त;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, PORT_PIC1_CMD);
}

स्थिर अन्तःस्थ शून्य
pic_eoi2(शून्य)
{
    सचे (!CONFIG_HARDWARE_IRQ)
        पटिनिवत्त;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, PORT_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(शून्य);
शून्य pic_irqmask_write(u16 mask);
शून्य pic_irqmask_mask(u16 off, u16 on);
शून्य pic_reset(u8 irq0, u8 irq8);
शून्य pic_setup(शून्य);
शून्य enable_hwirq(पूर्णाङ्क hwirq, संरचना segoff_s func);

#endif // pic.h
