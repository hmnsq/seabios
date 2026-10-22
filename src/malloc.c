// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_BDA
#ಅನ್ತರ್ಭಾವ "config.h" // BUILD_BIOS_ADDR
#ಅನ್ತರ್ಭಾವ "e820map.h" // struct e820entry
#ಅನ್ತರ್ಭಾವ "list.h" // hlist_node
#ಅನ್ತರ್ಭಾವ "malloc.h" // _malloc
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // wait_preempt
#ಅನ್ತರ್ಭಾವ "std/optionrom.h" // OPTION_ROM_ALIGN
#ಅನ್ತರ್ಭಾವ "string.h" // memset

// Information on a reserved area.
ಸಂರಚನಾ allocinfo_s {
    ಸಂರಚನಾ hlist_ಗಂಟು ಗಂಟು;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
ಸಂರಚನಾ allocdetail_s {
    ಸಂರಚನಾ allocinfo_s detailinfo;
    ಸಂರಚನಾ allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
ಸಂರಚನಾ zone_s {
    ಸಂರಚನಾ hlist_head head;
};

ಸಂರಚನಾ zone_s Zone_ಕೆಳಗಿನ VARVERIFY32INIT, Zone_ಮೇಲಿನ VARVERIFY32INIT;
ಸಂರಚನಾ zone_s ZoneFSeg VARVERIFY32INIT;
ಸಂರಚನಾ zone_s Zone_Tmp_ಕೆಳಗಿನ VARVERIFY32INIT, Zone_Tmp_ಮೇಲಿನ VARVERIFY32INIT;

ಸ್ಥಿರ ಸಂರಚನಾ zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_ಕೆಳಗಿನ, &Zone_ಕೆಳಗಿನ, &ZoneFSeg, &Zone_Tmp_ಮೇಲಿನ, &Zone_ಮೇಲಿನ
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ಸ್ಥಿರ u32
alloc_new(ಸಂರಚನಾ zone_s *zone, u32 size, u32 align, ಸಂರಚನಾ allocinfo_s *fill)
{
    ಸಂರಚನಾ allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, ಗಂಟು) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        ಯದಿ (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->ಗಂಟು, &info->ಗಂಟು);
            ಪ್ರತ್ಯಾಗಮ new_range_end;
        }
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ಸ್ಥಿರ ಸಂರಚನಾ allocdetail_s *
alloc_new_detail(ಸಂರಚನಾ allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_ಮೇಲಿನ, ಪರಿಮಾಣ(ಸಂರಚನಾ allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    ಯದಿ (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_ಕೆಳಗಿನ, ಪರಿಮಾಣ(ಸಂರಚನಾ allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        ಯದಿ (!detail_addr) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        }
    }
    ಸಂರಚನಾ allocdetail_s *detail = memremap(detail_addr, ಪರಿಮಾಣ(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, ಪರಿಮಾಣ(*detail));
    hlist_replace(&temp->detailinfo.ಗಂಟು, &detail->detailinfo.ಗಂಟು);
    hlist_replace(&temp->datainfo.ಗಂಟು, &detail->datainfo.ಗಂಟು);
    ಪ್ರತ್ಯಾಗಮ detail;
}

// Add new memory to a zone
ಸ್ಥಿರ ಶೂನ್ಯ
alloc_add(ಸಂರಚನಾ zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    ಸಂರಚನಾ allocinfo_s *info;
    ಸಂರಚನಾ hlist_ಗಂಟು **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, ಗಂಟು) {
        ಯದಿ (info->range_start < start)
            ಭಙ್ಗ;
    }

    // Add space using temporary allocation info.
    ಸಂರಚನಾ allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.ಗಂಟು, pprev);

    // Allocate final allocation info.
    ಸಂರಚನಾ allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ಯದಿ (!detail)
        hlist_del(&tempdetail.datainfo.ಗಂಟು);
}

// Release space allocated with alloc_new()
ಸ್ಥಿರ ಶೂನ್ಯ
alloc_free(ಸಂರಚನಾ allocinfo_s *info)
{
    ಸಂರಚನಾ allocinfo_s *next = container_of_or_null(
        info->ಗಂಟು.next, ಸಂರಚನಾ allocinfo_s, ಗಂಟು);
    ಯದಿ (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->ಗಂಟು);
}

// Search all zones for an allocation obtained from alloc_new()
ಸ್ಥಿರ ಸಂರಚನಾ allocinfo_s *
alloc_find(u32 data)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ARRAY_SIZE(Zones); i++) {
        ಸಂರಚನಾ allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, ಗಂಟು) {
            ಯದಿ (info->range_start == data)
                ಪ್ರತ್ಯಾಗಮ info;
        }
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

// Find the lowest memory range added by alloc_add()
ಸ್ಥಿರ ಸಂರಚನಾ allocinfo_s *
alloc_find_lowest(ಸಂರಚನಾ zone_s *zone)
{
    ಸಂರಚನಾ allocinfo_s *info, *last = ರಿಕ್ತ;
    hlist_for_each_entry(info, &zone->head, ಗಂಟು) {
        last = info;
    }
    ಪ್ರತ್ಯಾಗಮ last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    ಯದಿ (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        ಪ್ರತ್ಯಾಗಮ -1;

    // Do copy
    memmove((ಶೂನ್ಯ*)newebda, (ಶೂನ್ಯ*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Support expanding the ZoneLow dynamically.
ಸ್ಥಿರ u32
zonelow_expand(u32 size, u32 align, ಸಂರಚನಾ allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    ಯದಿ (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_ಕೆಳಗಿನ, size, align, fill);
        ಯದಿ (data)
            ಪ್ರತ್ಯಾಗಮ data;
    }

    ಸಂರಚನಾ allocinfo_s *info = alloc_find_lowest(&Zone_ಕೆಳಗಿನ);
    ಯದಿ (!info)
        ಪ್ರತ್ಯಾಗಮ 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    ಯದಿ (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        ಪ್ರತ್ಯಾಗಮ alloc_new(&Zone_ಕೆಳಗಿನ, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    ಯದಿ (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    ಯದಿ (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        ಪ್ರತ್ಯಾಗಮ 0;

    // Move ebda
    ಪೂರ್ಣಾಙ್ಕ ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ 0;

    // Update zone
    ಯದಿ (ebda_end == bottom)
        info->range_start = newbottom;
    ಅನ್ಯಥಾ
        alloc_add(&Zone_ಕೆಳಗಿನ, newbottom, ebda_end);

    ಪ್ರತ್ಯಾಗಮ alloc_new(&Zone_ಕೆಳಗಿನ, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(ಸಂರಚನಾ zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    ಯದಿ (!size)
        ಪ್ರತ್ಯಾಗಮ 0;

    // Find and reserve space for main allocation
    ಸಂರಚನಾ allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    ಯದಿ (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_ಕೆಳಗಿನ)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    ಯದಿ (!data)
        ಪ್ರತ್ಯಾಗಮ 0;

    // Find and reserve space for bookkeeping.
    ಸಂರಚನಾ allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ಯದಿ (!detail) {
        alloc_free(&tempdetail.datainfo);
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    ಪ್ರತ್ಯಾಗಮ data;
}

// Allocate virtual memory from the given zone
ಶೂನ್ಯ * __malloc
_malloc(ಸಂರಚನಾ zone_s *zone, u32 size, u32 align)
{
    ಪ್ರತ್ಯಾಗಮ memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
ಪೂರ್ಣಾಙ್ಕ
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    ಸಂರಚನಾ allocinfo_s *info = alloc_find(data);
    ಯದಿ (!info || data == virt_to_phys(info) || !info->alloc_size)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ allocdetail_s *detail = container_of(
        info, ಸಂರಚನಾ allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ
ಮುಕ್ತ(ಶೂನ್ಯ *data)
{
    ಯದಿ (!data)
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ ret = malloc_pfree(virt_to_phys(data));
    ಯದಿ (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(ಸಂರಚನಾ zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    ಸಂರಚನಾ allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, ಗಂಟು) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        ಯದಿ (space > maxspace)
            maxspace = space;
    }

    ಯದಿ (zone != &Zone_Tmp_ಮೇಲಿನ && zone != &Zone_Tmp_ಕೆಳಗಿನ)
        ಪ್ರತ್ಯಾಗಮ maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(ಪರಿಮಾಣ(ಸಂರಚನಾ allocdetail_s), MALLOC_MIN_ALIGN);
    ಯದಿ (maxspace <= reserve)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ maxspace - reserve;
}

// Set a handle associated with an allocation.
ಶೂನ್ಯ
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    ಸಂರಚನಾ allocinfo_s *info = alloc_find(data);
    ಯದಿ (!info || data == virt_to_phys(info) || !info->alloc_size)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ allocdetail_s *detail = container_of(
        info, ಸಂರಚನಾ allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ARRAY_SIZE(Zones); i++) {
        ಸಂರಚನಾ allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, ಗಂಟು) {
            ಯದಿ (info->range_start != virt_to_phys(info))
                ಅನುವರ್ತ;
            ಸಂರಚನಾ allocdetail_s *detail = container_of(
                info, ಸಂರಚನಾ allocdetail_s, detailinfo);
            ಯದಿ (detail->handle == handle)
                ಪ್ರತ್ಯಾಗಮ detail->datainfo.range_start;
        }
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ಸ್ಥಿರ u32 RomEnd = BUILD_ROM_START;
ಸ್ಥಿರ ಸಂರಚನಾ allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(ಶೂನ್ಯ)
{
    ಯದಿ (CONFIG_MALLOC_UPPERMEMORY)
        ಪ್ರತ್ಯಾಗಮ ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    ಪ್ರತ್ಯಾಗಮ ಚಿಹ್ನೆ(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
ಸಂರಚನಾ rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    ಯದಿ (newend > rom_get_max())
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಯದಿ (CONFIG_MALLOC_UPPERMEMORY) {
        ಯದಿ (newend < ಚಿಹ್ನೆ(zonelow_base))
            newend = ಚಿಹ್ನೆ(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)RomEnd;
}

// Confirm space as in use by an optionrom.
ಪೂರ್ಣಾಙ್ಕ
rom_confirm(u32 size)
{
    ಶೂನ್ಯ *new = rom_reserve(size);
    ಯದಿ (!new) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    ಪ್ರತ್ಯಾಗಮ 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

ಶೂನ್ಯ
malloc_preinit(ಶೂನ್ಯ)
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
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=e820_ಎಣಿಕೆ-1; i>=0; i--) {
        ಸಂರಚನಾ e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        ಯದಿ (end < 1024*1024)
            ಭಙ್ಗ;
        ಯದಿ (en->type != E820_RAM || end > 0xffffffff)
            ಅನುವರ್ತ;
        u32 s = en->start, e = end;
        ಯದಿ (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            ಯದಿ (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } ಅನ್ಯಥಾ ಯದಿ (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_ಮೇಲಿನ, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_ಕೆಳಗಿನ, BUILD_ರಾಶಿ_ADDR, BUILD_EBDA_MINIMUM);
    ಯದಿ (highram_start) {
        alloc_add(&Zone_ಮೇಲಿನ, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

ಶೂನ್ಯ
malloc_csm_preinit(u32 ಕೆಳಗಿನ_pmm, u32 ಕೆಳಗಿನ_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    ಯದಿ (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_ಮೇಲಿನ, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_ಮೇಲಿನ, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } ಅನ್ಯಥಾ {
        alloc_add(&Zone_Tmp_ಮೇಲಿನ, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_ಕೆಳಗಿನ, ಕೆಳಗಿನ_pmm, ಕೆಳಗಿನ_pmm + ಕೆಳಗಿನ_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ಸ್ಥಿರ ಶೂನ್ಯ
calcRamSize(ಶೂನ್ಯ)
{
    u32 rs = 0;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=e820_ಎಣಿಕೆ-1; i>=0; i--) {
        ಸಂರಚನಾ e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        ಯದಿ (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            ಭಙ್ಗ;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
ಶೂನ್ಯ
malloc_init(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    ಯದಿ (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i=0; i<ARRAY_SIZE(Zones); i++) {
            ಸಂರಚನಾ zone_s *zone = Zones[i];
            ಯದಿ (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , ಚಿಹ್ನೆ(varlow_end) - ಚಿಹ್ನೆ(varlow_start));
    ಯದಿ (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_ಕೆಳಗಿನ, ಚಿಹ್ನೆ(zonelow_base) + OPROM_HEADER_RESERVE
                  , ಚಿಹ್ನೆ(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_ಕೆಳಗಿನ);
    } ಅನ್ಯಥಾ {
        alloc_add(&Zone_ಕೆಳಗಿನ, ALIGN_DOWN(ಚಿಹ್ನೆ(final_varlow_start), 1024)
                  , ಚಿಹ್ನೆ(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , ಚಿಹ್ನೆ(zonefseg_end) - ಚಿಹ್ನೆ(zonefseg_start));
    alloc_add(&ZoneFSeg, ಚಿಹ್ನೆ(zonefseg_start), ಚಿಹ್ನೆ(zonefseg_end));

    calcRamSize();
}

ಶೂನ್ಯ
malloc_prepboot(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((ಶೂನ್ಯ*)RomEnd, 0, base-RomEnd);
    ಯದಿ (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        ಸಂರಚನಾ rom_header *dummyrom = (ಶೂನ್ಯ*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        ಪೂರ್ಣಾಙ್ಕ size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    ಸಂರಚನಾ allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_ಮೇಲಿನ);
    ಯದಿ (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
