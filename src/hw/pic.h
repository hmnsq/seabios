// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#اگر_غیر_تعریف_شدہ __PIC_H
#تعریف_کرو __PIC_H

#شامل "x86.h" // outb

#تعریف_کرو پورٹ_PIC1_CMD          0x0020
#تعریف_کرو پورٹ_PIC1_ڈیٹا         0x0021
#تعریف_کرو پورٹ_PIC2_CMD          0x00a0
#تعریف_کرو پورٹ_PIC2_ڈیٹا         0x00a1

// PORT_PIC1 bitdefs
#تعریف_کرو PIC1_IRQ0  (1<<0)
#تعریف_کرو PIC1_IRQ1  (1<<1)
#تعریف_کرو PIC1_IRQ2  (1<<2)
#تعریف_کرو PIC1_IRQ5  (1<<5)
#تعریف_کرو PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#تعریف_کرو PIC2_IRQ8  (1<<8)
#تعریف_کرو PIC2_IRQ12 (1<<12)
#تعریف_کرو PIC2_IRQ13 (1<<13)
#تعریف_کرو PIC2_IRQ14 (1<<14)

#تعریف_کرو PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#تعریف_کرو BIOS_HWIRQ0_سمتیہ 0x08
#تعریف_کرو BIOS_HWIRQ8_سمتیہ 0x70

ساکن اندرونی خالی
pic_eoi1(خالی)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, پورٹ_PIC1_CMD);
}

ساکن اندرونی خالی
pic_eoi2(خالی)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, پورٹ_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(خالی);
خالی pic_irqmask_write(u16 نقاب);
خالی pic_irqmask_نقاب(u16 off, u16 on);
خالی pic_reset(u8 irq0, u8 irq8);
خالی pic_تیاری(خالی);
خالی enable_hwirq(صحیح hwirq, ساخت segoff_s func);

#اگر_ختم // pic.h
