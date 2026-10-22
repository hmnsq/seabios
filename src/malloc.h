#که_نه_تعریف_شوی __MALLOC_H
#تعریف_کړه __MALLOC_H

#ضمن "types.h" // u32

// malloc.c
خارجي بنية zone_s Zone_ټیټ, Zone_لوړ, ZoneFSeg, Zone_Tmp_ټیټ, Zone_Tmp_لوړ;
u32 rom_get_max(فراغ);
u32 rom_get_وروستی(فراغ);
بنية rom_سرلیک *rom_reserve(u32 اندازه);
صحيح rom_confirm(u32 اندازه);
فراغ malloc_csm_preinit(u32 ټیټ_pmm, u32 ټیټ_pmm_اندازه, u32 hi_pmm,
                        u32 hi_pmm_اندازه);
فراغ malloc_preinit(فراغ);
خارجي u32 Legacy_Ram_اندازه;
فراغ malloc_init(فراغ);
فراغ malloc_prepboot(فراغ);
u32 malloc_palloc(بنية zone_s *zone, u32 اندازه, u32 align);
فراغ *_malloc(بنية zone_s *zone, u32 اندازه, u32 align);
صحيح malloc_pfree(u32 معلومات);
فراغ حرر(فراغ *معلومات);
u32 malloc_getspace(بنية zone_s *zone);
فراغ malloc_sethandle(u32 معلومات, u32 handle);
u32 malloc_findhandle(u32 handle);

#تعریف_کړه MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#تعریف_کړه MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ساكن مضمن فراغ *malloc_ټیټ(u32 اندازه) {
    أرجع _malloc(&Zone_ټیټ, اندازه, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_لوړ(u32 اندازه) {
    أرجع _malloc(&Zone_لوړ, اندازه, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_fseg(u32 اندازه) {
    أرجع _malloc(&ZoneFSeg, اندازه, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmplow(u32 اندازه) {
    أرجع _malloc(&Zone_Tmp_ټیټ, اندازه, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmphigh(u32 اندازه) {
    أرجع _malloc(&Zone_Tmp_لوړ, اندازه, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmp(u32 اندازه) {
    فراغ *ret = malloc_tmphigh(اندازه);
    إذا (ret)
        أرجع ret;
    أرجع malloc_tmplow(اندازه);
}
ساكن مضمن فراغ *memalign_ټیټ(u32 align, u32 اندازه) {
    أرجع _malloc(&Zone_ټیټ, اندازه, align);
}
ساكن مضمن فراغ *memalign_لوړ(u32 align, u32 اندازه) {
    أرجع _malloc(&Zone_لوړ, اندازه, align);
}
ساكن مضمن فراغ *memalign_tmplow(u32 align, u32 اندازه) {
    أرجع _malloc(&Zone_Tmp_ټیټ, اندازه, align);
}
ساكن مضمن فراغ *memalign_tmphigh(u32 align, u32 اندازه) {
    أرجع _malloc(&Zone_Tmp_لوړ, اندازه, align);
}
ساكن مضمن فراغ *memalign_tmp(u32 align, u32 اندازه) {
    فراغ *ret = memalign_tmphigh(align, اندازه);
    إذا (ret)
        أرجع ret;
    أرجع memalign_tmplow(align, اندازه);
}

#د_که_پای // malloc.h
