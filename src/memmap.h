#ifndef __MEMMAP_H
#define __MEMMAP_H

#ضمن "types.h" // u32

// A typical OS page size
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

ساكن مضمن u32 virt_to_phys(فراغ *v) {
    أرجع (u32)v;
}
ساكن مضمن فراغ *memremap(u32 addr, u32 len) {
    أرجع (فراغ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#define SYMBOL(SYM) ({ خارجي حرف SYM[]; (u32)SYM; })
#define VSYMBOL(SYM) ((فراغ*)SYMBOL(SYM))

#endif // memmap.h
