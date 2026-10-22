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
    بنية hlist_غوټه غوټه;
    u32 range_پیل, range_پای_ټکی, alloc_اندازه;
};

// Information on a tracked memory allocation.
بنية allocdetail_s {
    بنية allocinfo_s detailinfo;
    بنية allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
بنية zone_s {
    بنية hlist_سر سر;
};

بنية zone_s Zone_ټیټ VARVERIFY32INIT, Zone_لوړ VARVERIFY32INIT;
بنية zone_s ZoneFSeg VARVERIFY32INIT;
بنية zone_s Zone_Tmp_ټیټ VARVERIFY32INIT, Zone_Tmp_لوړ VARVERIFY32INIT;

ساكن بنية zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_ټیټ, &Zone_ټیټ, &ZoneFSeg, &Zone_Tmp_لوړ, &Zone_لوړ
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ساكن u32
alloc_new(بنية zone_s *zone, u32 اندازه, u32 align, بنية allocinfo_s *fill)
{
    بنية allocinfo_s *info;
    hlist_for_each_ثبت(info, &zone->سر, غوټه) {
        u32 alloc_پای_ټکی = info->range_پیل + info->alloc_اندازه;
        u32 range_پای_ټکی = info->range_پای_ټکی;
        u32 new_range_پای_ټکی = ALIGN_ښکته(range_پای_ټکی - اندازه, align);
        إذا (new_range_پای_ټکی >= alloc_پای_ټکی && new_range_پای_ټکی <= range_پای_ټکی) {
            // Found space - now reserve it.
            fill->range_پیل = new_range_پای_ټکی;
            fill->range_پای_ټکی = range_پای_ټکی;
            fill->alloc_اندازه = اندازه;

            info->range_پای_ټکی = new_range_پای_ټکی;
            hlist_جمع_کړه_before(&fill->غوټه, &info->غوټه);
            أرجع new_range_پای_ټکی;
        }
    }
    أرجع 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ساكن بنية allocdetail_s *
alloc_new_detail(بنية allocdetail_s *لنډمهاله)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_لوړ, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &لنډمهاله->detailinfo);
    إذا (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_ټیټ, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &لنډمهاله->detailinfo);
        إذا (!detail_addr) {
            warn_noalloc();
            أرجع عدم;
        }
    }
    بنية allocdetail_s *detail = memremap(detail_addr, حجم(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    حافظه_نقل(detail, لنډمهاله, حجم(*detail));
    hlist_replace(&لنډمهاله->detailinfo.غوټه, &detail->detailinfo.غوټه);
    hlist_replace(&لنډمهاله->datainfo.غوټه, &detail->datainfo.غوټه);
    أرجع detail;
}

// Add new memory to a zone
ساكن فراغ
alloc_جمع_کړه(بنية zone_s *zone, u32 پیل, u32 پای_ټکی)
{
    // Find position to add space
    بنية allocinfo_s *info;
    بنية hlist_غوټه **pprev;
    hlist_for_each_ثبت_pprev(info, pprev, &zone->سر, غوټه) {
        إذا (info->range_پیل < پیل)
            اكسر;
    }

    // Add space using temporary allocation info.
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_پیل = پیل;
    tempdetail.datainfo.range_پای_ټکی = پای_ټکی;
    tempdetail.datainfo.alloc_اندازه = 0;
    hlist_جمع_کړه(&tempdetail.datainfo.غوټه, pprev);

    // Allocate final allocation info.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail)
        hlist_del(&tempdetail.datainfo.غوټه);
}

// Release space allocated with alloc_new()
ساكن فراغ
alloc_free(بنية allocinfo_s *info)
{
    بنية allocinfo_s *بل = container_د_or_null(
        info->غوټه.بل, بنية allocinfo_s, غوټه);
    إذا (بل && بل->range_پای_ټکی == info->range_پیل)
        بل->range_پای_ټکی = info->range_پای_ټکی;
    hlist_del(&info->غوټه);
}

// Search all zones for an allocation obtained from alloc_new()
ساكن بنية allocinfo_s *
alloc_پیدا_کړه(u32 معلومات)
{
    صحيح i;
    لكل (i=0; i<ARRAY_اندازه(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_ثبت(info, &Zones[i]->سر, غوټه) {
            إذا (info->range_پیل == معلومات)
                أرجع info;
        }
    }
    أرجع عدم;
}

// Find the lowest memory range added by alloc_add()
ساكن بنية allocinfo_s *
alloc_پیدا_کړه_ټیټ_ترین(بنية zone_s *zone)
{
    بنية allocinfo_s *info, *وروستی = عدم;
    hlist_for_each_ثبت(info, &zone->سر, غوټه) {
        وروستی = info;
    }
    أرجع وروستی;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ساكن صحيح
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_اندازه)
{
    u32 lowram = GET_BDA(mem_اندازه_kb) * 1024;
    إذا (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        أرجع -1;

    // Do copy
    memmove((فراغ*)newebda, (فراغ*)oldebda, ebda_اندازه * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    سېټ_BDA(mem_اندازه_kb, newebda / 1024);
    سېټ_BDA(ebda_seg, FLATPTR_منزل_SEG(newebda));
    أرجع 0;
}

// Support expanding the ZoneLow dynamically.
ساكن u32
zonelow_expand(u32 اندازه, u32 align, بنية allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    إذا (unlikely(wait_preempt())) {
        u32 معلومات = alloc_new(&Zone_ټیټ, اندازه, align, fill);
        إذا (معلومات)
            أرجع معلومات;
    }

    بنية allocinfo_s *info = alloc_پیدا_کړه_ټیټ_ترین(&Zone_ټیټ);
    إذا (!info)
        أرجع 0;
    u32 oldpos = info->range_پای_ټکی;
    u32 newpos = ALIGN_ښکته(oldpos - اندازه, align);
    u32 bottom = info->range_پیل + info->alloc_اندازه;
    إذا (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        أرجع alloc_new(&Zone_ټیټ, اندازه, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_اندازه = GET_EBDA(ebda_seg, اندازه);
    u32 ebda_پای_ټکی = ebda_pos + ebda_اندازه * 1024;
    إذا (ebda_پای_ټکی != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_ښکته(ebda_پای_ټکی - اندازه, align);
    u32 newbottom = ALIGN_ښکته(newpos, 1024);
    u32 newebda = ALIGN_ښکته(newbottom - ebda_اندازه * 1024, 1024);
    إذا (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        أرجع 0;

    // Move ebda
    صحيح ret = relocate_ebda(newebda, ebda_pos, ebda_اندازه);
    إذا (ret)
        أرجع 0;

    // Update zone
    إذا (ebda_پای_ټکی == bottom)
        info->range_پیل = newbottom;
    وإلا
        alloc_جمع_کړه(&Zone_ټیټ, newbottom, ebda_پای_ټکی);

    أرجع alloc_new(&Zone_ټیټ, اندازه, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(بنية zone_s *zone, u32 اندازه, u32 align)
{
    ASSERT32FLAT();
    إذا (!اندازه)
        أرجع 0;

    // Find and reserve space for main allocation
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 معلومات = alloc_new(zone, اندازه, align, &tempdetail.datainfo);
    إذا (!CONFIG_MALLOC_UPPERMEMORY && !معلومات && zone == &Zone_ټیټ)
        معلومات = zonelow_expand(اندازه, align, &tempdetail.datainfo);
    إذا (!معلومات)
        أرجع 0;

    // Find and reserve space for bookkeeping.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail) {
        alloc_free(&tempdetail.datainfo);
        أرجع 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, اندازه, align, معلومات, detail);

    أرجع معلومات;
}

// Allocate virtual memory from the given zone
فراغ * __malloc
_malloc(بنية zone_s *zone, u32 اندازه, u32 align)
{
    أرجع memremap(malloc_palloc(zone, اندازه, align), اندازه);
}

// Free a data block allocated with phys_alloc
صحيح
malloc_pfree(u32 معلومات)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_پیدا_کړه(معلومات);
    إذا (!info || معلومات == virt_منزل_phys(info) || !info->alloc_اندازه)
        أرجع -1;
    بنية allocdetail_s *detail = container_د(
        info, بنية allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", معلومات, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    أرجع 0;
}

فراغ
حرر(فراغ *معلومات)
{
    إذا (!معلومات)
        أرجع;
    صحيح ret = malloc_pfree(virt_منزل_phys(معلومات));
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
    hlist_for_each_ثبت(info, &zone->سر, غوټه) {
        u32 space = info->range_پای_ټکی - info->range_پیل - info->alloc_اندازه;
        إذا (space > maxspace)
            maxspace = space;
    }

    إذا (zone != &Zone_Tmp_لوړ && zone != &Zone_Tmp_ټیټ)
        أرجع maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(حجم(بنية allocdetail_s), MALLOC_MIN_ALIGN);
    إذا (maxspace <= reserve)
        أرجع 0;
    أرجع maxspace - reserve;
}

// Set a handle associated with an allocation.
فراغ
malloc_sethandle(u32 معلومات, u32 handle)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_پیدا_کړه(معلومات);
    إذا (!info || معلومات == virt_منزل_phys(info) || !info->alloc_اندازه)
        أرجع;
    بنية allocdetail_s *detail = container_د(
        info, بنية allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    صحيح i;
    لكل (i=0; i<ARRAY_اندازه(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_ثبت(info, &Zones[i]->سر, غوټه) {
            إذا (info->range_پیل != virt_منزل_phys(info))
                استمر;
            بنية allocdetail_s *detail = container_د(
                info, بنية allocdetail_s, detailinfo);
            إذا (detail->handle == handle)
                أرجع detail->datainfo.range_پیل;
        }
    }
    أرجع 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ساكن u32 Rom_پای_ټکی = BUILD_ROM_پیل;
ساكن بنية allocinfo_s *Rom_بنسټ;

#تعریف_کړه OPROM_سرلیک_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(فراغ)
{
    إذا (CONFIG_MALLOC_UPPERMEMORY)
        أرجع ALIGN_ښکته(Rom_بنسټ->range_پای_ټکی - OPROM_سرلیک_RESERVE
                          , OPTION_ROM_ALIGN);
    أرجع نښه(final_readonly_پیل);
}

// Return the end of the last deployed option rom.
u32
rom_get_وروستی(فراغ)
{
    أرجع Rom_پای_ټکی;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
بنية rom_سرلیک *
rom_reserve(u32 اندازه)
{
    u32 newend = ALIGN(Rom_پای_ټکی + اندازه, OPTION_ROM_ALIGN);
    إذا (newend > rom_get_max())
        أرجع عدم;
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        إذا (newend < نښه(zonelow_بنسټ))
            newend = نښه(zonelow_بنسټ);
        Rom_بنسټ->range_پیل = newend + OPROM_سرلیک_RESERVE;
    }
    أرجع (فراغ*)Rom_پای_ټکی;
}

// Confirm space as in use by an optionrom.
صحيح
rom_confirm(u32 اندازه)
{
    فراغ *new = rom_reserve(اندازه);
    إذا (!new) {
        warn_noalloc();
        أرجع -1;
    }
    Rom_پای_ټکی = ALIGN(Rom_پای_ټکی + اندازه, OPTION_ROM_ALIGN);
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
    e820_remove(BUILD_LOWRAM_پای_ټکی, BUILD_BIOS_ADDR-BUILD_LOWRAM_پای_ټکی);

    // Mark known areas as reserved.
    e820_جمع_کړه(BUILD_BIOS_ADDR, BUILD_BIOS_اندازه, E820_RESERVED);

    // Populate temp high ram
    u32 highram_پیل = 0;
    u32 highram_اندازه = 0;
    صحيح i;
    لكل (i=e820_شمېر-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 پای_ټکی = en->پیل + en->اندازه;
        إذا (پای_ټکی < 1024*1024)
            اكسر;
        إذا (en->type != E820_RAM || پای_ټکی > 0xffffffff)
            استمر;
        u32 s = en->پیل, e = پای_ټکی;
        إذا (!highram_پیل) {
            u32 new_max = ALIGN_ښکته(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_ښکته(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            إذا (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_پیل = e = new_max;
                highram_اندازه = BUILD_MAX_HIGHTABLE;
            } وإلا إذا (new_min <= e && new_min >= s) {
                highram_پیل = e = new_min;
                highram_اندازه = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_جمع_کړه(&Zone_Tmp_لوړ, s, e);
    }

    // Populate regions
    alloc_جمع_کړه(&Zone_Tmp_ټیټ, BUILD_ډېری_ADDR, BUILD_EBDA_MINIMUM);
    إذا (highram_پیل) {
        alloc_جمع_کړه(&Zone_لوړ, highram_پیل, highram_پیل + highram_اندازه);
        e820_جمع_کړه(highram_پیل, highram_اندازه, E820_RESERVED);
    }
}

فراغ
malloc_csm_preinit(u32 ټیټ_pmm, u32 ټیټ_pmm_اندازه, u32 hi_pmm, u32 hi_pmm_اندازه)
{
    ASSERT32FLAT();

    إذا (hi_pmm_اندازه > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_پای_ټکی = hi_pmm + hi_pmm_اندازه;
        alloc_جمع_کړه(&Zone_Tmp_لوړ, hi_pmm, hi_pmm_پای_ټکی - BUILD_MIN_HIGHTABLE);
        alloc_جمع_کړه(&Zone_لوړ, hi_pmm_پای_ټکی - BUILD_MIN_HIGHTABLE, hi_pmm_پای_ټکی);
    } وإلا {
        alloc_جمع_کړه(&Zone_Tmp_لوړ, hi_pmm, hi_pmm + hi_pmm_اندازه);
    }
    alloc_جمع_کړه(&Zone_Tmp_ټیټ, ټیټ_pmm, ټیټ_pmm + ټیټ_pmm_اندازه);
}

u32 Legacy_Ram_اندازه VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ساكن فراغ
calc_Ram_اندازه(فراغ)
{
    u32 rs = 0;
    صحيح i;
    لكل (i=e820_شمېر-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 پای_ټکی = en->پیل + en->اندازه;
        u32 type = en->type;
        إذا (پای_ټکی <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = پای_ټکی;
            اكسر;
        }
    }
    Legacy_Ram_اندازه = rs >= 1024*1024 ? rs : 1024*1024;
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
        لكل (i=0; i<ARRAY_اندازه(Zones); i++) {
            بنية zone_s *zone = Zones[i];
            إذا (zone->سر.لومړی)
                zone->سر.لومړی->pprev = &zone->سر.لومړی;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_پیل), VSYMBOL(varlow_پیل)
            , نښه(varlow_پای_ټکی) - نښه(varlow_پیل));
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_جمع_کړه(&Zone_ټیټ, نښه(zonelow_بنسټ) + OPROM_سرلیک_RESERVE
                  , نښه(final_varlow_پیل));
        Rom_بنسټ = alloc_پیدا_کړه_ټیټ_ترین(&Zone_ټیټ);
    } وإلا {
        alloc_جمع_کړه(&Zone_ټیټ, ALIGN_ښکته(نښه(final_varlow_پیل), 1024)
                  , نښه(final_varlow_پیل));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_پیل), 0
           , نښه(zonefseg_پای_ټکی) - نښه(zonefseg_پیل));
    alloc_جمع_کړه(&ZoneFSeg, نښه(zonefseg_پیل), نښه(zonefseg_پای_ټکی));

    calc_Ram_اندازه();
}

فراغ
malloc_prepboot(فراغ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 بنسټ = rom_get_max();
    memset((فراغ*)Rom_پای_ټکی, 0, بنسټ-Rom_پای_ټکی);
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        بنية rom_سرلیک *dummyrom = (فراغ*)بنسټ;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        صحيح اندازه = (BUILD_BIOS_ADDR - بنسټ) / 512;
        dummyrom->اندازه = (اندازه > 255) ? 255 : اندازه;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_اندازه_kb)*1024;
    e820_جمع_کړه(endlow, BUILD_LOWRAM_پای_ټکی-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    بنية allocinfo_s *info = alloc_پیدا_کړه_ټیټ_ترین(&ZoneFSeg);
    u32 اندازه = info->range_پای_ټکی - info->range_پیل;
    memset(memremap(info->range_پیل, اندازه), 0, اندازه);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_پای_ټکی, بنسټ, info->range_پیل, info->range_پای_ټکی);

    // Give back unused high ram.
    info = alloc_پیدا_کړه_ټیټ_ترین(&Zone_لوړ);
    إذا (info) {
        u32 giveback = ALIGN_ښکته(info->range_پای_ټکی-info->range_پیل, پاڼه_اندازه);
        e820_جمع_کړه(info->range_پیل, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_اندازه();
}
