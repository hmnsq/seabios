#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __MALLOC_H
#ವ್ಯಾಖ್ಯಾನಿಸು __MALLOC_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// malloc.c
ಬಾಹ್ಯ ರಚನೆ zone_s Zone_ಕೆಳಗಿನ, Zone_ಮೇಲಿನ, ZoneFSeg, Zone_Tmp_ಕೆಳಗಿನ, Zone_Tmp_ಮೇಲಿನ;
u32 rom_get_max(ಶೂನ್ಯ);
u32 rom_get_ಕೊನೆಯ(ಶೂನ್ಯ);
ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom_reserve(u32 ಗಾತ್ರ_2);
ಪೂರ್ಣಾಂಕ rom_confirm(u32 ಗಾತ್ರ_2);
ಶೂನ್ಯ malloc_csm_preinit(u32 ಕೆಳಗಿನ_pmm, u32 ಕೆಳಗಿನ_pmm_ಗಾತ್ರ, u32 hi_pmm,
                        u32 hi_pmm_ಗಾತ್ರ);
ಶೂನ್ಯ malloc_preinit(ಶೂನ್ಯ);
ಬಾಹ್ಯ u32 Legacy_Ram_ಗಾತ್ರ;
ಶೂನ್ಯ malloc_init(ಶೂನ್ಯ);
ಶೂನ್ಯ malloc_prepboot(ಶೂನ್ಯ);
u32 malloc_palloc(ರಚನೆ zone_s *zone, u32 ಗಾತ್ರ_2, u32 align);
ಶೂನ್ಯ *_malloc(ರಚನೆ zone_s *zone, u32 ಗಾತ್ರ_2, u32 align);
ಪೂರ್ಣಾಂಕ malloc_pfree(u32 ದತ್ತಾಂಶ);
ಶೂನ್ಯ ಬಿಡುಗಡೆ(ಶೂನ್ಯ *ದತ್ತಾಂಶ);
u32 malloc_getspace(ರಚನೆ zone_s *zone);
ಶೂನ್ಯ malloc_sethandle(u32 ದತ್ತಾಂಶ, u32 handle);
u32 malloc_findhandle(u32 handle);

#ವ್ಯಾಖ್ಯಾನಿಸು MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#ವ್ಯಾಖ್ಯಾನಿಸು MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *malloc_ಕೆಳಗಿನ(u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_ಕೆಳಗಿನ, ಗಾತ್ರ_2, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *malloc_ಮೇಲಿನ(u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_ಮೇಲಿನ, ಗಾತ್ರ_2, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *malloc_fseg(u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&ZoneFSeg, ಗಾತ್ರ_2, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *malloc_tmplow(u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_Tmp_ಕೆಳಗಿನ, ಗಾತ್ರ_2, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *malloc_tmphigh(u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_Tmp_ಮೇಲಿನ, ಗಾತ್ರ_2, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *malloc_tmp(u32 ಗಾತ್ರ_2) {
    ಶೂನ್ಯ *ret = malloc_tmphigh(ಗಾತ್ರ_2);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    ಹಿಂತಿರುಗಿ malloc_tmplow(ಗಾತ್ರ_2);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *memalign_ಕೆಳಗಿನ(u32 align, u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_ಕೆಳಗಿನ, ಗಾತ್ರ_2, align);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *memalign_ಮೇಲಿನ(u32 align, u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_ಮೇಲಿನ, ಗಾತ್ರ_2, align);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *memalign_tmplow(u32 align, u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_Tmp_ಕೆಳಗಿನ, ಗಾತ್ರ_2, align);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *memalign_tmphigh(u32 align, u32 ಗಾತ್ರ_2) {
    ಹಿಂತಿರುಗಿ _malloc(&Zone_Tmp_ಮೇಲಿನ, ಗಾತ್ರ_2, align);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *memalign_tmp(u32 align, u32 ಗಾತ್ರ_2) {
    ಶೂನ್ಯ *ret = memalign_tmphigh(align, ಗಾತ್ರ_2);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    ಹಿಂತಿರುಗಿ memalign_tmplow(align, ಗಾತ್ರ_2);
}

#ಆದರೆ_ಅಂತ್ಯ // malloc.h
