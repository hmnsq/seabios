// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_BDA
#அந்தர்பாவ "config.h" // BUILD_BIOS_ADDR
#அந்தர்பாவ "e820map.h" // struct e820entry
#அந்தர்பாவ "list.h" // hlist_node
#அந்தர்பாவ "malloc.h" // _malloc
#அந்தர்பாவ "memmap.h" // PAGE_SIZE
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "stacks.h" // wait_preempt
#அந்தர்பாவ "std/optionrom.h" // OPTION_ROM_ALIGN
#அந்தர்பாவ "string.h" // memset

// Information on a reserved area.
கட்டமைப்பு allocinfo_s {
    கட்டமைப்பு hlist_முனை முனை;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
கட்டமைப்பு allocdetail_s {
    கட்டமைப்பு allocinfo_s detailinfo;
    கட்டமைப்பு allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
கட்டமைப்பு zone_s {
    கட்டமைப்பு hlist_head head;
};

கட்டமைப்பு zone_s Zone_கீழ் VARVERIFY32INIT, Zone_உயர்வு VARVERIFY32INIT;
கட்டமைப்பு zone_s ZoneFSeg VARVERIFY32INIT;
கட்டமைப்பு zone_s Zone_Tmp_கீழ் VARVERIFY32INIT, Zone_Tmp_உயர்வு VARVERIFY32INIT;

நிலையான கட்டமைப்பு zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_கீழ், &Zone_கீழ், &ZoneFSeg, &Zone_Tmp_உயர்வு, &Zone_உயர்வு
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
நிலையான u32
alloc_new(கட்டமைப்பு zone_s *zone, u32 size, u32 align, கட்டமைப்பு allocinfo_s *fill)
{
    கட்டமைப்பு allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, முனை) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        எனில் (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->முனை, &info->முனை);
            திரும்பு new_range_end;
        }
    }
    திரும்பு 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
நிலையான கட்டமைப்பு allocdetail_s *
alloc_new_detail(கட்டமைப்பு allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_உயர்வு, அளவு(கட்டமைப்பு allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    எனில் (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_கீழ், அளவு(கட்டமைப்பு allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        எனில் (!detail_addr) {
            warn_noalloc();
            திரும்பு வெற்று;
        }
    }
    கட்டமைப்பு allocdetail_s *detail = memremap(detail_addr, அளவு(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, அளவு(*detail));
    hlist_replace(&temp->detailinfo.முனை, &detail->detailinfo.முனை);
    hlist_replace(&temp->datainfo.முனை, &detail->datainfo.முனை);
    திரும்பு detail;
}

// Add new memory to a zone
நிலையான வெறுமை
alloc_add(கட்டமைப்பு zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    கட்டமைப்பு allocinfo_s *info;
    கட்டமைப்பு hlist_முனை **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, முனை) {
        எனில் (info->range_start < start)
            நிறுத்து;
    }

    // Add space using temporary allocation info.
    கட்டமைப்பு allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.முனை, pprev);

    // Allocate final allocation info.
    கட்டமைப்பு allocdetail_s *detail = alloc_new_detail(&tempdetail);
    எனில் (!detail)
        hlist_del(&tempdetail.datainfo.முனை);
}

// Release space allocated with alloc_new()
நிலையான வெறுமை
alloc_free(கட்டமைப்பு allocinfo_s *info)
{
    கட்டமைப்பு allocinfo_s *next = container_of_or_null(
        info->முனை.next, கட்டமைப்பு allocinfo_s, முனை);
    எனில் (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->முனை);
}

// Search all zones for an allocation obtained from alloc_new()
நிலையான கட்டமைப்பு allocinfo_s *
alloc_find(u32 data)
{
    முழு i;
    க்கு (i=0; i<ARRAY_SIZE(Zones); i++) {
        கட்டமைப்பு allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, முனை) {
            எனில் (info->range_start == data)
                திரும்பு info;
        }
    }
    திரும்பு வெற்று;
}

// Find the lowest memory range added by alloc_add()
நிலையான கட்டமைப்பு allocinfo_s *
alloc_find_lowest(கட்டமைப்பு zone_s *zone)
{
    கட்டமைப்பு allocinfo_s *info, *last = வெற்று;
    hlist_for_each_entry(info, &zone->head, முனை) {
        last = info;
    }
    திரும்பு last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
நிலையான முழு
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    எனில் (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        திரும்பு -1;

    // Do copy
    memmove((வெறுமை*)newebda, (வெறுமை*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    திரும்பு 0;
}

// Support expanding the ZoneLow dynamically.
நிலையான u32
zonelow_expand(u32 size, u32 align, கட்டமைப்பு allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    எனில் (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_கீழ், size, align, fill);
        எனில் (data)
            திரும்பு data;
    }

    கட்டமைப்பு allocinfo_s *info = alloc_find_lowest(&Zone_கீழ்);
    எனில் (!info)
        திரும்பு 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    எனில் (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        திரும்பு alloc_new(&Zone_கீழ், size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    எனில் (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    எனில் (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        திரும்பு 0;

    // Move ebda
    முழு ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    எனில் (ret)
        திரும்பு 0;

    // Update zone
    எனில் (ebda_end == bottom)
        info->range_start = newbottom;
    இல்லையெனில்
        alloc_add(&Zone_கீழ், newbottom, ebda_end);

    திரும்பு alloc_new(&Zone_கீழ், size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(கட்டமைப்பு zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    எனில் (!size)
        திரும்பு 0;

    // Find and reserve space for main allocation
    கட்டமைப்பு allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    எனில் (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_கீழ்)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    எனில் (!data)
        திரும்பு 0;

    // Find and reserve space for bookkeeping.
    கட்டமைப்பு allocdetail_s *detail = alloc_new_detail(&tempdetail);
    எனில் (!detail) {
        alloc_free(&tempdetail.datainfo);
        திரும்பு 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    திரும்பு data;
}

// Allocate virtual memory from the given zone
வெறுமை * __malloc
_malloc(கட்டமைப்பு zone_s *zone, u32 size, u32 align)
{
    திரும்பு memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
முழு
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    கட்டமைப்பு allocinfo_s *info = alloc_find(data);
    எனில் (!info || data == virt_to_phys(info) || !info->alloc_size)
        திரும்பு -1;
    கட்டமைப்பு allocdetail_s *detail = container_of(
        info, கட்டமைப்பு allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    திரும்பு 0;
}

வெறுமை
விடுவி(வெறுமை *data)
{
    எனில் (!data)
        திரும்பு;
    முழு ret = malloc_pfree(virt_to_phys(data));
    எனில் (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(கட்டமைப்பு zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    கட்டமைப்பு allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, முனை) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        எனில் (space > maxspace)
            maxspace = space;
    }

    எனில் (zone != &Zone_Tmp_உயர்வு && zone != &Zone_Tmp_கீழ்)
        திரும்பு maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(அளவு(கட்டமைப்பு allocdetail_s), MALLOC_MIN_ALIGN);
    எனில் (maxspace <= reserve)
        திரும்பு 0;
    திரும்பு maxspace - reserve;
}

// Set a handle associated with an allocation.
வெறுமை
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    கட்டமைப்பு allocinfo_s *info = alloc_find(data);
    எனில் (!info || data == virt_to_phys(info) || !info->alloc_size)
        திரும்பு;
    கட்டமைப்பு allocdetail_s *detail = container_of(
        info, கட்டமைப்பு allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    முழு i;
    க்கு (i=0; i<ARRAY_SIZE(Zones); i++) {
        கட்டமைப்பு allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, முனை) {
            எனில் (info->range_start != virt_to_phys(info))
                தொடர்;
            கட்டமைப்பு allocdetail_s *detail = container_of(
                info, கட்டமைப்பு allocdetail_s, detailinfo);
            எனில் (detail->handle == handle)
                திரும்பு detail->datainfo.range_start;
        }
    }
    திரும்பு 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

நிலையான u32 RomEnd = BUILD_ROM_START;
நிலையான கட்டமைப்பு allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(வெறுமை)
{
    எனில் (CONFIG_MALLOC_UPPERMEMORY)
        திரும்பு ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    திரும்பு குறியீடு(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(வெறுமை)
{
    திரும்பு RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
கட்டமைப்பு rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    எனில் (newend > rom_get_max())
        திரும்பு வெற்று;
    எனில் (CONFIG_MALLOC_UPPERMEMORY) {
        எனில் (newend < குறியீடு(zonelow_base))
            newend = குறியீடு(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    திரும்பு (வெறுமை*)RomEnd;
}

// Confirm space as in use by an optionrom.
முழு
rom_confirm(u32 size)
{
    வெறுமை *new = rom_reserve(size);
    எனில் (!new) {
        warn_noalloc();
        திரும்பு -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    திரும்பு 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

வெறுமை
malloc_preinit(வெறுமை)
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
    முழு i;
    க்கு (i=e820_மொத்தம்-1; i>=0; i--) {
        கட்டமைப்பு e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        எனில் (end < 1024*1024)
            நிறுத்து;
        எனில் (en->type != E820_RAM || end > 0xffffffff)
            தொடர்;
        u32 s = en->start, e = end;
        எனில் (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            எனில் (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } இல்லையெனில் எனில் (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_உயர்வு, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_கீழ், BUILD_அடுக்கு_ADDR, BUILD_EBDA_MINIMUM);
    எனில் (highram_start) {
        alloc_add(&Zone_உயர்வு, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

வெறுமை
malloc_csm_preinit(u32 கீழ்_pmm, u32 கீழ்_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    எனில் (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_உயர்வு, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_உயர்வு, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } இல்லையெனில் {
        alloc_add(&Zone_Tmp_உயர்வு, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_கீழ், கீழ்_pmm, கீழ்_pmm + கீழ்_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
நிலையான வெறுமை
calcRamSize(வெறுமை)
{
    u32 rs = 0;
    முழு i;
    க்கு (i=e820_மொத்தம்-1; i>=0; i--) {
        கட்டமைப்பு e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        எனில் (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            நிறுத்து;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
வெறுமை
malloc_init(வெறுமை)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    எனில் (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        முழு i;
        க்கு (i=0; i<ARRAY_SIZE(Zones); i++) {
            கட்டமைப்பு zone_s *zone = Zones[i];
            எனில் (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , குறியீடு(varlow_end) - குறியீடு(varlow_start));
    எனில் (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_கீழ், குறியீடு(zonelow_base) + OPROM_HEADER_RESERVE
                  , குறியீடு(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_கீழ்);
    } இல்லையெனில் {
        alloc_add(&Zone_கீழ், ALIGN_DOWN(குறியீடு(final_varlow_start), 1024)
                  , குறியீடு(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , குறியீடு(zonefseg_end) - குறியீடு(zonefseg_start));
    alloc_add(&ZoneFSeg, குறியீடு(zonefseg_start), குறியீடு(zonefseg_end));

    calcRamSize();
}

வெறுமை
malloc_prepboot(வெறுமை)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((வெறுமை*)RomEnd, 0, base-RomEnd);
    எனில் (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        கட்டமைப்பு rom_header *dummyrom = (வெறுமை*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        முழு size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    கட்டமைப்பு allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_உயர்வு);
    எனில் (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
