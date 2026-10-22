#اگر_غیر_تعریف_شدہ __MALLOC_H
#تعریف_کرو __MALLOC_H

#شامل "types.h" // u32

// malloc.c
خارجی ساخت zone_s Zone_کم, Zone_بلند, ZoneFSeg, Zone_Tmp_کم, Zone_Tmp_بلند;
u32 rom_get_max(خالی);
u32 rom_get_آخری(خالی);
ساخت rom_سرخی *rom_reserve(u32 سائز);
صحیح rom_confirm(u32 سائز);
خالی malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_سائز, u32 hi_pmm,
                        u32 hi_pmm_سائز);
خالی malloc_preinit(خالی);
خارجی u32 Legacy_Ram_سائز;
خالی malloc_init(خالی);
خالی malloc_prepboot(خالی);
u32 malloc_palloc(ساخت zone_s *zone, u32 سائز, u32 align);
خالی *_malloc(ساخت zone_s *zone, u32 سائز, u32 align);
صحیح malloc_pfree(u32 ڈیٹا);
خالی آزاد(خالی *ڈیٹا);
u32 malloc_getspace(ساخت zone_s *zone);
خالی malloc_sethandle(u32 ڈیٹا, u32 handle);
u32 malloc_findhandle(u32 handle);

#تعریف_کرو MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#تعریف_کرو MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
ساکن اندرونی خالی *malloc_کم(u32 سائز) {
    واپس _malloc(&Zone_کم, سائز, MALLOC_MIN_ALIGN);
}
ساکن اندرونی خالی *malloc_بلند(u32 سائز) {
    واپس _malloc(&Zone_بلند, سائز, MALLOC_MIN_ALIGN);
}
ساکن اندرونی خالی *malloc_fseg(u32 سائز) {
    واپس _malloc(&ZoneFSeg, سائز, MALLOC_MIN_ALIGN);
}
ساکن اندرونی خالی *malloc_tmplow(u32 سائز) {
    واپس _malloc(&Zone_Tmp_کم, سائز, MALLOC_MIN_ALIGN);
}
ساکن اندرونی خالی *malloc_tmphigh(u32 سائز) {
    واپس _malloc(&Zone_Tmp_بلند, سائز, MALLOC_MIN_ALIGN);
}
ساکن اندرونی خالی *malloc_tmp(u32 سائز) {
    خالی *ret = malloc_tmphigh(سائز);
    اگر (ret)
        واپس ret;
    واپس malloc_tmplow(سائز);
}
ساکن اندرونی خالی *memalign_کم(u32 align, u32 سائز) {
    واپس _malloc(&Zone_کم, سائز, align);
}
ساکن اندرونی خالی *memalign_بلند(u32 align, u32 سائز) {
    واپس _malloc(&Zone_بلند, سائز, align);
}
ساکن اندرونی خالی *memalign_tmplow(u32 align, u32 سائز) {
    واپس _malloc(&Zone_Tmp_کم, سائز, align);
}
ساکن اندرونی خالی *memalign_tmphigh(u32 align, u32 سائز) {
    واپس _malloc(&Zone_Tmp_بلند, سائز, align);
}
ساکن اندرونی خالی *memalign_tmp(u32 align, u32 سائز) {
    خالی *ret = memalign_tmphigh(align, سائز);
    اگر (ret)
        واپس ret;
    واپس memalign_tmplow(align, سائز);
}

#اگر_ختم // malloc.h
