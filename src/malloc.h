#ifndef __MALLOC_H
#define __MALLOC_H

#include "types.h" // u32

// malloc.c
extern sa_nracanaa zone_s ZoneLow, ZoneHigh, ZoneFSeg, ZoneTmpLow, ZoneTmpHigh;
u32 rom_get_max(void);
u32 rom_get_last(void);
sa_nracanaa rom_header *rom_reserve(u32 size);
puur_naa_nka rom_confirm(u32 size);
void malloc_csm_preinit(u32 low_pmm, u32 low_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
void malloc_preinit(void);
extern u32 LegacyRamSize;
void malloc_init(void);
void malloc_prepboot(void);
u32 malloc_palloc(sa_nracanaa zone_s *zone, u32 size, u32 align);
void *_malloc(sa_nracanaa zone_s *zone, u32 size, u32 align);
puur_naa_nka malloc_pfree(u32 data);
void free(void *data);
u32 malloc_getspace(sa_nracanaa zone_s *zone);
void malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
static anta_hstha void *malloc_low(u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneLow, size, MALLOC_MIN_ALIGN);
}
static anta_hstha void *malloc_high(u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneHigh, size, MALLOC_MIN_ALIGN);
}
static anta_hstha void *malloc_fseg(u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
static anta_hstha void *malloc_tmplow(u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneTmpLow, size, MALLOC_MIN_ALIGN);
}
static anta_hstha void *malloc_tmphigh(u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneTmpHigh, size, MALLOC_MIN_ALIGN);
}
static anta_hstha void *malloc_tmp(u32 size) {
    void *ret = malloc_tmphigh(size);
    ᱡᱩᱫᱤ (ret)
        ᱨᱩᱣᱟᱹᱲ ret;
    ᱨᱩᱣᱟᱹᱲ malloc_tmplow(size);
}
static anta_hstha void *memalign_low(u32 align, u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneLow, size, align);
}
static anta_hstha void *memalign_high(u32 align, u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneHigh, size, align);
}
static anta_hstha void *memalign_tmplow(u32 align, u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneTmpLow, size, align);
}
static anta_hstha void *memalign_tmphigh(u32 align, u32 size) {
    ᱨᱩᱣᱟᱹᱲ _malloc(&ZoneTmpHigh, size, align);
}
static anta_hstha void *memalign_tmp(u32 align, u32 size) {
    void *ret = memalign_tmphigh(align, size);
    ᱡᱩᱫᱤ (ret)
        ᱨᱩᱣᱟᱹᱲ ret;
    ᱨᱩᱣᱟᱹᱲ memalign_tmplow(align, size);
}

#endif // malloc.h
