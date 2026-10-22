// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "util.h" // dma_setup
#అన్తర్భావ "x86.h" // outb

#నిర్వచించు పోర్టు_DMA_ADDR_2        0x0004
#నిర్వచించు పోర్టు_DMA_CNT_2         0x0005
#నిర్వచించు పోర్టు_DMA1_ముసుగు_REG     0x000a
#నిర్వచించు పోర్టు_DMA1_బహుళకం_REG     0x000b
#నిర్వచించు పోర్టు_DMA1_CLEAR_FF_REG 0x000c
#నిర్వచించు పోర్టు_DMA1_MASTER_CLEAR 0x000d
#నిర్వచించు పోర్టు_DMA_పుట_2        0x0081
#నిర్వచించు పోర్టు_DMA2_ముసుగు_REG     0x00d4
#నిర్వచించు పోర్టు_DMA2_బహుళకం_REG     0x00d6
#నిర్వచించు పోర్టు_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
పూర్ణాంకం
dma_floppy(u32 addr, పూర్ణాంకం లెక్క, పూర్ణాంకం isWrite)
{
    // check for 64K boundary overrun
    u16 చివర = లెక్క - 1;
    u32 చివరి_addr = addr + చివర;
    ఒకవేళ ((addr >> 16) != (చివరి_addr >> 16))
        తిరిగి -1;

    u8 బహుళకం_రిజిస్టరు = 0x46; // single mode, increment, autoinit disable,
    ఒకవేళ (isWrite)
        బహుళకం_రిజిస్టరు = 0x4a;

    outb(0x06, పోర్టు_DMA1_ముసుగు_REG);
    outb(0x00, పోర్టు_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, పోర్టు_DMA_ADDR_2);
    outb(addr>>8, పోర్టు_DMA_ADDR_2);
    outb(0x00, పోర్టు_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(చివర, పోర్టు_DMA_CNT_2);
    outb(చివర>>8, పోర్టు_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(బహుళకం_రిజిస్టరు, పోర్టు_DMA1_బహుళకం_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, పోర్టు_DMA_పుట_2);

    outb(0x02, పోర్టు_DMA1_ముసుగు_REG); // unmask channel 2

    తిరిగి 0;
}

// Reset DMA controller
శూన్య
dma_అమరిక(శూన్య)
{
    // first reset the DMA controllers
    outb(0, పోర్టు_DMA1_MASTER_CLEAR);
    outb(0, పోర్టు_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, పోర్టు_DMA2_బహుళకం_REG);
    outb(0x00, పోర్టు_DMA2_ముసుగు_REG);
}
