// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // SET_IVT
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pic.h" // pic_*

u16
pic_irqmask_read(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ inb(PORT_PIC1_DATA) | (inb(PORT_PIC2_DATA) << 8);
}

ಶೂನ್ಯ
pic_irqmask_write(u16 mask)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ;
    outb(mask, PORT_PIC1_DATA);
    outb(mask >> 8, PORT_PIC2_DATA);
}

ಶೂನ್ಯ
pic_irqmask_mask(u16 off, u16 on)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(PORT_PIC1_DATA) & ~pic1off) | pic1on, PORT_PIC1_DATA);
    outb((inb(PORT_PIC2_DATA) & ~pic2off) | pic2on, PORT_PIC2_DATA);
}

ಶೂನ್ಯ
pic_reset(u8 irq0, u8 irq8)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ;
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

ಶೂನ್ಯ
pic_setup(ಶೂನ್ಯ)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_VECTOR, BIOS_HWIRQ8_VECTOR);
}

ಶೂನ್ಯ
enable_hwirq(ಪೂರ್ಣಾಙ್ಕ hwirq, ಸಂರಚನಾ segoff_s func)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ;
    pic_irqmask_mask(1 << hwirq, 0);
    ಪೂರ್ಣಾಙ್ಕ vector;
    ಯದಿ (hwirq < 8)
        vector = BIOS_HWIRQ0_VECTOR + hwirq;
    ಅನ್ಯಥಾ
        vector = BIOS_HWIRQ8_VECTOR + hwirq - 8;
    SET_IVT(vector, func);
}

ಸ್ಥಿರ u8
pic_isr1_read(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC1_CMD);
    ಪ್ರತ್ಯಾಗಮ inb(PORT_PIC1_CMD);
}

ಸ್ಥಿರ u8
pic_isr2_read(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, PORT_PIC2_CMD);
    ಪ್ರತ್ಯಾಗಮ inb(PORT_PIC2_CMD);
}

// Handler for otherwise unused hardware irqs.
ಶೂನ್ಯ VISIBLE16
handle_hwpic1(ಶೂನ್ಯ)
{
    dprintf(DEBUG_ISR_hwpic1, "handle_hwpic1 irq=%x\n", pic_isr1_read());
    pic_eoi1();
}

ಶೂನ್ಯ VISIBLE16
handle_hwpic2(ಶೂನ್ಯ)
{
    dprintf(DEBUG_ISR_hwpic2, "handle_hwpic2 irq=%x\n", pic_isr2_read());
    pic_eoi2();
}
