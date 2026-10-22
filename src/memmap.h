#اگر_تعریف_نشده __MEMMAP_H
#تعریف_کن __MEMMAP_H

#ضمن "types.h" // u32

// A typical OS page size
#تعریف_کن صفحه_اندازه 4096
#تعریف_کن صفحه_لغزش 12

ساكن مضمن u32 virt_مقصد_phys(فراغ *v) {
    أرجع (u32)v;
}
ساكن مضمن فراغ *memremap(u32 addr, u32 len) {
    أرجع (فراغ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#تعریف_کن نماد(SYM) ({ خارجي حرف SYM[]; (u32)SYM; })
#تعریف_کن VSYMBOL(SYM) ((فراغ*)نماد(SYM))

#پایان_اگر // memmap.h
