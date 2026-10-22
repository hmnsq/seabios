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
    أرجع inb(پورت_PIC1_داده) | (inb(پورت_PIC2_داده) << 8);
}

فراغ
pic_irqmask_write(u16 ماسک)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    outb(ماسک, پورت_PIC1_داده);
    outb(ماسک >> 8, پورت_PIC2_داده);
}

فراغ
pic_irqmask_ماسک(u16 off, u16 on)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(پورت_PIC1_داده) & ~pic1off) | pic1on, پورت_PIC1_داده);
    outb((inb(پورت_PIC2_داده) & ~pic2off) | pic2on, پورت_PIC2_داده);
}

فراغ
pic_reset(u8 irq0, u8 irq8)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, پورت_PIC1_CMD);
    outb(0x11, پورت_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, پورت_PIC1_داده);
    outb(irq8, پورت_PIC2_داده);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, پورت_PIC1_داده);
    outb(0x02, پورت_PIC2_داده);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, پورت_PIC1_داده);
    outb(0x01, پورت_PIC2_داده);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

فراغ
pic_راهاندازی(فراغ)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_وکتور, BIOS_HWIRQ8_وکتور);
}

فراغ
enable_hwirq(صحيح hwirq, بنية segoff_s func)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    pic_irqmask_ماسک(1 << hwirq, 0);
    صحيح وکتور;
    إذا (hwirq < 8)
        وکتور = BIOS_HWIRQ0_وکتور + hwirq;
    وإلا
        وکتور = BIOS_HWIRQ8_وکتور + hwirq - 8;
    مجموعه_IVT(وکتور, func);
}

ساكن u8
pic_isr1_read(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, پورت_PIC1_CMD);
    أرجع inb(پورت_PIC1_CMD);
}

ساكن u8
pic_isr2_read(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, پورت_PIC2_CMD);
    أرجع inb(پورت_PIC2_CMD);
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
