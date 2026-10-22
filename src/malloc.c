// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_BDA
#অন্তর্ভাব "config.h" // BUILD_BIOS_ADDR
#অন্তর্ভাব "e820map.h" // struct e820entry
#অন্তর্ভাব "list.h" // hlist_node
#অন্তর্ভাব "malloc.h" // _malloc
#অন্তর্ভাব "memmap.h" // PAGE_SIZE
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "stacks.h" // wait_preempt
#অন্তর্ভাব "std/optionrom.h" // OPTION_ROM_ALIGN
#অন্তর্ভাব "string.h" // memset

// Information on a reserved area.
সংরচনা allocinfo_s {
    সংরচনা hlist_গ্রন্থি গ্রন্থি;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
সংরচনা allocdetail_s {
    সংরচনা allocinfo_s detailinfo;
    সংরচনা allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
সংরচনা zone_s {
    সংরচনা hlist_head head;
};

সংরচনা zone_s Zone_নিচের VARVERIFY32INIT, Zone_উপরের VARVERIFY32INIT;
সংরচনা zone_s ZoneFSeg VARVERIFY32INIT;
সংরচনা zone_s Zone_Tmp_নিচের VARVERIFY32INIT, Zone_Tmp_উপরের VARVERIFY32INIT;

স্থির সংরচনা zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_নিচের, &Zone_নিচের, &ZoneFSeg, &Zone_Tmp_উপরের, &Zone_উপরের
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
স্থির u32
alloc_new(সংরচনা zone_s *zone, u32 size, u32 align, সংরচনা allocinfo_s *fill)
{
    সংরচনা allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, গ্রন্থি) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        যদি (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->গ্রন্থি, &info->গ্রন্থি);
            প্রত্যাগম new_range_end;
        }
    }
    প্রত্যাগম 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
স্থির সংরচনা allocdetail_s *
alloc_new_detail(সংরচনা allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_উপরের, পরিমাণ(সংরচনা allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    যদি (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_নিচের, পরিমাণ(সংরচনা allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        যদি (!detail_addr) {
            warn_noalloc();
            প্রত্যাগম রিক্ত;
        }
    }
    সংরচনা allocdetail_s *detail = memremap(detail_addr, পরিমাণ(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, পরিমাণ(*detail));
    hlist_replace(&temp->detailinfo.গ্রন্থি, &detail->detailinfo.গ্রন্থি);
    hlist_replace(&temp->datainfo.গ্রন্থি, &detail->datainfo.গ্রন্থি);
    প্রত্যাগম detail;
}

// Add new memory to a zone
স্থির শূন্য
alloc_add(সংরচনা zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    সংরচনা allocinfo_s *info;
    সংরচনা hlist_গ্রন্থি **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, গ্রন্থি) {
        যদি (info->range_start < start)
            ভঙ্গ;
    }

    // Add space using temporary allocation info.
    সংরচনা allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.গ্রন্থি, pprev);

    // Allocate final allocation info.
    সংরচনা allocdetail_s *detail = alloc_new_detail(&tempdetail);
    যদি (!detail)
        hlist_del(&tempdetail.datainfo.গ্রন্থি);
}

// Release space allocated with alloc_new()
স্থির শূন্য
alloc_free(সংরচনা allocinfo_s *info)
{
    সংরচনা allocinfo_s *next = container_of_or_null(
        info->গ্রন্থি.next, সংরচনা allocinfo_s, গ্রন্থি);
    যদি (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->গ্রন্থি);
}

// Search all zones for an allocation obtained from alloc_new()
স্থির সংরচনা allocinfo_s *
alloc_find(u32 data)
{
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<ARRAY_SIZE(Zones); i++) {
        সংরচনা allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, গ্রন্থি) {
            যদি (info->range_start == data)
                প্রত্যাগম info;
        }
    }
    প্রত্যাগম রিক্ত;
}

// Find the lowest memory range added by alloc_add()
স্থির সংরচনা allocinfo_s *
alloc_find_lowest(সংরচনা zone_s *zone)
{
    সংরচনা allocinfo_s *info, *last = রিক্ত;
    hlist_for_each_entry(info, &zone->head, গ্রন্থি) {
        last = info;
    }
    প্রত্যাগম last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
স্থির পূর্ণাঙ্ক
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    যদি (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        প্রত্যাগম -1;

    // Do copy
    memmove((শূন্য*)newebda, (শূন্য*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    প্রত্যাগম 0;
}

// Support expanding the ZoneLow dynamically.
স্থির u32
zonelow_expand(u32 size, u32 align, সংরচনা allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    যদি (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_নিচের, size, align, fill);
        যদি (data)
            প্রত্যাগম data;
    }

    সংরচনা allocinfo_s *info = alloc_find_lowest(&Zone_নিচের);
    যদি (!info)
        প্রত্যাগম 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    যদি (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        প্রত্যাগম alloc_new(&Zone_নিচের, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    যদি (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    যদি (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        প্রত্যাগম 0;

    // Move ebda
    পূর্ণাঙ্ক ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    যদি (ret)
        প্রত্যাগম 0;

    // Update zone
    যদি (ebda_end == bottom)
        info->range_start = newbottom;
    অন্যথা
        alloc_add(&Zone_নিচের, newbottom, ebda_end);

    প্রত্যাগম alloc_new(&Zone_নিচের, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(সংরচনা zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    যদি (!size)
        প্রত্যাগম 0;

    // Find and reserve space for main allocation
    সংরচনা allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    যদি (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_নিচের)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    যদি (!data)
        প্রত্যাগম 0;

    // Find and reserve space for bookkeeping.
    সংরচনা allocdetail_s *detail = alloc_new_detail(&tempdetail);
    যদি (!detail) {
        alloc_free(&tempdetail.datainfo);
        প্রত্যাগম 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    প্রত্যাগম data;
}

// Allocate virtual memory from the given zone
শূন্য * __malloc
_malloc(সংরচনা zone_s *zone, u32 size, u32 align)
{
    প্রত্যাগম memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
পূর্ণাঙ্ক
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    সংরচনা allocinfo_s *info = alloc_find(data);
    যদি (!info || data == virt_to_phys(info) || !info->alloc_size)
        প্রত্যাগম -1;
    সংরচনা allocdetail_s *detail = container_of(
        info, সংরচনা allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    প্রত্যাগম 0;
}

শূন্য
মুক্ত(শূন্য *data)
{
    যদি (!data)
        প্রত্যাগম;
    পূর্ণাঙ্ক ret = malloc_pfree(virt_to_phys(data));
    যদি (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(সংরচনা zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    সংরচনা allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, গ্রন্থি) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        যদি (space > maxspace)
            maxspace = space;
    }

    যদি (zone != &Zone_Tmp_উপরের && zone != &Zone_Tmp_নিচের)
        প্রত্যাগম maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(পরিমাণ(সংরচনা allocdetail_s), MALLOC_MIN_ALIGN);
    যদি (maxspace <= reserve)
        প্রত্যাগম 0;
    প্রত্যাগম maxspace - reserve;
}

// Set a handle associated with an allocation.
শূন্য
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    সংরচনা allocinfo_s *info = alloc_find(data);
    যদি (!info || data == virt_to_phys(info) || !info->alloc_size)
        প্রত্যাগম;
    সংরচনা allocdetail_s *detail = container_of(
        info, সংরচনা allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<ARRAY_SIZE(Zones); i++) {
        সংরচনা allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, গ্রন্থি) {
            যদি (info->range_start != virt_to_phys(info))
                অনুবর্ত;
            সংরচনা allocdetail_s *detail = container_of(
                info, সংরচনা allocdetail_s, detailinfo);
            যদি (detail->handle == handle)
                প্রত্যাগম detail->datainfo.range_start;
        }
    }
    প্রত্যাগম 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

স্থির u32 RomEnd = BUILD_ROM_START;
স্থির সংরচনা allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(শূন্য)
{
    যদি (CONFIG_MALLOC_UPPERMEMORY)
        প্রত্যাগম ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    প্রত্যাগম প্রতীক(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(শূন্য)
{
    প্রত্যাগম RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
সংরচনা rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    যদি (newend > rom_get_max())
        প্রত্যাগম রিক্ত;
    যদি (CONFIG_MALLOC_UPPERMEMORY) {
        যদি (newend < প্রতীক(zonelow_base))
            newend = প্রতীক(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    প্রত্যাগম (শূন্য*)RomEnd;
}

// Confirm space as in use by an optionrom.
পূর্ণাঙ্ক
rom_confirm(u32 size)
{
    শূন্য *new = rom_reserve(size);
    যদি (!new) {
        warn_noalloc();
        প্রত্যাগম -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    প্রত্যাগম 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

শূন্য
malloc_preinit(শূন্য)
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
    পূর্ণাঙ্ক i;
    প্রতি (i=e820_গুনতি-1; i>=0; i--) {
        সংরচনা e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        যদি (end < 1024*1024)
            ভঙ্গ;
        যদি (en->type != E820_RAM || end > 0xffffffff)
            অনুবর্ত;
        u32 s = en->start, e = end;
        যদি (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            যদি (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } অন্যথা যদি (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_উপরের, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_নিচের, BUILD_স্তূপ_ADDR, BUILD_EBDA_MINIMUM);
    যদি (highram_start) {
        alloc_add(&Zone_উপরের, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

শূন্য
malloc_csm_preinit(u32 নিচের_pmm, u32 নিচের_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    যদি (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_উপরের, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_উপরের, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } অন্যথা {
        alloc_add(&Zone_Tmp_উপরের, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_নিচের, নিচের_pmm, নিচের_pmm + নিচের_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
স্থির শূন্য
calcRamSize(শূন্য)
{
    u32 rs = 0;
    পূর্ণাঙ্ক i;
    প্রতি (i=e820_গুনতি-1; i>=0; i--) {
        সংরচনা e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        যদি (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            ভঙ্গ;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
শূন্য
malloc_init(শূন্য)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    যদি (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        পূর্ণাঙ্ক i;
        প্রতি (i=0; i<ARRAY_SIZE(Zones); i++) {
            সংরচনা zone_s *zone = Zones[i];
            যদি (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , প্রতীক(varlow_end) - প্রতীক(varlow_start));
    যদি (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_নিচের, প্রতীক(zonelow_base) + OPROM_HEADER_RESERVE
                  , প্রতীক(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_নিচের);
    } অন্যথা {
        alloc_add(&Zone_নিচের, ALIGN_DOWN(প্রতীক(final_varlow_start), 1024)
                  , প্রতীক(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , প্রতীক(zonefseg_end) - প্রতীক(zonefseg_start));
    alloc_add(&ZoneFSeg, প্রতীক(zonefseg_start), প্রতীক(zonefseg_end));

    calcRamSize();
}

শূন্য
malloc_prepboot(শূন্য)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((শূন্য*)RomEnd, 0, base-RomEnd);
    যদি (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        সংরচনা rom_header *dummyrom = (শূন্য*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        পূর্ণাঙ্ক size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    সংরচনা allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_উপরের);
    যদি (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
