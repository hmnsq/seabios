#ifndef __MEMMAP_H
#define __MEMMAP_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 virt_to_phys(ಶೂನ್ಯ *v) {
    ಪ್ರತ್ಯಾಗಮ (u32)v;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *memremap(u32 addr, u32 len) {
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define ಚಿಹ್ನೆ(SYM) ({ ಬಾಹ್ಯ ವರ್ಣ SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((ಶೂನ್ಯ*)ಚಿಹ್ನೆ(SYM))

#endif // memmap.h
