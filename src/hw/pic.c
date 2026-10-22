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
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ inb(ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ) | (inb(ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ) << 8);
}

ಶೂನ್ಯ
pic_irqmask_write(u16 ಮುಖವಾಡ)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    outb(ಮುಖವಾಡ, ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ);
    outb(ಮುಖವಾಡ >> 8, ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ);
}

ಶೂನ್ಯ
pic_irqmask_ಮುಖವಾಡ(u16 off, u16 on)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    u8 pic1off = off, pic1on = on, pic2off = off>>8, pic2on = on>>8;
    outb((inb(ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ) & ~pic1off) | pic1on, ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ);
    outb((inb(ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ) & ~pic2off) | pic2on, ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ);
}

ಶೂನ್ಯ
pic_reset(u8 irq0, u8 irq8)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    // Send ICW1 (select OCW1 + will send ICW4)
    outb(0x11, ಪೋರ್ಟ್_PIC1_CMD);
    outb(0x11, ಪೋರ್ಟ್_PIC2_CMD);
    // Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15)
    outb(irq0, ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ);
    outb(irq8, ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ);
    // Send ICW3 (cascaded pic ids)
    outb(0x04, ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ);
    outb(0x02, ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ);
    // Send ICW4 (enable 8086 mode)
    outb(0x01, ಪೋರ್ಟ್_PIC1_ದತ್ತಾಂಶ);
    outb(0x01, ಪೋರ್ಟ್_PIC2_ದತ್ತಾಂಶ);
    // Mask all irqs (except cascaded PIC2 irq)
    pic_irqmask_write(PIC_IRQMASK_DEFAULT);
}

ಶೂನ್ಯ
pic_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    dprintf(3, "init pic\n");
    pic_reset(BIOS_HWIRQ0_ಸದಿಶ, BIOS_HWIRQ8_ಸದಿಶ);
}

ಶೂನ್ಯ
enable_hwirq(ಪೂರ್ಣಾಂಕ hwirq, ರಚನೆ segoff_s func)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    pic_irqmask_ಮುಖವಾಡ(1 << hwirq, 0);
    ಪೂರ್ಣಾಂಕ ಸದಿಶ;
    ಆದರೆ (hwirq < 8)
        ಸದಿಶ = BIOS_HWIRQ0_ಸದಿಶ + hwirq;
    ಇಲ್ಲದಿದ್ದರೆ
        ಸದಿಶ = BIOS_HWIRQ8_ಸದಿಶ + hwirq - 8;
    ಗಣ_IVT(ಸದಿಶ, func);
}

ಸ್ಥಿರ u8
pic_isr1_read(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, ಪೋರ್ಟ್_PIC1_CMD);
    ಹಿಂತಿರುಗಿ inb(ಪೋರ್ಟ್_PIC1_CMD);
}

ಸ್ಥಿರ u8
pic_isr2_read(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ 0;
    // 0x0b == select OCW1 + read ISR
    outb(0x0b, ಪೋರ್ಟ್_PIC2_CMD);
    ಹಿಂತಿರುಗಿ inb(ಪೋರ್ಟ್_PIC2_CMD);
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
