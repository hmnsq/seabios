// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // SET_IVT
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pic.h" // pic_*

u16
pic_irqmask_read(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ 0;
    ਵਾਪਸ inb(ਪੋਰਟ_PIC1_ਅੰਕੜੇ) | (inb(ਪੋਰਟ_PIC2_ਅੰਕੜੇ) << 8);
}

ਖਾਲੀ
pic_irqmask_write(u16 ਨਕਾਬ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    outb(ਨਕਾਬ, ਪੋਰਟ_PIC1_ਅੰਕੜੇ);
    outb(ਨਕਾਬ >> 8, ਪੋਰਟ_PIC2_ਅੰਕੜੇ);
}

ਖਾਲੀ
pic_irqmask_ਨਕਾਬ(u16 off, u16 on)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(ਪੋਰਟ_PIC1_ਅੰਕੜੇ) & ~pic1off) | pic1on, ਪੋਰਟ_PIC1_ਅੰਕੜੇ);
    outb((inb(ਪੋਰਟ_PIC2_ਅੰਕੜੇ) & ~pic2off) | pic2on, ਪੋਰਟ_PIC2_ਅੰਕੜੇ);
}

ਖਾਲੀ
pic_reset(u8 irq0, u8 irq8)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, ਪੋਰਟ_PIC1_CMD);
    outb(0x11, ਪੋਰਟ_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, ਪੋਰਟ_PIC1_ਅੰਕੜੇ);
    outb(irq8, ਪੋਰਟ_PIC2_ਅੰਕੜੇ);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, ਪੋਰਟ_PIC1_ਅੰਕੜੇ);
    outb(0x02, ਪੋਰਟ_PIC2_ਅੰਕੜੇ);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, ਪੋਰਟ_PIC1_ਅੰਕੜੇ);
    outb(0x01, ਪੋਰਟ_PIC2_ਅੰਕੜੇ);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

ਖਾਲੀ
pic_ਤਿਆਰੀ(ਖਾਲੀ)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_ਸਦਿਸ਼, BIOS_HWIRQ8_ਸਦਿਸ਼);
}

ਖਾਲੀ
enable_hwirq(ਪੂਰਨ hwirq, ਬਣਤਰ segoff_s func)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    pic_irqmask_ਨਕਾਬ(1 << hwirq, 0);
    ਪੂਰਨ ਸਦਿਸ਼;
    ਜੇ (hwirq < 8)
        ਸਦਿਸ਼ = BIOS_HWIRQ0_ਸਦਿਸ਼ + hwirq;
    ਨਹੀਂਤਾਂ
        ਸਦਿਸ਼ = BIOS_HWIRQ8_ਸਦਿਸ਼ + hwirq - 8;
    ਸੈੱਟ_IVT(ਸਦਿਸ਼, func);
}

ਸਥਿਰ u8
pic_isr1_read(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, ਪੋਰਟ_PIC1_CMD);
    ਵਾਪਸ inb(ਪੋਰਟ_PIC1_CMD);
}

ਸਥਿਰ u8
pic_isr2_read(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, ਪੋਰਟ_PIC2_CMD);
    ਵਾਪਸ inb(ਪੋਰਟ_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
ਖਾਲੀ VISIBLE16
handle_hwpic1(ਖਾਲੀ)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

ਖਾਲੀ VISIBLE16
handle_hwpic2(ਖਾਲੀ)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
