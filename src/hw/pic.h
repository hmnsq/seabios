// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PIC_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PIC_H

#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outb

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIC1_CMD          0x0020
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIC1_ਅੰਕੜੇ         0x0021
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIC2_CMD          0x00a0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIC2_ਅੰਕੜੇ         0x00a1

// PORT_PIC1 bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC1_IRQ0  (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC1_IRQ1  (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC1_IRQ2  (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC1_IRQ5  (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC2_IRQ8  (1<<8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC2_IRQ12 (1<<12)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC2_IRQ13 (1<<13)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC2_IRQ14 (1<<14)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BIOS_HWIRQ0_ਸਦਿਸ਼ 0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BIOS_HWIRQ8_ਸਦਿਸ਼ 0x70

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
pic_eoi1(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, ਪੋਰਟ_PIC1_CMD);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
pic_eoi2(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, ਪੋਰਟ_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(ਖਾਲੀ);
ਖਾਲੀ pic_irqmask_write(u16 ਨਕਾਬ);
ਖਾਲੀ pic_irqmask_ਨਕਾਬ(u16 off, u16 on);
ਖਾਲੀ pic_reset(u8 irq0, u8 irq8);
ਖਾਲੀ pic_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ enable_hwirq(ਪੂਰਨ hwirq, ਬਣਤਰ segoff_s func);

#ਜੇ_ਅੰਤ // pic.h
