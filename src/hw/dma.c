// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "util.h" // dma_setup
#ضمن "x86.h" // outb

#وصف_ڪريو پورٽ_DMA_ADDR_2        0x0004
#وصف_ڪريو پورٽ_DMA_CNT_2         0x0005
#وصف_ڪريو پورٽ_DMA1_نقاب_REG     0x000a
#وصف_ڪريو پورٽ_DMA1_گهڻائي_REG     0x000b
#وصف_ڪريو پورٽ_DMA1_CLEAR_FF_REG 0x000c
#وصف_ڪريو پورٽ_DMA1_MASTER_CLEAR 0x000d
#وصف_ڪريو پورٽ_DMA_صفحو_2        0x0081
#وصف_ڪريو پورٽ_DMA2_نقاب_REG     0x00d4
#وصف_ڪريو پورٽ_DMA2_گهڻائي_REG     0x00d6
#وصف_ڪريو پورٽ_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
صحيح
dma_floppy(u32 addr, صحيح ڳڻپ, صحيح isWrite)
{
    // check for 64K boundary overrun
    u16 پڇاڙي = ڳڻپ - 1;
    u32 آخري_addr = addr + پڇاڙي;
    إذا ((addr >> 16) != (آخري_addr >> 16))
        أرجع -1;

    u8 گهڻائي_رجسٽر = 0x46; // single mode, increment, autoinit disable,
    إذا (isWrite)
        گهڻائي_رجسٽر = 0x4a;

    outb(0x06, پورٽ_DMA1_نقاب_REG);
    outb(0x00, پورٽ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, پورٽ_DMA_ADDR_2);
    outb(addr>>8, پورٽ_DMA_ADDR_2);
    outb(0x00, پورٽ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(پڇاڙي, پورٽ_DMA_CNT_2);
    outb(پڇاڙي>>8, پورٽ_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(گهڻائي_رجسٽر, پورٽ_DMA1_گهڻائي_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, پورٽ_DMA_صفحو_2);

    outb(0x02, پورٽ_DMA1_نقاب_REG); // unmask channel 2

    أرجع 0;
}

// Reset DMA controller
فراغ
dma_تياري(فراغ)
{
    // first reset the DMA controllers
    outb(0, پورٽ_DMA1_MASTER_CLEAR);
    outb(0, پورٽ_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, پورٽ_DMA2_گهڻائي_REG);
    outb(0x00, پورٽ_DMA2_نقاب_REG);
}
