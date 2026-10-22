#अगर_अघोषित __MALLOC_H
#घोषणा __MALLOC_H

#समावेश "types.h" // u32

// malloc.c
बाह्य काष्ठा zone_s Zone_निचला, Zone_ऊपरी, ZoneFSeg, Zone_Tmp_निचला, Zone_Tmp_ऊपरी;
u32 rom_get_max(व्योम);
u32 rom_get_last(व्योम);
काष्ठा rom_header *rom_reserve(u32 आकार);
पूर्णांक rom_confirm(u32 आकार);
व्योम malloc_csm_preinit(u32 निचला_pmm, u32 निचला_pmm_आकार, u32 hi_pmm,
                        u32 hi_pmm_आकार);
व्योम malloc_preinit(व्योम);
बाह्य u32 Legacy_Ram_आकार;
व्योम malloc_init(व्योम);
व्योम malloc_prepboot(व्योम);
u32 malloc_palloc(काष्ठा zone_s *zone, u32 आकार, u32 align);
व्योम *_malloc(काष्ठा zone_s *zone, u32 आकार, u32 align);
पूर्णांक malloc_pfree(u32 तथ्य);
व्योम मुक्त(व्योम *तथ्य);
u32 malloc_getspace(काष्ठा zone_s *zone);
व्योम malloc_sethandle(u32 तथ्य, u32 handle);
u32 malloc_findhandle(u32 handle);

#घोषणा MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#घोषणा MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
अटल अंतरभूत व्योम *malloc_निचला(u32 आकार) {
    वापस _malloc(&Zone_निचला, आकार, MALLOC_MIN_ALIGN);
}
अटल अंतरभूत व्योम *malloc_ऊपरी(u32 आकार) {
    वापस _malloc(&Zone_ऊपरी, आकार, MALLOC_MIN_ALIGN);
}
अटल अंतरभूत व्योम *malloc_fseg(u32 आकार) {
    वापस _malloc(&ZoneFSeg, आकार, MALLOC_MIN_ALIGN);
}
अटल अंतरभूत व्योम *malloc_tmplow(u32 आकार) {
    वापस _malloc(&Zone_Tmp_निचला, आकार, MALLOC_MIN_ALIGN);
}
अटल अंतरभूत व्योम *malloc_tmphigh(u32 आकार) {
    वापस _malloc(&Zone_Tmp_ऊपरी, आकार, MALLOC_MIN_ALIGN);
}
अटल अंतरभूत व्योम *malloc_tmp(u32 आकार) {
    व्योम *ret = malloc_tmphigh(आकार);
    अगर (ret)
        वापस ret;
    वापस malloc_tmplow(आकार);
}
अटल अंतरभूत व्योम *memalign_निचला(u32 align, u32 आकार) {
    वापस _malloc(&Zone_निचला, आकार, align);
}
अटल अंतरभूत व्योम *memalign_ऊपरी(u32 align, u32 आकार) {
    वापस _malloc(&Zone_ऊपरी, आकार, align);
}
अटल अंतरभूत व्योम *memalign_tmplow(u32 align, u32 आकार) {
    वापस _malloc(&Zone_Tmp_निचला, आकार, align);
}
अटल अंतरभूत व्योम *memalign_tmphigh(u32 align, u32 आकार) {
    वापस _malloc(&Zone_Tmp_ऊपरी, आकार, align);
}
अटल अंतरभूत व्योम *memalign_tmp(u32 align, u32 आकार) {
    व्योम *ret = memalign_tmphigh(align, आकार);
    अगर (ret)
        वापस ret;
    वापस memalign_tmplow(align, आकार);
}

#पूर्ण_अगर // malloc.h
