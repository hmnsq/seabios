// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __PIC_H
#define __PIC_H

#அந்தர்பாவ "x86.h" // outb

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

நிலையான உள்வரி வெறுமை
pic_eoi1(வெறுமை)
{
    எனில் (!CONFIG_HARDWARE_IRQ)
        திரும்பு;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, PORT_PIC1_CMD);
}

நிலையான உள்வரி வெறுமை
pic_eoi2(வெறுமை)
{
    எனில் (!CONFIG_HARDWARE_IRQ)
        திரும்பு;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, PORT_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(வெறுமை);
வெறுமை pic_irqmask_write(u16 mask);
வெறுமை pic_irqmask_mask(u16 off, u16 on);
வெறுமை pic_reset(u8 irq0, u8 irq8);
வெறுமை pic_setup(வெறுமை);
வெறுமை enable_hwirq(முழு hwirq, கட்டமைப்பு segoff_s func);

#endif // pic.h
