// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // dma_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outb

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA_ADDR_2        0x0004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA_CNT_2         0x0005
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA1_ਨਕਾਬ_REG     0x000a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA1_ਬਹੁਲਕ_REG     0x000b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA1_CLEAR_FF_REG 0x000c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA1_MASTER_CLEAR 0x000d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA_ਪੰਨਾ_2        0x0081
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA2_ਨਕਾਬ_REG     0x00d4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA2_ਬਹੁਲਕ_REG     0x00d6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
ਪੂਰਨ
dma_floppy(u32 addr, ਪੂਰਨ ਗਿਣਤੀ_2, ਪੂਰਨ isWrite)
{
    // check for 64K boundary overrun
    u16 ਸਿਰਾ = ਗਿਣਤੀ_2 - 1;
    u32 ਆਖਰੀ_addr = addr + ਸਿਰਾ;
    ਜੇ ((addr >> 16) != (ਆਖਰੀ_addr >> 16))
        ਵਾਪਸ -1;

    u8 ਬਹੁਲਕ_ਰਜਿਸਟਰ = 0x46; // single mode, increment, autoinit disable,
    ਜੇ (isWrite)
        ਬਹੁਲਕ_ਰਜਿਸਟਰ = 0x4a;

    outb(0x06, ਪੋਰਟ_DMA1_ਨਕਾਬ_REG);
    outb(0x00, ਪੋਰਟ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, ਪੋਰਟ_DMA_ADDR_2);
    outb(addr>>8, ਪੋਰਟ_DMA_ADDR_2);
    outb(0x00, ਪੋਰਟ_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(ਸਿਰਾ, ਪੋਰਟ_DMA_CNT_2);
    outb(ਸਿਰਾ>>8, ਪੋਰਟ_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(ਬਹੁਲਕ_ਰਜਿਸਟਰ, ਪੋਰਟ_DMA1_ਬਹੁਲਕ_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, ਪੋਰਟ_DMA_ਪੰਨਾ_2);

    outb(0x02, ਪੋਰਟ_DMA1_ਨਕਾਬ_REG); // unmask channel 2

    ਵਾਪਸ 0;
}

// Reset DMA controller
ਖਾਲੀ
dma_ਤਿਆਰੀ(ਖਾਲੀ)
{
    // first reset the DMA controllers
    outb(0, ਪੋਰਟ_DMA1_MASTER_CLEAR);
    outb(0, ਪੋਰਟ_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, ਪੋਰਟ_DMA2_ਬਹੁਲਕ_REG);
    outb(0x00, ਪੋਰਟ_DMA2_ਨਕਾਬ_REG);
}
