#ifndef __MALLOC_H
#define __MALLOC_H

#include "types.h" // u32

// malloc.c
extern struct zone_s Zone_माबाबा, Zone_माताअस, ZoneFSeg, Zone_Tmp_माबाबा, Zone_Tmp_माताअस;
u32 rom_get_max(void);
u32 rom_get_last(void);
struct rom_header *rom_reserve(u32 size);
int rom_confirm(u32 size);
void malloc_csm_preinit(u32 माबाबा_pmm, u32 माबाबा_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
void malloc_preinit(void);
extern u32 LegacyRamSize;
void malloc_init(void);
void malloc_prepboot(void);
u32 malloc_palloc(struct zone_s *zone, u32 size, u32 align);
void *_malloc(struct zone_s *zone, u32 size, u32 align);
int malloc_pfree(u32 data);
void free(void *data);
u32 malloc_getspace(struct zone_s *zone);
void malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
static inline void *malloc_माबाबा(u32 size) {
    return _malloc(&Zone_माबाबा, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_माताअस(u32 size) {
    return _malloc(&Zone_माताअस, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_fseg(u32 size) {
    return _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_tmplow(u32 size) {
    return _malloc(&Zone_Tmp_माबाबा, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_tmphigh(u32 size) {
    return _malloc(&Zone_Tmp_माताअस, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_tmp(u32 size) {
    void *ret = malloc_tmphigh(size);
    if (ret)
        return ret;
    return malloc_tmplow(size);
}
static inline void *memalign_माबाबा(u32 align, u32 size) {
    return _malloc(&Zone_माबाबा, size, align);
}
static inline void *memalign_माताअस(u32 align, u32 size) {
    return _malloc(&Zone_माताअस, size, align);
}
static inline void *memalign_tmplow(u32 align, u32 size) {
    return _malloc(&Zone_Tmp_माबाबा, size, align);
}
static inline void *memalign_tmphigh(u32 align, u32 size) {
    return _malloc(&Zone_Tmp_माताअस, size, align);
}
static inline void *memalign_tmp(u32 align, u32 size) {
    void *ret = memalign_tmphigh(align, size);
    if (ret)
        return ret;
    return memalign_tmplow(align, size);
}

#endif // malloc.h
