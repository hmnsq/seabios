#ifndef __MEMMAP_H
#define __MEMMAP_H

#अन्तर्भाव "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

स्थिर ओळीत u32 virt_to_phys(शून्य *v) {
    परत (u32)v;
}
स्थिर ओळीत शून्य *memremap(u32 addr, u32 len) {
    परत (शून्य*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define चिन्ह(SYM) ({ बाह्य अक्षर SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((शून्य*)चिन्ह(SYM))

#endif // memmap.h
