#جيڪڏهن_وصف_نه_ٿيل __MALLOC_H
#وصف_ڪريو __MALLOC_H

#ضمن "types.h" // u32

// malloc.c
خارجي بنية zone_s Zone_هيٺيون, Zone_مٿاهون, ZoneFSeg, Zone_Tmp_هيٺيون, Zone_Tmp_مٿاهون;
u32 rom_get_max(فراغ);
u32 rom_get_آخري(فراغ);
بنية rom_سرو *rom_reserve(u32 سائيز);
صحيح rom_confirm(u32 سائيز);
فراغ malloc_csm_preinit(u32 هيٺيون_pmm, u32 هيٺيون_pmm_سائيز, u32 hi_pmm,
                        u32 hi_pmm_سائيز);
فراغ malloc_preinit(فراغ);
خارجي u32 Legacy_Ram_سائيز;
فراغ malloc_init(فراغ);
فراغ malloc_prepboot(فراغ);
u32 malloc_palloc(بنية zone_s *zone, u32 سائيز, u32 align);
فراغ *_malloc(بنية zone_s *zone, u32 سائيز, u32 align);
صحيح malloc_pfree(u32 معلومات);
فراغ حرر(فراغ *معلومات);
u32 malloc_getspace(بنية zone_s *zone);
فراغ malloc_sethandle(u32 معلومات, u32 handle);
u32 malloc_findhandle(u32 handle);

#وصف_ڪريو MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#وصف_ڪريو MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ساكن مضمن فراغ *malloc_هيٺيون(u32 سائيز) {
    أرجع _malloc(&Zone_هيٺيون, سائيز, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_مٿاهون(u32 سائيز) {
    أرجع _malloc(&Zone_مٿاهون, سائيز, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_fseg(u32 سائيز) {
    أرجع _malloc(&ZoneFSeg, سائيز, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmplow(u32 سائيز) {
    أرجع _malloc(&Zone_Tmp_هيٺيون, سائيز, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmphigh(u32 سائيز) {
    أرجع _malloc(&Zone_Tmp_مٿاهون, سائيز, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmp(u32 سائيز) {
    فراغ *ret = malloc_tmphigh(سائيز);
    إذا (ret)
        أرجع ret;
    أرجع malloc_tmplow(سائيز);
}
ساكن مضمن فراغ *memalign_هيٺيون(u32 align, u32 سائيز) {
    أرجع _malloc(&Zone_هيٺيون, سائيز, align);
}
ساكن مضمن فراغ *memalign_مٿاهون(u32 align, u32 سائيز) {
    أرجع _malloc(&Zone_مٿاهون, سائيز, align);
}
ساكن مضمن فراغ *memalign_tmplow(u32 align, u32 سائيز) {
    أرجع _malloc(&Zone_Tmp_هيٺيون, سائيز, align);
}
ساكن مضمن فراغ *memalign_tmphigh(u32 align, u32 سائيز) {
    أرجع _malloc(&Zone_Tmp_مٿاهون, سائيز, align);
}
ساكن مضمن فراغ *memalign_tmp(u32 align, u32 سائيز) {
    فراغ *ret = memalign_tmphigh(align, سائيز);
    إذا (ret)
        أرجع ret;
    أرجع memalign_tmplow(align, سائيز);
}

#جيڪڏهن_پورو // malloc.h
