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
ರಚನೆ allocinfo_s {
    ರಚನೆ hlist_ಗಂಟು ಗಂಟು;
    u32 range_ಆರಂಭ, range_ಕೊನೆ, alloc_ಗಾತ್ರ;
};

// Information on a tracked memory allocation.
ರಚನೆ allocdetail_s {
    ರಚನೆ allocinfo_s detailinfo;
    ರಚನೆ allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
ರಚನೆ zone_s {
    ರಚನೆ hlist_ತಲೆ ತಲೆ;
};

ರಚನೆ zone_s Zone_ಕೆಳಗಿನ VARVERIFY32INIT, Zone_ಮೇಲಿನ VARVERIFY32INIT;
ರಚನೆ zone_s ZoneFSeg VARVERIFY32INIT;
ರಚನೆ zone_s Zone_Tmp_ಕೆಳಗಿನ VARVERIFY32INIT, Zone_Tmp_ಮೇಲಿನ VARVERIFY32INIT;

ಸ್ಥಿರ ರಚನೆ zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_ಕೆಳಗಿನ, &Zone_ಕೆಳಗಿನ, &ZoneFSeg, &Zone_Tmp_ಮೇಲಿನ, &Zone_ಮೇಲಿನ
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ಸ್ಥಿರ u32
alloc_new(ರಚನೆ zone_s *zone, u32 ಗಾತ್ರ_2, u32 align, ರಚನೆ allocinfo_s *fill)
{
    ರಚನೆ allocinfo_s *info;
    hlist_for_each_ನಮೂದು(info, &zone->ತಲೆ, ಗಂಟು) {
        u32 alloc_ಕೊನೆ = info->range_ಆರಂಭ + info->alloc_ಗಾತ್ರ;
        u32 range_ಕೊನೆ = info->range_ಕೊನೆ;
        u32 new_range_ಕೊನೆ = ALIGN_ಕೆಳಕ್ಕೆ(range_ಕೊನೆ - ಗಾತ್ರ_2, align);
        ಆದರೆ (new_range_ಕೊನೆ >= alloc_ಕೊನೆ && new_range_ಕೊನೆ <= range_ಕೊನೆ) {
            // Found space - now reserve it.
            fill->range_ಆರಂಭ = new_range_ಕೊನೆ;
            fill->range_ಕೊನೆ = range_ಕೊನೆ;
            fill->alloc_ಗಾತ್ರ = ಗಾತ್ರ_2;

            info->range_ಕೊನೆ = new_range_ಕೊನೆ;
            hlist_ಕೂಡಿಸು_before(&fill->ಗಂಟು, &info->ಗಂಟು);
            ಹಿಂತಿರುಗಿ new_range_ಕೊನೆ;
        }
    }
    ಹಿಂತಿರುಗಿ 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ಸ್ಥಿರ ರಚನೆ allocdetail_s *
alloc_new_detail(ರಚನೆ allocdetail_s *ತಾತ್ಕಾಲಿಕ)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_ಮೇಲಿನ, ಗಾತ್ರ(ರಚನೆ allocdetail_s)
                                , MALLOC_MIN_ALIGN, &ತಾತ್ಕಾಲಿಕ->detailinfo);
    ಆದರೆ (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_ಕೆಳಗಿನ, ಗಾತ್ರ(ರಚನೆ allocdetail_s)
                                , MALLOC_MIN_ALIGN, &ತಾತ್ಕಾಲಿಕ->detailinfo);
        ಆದರೆ (!detail_addr) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        }
    }
    ರಚನೆ allocdetail_s *detail = memremap(detail_addr, ಗಾತ್ರ(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    ಸ್ಮರಣೆ_ನಕಲು(detail, ತಾತ್ಕಾಲಿಕ, ಗಾತ್ರ(*detail));
    hlist_replace(&ತಾತ್ಕಾಲಿಕ->detailinfo.ಗಂಟು, &detail->detailinfo.ಗಂಟು);
    hlist_replace(&ತಾತ್ಕಾಲಿಕ->datainfo.ಗಂಟು, &detail->datainfo.ಗಂಟು);
    ಹಿಂತಿರುಗಿ detail;
}

// Add new memory to a zone
ಸ್ಥಿರ ಶೂನ್ಯ
alloc_ಕೂಡಿಸು(ರಚನೆ zone_s *zone, u32 ಆರಂಭ, u32 ಕೊನೆ)
{
    // Find position to add space
    ರಚನೆ allocinfo_s *info;
    ರಚನೆ hlist_ಗಂಟು **pprev;
    hlist_for_each_ನಮೂದು_pprev(info, pprev, &zone->ತಲೆ, ಗಂಟು) {
        ಆದರೆ (info->range_ಆರಂಭ < ಆರಂಭ)
            ನಿಲ್ಲಿಸು;
    }

    // Add space using temporary allocation info.
    ರಚನೆ allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_ಆರಂಭ = ಆರಂಭ;
    tempdetail.datainfo.range_ಕೊನೆ = ಕೊನೆ;
    tempdetail.datainfo.alloc_ಗಾತ್ರ = 0;
    hlist_ಕೂಡಿಸು(&tempdetail.datainfo.ಗಂಟು, pprev);

    // Allocate final allocation info.
    ರಚನೆ allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ಆದರೆ (!detail)
        hlist_del(&tempdetail.datainfo.ಗಂಟು);
}

// Release space allocated with alloc_new()
ಸ್ಥಿರ ಶೂನ್ಯ
alloc_free(ರಚನೆ allocinfo_s *info)
{
    ರಚನೆ allocinfo_s *ಮುಂದಿನ = container_ದ_or_null(
        info->ಗಂಟು.ಮುಂದಿನ, ರಚನೆ allocinfo_s, ಗಂಟು);
    ಆದರೆ (ಮುಂದಿನ && ಮುಂದಿನ->range_ಕೊನೆ == info->range_ಆರಂಭ)
        ಮುಂದಿನ->range_ಕೊನೆ = info->range_ಕೊನೆ;
    hlist_del(&info->ಗಂಟು);
}

// Search all zones for an allocation obtained from alloc_new()
ಸ್ಥಿರ ರಚನೆ allocinfo_s *
alloc_ಕಂಡುಹಿಡಿ(u32 ದತ್ತಾಂಶ)
{
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(Zones); i++) {
        ರಚನೆ allocinfo_s *info;
        hlist_for_each_ನಮೂದು(info, &Zones[i]->ತಲೆ, ಗಂಟು) {
            ಆದರೆ (info->range_ಆರಂಭ == ದತ್ತಾಂಶ)
                ಹಿಂತಿರುಗಿ info;
        }
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

// Find the lowest memory range added by alloc_add()
ಸ್ಥಿರ ರಚನೆ allocinfo_s *
alloc_ಕಂಡುಹಿಡಿ_ಅತಿಕಡಿಮೆ(ರಚನೆ zone_s *zone)
{
    ರಚನೆ allocinfo_s *info, *ಕೊನೆಯ = ಖಾಲಿ;
    hlist_for_each_ನಮೂದು(info, &zone->ತಲೆ, ಗಂಟು) {
        ಕೊನೆಯ = info;
    }
    ಹಿಂತಿರುಗಿ ಕೊನೆಯ;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_ಗಾತ್ರ)
{
    u32 lowram = GET_BDA(mem_ಗಾತ್ರ_kb) * 1024;
    ಆದರೆ (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        ಹಿಂತಿರುಗಿ -1;

    // Do copy
    memmove((ಶೂನ್ಯ*)newebda, (ಶೂನ್ಯ*)oldebda, ebda_ಗಾತ್ರ * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    ಗಣ_BDA(mem_ಗಾತ್ರ_kb, newebda / 1024);
    ಗಣ_BDA(ebda_seg, FLATPTR_ಗಮ್ಯ_SEG(newebda));
    ಹಿಂತಿರುಗಿ 0;
}

// Support expanding the ZoneLow dynamically.
ಸ್ಥಿರ u32
zonelow_expand(u32 ಗಾತ್ರ_2, u32 align, ರಚನೆ allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    ಆದರೆ (unlikely(wait_preempt())) {
        u32 ದತ್ತಾಂಶ = alloc_new(&Zone_ಕೆಳಗಿನ, ಗಾತ್ರ_2, align, fill);
        ಆದರೆ (ದತ್ತಾಂಶ)
            ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
    }

    ರಚನೆ allocinfo_s *info = alloc_ಕಂಡುಹಿಡಿ_ಅತಿಕಡಿಮೆ(&Zone_ಕೆಳಗಿನ);
    ಆದರೆ (!info)
        ಹಿಂತಿರುಗಿ 0;
    u32 oldpos = info->range_ಕೊನೆ;
    u32 newpos = ALIGN_ಕೆಳಕ್ಕೆ(oldpos - ಗಾತ್ರ_2, align);
    u32 bottom = info->range_ಆರಂಭ + info->alloc_ಗಾತ್ರ;
    ಆದರೆ (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        ಹಿಂತಿರುಗಿ alloc_new(&Zone_ಕೆಳಗಿನ, ಗಾತ್ರ_2, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_ಗಾತ್ರ = GET_EBDA(ebda_seg, ಗಾತ್ರ_2);
    u32 ebda_ಕೊನೆ = ebda_pos + ebda_ಗಾತ್ರ * 1024;
    ಆದರೆ (ebda_ಕೊನೆ != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_ಕೆಳಕ್ಕೆ(ebda_ಕೊನೆ - ಗಾತ್ರ_2, align);
    u32 newbottom = ALIGN_ಕೆಳಕ್ಕೆ(newpos, 1024);
    u32 newebda = ALIGN_ಕೆಳಕ್ಕೆ(newbottom - ebda_ಗಾತ್ರ * 1024, 1024);
    ಆದರೆ (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        ಹಿಂತಿರುಗಿ 0;

    // Move ebda
    ಪೂರ್ಣಾಂಕ ret = relocate_ebda(newebda, ebda_pos, ebda_ಗಾತ್ರ);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ 0;

    // Update zone
    ಆದರೆ (ebda_ಕೊನೆ == bottom)
        info->range_ಆರಂಭ = newbottom;
    ಇಲ್ಲದಿದ್ದರೆ
        alloc_ಕೂಡಿಸು(&Zone_ಕೆಳಗಿನ, newbottom, ebda_ಕೊನೆ);

    ಹಿಂತಿರುಗಿ alloc_new(&Zone_ಕೆಳಗಿನ, ಗಾತ್ರ_2, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(ರಚನೆ zone_s *zone, u32 ಗಾತ್ರ_2, u32 align)
{
    ASSERT32FLAT();
    ಆದರೆ (!ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ 0;

    // Find and reserve space for main allocation
    ರಚನೆ allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 ದತ್ತಾಂಶ = alloc_new(zone, ಗಾತ್ರ_2, align, &tempdetail.datainfo);
    ಆದರೆ (!CONFIG_MALLOC_UPPERMEMORY && !ದತ್ತಾಂಶ && zone == &Zone_ಕೆಳಗಿನ)
        ದತ್ತಾಂಶ = zonelow_expand(ಗಾತ್ರ_2, align, &tempdetail.datainfo);
    ಆದರೆ (!ದತ್ತಾಂಶ)
        ಹಿಂತಿರುಗಿ 0;

    // Find and reserve space for bookkeeping.
    ರಚನೆ allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ಆದರೆ (!detail) {
        alloc_free(&tempdetail.datainfo);
        ಹಿಂತಿರುಗಿ 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, ಗಾತ್ರ_2, align, ದತ್ತಾಂಶ, detail);

    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
}

// Allocate virtual memory from the given zone
ಶೂನ್ಯ * __malloc
_malloc(ರಚನೆ zone_s *zone, u32 ಗಾತ್ರ_2, u32 align)
{
    ಹಿಂತಿರುಗಿ memremap(malloc_palloc(zone, ಗಾತ್ರ_2, align), ಗಾತ್ರ_2);
}

// Free a data block allocated with phys_alloc
ಪೂರ್ಣಾಂಕ
malloc_pfree(u32 ದತ್ತಾಂಶ)
{
    ASSERT32FLAT();
    ರಚನೆ allocinfo_s *info = alloc_ಕಂಡುಹಿಡಿ(ದತ್ತಾಂಶ);
    ಆದರೆ (!info || ದತ್ತಾಂಶ == virt_ಗಮ್ಯ_phys(info) || !info->alloc_ಗಾತ್ರ)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ allocdetail_s *detail = container_ದ(
        info, ರಚನೆ allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", ದತ್ತಾಂಶ, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ
ಬಿಡುಗಡೆ(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ಆದರೆ (!ದತ್ತಾಂಶ)
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ ret = malloc_pfree(virt_ಗಮ್ಯ_phys(ದತ್ತಾಂಶ));
    ಆದರೆ (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(ರಚನೆ zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    ರಚನೆ allocinfo_s *info;
    hlist_for_each_ನಮೂದು(info, &zone->ತಲೆ, ಗಂಟು) {
        u32 space = info->range_ಕೊನೆ - info->range_ಆರಂಭ - info->alloc_ಗಾತ್ರ;
        ಆದರೆ (space > maxspace)
            maxspace = space;
    }

    ಆದರೆ (zone != &Zone_Tmp_ಮೇಲಿನ && zone != &Zone_Tmp_ಕೆಳಗಿನ)
        ಹಿಂತಿರುಗಿ maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(ಗಾತ್ರ(ರಚನೆ allocdetail_s), MALLOC_MIN_ALIGN);
    ಆದರೆ (maxspace <= reserve)
        ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ maxspace - reserve;
}

// Set a handle associated with an allocation.
ಶೂನ್ಯ
malloc_sethandle(u32 ದತ್ತಾಂಶ, u32 handle)
{
    ASSERT32FLAT();
    ರಚನೆ allocinfo_s *info = alloc_ಕಂಡುಹಿಡಿ(ದತ್ತಾಂಶ);
    ಆದರೆ (!info || ದತ್ತಾಂಶ == virt_ಗಮ್ಯ_phys(info) || !info->alloc_ಗಾತ್ರ)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ allocdetail_s *detail = container_ದ(
        info, ರಚನೆ allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(Zones); i++) {
        ರಚನೆ allocinfo_s *info;
        hlist_for_each_ನಮೂದು(info, &Zones[i]->ತಲೆ, ಗಂಟು) {
            ಆದರೆ (info->range_ಆರಂಭ != virt_ಗಮ್ಯ_phys(info))
                ಮುಂದುವರಿ;
            ರಚನೆ allocdetail_s *detail = container_ದ(
                info, ರಚನೆ allocdetail_s, detailinfo);
            ಆದರೆ (detail->handle == handle)
                ಹಿಂತಿರುಗಿ detail->datainfo.range_ಆರಂಭ;
        }
    }
    ಹಿಂತಿರುಗಿ 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ಸ್ಥಿರ u32 Rom_ಕೊನೆ = BUILD_ROM_ಆರಂಭ;
ಸ್ಥಿರ ರಚನೆ allocinfo_s *Rom_ತಳ;

#ವ್ಯಾಖ್ಯಾನಿಸು OPROM_ಶೀರ್ಷಿಕೆ_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(ಶೂನ್ಯ)
{
    ಆದರೆ (CONFIG_MALLOC_UPPERMEMORY)
        ಹಿಂತಿರುಗಿ ALIGN_ಕೆಳಕ್ಕೆ(Rom_ತಳ->range_ಕೊನೆ - OPROM_ಶೀರ್ಷಿಕೆ_RESERVE
                          , OPTION_ROM_ALIGN);
    ಹಿಂತಿರುಗಿ ಚಿಹ್ನೆ(final_readonly_ಆರಂಭ);
}

// Return the end of the last deployed option rom.
u32
rom_get_ಕೊನೆಯ(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ Rom_ಕೊನೆ;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *
rom_reserve(u32 ಗಾತ್ರ_2)
{
    u32 newend = ALIGN(Rom_ಕೊನೆ + ಗಾತ್ರ_2, OPTION_ROM_ALIGN);
    ಆದರೆ (newend > rom_get_max())
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಆದರೆ (CONFIG_MALLOC_UPPERMEMORY) {
        ಆದರೆ (newend < ಚಿಹ್ನೆ(zonelow_ತಳ))
            newend = ಚಿಹ್ನೆ(zonelow_ತಳ);
        Rom_ತಳ->range_ಆರಂಭ = newend + OPROM_ಶೀರ್ಷಿಕೆ_RESERVE;
    }
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)Rom_ಕೊನೆ;
}

// Confirm space as in use by an optionrom.
ಪೂರ್ಣಾಂಕ
rom_confirm(u32 ಗಾತ್ರ_2)
{
    ಶೂನ್ಯ *new = rom_reserve(ಗಾತ್ರ_2);
    ಆದರೆ (!new) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    Rom_ಕೊನೆ = ALIGN(Rom_ಕೊನೆ + ಗಾತ್ರ_2, OPTION_ROM_ALIGN);
    ಹಿಂತಿರುಗಿ 0;
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
    e820_remove(BUILD_LOWRAM_ಕೊನೆ, BUILD_BIOS_ADDR-BUILD_LOWRAM_ಕೊನೆ);

    // Mark known areas as reserved.
    e820_ಕೂಡಿಸು(BUILD_BIOS_ADDR, BUILD_BIOS_ಗಾತ್ರ, E820_RESERVED);

    // Populate temp high ram
    u32 highram_ಆರಂಭ = 0;
    u32 highram_ಗಾತ್ರ = 0;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=e820_ಎಣಿಕೆ-1; i>=0; i--) {
        ರಚನೆ e820entry *en = &e820_list[i];
        u64 ಕೊನೆ = en->ಆರಂಭ + en->ಗಾತ್ರ_2;
        ಆದರೆ (ಕೊನೆ < 1024*1024)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (en->type != E820_RAM || ಕೊನೆ > 0xffffffff)
            ಮುಂದುವರಿ;
        u32 s = en->ಆರಂಭ, e = ಕೊನೆ;
        ಆದರೆ (!highram_ಆರಂಭ) {
            u32 new_max = ALIGN_ಕೆಳಕ್ಕೆ(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_ಕೆಳಕ್ಕೆ(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            ಆದರೆ (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_ಆರಂಭ = e = new_max;
                highram_ಗಾತ್ರ = BUILD_MAX_HIGHTABLE;
            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (new_min <= e && new_min >= s) {
                highram_ಆರಂಭ = e = new_min;
                highram_ಗಾತ್ರ = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_ಕೂಡಿಸು(&Zone_Tmp_ಮೇಲಿನ, s, e);
    }

    // Populate regions
    alloc_ಕೂಡಿಸು(&Zone_Tmp_ಕೆಳಗಿನ, BUILD_ರಾಶಿ_ADDR, BUILD_EBDA_MINIMUM);
    ಆದರೆ (highram_ಆರಂಭ) {
        alloc_ಕೂಡಿಸು(&Zone_ಮೇಲಿನ, highram_ಆರಂಭ, highram_ಆರಂಭ + highram_ಗಾತ್ರ);
        e820_ಕೂಡಿಸು(highram_ಆರಂಭ, highram_ಗಾತ್ರ, E820_RESERVED);
    }
}

ಶೂನ್ಯ
malloc_csm_preinit(u32 ಕೆಳಗಿನ_pmm, u32 ಕೆಳಗಿನ_pmm_ಗಾತ್ರ, u32 hi_pmm, u32 hi_pmm_ಗಾತ್ರ)
{
    ASSERT32FLAT();

    ಆದರೆ (hi_pmm_ಗಾತ್ರ > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_ಕೊನೆ = hi_pmm + hi_pmm_ಗಾತ್ರ;
        alloc_ಕೂಡಿಸು(&Zone_Tmp_ಮೇಲಿನ, hi_pmm, hi_pmm_ಕೊನೆ - BUILD_MIN_HIGHTABLE);
        alloc_ಕೂಡಿಸು(&Zone_ಮೇಲಿನ, hi_pmm_ಕೊನೆ - BUILD_MIN_HIGHTABLE, hi_pmm_ಕೊನೆ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        alloc_ಕೂಡಿಸು(&Zone_Tmp_ಮೇಲಿನ, hi_pmm, hi_pmm + hi_pmm_ಗಾತ್ರ);
    }
    alloc_ಕೂಡಿಸು(&Zone_Tmp_ಕೆಳಗಿನ, ಕೆಳಗಿನ_pmm, ಕೆಳಗಿನ_pmm + ಕೆಳಗಿನ_pmm_ಗಾತ್ರ);
}

u32 Legacy_Ram_ಗಾತ್ರ VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ಸ್ಥಿರ ಶೂನ್ಯ
calc_Ram_ಗಾತ್ರ(ಶೂನ್ಯ)
{
    u32 rs = 0;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=e820_ಎಣಿಕೆ-1; i>=0; i--) {
        ರಚನೆ e820entry *en = &e820_list[i];
        u64 ಕೊನೆ = en->ಆರಂಭ + en->ಗಾತ್ರ_2;
        u32 type = en->type;
        ಆದರೆ (ಕೊನೆ <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = ಕೊನೆ;
            ನಿಲ್ಲಿಸು;
        }
    }
    Legacy_Ram_ಗಾತ್ರ = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
ಶೂನ್ಯ
malloc_init(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    ಆದರೆ (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(Zones); i++) {
            ರಚನೆ zone_s *zone = Zones[i];
            ಆದರೆ (zone->ತಲೆ.ಮೊದಲ)
                zone->ತಲೆ.ಮೊದಲ->pprev = &zone->ತಲೆ.ಮೊದಲ;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_ಆರಂಭ), VSYMBOL(varlow_ಆರಂಭ)
            , ಚಿಹ್ನೆ(varlow_ಕೊನೆ) - ಚಿಹ್ನೆ(varlow_ಆರಂಭ));
    ಆದರೆ (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_ಕೂಡಿಸು(&Zone_ಕೆಳಗಿನ, ಚಿಹ್ನೆ(zonelow_ತಳ) + OPROM_ಶೀರ್ಷಿಕೆ_RESERVE
                  , ಚಿಹ್ನೆ(final_varlow_ಆರಂಭ));
        Rom_ತಳ = alloc_ಕಂಡುಹಿಡಿ_ಅತಿಕಡಿಮೆ(&Zone_ಕೆಳಗಿನ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        alloc_ಕೂಡಿಸು(&Zone_ಕೆಳಗಿನ, ALIGN_ಕೆಳಕ್ಕೆ(ಚಿಹ್ನೆ(final_varlow_ಆರಂಭ), 1024)
                  , ಚಿಹ್ನೆ(final_varlow_ಆರಂಭ));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_ಆರಂಭ), 0
           , ಚಿಹ್ನೆ(zonefseg_ಕೊನೆ) - ಚಿಹ್ನೆ(zonefseg_ಆರಂಭ));
    alloc_ಕೂಡಿಸು(&ZoneFSeg, ಚಿಹ್ನೆ(zonefseg_ಆರಂಭ), ಚಿಹ್ನೆ(zonefseg_ಕೊನೆ));

    calc_Ram_ಗಾತ್ರ();
}

ಶೂನ್ಯ
malloc_prepboot(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 ತಳ = rom_get_max();
    memset((ಶೂನ್ಯ*)Rom_ಕೊನೆ, 0, ತಳ-Rom_ಕೊನೆ);
    ಆದರೆ (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *dummyrom = (ಶೂನ್ಯ*)ತಳ;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = (BUILD_BIOS_ADDR - ತಳ) / 512;
        dummyrom->ಗಾತ್ರ_2 = (ಗಾತ್ರ_2 > 255) ? 255 : ಗಾತ್ರ_2;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_ಗಾತ್ರ_kb)*1024;
    e820_ಕೂಡಿಸು(endlow, BUILD_LOWRAM_ಕೊನೆ-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    ರಚನೆ allocinfo_s *info = alloc_ಕಂಡುಹಿಡಿ_ಅತಿಕಡಿಮೆ(&ZoneFSeg);
    u32 ಗಾತ್ರ_2 = info->range_ಕೊನೆ - info->range_ಆರಂಭ;
    memset(memremap(info->range_ಆರಂಭ, ಗಾತ್ರ_2), 0, ಗಾತ್ರ_2);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_ಕೊನೆ, ತಳ, info->range_ಆರಂಭ, info->range_ಕೊನೆ);

    // Give back unused high ram.
    info = alloc_ಕಂಡುಹಿಡಿ_ಅತಿಕಡಿಮೆ(&Zone_ಮೇಲಿನ);
    ಆದರೆ (info) {
        u32 giveback = ALIGN_ಕೆಳಕ್ಕೆ(info->range_ಕೊನೆ-info->range_ಆರಂಭ, ಪುಟ_ಗಾತ್ರ);
        e820_ಕೂಡಿಸು(info->range_ಆರಂಭ, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_ಗಾತ್ರ();
}
