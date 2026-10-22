// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#यदि_अपरिभाषितम् __PIC_H
#परिभाषय __PIC_H

#अन्तर्भाव "x86.h" // outb

#परिभाषय पत्तनम्_PIC1_CMD          0x0020
#परिभाषय पत्तनम्_PIC1_दत्त         0x0021
#परिभाषय पत्तनम्_PIC2_CMD          0x00a0
#परिभाषय पत्तनम्_PIC2_दत्त         0x00a1

// PORT_PIC1 bitdefs
#परिभाषय PIC1_IRQ0  (1<<0)
#परिभाषय PIC1_IRQ1  (1<<1)
#परिभाषय PIC1_IRQ2  (1<<2)
#परिभाषय PIC1_IRQ5  (1<<5)
#परिभाषय PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#परिभाषय PIC2_IRQ8  (1<<8)
#परिभाषय PIC2_IRQ12 (1<<12)
#परिभाषय PIC2_IRQ13 (1<<13)
#परिभाषय PIC2_IRQ14 (1<<14)

#परिभाषय PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#परिभाषय BIOS_HWIRQ0_सदिशम् 0x08
#परिभाषय BIOS_HWIRQ8_सदिशम् 0x70

स्थिर अन्तःस्थ शून्य
pic_eoi1(शून्य)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, पत्तनम्_PIC1_CMD);
}

स्थिर अन्तःस्थ शून्य
pic_eoi2(शून्य)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, पत्तनम्_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(शून्य);
शून्य pic_irqmask_write(u16 आवरणम्);
शून्य pic_irqmask_आवरणम्(u16 off, u16 on);
शून्य pic_reset(u8 irq0, u8 irq8);
शून्य pic_सज्जता(शून्य);
शून्य enable_hwirq(पूर्णाङ्क hwirq, संरचना segoff_s func);

#यदि_अन्तः // pic.h
