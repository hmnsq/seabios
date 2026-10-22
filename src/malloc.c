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
    بنية hlist_گره گره;
    u32 range_شروع, range_انتها, alloc_اندازه;
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

بنية zone_s Zone_کم VARVERIFY32INIT, Zone_بالا VARVERIFY32INIT;
بنية zone_s ZoneFSeg VARVERIFY32INIT;
بنية zone_s Zone_Tmp_کم VARVERIFY32INIT, Zone_Tmp_بالا VARVERIFY32INIT;

ساكن بنية zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_کم, &Zone_کم, &ZoneFSeg, &Zone_Tmp_بالا, &Zone_بالا
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ساكن u32
alloc_new(بنية zone_s *zone, u32 اندازه, u32 align, بنية allocinfo_s *fill)
{
    بنية allocinfo_s *info;
    hlist_for_each_مدخل(info, &zone->سر, گره) {
        u32 alloc_انتها = info->range_شروع + info->alloc_اندازه;
        u32 range_انتها = info->range_انتها;
        u32 new_range_انتها = ALIGN_پایین(range_انتها - اندازه, align);
        إذا (new_range_انتها >= alloc_انتها && new_range_انتها <= range_انتها) {
            // Found space - now reserve it.
            fill->range_شروع = new_range_انتها;
            fill->range_انتها = range_انتها;
            fill->alloc_اندازه = اندازه;

            info->range_انتها = new_range_انتها;
            hlist_جمع_کن_before(&fill->گره, &info->گره);
            أرجع new_range_انتها;
        }
    }
    أرجع 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ساكن بنية allocdetail_s *
alloc_new_detail(بنية allocdetail_s *موقت)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_بالا, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &موقت->detailinfo);
    إذا (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_کم, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &موقت->detailinfo);
        إذا (!detail_addr) {
            warn_noalloc();
            أرجع عدم;
        }
    }
    بنية allocdetail_s *detail = memremap(detail_addr, حجم(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    کپی_حافظه(detail, موقت, حجم(*detail));
    hlist_replace(&موقت->detailinfo.گره, &detail->detailinfo.گره);
    hlist_replace(&موقت->datainfo.گره, &detail->datainfo.گره);
    أرجع detail;
}

// Add new memory to a zone
ساكن فراغ
alloc_جمع_کن(بنية zone_s *zone, u32 شروع, u32 انتها)
{
    // Find position to add space
    بنية allocinfo_s *info;
    بنية hlist_گره **pprev;
    hlist_for_each_مدخل_pprev(info, pprev, &zone->سر, گره) {
        إذا (info->range_شروع < شروع)
            اكسر;
    }

    // Add space using temporary allocation info.
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_شروع = شروع;
    tempdetail.datainfo.range_انتها = انتها;
    tempdetail.datainfo.alloc_اندازه = 0;
    hlist_جمع_کن(&tempdetail.datainfo.گره, pprev);

    // Allocate final allocation info.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail)
        hlist_del(&tempdetail.datainfo.گره);
}

// Release space allocated with alloc_new()
ساكن فراغ
alloc_free(بنية allocinfo_s *info)
{
    بنية allocinfo_s *بعدی = container_از_or_null(
        info->گره.بعدی, بنية allocinfo_s, گره);
    إذا (بعدی && بعدی->range_انتها == info->range_شروع)
        بعدی->range_انتها = info->range_انتها;
    hlist_del(&info->گره);
}

// Search all zones for an allocation obtained from alloc_new()
ساكن بنية allocinfo_s *
alloc_پیدا_کن(u32 داده)
{
    صحيح i;
    لكل (i=0; i<ARRAY_اندازه(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_مدخل(info, &Zones[i]->سر, گره) {
            إذا (info->range_شروع == داده)
                أرجع info;
        }
    }
    أرجع عدم;
}

// Find the lowest memory range added by alloc_add()
ساكن بنية allocinfo_s *
alloc_پیدا_کن_پایینترین(بنية zone_s *zone)
{
    بنية allocinfo_s *info, *آخرین = عدم;
    hlist_for_each_مدخل(info, &zone->سر, گره) {
        آخرین = info;
    }
    أرجع آخرین;
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
    مجموعه_BDA(mem_اندازه_kb, newebda / 1024);
    مجموعه_BDA(ebda_seg, FLATPTR_مقصد_SEG(newebda));
    أرجع 0;
}

// Support expanding the ZoneLow dynamically.
ساكن u32
zonelow_expand(u32 اندازه, u32 align, بنية allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    إذا (unlikely(wait_preempt())) {
        u32 داده = alloc_new(&Zone_کم, اندازه, align, fill);
        إذا (داده)
            أرجع داده;
    }

    بنية allocinfo_s *info = alloc_پیدا_کن_پایینترین(&Zone_کم);
    إذا (!info)
        أرجع 0;
    u32 oldpos = info->range_انتها;
    u32 newpos = ALIGN_پایین(oldpos - اندازه, align);
    u32 bottom = info->range_شروع + info->alloc_اندازه;
    إذا (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        أرجع alloc_new(&Zone_کم, اندازه, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_اندازه = GET_EBDA(ebda_seg, اندازه);
    u32 ebda_انتها = ebda_pos + ebda_اندازه * 1024;
    إذا (ebda_انتها != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_پایین(ebda_انتها - اندازه, align);
    u32 newbottom = ALIGN_پایین(newpos, 1024);
    u32 newebda = ALIGN_پایین(newbottom - ebda_اندازه * 1024, 1024);
    إذا (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        أرجع 0;

    // Move ebda
    صحيح ret = relocate_ebda(newebda, ebda_pos, ebda_اندازه);
    إذا (ret)
        أرجع 0;

    // Update zone
    إذا (ebda_انتها == bottom)
        info->range_شروع = newbottom;
    وإلا
        alloc_جمع_کن(&Zone_کم, newbottom, ebda_انتها);

    أرجع alloc_new(&Zone_کم, اندازه, align, fill);
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
    u32 داده = alloc_new(zone, اندازه, align, &tempdetail.datainfo);
    إذا (!CONFIG_MALLOC_UPPERMEMORY && !داده && zone == &Zone_کم)
        داده = zonelow_expand(اندازه, align, &tempdetail.datainfo);
    إذا (!داده)
        أرجع 0;

    // Find and reserve space for bookkeeping.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail) {
        alloc_free(&tempdetail.datainfo);
        أرجع 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, اندازه, align, داده, detail);

    أرجع داده;
}

// Allocate virtual memory from the given zone
فراغ * __malloc
_malloc(بنية zone_s *zone, u32 اندازه, u32 align)
{
    أرجع memremap(malloc_palloc(zone, اندازه, align), اندازه);
}

// Free a data block allocated with phys_alloc
صحيح
malloc_pfree(u32 داده)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_پیدا_کن(داده);
    إذا (!info || داده == virt_مقصد_phys(info) || !info->alloc_اندازه)
        أرجع -1;
    بنية allocdetail_s *detail = container_از(
        info, بنية allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", داده, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    أرجع 0;
}

فراغ
حرر(فراغ *داده)
{
    إذا (!داده)
        أرجع;
    صحيح ret = malloc_pfree(virt_مقصد_phys(داده));
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
    hlist_for_each_مدخل(info, &zone->سر, گره) {
        u32 space = info->range_انتها - info->range_شروع - info->alloc_اندازه;
        إذا (space > maxspace)
            maxspace = space;
    }

    إذا (zone != &Zone_Tmp_بالا && zone != &Zone_Tmp_کم)
        أرجع maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(حجم(بنية allocdetail_s), MALLOC_MIN_ALIGN);
    إذا (maxspace <= reserve)
        أرجع 0;
    أرجع maxspace - reserve;
}

// Set a handle associated with an allocation.
فراغ
malloc_sethandle(u32 داده, u32 handle)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_پیدا_کن(داده);
    إذا (!info || داده == virt_مقصد_phys(info) || !info->alloc_اندازه)
        أرجع;
    بنية allocdetail_s *detail = container_از(
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
        hlist_for_each_مدخل(info, &Zones[i]->سر, گره) {
            إذا (info->range_شروع != virt_مقصد_phys(info))
                استمر;
            بنية allocdetail_s *detail = container_از(
                info, بنية allocdetail_s, detailinfo);
            إذا (detail->handle == handle)
                أرجع detail->datainfo.range_شروع;
        }
    }
    أرجع 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ساكن u32 Rom_انتها = BUILD_ROM_شروع;
ساكن بنية allocinfo_s *Rom_پایه;

#تعریف_کن OPROM_سرآیند_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(فراغ)
{
    إذا (CONFIG_MALLOC_UPPERMEMORY)
        أرجع ALIGN_پایین(Rom_پایه->range_انتها - OPROM_سرآیند_RESERVE
                          , OPTION_ROM_ALIGN);
    أرجع نماد(final_readonly_شروع);
}

// Return the end of the last deployed option rom.
u32
rom_get_آخرین(فراغ)
{
    أرجع Rom_انتها;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
بنية rom_سرآیند *
rom_reserve(u32 اندازه)
{
    u32 newend = ALIGN(Rom_انتها + اندازه, OPTION_ROM_ALIGN);
    إذا (newend > rom_get_max())
        أرجع عدم;
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        إذا (newend < نماد(zonelow_پایه))
            newend = نماد(zonelow_پایه);
        Rom_پایه->range_شروع = newend + OPROM_سرآیند_RESERVE;
    }
    أرجع (فراغ*)Rom_انتها;
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
    Rom_انتها = ALIGN(Rom_انتها + اندازه, OPTION_ROM_ALIGN);
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
    e820_remove(BUILD_LOWRAM_انتها, BUILD_BIOS_ADDR-BUILD_LOWRAM_انتها);

    // Mark known areas as reserved.
    e820_جمع_کن(BUILD_BIOS_ADDR, BUILD_BIOS_اندازه, E820_RESERVED);

    // Populate temp high ram
    u32 highram_شروع = 0;
    u32 highram_اندازه = 0;
    صحيح i;
    لكل (i=e820_شمار-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 انتها = en->شروع + en->اندازه;
        إذا (انتها < 1024*1024)
            اكسر;
        إذا (en->type != E820_RAM || انتها > 0xffffffff)
            استمر;
        u32 s = en->شروع, e = انتها;
        إذا (!highram_شروع) {
            u32 new_max = ALIGN_پایین(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_پایین(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            إذا (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_شروع = e = new_max;
                highram_اندازه = BUILD_MAX_HIGHTABLE;
            } وإلا إذا (new_min <= e && new_min >= s) {
                highram_شروع = e = new_min;
                highram_اندازه = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_جمع_کن(&Zone_Tmp_بالا, s, e);
    }

    // Populate regions
    alloc_جمع_کن(&Zone_Tmp_کم, BUILD_پشته_ADDR, BUILD_EBDA_MINIMUM);
    إذا (highram_شروع) {
        alloc_جمع_کن(&Zone_بالا, highram_شروع, highram_شروع + highram_اندازه);
        e820_جمع_کن(highram_شروع, highram_اندازه, E820_RESERVED);
    }
}

فراغ
malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_اندازه, u32 hi_pmm, u32 hi_pmm_اندازه)
{
    ASSERT32FLAT();

    إذا (hi_pmm_اندازه > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_انتها = hi_pmm + hi_pmm_اندازه;
        alloc_جمع_کن(&Zone_Tmp_بالا, hi_pmm, hi_pmm_انتها - BUILD_MIN_HIGHTABLE);
        alloc_جمع_کن(&Zone_بالا, hi_pmm_انتها - BUILD_MIN_HIGHTABLE, hi_pmm_انتها);
    } وإلا {
        alloc_جمع_کن(&Zone_Tmp_بالا, hi_pmm, hi_pmm + hi_pmm_اندازه);
    }
    alloc_جمع_کن(&Zone_Tmp_کم, کم_pmm, کم_pmm + کم_pmm_اندازه);
}

u32 Legacy_Ram_اندازه VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ساكن فراغ
calc_Ram_اندازه(فراغ)
{
    u32 rs = 0;
    صحيح i;
    لكل (i=e820_شمار-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 انتها = en->شروع + en->اندازه;
        u32 type = en->type;
        إذا (انتها <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = انتها;
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
            إذا (zone->سر.اولی)
                zone->سر.اولی->pprev = &zone->سر.اولی;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_شروع), VSYMBOL(varlow_شروع)
            , نماد(varlow_انتها) - نماد(varlow_شروع));
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_جمع_کن(&Zone_کم, نماد(zonelow_پایه) + OPROM_سرآیند_RESERVE
                  , نماد(final_varlow_شروع));
        Rom_پایه = alloc_پیدا_کن_پایینترین(&Zone_کم);
    } وإلا {
        alloc_جمع_کن(&Zone_کم, ALIGN_پایین(نماد(final_varlow_شروع), 1024)
                  , نماد(final_varlow_شروع));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_شروع), 0
           , نماد(zonefseg_انتها) - نماد(zonefseg_شروع));
    alloc_جمع_کن(&ZoneFSeg, نماد(zonefseg_شروع), نماد(zonefseg_انتها));

    calc_Ram_اندازه();
}

فراغ
malloc_prepboot(فراغ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 پایه = rom_get_max();
    memset((فراغ*)Rom_انتها, 0, پایه-Rom_انتها);
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        بنية rom_سرآیند *dummyrom = (فراغ*)پایه;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        صحيح اندازه = (BUILD_BIOS_ADDR - پایه) / 512;
        dummyrom->اندازه = (اندازه > 255) ? 255 : اندازه;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_اندازه_kb)*1024;
    e820_جمع_کن(endlow, BUILD_LOWRAM_انتها-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    بنية allocinfo_s *info = alloc_پیدا_کن_پایینترین(&ZoneFSeg);
    u32 اندازه = info->range_انتها - info->range_شروع;
    memset(memremap(info->range_شروع, اندازه), 0, اندازه);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_انتها, پایه, info->range_شروع, info->range_انتها);

    // Give back unused high ram.
    info = alloc_پیدا_کن_پایینترین(&Zone_بالا);
    إذا (info) {
        u32 giveback = ALIGN_پایین(info->range_انتها-info->range_شروع, صفحه_اندازه);
        e820_جمع_کن(info->range_شروع, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_اندازه();
}
