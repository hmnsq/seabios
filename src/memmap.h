#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __MEMMAP_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __MEMMAP_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

// A typical OS page size
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੰਨਾ_ਮਾਪ 4096
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੰਨਾ_ਖਿਸਕਾਓ 12

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 virt_ਮੰਜ਼ਿਲ_phys(ਖਾਲੀ *v) {
    ਵਾਪਸ (u32)v;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *memremap(u32 addr, u32 len) {
    ਵਾਪਸ (ਖਾਲੀ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਚਿੰਨ੍ਹ(SYM) ({ ਬਾਹਰੀ ਅੱਖਰ SYM[]; (u32)SYM; })
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VSYMBOL(SYM) ((ਖਾਲੀ*)ਚਿੰਨ੍ਹ(SYM))

#ਜੇ_ਅੰਤ // memmap.h
