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
struct allocinfo_s {
    struct hlist_node node;
    u32 range_start, range_end, alloc_size;
};

// Information on a tracked memory allocation.
struct allocdetail_s {
    struct allocinfo_s detailinfo;
    struct allocinfo_s datainfo;
    u32 handle;
};

// The various memory zones.
struct zone_s {
    struct hlist_head head;
};

struct zone_s ZoneLow VARVERIFY32INIT, ZoneHigh VARVERIFY32INIT;
struct zone_s ZoneFSeg VARVERIFY32INIT;
struct zone_s ZoneTmpLow VARVERIFY32INIT, ZoneTmpHigh VARVERIFY32INIT;

static struct zone_s *Zones[] VARVERIFY32INIT = {
    &ZoneTmpLow, &ZoneLow, &ZoneFSeg, &ZoneTmpHigh, &ZoneHigh
};


/****************************************************************
 * low-level memory reservations
 ****************************************************************/

// Find and reserve space from a given zone
static u32
alloc_new(struct zone_s *zone, u32 size, u32 align, struct allocinfo_s *fill)
{
    struct allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, node) {
        u32 alloc_end = info->range_start + info->alloc_size;
        u32 range_end = info->range_end;
        u32 new_range_end = ALIGN_DOWN(range_end - size, align);
        如果 (new_range_end >= alloc_end && new_range_end <= range_end) {
            // Found space - now reserve it.
            fill->range_start = new_range_end;
            fill->range_end = range_end;
            fill->alloc_size = size;

            info->range_end = new_range_end;
            hlist_add_before(&fill->node, &info->node);
            返回 new_range_end;
        }
    }
    返回 0;
}

// Reserve space for a 'struct allocdetail_s' and fill
static struct allocdetail_s *
alloc_new_detail(struct allocdetail_s *temp)
{
    u32 detail_addr = alloc_new(&ZoneTmpHigh, sizeof(struct allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
    如果 (!detail_addr) {
        detail_addr = alloc_new(&ZoneTmpLow, sizeof(struct allocdetail_s)
                                , MALLOC_MIN_ALIGN, &temp->detailinfo);
        如果 (!detail_addr) {
            warn_noalloc();
            返回 NULL;
        }
    }
    struct allocdetail_s *detail = memremap(detail_addr, sizeof(*detail));

    // Fill final 'detail' allocation from data in 'temp'
    memcpy(detail, temp, sizeof(*detail));
    hlist_replace(&temp->detailinfo.node, &detail->detailinfo.node);
    hlist_replace(&temp->datainfo.node, &detail->datainfo.node);
    返回 detail;
}

// Add new memory to a zone
static void
alloc_add(struct zone_s *zone, u32 start, u32 end)
{
    // Find position to add space
    struct allocinfo_s *info;
    struct hlist_node **pprev;
    hlist_for_each_entry_pprev(info, pprev, &zone->head, node) {
        如果 (info->range_start < start)
            break;
    }

    // Add space using temporary allocation info.
    struct allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    tempdetail.datainfo.range_start = start;
    tempdetail.datainfo.range_end = end;
    tempdetail.datainfo.alloc_size = 0;
    hlist_add(&tempdetail.datainfo.node, pprev);

    // Allocate final allocation info.
    struct allocdetail_s *detail = alloc_new_detail(&tempdetail);
    如果 (!detail)
        hlist_del(&tempdetail.datainfo.node);
}

// Release space allocated with alloc_new()
static void
alloc_free(struct allocinfo_s *info)
{
    struct allocinfo_s *next = container_of_or_null(
        info->node.next, struct allocinfo_s, node);
    如果 (next && next->range_end == info->range_start)
        next->range_end = info->range_end;
    hlist_del(&info->node);
}

// Search all zones for an allocation obtained from alloc_new()
static struct allocinfo_s *
alloc_find(u32 data)
{
    整数 i;
    为 (i=0; i<ARRAY_SIZE(Zones); i++) {
        struct allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, node) {
            如果 (info->range_start == data)
                返回 info;
        }
    }
    返回 NULL;
}

// Find the lowest memory range added by alloc_add()
static struct allocinfo_s *
alloc_find_lowest(struct zone_s *zone)
{
    struct allocinfo_s *info, *last = NULL;
    hlist_for_each_entry(info, &zone->head, node) {
        last = info;
    }
    返回 last;
}


/****************************************************************
 * ebda movement
 ****************************************************************/

// Move ebda
static 整数
relocate_ebda(u32 newebda, u32 oldebda, u8 ebda_size)
{
    u32 lowram = GET_BDA(mem_size_kb) * 1024;
    如果 (oldebda != lowram)
        // EBDA isn't at end of ram - give up.
        返回 -1;

    // Do copy
    memmove((void*)newebda, (void*)oldebda, ebda_size * 1024);

    // Update indexes
    dprintf(1, "ebda moved from %x to %x\n", oldebda, newebda);
    SET_BDA(mem_size_kb, newebda / 1024);
    SET_BDA(ebda_seg, FLATPTR_TO_SEG(newebda));
    返回 0;
}

// Support expanding the ZoneLow dynamically.
static u32
zonelow_expand(u32 size, u32 align, struct allocinfo_s *fill)
{
    // Make sure to not move ebda while an optionrom is running.
    如果 (unlikely(wait_preempt())) {
        u32 data = alloc_new(&ZoneLow, size, align, fill);
        如果 (data)
            返回 data;
    }

    struct allocinfo_s *info = alloc_find_lowest(&ZoneLow);
    如果 (!info)
        返回 0;
    u32 oldpos = info->range_end;
    u32 newpos = ALIGN_DOWN(oldpos - size, align);
    u32 bottom = info->range_start + info->alloc_size;
    如果 (newpos >= bottom && newpos <= oldpos)
        // Space already present.
        返回 alloc_new(&ZoneLow, size, align, fill);
    u16 ebda_seg = get_ebda_seg();
    u32 ebda_pos = (u32)MAKE_FLATPTR(ebda_seg, 0);
    u8 ebda_size = GET_EBDA(ebda_seg, size);
    u32 ebda_end = ebda_pos + ebda_size * 1024;
    如果 (ebda_end != bottom)
        // Something else is after ebda - can't use any existing space.
        newpos = ALIGN_DOWN(ebda_end - size, align);
    u32 newbottom = ALIGN_DOWN(newpos, 1024);
    u32 newebda = ALIGN_DOWN(newbottom - ebda_size * 1024, 1024);
    如果 (newebda < BUILD_EBDA_MINIMUM)
        // Not enough space.
        返回 0;

    // Move ebda
    整数 ret = relocate_ebda(newebda, ebda_pos, ebda_size);
    如果 (ret)
        返回 0;

    // Update zone
    如果 (ebda_end == bottom)
        info->range_start = newbottom;
    否则
        alloc_add(&ZoneLow, newbottom, ebda_end);

    返回 alloc_new(&ZoneLow, size, align, fill);
}


/****************************************************************
 * tracked memory allocations
 ****************************************************************/

// Allocate physical memory from the given zone and track it as a PMM allocation
u32
malloc_palloc(struct zone_s *zone, u32 size, u32 align)
{
    ASSERT32FLAT();
    如果 (!size)
        返回 0;

    // Find and reserve space for main allocation
    struct allocdetail_s tempdetail;
    tempdetail.handle = MALLOC_DEFAULT_HANDLE;
    u32 data = alloc_new(zone, size, align, &tempdetail.datainfo);
    如果 (!CONFIG_MALLOC_UPPERMEMORY && !data && zone == &ZoneLow)
        data = zonelow_expand(size, align, &tempdetail.datainfo);
    如果 (!data)
        返回 0;

    // Find and reserve space for bookkeeping.
    struct allocdetail_s *detail = alloc_new_detail(&tempdetail);
    如果 (!detail) {
        alloc_free(&tempdetail.datainfo);
        返回 0;
    }

    dprintf(8, "phys_alloc zone=%p size=%d align=%x ret=%x (detail=%p)\n"
            , zone, size, align, data, detail);

    返回 data;
}

// Allocate virtual memory from the given zone
void * __malloc
_malloc(struct zone_s *zone, u32 size, u32 align)
{
    返回 memremap(malloc_palloc(zone, size, align), size);
}

// Free a data block allocated with phys_alloc
整数
malloc_pfree(u32 data)
{
    ASSERT32FLAT();
    struct allocinfo_s *info = alloc_find(data);
    如果 (!info || data == virt_to_phys(info) || !info->alloc_size)
        返回 -1;
    struct allocdetail_s *detail = container_of(
        info, struct allocdetail_s, datainfo);
    dprintf(8, "phys_free %x (detail=%p)\n", data, detail);
    alloc_free(info);
    alloc_free(&detail->detailinfo);
    返回 0;
}

void
free(void *data)
{
    如果 (!data)
        返回;
    整数 ret = malloc_pfree(virt_to_phys(data));
    如果 (ret)
        warn_internalerror();
}

// Find the amount of free space in a given zone.
u32
malloc_getspace(struct zone_s *zone)
{
    // XXX - doesn't account for ZoneLow being able to grow.
    // XXX - results not reliable when CONFIG_THREAD_OPTIONROMS
    u32 maxspace = 0;
    struct allocinfo_s *info;
    hlist_for_each_entry(info, &zone->head, node) {
        u32 space = info->range_end - info->range_start - info->alloc_size;
        如果 (space > maxspace)
            maxspace = space;
    }

    如果 (zone != &ZoneTmpHigh && zone != &ZoneTmpLow)
        返回 maxspace;
    // Account for space needed for PMM tracking.
    u32 reserve = ALIGN(sizeof(struct allocdetail_s), MALLOC_MIN_ALIGN);
    如果 (maxspace <= reserve)
        返回 0;
    返回 maxspace - reserve;
}

// Set a handle associated with an allocation.
void
malloc_sethandle(u32 data, u32 handle)
{
    ASSERT32FLAT();
    struct allocinfo_s *info = alloc_find(data);
    如果 (!info || data == virt_to_phys(info) || !info->alloc_size)
        返回;
    struct allocdetail_s *detail = container_of(
        info, struct allocdetail_s, datainfo);
    detail->handle = handle;
}

// Find the data block allocated with phys_alloc with a given handle.
u32
malloc_findhandle(u32 handle)
{
    整数 i;
    为 (i=0; i<ARRAY_SIZE(Zones); i++) {
        struct allocinfo_s *info;
        hlist_for_each_entry(info, &Zones[i]->head, node) {
            如果 (info->range_start != virt_to_phys(info))
                continue;
            struct allocdetail_s *detail = container_of(
                info, struct allocdetail_s, detailinfo);
            如果 (detail->handle == handle)
                返回 detail->datainfo.range_start;
        }
    }
    返回 0;
}


/****************************************************************
 * 0xc0000-0xf0000 management
 ****************************************************************/

static u32 RomEnd = BUILD_ROM_START;
static struct allocinfo_s *RomBase;

#define OPROM_HEADER_RESERVE 16

// Return the maximum memory position option roms may use.
u32
rom_get_max(void)
{
    如果 (CONFIG_MALLOC_UPPERMEMORY)
        返回 ALIGN_DOWN(RomBase->range_end - OPROM_HEADER_RESERVE
                          , OPTION_ROM_ALIGN);
    返回 SYMBOL(final_readonly_start);
}

// Return the end of the last deployed option rom.
u32
rom_get_last(void)
{
    返回 RomEnd;
}

// Request space for an optionrom in 0xc0000-0xf0000 area.
struct rom_header *
rom_reserve(u32 size)
{
    u32 newend = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    如果 (newend > rom_get_max())
        返回 NULL;
    如果 (CONFIG_MALLOC_UPPERMEMORY) {
        如果 (newend < SYMBOL(zonelow_base))
            newend = SYMBOL(zonelow_base);
        RomBase->range_start = newend + OPROM_HEADER_RESERVE;
    }
    返回 (void*)RomEnd;
}

// Confirm space as in use by an optionrom.
整数
rom_confirm(u32 size)
{
    void *new = rom_reserve(size);
    如果 (!new) {
        warn_noalloc();
        返回 -1;
    }
    RomEnd = ALIGN(RomEnd + size, OPTION_ROM_ALIGN);
    返回 0;
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
    整数 i;
    为 (i=e820_count-1; i>=0; i--) {
        struct e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        如果 (end < 1024*1024)
            break;
        如果 (en->type != E820_RAM || end > 0xffffffff)
            continue;
        u32 s = en->start, e = end;
        如果 (!highram_start) {
            u32 new_max = ALIGN_DOWN(e - BUILD_MAX_HIGHTABLE, MALLOC_MIN_ALIGN);
            u32 new_min = ALIGN_DOWN(e - BUILD_MIN_HIGHTABLE, MALLOC_MIN_ALIGN);
            如果 (new_max <= e && new_max >= s + BUILD_MAX_HIGHTABLE) {
                highram_start = e = new_max;
                highram_size = BUILD_MAX_HIGHTABLE;
            } 否则 如果 (new_min <= e && new_min >= s) {
                highram_start = e = new_min;
                highram_size = BUILD_MIN_HIGHTABLE;
            }
        }
        alloc_add(&ZoneTmpHigh, s, e);
    }

    // Populate regions
    alloc_add(&ZoneTmpLow, BUILD_STACK_ADDR, BUILD_EBDA_MINIMUM);
    如果 (highram_start) {
        alloc_add(&ZoneHigh, highram_start, highram_start + highram_size);
        e820_add(highram_start, highram_size, E820_RESERVED);
    }
}

void
malloc_csm_preinit(u32 low_pmm, u32 low_pmm_size, u32 hi_pmm, u32 hi_pmm_size)
{
    ASSERT32FLAT();

    如果 (hi_pmm_size > BUILD_MIN_HIGHTABLE) {
        u32 hi_pmm_end = hi_pmm + hi_pmm_size;
        alloc_add(&ZoneTmpHigh, hi_pmm, hi_pmm_end - BUILD_MIN_HIGHTABLE);
        alloc_add(&ZoneHigh, hi_pmm_end - BUILD_MIN_HIGHTABLE, hi_pmm_end);
    } 否则 {
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
    整数 i;
    为 (i=e820_count-1; i>=0; i--) {
        struct e820entry *en = &e820_list[i];
        u64 end = en->start + en->size;
        u32 type = en->type;
        如果 (end <= 0xffffffff && (type == E820_ACPI || type == E820_RAM)) {
            rs = end;
            break;
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

    如果 (CONFIG_RELOCATE_INIT) {
        // Fixup malloc pointers after relocation
        整数 i;
        为 (i=0; i<ARRAY_SIZE(Zones); i++) {
            struct zone_s *zone = Zones[i];
            如果 (zone->head.first)
                zone->head.first->pprev = &zone->head.first;
        }
    }

    // Initialize low-memory region
    memmove(VSYMBOL(final_varlow_start), VSYMBOL(varlow_start)
            , SYMBOL(varlow_end) - SYMBOL(varlow_start));
    如果 (CONFIG_MALLOC_UPPERMEMORY) {
        alloc_add(&ZoneLow, SYMBOL(zonelow_base) + OPROM_HEADER_RESERVE
                  , SYMBOL(final_varlow_start));
        RomBase = alloc_find_lowest(&ZoneLow);
    } 否则 {
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
    如果 (CONFIG_MALLOC_UPPERMEMORY) {
        // Place an optionrom signature around used low mem area.
        struct rom_header *dummyrom = (void*)base;
        dummyrom->signature = OPTION_ROM_SIGNATURE;
        整数 size = (BUILD_BIOS_ADDR - base) / 512;
        dummyrom->size = (size > 255) ? 255 : size;
    }

    // Reserve more low-mem if needed.
    u32 endlow = GET_BDA(mem_size_kb)*1024;
    e820_add(endlow, BUILD_LOWRAM_END-endlow, E820_RESERVED);

    // Clear unused f-seg ram.
    struct allocinfo_s *info = alloc_find_lowest(&ZoneFSeg);
    u32 size = info->range_end - info->range_start;
    memset(memremap(info->range_start, size), 0, size);
    dprintf(1, "Space available for UMB: %x-%x, %x-%x\n"
            , RomEnd, base, info->range_start, info->range_end);

    // Give back unused high ram.
    info = alloc_find_lowest(&ZoneHigh);
    如果 (info) {
        u32 giveback = ALIGN_DOWN(info->range_end-info->range_start, PAGE_SIZE);
        e820_add(info->range_start, giveback, E820_RAM);
        dprintf(1, "Returned %d bytes of ZoneHigh\n", giveback);
    }

    calcRamSize();
}
