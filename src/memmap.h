#ifndef __MEMMAP_H
#define __MEMMAP_H

#شامل "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

ایستا درونخطی u32 virt_to_phys(تهی *v) {
    بازگشت (u32)v;
}
ایستا درونخطی تهی *memremap(u32 addr, u32 len) {
    بازگشت (تهی*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define نماد(SYM) ({ بیرونی نویسه SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((تهی*)نماد(SYM))

#endif // memmap.h
