// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // SET_IVT
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pic.h" // pic_*

u16
pic_irqmask_read(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ 0;
    തിരികെ inb(PORT_PIC1_DATA) | (inb(PORT_PIC2_DATA) << 8);
}

ശൂന്യം
pic_irqmask_write(u16 mask)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ;
    outb(mask, PORT_PIC1_DATA);
    outb(mask >> 8, PORT_PIC2_DATA);
}

ശൂന്യം
pic_irqmask_mask(u16 off, u16 on)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(PORT_PIC1_DATA) & ~pic1off) | pic1on, PORT_PIC1_DATA);
    outb((inb(PORT_PIC2_DATA) & ~pic2off) | pic2on, PORT_PIC2_DATA);
}

ശൂന്യം
pic_reset(u8 irq0, u8 irq8)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, PORT_PIC1_CMD);
    outb(0x11, PORT_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, PORT_PIC1_DATA);
    outb(irq8, PORT_PIC2_DATA);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, PORT_PIC1_DATA);
    outb(0x02, PORT_PIC2_DATA);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, PORT_PIC1_DATA);
    outb(0x01, PORT_PIC2_DATA);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

ശൂന്യം
pic_setup(ശൂന്യം)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_VECTOR, BIOS_HWIRQ8_VECTOR);
}

ശൂന്യം
enable_hwirq(പൂർണ്ണസംഖ്യ hwirq, ഘടന segoff_s func)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ;
    pic_irqmask_mask(1 << hwirq, 0);
    പൂർണ്ണസംഖ്യ vector;
    എങ്കിൽ (hwirq < 8)
        vector = BIOS_HWIRQ0_VECTOR + hwirq;
    അല്ലെങ്കിൽ
        vector = BIOS_HWIRQ8_VECTOR + hwirq - 8;
    SET_IVT(vector, func);
}

സ്ഥിതം u8
pic_isr1_read(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC1_CMD);
    തിരികെ inb(PORT_PIC1_CMD);
}

സ്ഥിതം u8
pic_isr2_read(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC2_CMD);
    തിരികെ inb(PORT_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
ശൂന്യം VISIBLE16
handle_hwpic1(ശൂന്യം)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

ശൂന്യം VISIBLE16
handle_hwpic2(ശൂന്യം)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
