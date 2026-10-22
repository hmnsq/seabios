// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_BDA
#അന്തര്ഭാവ "config.h" // BUILD_BIOS_ADDR
#അന്തര്ഭാവ "e820map.h" // struct e820entry
#അന്തര്ഭാവ "list.h" // hlist_node
#അന്തര്ഭാവ "malloc.h" // _malloc
#അന്തര്ഭാവ "memmap.h" // PAGE_SIZE
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // wait_preempt
#അന്തര്ഭാവ "std/optionrom.h" // OPTION_ROM_ALIGN
#അന്തര്ഭാവ "string.h" // memset

// Information on a reserved area.
ഘടന allocinfo_s {
    ഘടന hlist_കണ്ണി കണ്ണി;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
ഘടന allocdetail_s {
    ഘടന allocinfo_s detailinfo;
    ഘടന allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
ഘടന zone_s {
    ഘടന hlist_head head;
};

ഘടന zone_s Zone_താഴെയുള്ള VARVERIFY32INIT, Zone_മുകളിലെ VARVERIFY32INIT;
ഘടന zone_s ZoneFSeg VARVERIFY32INIT;
ഘടന zone_s Zone_Tmp_താഴെയുള്ള VARVERIFY32INIT, Zone_Tmp_മുകളിലെ VARVERIFY32INIT;

സ്ഥിതം ഘടന zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_താഴെയുള്ള, &Zone_താഴെയുള്ള, &ZoneFSeg, &Zone_Tmp_മുകളിലെ, &Zone_മുകളിലെ
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
സ്ഥിതം u32
alloc_new(ഘടന zone_s *zone, u32 size, u32 align, ഘടന allocinfo_s *fill)
{
    ഘടന allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, കണ്ണി) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        എങ്കിൽ (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->കണ്ണി, &info->കണ്ണി);
            തിരികെ new_range_end;
        }
    }
    തിരികെ 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
സ്ഥിതം ഘടന allocdetail_s *
alloc_new_detail(ഘടന allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_മുകളിലെ, വലുപ്പം(ഘടന allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    എങ്കിൽ (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_താഴെയുള്ള, വലുപ്പം(ഘടന allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        എങ്കിൽ (!detail_addr) {
            warn_noalloc();
            തിരികെ ശൂന്യ;
        }
    }
    ഘടന allocdetail_s *detail = memremap(detail_addr, വലുപ്പം(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, വലുപ്പം(*detail));
    hlist_replace(&temp->detailinfo.കണ്ണി, &detail->detailinfo.കണ്ണി);
    hlist_replace(&temp->datainfo.കണ്ണി, &detail->datainfo.കണ്ണി);
    തിരികെ detail;
}

// Add new memory to a zone
സ്ഥിതം ശൂന്യം
alloc_add(ഘടന zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    ഘടന allocinfo_s *info;
    ഘടന hlist_കണ്ണി **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, കണ്ണി) {
        എങ്കിൽ (info->range_start < start)
            നിർത്തുക;
    }

    // Add space using temporary allocation info.
    ഘടന allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.കണ്ണി, pprev);

    // Allocate final allocation info.
    ഘടന allocdetail_s *detail = alloc_new_detail(&tempdetail);
    എങ്കിൽ (!detail)
        hlist_del(&tempdetail.datainfo.കണ്ണി);
}

// Release space allocated with alloc_new()
സ്ഥിതം ശൂന്യം
alloc_free(ഘടന allocinfo_s *info)
{
    ഘടന allocinfo_s *next = container_of_or_null(
        info->കണ്ണി.next, ഘടന allocinfo_s, കണ്ണി);
    എങ്കിൽ (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->കണ്ണി);
}

// Search all zones for an allocation obtained from alloc_new()
സ്ഥിതം ഘടന allocinfo_s *
alloc_find(u32 data)
{
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<ARRAY_SIZE(Zones); i++) {
        ഘടന allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, കണ്ണി) {
            എങ്കിൽ (info->range_start == data)
                തിരികെ info;
        }
    }
    തിരികെ ശൂന്യ;
}

// Find the lowest memory range added by alloc_add()
സ്ഥിതം ഘടന allocinfo_s *
alloc_find_lowest(ഘടന zone_s *zone)
{
    ഘടന allocinfo_s *info, *last = ശൂന്യ;
    hlist_for_each_entry(info, &zone->head, കണ്ണി) {
        last = info;
    }
    തിരികെ last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
സ്ഥിതം പൂർണ്ണസംഖ്യ
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    എങ്കിൽ (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        തിരികെ -1;

    // Do copy
    memmove((ശൂന്യം*)newebda, (ശൂന്യം*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    തിരികെ 0;
}

// Support expanding the ZoneLow dynamically.
സ്ഥിതം u32
zonelow_expand(u32 size, u32 align, ഘടന allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    എങ്കിൽ (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_താഴെയുള്ള, size, align, fill);
        എങ്കിൽ (data)
            തിരികെ data;
    }

    ഘടന allocinfo_s *info = alloc_find_lowest(&Zone_താഴെയുള്ള);
    എങ്കിൽ (!info)
        തിരികെ 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    എങ്കിൽ (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        തിരികെ alloc_new(&Zone_താഴെയുള്ള, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    എങ്കിൽ (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    എങ്കിൽ (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        തിരികെ 0;

    // Move ebda
    പൂർണ്ണസംഖ്യ ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    എങ്കിൽ (ret)
        തിരികെ 0;

    // Update zone
    എങ്കിൽ (ebda_end == bottom)
        info->range_start = newbottom;
    അല്ലെങ്കിൽ
        alloc_add(&Zone_താഴെയുള്ള, newbottom, ebda_end);

    തിരികെ alloc_new(&Zone_താഴെയുള്ള, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(ഘടന zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    എങ്കിൽ (!size)
        തിരികെ 0;

    // Find and reserve space for main allocation
    ഘടന allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    എങ്കിൽ (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_താഴെയുള്ള)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    എങ്കിൽ (!data)
        തിരികെ 0;

    // Find and reserve space for bookkeeping.
    ഘടന allocdetail_s *detail = alloc_new_detail(&tempdetail);
    എങ്കിൽ (!detail) {
        alloc_free(&tempdetail.datainfo);
        തിരികെ 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    തിരികെ data;
}

// Allocate virtual memory from the given zone
ശൂന്യം * __malloc
_malloc(ഘടന zone_s *zone, u32 size, u32 align)
{
    തിരികെ memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
പൂർണ്ണസംഖ്യ
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    ഘടന allocinfo_s *info = alloc_find(data);
    എങ്കിൽ (!info || data == virt_to_phys(info) || !info->alloc_size)
        തിരികെ -1;
    ഘടന allocdetail_s *detail = container_of(
        info, ഘടന allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    തിരികെ 0;
}

ശൂന്യം
സ്വതന്ത്രം(ശൂന്യം *data)
{
    എങ്കിൽ (!data)
        തിരികെ;
    പൂർണ്ണസംഖ്യ ret = malloc_pfree(virt_to_phys(data));
    എങ്കിൽ (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(ഘടന zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    ഘടന allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, കണ്ണി) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        എങ്കിൽ (space > maxspace)
            maxspace = space;
    }

    എങ്കിൽ (zone != &Zone_Tmp_മുകളിലെ && zone != &Zone_Tmp_താഴെയുള്ള)
        തിരികെ maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(വലുപ്പം(ഘടന allocdetail_s), MALLOC_MIN_ALIGN);
    എങ്കിൽ (maxspace <= reserve)
        തിരികെ 0;
    തിരികെ maxspace - reserve;
}

// Set a handle associated with an allocation.
ശൂന്യം
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    ഘടന allocinfo_s *info = alloc_find(data);
    എങ്കിൽ (!info || data == virt_to_phys(info) || !info->alloc_size)
        തിരികെ;
    ഘടന allocdetail_s *detail = container_of(
        info, ഘടന allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<ARRAY_SIZE(Zones); i++) {
        ഘടന allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, കണ്ണി) {
            എങ്കിൽ (info->range_start != virt_to_phys(info))
                തുടരുക;
            ഘടന allocdetail_s *detail = container_of(
                info, ഘടന allocdetail_s, detailinfo);
            എങ്കിൽ (detail->handle == handle)
                തിരികെ detail->datainfo.range_start;
        }
    }
    തിരികെ 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

സ്ഥിതം u32 RomEnd = BUILD_ROM_START;
സ്ഥിതം ഘടന allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(ശൂന്യം)
{
    എങ്കിൽ (CONFIG_MALLOC_UPPERMEMORY)
        തിരികെ ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    തിരികെ ചിഹ്നം(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(ശൂന്യം)
{
    തിരികെ RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
ഘടന rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    എങ്കിൽ (newend > rom_get_max())
        തിരികെ ശൂന്യ;
    എങ്കിൽ (CONFIG_MALLOC_UPPERMEMORY) {
        എങ്കിൽ (newend < ചിഹ്നം(zonelow_base))
            newend = ചിഹ്നം(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    തിരികെ (ശൂന്യം*)RomEnd;
}

// Confirm space as in use by an optionrom.
പൂർണ്ണസംഖ്യ
rom_confirm(u32 size)
{
    ശൂന്യം *new = rom_reserve(size);
    എങ്കിൽ (!new) {
        warn_noalloc();
        തിരികെ -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    തിരികെ 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

ശൂന്യം
malloc_preinit(ശൂന്യം)
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
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=e820_എണ്ണം-1; i>=0; i--) {
        ഘടന e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        എങ്കിൽ (end < 1024*1024)
            നിർത്തുക;
        എങ്കിൽ (en->type != E820_RAM || end > 0xffffffff)
            തുടരുക;
        u32 s = en->start, e = end;
        എങ്കിൽ (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            എങ്കിൽ (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } അല്ലെങ്കിൽ എങ്കിൽ (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_മുകളിലെ, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_താഴെയുള്ള, BUILD_അട്ടി_ADDR, BUILD_EBDA_MINIMUM);
    എങ്കിൽ (highram_start) {
        alloc_add(&Zone_മുകളിലെ, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

ശൂന്യം
malloc_csm_preinit(u32 താഴെയുള്ള_pmm, u32 താഴെയുള്ള_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    എങ്കിൽ (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_മുകളിലെ, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_മുകളിലെ, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } അല്ലെങ്കിൽ {
        alloc_add(&Zone_Tmp_മുകളിലെ, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_താഴെയുള്ള, താഴെയുള്ള_pmm, താഴെയുള്ള_pmm + താഴെയുള്ള_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
സ്ഥിതം ശൂന്യം
calcRamSize(ശൂന്യം)
{
    u32 rs = 0;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=e820_എണ്ണം-1; i>=0; i--) {
        ഘടന e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        എങ്കിൽ (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            നിർത്തുക;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
ശൂന്യം
malloc_init(ശൂന്യം)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    എങ്കിൽ (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i=0; i<ARRAY_SIZE(Zones); i++) {
            ഘടന zone_s *zone = Zones[i];
            എങ്കിൽ (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , ചിഹ്നം(varlow_end) - ചിഹ്നം(varlow_start));
    എങ്കിൽ (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_താഴെയുള്ള, ചിഹ്നം(zonelow_base) + OPROM_HEADER_RESERVE
                  , ചിഹ്നം(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_താഴെയുള്ള);
    } അല്ലെങ്കിൽ {
        alloc_add(&Zone_താഴെയുള്ള, ALIGN_DOWN(ചിഹ്നം(final_varlow_start), 1024)
                  , ചിഹ്നം(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , ചിഹ്നം(zonefseg_end) - ചിഹ്നം(zonefseg_start));
    alloc_add(&ZoneFSeg, ചിഹ്നം(zonefseg_start), ചിഹ്നം(zonefseg_end));

    calcRamSize();
}

ശൂന്യം
malloc_prepboot(ശൂന്യം)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((ശൂന്യം*)RomEnd, 0, base-RomEnd);
    എങ്കിൽ (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        ഘടന rom_header *dummyrom = (ശൂന്യം*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        പൂർണ്ണസംഖ്യ size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    ഘടന allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_മുകളിലെ);
    എങ്കിൽ (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
