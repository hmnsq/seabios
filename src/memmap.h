#ifndef __MEMMAP_H
#define __MEMMAP_H

#include "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

static anta_hstha u32 virt_to_phys(void *v) {
    คืน (u32)v;
}
static anta_hstha void *memremap(u32 addr, u32 len) {
    คืน (void*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define SYMBOL(SYM) ({ extern var_na SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((void*)SYMBOL(SYM))

#endif // memmap.h
