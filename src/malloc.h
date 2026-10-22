#ifndef __MALLOC_H
#define __MALLOC_H

#include "types.h" // u32

// malloc.c
extern struct zone_s ZoneLow, ZoneHigh, ZoneFSeg, ZoneTmpLow, ZoneTmpHigh;
u32 rom_get_max(void);
u32 rom_get_last(void);
struct rom_header *rom_reserve(u32 size);
정수 rom_confirm(u32 size);
void malloc_csm_preinit(u32 low_pmm, u32 low_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
void malloc_preinit(void);
extern u32 LegacyRamSize;
void malloc_init(void);
void malloc_prepboot(void);
u32 malloc_palloc(struct zone_s *zone, u32 size, u32 align);
void *_malloc(struct zone_s *zone, u32 size, u32 align);
정수 malloc_pfree(u32 data);
void free(void *data);
u32 malloc_getspace(struct zone_s *zone);
void malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
static inline void *malloc_low(u32 size) {
    반환 _malloc(&ZoneLow, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_high(u32 size) {
    반환 _malloc(&ZoneHigh, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_fseg(u32 size) {
    반환 _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_tmplow(u32 size) {
    반환 _malloc(&ZoneTmpLow, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_tmphigh(u32 size) {
    반환 _malloc(&ZoneTmpHigh, size, MALLOC_MIN_ALIGN);
}
static inline void *malloc_tmp(u32 size) {
    void *ret = malloc_tmphigh(size);
    만약 (ret)
        반환 ret;
    반환 malloc_tmplow(size);
}
static inline void *memalign_low(u32 align, u32 size) {
    반환 _malloc(&ZoneLow, size, align);
}
static inline void *memalign_high(u32 align, u32 size) {
    반환 _malloc(&ZoneHigh, size, align);
}
static inline void *memalign_tmplow(u32 align, u32 size) {
    반환 _malloc(&ZoneTmpLow, size, align);
}
static inline void *memalign_tmphigh(u32 align, u32 size) {
    반환 _malloc(&ZoneTmpHigh, size, align);
}
static inline void *memalign_tmp(u32 align, u32 size) {
    void *ret = memalign_tmphigh(align, size);
    만약 (ret)
        반환 ret;
    반환 memalign_tmplow(align, size);
}

#endif // malloc.h
