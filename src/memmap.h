#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __MEMMAP_H
#ವ್ಯಾಖ್ಯಾನಿಸು __MEMMAP_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// A typical OS page size
#ವ್ಯಾಖ್ಯಾನಿಸು ಪುಟ_ಗಾತ್ರ 4096
#ವ್ಯಾಖ್ಯಾನಿಸು ಪುಟ_ಸರಿಸು 12

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 virt_ಗಮ್ಯ_phys(ಶೂನ್ಯ *v) {
    ಹಿಂತಿರುಗಿ (u32)v;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *memremap(u32 addr, u32 len) {
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)addr;
}

// Return the value of a linker script symbol (see scripts/layoutrom.py)
#ವ್ಯಾಖ್ಯಾನಿಸು ಚಿಹ್ನೆ(SYM) ({ ಬಾಹ್ಯ ಅಕ್ಷರ SYM[]; (u32)SYM; })
#ವ್ಯಾಖ್ಯಾನಿಸು VSYMBOL(SYM) ((ಶೂನ್ಯ*)ಚಿಹ್ನೆ(SYM))

#ಆದರೆ_ಅಂತ್ಯ // memmap.h
