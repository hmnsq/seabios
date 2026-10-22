#که_نه_تعریف_شوی __MEMMAP_H
#تعریف_کړه __MEMMAP_H

#ضمن "types.h" // u32

// A typical OS page size
#تعریف_کړه پاڼه_اندازه 4096
#تعریف_کړه پاڼه_ښويدنه 12

ساكن مضمن u32 virt_منزل_phys(فراغ *v) {
    أرجع (u32)v;
}
ساكن مضمن فراغ *memremap(u32 addr, u32 len) {
    أرجع (فراغ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#تعریف_کړه نښه(SYM) ({ خارجي حرف SYM[]; (u32)SYM; })
#تعریف_کړه VSYMBOL(SYM) ((فراغ*)نښه(SYM))

#د_که_پای // memmap.h
