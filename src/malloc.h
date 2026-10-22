#ifndef __MALLOC_H
#define __MALLOC_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

// malloc.c
ବାହ୍ୟ ସଂରଚନା zone_s Zone_ତଳ, Zone_ଉପର, ZoneFSeg, Zone_Tmp_ତଳ, Zone_Tmp_ଉପର;
u32 rom_get_max(ଶୂନ୍ୟ);
u32 rom_get_last(ଶୂନ୍ୟ);
ସଂରଚନା rom_header *rom_reserve(u32 size);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା rom_confirm(u32 size);
ଶୂନ୍ୟ malloc_csm_preinit(u32 ତଳ_pmm, u32 ତଳ_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
ଶୂନ୍ୟ malloc_preinit(ଶୂନ୍ୟ);
ବାହ୍ୟ u32 LegacyRamSize;
ଶୂନ୍ୟ malloc_init(ଶୂନ୍ୟ);
ଶୂନ୍ୟ malloc_prepboot(ଶୂନ୍ୟ);
u32 malloc_palloc(ସଂରଚନା zone_s *zone, u32 size, u32 align);
ଶୂନ୍ୟ *_malloc(ସଂରଚନା zone_s *zone, u32 size, u32 align);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା malloc_pfree(u32 data);
ଶୂନ୍ୟ ମୁକ୍ତ(ଶୂନ୍ୟ *data);
u32 malloc_getspace(ସଂରଚନା zone_s *zone);
ଶୂନ୍ୟ malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *malloc_ତଳ(u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_ତଳ, size, MALLOC_MIN_ALIGN);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *malloc_ଉପର(u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_ଉପର, size, MALLOC_MIN_ALIGN);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *malloc_fseg(u32 size) {
    ଫେରନ୍ତୁ _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *malloc_tmplow(u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_Tmp_ତଳ, size, MALLOC_MIN_ALIGN);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *malloc_tmphigh(u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_Tmp_ଉପର, size, MALLOC_MIN_ALIGN);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *malloc_tmp(u32 size) {
    ଶୂନ୍ୟ *ret = malloc_tmphigh(size);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;
    ଫେରନ୍ତୁ malloc_tmplow(size);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *memalign_ତଳ(u32 align, u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_ତଳ, size, align);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *memalign_ଉପର(u32 align, u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_ଉପର, size, align);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *memalign_tmplow(u32 align, u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_Tmp_ତଳ, size, align);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *memalign_tmphigh(u32 align, u32 size) {
    ଫେରନ୍ତୁ _malloc(&Zone_Tmp_ଉପର, size, align);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *memalign_tmp(u32 align, u32 size) {
    ଶୂନ୍ୟ *ret = memalign_tmphigh(align, size);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;
    ଫେରନ୍ତୁ memalign_tmplow(align, size);
}

#endif // malloc.h
