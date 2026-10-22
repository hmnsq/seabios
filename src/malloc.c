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
ساخت allocinfo_s {
    ساخت hlist_گرہ گرہ;
    u32 range_آغاز, range_انتہا, alloc_سائز;
};

// Information on a tracked memory allocation.
ساخت allocdetail_s {
    ساخت allocinfo_s detailinfo;
    ساخت allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
ساخت zone_s {
    ساخت hlist_سر سر;
};

ساخت zone_s Zone_کم VARVERIFY32INIT, Zone_بلند VARVERIFY32INIT;
ساخت zone_s ZoneFSeg VARVERIFY32INIT;
ساخت zone_s Zone_Tmp_کم VARVERIFY32INIT, Zone_Tmp_بلند VARVERIFY32INIT;

ساکن ساخت zone_s *Zones[] VARVERIFY32INIT = {
    &Zone_Tmp_کم, &Zone_کم, &ZoneFSeg, &Zone_Tmp_بلند, &Zone_بلند
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
ساکن u32
alloc_new(ساخت zone_s *zone, u32 سائز, u32 align, ساخت allocinfo_s *fill)
{
    ساخت allocinfo_s *info;
    hlist_for_each_اندراج(info, &zone->سر, گرہ) {
        u32 alloc_انتہا = info->range_آغاز + info->alloc_سائز;
        u32 range_انتہا = info->range_انتہا;
        u32 new_range_انتہا = ALIGN_نیچے_کو(range_انتہا - سائز, align);
        اگر (new_range_انتہا >= alloc_انتہا && new_range_انتہا <= range_انتہا) {
            // Found space - now reserve it.
            fill->range_آغاز = new_range_انتہا;
            fill->range_انتہا = range_انتہا;
            fill->alloc_سائز = سائز;

            info->range_انتہا = new_range_انتہا;
            hlist_جوڑو_before(&fill->گرہ, &info->گرہ);
            واپس new_range_انتہا;
        }
    }
    واپس 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
ساکن ساخت allocdetail_s *
alloc_new_detail(ساخت allocdetail_s *عارضی)
{
    u32 detail_addr = alloc_new(&Zone_Tmp_بلند, حجم(ساخت allocdetail_s)
                                , MALLOC_MIN_ALIGN, &عارضی->detailinfo);
    اگر (!detail_addr) {
        detail_addr = alloc_new(&Zone_Tmp_کم, حجم(ساخت allocdetail_s)
                                , MALLOC_MIN_ALIGN, &عارضی->detailinfo);
        اگر (!detail_addr) {
            warn_noalloc();
            واپس تہی;
        }
    }
    ساخت allocdetail_s *detail = memremap(detail_addr, حجم(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    حافظہ_نقل(detail, عارضی, حجم(*detail));
    hlist_replace(&عارضی->detailinfo.گرہ, &detail->detailinfo.گرہ);
    hlist_replace(&عارضی->datainfo.گرہ, &detail->datainfo.گرہ);
    واپس detail;
}

// Add new memory to a zone
ساکن خالی
alloc_جوڑو(ساخت zone_s *zone, u32 آغاز, u32 انتہا)
{
    // Find position to add space
    ساخت allocinfo_s *info;
    ساخت hlist_گرہ **pprev;
    hlist_for_each_اندراج_pprev(info, pprev, &zone->سر, گرہ) {
        اگر (info->range_آغاز < آغاز)
            توڑو;
    }

    // Add space using temporary allocation info.
    ساخت allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_آغاز = آغاز;
    tempdetail.datainfo.range_انتہا = انتہا;
    tempdetail.datainfo.alloc_سائز = 0;
    hlist_جوڑو(&tempdetail.datainfo.گرہ, pprev);

    // Allocate final allocation info.
    ساخت allocdetail_s *detail = alloc_new_detail(&tempdetail);
    اگر (!detail)
        hlist_del(&tempdetail.datainfo.گرہ);
}

// Release space allocated with alloc_new()
ساکن خالی
alloc_free(ساخت allocinfo_s *info)
{
    ساخت allocinfo_s *اگلا = container_کا_or_null(
        info->گرہ.اگلا, ساخت allocinfo_s, گرہ);
    اگر (اگلا && اگلا->range_انتہا == info->range_آغاز)
        اگلا->range_انتہا = info->range_انتہا;
    hlist_del(&info->گرہ);
}

// Search all zones for an allocation obtained from alloc_new()
ساکن ساخت allocinfo_s *
alloc_ڈھونڈو(u32 ڈیٹا)
{
    صحیح i;
    برائے (i=0; i<ARRAY_سائز(Zones); i++) {
        ساخت allocinfo_s *info;
        hlist_for_each_اندراج(info, &Zones[i]->سر, گرہ) {
            اگر (info->range_آغاز == ڈیٹا)
                واپس info;
        }
    }
    واپس تہی;
}

// Find the lowest memory range added by alloc_add()
ساکن ساخت allocinfo_s *
alloc_ڈھونڈو_سب_سے_نیچا(ساخت zone_s *zone)
{
    ساخت allocinfo_s *info, *آخری = تہی;
    hlist_for_each_اندراج(info, &zone->سر, گرہ) {
        آخری = info;
    }
    واپس آخری;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
ساکن صحیح
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_سائز)
{
    u32 lowram = GET_BDA(mem_سائز_kb) * 1024;
    اگر (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        واپس -1;

    // Do copy
    memmove((خالی*)newebda, (خالی*)oldebda, ebda_سائز * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    سیٹ_BDA(mem_سائز_kb, newebda / 1024);
    سیٹ_BDA(ebda_seg, FLATPTR_مقصد_SEG(newebda));
    واپس 0;
}

// Support expanding the ZoneLow dynamically.
ساکن u32
zonelow_expand(u32 سائز, u32 align, ساخت allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    اگر (unlikely(wait_preempt())) {
        u32 ڈیٹا = alloc_new(&Zone_کم, سائز, align, fill);
        اگر (ڈیٹا)
            واپس ڈیٹا;
    }

    ساخت allocinfo_s *info = alloc_ڈھونڈو_سب_سے_نیچا(&Zone_کم);
    اگر (!info)
        واپس 0;
    u32 oldpos = info->range_انتہا;
    u32 newpos = ALIGN_نیچے_کو(oldpos - سائز, align);
    u32 bottom = info->range_آغاز + info->alloc_سائز;
    اگر (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        واپس alloc_new(&Zone_کم, سائز, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_سائز = GET_EBDA(ebda_seg, سائز);
    u32 ebda_انتہا = ebda_pos + ebda_سائز * 1024;
    اگر (ebda_انتہا != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_نیچے_کو(ebda_انتہا - سائز, align);
    u32 newbottom = ALIGN_نیچے_کو(newpos, 1024);
    u32 newebda = ALIGN_نیچے_کو(newbottom - ebda_سائز * 1024, 1024);
    اگر (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        واپس 0;

    // Move ebda
    صحیح ret = relocate_ebda(newebda, ebda_pos, ebda_سائز);
    اگر (ret)
        واپس 0;

    // Update zone
    اگر (ebda_انتہا == bottom)
        info->range_آغاز = newbottom;
    ورنہ
        alloc_جوڑو(&Zone_کم, newbottom, ebda_انتہا);

    واپس alloc_new(&Zone_کم, سائز, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(ساخت zone_s *zone, u32 سائز, u32 align)
{
    ASSERT32FLAT();
    اگر (!سائز)
        واپس 0;

    // Find and reserve space for main allocation
    ساخت allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 ڈیٹا = alloc_new(zone, سائز, align, &tempdetail.datainfo);
    اگر (!CONFIG_MALLOC_UPPERMEMORY && !ڈیٹا && zone == &Zone_کم)
        ڈیٹا = zonelow_expand(سائز, align, &tempdetail.datainfo);
    اگر (!ڈیٹا)
        واپس 0;

    // Find and reserve space for bookkeeping.
    ساخت allocdetail_s *detail = alloc_new_detail(&tempdetail);
    اگر (!detail) {
        alloc_free(&tempdetail.datainfo);
        واپس 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, سائز, align, ڈیٹا, detail);

    واپس ڈیٹا;
}

// Allocate virtual memory from the given zone
خالی * __malloc
_malloc(ساخت zone_s *zone, u32 سائز, u32 align)
{
    واپس memremap(malloc_palloc(zone, سائز, align), سائز);
}

// Free a data block allocated with phys_alloc
صحیح
malloc_pfree(u32 ڈیٹا)
{
    ASSERT32FLAT();
    ساخت allocinfo_s *info = alloc_ڈھونڈو(ڈیٹا);
    اگر (!info || ڈیٹا == virt_مقصد_phys(info) || !info->alloc_سائز)
        واپس -1;
    ساخت allocdetail_s *detail = container_کا(
        info, ساخت allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", ڈیٹا, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    واپس 0;
}

خالی
آزاد(خالی *ڈیٹا)
{
    اگر (!ڈیٹا)
        واپس;
    صحیح ret = malloc_pfree(virt_مقصد_phys(ڈیٹا));
    اگر (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(ساخت zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    ساخت allocinfo_s *info;
    hlist_for_each_اندراج(info, &zone->سر, گرہ) {
        u32 space = info->range_انتہا - info->range_آغاز - info->alloc_سائز;
        اگر (space > maxspace)
            maxspace = space;
    }

    اگر (zone != &Zone_Tmp_بلند && zone != &Zone_Tmp_کم)
        واپس maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(حجم(ساخت allocdetail_s), MALLOC_MIN_ALIGN);
    اگر (maxspace <= reserve)
        واپس 0;
    واپس maxspace - reserve;
}

// Set a handle associated with an allocation.
خالی
malloc_sethandle(u32 ڈیٹا, u32 handle)
{
    ASSERT32FLAT();
    ساخت allocinfo_s *info = alloc_ڈھونڈو(ڈیٹا);
    اگر (!info || ڈیٹا == virt_مقصد_phys(info) || !info->alloc_سائز)
        واپس;
    ساخت allocdetail_s *detail = container_کا(
        info, ساخت allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    صحیح i;
    برائے (i=0; i<ARRAY_سائز(Zones); i++) {
        ساخت allocinfo_s *info;
        hlist_for_each_اندراج(info, &Zones[i]->سر, گرہ) {
            اگر (info->range_آغاز != virt_مقصد_phys(info))
                جاری;
            ساخت allocdetail_s *detail = container_کا(
                info, ساخت allocdetail_s, detailinfo);
            اگر (detail->handle == handle)
                واپس detail->datainfo.range_آغاز;
        }
    }
    واپس 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

ساکن u32 Rom_انتہا = BUILD_ROM_آغاز;
ساکن ساخت allocinfo_s *Rom_بنیاد;

#تعریف_کرو OPROM_سرخی_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(خالی)
{
    اگر (CONFIG_MALLOC_UPPERMEMORY)
        واپس ALIGN_نیچے_کو(Rom_بنیاد->range_انتہا - OPROM_سرخی_RESERVE
                          , OPTION_ROM_ALIGN);
    واپس علامت(final_readonly_آغاز);
}

// Return the end of the last deployed option rom.
u32
rom_get_آخری(خالی)
{
    واپس Rom_انتہا;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
ساخت rom_سرخی *
rom_reserve(u32 سائز)
{
    u32 newend = ALIGN(Rom_انتہا + سائز, OPTION_ROM_ALIGN);
    اگر (newend > rom_get_max())
        واپس تہی;
    اگر (CONFIG_MALLOC_UPPERMEMORY) {
        اگر (newend < علامت(zonelow_بنیاد))
            newend = علامت(zonelow_بنیاد);
        Rom_بنیاد->range_آغاز = newend + OPROM_سرخی_RESERVE;
    }
    واپس (خالی*)Rom_انتہا;
}

// Confirm space as in use by an optionrom.
صحیح
rom_confirm(u32 سائز)
{
    خالی *new = rom_reserve(سائز);
    اگر (!new) {
        warn_noalloc();
        واپس -1;
    }
    Rom_انتہا = ALIGN(Rom_انتہا + سائز, OPTION_ROM_ALIGN);
    واپس 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

خالی
malloc_preinit(خالی)
{
    ASSERT32FLAT();
    dprintf(3, "malloc preinit\n");

    // Don't declare any memory between 0xa0000 and 0x100000
    e820_remove(BUILD_LOWRAM_انتہا, BUILD_BIOS_ADDR-BUILD_LOWRAM_انتہا);

    // Mark known areas as reserved.
    e820_جوڑو(BUILD_BIOS_ADDR, BUILD_BIOS_سائز, E820_RESERVED);

    // Populate temp high ram
    u32 highram_آغاز = 0;
    u32 highram_سائز = 0;
    صحیح i;
    برائے (i=e820_گنتی-1; i>=0; i--) {
        ساخت e820entry *en = &e820_list[i];
        u64 انتہا = en->آغاز + en->سائز;
        اگر (انتہا < 1024*1024)
            توڑو;
        اگر (en->type != E820_RAM || انتہا > 0xffffffff)
            جاری;
        u32 s = en->آغاز, e = انتہا;
        اگر (!highram_آغاز) {
            u32 new_max = ALIGN_نیچے_کو(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_نیچے_کو(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            اگر (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_آغاز = e = new_max;
                highram_سائز = BUILD_MAX_HIGHTABLE;
            } ورنہ اگر (new_min <= e && new_min >= s) {
                highram_آغاز = e = new_min;
                highram_سائز = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_جوڑو(&Zone_Tmp_بلند, s, e);
    }

    // Populate regions
    alloc_جوڑو(&Zone_Tmp_کم, BUILD_ڈھیر_ADDR, BUILD_EBDA_MINIMUM);
    اگر (highram_آغاز) {
        alloc_جوڑو(&Zone_بلند, highram_آغاز, highram_آغاز + highram_سائز);
        e820_جوڑو(highram_آغاز, highram_سائز, E820_RESERVED);
    }
}

خالی
malloc_csm_preinit(u32 کم_pmm, u32 کم_pmm_سائز, u32 hi_pmm, u32 hi_pmm_سائز)
{
    ASSERT32FLAT();

    اگر (hi_pmm_سائز > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_انتہا = hi_pmm + hi_pmm_سائز;
        alloc_جوڑو(&Zone_Tmp_بلند, hi_pmm, hi_pmm_انتہا - BUILD_MIN_HIGHTABLE);
        alloc_جوڑو(&Zone_بلند, hi_pmm_انتہا - BUILD_MIN_HIGHTABLE, hi_pmm_انتہا);
    } ورنہ {
        alloc_جوڑو(&Zone_Tmp_بلند, hi_pmm, hi_pmm + hi_pmm_سائز);
    }
    alloc_جوڑو(&Zone_Tmp_کم, کم_pmm, کم_pmm + کم_pmm_سائز);
}

u32 Legacy_Ram_سائز VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
ساکن خالی
calc_Ram_سائز(خالی)
{
    u32 rs = 0;
    صحیح i;
    برائے (i=e820_گنتی-1; i>=0; i--) {
        ساخت e820entry *en = &e820_list[i];
        u64 انتہا = en->آغاز + en->سائز;
        u32 type = en->type;
        اگر (انتہا <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = انتہا;
            توڑو;
        }
    }
    Legacy_Ram_سائز = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
خالی
malloc_init(خالی)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    اگر (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        صحیح i;
        برائے (i=0; i<ARRAY_سائز(Zones); i++) {
            ساخت zone_s *zone = Zones[i];
            اگر (zone->سر.پہلا)
                zone->سر.پہلا->pprev = &zone->سر.پہلا;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_آغاز), VSYMBOL(varlow_آغاز)
            , علامت(varlow_انتہا) - علامت(varlow_آغاز));
    اگر (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_جوڑو(&Zone_کم, علامت(zonelow_بنیاد) + OPROM_سرخی_RESERVE
                  , علامت(final_varlow_آغاز));
        Rom_بنیاد = alloc_ڈھونڈو_سب_سے_نیچا(&Zone_کم);
    } ورنہ {
        alloc_جوڑو(&Zone_کم, ALIGN_نیچے_کو(علامت(final_varlow_آغاز), 1024)
                  , علامت(final_varlow_آغاز));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_آغاز), 0
           , علامت(zonefseg_انتہا) - علامت(zonefseg_آغاز));
    alloc_جوڑو(&ZoneFSeg, علامت(zonefseg_آغاز), علامت(zonefseg_انتہا));

    calc_Ram_سائز();
}

خالی
malloc_prepboot(خالی)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 بنیاد = rom_get_max();
    memset((خالی*)Rom_انتہا, 0, بنیاد-Rom_انتہا);
    اگر (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        ساخت rom_سرخی *dummyrom = (خالی*)بنیاد;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        صحیح سائز = (BUILD_BIOS_ADDR - بنیاد) / 512;
        dummyrom->سائز = (سائز > 255) ? 255 : سائز;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_سائز_kb)*1024;
    e820_جوڑو(endlow, BUILD_LOWRAM_انتہا-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    ساخت allocinfo_s *info = alloc_ڈھونڈو_سب_سے_نیچا(&ZoneFSeg);
    u32 سائز = info->range_انتہا - info->range_آغاز;
    memset(memremap(info->range_آغاز, سائز), 0, سائز);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , Rom_انتہا, بنیاد, info->range_آغاز, info->range_انتہا);

    // Give back unused high ram.
    info = alloc_ڈھونڈو_سب_سے_نیچا(&Zone_بلند);
    اگر (info) {
        u32 giveback = ALIGN_نیچے_کو(info->range_انتہا-info->range_آغاز, صفحہ_سائز);
        e820_جوڑو(info->range_آغاز, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calc_Ram_سائز();
}
