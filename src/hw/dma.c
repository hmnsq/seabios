// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "util.h" // dma_setup
#ضمن "x86.h" // outb

#تعریف_کړه پورټ_DMA_ADDR_2        0x0004
#تعریف_کړه پورټ_DMA_CNT_2         0x0005
#تعریف_کړه پورټ_DMA1_ماسک_REG     0x000a
#تعریف_کړه پورټ_DMA1_ډېرپېښ_REG     0x000b
#تعریف_کړه پورټ_DMA1_CLEAR_FF_REG 0x000c
#تعریف_کړه پورټ_DMA1_MASTER_CLEAR 0x000d
#تعریف_کړه پورټ_DMA_پاڼه_2        0x0081
#تعریف_کړه پورټ_DMA2_ماسک_REG     0x00d4
#تعریف_کړه پورټ_DMA2_ډېرپېښ_REG     0x00d6
#تعریف_کړه پورټ_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
صحيح
dma_floppy(u32 addr, صحيح شمېر, صحيح isWrite)
{
    // check for 64K boundary overrun
    u16 پای_ټکی = شمېر - 1;
    u32 وروستی_addr = addr + پای_ټکی;
    إذا ((addr >> 16) != (وروستی_addr >> 16))
        أرجع -1;

    u8 ډېرپېښ_ثبتوونکی = 0x46; // single mode, increment, autoinit disable,
    إذا (isWrite)
        ډېرپېښ_ثبتوونکی = 0x4a;

    outb(0x06, پورټ_DMA1_ماسک_REG);
    outb(0x00, پورټ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, پورټ_DMA_ADDR_2);
    outb(addr>>8, پورټ_DMA_ADDR_2);
    outb(0x00, پورټ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(پای_ټکی, پورټ_DMA_CNT_2);
    outb(پای_ټکی>>8, پورټ_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(ډېرپېښ_ثبتوونکی, پورټ_DMA1_ډېرپېښ_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, پورټ_DMA_پاڼه_2);

    outb(0x02, پورټ_DMA1_ماسک_REG); // unmask channel 2

    أرجع 0;
}

// Reset DMA controller
فراغ
dma_تیاری(فراغ)
{
    // first reset the DMA controllers
    outb(0, پورټ_DMA1_MASTER_CLEAR);
    outb(0, پورټ_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, پورټ_DMA2_ډېرپېښ_REG);
    outb(0x00, پورټ_DMA2_ماسک_REG);
}
