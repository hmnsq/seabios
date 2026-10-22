#ifndef __MALLOC_H
#define __MALLOC_H

#ضمن "types.h" // u32

// malloc.c
خارجي بنية zone_s Zone_کم, Zone_بلند, ZoneFSeg, Zone_Tmp_کم, Zone_Tmp_بلند;
u32 rom_get_max(فراغ);
u32 rom_get_last(فراغ);
بنية rom_header *rom_reserve(u32 size);
صحيح rom_confirm(u32 size);
فراغ malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
فراغ malloc_preinit(فراغ);
خارجي u32 LegacyRamSize;
فراغ malloc_init(فراغ);
فراغ malloc_prepboot(فراغ);
u32 malloc_palloc(بنية zone_s *zone, u32 size, u32 align);
فراغ *_malloc(بنية zone_s *zone, u32 size, u32 align);
صحيح malloc_pfree(u32 data);
فراغ حرر(فراغ *data);
u32 malloc_getspace(بنية zone_s *zone);
فراغ malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ساكن مضمن فراغ *malloc_کم(u32 size) {
    أرجع _malloc(&Zone_کم, size, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_بلند(u32 size) {
    أرجع _malloc(&Zone_بلند, size, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_fseg(u32 size) {
    أرجع _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmplow(u32 size) {
    أرجع _malloc(&Zone_Tmp_کم, size, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmphigh(u32 size) {
    أرجع _malloc(&Zone_Tmp_بلند, size, MALLOC_MIN_ALIGN);
}
ساكن مضمن فراغ *malloc_tmp(u32 size) {
    فراغ *ret = malloc_tmphigh(size);
    إذا (ret)
        أرجع ret;
    أرجع malloc_tmplow(size);
}
ساكن مضمن فراغ *memalign_کم(u32 align, u32 size) {
    أرجع _malloc(&Zone_کم, size, align);
}
ساكن مضمن فراغ *memalign_بلند(u32 align, u32 size) {
    أرجع _malloc(&Zone_بلند, size, align);
}
ساكن مضمن فراغ *memalign_tmplow(u32 align, u32 size) {
    أرجع _malloc(&Zone_Tmp_کم, size, align);
}
ساكن مضمن فراغ *memalign_tmphigh(u32 align, u32 size) {
    أرجع _malloc(&Zone_Tmp_بلند, size, align);
}
ساكن مضمن فراغ *memalign_tmp(u32 align, u32 size) {
    فراغ *ret = memalign_tmphigh(align, size);
    إذا (ret)
        أرجع ret;
    أرجع memalign_tmplow(align, size);
}

#endif // malloc.h
