#ifndef __MEMMAP_H
#define __MEMMAP_H

#अन्तर्भाव "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

स्थिर अन्तःस्थ u32 virt_to_phys(शून्य *v) {
    फर्काऊ (u32)v;
}
स्थिर अन्तःस्थ शून्य *memremap(u32 addr, u32 len) {
    फर्काऊ (शून्य*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define चिह्न(SYM) ({ बाह्य वर्ण SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((शून्य*)चिह्न(SYM))

#endif // memmap.h
