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
    u32 range_شروع, range_سرا, alloc_ناپ;
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
alloc_new(بنية zone_s *zone, u32 ناپ, u32 align, بنية allocinfo_s *fill)
{
    بنية allocinfo_s *info;
    hlist_for_each_اندراج(info, &zone->سر, گرہ) {
        u32 alloc_سرا = info->range_شروع + info->alloc_ناپ;
        u32 range_سرا = info->range_سرا;
        u32 new_range_سرا = ALIGN_تھلے(range_سرا - ناپ, align);
        إذا (new_range_سرا >= alloc_سرا && new_range_سرا <= range_سرا) {
            // Found space - now reserve it.
            fill->range_شروع = new_range_سرا;
            fill->range_سرا = range_سرا;
            fill->alloc_ناپ = ناپ;

            info->range_سرا = new_range_سرا;
            hlist_جوڑو_before(&fill->گرہ, &info->گرہ);
            أرجع new_range_سرا;
        }
    }
    أرجع 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ساكن بنية allocdetail_s *
alloc_new_detail(بنية allocdetail_s *عارضی)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_بلند, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &عارضی->detailinfo);
    إذا (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_کم, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &عارضی->detailinfo);
        إذا (!detail_addr) {
            warn_noalloc();
            أرجع عدم;
        }
    }
    بنية allocdetail_s *detail = memremap(detail_addr, حجم(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    حافظہ_نقل(detail, عارضی, حجم(*detail));
    hlist_replace(&عارضی->detailinfo.گرہ, &detail->detailinfo.گرہ);
    hlist_replace(&عارضی->datainfo.گرہ, &detail->datainfo.گرہ);
    أرجع detail;
}

// Add new memory to a zone
ساكن فراغ
alloc_جوڑو(بنية zone_s *zone, u32 شروع, u32 سرا)
{
    // Find position to add space
    بنية allocinfo_s *info;
    بنية hlist_گرہ **pprev;
    hlist_for_each_اندراج_pprev(info, pprev, &zone->سر, گرہ) {
        إذا (info->range_شروع < شروع)
            اكسر;
    }

    // Add space using temporary allocation info.
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_شروع = شروع;
    tempdetail.datainfo.range_سرا = سرا;
    tempdetail.datainfo.alloc_ناپ = 0;
    hlist_جوڑو(&tempdetail.datainfo.گرہ, pprev);

    // Allocate final allocation info.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail)
        hlist_del(&tempdetail.datainfo.گرہ);
}

// Release space allocated with alloc_new()
ساكن فراغ
alloc_free(بنية allocinfo_s *info)
{
    بنية allocinfo_s *اگلا = container_دا_or_null(
        info->گرہ.اگلا, بنية allocinfo_s, گرہ);
    إذا (اگلا && اگلا->range_سرا == info->range_شروع)
        اگلا->range_سرا = info->range_سرا;
    hlist_del(&info->گرہ);
}

// Search all zones for an allocation obtained from alloc_new()
ساكن بنية allocinfo_s *
alloc_لبھو(u32 انکڑے)
{
    صحيح i;
    لكل (i=0; i<ARRAY_ناپ(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_اندراج(info, &Zones[i]->سر, گرہ) {
            إذا (info->range_شروع == انکڑے)
                أرجع info;
        }
    }
    أرجع عدم;
}

// Find the lowest memory range added by alloc_add()
ساكن بنية allocinfo_s *
alloc_لبھو_سبھ_توں_نیواں(بنية zone_s *zone)
{
    بنية allocinfo_s *info, *آخری = عدم;
    hlist_for_each_اندراج(info, &zone->سر, گرہ) {
        آخری = info;
    }
    أرجع آخری;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ساكن صحيح
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_ناپ)
{
    u32 lowram = GET_BDA(mem_ناپ_kb) * 1024;
    إذا (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        أرجع -1;

    // Do copy
    memmove((فراغ*)newebda, (فراغ*)oldebda, ebda_ناپ * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    سیٹ_BDA(mem_ناپ_kb, newebda / 1024);
    سیٹ_BDA(ebda_seg, FLATPTR_منزل_SEG(newebda));
    أرجع 0;
}

// Support expanding the ZoneLow dynamically.
ساكن u32
zonelow_expand(u32 ناپ, u32 align, بنية allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    إذا (unlikely(wait_preempt())) {
        u32 انکڑے = alloc_new(&Zone_کم, ناپ, align, fill);
        إذا (انکڑے)
            أرجع انکڑے;
    }

    بنية allocinfo_s *info = alloc_لبھو_سبھ_توں_نیواں(&Zone_کم);
    إذا (!info)
        أرجع 0;
    u32 oldpos = info->range_سرا;
    u32 newpos = ALIGN_تھلے(oldpos - ناپ, align);
    u32 bottom = info->range_شروع + info->alloc_ناپ;
    إذا (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        أرجع alloc_new(&Zone_کم, ناپ, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_ناپ = GET_EBDA(ebda_seg, ناپ);
    u32 ebda_سرا = ebda_pos + ebda_ناپ * 1024;
    إذا (ebda_سرا != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_تھلے(ebda_سرا - ناپ, align);
    u32 newbottom = ALIGN_تھلے(newpos, 1024);
    u32 newebda = ALIGN_تھلے(newbottom - ebda_ناپ * 1024, 1024);
    إذا (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        أرجع 0;

    // Move ebda
    صحيح ret = relocate_ebda(newebda, ebda_pos, ebda_ناپ);
    إذا (ret)
        أرجع 0;

    // Update zone
    إذا (ebda_سرا == bottom)
        info->range_شروع = newbottom;
    وإلا
        alloc_جوڑو(&Zone_کم, newbottom, ebda_سرا);

    أرجع alloc_new(&Zone_کم, ناپ, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(بنية zone_s *zone, u32 ناپ, u32 align)
{
    ASSERT32FLAT();
    إذا (!ناپ)
        أرجع 0;

    // Find and reserve space for main allocation
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 انکڑے = alloc_new(zone, ناپ, align, &tempdetail.datainfo);
    إذا (!CONFIG_MALLOC_UPPERMEMORY && !انکڑے && zone == &Zone_کم)
        انکڑے = zonelow_expand(ناپ, align, &tempdetail.datainfo);
    إذا (!انکڑے)
        أرجع 0;

    // Find and reserve space for bookkeeping.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail) {
        alloc_free(&tempdetail.datainfo);
        أرجع 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, ناپ, align, انکڑے, detail);

    أرجع انکڑے;
}

// Allocate virtual memory from the given zone
فراغ * __malloc
_malloc(بنية zone_s *zone, u32 ناپ, u32 align)
{
    أرجع memremap(malloc_palloc(zone, ناپ, align), ناپ);
}

// Free a data block allocated with phys_alloc
صحيح
malloc_pfree(u32 انکڑے)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_لبھو(انکڑے);
    إذا (!info || انکڑے == virt_منزل_phys(info) || !info->alloc_ناپ)
        أرجع -1;
    بنية allocdetail_s *detail = container_دا(
        info, بنية allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", انکڑے, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    أرجع 0;
}

فراغ
حرر(فراغ *انکڑے)
{
    إذا (!انکڑے)
        أرجع;
    صحيح ret = malloc_pfree(virt_منزل_phys(انکڑے));
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
    hlist_for_each_اندراج(info, &zone->سر, گرہ) {
        u32 space = info->range_سرا - info->range_شروع - info->alloc_ناپ;
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
malloc_sethandle(u32 انکڑے, u32 handle)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_لبھو(انکڑے);
    إذا (!info || انکڑے == virt_منزل_phys(info) || !info->alloc_ناپ)
        أرجع;
    بنية allocdetail_s *detail = container_دا(
        info, بنية allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    صحيح i;
    لكل (i=0; i<ARRAY_ناپ(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_اندراج(info, &Zones[i]->سر, گرہ) {
            إذا (info->range_شروع != virt_منزل_phys(info))
                استمر;
            بنية allocdetail_s *detail = container_دا(
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

ساكن u32 Rom_سرا = BUILD_ROM_شروع;
ساكن بنية allocinfo_s *Rom_ادھار;

#تعریف_کرو OPROM_سرخی_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(فراغ)
{
    إذا (CONFIG_MALLOC_UPPERMEMORY)
        أرجع ALIGN_تھلے(Rom_ادھار->range_سرا - OPROM_سرخی_RESERVE
                          , OPTION_ROM_ALIGN);
    أرجع علامت(final_readonly_شروع);
}

// Return the end of the last deployed option rom.
u32
rom_get_آخری(فراغ)
{
    أرجع Rom_سرا;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
بنية rom_سرخی *
rom_reserve(u32 ناپ)
{
    u32 newend = ALIGN(Rom_سرا + ناپ, OPTION_ROM_ALIGN);
    إذا (newend > rom_get_max())
        أرجع عدم;
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        إذا (newend < علامت(zonelow_ادھار))
            newend = علامت(zonelow_ادھار);
        Rom_ادھار->range_شروع = newend + OPROM_سرخی_RESERVE;
    }
    أرجع (فراغ*)Rom_سرا;
}

// Confirm space as in use by an optionrom.
صحيح
rom_confirm(u32 ناپ)
{
    فراغ *new = rom_reserve(ناپ);
    إذا (!new) {
        warn_noalloc();
        أرجع -1;
    }
    Rom_سرا = ALIGN(Rom_سرا + ناپ, OPTION_ROM_ALIGN);
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
    e820_remove(BUILD_LOWRAM_سرا, BUILD_BIOS_ADDR-BUILD_LOWRAM_سرا);

    // Mark known areas as reserved.
    e820_جوڑو(BUILD_BIOS_ADDR, BUILD_BIOS_ناپ, E820_RESERVED);

    // Populate temp high ram
    u32 highram_شروع = 0;
    u32 highram_ناپ = 0;
    صحيح i;
    لكل (i=e820_گنتی-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 سرا = en->شروع + en->ناپ;
        إذا (سرا < 1024*1024)
            اكسر;
        إذا (en->type != E820_RAM || سرا > 0xffffffff)
            استمر;
        u32 s = en->شروع, e = سرا;
        إذا (!highram_شروع) {
            u32 new_max = ALIGN_تھلے(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_تھلے(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            إذا (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_شروع = e = new_max;
                highram_ناپ = BUILD_MAX_HIGHTABLE;
            } وإلا إذا (new_min <= e && new_min >= s) {
                highram_شروع = e = new_min;
                highram_ناپ = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_جوڑو(&Zone_Tmp_بلند, s, e);
    }

    // Populate regions
    alloc_جوڑو(&Zone_Tmp_کم, BUILD_ڈھیر_ADDR, BUILD_EBDA_MINIMUM);
    إذا (highram_شروع) {
        alloc_جوڑو(&Zone_بلند, highram_شروع, highram_شروع + highram_ناپ);
        e820_جوڑو(highram_شروع, highram_ناپ, E820_RESERVED);
    }
}

فراغ
malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_ناپ, u32 hi_pmm, u32 hi_pmm_ناپ)
{
    ASSERT32FLAT();

    إذا (hi_pmm_ناپ > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_سرا = hi_pmm + hi_pmm_ناپ;
        alloc_جوڑو(&Zone_Tmp_بلند, hi_pmm, hi_pmm_سرا - BUILD_MIN_HIGHTABLE);
        alloc_جوڑو(&Zone_بلند, hi_pmm_سرا - BUILD_MIN_HIGHTABLE, hi_pmm_سرا);
    } وإلا {
        alloc_جوڑو(&Zone_Tmp_بلند, hi_pmm, hi_pmm + hi_pmm_ناپ);
    }
    alloc_جوڑو(&Zone_Tmp_کم, کم_pmm, کم_pmm + کم_pmm_ناپ);
}

u32 Legacy_Ram_ناپ VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ساكن فراغ
calc_Ram_ناپ(فراغ)
{
    u32 rs = 0;
    صحيح i;
    لكل (i=e820_گنتی-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 سرا = en->شروع + en->ناپ;
        u32 type = en->type;
        إذا (سرا <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = سرا;
            اكسر;
        }
    }
    Legacy_Ram_ناپ = rs >= 1024*1024 ? rs : 1024*1024;
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
        لكل (i=0; i<ARRAY_ناپ(Zones); i++) {
            بنية zone_s *zone = Zones[i];
            إذا (zone->سر.پہلا)
                zone->سر.پہلا->pprev = &zone->سر.پہلا;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_شروع), VSYMBOL(varlow_شروع)
            , علامت(varlow_سرا) - علامت(varlow_شروع));
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_جوڑو(&Zone_کم, علامت(zonelow_ادھار) + OPROM_سرخی_RESERVE
                  , علامت(final_varlow_شروع));
        Rom_ادھار = alloc_لبھو_سبھ_توں_نیواں(&Zone_کم);
    } وإلا {
        alloc_جوڑو(&Zone_کم, ALIGN_تھلے(علامت(final_varlow_شروع), 1024)
                  , علامت(final_varlow_شروع));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_شروع), 0
           , علامت(zonefseg_سرا) - علامت(zonefseg_شروع));
    alloc_جوڑو(&ZoneFSeg, علامت(zonefseg_شروع), علامت(zonefseg_سرا));

    calc_Ram_ناپ();
}

فراغ
malloc_prepboot(فراغ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 ادھار = rom_get_max();
    memset((فراغ*)Rom_سرا, 0, ادھار-Rom_سرا);
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        بنية rom_سرخی *dummyrom = (فراغ*)ادھار;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        صحيح ناپ = (BUILD_BIOS_ADDR - ادھار) / 512;
        dummyrom->ناپ = (ناپ > 255) ? 255 : ناپ;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_ناپ_kb)*1024;
    e820_جوڑو(endlow, BUILD_LOWRAM_سرا-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    بنية allocinfo_s *info = alloc_لبھو_سبھ_توں_نیواں(&ZoneFSeg);
    u32 ناپ = info->range_سرا - info->range_شروع;
    memset(memremap(info->range_شروع, ناپ), 0, ناپ);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_سرا, ادھار, info->range_شروع, info->range_سرا);

    // Give back unused high ram.
    info = alloc_لبھو_سبھ_توں_نیواں(&Zone_بلند);
    إذا (info) {
        u32 giveback = ALIGN_تھلے(info->range_سرا-info->range_شروع, ورقہ_ناپ);
        e820_جوڑو(info->range_شروع, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_ناپ();
}
