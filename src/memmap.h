#ifndef __MEMMAP_H
#define __MEMMAP_H

#અન્તર્ભાવ "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

સ્થિર અંતઃરેખ u32 virt_to_phys(શૂન્ય *v) {
    પાછા (u32)v;
}
સ્થિર અંતઃરેખ શૂન્ય *memremap(u32 addr, u32 len) {
    પાછા (શૂન્ય*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define પ્રતીક(SYM) ({ બાહ્ય અક્ષર SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((શૂન્ય*)પ્રતીક(SYM))

#endif // memmap.h
