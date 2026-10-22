// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_BDA
#شامل "config.h" // BUILD_BIOS_ADDR
#شامل "e820map.h" // struct e820entry
#شامل "list.h" // hlist_node
#شامل "malloc.h" // _malloc
#شامل "memmap.h" // PAGE_SIZE
#شامل "output.h" // dprintf
#شامل "stacks.h" // wait_preempt
#شامل "std/optionrom.h" // OPTION_ROM_ALIGN
#شامل "string.h" // memset

// Information on a reserved area.
ساختار allocinfo_s {
    ساختار hlist_گره گره;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
ساختار allocdetail_s {
    ساختار allocinfo_s detailinfo;
    ساختار allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
ساختار zone_s {
    ساختار hlist_head head;
};

ساختار zone_s Zone_کم VARVERIFY32INIT, Zone_بالا VARVERIFY32INIT;
ساختار zone_s ZoneFSeg VARVERIFY32INIT;
ساختار zone_s Zone_Tmp_کم VARVERIFY32INIT, Zone_Tmp_بالا VARVERIFY32INIT;

ایستا ساختار zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_کم, &Zone_کم, &ZoneFSeg, &Zone_Tmp_بالا, &Zone_بالا
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ایستا u32
alloc_new(ساختار zone_s *zone, u32 size, u32 align, ساختار allocinfo_s *fill)
{
    ساختار allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, گره) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        اگر (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->گره, &info->گره);
            بازگشت new_range_end;
        }
    }
    بازگشت 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ایستا ساختار allocdetail_s *
alloc_new_detail(ساختار allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_بالا, اندازه(ساختار allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    اگر (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_کم, اندازه(ساختار allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        اگر (!detail_addr) {
            warn_noalloc();
            بازگشت پوچ;
        }
    }
    ساختار allocdetail_s *detail = memremap(detail_addr, اندازه(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, اندازه(*detail));
    hlist_replace(&temp->detailinfo.گره, &detail->detailinfo.گره);
    hlist_replace(&temp->datainfo.گره, &detail->datainfo.گره);
    بازگشت detail;
}

// Add new memory to a zone
ایستا تهی
alloc_add(ساختار zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    ساختار allocinfo_s *info;
    ساختار hlist_گره **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, گره) {
        اگر (info->range_start < start)
            بشکن;
    }

    // Add space using temporary allocation info.
    ساختار allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.گره, pprev);

    // Allocate final allocation info.
    ساختار allocdetail_s *detail = alloc_new_detail(&tempdetail);
    اگر (!detail)
        hlist_del(&tempdetail.datainfo.گره);
}

// Release space allocated with alloc_new()
ایستا تهی
alloc_free(ساختار allocinfo_s *info)
{
    ساختار allocinfo_s *next = container_of_or_null(
        info->گره.next, ساختار allocinfo_s, گره);
    اگر (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->گره);
}

// Search all zones for an allocation obtained from alloc_new()
ایستا ساختار allocinfo_s *
alloc_find(u32 data)
{
    صحیح i;
    برای (i=0; i<ARRAY_SIZE(Zones); i++) {
        ساختار allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, گره) {
            اگر (info->range_start == data)
                بازگشت info;
        }
    }
    بازگشت پوچ;
}

// Find the lowest memory range added by alloc_add()
ایستا ساختار allocinfo_s *
alloc_find_lowest(ساختار zone_s *zone)
{
    ساختار allocinfo_s *info, *last = پوچ;
    hlist_for_each_entry(info, &zone->head, گره) {
        last = info;
    }
    بازگشت last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ایستا صحیح
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    اگر (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        بازگشت -1;

    // Do copy
    memmove((تهی*)newebda, (تهی*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    بازگشت 0;
}

// Support expanding the ZoneLow dynamically.
ایستا u32
zonelow_expand(u32 size, u32 align, ساختار allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    اگر (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_کم, size, align, fill);
        اگر (data)
            بازگشت data;
    }

    ساختار allocinfo_s *info = alloc_find_lowest(&Zone_کم);
    اگر (!info)
        بازگشت 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    اگر (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        بازگشت alloc_new(&Zone_کم, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    اگر (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    اگر (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        بازگشت 0;

    // Move ebda
    صحیح ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    اگر (ret)
        بازگشت 0;

    // Update zone
    اگر (ebda_end == bottom)
        info->range_start = newbottom;
    وگرنه
        alloc_add(&Zone_کم, newbottom, ebda_end);

    بازگشت alloc_new(&Zone_کم, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(ساختار zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    اگر (!size)
        بازگشت 0;

    // Find and reserve space for main allocation
    ساختار allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    اگر (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_کم)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    اگر (!data)
        بازگشت 0;

    // Find and reserve space for bookkeeping.
    ساختار allocdetail_s *detail = alloc_new_detail(&tempdetail);
    اگر (!detail) {
        alloc_free(&tempdetail.datainfo);
        بازگشت 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    بازگشت data;
}

// Allocate virtual memory from the given zone
تهی * __malloc
_malloc(ساختار zone_s *zone, u32 size, u32 align)
{
    بازگشت memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
صحیح
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    ساختار allocinfo_s *info = alloc_find(data);
    اگر (!info || data == virt_to_phys(info) || !info->alloc_size)
        بازگشت -1;
    ساختار allocdetail_s *detail = container_of(
        info, ساختار allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    بازگشت 0;
}

تهی
آزاد(تهی *data)
{
    اگر (!data)
        بازگشت;
    صحیح ret = malloc_pfree(virt_to_phys(data));
    اگر (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(ساختار zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    ساختار allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, گره) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        اگر (space > maxspace)
            maxspace = space;
    }

    اگر (zone != &Zone_Tmp_بالا && zone != &Zone_Tmp_کم)
        بازگشت maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(اندازه(ساختار allocdetail_s), MALLOC_MIN_ALIGN);
    اگر (maxspace <= reserve)
        بازگشت 0;
    بازگشت maxspace - reserve;
}

// Set a handle associated with an allocation.
تهی
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    ساختار allocinfo_s *info = alloc_find(data);
    اگر (!info || data == virt_to_phys(info) || !info->alloc_size)
        بازگشت;
    ساختار allocdetail_s *detail = container_of(
        info, ساختار allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    صحیح i;
    برای (i=0; i<ARRAY_SIZE(Zones); i++) {
        ساختار allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, گره) {
            اگر (info->range_start != virt_to_phys(info))
                ادامه;
            ساختار allocdetail_s *detail = container_of(
                info, ساختار allocdetail_s, detailinfo);
            اگر (detail->handle == handle)
                بازگشت detail->datainfo.range_start;
        }
    }
    بازگشت 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ایستا u32 RomEnd = BUILD_ROM_START;
ایستا ساختار allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(تهی)
{
    اگر (CONFIG_MALLOC_UPPERMEMORY)
        بازگشت ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    بازگشت نماد(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(تهی)
{
    بازگشت RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
ساختار rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    اگر (newend > rom_get_max())
        بازگشت پوچ;
    اگر (CONFIG_MALLOC_UPPERMEMORY) {
        اگر (newend < نماد(zonelow_base))
            newend = نماد(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    بازگشت (تهی*)RomEnd;
}

// Confirm space as in use by an optionrom.
صحیح
rom_confirm(u32 size)
{
    تهی *new = rom_reserve(size);
    اگر (!new) {
        warn_noalloc();
        بازگشت -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    بازگشت 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

تهی
malloc_preinit(تهی)
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
    صحیح i;
    برای (i=e820_شمار-1; i>=0; i--) {
        ساختار e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        اگر (end < 1024*1024)
            بشکن;
        اگر (en->type != E820_RAM || end > 0xffffffff)
            ادامه;
        u32 s = en->start, e = end;
        اگر (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            اگر (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } وگرنه اگر (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_بالا, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_کم, BUILD_پشته_ADDR, BUILD_EBDA_MINIMUM);
    اگر (highram_start) {
        alloc_add(&Zone_بالا, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

تهی
malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    اگر (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_بالا, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_بالا, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } وگرنه {
        alloc_add(&Zone_Tmp_بالا, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_کم, کم_pmm, کم_pmm + کم_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ایستا تهی
calcRamSize(تهی)
{
    u32 rs = 0;
    صحیح i;
    برای (i=e820_شمار-1; i>=0; i--) {
        ساختار e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        اگر (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            بشکن;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
تهی
malloc_init(تهی)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    اگر (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        صحیح i;
        برای (i=0; i<ARRAY_SIZE(Zones); i++) {
            ساختار zone_s *zone = Zones[i];
            اگر (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , نماد(varlow_end) - نماد(varlow_start));
    اگر (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_کم, نماد(zonelow_base) + OPROM_HEADER_RESERVE
                  , نماد(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_کم);
    } وگرنه {
        alloc_add(&Zone_کم, ALIGN_DOWN(نماد(final_varlow_start), 1024)
                  , نماد(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , نماد(zonefseg_end) - نماد(zonefseg_start));
    alloc_add(&ZoneFSeg, نماد(zonefseg_start), نماد(zonefseg_end));

    calcRamSize();
}

تهی
malloc_prepboot(تهی)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((تهی*)RomEnd, 0, base-RomEnd);
    اگر (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        ساختار rom_header *dummyrom = (تهی*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        صحیح size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    ساختار allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_بالا);
    اگر (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
