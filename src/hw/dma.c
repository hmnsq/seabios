// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "util.h" // dma_setup
#અન્તર્ભાવ "x86.h" // outb

#define PORT_DMA_ADDR_2        0x0004
#define PORT_DMA_CNT_2         0x0005
#define PORT_DMA1_MASK_REG     0x000a
#define PORT_DMA1_MODE_REG     0x000b
#define PORT_DMA1_CLEAR_FF_REG 0x000c
#define PORT_DMA1_MASTER_CLEAR 0x000d
#define PORT_DMA_PAGE_2        0x0081
#define PORT_DMA2_MASK_REG     0x00d4
#define PORT_DMA2_MODE_REG     0x00d6
#define PORT_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
પૂર્ણાંક
dma_floppy(u32 addr, પૂર્ણાંક ગણતરી, પૂર્ણાંક isWrite)
{
    // check for 64K boundary overrun
    u16 end = ગણતરી - 1;
    u32 last_addr = addr + end;
    જો ((addr >> 16) != (last_addr >> 16))
        પાછા -1;

    u8 mode_register = 0x46; // single mode, increment, autoinit disable,
    જો (isWrite)
        mode_register = 0x4a;

    outb(0x06, PORT_DMA1_MASK_REG);
    outb(0x00, PORT_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, PORT_DMA_ADDR_2);
    outb(addr>>8, PORT_DMA_ADDR_2);
    outb(0x00, PORT_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(end, PORT_DMA_CNT_2);
    outb(end>>8, PORT_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(mode_register, PORT_DMA1_MODE_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, PORT_DMA_PAGE_2);

    outb(0x02, PORT_DMA1_MASK_REG); // unmask channel 2

    પાછા 0;
}

// Reset DMA controller
શૂન્ય
dma_setup(શૂન્ય)
{
    // first reset the DMA controllers
    outb(0, PORT_DMA1_MASTER_CLEAR);
    outb(0, PORT_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, PORT_DMA2_MODE_REG);
    outb(0x00, PORT_DMA2_MASK_REG);
}
