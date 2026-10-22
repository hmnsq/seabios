// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#جيڪڏهن_وصف_نه_ٿيل __PIC_H
#وصف_ڪريو __PIC_H

#ضمن "x86.h" // outb

#وصف_ڪريو پورٽ_PIC1_CMD          0x0020
#وصف_ڪريو پورٽ_PIC1_معلومات         0x0021
#وصف_ڪريو پورٽ_PIC2_CMD          0x00a0
#وصف_ڪريو پورٽ_PIC2_معلومات         0x00a1

// PORT_PIC1 bitdefs
#وصف_ڪريو PIC1_IRQ0  (1<<0)
#وصف_ڪريو PIC1_IRQ1  (1<<1)
#وصف_ڪريو PIC1_IRQ2  (1<<2)
#وصف_ڪريو PIC1_IRQ5  (1<<5)
#وصف_ڪريو PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#وصف_ڪريو PIC2_IRQ8  (1<<8)
#وصف_ڪريو PIC2_IRQ12 (1<<12)
#وصف_ڪريو PIC2_IRQ13 (1<<13)
#وصف_ڪريو PIC2_IRQ14 (1<<14)

#وصف_ڪريو PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#وصف_ڪريو BIOS_HWIRQ0_ويڪٽر 0x08
#وصف_ڪريو BIOS_HWIRQ8_ويڪٽر 0x70

ساكن مضمن فراغ
pic_eoi1(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, پورٽ_PIC1_CMD);
}

ساكن مضمن فراغ
pic_eoi2(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, پورٽ_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(فراغ);
فراغ pic_irqmask_write(u16 نقاب);
فراغ pic_irqmask_نقاب(u16 off, u16 on);
فراغ pic_reset(u8 irq0, u8 irq8);
فراغ pic_تياري(فراغ);
فراغ enable_hwirq(صحيح hwirq, بنية segoff_s func);

#جيڪڏهن_پورو // pic.h
