#ifndef __MALLOC_H
#define __MALLOC_H

#شامل "types.h" // u32

// malloc.c
بیرونی ساختار zone_s Zone_کم, Zone_بالا, ZoneFSeg, Zone_Tmp_کم, Zone_Tmp_بالا;
u32 rom_get_max(تهی);
u32 rom_get_last(تهی);
ساختار rom_header *rom_reserve(u32 size);
صحیح rom_confirm(u32 size);
تهی malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
تهی malloc_preinit(تهی);
بیرونی u32 LegacyRamSize;
تهی malloc_init(تهی);
تهی malloc_prepboot(تهی);
u32 malloc_palloc(ساختار zone_s *zone, u32 size, u32 align);
تهی *_malloc(ساختار zone_s *zone, u32 size, u32 align);
صحیح malloc_pfree(u32 data);
تهی آزاد(تهی *data);
u32 malloc_getspace(ساختار zone_s *zone);
تهی malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ایستا درونخطی تهی *malloc_کم(u32 size) {
    بازگشت _malloc(&Zone_کم, size, MALLOC_MIN_ALIGN);
}
ایستا درونخطی تهی *malloc_بالا(u32 size) {
    بازگشت _malloc(&Zone_بالا, size, MALLOC_MIN_ALIGN);
}
ایستا درونخطی تهی *malloc_fseg(u32 size) {
    بازگشت _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
ایستا درونخطی تهی *malloc_tmplow(u32 size) {
    بازگشت _malloc(&Zone_Tmp_کم, size, MALLOC_MIN_ALIGN);
}
ایستا درونخطی تهی *malloc_tmphigh(u32 size) {
    بازگشت _malloc(&Zone_Tmp_بالا, size, MALLOC_MIN_ALIGN);
}
ایستا درونخطی تهی *malloc_tmp(u32 size) {
    تهی *ret = malloc_tmphigh(size);
    اگر (ret)
        بازگشت ret;
    بازگشت malloc_tmplow(size);
}
ایستا درونخطی تهی *memalign_کم(u32 align, u32 size) {
    بازگشت _malloc(&Zone_کم, size, align);
}
ایستا درونخطی تهی *memalign_بالا(u32 align, u32 size) {
    بازگشت _malloc(&Zone_بالا, size, align);
}
ایستا درونخطی تهی *memalign_tmplow(u32 align, u32 size) {
    بازگشت _malloc(&Zone_Tmp_کم, size, align);
}
ایستا درونخطی تهی *memalign_tmphigh(u32 align, u32 size) {
    بازگشت _malloc(&Zone_Tmp_بالا, size, align);
}
ایستا درونخطی تهی *memalign_tmp(u32 align, u32 size) {
    تهی *ret = memalign_tmphigh(align, size);
    اگر (ret)
        بازگشت ret;
    بازگشت memalign_tmplow(align, size);
}

#endif // malloc.h
