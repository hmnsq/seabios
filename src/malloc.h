#ifndef __MALLOC_H
#define __MALLOC_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// malloc.c
ಬಾಹ್ಯ ಸಂರಚನಾ zone_s Zone_ಕೆಳಗಿನ, Zone_ಮೇಲಿನ, ZoneFSeg, Zone_Tmp_ಕೆಳಗಿನ, Zone_Tmp_ಮೇಲಿನ;
u32 rom_get_max(ಶೂನ್ಯ);
u32 rom_get_last(ಶೂನ್ಯ);
ಸಂರಚನಾ rom_header *rom_reserve(u32 size);
ಪೂರ್ಣಾಙ್ಕ rom_confirm(u32 size);
ಶೂನ್ಯ malloc_csm_preinit(u32 ಕೆಳಗಿನ_pmm, u32 ಕೆಳಗಿನ_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
ಶೂನ್ಯ malloc_preinit(ಶೂನ್ಯ);
ಬಾಹ್ಯ u32 LegacyRamSize;
ಶೂನ್ಯ malloc_init(ಶೂನ್ಯ);
ಶೂನ್ಯ malloc_prepboot(ಶೂನ್ಯ);
u32 malloc_palloc(ಸಂರಚನಾ zone_s *zone, u32 size, u32 align);
ಶೂನ್ಯ *_malloc(ಸಂರಚನಾ zone_s *zone, u32 size, u32 align);
ಪೂರ್ಣಾಙ್ಕ malloc_pfree(u32 data);
ಶೂನ್ಯ ಮುಕ್ತ(ಶೂನ್ಯ *data);
u32 malloc_getspace(ಸಂರಚನಾ zone_s *zone);
ಶೂನ್ಯ malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *malloc_ಕೆಳಗಿನ(u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_ಕೆಳಗಿನ, size, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *malloc_ಮೇಲಿನ(u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_ಮೇಲಿನ, size, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *malloc_fseg(u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *malloc_tmplow(u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_Tmp_ಕೆಳಗಿನ, size, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *malloc_tmphigh(u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_Tmp_ಮೇಲಿನ, size, MALLOC_MIN_ALIGN);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *malloc_tmp(u32 size) {
    ಶೂನ್ಯ *ret = malloc_tmphigh(size);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;
    ಪ್ರತ್ಯಾಗಮ malloc_tmplow(size);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *memalign_ಕೆಳಗಿನ(u32 align, u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_ಕೆಳಗಿನ, size, align);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *memalign_ಮೇಲಿನ(u32 align, u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_ಮೇಲಿನ, size, align);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *memalign_tmplow(u32 align, u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_Tmp_ಕೆಳಗಿನ, size, align);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *memalign_tmphigh(u32 align, u32 size) {
    ಪ್ರತ್ಯಾಗಮ _malloc(&Zone_Tmp_ಮೇಲಿನ, size, align);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *memalign_tmp(u32 align, u32 size) {
    ಶೂನ್ಯ *ret = memalign_tmphigh(align, size);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;
    ಪ್ರತ್ಯಾಗಮ memalign_tmplow(align, size);
}

#endif // malloc.h
