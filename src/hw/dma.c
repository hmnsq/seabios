// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "util.h" // dma_setup
#ಅನ್ತರ್ಭಾವ "x86.h" // outb

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA_ADDR_2        0x0004
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA_CNT_2         0x0005
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA1_ಮುಖವಾಡ_REG     0x000a
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA1_ಬಹುಲಕ_REG     0x000b
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA1_CLEAR_FF_REG 0x000c
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA1_MASTER_CLEAR 0x000d
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA_ಪುಟ_2        0x0081
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA2_ಮುಖವಾಡ_REG     0x00d4
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA2_ಬಹುಲಕ_REG     0x00d6
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
ಪೂರ್ಣಾಂಕ
dma_floppy(u32 addr, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಪೂರ್ಣಾಂಕ isWrite)
{
    // check for 64K boundary overrun
    u16 ಕೊನೆ = ಎಣಿಕೆ_2 - 1;
    u32 ಕೊನೆಯ_addr = addr + ಕೊನೆ;
    ಆದರೆ ((addr >> 16) != (ಕೊನೆಯ_addr >> 16))
        ಹಿಂತಿರುಗಿ -1;

    u8 ಬಹುಲಕ_ರಿಜಿಸ್ಟರ್ = 0x46; // single mode, increment, autoinit disable,
    ಆದರೆ (isWrite)
        ಬಹುಲಕ_ರಿಜಿಸ್ಟರ್ = 0x4a;

    outb(0x06, ಪೋರ್ಟ್_DMA1_ಮುಖವಾಡ_REG);
    outb(0x00, ಪೋರ್ಟ್_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, ಪೋರ್ಟ್_DMA_ADDR_2);
    outb(addr>>8, ಪೋರ್ಟ್_DMA_ADDR_2);
    outb(0x00, ಪೋರ್ಟ್_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(ಕೊನೆ, ಪೋರ್ಟ್_DMA_CNT_2);
    outb(ಕೊನೆ>>8, ಪೋರ್ಟ್_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(ಬಹುಲಕ_ರಿಜಿಸ್ಟರ್, ಪೋರ್ಟ್_DMA1_ಬಹುಲಕ_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, ಪೋರ್ಟ್_DMA_ಪುಟ_2);

    outb(0x02, ಪೋರ್ಟ್_DMA1_ಮುಖವಾಡ_REG); // unmask channel 2

    ಹಿಂತಿರುಗಿ 0;
}

// Reset DMA controller
ಶೂನ್ಯ
dma_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    // first reset the DMA controllers
    outb(0, ಪೋರ್ಟ್_DMA1_MASTER_CLEAR);
    outb(0, ಪೋರ್ಟ್_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, ಪೋರ್ಟ್_DMA2_ಬಹುಲಕ_REG);
    outb(0x00, ಪೋರ್ಟ್_DMA2_ಮುಖವಾಡ_REG);
}
