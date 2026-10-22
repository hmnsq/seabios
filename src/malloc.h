#ifndef __MALLOC_H
#define __MALLOC_H

#അന്തര്ഭാവ "types.h" // u32

// malloc.c
ബാഹ്യ ഘടന zone_s Zone_താഴെയുള്ള, Zone_മുകളിലെ, ZoneFSeg, Zone_Tmp_താഴെയുള്ള, Zone_Tmp_മുകളിലെ;
u32 rom_get_max(ശൂന്യം);
u32 rom_get_last(ശൂന്യം);
ഘടന rom_header *rom_reserve(u32 size);
പൂർണ്ണസംഖ്യ rom_confirm(u32 size);
ശൂന്യം malloc_csm_preinit(u32 താഴെയുള്ള_pmm, u32 താഴെയുള്ള_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
ശൂന്യം malloc_preinit(ശൂന്യം);
ബാഹ്യ u32 LegacyRamSize;
ശൂന്യം malloc_init(ശൂന്യം);
ശൂന്യം malloc_prepboot(ശൂന്യം);
u32 malloc_palloc(ഘടന zone_s *zone, u32 size, u32 align);
ശൂന്യം *_malloc(ഘടന zone_s *zone, u32 size, u32 align);
പൂർണ്ണസംഖ്യ malloc_pfree(u32 data);
ശൂന്യം സ്വതന്ത്രം(ശൂന്യം *data);
u32 malloc_getspace(ഘടന zone_s *zone);
ശൂന്യം malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
സ്ഥിതം വരിയിൽ ശൂന്യം *malloc_താഴെയുള്ള(u32 size) {
    തിരികെ _malloc(&Zone_താഴെയുള്ള, size, MALLOC_MIN_ALIGN);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *malloc_മുകളിലെ(u32 size) {
    തിരികെ _malloc(&Zone_മുകളിലെ, size, MALLOC_MIN_ALIGN);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *malloc_fseg(u32 size) {
    തിരികെ _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *malloc_tmplow(u32 size) {
    തിരികെ _malloc(&Zone_Tmp_താഴെയുള്ള, size, MALLOC_MIN_ALIGN);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *malloc_tmphigh(u32 size) {
    തിരികെ _malloc(&Zone_Tmp_മുകളിലെ, size, MALLOC_MIN_ALIGN);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *malloc_tmp(u32 size) {
    ശൂന്യം *ret = malloc_tmphigh(size);
    എങ്കിൽ (ret)
        തിരികെ ret;
    തിരികെ malloc_tmplow(size);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *memalign_താഴെയുള്ള(u32 align, u32 size) {
    തിരികെ _malloc(&Zone_താഴെയുള്ള, size, align);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *memalign_മുകളിലെ(u32 align, u32 size) {
    തിരികെ _malloc(&Zone_മുകളിലെ, size, align);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *memalign_tmplow(u32 align, u32 size) {
    തിരികെ _malloc(&Zone_Tmp_താഴെയുള്ള, size, align);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *memalign_tmphigh(u32 align, u32 size) {
    തിരികെ _malloc(&Zone_Tmp_മുകളിലെ, size, align);
}
സ്ഥിതം വരിയിൽ ശൂന്യം *memalign_tmp(u32 align, u32 size) {
    ശൂന്യം *ret = memalign_tmphigh(align, size);
    എങ്കിൽ (ret)
        തിരികെ ret;
    തിരികെ memalign_tmplow(align, size);
}

#endif // malloc.h
