#ifndef __MALLOC_H
#define __MALLOC_H

#अन्तर्भाव "types.h" // u32

// malloc.c
बाह्य संरचना zone_s Zone_खालचा, Zone_वरचा, ZoneFSeg, Zone_Tmp_खालचा, Zone_Tmp_वरचा;
u32 rom_get_max(शून्य);
u32 rom_get_last(शून्य);
संरचना rom_header *rom_reserve(u32 size);
पूर्णांक rom_confirm(u32 size);
शून्य malloc_csm_preinit(u32 खालचा_pmm, u32 खालचा_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
शून्य malloc_preinit(शून्य);
बाह्य u32 LegacyRamSize;
शून्य malloc_init(शून्य);
शून्य malloc_prepboot(शून्य);
u32 malloc_palloc(संरचना zone_s *zone, u32 size, u32 align);
शून्य *_malloc(संरचना zone_s *zone, u32 size, u32 align);
पूर्णांक malloc_pfree(u32 data);
शून्य मुक्त(शून्य *data);
u32 malloc_getspace(संरचना zone_s *zone);
शून्य malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
स्थिर ओळीत शून्य *malloc_खालचा(u32 size) {
    परत _malloc(&Zone_खालचा, size, MALLOC_MIN_ALIGN);
}
स्थिर ओळीत शून्य *malloc_वरचा(u32 size) {
    परत _malloc(&Zone_वरचा, size, MALLOC_MIN_ALIGN);
}
स्थिर ओळीत शून्य *malloc_fseg(u32 size) {
    परत _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
स्थिर ओळीत शून्य *malloc_tmplow(u32 size) {
    परत _malloc(&Zone_Tmp_खालचा, size, MALLOC_MIN_ALIGN);
}
स्थिर ओळीत शून्य *malloc_tmphigh(u32 size) {
    परत _malloc(&Zone_Tmp_वरचा, size, MALLOC_MIN_ALIGN);
}
स्थिर ओळीत शून्य *malloc_tmp(u32 size) {
    शून्य *ret = malloc_tmphigh(size);
    जर (ret)
        परत ret;
    परत malloc_tmplow(size);
}
स्थिर ओळीत शून्य *memalign_खालचा(u32 align, u32 size) {
    परत _malloc(&Zone_खालचा, size, align);
}
स्थिर ओळीत शून्य *memalign_वरचा(u32 align, u32 size) {
    परत _malloc(&Zone_वरचा, size, align);
}
स्थिर ओळीत शून्य *memalign_tmplow(u32 align, u32 size) {
    परत _malloc(&Zone_Tmp_खालचा, size, align);
}
स्थिर ओळीत शून्य *memalign_tmphigh(u32 align, u32 size) {
    परत _malloc(&Zone_Tmp_वरचा, size, align);
}
स्थिर ओळीत शून्य *memalign_tmp(u32 align, u32 size) {
    शून्य *ret = memalign_tmphigh(align, size);
    जर (ret)
        परत ret;
    परत memalign_tmplow(align, size);
}

#endif // malloc.h
