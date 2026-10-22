// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // SET_IVT
#شامل "config.h" // CONFIG_*
#شامل "output.h" // dprintf
#شامل "pic.h" // pic_*

u16
pic_irqmask_read(خالی)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس 0;
    واپس inb(پورٹ_PIC1_ڈیٹا) | (inb(پورٹ_PIC2_ڈیٹا) << 8);
}

خالی
pic_irqmask_write(u16 نقاب)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس;
    outb(نقاب, پورٹ_PIC1_ڈیٹا);
    outb(نقاب >> 8, پورٹ_PIC2_ڈیٹا);
}

خالی
pic_irqmask_نقاب(u16 off, u16 on)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(پورٹ_PIC1_ڈیٹا) & ~pic1off) | pic1on, پورٹ_PIC1_ڈیٹا);
    outb((inb(پورٹ_PIC2_ڈیٹا) & ~pic2off) | pic2on, پورٹ_PIC2_ڈیٹا);
}

خالی
pic_reset(u8 irq0, u8 irq8)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, پورٹ_PIC1_CMD);
    outb(0x11, پورٹ_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, پورٹ_PIC1_ڈیٹا);
    outb(irq8, پورٹ_PIC2_ڈیٹا);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, پورٹ_PIC1_ڈیٹا);
    outb(0x02, پورٹ_PIC2_ڈیٹا);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, پورٹ_PIC1_ڈیٹا);
    outb(0x01, پورٹ_PIC2_ڈیٹا);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

خالی
pic_تیاری(خالی)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_سمتیہ, BIOS_HWIRQ8_سمتیہ);
}

خالی
enable_hwirq(صحیح hwirq, ساخت segoff_s func)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس;
    pic_irqmask_نقاب(1 << hwirq, 0);
    صحیح سمتیہ;
    اگر (hwirq < 8)
        سمتیہ = BIOS_HWIRQ0_سمتیہ + hwirq;
    ورنہ
        سمتیہ = BIOS_HWIRQ8_سمتیہ + hwirq - 8;
    سیٹ_IVT(سمتیہ, func);
}

ساکن u8
pic_isr1_read(خالی)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, پورٹ_PIC1_CMD);
    واپس inb(پورٹ_PIC1_CMD);
}

ساکن u8
pic_isr2_read(خالی)
{
    اگر (!CONFIG_HARDWARE_IRQ)
        واپس 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, پورٹ_PIC2_CMD);
    واپس inb(پورٹ_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
خالی VISIBLE16
handle_hwpic1(خالی)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

خالی VISIBLE16
handle_hwpic2(خالی)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
