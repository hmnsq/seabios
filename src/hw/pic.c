// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // SET_IVT
#ضمن "config.h" // CONFIG_*
#ضمن "output.h" // dprintf
#ضمن "pic.h" // pic_*

u16
pic_irqmask_read(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        إرجاع 0;
    إرجاع inb(PORT_PIC1_DATA) | (inb(PORT_PIC2_DATA) << 8);
}

فراغ
pic_irqmask_write(u16 mask)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        إرجاع;
    outb(mask, PORT_PIC1_DATA);
    outb(mask >> 8, PORT_PIC2_DATA);
}

فراغ
pic_irqmask_mask(u16 off, u16 on)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        إرجاع;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(PORT_PIC1_DATA) & ~pic1off) | pic1on, PORT_PIC1_DATA);
    outb((inb(PORT_PIC2_DATA) & ~pic2off) | pic2on, PORT_PIC2_DATA);
}

فراغ
pic_reset(u8 irq0, u8 irq8)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        إرجاع;
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

فراغ
pic_setup(فراغ)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_VECTOR, BIOS_HWIRQ8_VECTOR);
}

فراغ
enable_hwirq(صحيح hwirq, بنية segoff_s func)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        إرجاع;
    pic_irqmask_mask(1 << hwirq, 0);
    صحيح vector;
    إذا (hwirq < 8)
        vector = BIOS_HWIRQ0_VECTOR + hwirq;
    وإلا
        vector = BIOS_HWIRQ8_VECTOR + hwirq - 8;
    SET_IVT(vector, func);
}

ساكن u8
pic_isr1_read(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        إرجاع 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC1_CMD);
    إرجاع inb(PORT_PIC1_CMD);
}

ساكن u8
pic_isr2_read(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        إرجاع 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC2_CMD);
    إرجاع inb(PORT_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
فراغ VISIBLE16
handle_hwpic1(فراغ)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

فراغ VISIBLE16
handle_hwpic2(فراغ)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
