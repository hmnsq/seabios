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
    संरचना hlist_गाठ गाठ;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
संरचना allocdetail_s {
    संरचना allocinfo_s detailinfo;
    संरचना allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
संरचना zone_s {
    संरचना hlist_head head;
};

संरचना zone_s Zone_खालचा VARVERIFY32INIT, Zone_वरचा VARVERIFY32INIT;
संरचना zone_s ZoneFSeg VARVERIFY32INIT;
संरचना zone_s Zone_Tmp_खालचा VARVERIFY32INIT, Zone_Tmp_वरचा VARVERIFY32INIT;

स्थिर संरचना zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_खालचा, &Zone_खालचा, &ZoneFSeg, &Zone_Tmp_वरचा, &Zone_वरचा
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
स्थिर u32
alloc_new(संरचना zone_s *zone, u32 size, u32 align, संरचना allocinfo_s *fill)
{
    संरचना allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, गाठ) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        जर (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->गाठ, &info->गाठ);
            परत new_range_end;
        }
    }
    परत 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
स्थिर संरचना allocdetail_s *
alloc_new_detail(संरचना allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_वरचा, आकार(संरचना allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    जर (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_खालचा, आकार(संरचना allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        जर (!detail_addr) {
            warn_noalloc();
            परत रिक्त;
        }
    }
    संरचना allocdetail_s *detail = memremap(detail_addr, आकार(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    स_नकल(detail, temp, आकार(*detail));
    hlist_replace(&temp->detailinfo.गाठ, &detail->detailinfo.गाठ);
    hlist_replace(&temp->datainfo.गाठ, &detail->datainfo.गाठ);
    परत detail;
}

// Add new memory to a zone
स्थिर शून्य
alloc_add(संरचना zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    संरचना allocinfo_s *info;
    संरचना hlist_गाठ **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, गाठ) {
        जर (info->range_start < start)
            थांबा;
    }

    // Add space using temporary allocation info.
    संरचना allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.गाठ, pprev);

    // Allocate final allocation info.
    संरचना allocdetail_s *detail = alloc_new_detail(&tempdetail);
    जर (!detail)
        hlist_del(&tempdetail.datainfo.गाठ);
}

// Release space allocated with alloc_new()
स्थिर शून्य
alloc_free(संरचना allocinfo_s *info)
{
    संरचना allocinfo_s *next = container_of_or_null(
        info->गाठ.next, संरचना allocinfo_s, गाठ);
    जर (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->गाठ);
}

// Search all zones for an allocation obtained from alloc_new()
स्थिर संरचना allocinfo_s *
alloc_find(u32 data)
{
    पूर्णांक i;
    साठी (i=0; i<ARRAY_SIZE(Zones); i++) {
        संरचना allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, गाठ) {
            जर (info->range_start == data)
                परत info;
        }
    }
    परत रिक्त;
}

// Find the lowest memory range added by alloc_add()
स्थिर संरचना allocinfo_s *
alloc_find_lowest(संरचना zone_s *zone)
{
    संरचना allocinfo_s *info, *last = रिक्त;
    hlist_for_each_entry(info, &zone->head, गाठ) {
        last = info;
    }
    परत last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
स्थिर पूर्णांक
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    जर (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        परत -1;

    // Do copy
    स_हटाओ((शून्य*)newebda, (शून्य*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    परत 0;
}

// Support expanding the ZoneLow dynamically.
स्थिर u32
zonelow_expand(u32 size, u32 align, संरचना allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    जर (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_खालचा, size, align, fill);
        जर (data)
            परत data;
    }

    संरचना allocinfo_s *info = alloc_find_lowest(&Zone_खालचा);
    जर (!info)
        परत 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    जर (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        परत alloc_new(&Zone_खालचा, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    जर (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    जर (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        परत 0;

    // Move ebda
    पूर्णांक ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    जर (ret)
        परत 0;

    // Update zone
    जर (ebda_end == bottom)
        info->range_start = newbottom;
    अन्यथा
        alloc_add(&Zone_खालचा, newbottom, ebda_end);

    परत alloc_new(&Zone_खालचा, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(संरचना zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    जर (!size)
        परत 0;

    // Find and reserve space for main allocation
    संरचना allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    जर (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_खालचा)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    जर (!data)
        परत 0;

    // Find and reserve space for bookkeeping.
    संरचना allocdetail_s *detail = alloc_new_detail(&tempdetail);
    जर (!detail) {
        alloc_free(&tempdetail.datainfo);
        परत 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    परत data;
}

// Allocate virtual memory from the given zone
शून्य * __malloc
_malloc(संरचना zone_s *zone, u32 size, u32 align)
{
    परत memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
पूर्णांक
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    संरचना allocinfo_s *info = alloc_find(data);
    जर (!info || data == virt_to_phys(info) || !info->alloc_size)
        परत -1;
    संरचना allocdetail_s *detail = container_of(
        info, संरचना allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    परत 0;
}

शून्य
मुक्त(शून्य *data)
{
    जर (!data)
        परत;
    पूर्णांक ret = malloc_pfree(virt_to_phys(data));
    जर (ret)
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
    hlist_for_each_entry(info, &zone->head, गाठ) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        जर (space > maxspace)
            maxspace = space;
    }

    जर (zone != &Zone_Tmp_वरचा && zone != &Zone_Tmp_खालचा)
        परत maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(आकार(संरचना allocdetail_s), MALLOC_MIN_ALIGN);
    जर (maxspace <= reserve)
        परत 0;
    परत maxspace - reserve;
}

// Set a handle associated with an allocation.
शून्य
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    संरचना allocinfo_s *info = alloc_find(data);
    जर (!info || data == virt_to_phys(info) || !info->alloc_size)
        परत;
    संरचना allocdetail_s *detail = container_of(
        info, संरचना allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    पूर्णांक i;
    साठी (i=0; i<ARRAY_SIZE(Zones); i++) {
        संरचना allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, गाठ) {
            जर (info->range_start != virt_to_phys(info))
                सुरूठेवा;
            संरचना allocdetail_s *detail = container_of(
                info, संरचना allocdetail_s, detailinfo);
            जर (detail->handle == handle)
                परत detail->datainfo.range_start;
        }
    }
    परत 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

स्थिर u32 RomEnd = BUILD_ROM_START;
स्थिर संरचना allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(शून्य)
{
    जर (CONFIG_MALLOC_UPPERMEMORY)
        परत ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    परत चिन्ह(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(शून्य)
{
    परत RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
संरचना rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    जर (newend > rom_get_max())
        परत रिक्त;
    जर (CONFIG_MALLOC_UPPERMEMORY) {
        जर (newend < चिन्ह(zonelow_base))
            newend = चिन्ह(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    परत (शून्य*)RomEnd;
}

// Confirm space as in use by an optionrom.
पूर्णांक
rom_confirm(u32 size)
{
    शून्य *new = rom_reserve(size);
    जर (!new) {
        warn_noalloc();
        परत -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    परत 0;
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
    e820_remove(BUILD_LOWRAM_END, BUILD_BIOS_ADDR-BUILD_LOWRAM_END);

    // Mark known areas as reserved.
    e820_add(BUILD_BIOS_ADDR, BUILD_BIOS_SIZE, E820_RESERVED);

    // Populate temp high ram
    u32 highram_start = 0;
    u32 highram_size = 0;
    पूर्णांक i;
    साठी (i=e820_मोजणी-1; i>=0; i--) {
        संरचना e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        जर (end < 1024*1024)
            थांबा;
        जर (en->type != E820_RAM || end > 0xffffffff)
            सुरूठेवा;
        u32 s = en->start, e = end;
        जर (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            जर (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } अन्यथा जर (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_वरचा, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_खालचा, BUILD_चळत_ADDR, BUILD_EBDA_MINIMUM);
    जर (highram_start) {
        alloc_add(&Zone_वरचा, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

शून्य
malloc_csm_preinit(u32 खालचा_pmm, u32 खालचा_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    जर (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_वरचा, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_वरचा, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } अन्यथा {
        alloc_add(&Zone_Tmp_वरचा, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_खालचा, खालचा_pmm, खालचा_pmm + खालचा_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
स्थिर शून्य
calcRamSize(शून्य)
{
    u32 rs = 0;
    पूर्णांक i;
    साठी (i=e820_मोजणी-1; i>=0; i--) {
        संरचना e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        जर (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            थांबा;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
शून्य
malloc_init(शून्य)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    जर (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        पूर्णांक i;
        साठी (i=0; i<ARRAY_SIZE(Zones); i++) {
            संरचना zone_s *zone = Zones[i];
            जर (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    स_हटाओ(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , चिन्ह(varlow_end) - चिन्ह(varlow_start));
    जर (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_खालचा, चिन्ह(zonelow_base) + OPROM_HEADER_RESERVE
                  , चिन्ह(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_खालचा);
    } अन्यथा {
        alloc_add(&Zone_खालचा, ALIGN_DOWN(चिन्ह(final_varlow_start), 1024)
                  , चिन्ह(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    स_रखो(VSYMBOL(zonefseg_start), 0
           , चिन्ह(zonefseg_end) - चिन्ह(zonefseg_start));
    alloc_add(&ZoneFSeg, चिन्ह(zonefseg_start), चिन्ह(zonefseg_end));

    calcRamSize();
}

शून्य
malloc_prepboot(शून्य)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    स_रखो((शून्य*)RomEnd, 0, base-RomEnd);
    जर (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        संरचना rom_header *dummyrom = (शून्य*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        पूर्णांक size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    संरचना allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    स_रखो(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_वरचा);
    जर (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
