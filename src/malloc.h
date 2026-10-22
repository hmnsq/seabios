#यदि_अपरिभाषितम् __MALLOC_H
#परिभाषय __MALLOC_H

#अन्तर्भाव "types.h" // u32

// malloc.c
बाह्य संरचना zone_s Zone_नीचः, Zone_उच्चः, ZoneFSeg, Zone_Tmp_नीचः, Zone_Tmp_उच्चः;
u32 rom_get_max(शून्य);
u32 rom_get_चरमः(शून्य);
संरचना rom_शीर्षकम् *rom_reserve(u32 आकार);
पूर्णाङ्क rom_confirm(u32 आकार);
शून्य malloc_csm_preinit(u32 नीचः_pmm, u32 नीचः_pmm_आकार, u32 hi_pmm,
                        u32 hi_pmm_आकार);
शून्य malloc_preinit(शून्य);
बाह्य u32 Legacy_Ram_आकार;
शून्य malloc_init(शून्य);
शून्य malloc_prepboot(शून्य);
u32 malloc_palloc(संरचना zone_s *zone, u32 आकार, u32 align);
शून्य *_malloc(संरचना zone_s *zone, u32 आकार, u32 align);
पूर्णाङ्क malloc_pfree(u32 दत्त);
शून्य मुक्त(शून्य *दत्त);
u32 malloc_getspace(संरचना zone_s *zone);
शून्य malloc_sethandle(u32 दत्त, u32 handle);
u32 malloc_findhandle(u32 handle);

#परिभाषय MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#परिभाषय MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
स्थिर अन्तःस्थ शून्य *malloc_नीचः(u32 आकार) {
    प्रत्यागम _malloc(&Zone_नीचः, आकार, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_उच्चः(u32 आकार) {
    प्रत्यागम _malloc(&Zone_उच्चः, आकार, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_fseg(u32 आकार) {
    प्रत्यागम _malloc(&ZoneFSeg, आकार, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_tmplow(u32 आकार) {
    प्रत्यागम _malloc(&Zone_Tmp_नीचः, आकार, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_tmphigh(u32 आकार) {
    प्रत्यागम _malloc(&Zone_Tmp_उच्चः, आकार, MALLOC_MIN_ALIGN);
}
स्थिर अन्तःस्थ शून्य *malloc_tmp(u32 आकार) {
    शून्य *ret = malloc_tmphigh(आकार);
    यदि (ret)
        प्रत्यागम ret;
    प्रत्यागम malloc_tmplow(आकार);
}
स्थिर अन्तःस्थ शून्य *memalign_नीचः(u32 align, u32 आकार) {
    प्रत्यागम _malloc(&Zone_नीचः, आकार, align);
}
स्थिर अन्तःस्थ शून्य *memalign_उच्चः(u32 align, u32 आकार) {
    प्रत्यागम _malloc(&Zone_उच्चः, आकार, align);
}
स्थिर अन्तःस्थ शून्य *memalign_tmplow(u32 align, u32 आकार) {
    प्रत्यागम _malloc(&Zone_Tmp_नीचः, आकार, align);
}
स्थिर अन्तःस्थ शून्य *memalign_tmphigh(u32 align, u32 आकार) {
    प्रत्यागम _malloc(&Zone_Tmp_उच्चः, आकार, align);
}
स्थिर अन्तःस्थ शून्य *memalign_tmp(u32 align, u32 आकार) {
    शून्य *ret = memalign_tmphigh(align, आकार);
    यदि (ret)
        प्रत्यागम ret;
    प्रत्यागम memalign_tmplow(align, आकार);
}

#यदि_अन्तः // malloc.h
