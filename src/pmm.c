// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // FUNC16
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // _malloc
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "e820map.h" // struct e820entry
#ಅನ್ತರ್ಭಾವ "std/pmm.h" // PMM_SIGNATURE
#ಅನ್ತರ್ಭಾವ "string.h" // checksum
#ಅನ್ತರ್ಭಾವ "util.h" // pmm_init
#ಅನ್ತರ್ಭಾವ "x86.h" // __ffs

ಬಾಹ್ಯ ಸಂರಚನಾ pmmheader PMMHEADER;

#ಯದಿ CONFIG_PMM
ಸಂರಚನಾ pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = ಪರಿಮಾಣ(PMMHEADER),
};
#endif

// PMM - allocate
ಸ್ಥಿರ u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    ಸಂರಚನಾ zone_s *lowzone = &Zone_Tmp_ಕೆಳಗಿನ, *highzone = &Zone_Tmp_ಮೇಲಿನ;
    ಯದಿ (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_ಕೆಳಗಿನ;
        highzone = &Zone_ಮೇಲಿನ;
    }
    ಯದಿ (!length) {
        // Memory size request
        ವಿಕಲ್ಪ (flags & 3) {
        ಸ್ವಭಾವ:
        ಪಕ್ಷ 0:
            ಪ್ರತ್ಯಾಗಮ 0;
        ಪಕ್ಷ 1:
            ಪ್ರತ್ಯಾಗಮ malloc_getspace(lowzone);
        ಪಕ್ಷ 2:
            ಪ್ರತ್ಯಾಗಮ malloc_getspace(highzone);
        ಪಕ್ಷ 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            ಯದಿ (spacelow > spacehigh)
                ಪ್ರತ್ಯಾಗಮ spacelow;
            ಪ್ರತ್ಯಾಗಮ spacehigh;
        }
        }
    }
    u32 size = length * 16;
    ಯದಿ ((s32)size <= 0)
        ಪ್ರತ್ಯಾಗಮ 0;
    u32 align = MALLOC_MIN_ALIGN;
    ಯದಿ (flags & 4) {
        align = 1<<__ffs(size);
        ಯದಿ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    ವಿಕಲ್ಪ (flags & 3) {
    ಸ್ವಭಾವ:
    ಪಕ್ಷ 0:
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪಕ್ಷ 1:
        data = malloc_palloc(lowzone, size, align);
        ಭಙ್ಗ;
    ಪಕ್ಷ 2:
        data = malloc_palloc(highzone, size, align);
        ಯದಿ (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_ಮೇಲಿನ, size, align);
            ಯದಿ (data)
                e820_add(data, size, E820_RESERVED);
        }
        ಭಙ್ಗ;
    ಪಕ್ಷ 3: {
        data = malloc_palloc(lowzone, size, align);
        ಯದಿ (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    ಯದಿ (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    ಪ್ರತ್ಯಾಗಮ data;
}

// PMM - find
ಸ್ಥಿರ u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    ಯದಿ (handle == MALLOC_DEFAULT_HANDLE)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ malloc_findhandle(handle);
}

// PMM - deallocate
ಸ್ಥಿರ u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    ಪೂರ್ಣಾಙ್ಕ ret = malloc_pfree(buffer);
    ಯದಿ (ret)
        // Error
        ಪ್ರತ್ಯಾಗಮ 1;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32
handle_pmmXX(u16 *args)
{
    ಪ್ರತ್ಯಾಗಮ PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_PMM)
        ಪ್ರತ್ಯಾಗಮ PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    ವಿಕಲ್ಪ (arg1) {
    ಪಕ್ಷ 0x00: ret = handle_pmm00(args); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: ret = handle_pmm01(args); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: ret = handle_pmm02(args); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   ret = handle_pmmXX(args); ಭಙ್ಗ;
    }

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ
pmm_init(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PMM)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, ಪರಿಮಾಣ(PMMHEADER));
}

ಶೂನ್ಯ
pmm_prepboot(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PMM)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
