// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // FUNC16
#include "config.h" // CONFIG_*
#include "malloc.h" // _malloc
#include "output.h" // dprintf
#include "e820map.h" // struct e820entry
#include "std/pmm.h" // PMM_SIGNATURE
#include "string.h" // checksum
#include "util.h" // pmm_init
#include "x86.h" // __ffs

extern struct pmmheader PMMHEADER;

#もし CONFIG_PMM
struct pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = sizeof(PMMHEADER),
};
#endif

// PMM - allocate
static u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    struct zone_s *lowzone = &ZoneTmpLow, *highzone = &ZoneTmpHigh;
    もし (flags & 8) {
        // Permanent memory request.
        lowzone = &ZoneLow;
        highzone = &ZoneHigh;
    }
    もし (!length) {
        // Memory size request
        switch (flags & 3) {
        default:
        case 0:
            戻る 0;
        case 1:
            戻る malloc_getspace(lowzone);
        case 2:
            戻る malloc_getspace(highzone);
        case 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            もし (spacelow > spacehigh)
                戻る spacelow;
            戻る spacehigh;
        }
        }
    }
    u32 size = length * 16;
    もし ((s32)size <= 0)
        戻る 0;
    u32 align = MALLOC_MIN_ALIGN;
    もし (flags & 4) {
        align = 1<<__ffs(size);
        もし (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    switch (flags & 3) {
    default:
    case 0:
        戻る 0;
    case 1:
        data = malloc_palloc(lowzone, size, align);
        break;
    case 2:
        data = malloc_palloc(highzone, size, align);
        もし (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&ZoneTmpHigh, size, align);
            もし (data)
                e820_add(data, size, E820_RESERVED);
        }
        break;
    case 3: {
        data = malloc_palloc(lowzone, size, align);
        もし (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    もし (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    戻る data;
}

// PMM - find
static u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    もし (handle == MALLOC_DEFAULT_HANDLE)
        戻る 0;
    戻る malloc_findhandle(handle);
}

// PMM - deallocate
static u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    整数 ret = malloc_pfree(buffer);
    もし (ret)
        // Error
        戻る 1;
    戻る 0;
}

static u32
handle_pmmXX(u16 *args)
{
    戻る PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    もし (! CONFIG_PMM)
        戻る PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    switch (arg1) {
    case 0x00: ret = handle_pmm00(args); break;
    case 0x01: ret = handle_pmm01(args); break;
    case 0x02: ret = handle_pmm02(args); break;
    default:   ret = handle_pmmXX(args); break;
    }

    戻る ret;
}

void
pmm_init(void)
{
    もし (! CONFIG_PMM)
        戻る;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, sizeof(PMMHEADER));
}

void
pmm_prepboot(void)
{
    もし (! CONFIG_PMM)
        戻る;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
