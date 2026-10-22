// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_BDA
#समावेश "config.h" // BUILD_BIOS_ADDR
#समावेश "e820map.h" // struct e820entry
#समावेश "list.h" // hlist_node
#समावेश "malloc.h" // _malloc
#समावेश "memmap.h" // PAGE_SIZE
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // wait_preempt
#समावेश "std/optionrom.h" // OPTION_ROM_ALIGN
#समावेश "string.h" // memset

// Information on a reserved area.
काष्ठा allocinfo_s {
    काष्ठा hlist_गाँठ गाँठ;
    u32 range_start, range_end, alloc_आकार;
};

// Information on a tracked memory allocation.
काष्ठा allocdetail_s {
    काष्ठा allocinfo_s detailinfo;
    काष्ठा allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
काष्ठा zone_s {
    काष्ठा hlist_शीर्ष शीर्ष;
};

काष्ठा zone_s Zone_निचला VARVERIFY32INIT, Zone_ऊपरी VARVERIFY32INIT;
काष्ठा zone_s ZoneFSeg VARVERIFY32INIT;
काष्ठा zone_s Zone_Tmp_निचला VARVERIFY32INIT, Zone_Tmp_ऊपरी VARVERIFY32INIT;

अटल काष्ठा zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_निचला, &Zone_निचला, &ZoneFSeg, &Zone_Tmp_ऊपरी, &Zone_ऊपरी
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
अटल u32
alloc_new(काष्ठा zone_s *zone, u32 आकार, u32 align, काष्ठा allocinfo_s *fill)
{
    काष्ठा allocinfo_s *info;
    hlist_for_each_entry(info, &zone->शीर्ष, गाँठ) {
        u32 alloc_end = info->range_start + info->alloc_आकार;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - आकार, align);
        अगर (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_आकार = आकार;

            info->range_end = new_range_end;
            hlist_add_before(&fill->गाँठ, &info->गाँठ);
            वापस new_range_end;
        }
    }
    वापस 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
अटल काष्ठा allocdetail_s *
alloc_new_detail(काष्ठा allocdetail_s *अस्थायी)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_ऊपरी, माप(काष्ठा allocdetail_s)
                                , MALLOC_MIN_ALIGN, &अस्थायी->detailinfo);
    अगर (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_निचला, माप(काष्ठा allocdetail_s)
                                , MALLOC_MIN_ALIGN, &अस्थायी->detailinfo);
        अगर (!detail_addr) {
            warn_noalloc();
            वापस शून्य;
        }
    }
    काष्ठा allocdetail_s *detail = memremap(detail_addr, माप(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    स_नकल(detail, अस्थायी, माप(*detail));
    hlist_replace(&अस्थायी->detailinfo.गाँठ, &detail->detailinfo.गाँठ);
    hlist_replace(&अस्थायी->datainfo.गाँठ, &detail->datainfo.गाँठ);
    वापस detail;
}

// Add new memory to a zone
अटल व्योम
alloc_add(काष्ठा zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    काष्ठा allocinfo_s *info;
    काष्ठा hlist_गाँठ **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->शीर्ष, गाँठ) {
        अगर (info->range_start < start)
            अवरोध;
    }

    // Add space using temporary allocation info.
    काष्ठा allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_आकार = 0;
    hlist_add(&tempdetail.datainfo.गाँठ, pprev);

    // Allocate final allocation info.
    काष्ठा allocdetail_s *detail = alloc_new_detail(&tempdetail);
    अगर (!detail)
        hlist_del(&tempdetail.datainfo.गाँठ);
}

// Release space allocated with alloc_new()
अटल व्योम
alloc_free(काष्ठा allocinfo_s *info)
{
    काष्ठा allocinfo_s *अग्र = container_of_or_null(
        info->गाँठ.अग्र, काष्ठा allocinfo_s, गाँठ);
    अगर (अग्र && अग्र->range_end == info->range_start)
        अग्र->range_end = info->range_end;
    hlist_del(&info->गाँठ);
}

// Search all zones for an allocation obtained from alloc_new()
अटल काष्ठा allocinfo_s *
alloc_find(u32 तथ्य)
{
    पूर्णांक i;
    क्रम (i=0; i<ARRAY_आकार(Zones); i++) {
        काष्ठा allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->शीर्ष, गाँठ) {
            अगर (info->range_start == तथ्य)
                वापस info;
        }
    }
    वापस शून्य;
}

// Find the lowest memory range added by alloc_add()
अटल काष्ठा allocinfo_s *
alloc_find_lowest(काष्ठा zone_s *zone)
{
    काष्ठा allocinfo_s *info, *last = शून्य;
    hlist_for_each_entry(info, &zone->शीर्ष, गाँठ) {
        last = info;
    }
    वापस last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
अटल पूर्णांक
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_आकार)
{
    u32 lowram = GET_BDA(mem_आकार_kb) * 1024;
    अगर (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        वापस -1;

    // Do copy
    स_हटाओ((व्योम*)newebda, (व्योम*)oldebda, ebda_आकार * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_आकार_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    वापस 0;
}

// Support expanding the ZoneLow dynamically.
अटल u32
zonelow_expand(u32 आकार, u32 align, काष्ठा allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    अगर (unlikely(wait_preempt())) {
        u32 तथ्य = alloc_new(&Zone_निचला, आकार, align, fill);
        अगर (तथ्य)
            वापस तथ्य;
    }

    काष्ठा allocinfo_s *info = alloc_find_lowest(&Zone_निचला);
    अगर (!info)
        वापस 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - आकार, align);
    u32 bottom = info->range_start + info->alloc_आकार;
    अगर (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        वापस alloc_new(&Zone_निचला, आकार, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_आकार = GET_EBDA(ebda_seg, आकार);
    u32 ebda_end = ebda_pos + ebda_आकार * 1024;
    अगर (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - आकार, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_आकार * 1024, 1024);
    अगर (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        वापस 0;

    // Move ebda
    पूर्णांक ret = relocate_ebda(newebda, ebda_pos, ebda_आकार);
    अगर (ret)
        वापस 0;

    // Update zone
    अगर (ebda_end == bottom)
        info->range_start = newbottom;
    अन्यथा
        alloc_add(&Zone_निचला, newbottom, ebda_end);

    वापस alloc_new(&Zone_निचला, आकार, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(काष्ठा zone_s *zone, u32 आकार, u32 align)
{
    ASSERT32FLAT();
    अगर (!आकार)
        वापस 0;

    // Find and reserve space for main allocation
    काष्ठा allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 तथ्य = alloc_new(zone, आकार, align, &tempdetail.datainfo);
    अगर (!CONFIG_MALLOC_UPPERMEMORY && !तथ्य && zone == &Zone_निचला)
        तथ्य = zonelow_expand(आकार, align, &tempdetail.datainfo);
    अगर (!तथ्य)
        वापस 0;

    // Find and reserve space for bookkeeping.
    काष्ठा allocdetail_s *detail = alloc_new_detail(&tempdetail);
    अगर (!detail) {
        alloc_free(&tempdetail.datainfo);
        वापस 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, आकार, align, तथ्य, detail);

    वापस तथ्य;
}

// Allocate virtual memory from the given zone
व्योम * __malloc
_malloc(काष्ठा zone_s *zone, u32 आकार, u32 align)
{
    वापस memremap(malloc_palloc(zone, आकार, align), आकार);
}

// Free a data block allocated with phys_alloc
पूर्णांक
malloc_pfree(u32 तथ्य)
{
    ASSERT32FLAT();
    काष्ठा allocinfo_s *info = alloc_find(तथ्य);
    अगर (!info || तथ्य == virt_to_phys(info) || !info->alloc_आकार)
        वापस -1;
    काष्ठा allocdetail_s *detail = container_of(
        info, काष्ठा allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", तथ्य, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    वापस 0;
}

व्योम
मुक्त(व्योम *तथ्य)
{
    अगर (!तथ्य)
        वापस;
    पूर्णांक ret = malloc_pfree(virt_to_phys(तथ्य));
    अगर (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(काष्ठा zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    काष्ठा allocinfo_s *info;
    hlist_for_each_entry(info, &zone->शीर्ष, गाँठ) {
        u32 space = info->range_end - info->range_start - info->alloc_आकार;
        अगर (space > maxspace)
            maxspace = space;
    }

    अगर (zone != &Zone_Tmp_ऊपरी && zone != &Zone_Tmp_निचला)
        वापस maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(माप(काष्ठा allocdetail_s), MALLOC_MIN_ALIGN);
    अगर (maxspace <= reserve)
        वापस 0;
    वापस maxspace - reserve;
}

// Set a handle associated with an allocation.
व्योम
malloc_sethandle(u32 तथ्य, u32 handle)
{
    ASSERT32FLAT();
    काष्ठा allocinfo_s *info = alloc_find(तथ्य);
    अगर (!info || तथ्य == virt_to_phys(info) || !info->alloc_आकार)
        वापस;
    काष्ठा allocdetail_s *detail = container_of(
        info, काष्ठा allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    पूर्णांक i;
    क्रम (i=0; i<ARRAY_आकार(Zones); i++) {
        काष्ठा allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->शीर्ष, गाँठ) {
            अगर (info->range_start != virt_to_phys(info))
                जारी;
            काष्ठा allocdetail_s *detail = container_of(
                info, काष्ठा allocdetail_s, detailinfo);
            अगर (detail->handle == handle)
                वापस detail->datainfo.range_start;
        }
    }
    वापस 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

अटल u32 RomEnd = BUILD_ROM_START;
अटल काष्ठा allocinfo_s *RomBase;

#घोषणा OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(व्योम)
{
    अगर (CONFIG_MALLOC_UPPERMEMORY)
        वापस ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    वापस प्रतीक(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(व्योम)
{
    वापस RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
काष्ठा rom_header *
rom_reserve(u32 आकार)
{
    u32 newend = ALIGN(RomEnd + आकार, OPTION_ROM_ALIGN);
    अगर (newend > rom_get_max())
        वापस शून्य;
    अगर (CONFIG_MALLOC_UPPERMEMORY) {
        अगर (newend < प्रतीक(zonelow_base))
            newend = प्रतीक(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    वापस (व्योम*)RomEnd;
}

// Confirm space as in use by an optionrom.
पूर्णांक
rom_confirm(u32 आकार)
{
    व्योम *new = rom_reserve(आकार);
    अगर (!new) {
        warn_noalloc();
        वापस -1;
    }
    RomEnd = ALIGN(RomEnd + आकार, OPTION_ROM_ALIGN);
    वापस 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

व्योम
malloc_preinit(व्योम)
{
    ASSERT32FLAT();
    dprintf(3, "malloc preinit\n");

    // Don't declare any memory between 0xa0000 and 0x100000
    e820_remove(BUILD_LOWRAM_END, BUILD_BIOS_ADDR-BUILD_LOWRAM_END);

    // Mark known areas as reserved.
    e820_add(BUILD_BIOS_ADDR, BUILD_BIOS_आकार, E820_RESERVED);

    // Populate temp high ram
    u32 highram_start = 0;
    u32 highram_आकार = 0;
    पूर्णांक i;
    क्रम (i=e820_गिनती-1; i>=0; i--) {
        काष्ठा e820entry *en = &e820_list[i];
        u64 end = en->start + en->आकार;
        अगर (end < 1024*1024)
            अवरोध;
        अगर (en->type != E820_RAM || end > 0xffffffff)
            जारी;
        u32 s = en->start, e = end;
        अगर (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            अगर (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_आकार = BUILD_MAX_HIGHTABLE;
            } अन्यथा अगर (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_आकार = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_ऊपरी, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_निचला, BUILD_ढेर_ADDR, BUILD_EBDA_MINIMUM);
    अगर (highram_start) {
        alloc_add(&Zone_ऊपरी, highram_start, highram_start + highram_आकार);
        e820_add(highram_start, highram_आकार, E820_RESERVED);
    }
}

व्योम
malloc_csm_preinit(u32 निचला_pmm, u32 निचला_pmm_आकार, u32 hi_pmm, u32 hi_pmm_आकार)
{
    ASSERT32FLAT();

    अगर (hi_pmm_आकार > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_आकार;
        alloc_add(&Zone_Tmp_ऊपरी, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_ऊपरी, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } अन्यथा {
        alloc_add(&Zone_Tmp_ऊपरी, hi_pmm, hi_pmm + hi_pmm_आकार);
    }
    alloc_add(&Zone_Tmp_निचला, निचला_pmm, निचला_pmm + निचला_pmm_आकार);
}

u32 Legacy_Ram_आकार VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
अटल व्योम
calc_Ram_आकार(व्योम)
{
    u32 rs = 0;
    पूर्णांक i;
    क्रम (i=e820_गिनती-1; i>=0; i--) {
        काष्ठा e820entry *en = &e820_list[i];
        u64 end = en->start + en->आकार;
        u32 type = en->type;
        अगर (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            अवरोध;
        }
    }
    Legacy_Ram_आकार = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
व्योम
malloc_init(व्योम)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    अगर (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        पूर्णांक i;
        क्रम (i=0; i<ARRAY_आकार(Zones); i++) {
            काष्ठा zone_s *zone = Zones[i];
            अगर (zone->शीर्ष.first)
                zone->शीर्ष.first->pprev = &zone->शीर्ष.first;
        }
    }

    // Initialize low-memory region
    स_हटाओ(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , प्रतीक(varlow_end) - प्रतीक(varlow_start));
    अगर (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_निचला, प्रतीक(zonelow_base) + OPROM_HEADER_RESERVE
                  , प्रतीक(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_निचला);
    } अन्यथा {
        alloc_add(&Zone_निचला, ALIGN_DOWN(प्रतीक(final_varlow_start), 1024)
                  , प्रतीक(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    स_रखो(VSYMBOL(zonefseg_start), 0
           , प्रतीक(zonefseg_end) - प्रतीक(zonefseg_start));
    alloc_add(&ZoneFSeg, प्रतीक(zonefseg_start), प्रतीक(zonefseg_end));

    calc_Ram_आकार();
}

व्योम
malloc_prepboot(व्योम)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    स_रखो((व्योम*)RomEnd, 0, base-RomEnd);
    अगर (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        काष्ठा rom_header *dummyrom = (व्योम*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        पूर्णांक आकार = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->आकार = (आकार > 255) ? 255 : आकार;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_आकार_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    काष्ठा allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 आकार = info->range_end - info->range_start;
    स_रखो(memremap(info->range_start, आकार), 0, आकार);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_ऊपरी);
    अगर (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_आकार);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_आकार();
}
