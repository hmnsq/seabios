#यदि_अपरिभाषितम् __MEMMAP_H
#परिभाषय __MEMMAP_H

#अन्तर्भाव "types.h" // u32

// A typical OS page size
#परिभाषय PAGE_आकार 4096
#परिभाषय PAGE_सरणम् 12

स्थिर अन्तःस्थ u32 virt_गन्तव्यम्_phys(शून्य *v) {
    प्रत्यागम (u32)v;
}
स्थिर अन्तःस्थ शून्य *memremap(u32 addr, u32 len) {
    प्रत्यागम (शून्य*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#परिभाषय चिह्नम्(SYM) ({ बाह्य वर्ण SYM[]; (u32)SYM; })
#परिभाषय VSYMBOL(SYM) ((शून्य*)चिह्नम्(SYM))

#यदि_अन्तः // memmap.h
