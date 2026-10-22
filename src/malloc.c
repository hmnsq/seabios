// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // BUILD_BIOS_ADDR
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // struct e820entry
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h" // hlist_node
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // _malloc
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SIZE
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // wait_preempt
#ਅਨ੍ਤਰ੍ਭਾਵ "std/optionrom.h" // OPTION_ROM_ALIGN
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset

// Information on a reserved area.
ਬਣਤਰ allocinfo_s {
    ਬਣਤਰ hlist_ਗੰਢ ਗੰਢ;
    u32 range_ਸ਼ੁਰੂ, range_ਸਿਰਾ, alloc_ਮਾਪ;
};

// Information on a tracked memory allocation.
ਬਣਤਰ allocdetail_s {
    ਬਣਤਰ allocinfo_s detailinfo;
    ਬਣਤਰ allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
ਬਣਤਰ zone_s {
    ਬਣਤਰ hlist_ਸਿਰ ਸਿਰ;
};

ਬਣਤਰ zone_s Zone_ਨੀਵਾਂ VARVERIFY32INIT, Zone_ਉੱਚਾ VARVERIFY32INIT;
ਬਣਤਰ zone_s ZoneFSeg VARVERIFY32INIT;
ਬਣਤਰ zone_s Zone_Tmp_ਨੀਵਾਂ VARVERIFY32INIT, Zone_Tmp_ਉੱਚਾ VARVERIFY32INIT;

ਸਥਿਰ ਬਣਤਰ zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_ਨੀਵਾਂ, &Zone_ਨੀਵਾਂ, &ZoneFSeg, &Zone_Tmp_ਉੱਚਾ, &Zone_ਉੱਚਾ
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ਸਥਿਰ u32
alloc_new(ਬਣਤਰ zone_s *zone, u32 ਮਾਪ, u32 align, ਬਣਤਰ allocinfo_s *fill)
{
    ਬਣਤਰ allocinfo_s *info;
    hlist_for_each_ਇੰਦਰਾਜ(info, &zone->ਸਿਰ, ਗੰਢ) {
        u32 alloc_ਸਿਰਾ = info->range_ਸ਼ੁਰੂ + info->alloc_ਮਾਪ;
        u32 range_ਸਿਰਾ = info->range_ਸਿਰਾ;
        u32 new_range_ਸਿਰਾ = ALIGN_ਥੱਲੇ(range_ਸਿਰਾ - ਮਾਪ, align);
        ਜੇ (new_range_ਸਿਰਾ >= alloc_ਸਿਰਾ && new_range_ਸਿਰਾ <= range_ਸਿਰਾ) {
            // Found space - now reserve it.
            fill->range_ਸ਼ੁਰੂ = new_range_ਸਿਰਾ;
            fill->range_ਸਿਰਾ = range_ਸਿਰਾ;
            fill->alloc_ਮਾਪ = ਮਾਪ;

            info->range_ਸਿਰਾ = new_range_ਸਿਰਾ;
            hlist_ਜੋੜੋ_before(&fill->ਗੰਢ, &info->ਗੰਢ);
            ਵਾਪਸ new_range_ਸਿਰਾ;
        }
    }
    ਵਾਪਸ 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ਸਥਿਰ ਬਣਤਰ allocdetail_s *
alloc_new_detail(ਬਣਤਰ allocdetail_s *ਆਰਜ਼ੀ)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_ਉੱਚਾ, ਆਕਾਰ(ਬਣਤਰ allocdetail_s)
                                , MALLOC_MIN_ALIGN, &ਆਰਜ਼ੀ->detailinfo);
    ਜੇ (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_ਨੀਵਾਂ, ਆਕਾਰ(ਬਣਤਰ allocdetail_s)
                                , MALLOC_MIN_ALIGN, &ਆਰਜ਼ੀ->detailinfo);
        ਜੇ (!detail_addr) {
            warn_noalloc();
            ਵਾਪਸ ਨਿਰਾ;
        }
    }
    ਬਣਤਰ allocdetail_s *detail = memremap(detail_addr, ਆਕਾਰ(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(detail, ਆਰਜ਼ੀ, ਆਕਾਰ(*detail));
    hlist_replace(&ਆਰਜ਼ੀ->detailinfo.ਗੰਢ, &detail->detailinfo.ਗੰਢ);
    hlist_replace(&ਆਰਜ਼ੀ->datainfo.ਗੰਢ, &detail->datainfo.ਗੰਢ);
    ਵਾਪਸ detail;
}

// Add new memory to a zone
ਸਥਿਰ ਖਾਲੀ
alloc_ਜੋੜੋ(ਬਣਤਰ zone_s *zone, u32 ਸ਼ੁਰੂ, u32 ਸਿਰਾ)
{
    // Find position to add space
    ਬਣਤਰ allocinfo_s *info;
    ਬਣਤਰ hlist_ਗੰਢ **pprev;
    hlist_for_each_ਇੰਦਰਾਜ_pprev(info, pprev, &zone->ਸਿਰ, ਗੰਢ) {
        ਜੇ (info->range_ਸ਼ੁਰੂ < ਸ਼ੁਰੂ)
            ਰੋਕੋ;
    }

    // Add space using temporary allocation info.
    ਬਣਤਰ allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_ਸ਼ੁਰੂ = ਸ਼ੁਰੂ;
    tempdetail.datainfo.range_ਸਿਰਾ = ਸਿਰਾ;
    tempdetail.datainfo.alloc_ਮਾਪ = 0;
    hlist_ਜੋੜੋ(&tempdetail.datainfo.ਗੰਢ, pprev);

    // Allocate final allocation info.
    ਬਣਤਰ allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ਜੇ (!detail)
        hlist_del(&tempdetail.datainfo.ਗੰਢ);
}

// Release space allocated with alloc_new()
ਸਥਿਰ ਖਾਲੀ
alloc_free(ਬਣਤਰ allocinfo_s *info)
{
    ਬਣਤਰ allocinfo_s *ਅਗਲਾ = container_ਦਾ_or_null(
        info->ਗੰਢ.ਅਗਲਾ, ਬਣਤਰ allocinfo_s, ਗੰਢ);
    ਜੇ (ਅਗਲਾ && ਅਗਲਾ->range_ਸਿਰਾ == info->range_ਸ਼ੁਰੂ)
        ਅਗਲਾ->range_ਸਿਰਾ = info->range_ਸਿਰਾ;
    hlist_del(&info->ਗੰਢ);
}

// Search all zones for an allocation obtained from alloc_new()
ਸਥਿਰ ਬਣਤਰ allocinfo_s *
alloc_ਲੱਭੋ(u32 ਅੰਕੜੇ)
{
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ARRAY_ਮਾਪ(Zones); i++) {
        ਬਣਤਰ allocinfo_s *info;
        hlist_for_each_ਇੰਦਰਾਜ(info, &Zones[i]->ਸਿਰ, ਗੰਢ) {
            ਜੇ (info->range_ਸ਼ੁਰੂ == ਅੰਕੜੇ)
                ਵਾਪਸ info;
        }
    }
    ਵਾਪਸ ਨਿਰਾ;
}

// Find the lowest memory range added by alloc_add()
ਸਥਿਰ ਬਣਤਰ allocinfo_s *
alloc_ਲੱਭੋ_ਸਭ_ਤੋਂ_ਨੀਵਾਂ(ਬਣਤਰ zone_s *zone)
{
    ਬਣਤਰ allocinfo_s *info, *ਆਖਰੀ = ਨਿਰਾ;
    hlist_for_each_ਇੰਦਰਾਜ(info, &zone->ਸਿਰ, ਗੰਢ) {
        ਆਖਰੀ = info;
    }
    ਵਾਪਸ ਆਖਰੀ;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ਸਥਿਰ ਪੂਰਨ
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_ਮਾਪ)
{
    u32 lowram = GET_BDA(mem_ਮਾਪ_kb) * 1024;
    ਜੇ (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        ਵਾਪਸ -1;

    // Do copy
    memmove((ਖਾਲੀ*)newebda, (ਖਾਲੀ*)oldebda, ebda_ਮਾਪ * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    ਸੈੱਟ_BDA(mem_ਮਾਪ_kb, newebda / 1024);
    ਸੈੱਟ_BDA(ebda_seg, FLATPTR_ਮੰਜ਼ਿਲ_SEG(newebda));
    ਵਾਪਸ 0;
}

// Support expanding the ZoneLow dynamically.
ਸਥਿਰ u32
zonelow_expand(u32 ਮਾਪ, u32 align, ਬਣਤਰ allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    ਜੇ (unlikely(wait_preempt())) {
        u32 ਅੰਕੜੇ = alloc_new(&Zone_ਨੀਵਾਂ, ਮਾਪ, align, fill);
        ਜੇ (ਅੰਕੜੇ)
            ਵਾਪਸ ਅੰਕੜੇ;
    }

    ਬਣਤਰ allocinfo_s *info = alloc_ਲੱਭੋ_ਸਭ_ਤੋਂ_ਨੀਵਾਂ(&Zone_ਨੀਵਾਂ);
    ਜੇ (!info)
        ਵਾਪਸ 0;
    u32 oldpos = info->range_ਸਿਰਾ;
    u32 newpos = ALIGN_ਥੱਲੇ(oldpos - ਮਾਪ, align);
    u32 bottom = info->range_ਸ਼ੁਰੂ + info->alloc_ਮਾਪ;
    ਜੇ (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        ਵਾਪਸ alloc_new(&Zone_ਨੀਵਾਂ, ਮਾਪ, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_ਮਾਪ = GET_EBDA(ebda_seg, ਮਾਪ);
    u32 ebda_ਸਿਰਾ = ebda_pos + ebda_ਮਾਪ * 1024;
    ਜੇ (ebda_ਸਿਰਾ != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_ਥੱਲੇ(ebda_ਸਿਰਾ - ਮਾਪ, align);
    u32 newbottom = ALIGN_ਥੱਲੇ(newpos, 1024);
    u32 newebda = ALIGN_ਥੱਲੇ(newbottom - ebda_ਮਾਪ * 1024, 1024);
    ਜੇ (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        ਵਾਪਸ 0;

    // Move ebda
    ਪੂਰਨ ret = relocate_ebda(newebda, ebda_pos, ebda_ਮਾਪ);
    ਜੇ (ret)
        ਵਾਪਸ 0;

    // Update zone
    ਜੇ (ebda_ਸਿਰਾ == bottom)
        info->range_ਸ਼ੁਰੂ = newbottom;
    ਨਹੀਂਤਾਂ
        alloc_ਜੋੜੋ(&Zone_ਨੀਵਾਂ, newbottom, ebda_ਸਿਰਾ);

    ਵਾਪਸ alloc_new(&Zone_ਨੀਵਾਂ, ਮਾਪ, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(ਬਣਤਰ zone_s *zone, u32 ਮਾਪ, u32 align)
{
    ASSERT32FLAT();
    ਜੇ (!ਮਾਪ)
        ਵਾਪਸ 0;

    // Find and reserve space for main allocation
    ਬਣਤਰ allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 ਅੰਕੜੇ = alloc_new(zone, ਮਾਪ, align, &tempdetail.datainfo);
    ਜੇ (!CONFIG_MALLOC_UPPERMEMORY && !ਅੰਕੜੇ && zone == &Zone_ਨੀਵਾਂ)
        ਅੰਕੜੇ = zonelow_expand(ਮਾਪ, align, &tempdetail.datainfo);
    ਜੇ (!ਅੰਕੜੇ)
        ਵਾਪਸ 0;

    // Find and reserve space for bookkeeping.
    ਬਣਤਰ allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ਜੇ (!detail) {
        alloc_free(&tempdetail.datainfo);
        ਵਾਪਸ 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, ਮਾਪ, align, ਅੰਕੜੇ, detail);

    ਵਾਪਸ ਅੰਕੜੇ;
}

// Allocate virtual memory from the given zone
ਖਾਲੀ * __malloc
_malloc(ਬਣਤਰ zone_s *zone, u32 ਮਾਪ, u32 align)
{
    ਵਾਪਸ memremap(malloc_palloc(zone, ਮਾਪ, align), ਮਾਪ);
}

// Free a data block allocated with phys_alloc
ਪੂਰਨ
malloc_pfree(u32 ਅੰਕੜੇ)
{
    ASSERT32FLAT();
    ਬਣਤਰ allocinfo_s *info = alloc_ਲੱਭੋ(ਅੰਕੜੇ);
    ਜੇ (!info || ਅੰਕੜੇ == virt_ਮੰਜ਼ਿਲ_phys(info) || !info->alloc_ਮਾਪ)
        ਵਾਪਸ -1;
    ਬਣਤਰ allocdetail_s *detail = container_ਦਾ(
        info, ਬਣਤਰ allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", ਅੰਕੜੇ, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    ਵਾਪਸ 0;
}

ਖਾਲੀ
ਮੁਕਤ(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਜੇ (!ਅੰਕੜੇ)
        ਵਾਪਸ;
    ਪੂਰਨ ret = malloc_pfree(virt_ਮੰਜ਼ਿਲ_phys(ਅੰਕੜੇ));
    ਜੇ (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(ਬਣਤਰ zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    ਬਣਤਰ allocinfo_s *info;
    hlist_for_each_ਇੰਦਰਾਜ(info, &zone->ਸਿਰ, ਗੰਢ) {
        u32 space = info->range_ਸਿਰਾ - info->range_ਸ਼ੁਰੂ - info->alloc_ਮਾਪ;
        ਜੇ (space > maxspace)
            maxspace = space;
    }

    ਜੇ (zone != &Zone_Tmp_ਉੱਚਾ && zone != &Zone_Tmp_ਨੀਵਾਂ)
        ਵਾਪਸ maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(ਆਕਾਰ(ਬਣਤਰ allocdetail_s), MALLOC_MIN_ALIGN);
    ਜੇ (maxspace <= reserve)
        ਵਾਪਸ 0;
    ਵਾਪਸ maxspace - reserve;
}

// Set a handle associated with an allocation.
ਖਾਲੀ
malloc_sethandle(u32 ਅੰਕੜੇ, u32 handle)
{
    ASSERT32FLAT();
    ਬਣਤਰ allocinfo_s *info = alloc_ਲੱਭੋ(ਅੰਕੜੇ);
    ਜੇ (!info || ਅੰਕੜੇ == virt_ਮੰਜ਼ਿਲ_phys(info) || !info->alloc_ਮਾਪ)
        ਵਾਪਸ;
    ਬਣਤਰ allocdetail_s *detail = container_ਦਾ(
        info, ਬਣਤਰ allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ARRAY_ਮਾਪ(Zones); i++) {
        ਬਣਤਰ allocinfo_s *info;
        hlist_for_each_ਇੰਦਰਾਜ(info, &Zones[i]->ਸਿਰ, ਗੰਢ) {
            ਜੇ (info->range_ਸ਼ੁਰੂ != virt_ਮੰਜ਼ਿਲ_phys(info))
                ਜਾਰੀ;
            ਬਣਤਰ allocdetail_s *detail = container_ਦਾ(
                info, ਬਣਤਰ allocdetail_s, detailinfo);
            ਜੇ (detail->handle == handle)
                ਵਾਪਸ detail->datainfo.range_ਸ਼ੁਰੂ;
        }
    }
    ਵਾਪਸ 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ਸਥਿਰ u32 Rom_ਸਿਰਾ = BUILD_ROM_ਸ਼ੁਰੂ;
ਸਥਿਰ ਬਣਤਰ allocinfo_s *Rom_ਅਧਾਰ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPROM_ਸਿਰਲੇਖ_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(ਖਾਲੀ)
{
    ਜੇ (CONFIG_MALLOC_UPPERMEMORY)
        ਵਾਪਸ ALIGN_ਥੱਲੇ(Rom_ਅਧਾਰ->range_ਸਿਰਾ - OPROM_ਸਿਰਲੇਖ_RESERVE
                          , OPTION_ROM_ALIGN);
    ਵਾਪਸ ਚਿੰਨ੍ਹ(final_readonly_ਸ਼ੁਰੂ);
}

// Return the end of the last deployed option rom.
u32
rom_get_ਆਖਰੀ(ਖਾਲੀ)
{
    ਵਾਪਸ Rom_ਸਿਰਾ;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
ਬਣਤਰ rom_ਸਿਰਲੇਖ *
rom_reserve(u32 ਮਾਪ)
{
    u32 newend = ALIGN(Rom_ਸਿਰਾ + ਮਾਪ, OPTION_ROM_ALIGN);
    ਜੇ (newend > rom_get_max())
        ਵਾਪਸ ਨਿਰਾ;
    ਜੇ (CONFIG_MALLOC_UPPERMEMORY) {
        ਜੇ (newend < ਚਿੰਨ੍ਹ(zonelow_ਅਧਾਰ))
            newend = ਚਿੰਨ੍ਹ(zonelow_ਅਧਾਰ);
        Rom_ਅਧਾਰ->range_ਸ਼ੁਰੂ = newend + OPROM_ਸਿਰਲੇਖ_RESERVE;
    }
    ਵਾਪਸ (ਖਾਲੀ*)Rom_ਸਿਰਾ;
}

// Confirm space as in use by an optionrom.
ਪੂਰਨ
rom_confirm(u32 ਮਾਪ)
{
    ਖਾਲੀ *new = rom_reserve(ਮਾਪ);
    ਜੇ (!new) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    Rom_ਸਿਰਾ = ALIGN(Rom_ਸਿਰਾ + ਮਾਪ, OPTION_ROM_ALIGN);
    ਵਾਪਸ 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

ਖਾਲੀ
malloc_preinit(ਖਾਲੀ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc preinit\n");

    // Don't declare any memory between 0xa0000 and 0x100000
    e820_remove(BUILD_LOWRAM_ਸਿਰਾ, BUILD_BIOS_ADDR-BUILD_LOWRAM_ਸਿਰਾ);

    // Mark known areas as reserved.
    e820_ਜੋੜੋ(BUILD_BIOS_ADDR, BUILD_BIOS_ਮਾਪ, E820_RESERVED);

    // Populate temp high ram
    u32 highram_ਸ਼ੁਰੂ = 0;
    u32 highram_ਮਾਪ = 0;
    ਪੂਰਨ i;
    ਲਈ (i=e820_ਗਿਣਤੀ-1; i>=0; i--) {
        ਬਣਤਰ e820entry *en = &e820_list[i];
        u64 ਸਿਰਾ = en->ਸ਼ੁਰੂ + en->ਮਾਪ;
        ਜੇ (ਸਿਰਾ < 1024*1024)
            ਰੋਕੋ;
        ਜੇ (en->type != E820_RAM || ਸਿਰਾ > 0xffffffff)
            ਜਾਰੀ;
        u32 s = en->ਸ਼ੁਰੂ, e = ਸਿਰਾ;
        ਜੇ (!highram_ਸ਼ੁਰੂ) {
            u32 new_max = ALIGN_ਥੱਲੇ(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_ਥੱਲੇ(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            ਜੇ (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_ਸ਼ੁਰੂ = e = new_max;
                highram_ਮਾਪ = BUILD_MAX_HIGHTABLE;
            } ਨਹੀਂਤਾਂ ਜੇ (new_min <= e && new_min >= s) {
                highram_ਸ਼ੁਰੂ = e = new_min;
                highram_ਮਾਪ = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_ਜੋੜੋ(&Zone_Tmp_ਉੱਚਾ, s, e);
    }

    // Populate regions
    alloc_ਜੋੜੋ(&Zone_Tmp_ਨੀਵਾਂ, BUILD_ਢੇਰ_ADDR, BUILD_EBDA_MINIMUM);
    ਜੇ (highram_ਸ਼ੁਰੂ) {
        alloc_ਜੋੜੋ(&Zone_ਉੱਚਾ, highram_ਸ਼ੁਰੂ, highram_ਸ਼ੁਰੂ + highram_ਮਾਪ);
        e820_ਜੋੜੋ(highram_ਸ਼ੁਰੂ, highram_ਮਾਪ, E820_RESERVED);
    }
}

ਖਾਲੀ
malloc_csm_preinit(u32 ਨੀਵਾਂ_pmm, u32 ਨੀਵਾਂ_pmm_ਮਾਪ, u32 hi_pmm, u32 hi_pmm_ਮਾਪ)
{
    ASSERT32FLAT();

    ਜੇ (hi_pmm_ਮਾਪ > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_ਸਿਰਾ = hi_pmm + hi_pmm_ਮਾਪ;
        alloc_ਜੋੜੋ(&Zone_Tmp_ਉੱਚਾ, hi_pmm, hi_pmm_ਸਿਰਾ - BUILD_MIN_HIGHTABLE);
        alloc_ਜੋੜੋ(&Zone_ਉੱਚਾ, hi_pmm_ਸਿਰਾ - BUILD_MIN_HIGHTABLE, hi_pmm_ਸਿਰਾ);
    } ਨਹੀਂਤਾਂ {
        alloc_ਜੋੜੋ(&Zone_Tmp_ਉੱਚਾ, hi_pmm, hi_pmm + hi_pmm_ਮਾਪ);
    }
    alloc_ਜੋੜੋ(&Zone_Tmp_ਨੀਵਾਂ, ਨੀਵਾਂ_pmm, ਨੀਵਾਂ_pmm + ਨੀਵਾਂ_pmm_ਮਾਪ);
}

u32 Legacy_Ram_ਮਾਪ VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ਸਥਿਰ ਖਾਲੀ
calc_Ram_ਮਾਪ(ਖਾਲੀ)
{
    u32 rs = 0;
    ਪੂਰਨ i;
    ਲਈ (i=e820_ਗਿਣਤੀ-1; i>=0; i--) {
        ਬਣਤਰ e820entry *en = &e820_list[i];
        u64 ਸਿਰਾ = en->ਸ਼ੁਰੂ + en->ਮਾਪ;
        u32 type = en->type;
        ਜੇ (ਸਿਰਾ <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = ਸਿਰਾ;
            ਰੋਕੋ;
        }
    }
    Legacy_Ram_ਮਾਪ = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
ਖਾਲੀ
malloc_init(ਖਾਲੀ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    ਜੇ (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        ਪੂਰਨ i;
        ਲਈ (i=0; i<ARRAY_ਮਾਪ(Zones); i++) {
            ਬਣਤਰ zone_s *zone = Zones[i];
            ਜੇ (zone->ਸਿਰ.ਪਹਿਲਾ)
                zone->ਸਿਰ.ਪਹਿਲਾ->pprev = &zone->ਸਿਰ.ਪਹਿਲਾ;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_ਸ਼ੁਰੂ), VSYMBOL(varlow_ਸ਼ੁਰੂ)
            , ਚਿੰਨ੍ਹ(varlow_ਸਿਰਾ) - ਚਿੰਨ੍ਹ(varlow_ਸ਼ੁਰੂ));
    ਜੇ (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_ਜੋੜੋ(&Zone_ਨੀਵਾਂ, ਚਿੰਨ੍ਹ(zonelow_ਅਧਾਰ) + OPROM_ਸਿਰਲੇਖ_RESERVE
                  , ਚਿੰਨ੍ਹ(final_varlow_ਸ਼ੁਰੂ));
        Rom_ਅਧਾਰ = alloc_ਲੱਭੋ_ਸਭ_ਤੋਂ_ਨੀਵਾਂ(&Zone_ਨੀਵਾਂ);
    } ਨਹੀਂਤਾਂ {
        alloc_ਜੋੜੋ(&Zone_ਨੀਵਾਂ, ALIGN_ਥੱਲੇ(ਚਿੰਨ੍ਹ(final_varlow_ਸ਼ੁਰੂ), 1024)
                  , ਚਿੰਨ੍ਹ(final_varlow_ਸ਼ੁਰੂ));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_ਸ਼ੁਰੂ), 0
           , ਚਿੰਨ੍ਹ(zonefseg_ਸਿਰਾ) - ਚਿੰਨ੍ਹ(zonefseg_ਸ਼ੁਰੂ));
    alloc_ਜੋੜੋ(&ZoneFSeg, ਚਿੰਨ੍ਹ(zonefseg_ਸ਼ੁਰੂ), ਚਿੰਨ੍ਹ(zonefseg_ਸਿਰਾ));

    calc_Ram_ਮਾਪ();
}

ਖਾਲੀ
malloc_prepboot(ਖਾਲੀ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 ਅਧਾਰ = rom_get_max();
    memset((ਖਾਲੀ*)Rom_ਸਿਰਾ, 0, ਅਧਾਰ-Rom_ਸਿਰਾ);
    ਜੇ (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        ਬਣਤਰ rom_ਸਿਰਲੇਖ *dummyrom = (ਖਾਲੀ*)ਅਧਾਰ;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        ਪੂਰਨ ਮਾਪ = (BUILD_BIOS_ADDR - ਅਧਾਰ) / 512;
        dummyrom->ਮਾਪ = (ਮਾਪ > 255) ? 255 : ਮਾਪ;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_ਮਾਪ_kb)*1024;
    e820_ਜੋੜੋ(endlow, BUILD_LOWRAM_ਸਿਰਾ-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    ਬਣਤਰ allocinfo_s *info = alloc_ਲੱਭੋ_ਸਭ_ਤੋਂ_ਨੀਵਾਂ(&ZoneFSeg);
    u32 ਮਾਪ = info->range_ਸਿਰਾ - info->range_ਸ਼ੁਰੂ;
    memset(memremap(info->range_ਸ਼ੁਰੂ, ਮਾਪ), 0, ਮਾਪ);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_ਸਿਰਾ, ਅਧਾਰ, info->range_ਸ਼ੁਰੂ, info->range_ਸਿਰਾ);

    // Give back unused high ram.
    info = alloc_ਲੱਭੋ_ਸਭ_ਤੋਂ_ਨੀਵਾਂ(&Zone_ਉੱਚਾ);
    ਜੇ (info) {
        u32 giveback = ALIGN_ਥੱਲੇ(info->range_ਸਿਰਾ-info->range_ਸ਼ੁਰੂ, ਪੰਨਾ_ਮਾਪ);
        e820_ਜੋੜੋ(info->range_ਸ਼ੁਰੂ, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_ਮਾਪ();
}
