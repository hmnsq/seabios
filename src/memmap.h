#جيڪڏهن_وصف_نه_ٿيل __MEMMAP_H
#وصف_ڪريو __MEMMAP_H

#ضمن "types.h" // u32

// A typical OS page size
#وصف_ڪريو صفحو_سائيز 4096
#وصف_ڪريو صفحو_سرڪايو 12

ساكن مضمن u32 virt_منزل_phys(فراغ *v) {
    أرجع (u32)v;
}
ساكن مضمن فراغ *memremap(u32 addr, u32 len) {
    أرجع (فراغ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#وصف_ڪريو نشان(SYM) ({ خارجي حرف SYM[]; (u32)SYM; })
#وصف_ڪريو VSYMBOL(SYM) ((فراغ*)نشان(SYM))

#جيڪڏهن_پورو // memmap.h
