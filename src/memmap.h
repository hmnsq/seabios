#ifndef __MEMMAP_H
#define __MEMMAP_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 virt_to_phys(ଶୂନ୍ୟ *v) {
    ଫେରନ୍ତୁ (u32)v;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *memremap(u32 addr, u32 len) {
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define ପ୍ରତୀକ(SYM) ({ ବାହ୍ୟ ଅକ୍ଷର SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((ଶୂନ୍ୟ*)ପ୍ରତୀକ(SYM))

#endif // memmap.h
