// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#اگر_تعریف_نشده __PIC_H
#تعریف_کن __PIC_H

#ضمن "x86.h" // outb

#تعریف_کن پورت_PIC1_CMD          0x0020
#تعریف_کن پورت_PIC1_داده         0x0021
#تعریف_کن پورت_PIC2_CMD          0x00a0
#تعریف_کن پورت_PIC2_داده         0x00a1

// PORT_PIC1 bitdefs
#تعریف_کن PIC1_IRQ0  (1<<0)
#تعریف_کن PIC1_IRQ1  (1<<1)
#تعریف_کن PIC1_IRQ2  (1<<2)
#تعریف_کن PIC1_IRQ5  (1<<5)
#تعریف_کن PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#تعریف_کن PIC2_IRQ8  (1<<8)
#تعریف_کن PIC2_IRQ12 (1<<12)
#تعریف_کن PIC2_IRQ13 (1<<13)
#تعریف_کن PIC2_IRQ14 (1<<14)

#تعریف_کن PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#تعریف_کن BIOS_HWIRQ0_وکتور 0x08
#تعریف_کن BIOS_HWIRQ8_وکتور 0x70

ساكن مضمن فراغ
pic_eoi1(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, پورت_PIC1_CMD);
}

ساكن مضمن فراغ
pic_eoi2(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, پورت_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(فراغ);
فراغ pic_irqmask_write(u16 ماسک);
فراغ pic_irqmask_ماسک(u16 off, u16 on);
فراغ pic_reset(u8 irq0, u8 irq8);
فراغ pic_راهاندازی(فراغ);
فراغ enable_hwirq(صحيح hwirq, بنية segoff_s func);

#پایان_اگر // pic.h
