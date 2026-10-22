#ifndef __MEMMAP_H
#define __MEMMAP_H

#അന്തര്ഭാവ "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

സ്ഥിതം വരിയിൽ u32 virt_to_phys(ശൂന്യം *v) {
    തിരികെ (u32)v;
}
സ്ഥിതം വരിയിൽ ശൂന്യം *memremap(u32 addr, u32 len) {
    തിരികെ (ശൂന്യം*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define ചിഹ്നം(SYM) ({ ബാഹ്യ അക്ഷരം SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((ശൂന്യം*)ചിഹ്നം(SYM))

#endif // memmap.h
