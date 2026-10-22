// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_BDA
#అన్తర్భావ "config.h" // BUILD_BIOS_ADDR
#అన్తర్భావ "e820map.h" // struct e820entry
#అన్తర్భావ "list.h" // hlist_node
#అన్తర్భావ "malloc.h" // _malloc
#అన్తర్భావ "memmap.h" // PAGE_SIZE
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // wait_preempt
#అన్తర్భావ "std/optionrom.h" // OPTION_ROM_ALIGN
#అన్తర్భావ "string.h" // memset

// Information on a reserved area.
నిర్మాణం allocinfo_s {
    నిర్మాణం hlist_కణుపు కణుపు;
    u32 range_ప్రారంభం, range_చివర, alloc_కొలత;
};

// Information on a tracked memory allocation.
నిర్మాణం allocdetail_s {
    నిర్మాణం allocinfo_s detailinfo;
    నిర్మాణం allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
నిర్మాణం zone_s {
    నిర్మాణం hlist_తల తల;
};

నిర్మాణం zone_s Zone_దిగువ VARVERIFY32INIT, Zone_ఎగువ VARVERIFY32INIT;
నిర్మాణం zone_s ZoneFSeg VARVERIFY32INIT;
నిర్మాణం zone_s Zone_Tmp_దిగువ VARVERIFY32INIT, Zone_Tmp_ఎగువ VARVERIFY32INIT;

స్థిర నిర్మాణం zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_దిగువ, &Zone_దిగువ, &ZoneFSeg, &Zone_Tmp_ఎగువ, &Zone_ఎగువ
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
స్థిర u32
alloc_new(నిర్మాణం zone_s *zone, u32 కొలత, u32 align, నిర్మాణం allocinfo_s *fill)
{
    నిర్మాణం allocinfo_s *info;
    hlist_for_each_నమోదు(info, &zone->తల, కణుపు) {
        u32 alloc_చివర = info->range_ప్రారంభం + info->alloc_కొలత;
        u32 range_చివర = info->range_చివర;
        u32 new_range_చివర = ALIGN_కిందికి(range_చివర - కొలత, align);
        ఒకవేళ (new_range_చివర >= alloc_చివర && new_range_చివర <= range_చివర) {
            // Found space - now reserve it.
            fill->range_ప్రారంభం = new_range_చివర;
            fill->range_చివర = range_చివర;
            fill->alloc_కొలత = కొలత;

            info->range_చివర = new_range_చివర;
            hlist_కూడు_before(&fill->కణుపు, &info->కణుపు);
            తిరిగి new_range_చివర;
        }
    }
    తిరిగి 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
స్థిర నిర్మాణం allocdetail_s *
alloc_new_detail(నిర్మాణం allocdetail_s *తాత్కాలిక)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_ఎగువ, పరిమాణం(నిర్మాణం allocdetail_s)
                                , MALLOC_MIN_ALIGN, &తాత్కాలిక->detailinfo);
    ఒకవేళ (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_దిగువ, పరిమాణం(నిర్మాణం allocdetail_s)
                                , MALLOC_MIN_ALIGN, &తాత్కాలిక->detailinfo);
        ఒకవేళ (!detail_addr) {
            warn_noalloc();
            తిరిగి ఖాళీ;
        }
    }
    నిర్మాణం allocdetail_s *detail = memremap(detail_addr, పరిమాణం(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    జ్ఞాపకం_నకలు(detail, తాత్కాలిక, పరిమాణం(*detail));
    hlist_replace(&తాత్కాలిక->detailinfo.కణుపు, &detail->detailinfo.కణుపు);
    hlist_replace(&తాత్కాలిక->datainfo.కణుపు, &detail->datainfo.కణుపు);
    తిరిగి detail;
}

// Add new memory to a zone
స్థిర శూన్య
alloc_కూడు(నిర్మాణం zone_s *zone, u32 ప్రారంభం, u32 చివర)
{
    // Find position to add space
    నిర్మాణం allocinfo_s *info;
    నిర్మాణం hlist_కణుపు **pprev;
    hlist_for_each_నమోదు_pprev(info, pprev, &zone->తల, కణుపు) {
        ఒకవేళ (info->range_ప్రారంభం < ప్రారంభం)
            ఆపు;
    }

    // Add space using temporary allocation info.
    నిర్మాణం allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_ప్రారంభం = ప్రారంభం;
    tempdetail.datainfo.range_చివర = చివర;
    tempdetail.datainfo.alloc_కొలత = 0;
    hlist_కూడు(&tempdetail.datainfo.కణుపు, pprev);

    // Allocate final allocation info.
    నిర్మాణం allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ఒకవేళ (!detail)
        hlist_del(&tempdetail.datainfo.కణుపు);
}

// Release space allocated with alloc_new()
స్థిర శూన్య
alloc_free(నిర్మాణం allocinfo_s *info)
{
    నిర్మాణం allocinfo_s *తదుపరి = container_యొక్క_or_null(
        info->కణుపు.తదుపరి, నిర్మాణం allocinfo_s, కణుపు);
    ఒకవేళ (తదుపరి && తదుపరి->range_చివర == info->range_ప్రారంభం)
        తదుపరి->range_చివర = info->range_చివర;
    hlist_del(&info->కణుపు);
}

// Search all zones for an allocation obtained from alloc_new()
స్థిర నిర్మాణం allocinfo_s *
alloc_కనుగొను(u32 దత్తాంశం)
{
    పూర్ణాంకం i;
    కొరకు (i=0; i<ARRAY_కొలత(Zones); i++) {
        నిర్మాణం allocinfo_s *info;
        hlist_for_each_నమోదు(info, &Zones[i]->తల, కణుపు) {
            ఒకవేళ (info->range_ప్రారంభం == దత్తాంశం)
                తిరిగి info;
        }
    }
    తిరిగి ఖాళీ;
}

// Find the lowest memory range added by alloc_add()
స్థిర నిర్మాణం allocinfo_s *
alloc_కనుగొను_అత్యల్ప(నిర్మాణం zone_s *zone)
{
    నిర్మాణం allocinfo_s *info, *చివరి = ఖాళీ;
    hlist_for_each_నమోదు(info, &zone->తల, కణుపు) {
        చివరి = info;
    }
    తిరిగి చివరి;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
స్థిర పూర్ణాంకం
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_కొలత)
{
    u32 lowram = GET_BDA(mem_కొలత_kb) * 1024;
    ఒకవేళ (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        తిరిగి -1;

    // Do copy
    memmove((శూన్య*)newebda, (శూన్య*)oldebda, ebda_కొలత * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    సమితి_BDA(mem_కొలత_kb, newebda / 1024);
    సమితి_BDA(ebda_seg, FLATPTR_గమ్యం_SEG(newebda));
    తిరిగి 0;
}

// Support expanding the ZoneLow dynamically.
స్థిర u32
zonelow_expand(u32 కొలత, u32 align, నిర్మాణం allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    ఒకవేళ (unlikely(wait_preempt())) {
        u32 దత్తాంశం = alloc_new(&Zone_దిగువ, కొలత, align, fill);
        ఒకవేళ (దత్తాంశం)
            తిరిగి దత్తాంశం;
    }

    నిర్మాణం allocinfo_s *info = alloc_కనుగొను_అత్యల్ప(&Zone_దిగువ);
    ఒకవేళ (!info)
        తిరిగి 0;
    u32 oldpos = info->range_చివర;
    u32 newpos = ALIGN_కిందికి(oldpos - కొలత, align);
    u32 bottom = info->range_ప్రారంభం + info->alloc_కొలత;
    ఒకవేళ (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        తిరిగి alloc_new(&Zone_దిగువ, కొలత, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_కొలత = GET_EBDA(ebda_seg, కొలత);
    u32 ebda_చివర = ebda_pos + ebda_కొలత * 1024;
    ఒకవేళ (ebda_చివర != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_కిందికి(ebda_చివర - కొలత, align);
    u32 newbottom = ALIGN_కిందికి(newpos, 1024);
    u32 newebda = ALIGN_కిందికి(newbottom - ebda_కొలత * 1024, 1024);
    ఒకవేళ (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        తిరిగి 0;

    // Move ebda
    పూర్ణాంకం ret = relocate_ebda(newebda, ebda_pos, ebda_కొలత);
    ఒకవేళ (ret)
        తిరిగి 0;

    // Update zone
    ఒకవేళ (ebda_చివర == bottom)
        info->range_ప్రారంభం = newbottom;
    లేకపోతే
        alloc_కూడు(&Zone_దిగువ, newbottom, ebda_చివర);

    తిరిగి alloc_new(&Zone_దిగువ, కొలత, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(నిర్మాణం zone_s *zone, u32 కొలత, u32 align)
{
    ASSERT32FLAT();
    ఒకవేళ (!కొలత)
        తిరిగి 0;

    // Find and reserve space for main allocation
    నిర్మాణం allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 దత్తాంశం = alloc_new(zone, కొలత, align, &tempdetail.datainfo);
    ఒకవేళ (!CONFIG_MALLOC_UPPERMEMORY && !దత్తాంశం && zone == &Zone_దిగువ)
        దత్తాంశం = zonelow_expand(కొలత, align, &tempdetail.datainfo);
    ఒకవేళ (!దత్తాంశం)
        తిరిగి 0;

    // Find and reserve space for bookkeeping.
    నిర్మాణం allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ఒకవేళ (!detail) {
        alloc_free(&tempdetail.datainfo);
        తిరిగి 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, కొలత, align, దత్తాంశం, detail);

    తిరిగి దత్తాంశం;
}

// Allocate virtual memory from the given zone
శూన్య * __malloc
_malloc(నిర్మాణం zone_s *zone, u32 కొలత, u32 align)
{
    తిరిగి memremap(malloc_palloc(zone, కొలత, align), కొలత);
}

// Free a data block allocated with phys_alloc
పూర్ణాంకం
malloc_pfree(u32 దత్తాంశం)
{
    ASSERT32FLAT();
    నిర్మాణం allocinfo_s *info = alloc_కనుగొను(దత్తాంశం);
    ఒకవేళ (!info || దత్తాంశం == virt_గమ్యం_phys(info) || !info->alloc_కొలత)
        తిరిగి -1;
    నిర్మాణం allocdetail_s *detail = container_యొక్క(
        info, నిర్మాణం allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", దత్తాంశం, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    తిరిగి 0;
}

శూన్య
విడుదల(శూన్య *దత్తాంశం)
{
    ఒకవేళ (!దత్తాంశం)
        తిరిగి;
    పూర్ణాంకం ret = malloc_pfree(virt_గమ్యం_phys(దత్తాంశం));
    ఒకవేళ (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(నిర్మాణం zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    నిర్మాణం allocinfo_s *info;
    hlist_for_each_నమోదు(info, &zone->తల, కణుపు) {
        u32 space = info->range_చివర - info->range_ప్రారంభం - info->alloc_కొలత;
        ఒకవేళ (space > maxspace)
            maxspace = space;
    }

    ఒకవేళ (zone != &Zone_Tmp_ఎగువ && zone != &Zone_Tmp_దిగువ)
        తిరిగి maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(పరిమాణం(నిర్మాణం allocdetail_s), MALLOC_MIN_ALIGN);
    ఒకవేళ (maxspace <= reserve)
        తిరిగి 0;
    తిరిగి maxspace - reserve;
}

// Set a handle associated with an allocation.
శూన్య
malloc_sethandle(u32 దత్తాంశం, u32 handle)
{
    ASSERT32FLAT();
    నిర్మాణం allocinfo_s *info = alloc_కనుగొను(దత్తాంశం);
    ఒకవేళ (!info || దత్తాంశం == virt_గమ్యం_phys(info) || !info->alloc_కొలత)
        తిరిగి;
    నిర్మాణం allocdetail_s *detail = container_యొక్క(
        info, నిర్మాణం allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    పూర్ణాంకం i;
    కొరకు (i=0; i<ARRAY_కొలత(Zones); i++) {
        నిర్మాణం allocinfo_s *info;
        hlist_for_each_నమోదు(info, &Zones[i]->తల, కణుపు) {
            ఒకవేళ (info->range_ప్రారంభం != virt_గమ్యం_phys(info))
                కొనసాగు;
            నిర్మాణం allocdetail_s *detail = container_యొక్క(
                info, నిర్మాణం allocdetail_s, detailinfo);
            ఒకవేళ (detail->handle == handle)
                తిరిగి detail->datainfo.range_ప్రారంభం;
        }
    }
    తిరిగి 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

స్థిర u32 Rom_చివర = BUILD_ROM_ప్రారంభం;
స్థిర నిర్మాణం allocinfo_s *Rom_పునాది;

#నిర్వచించు OPROM_శీర్షిక_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(శూన్య)
{
    ఒకవేళ (CONFIG_MALLOC_UPPERMEMORY)
        తిరిగి ALIGN_కిందికి(Rom_పునాది->range_చివర - OPROM_శీర్షిక_RESERVE
                          , OPTION_ROM_ALIGN);
    తిరిగి చిహ్నం(final_readonly_ప్రారంభం);
}

// Return the end of the last deployed option rom.
u32
rom_get_చివరి(శూన్య)
{
    తిరిగి Rom_చివర;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
నిర్మాణం rom_శీర్షిక *
rom_reserve(u32 కొలత)
{
    u32 newend = ALIGN(Rom_చివర + కొలత, OPTION_ROM_ALIGN);
    ఒకవేళ (newend > rom_get_max())
        తిరిగి ఖాళీ;
    ఒకవేళ (CONFIG_MALLOC_UPPERMEMORY) {
        ఒకవేళ (newend < చిహ్నం(zonelow_పునాది))
            newend = చిహ్నం(zonelow_పునాది);
        Rom_పునాది->range_ప్రారంభం = newend + OPROM_శీర్షిక_RESERVE;
    }
    తిరిగి (శూన్య*)Rom_చివర;
}

// Confirm space as in use by an optionrom.
పూర్ణాంకం
rom_confirm(u32 కొలత)
{
    శూన్య *new = rom_reserve(కొలత);
    ఒకవేళ (!new) {
        warn_noalloc();
        తిరిగి -1;
    }
    Rom_చివర = ALIGN(Rom_చివర + కొలత, OPTION_ROM_ALIGN);
    తిరిగి 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

శూన్య
malloc_preinit(శూన్య)
{
    ASSERT32FLAT();
    dprintf(3, "malloc preinit\n");

    // Don't declare any memory between 0xa0000 and 0x100000
    e820_remove(BUILD_LOWRAM_చివర, BUILD_BIOS_ADDR-BUILD_LOWRAM_చివర);

    // Mark known areas as reserved.
    e820_కూడు(BUILD_BIOS_ADDR, BUILD_BIOS_కొలత, E820_RESERVED);

    // Populate temp high ram
    u32 highram_ప్రారంభం = 0;
    u32 highram_కొలత = 0;
    పూర్ణాంకం i;
    కొరకు (i=e820_లెక్క-1; i>=0; i--) {
        నిర్మాణం e820entry *en = &e820_list[i];
        u64 చివర = en->ప్రారంభం + en->కొలత;
        ఒకవేళ (చివర < 1024*1024)
            ఆపు;
        ఒకవేళ (en->type != E820_RAM || చివర > 0xffffffff)
            కొనసాగు;
        u32 s = en->ప్రారంభం, e = చివర;
        ఒకవేళ (!highram_ప్రారంభం) {
            u32 new_max = ALIGN_కిందికి(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_కిందికి(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            ఒకవేళ (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_ప్రారంభం = e = new_max;
                highram_కొలత = BUILD_MAX_HIGHTABLE;
            } లేకపోతే ఒకవేళ (new_min <= e && new_min >= s) {
                highram_ప్రారంభం = e = new_min;
                highram_కొలత = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_కూడు(&Zone_Tmp_ఎగువ, s, e);
    }

    // Populate regions
    alloc_కూడు(&Zone_Tmp_దిగువ, BUILD_దొంతర_ADDR, BUILD_EBDA_MINIMUM);
    ఒకవేళ (highram_ప్రారంభం) {
        alloc_కూడు(&Zone_ఎగువ, highram_ప్రారంభం, highram_ప్రారంభం + highram_కొలత);
        e820_కూడు(highram_ప్రారంభం, highram_కొలత, E820_RESERVED);
    }
}

శూన్య
malloc_csm_preinit(u32 దిగువ_pmm, u32 దిగువ_pmm_కొలత, u32 hi_pmm, u32 hi_pmm_కొలత)
{
    ASSERT32FLAT();

    ఒకవేళ (hi_pmm_కొలత > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_చివర = hi_pmm + hi_pmm_కొలత;
        alloc_కూడు(&Zone_Tmp_ఎగువ, hi_pmm, hi_pmm_చివర - BUILD_MIN_HIGHTABLE);
        alloc_కూడు(&Zone_ఎగువ, hi_pmm_చివర - BUILD_MIN_HIGHTABLE, hi_pmm_చివర);
    } లేకపోతే {
        alloc_కూడు(&Zone_Tmp_ఎగువ, hi_pmm, hi_pmm + hi_pmm_కొలత);
    }
    alloc_కూడు(&Zone_Tmp_దిగువ, దిగువ_pmm, దిగువ_pmm + దిగువ_pmm_కొలత);
}

u32 Legacy_Ram_కొలత VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
స్థిర శూన్య
calc_Ram_కొలత(శూన్య)
{
    u32 rs = 0;
    పూర్ణాంకం i;
    కొరకు (i=e820_లెక్క-1; i>=0; i--) {
        నిర్మాణం e820entry *en = &e820_list[i];
        u64 చివర = en->ప్రారంభం + en->కొలత;
        u32 type = en->type;
        ఒకవేళ (చివర <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = చివర;
            ఆపు;
        }
    }
    Legacy_Ram_కొలత = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
శూన్య
malloc_init(శూన్య)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    ఒకవేళ (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        పూర్ణాంకం i;
        కొరకు (i=0; i<ARRAY_కొలత(Zones); i++) {
            నిర్మాణం zone_s *zone = Zones[i];
            ఒకవేళ (zone->తల.మొదటి)
                zone->తల.మొదటి->pprev = &zone->తల.మొదటి;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_ప్రారంభం), VSYMBOL(varlow_ప్రారంభం)
            , చిహ్నం(varlow_చివర) - చిహ్నం(varlow_ప్రారంభం));
    ఒకవేళ (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_కూడు(&Zone_దిగువ, చిహ్నం(zonelow_పునాది) + OPROM_శీర్షిక_RESERVE
                  , చిహ్నం(final_varlow_ప్రారంభం));
        Rom_పునాది = alloc_కనుగొను_అత్యల్ప(&Zone_దిగువ);
    } లేకపోతే {
        alloc_కూడు(&Zone_దిగువ, ALIGN_కిందికి(చిహ్నం(final_varlow_ప్రారంభం), 1024)
                  , చిహ్నం(final_varlow_ప్రారంభం));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_ప్రారంభం), 0
           , చిహ్నం(zonefseg_చివర) - చిహ్నం(zonefseg_ప్రారంభం));
    alloc_కూడు(&ZoneFSeg, చిహ్నం(zonefseg_ప్రారంభం), చిహ్నం(zonefseg_చివర));

    calc_Ram_కొలత();
}

శూన్య
malloc_prepboot(శూన్య)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 పునాది = rom_get_max();
    memset((శూన్య*)Rom_చివర, 0, పునాది-Rom_చివర);
    ఒకవేళ (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        నిర్మాణం rom_శీర్షిక *dummyrom = (శూన్య*)పునాది;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        పూర్ణాంకం కొలత = (BUILD_BIOS_ADDR - పునాది) / 512;
        dummyrom->కొలత = (కొలత > 255) ? 255 : కొలత;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_కొలత_kb)*1024;
    e820_కూడు(endlow, BUILD_LOWRAM_చివర-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    నిర్మాణం allocinfo_s *info = alloc_కనుగొను_అత్యల్ప(&ZoneFSeg);
    u32 కొలత = info->range_చివర - info->range_ప్రారంభం;
    memset(memremap(info->range_ప్రారంభం, కొలత), 0, కొలత);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_చివర, పునాది, info->range_ప్రారంభం, info->range_చివర);

    // Give back unused high ram.
    info = alloc_కనుగొను_అత్యల్ప(&Zone_ఎగువ);
    ఒకవేళ (info) {
        u32 giveback = ALIGN_కిందికి(info->range_చివర-info->range_ప్రారంభం, పుట_కొలత);
        e820_కూడు(info->range_ప్రారంభం, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_కొలత();
}
