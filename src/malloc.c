// Internal dynamic memory allocations.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_BDA
#include "config.h" // BUILD_BIOS_ADDR
#include "e820map.h" // struct e820entry
#include "list.h" // hlist_node
#include "malloc.h" // _malloc
#include "memmap.h" // PAGE_SIZE
#include "output.h" // dprintf
#include "stacks.h" // wait_preempt
#include "std/optionrom.h" // OPTION_ROM_ALIGN
#include "string.h" // memset

// Information on a reserved area.
sa_nracanaa allocinfo_s {
    sa_nracanaa hlist_node node;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
sa_nracanaa allocdetail_s {
    sa_nracanaa allocinfo_s detailinfo;
    sa_nracanaa allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
sa_nracanaa zone_s {
    sa_nracanaa hlist_head head;
};

sa_nracanaa zone_s ZoneLow VARVERIFY32INIT, ZoneHigh VARVERIFY32INIT;
sa_nracanaa zone_s ZoneFSeg VARVERIFY32INIT;
sa_nracanaa zone_s ZoneTmpLow VARVERIFY32INIT, ZoneTmpHigh VARVERIFY32INIT;

static sa_nracanaa zone_s *Zones[] VARVERIFY32INIT = {
    &ZoneTmpLow, &ZoneLow, &ZoneFSeg, &ZoneTmpHigh, &ZoneHigh
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
static u32
alloc_new(sa_nracanaa zone_s *zone, u32 size, u32 align, sa_nracanaa allocinfo_s *fill)
{
    sa_nracanaa allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, node) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        ᬬᬾᬦ᭄ (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->node, &info->node);
            ᬯᬮᬶ new_range_end;
        }
    }
    ᬯᬮᬶ 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
static sa_nracanaa allocdetail_s *
alloc_new_detail(sa_nracanaa allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&ZoneTmpHigh, parimaa_na(sa_nracanaa allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    ᬬᬾᬦ᭄ (!detail_addr) {
        detail_addr = alloc_new(&ZoneTmpLow, parimaa_na(sa_nracanaa allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        ᬬᬾᬦ᭄ (!detail_addr) {
            warn_noalloc();
            ᬯᬮᬶ NULL;
        }
    }
    sa_nracanaa allocdetail_s *detail = memremap(detail_addr, parimaa_na(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, parimaa_na(*detail));
    hlist_replace(&temp->detailinfo.node, &detail->detailinfo.node);
    hlist_replace(&temp->datainfo.node, &detail->datainfo.node);
    ᬯᬮᬶ detail;
}

// Add new memory to a zone
static void
alloc_add(sa_nracanaa zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    sa_nracanaa allocinfo_s *info;
    sa_nracanaa hlist_node **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, node) {
        ᬬᬾᬦ᭄ (info->range_start < start)
            bha_nga;
    }

    // Add space using temporary allocation info.
    sa_nracanaa allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.node, pprev);

    // Allocate final allocation info.
    sa_nracanaa allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ᬬᬾᬦ᭄ (!detail)
        hlist_del(&tempdetail.datainfo.node);
}

// Release space allocated with alloc_new()
static void
alloc_free(sa_nracanaa allocinfo_s *info)
{
    sa_nracanaa allocinfo_s *next = container_of_or_null(
        info->node.next, sa_nracanaa allocinfo_s, node);
    ᬬᬾᬦ᭄ (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->node);
}

// Search all zones for an allocation obtained from alloc_new()
static sa_nracanaa allocinfo_s *
alloc_find(u32 data)
{
    puur_naa_nka i;
    for (i=0; i<ARRAY_SIZE(Zones); i++) {
        sa_nracanaa allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, node) {
            ᬬᬾᬦ᭄ (info->range_start == data)
                ᬯᬮᬶ info;
        }
    }
    ᬯᬮᬶ NULL;
}

// Find the lowest memory range added by alloc_add()
static sa_nracanaa allocinfo_s *
alloc_find_lowest(sa_nracanaa zone_s *zone)
{
    sa_nracanaa allocinfo_s *info, *last = NULL;
    hlist_for_each_entry(info, &zone->head, node) {
        last = info;
    }
    ᬯᬮᬶ last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
static puur_naa_nka
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    ᬬᬾᬦ᭄ (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        ᬯᬮᬶ -1;

    // Do copy
    memmove((void*)newebda, (void*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    ᬯᬮᬶ 0;
}

// Support expanding the ZoneLow dynamically.
static u32
zonelow_expand(u32 size, u32 align, sa_nracanaa allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    ᬬᬾᬦ᭄ (unlikely(wait_preempt())) {
        u32 data = alloc_new(&ZoneLow, size, align, fill);
        ᬬᬾᬦ᭄ (data)
            ᬯᬮᬶ data;
    }

    sa_nracanaa allocinfo_s *info = alloc_find_lowest(&ZoneLow);
    ᬬᬾᬦ᭄ (!info)
        ᬯᬮᬶ 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    ᬬᬾᬦ᭄ (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        ᬯᬮᬶ alloc_new(&ZoneLow, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    ᬬᬾᬦ᭄ (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    ᬬᬾᬦ᭄ (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        ᬯᬮᬶ 0;

    // Move ebda
    puur_naa_nka ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    ᬬᬾᬦ᭄ (ret)
        ᬯᬮᬶ 0;

    // Update zone
    ᬬᬾᬦ᭄ (ebda_end == bottom)
        info->range_start = newbottom;
    ᬮᬶᬬ
        alloc_add(&ZoneLow, newbottom, ebda_end);

    ᬯᬮᬶ alloc_new(&ZoneLow, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(sa_nracanaa zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    ᬬᬾᬦ᭄ (!size)
        ᬯᬮᬶ 0;

    // Find and reserve space for main allocation
    sa_nracanaa allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    ᬬᬾᬦ᭄ (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &ZoneLow)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    ᬬᬾᬦ᭄ (!data)
        ᬯᬮᬶ 0;

    // Find and reserve space for bookkeeping.
    sa_nracanaa allocdetail_s *detail = alloc_new_detail(&tempdetail);
    ᬬᬾᬦ᭄ (!detail) {
        alloc_free(&tempdetail.datainfo);
        ᬯᬮᬶ 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    ᬯᬮᬶ data;
}

// Allocate virtual memory from the given zone
void * __malloc
_malloc(sa_nracanaa zone_s *zone, u32 size, u32 align)
{
    ᬯᬮᬶ memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
puur_naa_nka
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    sa_nracanaa allocinfo_s *info = alloc_find(data);
    ᬬᬾᬦ᭄ (!info || data == virt_to_phys(info) || !info->alloc_size)
        ᬯᬮᬶ -1;
    sa_nracanaa allocdetail_s *detail = container_of(
        info, sa_nracanaa allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    ᬯᬮᬶ 0;
}

void
free(void *data)
{
    ᬬᬾᬦ᭄ (!data)
        ᬯᬮᬶ;
    puur_naa_nka ret = malloc_pfree(virt_to_phys(data));
    ᬬᬾᬦ᭄ (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(sa_nracanaa zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    sa_nracanaa allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, node) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        ᬬᬾᬦ᭄ (space > maxspace)
            maxspace = space;
    }

    ᬬᬾᬦ᭄ (zone != &ZoneTmpHigh && zone != &ZoneTmpLow)
        ᬯᬮᬶ maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(parimaa_na(sa_nracanaa allocdetail_s), MALLOC_MIN_ALIGN);
    ᬬᬾᬦ᭄ (maxspace <= reserve)
        ᬯᬮᬶ 0;
    ᬯᬮᬶ maxspace - reserve;
}

// Set a handle associated with an allocation.
void
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    sa_nracanaa allocinfo_s *info = alloc_find(data);
    ᬬᬾᬦ᭄ (!info || data == virt_to_phys(info) || !info->alloc_size)
        ᬯᬮᬶ;
    sa_nracanaa allocdetail_s *detail = container_of(
        info, sa_nracanaa allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    puur_naa_nka i;
    for (i=0; i<ARRAY_SIZE(Zones); i++) {
        sa_nracanaa allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, node) {
            ᬬᬾᬦ᭄ (info->range_start != virt_to_phys(info))
                continue;
            sa_nracanaa allocdetail_s *detail = container_of(
                info, sa_nracanaa allocdetail_s, detailinfo);
            ᬬᬾᬦ᭄ (detail->handle == handle)
                ᬯᬮᬶ detail->datainfo.range_start;
        }
    }
    ᬯᬮᬶ 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

static u32 RomEnd = BUILD_ROM_START;
static sa_nracanaa allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(void)
{
    ᬬᬾᬦ᭄ (CONFIG_MALLOC_UPPERMEMORY)
        ᬯᬮᬶ ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    ᬯᬮᬶ SYMBOL(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(void)
{
    ᬯᬮᬶ RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
sa_nracanaa rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    ᬬᬾᬦ᭄ (newend > rom_get_max())
        ᬯᬮᬶ NULL;
    ᬬᬾᬦ᭄ (CONFIG_MALLOC_UPPERMEMORY) {
        ᬬᬾᬦ᭄ (newend < SYMBOL(zonelow_base))
            newend = SYMBOL(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    ᬯᬮᬶ (void*)RomEnd;
}

// Confirm space as in use by an optionrom.
puur_naa_nka
rom_confirm(u32 size)
{
    void *new = rom_reserve(size);
    ᬬᬾᬦ᭄ (!new) {
        warn_noalloc();
        ᬯᬮᬶ -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    ᬯᬮᬶ 0;
}


/****************************************************************
 * Setup
 ****************************************************************/

void
malloc_preinit(void)
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
    puur_naa_nka i;
    for (i=e820_count-1; i>=0; i--) {
        sa_nracanaa e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        ᬬᬾᬦ᭄ (end < 1024*1024)
            bha_nga;
        ᬬᬾᬦ᭄ (en->type != E820_RAM || end > 0xffffffff)
            continue;
        u32 s = en->start, e = end;
        ᬬᬾᬦ᭄ (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            ᬬᬾᬦ᭄ (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } ᬮᬶᬬ ᬬᬾᬦ᭄ (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&ZoneTmpHigh, s, e);
    }

    // Populate regions
    alloc_add(&ZoneTmpLow, BUILD_STACK_ADDR, BUILD_EBDA_MINIMUM);
    ᬬᬾᬦ᭄ (highram_start) {
        alloc_add(&ZoneHigh, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

void
malloc_csm_preinit(u32 low_pmm, u32 low_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    ᬬᬾᬦ᭄ (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&ZoneTmpHigh, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&ZoneHigh, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } ᬮᬶᬬ {
        alloc_add(&ZoneTmpHigh, hi_pmm, hi_pmm + hi_pmm_size);
    }
    alloc_add(&ZoneTmpLow, low_pmm, low_pmm + low_pmm_size);
}

u32 LegacyRamSize VARFSEG;

// Calculate the maximum ramsize (less than 4gig) from e820 map.
static void
calcRamSize(void)
{
    u32 rs = 0;
    puur_naa_nka i;
    for (i=e820_count-1; i>=0; i--) {
        sa_nracanaa e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        ᬬᬾᬦ᭄ (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            bha_nga;
        }
    }
    LegacyRamSize = rs >= 1024*1024 ? rs : 1024*1024;
}

// Update pointers after code relocation.
void
malloc_init(void)
{
    ASSERT32FLAT();
    dprintf(3, "malloc init\n");

    ᬬᬾᬦ᭄ (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        puur_naa_nka i;
        for (i=0; i<ARRAY_SIZE(Zones); i++) {
            sa_nracanaa zone_s *zone = Zones[i];
            ᬬᬾᬦ᭄ (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , SYMBOL(varlow_end) - SYMBOL(varlow_start));
    ᬬᬾᬦ᭄ (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&ZoneLow, SYMBOL(zonelow_base) + OPROM_HEADER_RESERVE
                  , SYMBOL(final_varlow_start));
        RomBase = alloc_find_lowest(&ZoneLow);
    } ᬮᬶᬬ {
        alloc_add(&ZoneLow, ALIGN_DOWN(SYMBOL(final_varlow_start), 1024)
                  , SYMBOL(final_varlow_start));
    }

    // Add space available in f-segment to ZoneFSeg
    memset(VSYMBOL(zonefseg_start), 0
           , SYMBOL(zonefseg_end) - SYMBOL(zonefseg_start));
    alloc_add(&ZoneFSeg, SYMBOL(zonefseg_start), SYMBOL(zonefseg_end));

    calcRamSize();
}

void
malloc_prepboot(void)
{
    ASSERT32FLAT();
    dprintf(3, "malloc finalize\n");

    u32 base = rom_get_max();
    memset((void*)RomEnd, 0, base-RomEnd);
    ᬬᬾᬦ᭄ (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        sa_nracanaa rom_header *dummyrom = (void*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        puur_naa_nka size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    sa_nracanaa allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&ZoneHigh);
    ᬬᬾᬦ᭄ (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
