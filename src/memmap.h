#ifndef __MEMMAP_H
#define __MEMMAP_H

#অন্তর্ভাৱ "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

স্থির অন্তঃস্থ u32 virt_to_phys(শূন্য *v) {
    ঘূৰাই (u32)v;
}
স্থির অন্তঃস্থ শূন্য *memremap(u32 addr, u32 len) {
    ঘূৰাই (শূন্য*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define প্ৰতীক(SYM) ({ বাহ্য ৱর্ণ SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((শূন্য*)প্ৰতীক(SYM))

#endif // memmap.h
