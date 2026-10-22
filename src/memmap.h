#ifndef __MEMMAP_H
#define __MEMMAP_H

#அந்தர்பாவ "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

நிலையான உள்வரி u32 virt_to_phys(வெறுமை *v) {
    திரும்பு (u32)v;
}
நிலையான உள்வரி வெறுமை *memremap(u32 addr, u32 len) {
    திரும்பு (வெறுமை*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define குறியீடு(SYM) ({ வெளி எழுத்து SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((வெறுமை*)குறியீடு(SYM))

#endif // memmap.h
