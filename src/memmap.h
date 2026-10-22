#अगर_अघोषित __MEMMAP_H
#घोषणा __MEMMAP_H

#समावेश "types.h" // u32

// A typical OS page size
#घोषणा PAGE_आकार 4096
#घोषणा PAGE_SHIFT 12

अटल अंतरभूत u32 virt_to_phys(व्योम *v) {
    वापस (u32)v;
}
अटल अंतरभूत व्योम *memremap(u32 addr, u32 len) {
    वापस (व्योम*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#घोषणा प्रतीक(SYM) ({ बाह्य अक्षर SYM[]; (u32)SYM; })
#घोषणा VSYMBOL(SYM) ((व्योम*)प्रतीक(SYM))

#पूर्ण_अगर // memmap.h
