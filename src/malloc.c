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
    بنية hlist_ڳنڍ ڳنڍ;
    u32 range_شروعات, range_پڇاڙي, alloc_سائيز;
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

بنية zone_s Zone_هيٺيون VARVERIFY32INIT, Zone_مٿاهون VARVERIFY32INIT;
بنية zone_s ZoneFSeg VARVERIFY32INIT;
بنية zone_s Zone_Tmp_هيٺيون VARVERIFY32INIT, Zone_Tmp_مٿاهون VARVERIFY32INIT;

ساكن بنية zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_هيٺيون, &Zone_هيٺيون, &ZoneFSeg, &Zone_Tmp_مٿاهون, &Zone_مٿاهون
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ساكن u32
alloc_new(بنية zone_s *zone, u32 سائيز, u32 align, بنية allocinfo_s *fill)
{
    بنية allocinfo_s *info;
    hlist_for_each_داخلا(info, &zone->سر, ڳنڍ) {
        u32 alloc_پڇاڙي = info->range_شروعات + info->alloc_سائيز;
        u32 range_پڇاڙي = info->range_پڇاڙي;
        u32 new_range_پڇاڙي = ALIGN_هيٺ_ڏانهن(range_پڇاڙي - سائيز, align);
        إذا (new_range_پڇاڙي >= alloc_پڇاڙي && new_range_پڇاڙي <= range_پڇاڙي) {
            // Found space - now reserve it.
            fill->range_شروعات = new_range_پڇاڙي;
            fill->range_پڇاڙي = range_پڇاڙي;
            fill->alloc_سائيز = سائيز;

            info->range_پڇاڙي = new_range_پڇاڙي;
            hlist_جوڙيو_before(&fill->ڳنڍ, &info->ڳنڍ);
            أرجع new_range_پڇاڙي;
        }
    }
    أرجع 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ساكن بنية allocdetail_s *
alloc_new_detail(بنية allocdetail_s *عارضي)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_مٿاهون, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &عارضي->detailinfo);
    إذا (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_هيٺيون, حجم(بنية allocdetail_s)
                                , MALLOC_MIN_ALIGN, &عارضي->detailinfo);
        إذا (!detail_addr) {
            warn_noalloc();
            أرجع عدم;
        }
    }
    بنية allocdetail_s *detail = memremap(detail_addr, حجم(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    يادگيري_نقل(detail, عارضي, حجم(*detail));
    hlist_replace(&عارضي->detailinfo.ڳنڍ, &detail->detailinfo.ڳنڍ);
    hlist_replace(&عارضي->datainfo.ڳنڍ, &detail->datainfo.ڳنڍ);
    أرجع detail;
}

// Add new memory to a zone
ساكن فراغ
alloc_جوڙيو(بنية zone_s *zone, u32 شروعات, u32 پڇاڙي)
{
    // Find position to add space
    بنية allocinfo_s *info;
    بنية hlist_ڳنڍ **pprev;
    hlist_for_each_داخلا_pprev(info, pprev, &zone->سر, ڳنڍ) {
        إذا (info->range_شروعات < شروعات)
            اكسر;
    }

    // Add space using temporary allocation info.
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_شروعات = شروعات;
    tempdetail.datainfo.range_پڇاڙي = پڇاڙي;
    tempdetail.datainfo.alloc_سائيز = 0;
    hlist_جوڙيو(&tempdetail.datainfo.ڳنڍ, pprev);

    // Allocate final allocation info.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail)
        hlist_del(&tempdetail.datainfo.ڳنڍ);
}

// Release space allocated with alloc_new()
ساكن فراغ
alloc_free(بنية allocinfo_s *info)
{
    بنية allocinfo_s *ايندڙ = container_جو_or_null(
        info->ڳنڍ.ايندڙ, بنية allocinfo_s, ڳنڍ);
    إذا (ايندڙ && ايندڙ->range_پڇاڙي == info->range_شروعات)
        ايندڙ->range_پڇاڙي = info->range_پڇاڙي;
    hlist_del(&info->ڳنڍ);
}

// Search all zones for an allocation obtained from alloc_new()
ساكن بنية allocinfo_s *
alloc_ڳوليو(u32 معلومات)
{
    صحيح i;
    لكل (i=0; i<ARRAY_سائيز(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_داخلا(info, &Zones[i]->سر, ڳنڍ) {
            إذا (info->range_شروعات == معلومات)
                أرجع info;
        }
    }
    أرجع عدم;
}

// Find the lowest memory range added by alloc_add()
ساكن بنية allocinfo_s *
alloc_ڳوليو_سڀ_کان_هيٺ(بنية zone_s *zone)
{
    بنية allocinfo_s *info, *آخري = عدم;
    hlist_for_each_داخلا(info, &zone->سر, ڳنڍ) {
        آخري = info;
    }
    أرجع آخري;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ساكن صحيح
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_سائيز)
{
    u32 lowram = GET_BDA(mem_سائيز_kb) * 1024;
    إذا (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        أرجع -1;

    // Do copy
    memmove((فراغ*)newebda, (فراغ*)oldebda, ebda_سائيز * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    سيٽ_BDA(mem_سائيز_kb, newebda / 1024);
    سيٽ_BDA(ebda_seg, FLATPTR_منزل_SEG(newebda));
    أرجع 0;
}

// Support expanding the ZoneLow dynamically.
ساكن u32
zonelow_expand(u32 سائيز, u32 align, بنية allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    إذا (unlikely(wait_preempt())) {
        u32 معلومات = alloc_new(&Zone_هيٺيون, سائيز, align, fill);
        إذا (معلومات)
            أرجع معلومات;
    }

    بنية allocinfo_s *info = alloc_ڳوليو_سڀ_کان_هيٺ(&Zone_هيٺيون);
    إذا (!info)
        أرجع 0;
    u32 oldpos = info->range_پڇاڙي;
    u32 newpos = ALIGN_هيٺ_ڏانهن(oldpos - سائيز, align);
    u32 bottom = info->range_شروعات + info->alloc_سائيز;
    إذا (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        أرجع alloc_new(&Zone_هيٺيون, سائيز, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_سائيز = GET_EBDA(ebda_seg, سائيز);
    u32 ebda_پڇاڙي = ebda_pos + ebda_سائيز * 1024;
    إذا (ebda_پڇاڙي != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_هيٺ_ڏانهن(ebda_پڇاڙي - سائيز, align);
    u32 newbottom = ALIGN_هيٺ_ڏانهن(newpos, 1024);
    u32 newebda = ALIGN_هيٺ_ڏانهن(newbottom - ebda_سائيز * 1024, 1024);
    إذا (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        أرجع 0;

    // Move ebda
    صحيح ret = relocate_ebda(newebda, ebda_pos, ebda_سائيز);
    إذا (ret)
        أرجع 0;

    // Update zone
    إذا (ebda_پڇاڙي == bottom)
        info->range_شروعات = newbottom;
    وإلا
        alloc_جوڙيو(&Zone_هيٺيون, newbottom, ebda_پڇاڙي);

    أرجع alloc_new(&Zone_هيٺيون, سائيز, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(بنية zone_s *zone, u32 سائيز, u32 align)
{
    ASSERT32FLAT();
    إذا (!سائيز)
        أرجع 0;

    // Find and reserve space for main allocation
    بنية allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 معلومات = alloc_new(zone, سائيز, align, &tempdetail.datainfo);
    إذا (!CONFIG_MALLOC_UPPERMEMORY && !معلومات && zone == &Zone_هيٺيون)
        معلومات = zonelow_expand(سائيز, align, &tempdetail.datainfo);
    إذا (!معلومات)
        أرجع 0;

    // Find and reserve space for bookkeeping.
    بنية allocdetail_s *detail = alloc_new_detail(&tempdetail);
    إذا (!detail) {
        alloc_free(&tempdetail.datainfo);
        أرجع 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, سائيز, align, معلومات, detail);

    أرجع معلومات;
}

// Allocate virtual memory from the given zone
فراغ * __malloc
_malloc(بنية zone_s *zone, u32 سائيز, u32 align)
{
    أرجع memremap(malloc_palloc(zone, سائيز, align), سائيز);
}

// Free a data block allocated with phys_alloc
صحيح
malloc_pfree(u32 معلومات)
{
    ASSERT32FLAT();
    بنية allocinfo_s *info = alloc_ڳوليو(معلومات);
    إذا (!info || معلومات == virt_منزل_phys(info) || !info->alloc_سائيز)
        أرجع -1;
    بنية allocdetail_s *detail = container_جو(
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
    hlist_for_each_داخلا(info, &zone->سر, ڳنڍ) {
        u32 space = info->range_پڇاڙي - info->range_شروعات - info->alloc_سائيز;
        إذا (space > maxspace)
            maxspace = space;
    }

    إذا (zone != &Zone_Tmp_مٿاهون && zone != &Zone_Tmp_هيٺيون)
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
    بنية allocinfo_s *info = alloc_ڳوليو(معلومات);
    إذا (!info || معلومات == virt_منزل_phys(info) || !info->alloc_سائيز)
        أرجع;
    بنية allocdetail_s *detail = container_جو(
        info, بنية allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    صحيح i;
    لكل (i=0; i<ARRAY_سائيز(Zones); i++) {
        بنية allocinfo_s *info;
        hlist_for_each_داخلا(info, &Zones[i]->سر, ڳنڍ) {
            إذا (info->range_شروعات != virt_منزل_phys(info))
                استمر;
            بنية allocdetail_s *detail = container_جو(
                info, بنية allocdetail_s, detailinfo);
            إذا (detail->handle == handle)
                أرجع detail->datainfo.range_شروعات;
        }
    }
    أرجع 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ساكن u32 Rom_پڇاڙي = BUILD_ROM_شروعات;
ساكن بنية allocinfo_s *Rom_بنياد;

#وصف_ڪريو OPROM_سرو_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(فراغ)
{
    إذا (CONFIG_MALLOC_UPPERMEMORY)
        أرجع ALIGN_هيٺ_ڏانهن(Rom_بنياد->range_پڇاڙي - OPROM_سرو_RESERVE
                          , OPTION_ROM_ALIGN);
    أرجع نشان(final_readonly_شروعات);
}

// Return the end of the last deployed option rom.
u32
rom_get_آخري(فراغ)
{
    أرجع Rom_پڇاڙي;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
بنية rom_سرو *
rom_reserve(u32 سائيز)
{
    u32 newend = ALIGN(Rom_پڇاڙي + سائيز, OPTION_ROM_ALIGN);
    إذا (newend > rom_get_max())
        أرجع عدم;
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        إذا (newend < نشان(zonelow_بنياد))
            newend = نشان(zonelow_بنياد);
        Rom_بنياد->range_شروعات = newend + OPROM_سرو_RESERVE;
    }
    أرجع (فراغ*)Rom_پڇاڙي;
}

// Confirm space as in use by an optionrom.
صحيح
rom_confirm(u32 سائيز)
{
    فراغ *new = rom_reserve(سائيز);
    إذا (!new) {
        warn_noalloc();
        أرجع -1;
    }
    Rom_پڇاڙي = ALIGN(Rom_پڇاڙي + سائيز, OPTION_ROM_ALIGN);
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
    e820_remove(BUILD_LOWRAM_پڇاڙي, BUILD_BIOS_ADDR-BUILD_LOWRAM_پڇاڙي);

    // Mark known areas as reserved.
    e820_جوڙيو(BUILD_BIOS_ADDR, BUILD_BIOS_سائيز, E820_RESERVED);

    // Populate temp high ram
    u32 highram_شروعات = 0;
    u32 highram_سائيز = 0;
    صحيح i;
    لكل (i=e820_ڳڻپ-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 پڇاڙي = en->شروعات + en->سائيز;
        إذا (پڇاڙي < 1024*1024)
            اكسر;
        إذا (en->type != E820_RAM || پڇاڙي > 0xffffffff)
            استمر;
        u32 s = en->شروعات, e = پڇاڙي;
        إذا (!highram_شروعات) {
            u32 new_max = ALIGN_هيٺ_ڏانهن(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_هيٺ_ڏانهن(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            إذا (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_شروعات = e = new_max;
                highram_سائيز = BUILD_MAX_HIGHTABLE;
            } وإلا إذا (new_min <= e && new_min >= s) {
                highram_شروعات = e = new_min;
                highram_سائيز = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_جوڙيو(&Zone_Tmp_مٿاهون, s, e);
    }

    // Populate regions
    alloc_جوڙيو(&Zone_Tmp_هيٺيون, BUILD_ڍير_ADDR, BUILD_EBDA_MINIMUM);
    إذا (highram_شروعات) {
        alloc_جوڙيو(&Zone_مٿاهون, highram_شروعات, highram_شروعات + highram_سائيز);
        e820_جوڙيو(highram_شروعات, highram_سائيز, E820_RESERVED);
    }
}

فراغ
malloc_csm_preinit(u32 هيٺيون_pmm, u32 هيٺيون_pmm_سائيز, u32 hi_pmm, u32 hi_pmm_سائيز)
{
    ASSERT32FLAT();

    إذا (hi_pmm_سائيز > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_پڇاڙي = hi_pmm + hi_pmm_سائيز;
        alloc_جوڙيو(&Zone_Tmp_مٿاهون, hi_pmm, hi_pmm_پڇاڙي - BUILD_MIN_HIGHTABLE);
        alloc_جوڙيو(&Zone_مٿاهون, hi_pmm_پڇاڙي - BUILD_MIN_HIGHTABLE, hi_pmm_پڇاڙي);
    } وإلا {
        alloc_جوڙيو(&Zone_Tmp_مٿاهون, hi_pmm, hi_pmm + hi_pmm_سائيز);
    }
    alloc_جوڙيو(&Zone_Tmp_هيٺيون, هيٺيون_pmm, هيٺيون_pmm + هيٺيون_pmm_سائيز);
}

u32 Legacy_Ram_سائيز VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ساكن فراغ
calc_Ram_سائيز(فراغ)
{
    u32 rs = 0;
    صحيح i;
    لكل (i=e820_ڳڻپ-1; i>=0; i--) {
        بنية e820entry *en = &e820_list[i];
        u64 پڇاڙي = en->شروعات + en->سائيز;
        u32 type = en->type;
        إذا (پڇاڙي <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = پڇاڙي;
            اكسر;
        }
    }
    Legacy_Ram_سائيز = rs >= 1024*1024 ? rs : 1024*1024;
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
        لكل (i=0; i<ARRAY_سائيز(Zones); i++) {
            بنية zone_s *zone = Zones[i];
            إذا (zone->سر.پهريون)
                zone->سر.پهريون->pprev = &zone->سر.پهريون;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_شروعات), VSYMBOL(varlow_شروعات)
            , نشان(varlow_پڇاڙي) - نشان(varlow_شروعات));
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_جوڙيو(&Zone_هيٺيون, نشان(zonelow_بنياد) + OPROM_سرو_RESERVE
                  , نشان(final_varlow_شروعات));
        Rom_بنياد = alloc_ڳوليو_سڀ_کان_هيٺ(&Zone_هيٺيون);
    } وإلا {
        alloc_جوڙيو(&Zone_هيٺيون, ALIGN_هيٺ_ڏانهن(نشان(final_varlow_شروعات), 1024)
                  , نشان(final_varlow_شروعات));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_شروعات), 0
           , نشان(zonefseg_پڇاڙي) - نشان(zonefseg_شروعات));
    alloc_جوڙيو(&ZoneFSeg, نشان(zonefseg_شروعات), نشان(zonefseg_پڇاڙي));

    calc_Ram_سائيز();
}

فراغ
malloc_prepboot(فراغ)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 بنياد = rom_get_max();
    memset((فراغ*)Rom_پڇاڙي, 0, بنياد-Rom_پڇاڙي);
    إذا (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        بنية rom_سرو *dummyrom = (فراغ*)بنياد;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        صحيح سائيز = (BUILD_BIOS_ADDR - بنياد) / 512;
        dummyrom->سائيز = (سائيز > 255) ? 255 : سائيز;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_سائيز_kb)*1024;
    e820_جوڙيو(endlow, BUILD_LOWRAM_پڇاڙي-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    بنية allocinfo_s *info = alloc_ڳوليو_سڀ_کان_هيٺ(&ZoneFSeg);
    u32 سائيز = info->range_پڇاڙي - info->range_شروعات;
    memset(memremap(info->range_شروعات, سائيز), 0, سائيز);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_پڇاڙي, بنياد, info->range_شروعات, info->range_پڇاڙي);

    // Give back unused high ram.
    info = alloc_ڳوليو_سڀ_کان_هيٺ(&Zone_مٿاهون);
    إذا (info) {
        u32 giveback = ALIGN_هيٺ_ڏانهن(info->range_پڇاڙي-info->range_شروعات, صفحو_سائيز);
        e820_جوڙيو(info->range_شروعات, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_سائيز();
}
