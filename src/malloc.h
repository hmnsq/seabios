#ifndef __MALLOC_H
#define __MALLOC_H

#અન્તર્ભાવ "types.h" // u32

// malloc.c
બાહ્ય સંરચના zone_s Zone_નીચલું, Zone_ઉપલું, ZoneFSeg, Zone_Tmp_નીચલું, Zone_Tmp_ઉપલું;
u32 rom_get_max(શૂન્ય);
u32 rom_get_last(શૂન્ય);
સંરચના rom_header *rom_reserve(u32 size);
પૂર્ણાંક rom_confirm(u32 size);
શૂન્ય malloc_csm_preinit(u32 નીચલું_pmm, u32 નીચલું_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
શૂન્ય malloc_preinit(શૂન્ય);
બાહ્ય u32 LegacyRamSize;
શૂન્ય malloc_init(શૂન્ય);
શૂન્ય malloc_prepboot(શૂન્ય);
u32 malloc_palloc(સંરચના zone_s *zone, u32 size, u32 align);
શૂન્ય *_malloc(સંરચના zone_s *zone, u32 size, u32 align);
પૂર્ણાંક malloc_pfree(u32 data);
શૂન્ય મુક્ત(શૂન્ય *data);
u32 malloc_getspace(સંરચના zone_s *zone);
શૂન્ય malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
સ્થિર અંતઃરેખ શૂન્ય *malloc_નીચલું(u32 size) {
    પાછા _malloc(&Zone_નીચલું, size, MALLOC_MIN_ALIGN);
}
સ્થિર અંતઃરેખ શૂન્ય *malloc_ઉપલું(u32 size) {
    પાછા _malloc(&Zone_ઉપલું, size, MALLOC_MIN_ALIGN);
}
સ્થિર અંતઃરેખ શૂન્ય *malloc_fseg(u32 size) {
    પાછા _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
સ્થિર અંતઃરેખ શૂન્ય *malloc_tmplow(u32 size) {
    પાછા _malloc(&Zone_Tmp_નીચલું, size, MALLOC_MIN_ALIGN);
}
સ્થિર અંતઃરેખ શૂન્ય *malloc_tmphigh(u32 size) {
    પાછા _malloc(&Zone_Tmp_ઉપલું, size, MALLOC_MIN_ALIGN);
}
સ્થિર અંતઃરેખ શૂન્ય *malloc_tmp(u32 size) {
    શૂન્ય *ret = malloc_tmphigh(size);
    જો (ret)
        પાછા ret;
    પાછા malloc_tmplow(size);
}
સ્થિર અંતઃરેખ શૂન્ય *memalign_નીચલું(u32 align, u32 size) {
    પાછા _malloc(&Zone_નીચલું, size, align);
}
સ્થિર અંતઃરેખ શૂન્ય *memalign_ઉપલું(u32 align, u32 size) {
    પાછા _malloc(&Zone_ઉપલું, size, align);
}
સ્થિર અંતઃરેખ શૂન્ય *memalign_tmplow(u32 align, u32 size) {
    પાછા _malloc(&Zone_Tmp_નીચલું, size, align);
}
સ્થિર અંતઃરેખ શૂન્ય *memalign_tmphigh(u32 align, u32 size) {
    પાછા _malloc(&Zone_Tmp_ઉપલું, size, align);
}
સ્થિર અંતઃરેખ શૂન્ય *memalign_tmp(u32 align, u32 size) {
    શૂન્ય *ret = memalign_tmphigh(align, size);
    જો (ret)
        પાછા ret;
    પાછા memalign_tmplow(align, size);
}

#endif // malloc.h
