// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // SET_IVT
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pic.h" // pic_*

u16
pic_irqmask_read(శూన్య)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి 0;
    తిరిగి inb(పోర్టు_PIC1_దత్తాంశం) | (inb(పోర్టు_PIC2_దత్తాంశం) << 8);
}

శూన్య
pic_irqmask_write(u16 ముసుగు)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి;
    outb(ముసుగు, పోర్టు_PIC1_దత్తాంశం);
    outb(ముసుగు >> 8, పోర్టు_PIC2_దత్తాంశం);
}

శూన్య
pic_irqmask_ముసుగు(u16 off, u16 on)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(పోర్టు_PIC1_దత్తాంశం) & ~pic1off) | pic1on, పోర్టు_PIC1_దత్తాంశం);
    outb((inb(పోర్టు_PIC2_దత్తాంశం) & ~pic2off) | pic2on, పోర్టు_PIC2_దత్తాంశం);
}

శూన్య
pic_reset(u8 irq0, u8 irq8)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, పోర్టు_PIC1_CMD);
    outb(0x11, పోర్టు_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, పోర్టు_PIC1_దత్తాంశం);
    outb(irq8, పోర్టు_PIC2_దత్తాంశం);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, పోర్టు_PIC1_దత్తాంశం);
    outb(0x02, పోర్టు_PIC2_దత్తాంశం);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, పోర్టు_PIC1_దత్తాంశం);
    outb(0x01, పోర్టు_PIC2_దత్తాంశం);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

శూన్య
pic_అమరిక(శూన్య)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_సదిశ, BIOS_HWIRQ8_సదిశ);
}

శూన్య
enable_hwirq(పూర్ణాంకం hwirq, నిర్మాణం segoff_s func)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి;
    pic_irqmask_ముసుగు(1 << hwirq, 0);
    పూర్ణాంకం సదిశ;
    ఒకవేళ (hwirq < 8)
        సదిశ = BIOS_HWIRQ0_సదిశ + hwirq;
    లేకపోతే
        సదిశ = BIOS_HWIRQ8_సదిశ + hwirq - 8;
    సమితి_IVT(సదిశ, func);
}

స్థిర u8
pic_isr1_read(శూన్య)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, పోర్టు_PIC1_CMD);
    తిరిగి inb(పోర్టు_PIC1_CMD);
}

స్థిర u8
pic_isr2_read(శూన్య)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, పోర్టు_PIC2_CMD);
    తిరిగి inb(పోర్టు_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
శూన్య VISIBLE16
handle_hwpic1(శూన్య)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

శూన్య VISIBLE16
handle_hwpic2(శూన్య)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
