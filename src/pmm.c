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

extern sa_nracanaa pmmheader PMMHEADER;

#ᯀᯮᯝ CONFIG_PMM
sa_nracanaa pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = parimaa_na(PMMHEADER),
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
    sa_nracanaa zone_s *lowzone = &ZoneTmpLow, *highzone = &ZoneTmpHigh;
    ᯀᯮᯝ (flags & 8) {
        // Permanent memory request.
        lowzone = &ZoneLow;
        highzone = &ZoneHigh;
    }
    ᯀᯮᯝ (!length) {
        // Memory size request
        switch (flags & 3) {
        default:
        pak_sa 0:
            ᯀᯮᯞ 0;
        pak_sa 1:
            ᯀᯮᯞ malloc_getspace(lowzone);
        pak_sa 2:
            ᯀᯮᯞ malloc_getspace(highzone);
        pak_sa 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            ᯀᯮᯝ (spacelow > spacehigh)
                ᯀᯮᯞ spacelow;
            ᯀᯮᯞ spacehigh;
        }
        }
    }
    u32 size = length * 16;
    ᯀᯮᯝ ((s32)size <= 0)
        ᯀᯮᯞ 0;
    u32 align = MALLOC_MIN_ALIGN;
    ᯀᯮᯝ (flags & 4) {
        align = 1<<__ffs(size);
        ᯀᯮᯝ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    switch (flags & 3) {
    default:
    pak_sa 0:
        ᯀᯮᯞ 0;
    pak_sa 1:
        data = malloc_palloc(lowzone, size, align);
        bha_nga;
    pak_sa 2:
        data = malloc_palloc(highzone, size, align);
        ᯀᯮᯝ (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&ZoneTmpHigh, size, align);
            ᯀᯮᯝ (data)
                e820_add(data, size, E820_RESERVED);
        }
        bha_nga;
    pak_sa 3: {
        data = malloc_palloc(lowzone, size, align);
        ᯀᯮᯝ (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    ᯀᯮᯝ (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    ᯀᯮᯞ data;
}

// PMM - find
static u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    ᯀᯮᯝ (handle == MALLOC_DEFAULT_HANDLE)
        ᯀᯮᯞ 0;
    ᯀᯮᯞ malloc_findhandle(handle);
}

// PMM - deallocate
static u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    puur_naa_nka ret = malloc_pfree(buffer);
    ᯀᯮᯝ (ret)
        // Error
        ᯀᯮᯞ 1;
    ᯀᯮᯞ 0;
}

static u32
handle_pmmXX(u16 *args)
{
    ᯀᯮᯞ PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    ᯀᯮᯝ (! CONFIG_PMM)
        ᯀᯮᯞ PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    switch (arg1) {
    pak_sa 0x00: ret = handle_pmm00(args); bha_nga;
    pak_sa 0x01: ret = handle_pmm01(args); bha_nga;
    pak_sa 0x02: ret = handle_pmm02(args); bha_nga;
    default:   ret = handle_pmmXX(args); bha_nga;
    }

    ᯀᯮᯞ ret;
}

void
pmm_init(void)
{
    ᯀᯮᯝ (! CONFIG_PMM)
        ᯀᯮᯞ;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, parimaa_na(PMMHEADER));
}

void
pmm_prepboot(void)
{
    ᯀᯮᯝ (! CONFIG_PMM)
        ᯀᯮᯞ;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
