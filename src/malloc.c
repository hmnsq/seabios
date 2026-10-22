// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_BDA
#अन्तर्भाव "config.h" // BUILD_BIOS_ADDR
#अन्तर्भाव "e820map.h" // struct e820entry
#अन्तर्भाव "list.h" // hlist_node
#अन्तर्भाव "malloc.h" // _malloc
#अन्तर्भाव "memmap.h" // PAGE_SIZE
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // wait_preempt
#अन्तर्भाव "std/optionrom.h" // OPTION_ROM_ALIGN
#अन्तर्भाव "string.h" // memset

// Information on a reserved area.
संरचना allocinfo_s {
    संरचना hlist_ग्रन्थिः ग्रन्थिः;
    u32 range_start, range_अन्तः, alloc_आकार;
};

// Information on a tracked memory allocation.
संरचना allocdetail_s {
    संरचना allocinfo_s detailinfo;
    संरचना allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
संरचना zone_s {
    संरचना hlist_शीर्ष शीर्ष;
};

संरचना zone_s Zone_नीचः VARVERIFY32INIT, Zone_उच्चः VARVERIFY32INIT;
संरचना zone_s ZoneFSeg VARVERIFY32INIT;
संरचना zone_s Zone_Tmp_नीचः VARVERIFY32INIT, Zone_Tmp_उच्चः VARVERIFY32INIT;

स्थिर संरचना zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_नीचः, &Zone_नीचः, &ZoneFSeg, &Zone_Tmp_उच्चः, &Zone_उच्चः
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
स्थिर u32
alloc_new(संरचना zone_s *zone, u32 आकार, u32 align, संरचना allocinfo_s *fill)
{
    संरचना allocinfo_s *info;
    hlist_for_each_प्रविष्टिः(info, &zone->शीर्ष, ग्रन्थिः) {
        u32 alloc_अन्तः = info->range_start + info->alloc_आकार;
        u32 range_अन्तः = info->range_अन्तः;
        u32 new_range_अन्तः = ALIGN_अधस्तात्(range_अन्तः - आकार, align);
        यदि (new_range_अन्तः >= alloc_अन्तः && new_range_अन्तः <= range_अन्तः) {
            // Found space - now reserve it.
            fill->range_start = new_range_अन्तः;
            fill->range_अन्तः = range_अन्तः;
            fill->alloc_आकार = आकार;

            info->range_अन्तः = new_range_अन्तः;
            hlist_सङ्कलय_before(&fill->ग्रन्थिः, &info->ग्रन्थिः);
            प्रत्यागम new_range_अन्तः;
        }
    }
    प्रत्यागम 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
स्थिर संरचना allocdetail_s *
alloc_new_detail(संरचना allocdetail_s *अस्थायी)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_उच्चः, परिमाण(संरचना allocdetail_s)
                                , MALLOC_MIN_ALIGN, &अस्थायी->detailinfo);
    यदि (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_नीचः, परिमाण(संरचना allocdetail_s)
                                , MALLOC_MIN_ALIGN, &अस्थायी->detailinfo);
        यदि (!detail_addr) {
            warn_noalloc();
            प्रत्यागम रिक्त;
        }
    }
    संरचना allocdetail_s *detail = memremap(detail_addr, परिमाण(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    स्मृतिं_प्रतिलिख(detail, अस्थायी, परिमाण(*detail));
    hlist_replace(&अस्थायी->detailinfo.ग्रन्थिः, &detail->detailinfo.ग्रन्थिः);
    hlist_replace(&अस्थायी->datainfo.ग्रन्थिः, &detail->datainfo.ग्रन्थिः);
    प्रत्यागम detail;
}

// Add new memory to a zone
स्थिर शून्य
alloc_सङ्कलय(संरचना zone_s *zone, u32 start, u32 अन्तः)
{
    // Find position to add space
    संरचना allocinfo_s *info;
    संरचना hlist_ग्रन्थिः **pprev;
    hlist_for_each_प्रविष्टिः_pprev(info, pprev, &zone->शीर्ष, ग्रन्थिः) {
        यदि (info->range_start < start)
            भङ्ग;
    }

    // Add space using temporary allocation info.
    संरचना allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_अन्तः = अन्तः;
    tempdetail.datainfo.alloc_आकार = 0;
    hlist_सङ्कलय(&tempdetail.datainfo.ग्रन्थिः, pprev);

    // Allocate final allocation info.
    संरचना allocdetail_s *detail = alloc_new_detail(&tempdetail);
    यदि (!detail)
        hlist_del(&tempdetail.datainfo.ग्रन्थिः);
}

// Release space allocated with alloc_new()
स्थिर शून्य
alloc_free(संरचना allocinfo_s *info)
{
    संरचना allocinfo_s *अग्र = container_सम्बन्धि_or_null(
        info->ग्रन्थिः.अग्र, संरचना allocinfo_s, ग्रन्थिः);
    यदि (अग्र && अग्र->range_अन्तः == info->range_start)
        अग्र->range_अन्तः = info->range_अन्तः;
    hlist_del(&info->ग्रन्थिः);
}

// Search all zones for an allocation obtained from alloc_new()
स्थिर संरचना allocinfo_s *
alloc_find(u32 दत्त)
{
    पूर्णाङ्क i;
    प्रति (i=0; i<ARRAY_आकार(Zones); i++) {
        संरचना allocinfo_s *info;
        hlist_for_each_प्रविष्टिः(info, &Zones[i]->शीर्ष, ग्रन्थिः) {
            यदि (info->range_start == दत्त)
                प्रत्यागम info;
        }
    }
    प्रत्यागम रिक्त;
}

// Find the lowest memory range added by alloc_add()
स्थिर संरचना allocinfo_s *
alloc_find_निम्नतमम्(संरचना zone_s *zone)
{
    संरचना allocinfo_s *info, *चरमः = रिक्त;
    hlist_for_each_प्रविष्टिः(info, &zone->शीर्ष, ग्रन्थिः) {
        चरमः = info;
    }
    प्रत्यागम चरमः;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
स्थिर पूर्णाङ्क
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_आकार)
{
    u32 lowram = GET_BDA(mem_आकार_kb) * 1024;
    यदि (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        प्रत्यागम -1;

    // Do copy
    memmove((शून्य*)newebda, (शून्य*)oldebda, ebda_आकार * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    समुच्चयः_BDA(mem_आकार_kb, newebda / 1024);
    समुच्चयः_BDA(ebda_seg, FLATPTR_गन्तव्यम्_SEG(newebda));
    प्रत्यागम 0;
}

// Support expanding the ZoneLow dynamically.
स्थिर u32
zonelow_expand(u32 आकार, u32 align, संरचना allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    यदि (unlikely(wait_preempt())) {
        u32 दत्त = alloc_new(&Zone_नीचः, आकार, align, fill);
        यदि (दत्त)
            प्रत्यागम दत्त;
    }

    संरचना allocinfo_s *info = alloc_find_निम्नतमम्(&Zone_नीचः);
    यदि (!info)
        प्रत्यागम 0;
    u32 oldpos = info->range_अन्तः;
    u32 newpos = ALIGN_अधस्तात्(oldpos - आकार, align);
    u32 bottom = info->range_start + info->alloc_आकार;
    यदि (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        प्रत्यागम alloc_new(&Zone_नीचः, आकार, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_आकार = GET_EBDA(ebda_seg, आकार);
    u32 ebda_अन्तः = ebda_pos + ebda_आकार * 1024;
    यदि (ebda_अन्तः != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_अधस्तात्(ebda_अन्तः - आकार, align);
    u32 newbottom = ALIGN_अधस्तात्(newpos, 1024);
    u32 newebda = ALIGN_अधस्तात्(newbottom - ebda_आकार * 1024, 1024);
    यदि (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        प्रत्यागम 0;

    // Move ebda
    पूर्णाङ्क ret = relocate_ebda(newebda, ebda_pos, ebda_आकार);
    यदि (ret)
        प्रत्यागम 0;

    // Update zone
    यदि (ebda_अन्तः == bottom)
        info->range_start = newbottom;
    अन्यथा
        alloc_सङ्कलय(&Zone_नीचः, newbottom, ebda_अन्तः);

    प्रत्यागम alloc_new(&Zone_नीचः, आकार, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(संरचना zone_s *zone, u32 आकार, u32 align)
{
    ASSERT32FLAT();
    यदि (!आकार)
        प्रत्यागम 0;

    // Find and reserve space for main allocation
    संरचना allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 दत्त = alloc_new(zone, आकार, align, &tempdetail.datainfo);
    यदि (!CONFIG_MALLOC_UPPERMEMORY && !दत्त && zone == &Zone_नीचः)
        दत्त = zonelow_expand(आकार, align, &tempdetail.datainfo);
    यदि (!दत्त)
        प्रत्यागम 0;

    // Find and reserve space for bookkeeping.
    संरचना allocdetail_s *detail = alloc_new_detail(&tempdetail);
    यदि (!detail) {
        alloc_free(&tempdetail.datainfo);
        प्रत्यागम 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, आकार, align, दत्त, detail);

    प्रत्यागम दत्त;
}

// Allocate virtual memory from the given zone
शून्य * __malloc
_malloc(संरचना zone_s *zone, u32 आकार, u32 align)
{
    प्रत्यागम memremap(malloc_palloc(zone, आकार, align), आकार);
}

// Free a data block allocated with phys_alloc
पूर्णाङ्क
malloc_pfree(u32 दत्त)
{
    ASSERT32FLAT();
    संरचना allocinfo_s *info = alloc_find(दत्त);
    यदि (!info || दत्त == virt_गन्तव्यम्_phys(info) || !info->alloc_आकार)
        प्रत्यागम -1;
    संरचना allocdetail_s *detail = container_सम्बन्धि(
        info, संरचना allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", दत्त, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    प्रत्यागम 0;
}

शून्य
मुक्त(शून्य *दत्त)
{
    यदि (!दत्त)
        प्रत्यागम;
    पूर्णाङ्क ret = malloc_pfree(virt_गन्तव्यम्_phys(दत्त));
    यदि (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(संरचना zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    संरचना allocinfo_s *info;
    hlist_for_each_प्रविष्टिः(info, &zone->शीर्ष, ग्रन्थिः) {
        u32 space = info->range_अन्तः - info->range_start - info->alloc_आकार;
        यदि (space > maxspace)
            maxspace = space;
    }

    यदि (zone != &Zone_Tmp_उच्चः && zone != &Zone_Tmp_नीचः)
        प्रत्यागम maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(परिमाण(संरचना allocdetail_s), MALLOC_MIN_ALIGN);
    यदि (maxspace <= reserve)
        प्रत्यागम 0;
    प्रत्यागम maxspace - reserve;
}

// Set a handle associated with an allocation.
शून्य
malloc_sethandle(u32 दत्त, u32 handle)
{
    ASSERT32FLAT();
    संरचना allocinfo_s *info = alloc_find(दत्त);
    यदि (!info || दत्त == virt_गन्तव्यम्_phys(info) || !info->alloc_आकार)
        प्रत्यागम;
    संरचना allocdetail_s *detail = container_सम्बन्धि(
        info, संरचना allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    पूर्णाङ्क i;
    प्रति (i=0; i<ARRAY_आकार(Zones); i++) {
        संरचना allocinfo_s *info;
        hlist_for_each_प्रविष्टिः(info, &Zones[i]->शीर्ष, ग्रन्थिः) {
            यदि (info->range_start != virt_गन्तव्यम्_phys(info))
                अनुवर्त;
            संरचना allocdetail_s *detail = container_सम्बन्धि(
                info, संरचना allocdetail_s, detailinfo);
            यदि (detail->handle == handle)
                प्रत्यागम detail->datainfo.range_start;
        }
    }
    प्रत्यागम 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

स्थिर u32 Rom_अन्तः = BUILD_ROM_START;
स्थिर संरचना allocinfo_s *Rom_आधारः;

#परिभाषय OPROM_शीर्षकम्_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(शून्य)
{
    यदि (CONFIG_MALLOC_UPPERMEMORY)
        प्रत्यागम ALIGN_अधस्तात्(Rom_आधारः->range_अन्तः - OPROM_शीर्षकम्_RESERVE
                          , OPTION_ROM_ALIGN);
    प्रत्यागम चिह्नम्(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_चरमः(शून्य)
{
    प्रत्यागम Rom_अन्तः;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
संरचना rom_शीर्षकम् *
rom_reserve(u32 आकार)
{
    u32 newend = ALIGN(Rom_अन्तः + आकार, OPTION_ROM_ALIGN);
    यदि (newend > rom_get_max())
        प्रत्यागम रिक्त;
    यदि (CONFIG_MALLOC_UPPERMEMORY) {
        यदि (newend < चिह्नम्(zonelow_आधारः))
            newend = चिह्नम्(zonelow_आधारः);
        Rom_आधारः->range_start = newend + OPROM_शीर्षकम्_RESERVE;
    }
    प्रत्यागम (शून्य*)Rom_अन्तः;
}

// Confirm space as in use by an optionrom.
पूर्णाङ्क
rom_confirm(u32 आकार)
{
    शून्य *new = rom_reserve(आकार);
    यदि (!new) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    Rom_अन्तः = ALIGN(Rom_अन्तः + आकार, OPTION_ROM_ALIGN);
    प्रत्यागम 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

शून्य
malloc_preinit(शून्य)
{
    ASSERT32FLAT();
    dprintf(3, "malloc preinit\n");

    // Don't declare any memory between 0xa0000 and 0x100000
    e820_remove(BUILD_LOWRAM_अन्तः, BUILD_BIOS_ADDR-BUILD_LOWRAM_अन्तः);

    // Mark known areas as reserved.
    e820_सङ्कलय(BUILD_BIOS_ADDR, BUILD_BIOS_आकार, E820_RESERVED);

    // Populate temp high ram
    u32 highram_start = 0;
    u32 highram_आकार = 0;
    पूर्णाङ्क i;
    प्रति (i=e820_गणनम्-1; i>=0; i--) {
        संरचना e820entry *en = &e820_list[i];
        u64 अन्तः = en->start + en->आकार;
        यदि (अन्तः < 1024*1024)
            भङ्ग;
        यदि (en->type != E820_RAM || अन्तः > 0xffffffff)
            अनुवर्त;
        u32 s = en->start, e = अन्तः;
        यदि (!highram_start) {
            u32 new_max = ALIGN_अधस्तात्(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_अधस्तात्(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            यदि (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_आकार = BUILD_MAX_HIGHTABLE;
            } अन्यथा यदि (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_आकार = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_सङ्कलय(&Zone_Tmp_उच्चः, s, e);
    }

    // Populate regions
    alloc_सङ्कलय(&Zone_Tmp_नीचः, BUILD_राशिः_ADDR, BUILD_EBDA_MINIMUM);
    यदि (highram_start) {
        alloc_सङ्कलय(&Zone_उच्चः, highram_start, highram_start + highram_आकार);
        e820_सङ्कलय(highram_start, highram_आकार, E820_RESERVED);
    }
}

शून्य
malloc_csm_preinit(u32 नीचः_pmm, u32 नीचः_pmm_आकार, u32 hi_pmm, u32 hi_pmm_आकार)
{
    ASSERT32FLAT();

    यदि (hi_pmm_आकार > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_अन्तः = hi_pmm + hi_pmm_आकार;
        alloc_सङ्कलय(&Zone_Tmp_उच्चः, hi_pmm, hi_pmm_अन्तः - BUILD_MIN_HIGHTABLE);
        alloc_सङ्कलय(&Zone_उच्चः, hi_pmm_अन्तः - BUILD_MIN_HIGHTABLE, hi_pmm_अन्तः);
    } अन्यथा {
        alloc_सङ्कलय(&Zone_Tmp_उच्चः, hi_pmm, hi_pmm + hi_pmm_आकार);
    }
    alloc_सङ्कलय(&Zone_Tmp_नीचः, नीचः_pmm, नीचः_pmm + नीचः_pmm_आकार);
}

u32 Legacy_Ram_आकार VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
स्थिर शून्य
calc_Ram_आकार(शून्य)
{
    u32 rs = 0;
    पूर्णाङ्क i;
    प्रति (i=e820_गणनम्-1; i>=0; i--) {
        संरचना e820entry *en = &e820_list[i];
        u64 अन्तः = en->start + en->आकार;
        u32 type = en->type;
        यदि (अन्तः <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = अन्तः;
            भङ्ग;
        }
    }
    Legacy_Ram_आकार = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
शून्य
malloc_init(शून्य)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    यदि (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        पूर्णाङ्क i;
        प्रति (i=0; i<ARRAY_आकार(Zones); i++) {
            संरचना zone_s *zone = Zones[i];
            यदि (zone->शीर्ष.first)
                zone->शीर्ष.first->pprev = &zone->शीर्ष.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , चिह्नम्(varlow_अन्तः) - चिह्नम्(varlow_start));
    यदि (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_सङ्कलय(&Zone_नीचः, चिह्नम्(zonelow_आधारः) + OPROM_शीर्षकम्_RESERVE
                  , चिह्नम्(final_varlow_start));
        Rom_आधारः = alloc_find_निम्नतमम्(&Zone_नीचः);
    } अन्यथा {
        alloc_सङ्कलय(&Zone_नीचः, ALIGN_अधस्तात्(चिह्नम्(final_varlow_start), 1024)
                  , चिह्नम्(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    स्मृतिं_पूरय(VSYMBOL(zonefseg_start), 0
           , चिह्नम्(zonefseg_अन्तः) - चिह्नम्(zonefseg_start));
    alloc_सङ्कलय(&ZoneFSeg, चिह्नम्(zonefseg_start), चिह्नम्(zonefseg_अन्तः));

    calc_Ram_आकार();
}

शून्य
malloc_prepboot(शून्य)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 आधारः = rom_get_max();
    स्मृतिं_पूरय((शून्य*)Rom_अन्तः, 0, आधारः-Rom_अन्तः);
    यदि (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        संरचना rom_शीर्षकम् *dummyrom = (शून्य*)आधारः;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        पूर्णाङ्क आकार = (BUILD_BIOS_ADDR - आधारः) / 512;
        dummyrom->आकार = (आकार > 255) ? 255 : आकार;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_आकार_kb)*1024;
    e820_सङ्कलय(endlow, BUILD_LOWRAM_अन्तः-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    संरचना allocinfo_s *info = alloc_find_निम्नतमम्(&ZoneFSeg);
    u32 आकार = info->range_अन्तः - info->range_start;
    स्मृतिं_पूरय(memremap(info->range_start, आकार), 0, आकार);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_अन्तः, आधारः, info->range_start, info->range_अन्तः);

    // Give back unused high ram.
    info = alloc_find_निम्नतमम्(&Zone_उच्चः);
    यदि (info) {
        u32 giveback = ALIGN_अधस्तात्(info->range_अन्तः-info->range_start, PAGE_आकार);
        e820_सङ्कलय(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_आकार();
}
