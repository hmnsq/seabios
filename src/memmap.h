#اگر_غیر_تعریف_شدہ __MEMMAP_H
#تعریف_کرو __MEMMAP_H

#شامل "types.h" // u32

// A typical OS page size
#تعریف_کرو صفحہ_سائز 4096
#تعریف_کرو صفحہ_سرکاؤ 12

ساکن اندرونی u32 virt_مقصد_phys(خالی *v) {
    واپس (u32)v;
}
ساکن اندرونی خالی *memremap(u32 addr, u32 len) {
    واپس (خالی*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#تعریف_کرو علامت(SYM) ({ خارجی حرف SYM[]; (u32)SYM; })
#تعریف_کرو VSYMBOL(SYM) ((خالی*)علامت(SYM))

#اگر_ختم // memmap.h
