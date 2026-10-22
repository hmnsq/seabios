// Code to support legacy Intel 8237 DMA chip.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "util.h" // dma_setup
#अन्तर्भाव "x86.h" // outb

#परिभाषय पत्तनम्_DMA_ADDR_2        0x0004
#परिभाषय पत्तनम्_DMA_CNT_2         0x0005
#परिभाषय पत्तनम्_DMA1_आवरणम्_REG     0x000a
#परिभाषय पत्तनम्_DMA1_बहुलकम्_REG     0x000b
#परिभाषय पत्तनम्_DMA1_CLEAR_FF_REG 0x000c
#परिभाषय पत्तनम्_DMA1_MASTER_CLEAR 0x000d
#परिभाषय पत्तनम्_DMA_PAGE_2        0x0081
#परिभाषय पत्तनम्_DMA2_आवरणम्_REG     0x00d4
#परिभाषय पत्तनम्_DMA2_बहुलकम्_REG     0x00d6
#परिभाषय पत्तनम्_DMA2_MASTER_CLEAR 0x00da

// Setup the DMA controller for a floppy transfer.
पूर्णाङ्क
dma_floppy(u32 addr, पूर्णाङ्क गणनम्, पूर्णाङ्क isWrite)
{
    // check for 64K boundary overrun
    u16 अन्तः = गणनम् - 1;
    u32 चरमः_addr = addr + अन्तः;
    यदि ((addr >> 16) != (चरमः_addr >> 16))
        प्रत्यागम -1;

    u8 बहुलकम्_पञ्जिकाकोशः = 0x46; // single mode, increment, autoinit disable,
    यदि (isWrite)
        बहुलकम्_पञ्जिकाकोशः = 0x4a;

    outb(0x06, पत्तनम्_DMA1_आवरणम्_REG);
    outb(0x00, पत्तनम्_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(addr, पत्तनम्_DMA_ADDR_2);
    outb(addr>>8, पत्तनम्_DMA_ADDR_2);
    outb(0x00, पत्तनम्_DMA1_CLEAR_FF_REG); // clear flip-flop
    outb(अन्तः, पत्तनम्_DMA_CNT_2);
    outb(अन्तः>>8, पत्तनम्_DMA_CNT_2);

    // port 0b: DMA-1 Mode Register
    // transfer type=write, channel 2
    outb(बहुलकम्_पञ्जिकाकोशः, पत्तनम्_DMA1_बहुलकम्_REG);

    // port 81: DMA-1 Page Register, channel 2
    outb(addr>>16, पत्तनम्_DMA_PAGE_2);

    outb(0x02, पत्तनम्_DMA1_आवरणम्_REG); // unmask channel 2

    प्रत्यागम 0;
}

// Reset DMA controller
शून्य
dma_सज्जता(शून्य)
{
    // first reset the DMA controllers
    outb(0, पत्तनम्_DMA1_MASTER_CLEAR);
    outb(0, पत्तनम्_DMA2_MASTER_CLEAR);

    // then initialize the DMA controllers
    outb(0xc0, पत्तनम्_DMA2_बहुलकम्_REG);
    outb(0x00, पत्तनम्_DMA2_आवरणम्_REG);
}
