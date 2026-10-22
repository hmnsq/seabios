#جے_بنا_تعریف __MALLOC_H
#تعریف_کرو __MALLOC_H

#ضمن "types.h" // u32

// malloc.c
خارجي بنية zone_s Zone_کم, Zone_بلند, ZoneFSeg, Zone_Tmp_کم, Zone_Tmp_بلند;
u32 rom_get_max(فراغ);
u32 rom_get_آخری(فراغ);
بنية rom_سرخی *rom_reserve(u32 ناپ);
صحيح rom_confirm(u32 ناپ);
فراغ malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_ناپ, u32 hi_pmm,
                        u32 hi_pmm_ناپ);
فراغ malloc_preinit(فراغ);
خارجي u32 Legacy_Ram_ناپ;
فراغ malloc_init(فراغ);
فراغ malloc_prepboot(فراغ);
u32 malloc_palloc(بنية zone_s *zone, u32 ناپ, u32 align);
فراغ *_malloc(بنية zone_s *zone, u32 ناپ, u32 align);
صحيح malloc_pfree(u32 انکڑے);
فراغ حرر(فراغ *انکڑے);
u32 malloc_getspace(بنية zone_s *zone);
فراغ malloc_sethandle(u32 انکڑے, u32 handle);
u32 malloc_findhandle(u32 handle);

#تعریف_کرو MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#تعریف_کرو MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ساكن مضمن فراغ *malloc_کم(u32 ناپ) {
    أرجع _malloc(&Zone_کم, ناپ, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_بلند(u32 ناپ) {
    أرجع _malloc(&Zone_بلند, ناپ, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_fseg(u32 ناپ) {
    أرجع _malloc(&ZoneFSeg, ناپ, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmplow(u32 ناپ) {
    أرجع _malloc(&Zone_Tmp_کم, ناپ, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmphigh(u32 ناپ) {
    أرجع _malloc(&Zone_Tmp_بلند, ناپ, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmp(u32 ناپ) {
    فراغ *ret = malloc_tmphigh(ناپ);
    إذا (ret)
        أرجع ret;
    أرجع malloc_tmplow(ناپ);
}
ساكن مضمن فراغ *memalign_کم(u32 align, u32 ناپ) {
    أرجع _malloc(&Zone_کم, ناپ, align);
}
ساكن مضمن فراغ *memalign_بلند(u32 align, u32 ناپ) {
    أرجع _malloc(&Zone_بلند, ناپ, align);
}
ساكن مضمن فراغ *memalign_tmplow(u32 align, u32 ناپ) {
    أرجع _malloc(&Zone_Tmp_کم, ناپ, align);
}
ساكن مضمن فراغ *memalign_tmphigh(u32 align, u32 ناپ) {
    أرجع _malloc(&Zone_Tmp_بلند, ناپ, align);
}
ساكن مضمن فراغ *memalign_tmp(u32 align, u32 ناپ) {
    فراغ *ret = memalign_tmphigh(align, ناپ);
    إذا (ret)
        أرجع ret;
    أرجع memalign_tmplow(align, ناپ);
}

#جے_انت // malloc.h
