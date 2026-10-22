// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "util.h" // dma_setup
#شامل "x86.h" // outb

#تعریف_کرو پورٹ_DMA_ADDR_2        0x0004
#تعریف_کرو پورٹ_DMA_CNT_2         0x0005
#تعریف_کرو پورٹ_DMA1_نقاب_REG     0x000a
#تعریف_کرو پورٹ_DMA1_منوال_REG     0x000b
#تعریف_کرو پورٹ_DMA1_CLEAR_FF_REG 0x000c
#تعریف_کرو پورٹ_DMA1_MASTER_CLEAR 0x000d
#تعریف_کرو پورٹ_DMA_صفحہ_2        0x0081
#تعریف_کرو پورٹ_DMA2_نقاب_REG     0x00d4
#تعریف_کرو پورٹ_DMA2_منوال_REG     0x00d6
#تعریف_کرو پورٹ_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
صحیح
dma_floppy(u32 addr, صحیح گنتی, صحیح isWrite)
{
    // check for 64K boundary overrun
    u16 انتہا = گنتی - 1;
    u32 آخری_addr = addr + انتہا;
    اگر ((addr >> 16) != (آخری_addr >> 16))
        واپس -1;

    u8 منوال_رجسٹر = 0x46; // single mode, increment, autoinit disable,
    اگر (isWrite)
        منوال_رجسٹر = 0x4a;

    outb(0x06, پورٹ_DMA1_نقاب_REG);
    outb(0x00, پورٹ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, پورٹ_DMA_ADDR_2);
    outb(addr>>8, پورٹ_DMA_ADDR_2);
    outb(0x00, پورٹ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(انتہا, پورٹ_DMA_CNT_2);
    outb(انتہا>>8, پورٹ_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(منوال_رجسٹر, پورٹ_DMA1_منوال_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, پورٹ_DMA_صفحہ_2);

    outb(0x02, پورٹ_DMA1_نقاب_REG); // unmask channel 2

    واپس 0;
}

// Reset DMA controller
خالی
dma_تیاری(خالی)
{
    // first reset the DMA controllers
    outb(0, پورٹ_DMA1_MASTER_CLEAR);
    outb(0, پورٹ_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, پورٹ_DMA2_منوال_REG);
    outb(0x00, پورٹ_DMA2_نقاب_REG);
}
