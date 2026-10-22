// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // FUNC16
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // _malloc
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // struct e820entry
#ਅਨ੍ਤਰ੍ਭਾਵ "std/pmm.h" // PMM_SIGNATURE
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // checksum
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // pmm_init
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // __ffs

ਬਾਹਰੀ ਬਣਤਰ pmmheader PMMHEADER;

#ਜੇ CONFIG_PMM
ਬਣਤਰ pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .ਸੰਸਕਰਣ_2 = 0x01,
    .ਲੰਮਾਈ = ਆਕਾਰ(PMMHEADER),
};
#ਜੇ_ਅੰਤ

// PMM - allocate
ਸਥਿਰ u32
handle_pmm00(u16 *ਦਲੀਲਾਂ)
{
    u32 ਲੰਮਾਈ = *(u32*)&ਦਲੀਲਾਂ[1], handle = *(u32*)&ਦਲੀਲਾਂ[3];
    u16 flags = ਦਲੀਲਾਂ[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , ਲੰਮਾਈ, handle, flags);
    ਬਣਤਰ zone_s *lowzone = &Zone_Tmp_ਨੀਵਾਂ, *highzone = &Zone_Tmp_ਉੱਚਾ;
    ਜੇ (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_ਨੀਵਾਂ;
        highzone = &Zone_ਉੱਚਾ;
    }
    ਜੇ (!ਲੰਮਾਈ) {
        // Memory size request
        ਚੋਣ (flags & 3) {
        ਮੂਲ:
        ਹਾਲਤ 0:
            ਵਾਪਸ 0;
        ਹਾਲਤ 1:
            ਵਾਪਸ malloc_getspace(lowzone);
        ਹਾਲਤ 2:
            ਵਾਪਸ malloc_getspace(highzone);
        ਹਾਲਤ 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            ਜੇ (spacelow > spacehigh)
                ਵਾਪਸ spacelow;
            ਵਾਪਸ spacehigh;
        }
        }
    }
    u32 ਮਾਪ = ਲੰਮਾਈ * 16;
    ਜੇ ((s32)ਮਾਪ <= 0)
        ਵਾਪਸ 0;
    u32 align = MALLOC_MIN_ALIGN;
    ਜੇ (flags & 4) {
        align = 1<<__ffs(ਮਾਪ);
        ਜੇ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 ਅੰਕੜੇ;
    ਚੋਣ (flags & 3) {
    ਮੂਲ:
    ਹਾਲਤ 0:
        ਵਾਪਸ 0;
    ਹਾਲਤ 1:
        ਅੰਕੜੇ = malloc_palloc(lowzone, ਮਾਪ, align);
        ਰੋਕੋ;
    ਹਾਲਤ 2:
        ਅੰਕੜੇ = malloc_palloc(highzone, ਮਾਪ, align);
        ਜੇ (!ਅੰਕੜੇ && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            ਅੰਕੜੇ = malloc_palloc(&Zone_Tmp_ਉੱਚਾ, ਮਾਪ, align);
            ਜੇ (ਅੰਕੜੇ)
                e820_ਜੋੜੋ(ਅੰਕੜੇ, ਮਾਪ, E820_RESERVED);
        }
        ਰੋਕੋ;
    ਹਾਲਤ 3: {
        ਅੰਕੜੇ = malloc_palloc(lowzone, ਮਾਪ, align);
        ਜੇ (!ਅੰਕੜੇ)
            ਅੰਕੜੇ = malloc_palloc(highzone, ਮਾਪ, align);
    }
    }
    ਜੇ (ਅੰਕੜੇ && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(ਅੰਕੜੇ, handle);
    ਵਾਪਸ ਅੰਕੜੇ;
}

// PMM - find
ਸਥਿਰ u32
handle_pmm01(u16 *ਦਲੀਲਾਂ)
{
    u32 handle = *(u32*)&ਦਲੀਲਾਂ[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    ਜੇ (handle == MALLOC_DEFAULT_HANDLE)
        ਵਾਪਸ 0;
    ਵਾਪਸ malloc_findhandle(handle);
}

// PMM - deallocate
ਸਥਿਰ u32
handle_pmm02(u16 *ਦਲੀਲਾਂ)
{
    u32 ਬਫਰ = *(u32*)&ਦਲੀਲਾਂ[1];
    dprintf(3, "pmm02: buffer=%x\n", ਬਫਰ);
    ਪੂਰਨ ret = malloc_pfree(ਬਫਰ);
    ਜੇ (ret)
        // Error
        ਵਾਪਸ 1;
    ਵਾਪਸ 0;
}

ਸਥਿਰ u32
handle_pmmXX(u16 *ਦਲੀਲਾਂ)
{
    ਵਾਪਸ PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *ਦਲੀਲਾਂ)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_PMM)
        ਵਾਪਸ PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = ਦਲੀਲਾਂ[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    ਚੋਣ (arg1) {
    ਹਾਲਤ 0x00: ret = handle_pmm00(ਦਲੀਲਾਂ); ਰੋਕੋ;
    ਹਾਲਤ 0x01: ret = handle_pmm01(ਦਲੀਲਾਂ); ਰੋਕੋ;
    ਹਾਲਤ 0x02: ret = handle_pmm02(ਦਲੀਲਾਂ); ਰੋਕੋ;
    ਮੂਲ:   ret = handle_pmmXX(ਦਲੀਲਾਂ); ਰੋਕੋ;
    }

    ਵਾਪਸ ret;
}

ਖਾਲੀ
pmm_init(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PMM)
        ਵਾਪਸ;

    dprintf(3, "init PMM\n");

    PMMHEADER.ਇੰਦਰਾਜ = FUNC16(ਇੰਦਰਾਜ_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, ਆਕਾਰ(PMMHEADER));
}

ਖਾਲੀ
pmm_prepboot(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PMM)
        ਵਾਪਸ;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.ਇੰਦਰਾਜ.segoff = 0;
}
