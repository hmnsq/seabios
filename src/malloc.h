#నిర్వచితం_కాకపోతే __MALLOC_H
#నిర్వచించు __MALLOC_H

#అన్తర్భావ "types.h" // u32

// malloc.c
బాహ్య నిర్మాణం zone_s Zone_దిగువ, Zone_ఎగువ, ZoneFSeg, Zone_Tmp_దిగువ, Zone_Tmp_ఎగువ;
u32 rom_get_max(శూన్య);
u32 rom_get_చివరి(శూన్య);
నిర్మాణం rom_శీర్షిక *rom_reserve(u32 కొలత);
పూర్ణాంకం rom_confirm(u32 కొలత);
శూన్య malloc_csm_preinit(u32 దిగువ_pmm, u32 దిగువ_pmm_కొలత, u32 hi_pmm,
                        u32 hi_pmm_కొలత);
శూన్య malloc_preinit(శూన్య);
బాహ్య u32 Legacy_Ram_కొలత;
శూన్య malloc_init(శూన్య);
శూన్య malloc_prepboot(శూన్య);
u32 malloc_palloc(నిర్మాణం zone_s *zone, u32 కొలత, u32 align);
శూన్య *_malloc(నిర్మాణం zone_s *zone, u32 కొలత, u32 align);
పూర్ణాంకం malloc_pfree(u32 దత్తాంశం);
శూన్య విడుదల(శూన్య *దత్తాంశం);
u32 malloc_getspace(నిర్మాణం zone_s *zone);
శూన్య malloc_sethandle(u32 దత్తాంశం, u32 handle);
u32 malloc_findhandle(u32 handle);

#నిర్వచించు MALLOC_DEFAULT_HANDLE 0xFFFFFFFF
// Minimum alignment of malloc'd memory
#నిర్వచించు MALLOC_MIN_ALIGN 16
// Helper functions for memory allocation.
స్థిర అంతఃవరుస శూన్య *malloc_దిగువ(u32 కొలత) {
    తిరిగి _malloc(&Zone_దిగువ, కొలత, MALLOC_MIN_ALIGN);
}
స్థిర అంతఃవరుస శూన్య *malloc_ఎగువ(u32 కొలత) {
    తిరిగి _malloc(&Zone_ఎగువ, కొలత, MALLOC_MIN_ALIGN);
}
స్థిర అంతఃవరుస శూన్య *malloc_fseg(u32 కొలత) {
    తిరిగి _malloc(&ZoneFSeg, కొలత, MALLOC_MIN_ALIGN);
}
స్థిర అంతఃవరుస శూన్య *malloc_tmplow(u32 కొలత) {
    తిరిగి _malloc(&Zone_Tmp_దిగువ, కొలత, MALLOC_MIN_ALIGN);
}
స్థిర అంతఃవరుస శూన్య *malloc_tmphigh(u32 కొలత) {
    తిరిగి _malloc(&Zone_Tmp_ఎగువ, కొలత, MALLOC_MIN_ALIGN);
}
స్థిర అంతఃవరుస శూన్య *malloc_tmp(u32 కొలత) {
    శూన్య *ret = malloc_tmphigh(కొలత);
    ఒకవేళ (ret)
        తిరిగి ret;
    తిరిగి malloc_tmplow(కొలత);
}
స్థిర అంతఃవరుస శూన్య *memalign_దిగువ(u32 align, u32 కొలత) {
    తిరిగి _malloc(&Zone_దిగువ, కొలత, align);
}
స్థిర అంతఃవరుస శూన్య *memalign_ఎగువ(u32 align, u32 కొలత) {
    తిరిగి _malloc(&Zone_ఎగువ, కొలత, align);
}
స్థిర అంతఃవరుస శూన్య *memalign_tmplow(u32 align, u32 కొలత) {
    తిరిగి _malloc(&Zone_Tmp_దిగువ, కొలత, align);
}
స్థిర అంతఃవరుస శూన్య *memalign_tmphigh(u32 align, u32 కొలత) {
    తిరిగి _malloc(&Zone_Tmp_ఎగువ, కొలత, align);
}
స్థిర అంతఃవరుస శూన్య *memalign_tmp(u32 align, u32 కొలత) {
    శూన్య *ret = memalign_tmphigh(align, కొలత);
    ఒకవేళ (ret)
        తిరిగి ret;
    తిరిగి memalign_tmplow(align, కొలత);
}

#అయితే_ముగింపు // malloc.h
