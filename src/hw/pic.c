// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // SET_IVT
#समावेश "config.h" // CONFIG_*
#समावेश "output.h" // dprintf
#समावेश "pic.h" // pic_*

u16
pic_irqmask_read(व्योम)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस 0;
    वापस inb(PORT_PIC1_तथ्य) | (inb(PORT_PIC2_तथ्य) << 8);
}

व्योम
pic_irqmask_write(u16 mask)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस;
    outb(mask, PORT_PIC1_तथ्य);
    outb(mask >> 8, PORT_PIC2_तथ्य);
}

व्योम
pic_irqmask_mask(u16 off, u16 on)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(PORT_PIC1_तथ्य) & ~pic1off) | pic1on, PORT_PIC1_तथ्य);
    outb((inb(PORT_PIC2_तथ्य) & ~pic2off) | pic2on, PORT_PIC2_तथ्य);
}

व्योम
pic_reset(u8 irq0, u8 irq8)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, PORT_PIC1_CMD);
    outb(0x11, PORT_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, PORT_PIC1_तथ्य);
    outb(irq8, PORT_PIC2_तथ्य);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, PORT_PIC1_तथ्य);
    outb(0x02, PORT_PIC2_तथ्य);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, PORT_PIC1_तथ्य);
    outb(0x01, PORT_PIC2_तथ्य);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

व्योम
pic_setup(व्योम)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_VECTOR, BIOS_HWIRQ8_VECTOR);
}

व्योम
enable_hwirq(पूर्णांक hwirq, काष्ठा segoff_s func)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस;
    pic_irqmask_mask(1 << hwirq, 0);
    पूर्णांक vector;
    अगर (hwirq < 8)
        vector = BIOS_HWIRQ0_VECTOR + hwirq;
    अन्यथा
        vector = BIOS_HWIRQ8_VECTOR + hwirq - 8;
    SET_IVT(vector, func);
}

अटल u8
pic_isr1_read(व्योम)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC1_CMD);
    वापस inb(PORT_PIC1_CMD);
}

अटल u8
pic_isr2_read(व्योम)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC2_CMD);
    वापस inb(PORT_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
व्योम VISIBLE16
handle_hwpic1(व्योम)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

व्योम VISIBLE16
handle_hwpic2(व्योम)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
