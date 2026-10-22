#ifndef __MALLOC_H
#define __MALLOC_H

#অন্তর্ভাৱ "types.h" // u32

// malloc.c
বাহ্য সংরচনা zone_s Zone_নিচেৰ, Zone_উপৰেৰ, ZoneFSeg, Zone_Tmp_নিচেৰ, Zone_Tmp_উপৰেৰ;
u32 rom_get_max(শূন্য);
u32 rom_get_last(শূন্য);
সংরচনা rom_header *rom_reserve(u32 size);
পূর্ণাঙ্ক rom_confirm(u32 size);
শূন্য malloc_csm_preinit(u32 নিচেৰ_pmm, u32 নিচেৰ_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
শূন্য malloc_preinit(শূন্য);
বাহ্য u32 LegacyRamSize;
শূন্য malloc_init(শূন্য);
শূন্য malloc_prepboot(শূন্য);
u32 malloc_palloc(সংরচনা zone_s *zone, u32 size, u32 align);
শূন্য *_malloc(সংরচনা zone_s *zone, u32 size, u32 align);
পূর্ণাঙ্ক malloc_pfree(u32 data);
শূন্য মুক্ত(শূন্য *data);
u32 malloc_getspace(সংরচনা zone_s *zone);
শূন্য malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
স্থির অন্তঃস্থ শূন্য *malloc_নিচেৰ(u32 size) {
    ঘূৰাই _malloc(&Zone_নিচেৰ, size, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃস্থ শূন্য *malloc_উপৰেৰ(u32 size) {
    ঘূৰাই _malloc(&Zone_উপৰেৰ, size, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃস্থ শূন্য *malloc_fseg(u32 size) {
    ঘূৰাই _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃস্থ শূন্য *malloc_tmplow(u32 size) {
    ঘূৰাই _malloc(&Zone_Tmp_নিচেৰ, size, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃস্থ শূন্য *malloc_tmphigh(u32 size) {
    ঘূৰাই _malloc(&Zone_Tmp_উপৰেৰ, size, MALLOC_MIN_ALIGN);
}
স্থির অন্তঃস্থ শূন্য *malloc_tmp(u32 size) {
    শূন্য *ret = malloc_tmphigh(size);
    যদি (ret)
        ঘূৰাই ret;
    ঘূৰাই malloc_tmplow(size);
}
স্থির অন্তঃস্থ শূন্য *memalign_নিচেৰ(u32 align, u32 size) {
    ঘূৰাই _malloc(&Zone_নিচেৰ, size, align);
}
স্থির অন্তঃস্থ শূন্য *memalign_উপৰেৰ(u32 align, u32 size) {
    ঘূৰাই _malloc(&Zone_উপৰেৰ, size, align);
}
স্থির অন্তঃস্থ শূন্য *memalign_tmplow(u32 align, u32 size) {
    ঘূৰাই _malloc(&Zone_Tmp_নিচেৰ, size, align);
}
স্থির অন্তঃস্থ শূন্য *memalign_tmphigh(u32 align, u32 size) {
    ঘূৰাই _malloc(&Zone_Tmp_উপৰেৰ, size, align);
}
স্থির অন্তঃস্থ শূন্য *memalign_tmp(u32 align, u32 size) {
    শূন্য *ret = memalign_tmphigh(align, size);
    যদি (ret)
        ঘূৰাই ret;
    ঘূৰাই memalign_tmplow(align, size);
}

#endif // malloc.h
