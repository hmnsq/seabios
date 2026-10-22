#ifndef __MALLOC_H
#define __MALLOC_H

#अन्तर्भाव "types.h" // u32

// malloc.c
बाह्य संरचना zone_s Zone_निचला, Zone_ऊपरी, ZoneFSeg, Zone_Tmp_निचला, Zone_Tmp_ऊपरी;
u32 rom_get_max(शून्य);
u32 rom_get_last(शून्य);
संरचना rom_header *rom_reserve(u32 size);
पूर्णाङ्क rom_confirm(u32 size);
शून्य malloc_csm_preinit(u32 निचला_pmm, u32 निचला_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
शून्य malloc_preinit(शून्य);
बाह्य u32 LegacyRamSize;
शून्य malloc_init(शून्य);
शून्य malloc_prepboot(शून्य);
u32 malloc_palloc(संरचना zone_s *zone, u32 size, u32 align);
शून्य *_malloc(संरचना zone_s *zone, u32 size, u32 align);
पूर्णाङ्क malloc_pfree(u32 data);
शून्य मुक्त(शून्य *data);
u32 malloc_getspace(संरचना zone_s *zone);
शून्य malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
स्थिर अन्तःस्थ शून्य *malloc_निचला(u32 size) {
    प्रत्यागम _malloc(&Zone_निचला, size, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_ऊपरी(u32 size) {
    प्रत्यागम _malloc(&Zone_ऊपरी, size, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_fseg(u32 size) {
    प्रत्यागम _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_tmplow(u32 size) {
    प्रत्यागम _malloc(&Zone_Tmp_निचला, size, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_tmphigh(u32 size) {
    प्रत्यागम _malloc(&Zone_Tmp_ऊपरी, size, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_tmp(u32 size) {
    शून्य *ret = malloc_tmphigh(size);
    यदि (ret)
        प्रत्यागम ret;
    प्रत्यागम malloc_tmplow(size);
}
स्थिर अन्तःस्थ शून्य *memalign_निचला(u32 align, u32 size) {
    प्रत्यागम _malloc(&Zone_निचला, size, align);
}
स्थिर अन्तःस्थ शून्य *memalign_ऊपरी(u32 align, u32 size) {
    प्रत्यागम _malloc(&Zone_ऊपरी, size, align);
}
स्थिर अन्तःस्थ शून्य *memalign_tmplow(u32 align, u32 size) {
    प्रत्यागम _malloc(&Zone_Tmp_निचला, size, align);
}
स्थिर अन्तःस्थ शून्य *memalign_tmphigh(u32 align, u32 size) {
    प्रत्यागम _malloc(&Zone_Tmp_ऊपरी, size, align);
}
स्थिर अन्तःस्थ शून्य *memalign_tmp(u32 align, u32 size) {
    शून्य *ret = memalign_tmphigh(align, size);
    यदि (ret)
        प्रत्यागम ret;
    प्रत्यागम memalign_tmplow(align, size);
}

#endif // malloc.h
