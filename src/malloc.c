// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_BDA
#ضمن "config.h" // BUILD_BIOS_ADDR
#ضمن "e820map.h" // struct e820entry
#ضمن "list.h" // hlist_node
#ضمن "malloc.h" // _malloc
#ضمن "memmap.h" // PAGE_SIZE
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // wait_preempt
#ضمن "std/optionrom.h" // OPTION_ROM_ALIGN
#ضمن "string.h" // memset

// Information on a reserved area.
بنية allocinfo_s {
    بنية hlist_گرہ گرہ;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
بنية allocdetail_s {
    بنية allocinfo_s detailinfo;
    بنية allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
بنية zone_s {
    بنية hlist_head head;
};

بنية zone_s Zone_کم VARVERIFY32INIT, Zone_بلند VARVERIFY32INIT;
بنية zone_s ZoneFSeg VARVERIFY32INIT;
بنية zone_s Zone_Tmp_کم VARVERIFY32INIT, Zone_Tmp_بلند VARVERIFY32INIT;

ساكن بنية zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_کم, &Zone_کم, &ZoneFSeg, &Zone_Tmp_بلند, &Zone_بلند
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ساكن u32
alloc_new(بنية zone_s *zone, u32 size, u32 align, بنية allocinfo_s *fill)
{
    بنية allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, گرہ) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        إذا (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->گرہ, &info->گرہ);
            أرجع new_range_end;
        }
    }
    أرجع 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ساكن بنية allocdetail_s *
alloc_new_detail(بنية allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_بلند, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    إذا (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_کم, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        إذا (!detail_addr) {
            warn_noalloc();
            أرجع عدم;
        }
    }
    بنية allocdetail_s *detail = memremap(detail_addr, حجم(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, حجم(*detail));
    hlist_replace(&temp->detailinfo.گرہ, &detail->detailinfo.گرہ);
    hlist_replace(&temp->datainfo.گرہ, &detail->datainfo.گرہ);
    أرجع detail;
}

// Add new memory to a zone
ساكن فراغ
alloc_add(بنية zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    بنية allocinfo_s *info;
    بنية hlist_گرہ **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, گرہ) {
        إذا (info->range_start < start)
            اكسر;
    }

    // Add space using temporary allocation info.
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.گرہ, pprev);

    // Allocate final allocation info.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail)
        hlist_del(&tempdetail.datainfo.گرہ);
}

// Release space allocated with alloc_new()
ساكن فراغ
alloc_free(بنية allocinfo_s *info)
{
    بنية allocinfo_s *next = container_of_or_null(
        info->گرہ.next, بنية allocinfo_s, گرہ);
    إذا (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->گرہ);
}

// Search all zones for an allocation obtained from alloc_new()
ساكن بنية allocinfo_s *
alloc_find(u32 data)
{
    صحيح i;
    لكل (i=0; i<ARRAY_SIZE(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, گرہ) {
            إذا (info->range_start == data)
                أرجع info;
        }
    }
    أرجع عدم;
}

// Find the lowest memory range added by alloc_add()
ساكن بنية allocinfo_s *
alloc_find_lowest(بنية zone_s *zone)
{
    بنية allocinfo_s *info, *last = عدم;
    hlist_for_each_entry(info, &zone->head, گرہ) {
        last = info;
    }
    أرجع last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ساكن صحيح
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    إذا (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        أرجع -1;

    // Do copy
    memmove((فراغ*)newebda, (فراغ*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    أرجع 0;
}

// Support expanding the ZoneLow dynamically.
ساكن u32
zonelow_expand(u32 size, u32 align, بنية allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    إذا (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_کم, size, align, fill);
        إذا (data)
            أرجع data;
    }

    بنية allocinfo_s *info = alloc_find_lowest(&Zone_کم);
    إذا (!info)
        أرجع 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    إذا (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        أرجع alloc_new(&Zone_کم, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    إذا (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    إذا (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        أرجع 0;

    // Move ebda
    صحيح ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    إذا (ret)
        أرجع 0;

    // Update zone
    إذا (ebda_end == bottom)
        info->range_start = newbottom;
    وإلا
        alloc_add(&Zone_کم, newbottom, ebda_end);

    أرجع alloc_new(&Zone_کم, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(بنية zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    إذا (!size)
        أرجع 0;

    // Find and reserve space for main allocation
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    إذا (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_کم)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    إذا (!data)
        أرجع 0;

    // Find and reserve space for bookkeeping.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail) {
        alloc_free(&tempdetail.datainfo);
        أرجع 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    أرجع data;
}

// Allocate virtual memory from the given zone
فراغ * __malloc
_malloc(بنية zone_s *zone, u32 size, u32 align)
{
    أرجع memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
صحيح
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_find(data);
    إذا (!info || data == virt_to_phys(info) || !info->alloc_size)
        أرجع -1;
    بنية allocdetail_s *detail = container_of(
        info, بنية allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    أرجع 0;
}

فراغ
حرر(فراغ *data)
{
    إذا (!data)
        أرجع;
    صحيح ret = malloc_pfree(virt_to_phys(data));
    إذا (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(بنية zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    بنية allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, گرہ) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        إذا (space > maxspace)
            maxspace = space;
    }

    إذا (zone != &Zone_Tmp_بلند && zone != &Zone_Tmp_کم)
        أرجع maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(حجم(بنية allocdetail_s), MALLOC_MIN_ALIGN);
    إذا (maxspace <= reserve)
        أرجع 0;
    أرجع maxspace - reserve;
}

// Set a handle associated with an allocation.
فراغ
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_find(data);
    إذا (!info || data == virt_to_phys(info) || !info->alloc_size)
        أرجع;
    بنية allocdetail_s *detail = container_of(
        info, بنية allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    صحيح i;
    لكل (i=0; i<ARRAY_SIZE(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, گرہ) {
            إذا (info->range_start != virt_to_phys(info))
                استمر;
            بنية allocdetail_s *detail = container_of(
                info, بنية allocdetail_s, detailinfo);
            إذا (detail->handle == handle)
                أرجع detail->datainfo.range_start;
        }
    }
    أرجع 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ساكن u32 RomEnd = BUILD_ROM_START;
ساكن بنية allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(فراغ)
{
    إذا (CONFIG_MALLOC_UPPERMEMORY)
        أرجع ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    أرجع علامت(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(فراغ)
{
    أرجع RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
بنية rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    إذا (newend > rom_get_max())
        أرجع عدم;
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        إذا (newend < علامت(zonelow_base))
            newend = علامت(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    أرجع (فراغ*)RomEnd;
}

// Confirm space as in use by an optionrom.
صحيح
rom_confirm(u32 size)
{
    فراغ *new = rom_reserve(size);
    إذا (!new) {
        warn_noalloc();
        أرجع -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    أرجع 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

فراغ
malloc_preinit(فراغ)
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
    صحيح i;
    لكل (i=e820_گنتی-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        إذا (end < 1024*1024)
            اكسر;
        إذا (en->type != E820_RAM || end > 0xffffffff)
            استمر;
        u32 s = en->start, e = end;
        إذا (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            إذا (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } وإلا إذا (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_بلند, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_کم, BUILD_ڈھیر_ADDR, BUILD_EBDA_MINIMUM);
    إذا (highram_start) {
        alloc_add(&Zone_بلند, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

فراغ
malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    إذا (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_بلند, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_بلند, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } وإلا {
        alloc_add(&Zone_Tmp_بلند, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_کم, کم_pmm, کم_pmm + کم_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ساكن فراغ
calcRamSize(فراغ)
{
    u32 rs = 0;
    صحيح i;
    لكل (i=e820_گنتی-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        إذا (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            اكسر;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
فراغ
malloc_init(فراغ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    إذا (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        صحيح i;
        لكل (i=0; i<ARRAY_SIZE(Zones); i++) {
            بنية zone_s *zone = Zones[i];
            إذا (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , علامت(varlow_end) - علامت(varlow_start));
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_کم, علامت(zonelow_base) + OPROM_HEADER_RESERVE
                  , علامت(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_کم);
    } وإلا {
        alloc_add(&Zone_کم, ALIGN_DOWN(علامت(final_varlow_start), 1024)
                  , علامت(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , علامت(zonefseg_end) - علامت(zonefseg_start));
    alloc_add(&ZoneFSeg, علامت(zonefseg_start), علامت(zonefseg_end));

    calcRamSize();
}

فراغ
malloc_prepboot(فراغ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((فراغ*)RomEnd, 0, base-RomEnd);
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        بنية rom_header *dummyrom = (فراغ*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        صحيح size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    بنية allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_بلند);
    إذا (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
