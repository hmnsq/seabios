#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __MALLOC_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __MALLOC_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

// malloc.c
ਬਾਹਰੀ ਬਣਤਰ zone_s Zone_ਨੀਵਾਂ, Zone_ਉੱਚਾ, ZoneFSeg, Zone_Tmp_ਨੀਵਾਂ, Zone_Tmp_ਉੱਚਾ;
u32 rom_get_max(ਖਾਲੀ);
u32 rom_get_ਆਖਰੀ(ਖਾਲੀ);
ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom_reserve(u32 ਮਾਪ);
ਪੂਰਨ rom_confirm(u32 ਮਾਪ);
ਖਾਲੀ malloc_csm_preinit(u32 ਨੀਵਾਂ_pmm, u32 ਨੀਵਾਂ_pmm_ਮਾਪ, u32 hi_pmm,
                        u32 hi_pmm_ਮਾਪ);
ਖਾਲੀ malloc_preinit(ਖਾਲੀ);
ਬਾਹਰੀ u32 Legacy_Ram_ਮਾਪ;
ਖਾਲੀ malloc_init(ਖਾਲੀ);
ਖਾਲੀ malloc_prepboot(ਖਾਲੀ);
u32 malloc_palloc(ਬਣਤਰ zone_s *zone, u32 ਮਾਪ, u32 align);
ਖਾਲੀ *_malloc(ਬਣਤਰ zone_s *zone, u32 ਮਾਪ, u32 align);
ਪੂਰਨ malloc_pfree(u32 ਅੰਕੜੇ);
ਖਾਲੀ ਮੁਕਤ(ਖਾਲੀ *ਅੰਕੜੇ);
u32 malloc_getspace(ਬਣਤਰ zone_s *zone);
ਖਾਲੀ malloc_sethandle(u32 ਅੰਕੜੇ, u32 handle);
u32 malloc_findhandle(u32 handle);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *malloc_ਨੀਵਾਂ(u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_ਨੀਵਾਂ, ਮਾਪ, MALLOC_MIN_ALIGN);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *malloc_ਉੱਚਾ(u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_ਉੱਚਾ, ਮਾਪ, MALLOC_MIN_ALIGN);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *malloc_fseg(u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&ZoneFSeg, ਮਾਪ, MALLOC_MIN_ALIGN);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *malloc_tmplow(u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_Tmp_ਨੀਵਾਂ, ਮਾਪ, MALLOC_MIN_ALIGN);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *malloc_tmphigh(u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_Tmp_ਉੱਚਾ, ਮਾਪ, MALLOC_MIN_ALIGN);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *malloc_tmp(u32 ਮਾਪ) {
    ਖਾਲੀ *ret = malloc_tmphigh(ਮਾਪ);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    ਵਾਪਸ malloc_tmplow(ਮਾਪ);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *memalign_ਨੀਵਾਂ(u32 align, u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_ਨੀਵਾਂ, ਮਾਪ, align);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *memalign_ਉੱਚਾ(u32 align, u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_ਉੱਚਾ, ਮਾਪ, align);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *memalign_tmplow(u32 align, u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_Tmp_ਨੀਵਾਂ, ਮਾਪ, align);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *memalign_tmphigh(u32 align, u32 ਮਾਪ) {
    ਵਾਪਸ _malloc(&Zone_Tmp_ਉੱਚਾ, ਮਾਪ, align);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *memalign_tmp(u32 align, u32 ਮਾਪ) {
    ਖਾਲੀ *ret = memalign_tmphigh(align, ਮਾਪ);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    ਵਾਪਸ memalign_tmplow(align, ਮਾਪ);
}

#ਜੇ_ਅੰਤ // malloc.h
