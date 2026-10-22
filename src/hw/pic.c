// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // SET_IVT
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pic.h" // pic_*

u16
pic_irqmask_read(शून्य)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम 0;
    प्रत्यागम inb(पत्तनम्_PIC1_दत्त) | (inb(पत्तनम्_PIC2_दत्त) << 8);
}

शून्य
pic_irqmask_write(u16 आवरणम्)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    outb(आवरणम्, पत्तनम्_PIC1_दत्त);
    outb(आवरणम् >> 8, पत्तनम्_PIC2_दत्त);
}

शून्य
pic_irqmask_आवरणम्(u16 off, u16 on)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(पत्तनम्_PIC1_दत्त) & ~pic1off) | pic1on, पत्तनम्_PIC1_दत्त);
    outb((inb(पत्तनम्_PIC2_दत्त) & ~pic2off) | pic2on, पत्तनम्_PIC2_दत्त);
}

शून्य
pic_reset(u8 irq0, u8 irq8)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, पत्तनम्_PIC1_CMD);
    outb(0x11, पत्तनम्_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, पत्तनम्_PIC1_दत्त);
    outb(irq8, पत्तनम्_PIC2_दत्त);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, पत्तनम्_PIC1_दत्त);
    outb(0x02, पत्तनम्_PIC2_दत्त);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, पत्तनम्_PIC1_दत्त);
    outb(0x01, पत्तनम्_PIC2_दत्त);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

शून्य
pic_सज्जता(शून्य)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_सदिशम्, BIOS_HWIRQ8_सदिशम्);
}

शून्य
enable_hwirq(पूर्णाङ्क hwirq, संरचना segoff_s func)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    pic_irqmask_आवरणम्(1 << hwirq, 0);
    पूर्णाङ्क सदिशम्;
    यदि (hwirq < 8)
        सदिशम् = BIOS_HWIRQ0_सदिशम् + hwirq;
    अन्यथा
        सदिशम् = BIOS_HWIRQ8_सदिशम् + hwirq - 8;
    समुच्चयः_IVT(सदिशम्, func);
}

स्थिर u8
pic_isr1_read(शून्य)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, पत्तनम्_PIC1_CMD);
    प्रत्यागम inb(पत्तनम्_PIC1_CMD);
}

स्थिर u8
pic_isr2_read(शून्य)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, पत्तनम्_PIC2_CMD);
    प्रत्यागम inb(पत्तनम्_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
शून्य VISIBLE16
handle_hwpic1(शून्य)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

शून्य VISIBLE16
handle_hwpic2(शून्य)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
