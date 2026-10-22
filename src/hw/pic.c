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
        أرجع 0;
    أرجع inb(پورٹ_PIC1_انکڑے) | (inb(پورٹ_PIC2_انکڑے) << 8);
}

فراغ
pic_irqmask_write(u16 نقاب)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    outb(نقاب, پورٹ_PIC1_انکڑے);
    outb(نقاب >> 8, پورٹ_PIC2_انکڑے);
}

فراغ
pic_irqmask_نقاب(u16 off, u16 on)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(پورٹ_PIC1_انکڑے) & ~pic1off) | pic1on, پورٹ_PIC1_انکڑے);
    outb((inb(پورٹ_PIC2_انکڑے) & ~pic2off) | pic2on, پورٹ_PIC2_انکڑے);
}

فراغ
pic_reset(u8 irq0, u8 irq8)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, پورٹ_PIC1_CMD);
    outb(0x11, پورٹ_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, پورٹ_PIC1_انکڑے);
    outb(irq8, پورٹ_PIC2_انکڑے);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, پورٹ_PIC1_انکڑے);
    outb(0x02, پورٹ_PIC2_انکڑے);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, پورٹ_PIC1_انکڑے);
    outb(0x01, پورٹ_PIC2_انکڑے);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

فراغ
pic_تیاری(فراغ)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_سمتیہ, BIOS_HWIRQ8_سمتیہ);
}

فراغ
enable_hwirq(صحيح hwirq, بنية segoff_s func)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    pic_irqmask_نقاب(1 << hwirq, 0);
    صحيح سمتیہ;
    إذا (hwirq < 8)
        سمتیہ = BIOS_HWIRQ0_سمتیہ + hwirq;
    وإلا
        سمتیہ = BIOS_HWIRQ8_سمتیہ + hwirq - 8;
    سیٹ_IVT(سمتیہ, func);
}

ساكن u8
pic_isr1_read(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, پورٹ_PIC1_CMD);
    أرجع inb(پورٹ_PIC1_CMD);
}

ساكن u8
pic_isr2_read(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, پورٹ_PIC2_CMD);
    أرجع inb(پورٹ_PIC2_CMD);
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
