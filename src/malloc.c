// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_BDA
#અન્તર્ભાવ "config.h" // BUILD_BIOS_ADDR
#અન્તર્ભાવ "e820map.h" // struct e820entry
#અન્તર્ભાવ "list.h" // hlist_node
#અન્તર્ભાવ "malloc.h" // _malloc
#અન્તર્ભાવ "memmap.h" // PAGE_SIZE
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // wait_preempt
#અન્તર્ભાવ "std/optionrom.h" // OPTION_ROM_ALIGN
#અન્તર્ભાવ "string.h" // memset

// Information on a reserved area.
સંરચના allocinfo_s {
    સંરચના hlist_ગાંઠ ગાંઠ;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
સંરચના allocdetail_s {
    સંરચના allocinfo_s detailinfo;
    સંરચના allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
સંરચના zone_s {
    સંરચના hlist_head head;
};

સંરચના zone_s Zone_નીચલું VARVERIFY32INIT, Zone_ઉપલું VARVERIFY32INIT;
સંરચના zone_s ZoneFSeg VARVERIFY32INIT;
સંરચના zone_s Zone_Tmp_નીચલું VARVERIFY32INIT, Zone_Tmp_ઉપલું VARVERIFY32INIT;

સ્થિર સંરચના zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_નીચલું, &Zone_નીચલું, &ZoneFSeg, &Zone_Tmp_ઉપલું, &Zone_ઉપલું
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
સ્થિર u32
alloc_new(સંરચના zone_s *zone, u32 size, u32 align, સંરચના allocinfo_s *fill)
{
    સંરચના allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, ગાંઠ) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        જો (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->ગાંઠ, &info->ગાંઠ);
            પાછા new_range_end;
        }
    }
    પાછા 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
સ્થિર સંરચના allocdetail_s *
alloc_new_detail(સંરચના allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_ઉપલું, કદ(સંરચના allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    જો (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_નીચલું, કદ(સંરચના allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        જો (!detail_addr) {
            warn_noalloc();
            પાછા ખાલી;
        }
    }
    સંરચના allocdetail_s *detail = memremap(detail_addr, કદ(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, કદ(*detail));
    hlist_replace(&temp->detailinfo.ગાંઠ, &detail->detailinfo.ગાંઠ);
    hlist_replace(&temp->datainfo.ગાંઠ, &detail->datainfo.ગાંઠ);
    પાછા detail;
}

// Add new memory to a zone
સ્થિર શૂન્ય
alloc_add(સંરચના zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    સંરચના allocinfo_s *info;
    સંરચના hlist_ગાંઠ **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, ગાંઠ) {
        જો (info->range_start < start)
            તોડો;
    }

    // Add space using temporary allocation info.
    સંરચના allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.ગાંઠ, pprev);

    // Allocate final allocation info.
    સંરચના allocdetail_s *detail = alloc_new_detail(&tempdetail);
    જો (!detail)
        hlist_del(&tempdetail.datainfo.ગાંઠ);
}

// Release space allocated with alloc_new()
સ્થિર શૂન્ય
alloc_free(સંરચના allocinfo_s *info)
{
    સંરચના allocinfo_s *next = container_of_or_null(
        info->ગાંઠ.next, સંરચના allocinfo_s, ગાંઠ);
    જો (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->ગાંઠ);
}

// Search all zones for an allocation obtained from alloc_new()
સ્થિર સંરચના allocinfo_s *
alloc_find(u32 data)
{
    પૂર્ણાંક i;
    માટે (i=0; i<ARRAY_SIZE(Zones); i++) {
        સંરચના allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, ગાંઠ) {
            જો (info->range_start == data)
                પાછા info;
        }
    }
    પાછા ખાલી;
}

// Find the lowest memory range added by alloc_add()
સ્થિર સંરચના allocinfo_s *
alloc_find_lowest(સંરચના zone_s *zone)
{
    સંરચના allocinfo_s *info, *last = ખાલી;
    hlist_for_each_entry(info, &zone->head, ગાંઠ) {
        last = info;
    }
    પાછા last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
સ્થિર પૂર્ણાંક
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    જો (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        પાછા -1;

    // Do copy
    memmove((શૂન્ય*)newebda, (શૂન્ય*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    પાછા 0;
}

// Support expanding the ZoneLow dynamically.
સ્થિર u32
zonelow_expand(u32 size, u32 align, સંરચના allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    જો (unlikely(wait_preempt())) {
        u32 data = alloc_new(&Zone_નીચલું, size, align, fill);
        જો (data)
            પાછા data;
    }

    સંરચના allocinfo_s *info = alloc_find_lowest(&Zone_નીચલું);
    જો (!info)
        પાછા 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    જો (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        પાછા alloc_new(&Zone_નીચલું, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    જો (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    જો (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        પાછા 0;

    // Move ebda
    પૂર્ણાંક ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    જો (ret)
        પાછા 0;

    // Update zone
    જો (ebda_end == bottom)
        info->range_start = newbottom;
    નહીંતર
        alloc_add(&Zone_નીચલું, newbottom, ebda_end);

    પાછા alloc_new(&Zone_નીચલું, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(સંરચના zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    જો (!size)
        પાછા 0;

    // Find and reserve space for main allocation
    સંરચના allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    જો (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &Zone_નીચલું)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    જો (!data)
        પાછા 0;

    // Find and reserve space for bookkeeping.
    સંરચના allocdetail_s *detail = alloc_new_detail(&tempdetail);
    જો (!detail) {
        alloc_free(&tempdetail.datainfo);
        પાછા 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    પાછા data;
}

// Allocate virtual memory from the given zone
શૂન્ય * __malloc
_malloc(સંરચના zone_s *zone, u32 size, u32 align)
{
    પાછા memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
પૂર્ણાંક
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    સંરચના allocinfo_s *info = alloc_find(data);
    જો (!info || data == virt_to_phys(info) || !info->alloc_size)
        પાછા -1;
    સંરચના allocdetail_s *detail = container_of(
        info, સંરચના allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    પાછા 0;
}

શૂન્ય
મુક્ત(શૂન્ય *data)
{
    જો (!data)
        પાછા;
    પૂર્ણાંક ret = malloc_pfree(virt_to_phys(data));
    જો (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(સંરચના zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    સંરચના allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, ગાંઠ) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        જો (space > maxspace)
            maxspace = space;
    }

    જો (zone != &Zone_Tmp_ઉપલું && zone != &Zone_Tmp_નીચલું)
        પાછા maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(કદ(સંરચના allocdetail_s), MALLOC_MIN_ALIGN);
    જો (maxspace <= reserve)
        પાછા 0;
    પાછા maxspace - reserve;
}

// Set a handle associated with an allocation.
શૂન્ય
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    સંરચના allocinfo_s *info = alloc_find(data);
    જો (!info || data == virt_to_phys(info) || !info->alloc_size)
        પાછા;
    સંરચના allocdetail_s *detail = container_of(
        info, સંરચના allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    પૂર્ણાંક i;
    માટે (i=0; i<ARRAY_SIZE(Zones); i++) {
        સંરચના allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, ગાંઠ) {
            જો (info->range_start != virt_to_phys(info))
                ચાલુ;
            સંરચના allocdetail_s *detail = container_of(
                info, સંરચના allocdetail_s, detailinfo);
            જો (detail->handle == handle)
                પાછા detail->datainfo.range_start;
        }
    }
    પાછા 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

સ્થિર u32 RomEnd = BUILD_ROM_START;
સ્થિર સંરચના allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(શૂન્ય)
{
    જો (CONFIG_MALLOC_UPPERMEMORY)
        પાછા ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    પાછા પ્રતીક(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(શૂન્ય)
{
    પાછા RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
સંરચના rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    જો (newend > rom_get_max())
        પાછા ખાલી;
    જો (CONFIG_MALLOC_UPPERMEMORY) {
        જો (newend < પ્રતીક(zonelow_base))
            newend = પ્રતીક(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    પાછા (શૂન્ય*)RomEnd;
}

// Confirm space as in use by an optionrom.
પૂર્ણાંક
rom_confirm(u32 size)
{
    શૂન્ય *new = rom_reserve(size);
    જો (!new) {
        warn_noalloc();
        પાછા -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    પાછા 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

શૂન્ય
malloc_preinit(શૂન્ય)
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
    પૂર્ણાંક i;
    માટે (i=e820_ગણતરી-1; i>=0; i--) {
        સંરચના e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        જો (end < 1024*1024)
            તોડો;
        જો (en->type != E820_RAM || end > 0xffffffff)
            ચાલુ;
        u32 s = en->start, e = end;
        જો (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            જો (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } નહીંતર જો (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&Zone_Tmp_ઉપલું, s, e);
    }

    // Populate regions
    alloc_add(&Zone_Tmp_નીચલું, BUILD_ઢગલો_ADDR, BUILD_EBDA_MINIMUM);
    જો (highram_start) {
        alloc_add(&Zone_ઉપલું, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

શૂન્ય
malloc_csm_preinit(u32 નીચલું_pmm, u32 નીચલું_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    જો (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&Zone_Tmp_ઉપલું, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&Zone_ઉપલું, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } નહીંતર {
        alloc_add(&Zone_Tmp_ઉપલું, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&Zone_Tmp_નીચલું, નીચલું_pmm, નીચલું_pmm + નીચલું_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
સ્થિર શૂન્ય
calcRamSize(શૂન્ય)
{
    u32 rs = 0;
    પૂર્ણાંક i;
    માટે (i=e820_ગણતરી-1; i>=0; i--) {
        સંરચના e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        જો (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            તોડો;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
શૂન્ય
malloc_init(શૂન્ય)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    જો (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        પૂર્ણાંક i;
        માટે (i=0; i<ARRAY_SIZE(Zones); i++) {
            સંરચના zone_s *zone = Zones[i];
            જો (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , પ્રતીક(varlow_end) - પ્રતીક(varlow_start));
    જો (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&Zone_નીચલું, પ્રતીક(zonelow_base) + OPROM_HEADER_RESERVE
                  , પ્રતીક(final_varlow_start));
        RomBase = alloc_find_lowest(&Zone_નીચલું);
    } નહીંતર {
        alloc_add(&Zone_નીચલું, ALIGN_DOWN(પ્રતીક(final_varlow_start), 1024)
                  , પ્રતીક(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , પ્રતીક(zonefseg_end) - પ્રતીક(zonefseg_start));
    alloc_add(&ZoneFSeg, પ્રતીક(zonefseg_start), પ્રતીક(zonefseg_end));

    calcRamSize();
}

શૂન્ય
malloc_prepboot(શૂન્ય)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((શૂન્ય*)RomEnd, 0, base-RomEnd);
    જો (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        સંરચના rom_header *dummyrom = (શૂન્ય*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        પૂર્ણાંક size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    સંરચના allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&Zone_ઉપલું);
    જો (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
