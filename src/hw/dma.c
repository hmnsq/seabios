// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "util.h" // dma_setup
#ضمن "x86.h" // outb

#تعریف_کن پورت_DMA_ADDR_2        0x0004
#تعریف_کن پورت_DMA_CNT_2         0x0005
#تعریف_کن پورت_DMA1_ماسک_REG     0x000a
#تعریف_کن پورت_DMA1_نما_REG     0x000b
#تعریف_کن پورت_DMA1_CLEAR_FF_REG 0x000c
#تعریف_کن پورت_DMA1_MASTER_CLEAR 0x000d
#تعریف_کن پورت_DMA_صفحه_2        0x0081
#تعریف_کن پورت_DMA2_ماسک_REG     0x00d4
#تعریف_کن پورت_DMA2_نما_REG     0x00d6
#تعریف_کن پورت_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
صحيح
dma_floppy(u32 addr, صحيح شمار, صحيح isWrite)
{
    // check for 64K boundary overrun
    u16 انتها = شمار - 1;
    u32 آخرین_addr = addr + انتها;
    إذا ((addr >> 16) != (آخرین_addr >> 16))
        أرجع -1;

    u8 نما_ثبات = 0x46; // single mode, increment, autoinit disable,
    إذا (isWrite)
        نما_ثبات = 0x4a;

    outb(0x06, پورت_DMA1_ماسک_REG);
    outb(0x00, پورت_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, پورت_DMA_ADDR_2);
    outb(addr>>8, پورت_DMA_ADDR_2);
    outb(0x00, پورت_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(انتها, پورت_DMA_CNT_2);
    outb(انتها>>8, پورت_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(نما_ثبات, پورت_DMA1_نما_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, پورت_DMA_صفحه_2);

    outb(0x02, پورت_DMA1_ماسک_REG); // unmask channel 2

    أرجع 0;
}

// Reset DMA controller
فراغ
dma_راهاندازی(فراغ)
{
    // first reset the DMA controllers
    outb(0, پورت_DMA1_MASTER_CLEAR);
    outb(0, پورت_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, پورت_DMA2_نما_REG);
    outb(0x00, پورت_DMA2_ماسک_REG);
}
