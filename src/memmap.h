#ifndef __MEMMAP_H
#define __MEMMAP_H

#অন্তর্ভাব "types.h" // u32

// A typical OS page size
#define PAGE_আকার 4096
#define PAGE_SHIFT 12

স্থির অন্তঃরেখ u32 virt_to_phys(শূন্য *v) {
    ফেরত (u32)v;
}
স্থির অন্তঃরেখ শূন্য *memremap(u32 addr, u32 len) {
    ফেরত (শূন্য*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define প্রতীক(SYM) ({ বাহ্যিক অক্ষর SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((শূন্য*)প্রতীক(SYM))

#endif // memmap.h
