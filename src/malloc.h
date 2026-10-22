#ifndef __MALLOC_H
#define __MALLOC_H

#অন্তর্ভাব "types.h" // u32

// malloc.c
বাহ্যিক কাঠামো zone_s Zone_নিচের, Zone_উপরের, ZoneFSeg, Zone_Tmp_নিচের, Zone_Tmp_উপরের;
u32 rom_get_max(শূন্য);
u32 rom_get_last(শূন্য);
কাঠামো rom_header *rom_reserve(u32 আকার_2);
পূর্ণসংখ্যা rom_confirm(u32 আকার_2);
শূন্য malloc_csm_preinit(u32 নিচের_pmm, u32 নিচের_pmm_আকার, u32 hi_pmm,
                        u32 hi_pmm_আকার);
শূন্য malloc_preinit(শূন্য);
বাহ্যিক u32 Legacy_Ram_আকার;
শূন্য malloc_init(শূন্য);
শূন্য malloc_prepboot(শূন্য);
u32 malloc_palloc(কাঠামো zone_s *zone, u32 আকার_2, u32 align);
শূন্য *_malloc(কাঠামো zone_s *zone, u32 আকার_2, u32 align);
পূর্ণসংখ্যা malloc_pfree(u32 তথ্য);
শূন্য মুক্ত(শূন্য *তথ্য);
u32 malloc_getspace(কাঠামো zone_s *zone);
শূন্য malloc_sethandle(u32 তথ্য, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
স্থির অন্তঃরেখ শূন্য *malloc_নিচের(u32 আকার_2) {
    ফেরত _malloc(&Zone_নিচের, আকার_2, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃরেখ শূন্য *malloc_উপরের(u32 আকার_2) {
    ফেরত _malloc(&Zone_উপরের, আকার_2, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃরেখ শূন্য *malloc_fseg(u32 আকার_2) {
    ফেরত _malloc(&ZoneFSeg, আকার_2, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃরেখ শূন্য *malloc_tmplow(u32 আকার_2) {
    ফেরত _malloc(&Zone_Tmp_নিচের, আকার_2, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃরেখ শূন্য *malloc_tmphigh(u32 আকার_2) {
    ফেরত _malloc(&Zone_Tmp_উপরের, আকার_2, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃরেখ শূন্য *malloc_tmp(u32 আকার_2) {
    শূন্য *ret = malloc_tmphigh(আকার_2);
    যদি (ret)
        ফেরত ret;
    ফেরত malloc_tmplow(আকার_2);
}
স্থির অন্তঃরেখ শূন্য *memalign_নিচের(u32 align, u32 আকার_2) {
    ফেরত _malloc(&Zone_নিচের, আকার_2, align);
}
স্থির অন্তঃরেখ শূন্য *memalign_উপরের(u32 align, u32 আকার_2) {
    ফেরত _malloc(&Zone_উপরের, আকার_2, align);
}
স্থির অন্তঃরেখ শূন্য *memalign_tmplow(u32 align, u32 আকার_2) {
    ফেরত _malloc(&Zone_Tmp_নিচের, আকার_2, align);
}
স্থির অন্তঃরেখ শূন্য *memalign_tmphigh(u32 align, u32 আকার_2) {
    ফেরত _malloc(&Zone_Tmp_উপরের, আকার_2, align);
}
স্থির অন্তঃরেখ শূন্য *memalign_tmp(u32 align, u32 আকার_2) {
    শূন্য *ret = memalign_tmphigh(align, আকার_2);
    যদি (ret)
        ফেরত ret;
    ফেরত memalign_tmplow(align, আকার_2);
}

#endif // malloc.h
