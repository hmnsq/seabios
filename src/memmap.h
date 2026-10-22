#جے_بنا_تعریف __MEMMAP_H
#تعریف_کرو __MEMMAP_H

#ضمن "types.h" // u32

// A typical OS page size
#تعریف_کرو ورقہ_ناپ 4096
#تعریف_کرو ورقہ_کھسکاؤ 12

ساكن مضمن u32 virt_منزل_phys(فراغ *v) {
    أرجع (u32)v;
}
ساكن مضمن فراغ *memremap(u32 addr, u32 len) {
    أرجع (فراغ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#تعریف_کرو علامت(SYM) ({ خارجي حرف SYM[]; (u32)SYM; })
#تعریف_کرو VSYMBOL(SYM) ((فراغ*)علامت(SYM))

#جے_انت // memmap.h
