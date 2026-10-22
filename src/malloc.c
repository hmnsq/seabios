// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_BDA
#ଅନ୍ତର୍ଭାଵ "config.h" // BUILD_BIOS_ADDR
#ଅନ୍ତର୍ଭାଵ "e820map.h" // struct e820entry
#ଅନ୍ତର୍ଭାଵ "list.h" // hlist_node
#ଅନ୍ତର୍ଭାଵ "malloc.h" // _malloc
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SIZE
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "stacks.h" // wait_preempt
#ଅନ୍ତର୍ଭାଵ "std/optionrom.h" // OPTION_ROM_ALIGN
#ଅନ୍ତର୍ଭାଵ "string.h" // memset

// Information on a reserved area.
ସଂରଚନା allocinfo_s {
    ସଂରଚନା hlist_ଗଣ୍ଠି ଗଣ୍ଠି;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
ସଂରଚନା allocdetail_s {
    ସଂରଚନା allocinfo_s detailinfo;
    ସଂରଚନା allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
ସଂରଚନା zone_s {
    ସଂରଚନା hlist_head head;
};

ସଂରଚନା zone_s Zone_ତଳ VARVERIFY32INIT, Zone_ଉପର VARVERIFY32INIT;
ସଂରଚନା zone_s ZoneFSeg VARVERIFY32INIT;
ସଂରଚନା zone_s Zone_Tmp_ତଳ VARVERIFY32INIT, Zone_Tmp_ଉପର VARVERIFY32INIT;

ସ୍ଥିର ସଂରଚନା zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_ତଳ, &Zone_ତଳ, &ZoneFSeg, &Zone_Tmp_ଉପର, &Zone_ଉପର
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ସ୍ଥିର u32
alloc_new(ସଂରଚନା zone_s *zone, u32 size, u32 align, ସଂରଚନା allocinfo_s *fill)
{
    ସଂରଚନା allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, ଗଣ୍ଠି) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        ଯଦି (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->ଗଣ୍ଠି, &info->ଗଣ୍ଠି);
            ଫେରନ୍ତୁ new_range_end;
        }
    }
    ଫେରନ୍ତୁ 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ସ୍ଥିର ସଂରଚନା allocdetail_s *
alloc_new_detail(ସଂରଚନା allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_ଉପର, ଆକାର(ସଂରଚନା allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    ଯଦି (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_ତଳ, ଆକାର(ସଂରଚନା allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        ଯଦି (!detail_addr) {
            warn_noalloc();
            ଫେରନ୍ତୁ ଖାଲି;
        }
    }
    ସଂରଚନା allocdetail_s *detail = memremap(detail_addr, ଆକାର(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, ଆକାର(*detail));
    hlist_replace(&temp->detailinfo.ଗଣ୍ଠି, &detail->detailinfo.ଗଣ୍ଠି);
    hlist_replace(&temp->datainfo.ଗଣ୍ଠି, &detail->datainfo.ଗଣ୍ଠି);
    ଫେରନ୍ତୁ detail;
}

// Add new memory to a zone
ସ୍ଥିର ଶୂନ୍ୟ
alloc_add(ସଂରଚନା zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    ସଂରଚନା allocinfo_s *info;
    ସଂରଚନା hlist_ଗଣ୍ଠି **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, ଗଣ୍ଠି) {
        ଯଦି (info->range_start < start)
            ଭାଙ୍ଗ;
    }

    // Add space using temporary allocation info.
    ସଂରଚନା allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.ଗଣ୍ଠି, pprev);

    // Allocate final allocation info.
    ସଂରଚନା allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ଯଦି (!detail)
        hlist_del(&tempdetail.datainfo.ଗଣ୍ଠି);
}

// Release space allocated with alloc_new()
ସ୍ଥିର ଶୂନ୍ୟ
alloc_free(ସଂରଚନା allocinfo_s *info)
{
    ସଂରଚନା allocinfo_s *next = container_of_or_null(
        info->ଗଣ୍ଠି.next, ସଂରଚନା allocinfo_s, ଗଣ୍ଠି);
    ଯଦି (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->ଗଣ୍ଠି);
}

// Search all zones for an allocation obtained from alloc_new()
ସ୍ଥିର ସଂରଚନା allocinfo_s *
alloc_find(u32 data)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ARRAY_SIZE(Zones); i++) {
        ସଂରଚନା allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, ଗଣ୍ଠି) {
            ଯଦି (info->range_start == data)
                ଫେରନ୍ତୁ info;
        }
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

// Find the lowest memory range added by alloc_add()
ସ୍ଥିର ସଂରଚନା allocinfo_s *
alloc_find_lowest(ସଂରଚନା zone_s *zone)
{
    ସଂରଚନା allocinfo_s *info, *last = ଖାଲି;
    hlist_for_each_entry(info, &zone->head, ଗଣ୍ଠି) {
        last = info;
    }
    ଫେରନ୍ତୁ last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    ଯଦି (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        ଫେରନ୍ତୁ -1;

    // Do copy
    memmove((ଶୂନ୍ୟ*)newebda, (ଶୂନ୍ୟ*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    ଫେରନ୍ତୁ 0;
}

// Support expanding the ZoneLow dynamically.
ସ୍ଥିର u32
zonelow_expand(u32 size, u32 align, ସଂରଚନା allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    ଯଦି (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_ତଳ, size, align, fill);
        ଯଦି (data)
            ଫେରନ୍ତୁ data;
    }

    ସଂରଚନା allocinfo_s *info = alloc_find_lowest(&Zone_ତଳ);
    ଯଦି (!info)
        ଫେରନ୍ତୁ 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    ଯଦି (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        ଫେରନ୍ତୁ alloc_new(&Zone_ତଳ, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    ଯଦି (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    ଯଦି (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        ଫେରନ୍ତୁ 0;

    // Move ebda
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    ଯଦି (ret)
        ଫେରନ୍ତୁ 0;

    // Update zone
    ଯଦି (ebda_end == bottom)
        info->range_start = newbottom;
    ଅନ୍ୟଥା
        alloc_add(&Zone_ତଳ, newbottom, ebda_end);

    ଫେରନ୍ତୁ alloc_new(&Zone_ତଳ, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(ସଂରଚନା zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    ଯଦି (!size)
        ଫେରନ୍ତୁ 0;

    // Find and reserve space for main allocation
    ସଂରଚନା allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    ଯଦି (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_ତଳ)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    ଯଦି (!data)
        ଫେରନ୍ତୁ 0;

    // Find and reserve space for bookkeeping.
    ସଂରଚନା allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ଯଦି (!detail) {
        alloc_free(&tempdetail.datainfo);
        ଫେରନ୍ତୁ 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    ଫେରନ୍ତୁ data;
}

// Allocate virtual memory from the given zone
ଶୂନ୍ୟ * __malloc
_malloc(ସଂରଚନା zone_s *zone, u32 size, u32 align)
{
    ଫେରନ୍ତୁ memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    ସଂରଚନା allocinfo_s *info = alloc_find(data);
    ଯଦି (!info || data == virt_to_phys(info) || !info->alloc_size)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା allocdetail_s *detail = container_of(
        info, ସଂରଚନା allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ
ମୁକ୍ତ(ଶୂନ୍ୟ *data)
{
    ଯଦି (!data)
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = malloc_pfree(virt_to_phys(data));
    ଯଦି (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(ସଂରଚନା zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    ସଂରଚନା allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, ଗଣ୍ଠି) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        ଯଦି (space > maxspace)
            maxspace = space;
    }

    ଯଦି (zone != &Zone_Tmp_ଉପର && zone != &Zone_Tmp_ତଳ)
        ଫେରନ୍ତୁ maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(ଆକାର(ସଂରଚନା allocdetail_s), MALLOC_MIN_ALIGN);
    ଯଦି (maxspace <= reserve)
        ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ maxspace - reserve;
}

// Set a handle associated with an allocation.
ଶୂନ୍ୟ
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    ସଂରଚନା allocinfo_s *info = alloc_find(data);
    ଯଦି (!info || data == virt_to_phys(info) || !info->alloc_size)
        ଫେରନ୍ତୁ;
    ସଂରଚନା allocdetail_s *detail = container_of(
        info, ସଂରଚନା allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ARRAY_SIZE(Zones); i++) {
        ସଂରଚନା allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, ଗଣ୍ଠି) {
            ଯଦି (info->range_start != virt_to_phys(info))
                ଜାରି;
            ସଂରଚନା allocdetail_s *detail = container_of(
                info, ସଂରଚନା allocdetail_s, detailinfo);
            ଯଦି (detail->handle == handle)
                ଫେରନ୍ତୁ detail->datainfo.range_start;
        }
    }
    ଫେରନ୍ତୁ 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ସ୍ଥିର u32 RomEnd = BUILD_ROM_START;
ସ୍ଥିର ସଂରଚନା allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(ଶୂନ୍ୟ)
{
    ଯଦି (CONFIG_MALLOC_UPPERMEMORY)
        ଫେରନ୍ତୁ ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    ଫେରନ୍ତୁ ପ୍ରତୀକ(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
ସଂରଚନା rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    ଯଦି (newend > rom_get_max())
        ଫେରନ୍ତୁ ଖାଲି;
    ଯଦି (CONFIG_MALLOC_UPPERMEMORY) {
        ଯଦି (newend < ପ୍ରତୀକ(zonelow_base))
            newend = ପ୍ରତୀକ(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)RomEnd;
}

// Confirm space as in use by an optionrom.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
rom_confirm(u32 size)
{
    ଶୂନ୍ୟ *new = rom_reserve(size);
    ଯଦି (!new) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    ଫେରନ୍ତୁ 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

ଶୂନ୍ୟ
malloc_preinit(ଶୂନ୍ୟ)
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=e820_ଗଣନା-1; i>=0; i--) {
        ସଂରଚନା e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        ଯଦି (end < 1024*1024)
            ଭାଙ୍ଗ;
        ଯଦି (en->type != E820_RAM || end > 0xffffffff)
            ଜାରି;
        u32 s = en->start, e = end;
        ଯଦି (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            ଯଦି (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } ଅନ୍ୟଥା ଯଦି (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_ଉପର, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_ତଳ, BUILD_ଗଦା_ADDR, BUILD_EBDA_MINIMUM);
    ଯଦି (highram_start) {
        alloc_add(&Zone_ଉପର, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

ଶୂନ୍ୟ
malloc_csm_preinit(u32 ତଳ_pmm, u32 ତଳ_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    ଯଦି (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_ଉପର, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_ଉପର, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } ଅନ୍ୟଥା {
        alloc_add(&Zone_Tmp_ଉପର, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_ତଳ, ତଳ_pmm, ତଳ_pmm + ତଳ_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ସ୍ଥିର ଶୂନ୍ୟ
calcRamSize(ଶୂନ୍ୟ)
{
    u32 rs = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=e820_ଗଣନା-1; i>=0; i--) {
        ସଂରଚନା e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        ଯଦି (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            ଭାଙ୍ଗ;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
ଶୂନ୍ୟ
malloc_init(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    ଯଦି (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i=0; i<ARRAY_SIZE(Zones); i++) {
            ସଂରଚନା zone_s *zone = Zones[i];
            ଯଦି (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , ପ୍ରତୀକ(varlow_end) - ପ୍ରତୀକ(varlow_start));
    ଯଦି (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_ତଳ, ପ୍ରତୀକ(zonelow_base) + OPROM_HEADER_RESERVE
                  , ପ୍ରତୀକ(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_ତଳ);
    } ଅନ୍ୟଥା {
        alloc_add(&Zone_ତଳ, ALIGN_DOWN(ପ୍ରତୀକ(final_varlow_start), 1024)
                  , ପ୍ରତୀକ(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , ପ୍ରତୀକ(zonefseg_end) - ପ୍ରତୀକ(zonefseg_start));
    alloc_add(&ZoneFSeg, ପ୍ରତୀକ(zonefseg_start), ପ୍ରତୀକ(zonefseg_end));

    calcRamSize();
}

ଶୂନ୍ୟ
malloc_prepboot(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((ଶୂନ୍ୟ*)RomEnd, 0, base-RomEnd);
    ଯଦି (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        ସଂରଚନା rom_header *dummyrom = (ଶୂନ୍ୟ*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    ସଂରଚନା allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_ଉପର);
    ଯଦି (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
