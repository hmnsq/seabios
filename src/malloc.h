#ifndef __MALLOC_H
#define __MALLOC_H

#அந்தர்பாவ "types.h" // u32

// malloc.c
வெளி கட்டமைப்பு zone_s Zone_கீழ், Zone_உயர்வு, ZoneFSeg, Zone_Tmp_கீழ், Zone_Tmp_உயர்வு;
u32 rom_get_max(வெறுமை);
u32 rom_get_last(வெறுமை);
கட்டமைப்பு rom_header *rom_reserve(u32 size);
முழு rom_confirm(u32 size);
வெறுமை malloc_csm_preinit(u32 கீழ்_pmm, u32 கீழ்_pmm_size, u32 hi_pmm,
                        u32 hi_pmm_size);
வெறுமை malloc_preinit(வெறுமை);
வெளி u32 LegacyRamSize;
வெறுமை malloc_init(வெறுமை);
வெறுமை malloc_prepboot(வெறுமை);
u32 malloc_palloc(கட்டமைப்பு zone_s *zone, u32 size, u32 align);
வெறுமை *_malloc(கட்டமைப்பு zone_s *zone, u32 size, u32 align);
முழு malloc_pfree(u32 data);
வெறுமை விடுவி(வெறுமை *data);
u32 malloc_getspace(கட்டமைப்பு zone_s *zone);
வெறுமை malloc_sethandle(u32 data, u32 handle);
u32 malloc_findhandle(u32 handle);

#define MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#define MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
நிலையான உள்வரி வெறுமை *malloc_கீழ்(u32 size) {
    திரும்பு _malloc(&Zone_கீழ், size, MALLOC_MIN_ALIGN);
}
நிலையான உள்வரி வெறுமை *malloc_உயர்வு(u32 size) {
    திரும்பு _malloc(&Zone_உயர்வு, size, MALLOC_MIN_ALIGN);
}
நிலையான உள்வரி வெறுமை *malloc_fseg(u32 size) {
    திரும்பு _malloc(&ZoneFSeg, size, MALLOC_MIN_ALIGN);
}
நிலையான உள்வரி வெறுமை *malloc_tmplow(u32 size) {
    திரும்பு _malloc(&Zone_Tmp_கீழ், size, MALLOC_MIN_ALIGN);
}
நிலையான உள்வரி வெறுமை *malloc_tmphigh(u32 size) {
    திரும்பு _malloc(&Zone_Tmp_உயர்வு, size, MALLOC_MIN_ALIGN);
}
நிலையான உள்வரி வெறுமை *malloc_tmp(u32 size) {
    வெறுமை *ret = malloc_tmphigh(size);
    எனில் (ret)
        திரும்பு ret;
    திரும்பு malloc_tmplow(size);
}
நிலையான உள்வரி வெறுமை *memalign_கீழ்(u32 align, u32 size) {
    திரும்பு _malloc(&Zone_கீழ், size, align);
}
நிலையான உள்வரி வெறுமை *memalign_உயர்வு(u32 align, u32 size) {
    திரும்பு _malloc(&Zone_உயர்வு, size, align);
}
நிலையான உள்வரி வெறுமை *memalign_tmplow(u32 align, u32 size) {
    திரும்பு _malloc(&Zone_Tmp_கீழ், size, align);
}
நிலையான உள்வரி வெறுமை *memalign_tmphigh(u32 align, u32 size) {
    திரும்பு _malloc(&Zone_Tmp_உயர்வு, size, align);
}
நிலையான உள்வரி வெறுமை *memalign_tmp(u32 align, u32 size) {
    வெறுமை *ret = memalign_tmphigh(align, size);
    எனில் (ret)
        திரும்பு ret;
    திரும்பு memalign_tmplow(align, size);
}

#endif // malloc.h
