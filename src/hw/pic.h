// Helpers for working with i8259 interrupt controller.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#నిర్వచితం_కాకపోతే __PIC_H
#నిర్వచించు __PIC_H

#అన్తర్భావ "x86.h" // outb

#నిర్వచించు పోర్టు_PIC1_CMD          0x0020
#నిర్వచించు పోర్టు_PIC1_దత్తాంశం         0x0021
#నిర్వచించు పోర్టు_PIC2_CMD          0x00a0
#నిర్వచించు పోర్టు_PIC2_దత్తాంశం         0x00a1

// PORT_PIC1 bitdefs
#నిర్వచించు PIC1_IRQ0  (1<<0)
#నిర్వచించు PIC1_IRQ1  (1<<1)
#నిర్వచించు PIC1_IRQ2  (1<<2)
#నిర్వచించు PIC1_IRQ5  (1<<5)
#నిర్వచించు PIC1_IRQ6  (1<<6)
// PORT_PIC2 bitdefs
#నిర్వచించు PIC2_IRQ8  (1<<8)
#నిర్వచించు PIC2_IRQ12 (1<<12)
#నిర్వచించు PIC2_IRQ13 (1<<13)
#నిర్వచించు PIC2_IRQ14 (1<<14)

#నిర్వచించు PIC_IRQMASK_DEFAULT ((u16)~PIC1_IRQ2)

#నిర్వచించు BIOS_HWIRQ0_సదిశ 0x08
#నిర్వచించు BIOS_HWIRQ8_సదిశ 0x70

స్థిర అంతఃవరుస శూన్య
pic_eoi1(శూన్య)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, పోర్టు_PIC1_CMD);
}

స్థిర అంతఃవరుస శూన్య
pic_eoi2(శూన్య)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి;
    // Send eoi (select OCW2 + eoi)
    outb(0x20, పోర్టు_PIC2_CMD);
    pic_eoi1();
}

u16 pic_irqmask_read(శూన్య);
శూన్య pic_irqmask_write(u16 ముసుగు);
శూన్య pic_irqmask_ముసుగు(u16 off, u16 on);
శూన్య pic_reset(u8 irq0, u8 irq8);
శూన్య pic_అమరిక(శూన్య);
శూన్య enable_hwirq(పూర్ణాంకం hwirq, నిర్మాణం segoff_s func);

#అయితే_ముగింపు // pic.h
