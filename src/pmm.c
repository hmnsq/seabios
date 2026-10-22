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

ಬಾಹ್ಯ ರಚನೆ pmmheader PMMHEADER;

#ಆದರೆ CONFIG_PMM
ರಚನೆ pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .ಆವೃತ್ತಿ_2 = 0x01,
    .ಉದ್ದಳತೆ = ಗಾತ್ರ(PMMHEADER),
};
#ಆದರೆ_ಅಂತ್ಯ

// PMM - allocate
ಸ್ಥಿರ u32
handle_pmm00(u16 *ವಾದಗಳು)
{
    u32 ಉದ್ದಳತೆ = *(u32*)&ವಾದಗಳು[1], handle = *(u32*)&ವಾದಗಳು[3];
    u16 flags = ವಾದಗಳು[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , ಉದ್ದಳತೆ, handle, flags);
    ರಚನೆ zone_s *lowzone = &Zone_Tmp_ಕೆಳಗಿನ, *highzone = &Zone_Tmp_ಮೇಲಿನ;
    ಆದರೆ (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_ಕೆಳಗಿನ;
        highzone = &Zone_ಮೇಲಿನ;
    }
    ಆದರೆ (!ಉದ್ದಳತೆ) {
        // Memory size request
        ಆಯ್ಕೆ (flags & 3) {
        ಪೂರ್ವನಿಯೋಜಿತ:
        ಸ್ಥಿತಿ 0:
            ಹಿಂತಿರುಗಿ 0;
        ಸ್ಥಿತಿ 1:
            ಹಿಂತಿರುಗಿ malloc_getspace(lowzone);
        ಸ್ಥಿತಿ 2:
            ಹಿಂತಿರುಗಿ malloc_getspace(highzone);
        ಸ್ಥಿತಿ 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            ಆದರೆ (spacelow > spacehigh)
                ಹಿಂತಿರುಗಿ spacelow;
            ಹಿಂತಿರುಗಿ spacehigh;
        }
        }
    }
    u32 ಗಾತ್ರ_2 = ಉದ್ದಳತೆ * 16;
    ಆದರೆ ((s32)ಗಾತ್ರ_2 <= 0)
        ಹಿಂತಿರುಗಿ 0;
    u32 align = MALLOC_MIN_ALIGN;
    ಆದರೆ (flags & 4) {
        align = 1<<__ffs(ಗಾತ್ರ_2);
        ಆದರೆ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 ದತ್ತಾಂಶ;
    ಆಯ್ಕೆ (flags & 3) {
    ಪೂರ್ವನಿಯೋಜಿತ:
    ಸ್ಥಿತಿ 0:
        ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ 1:
        ದತ್ತಾಂಶ = malloc_palloc(lowzone, ಗಾತ್ರ_2, align);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 2:
        ದತ್ತಾಂಶ = malloc_palloc(highzone, ಗಾತ್ರ_2, align);
        ಆದರೆ (!ದತ್ತಾಂಶ && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            ದತ್ತಾಂಶ = malloc_palloc(&Zone_Tmp_ಮೇಲಿನ, ಗಾತ್ರ_2, align);
            ಆದರೆ (ದತ್ತಾಂಶ)
                e820_ಕೂಡಿಸು(ದತ್ತಾಂಶ, ಗಾತ್ರ_2, E820_RESERVED);
        }
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 3: {
        ದತ್ತಾಂಶ = malloc_palloc(lowzone, ಗಾತ್ರ_2, align);
        ಆದರೆ (!ದತ್ತಾಂಶ)
            ದತ್ತಾಂಶ = malloc_palloc(highzone, ಗಾತ್ರ_2, align);
    }
    }
    ಆದರೆ (ದತ್ತಾಂಶ && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(ದತ್ತಾಂಶ, handle);
    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
}

// PMM - find
ಸ್ಥಿರ u32
handle_pmm01(u16 *ವಾದಗಳು)
{
    u32 handle = *(u32*)&ವಾದಗಳು[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    ಆದರೆ (handle == MALLOC_DEFAULT_HANDLE)
        ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ malloc_findhandle(handle);
}

// PMM - deallocate
ಸ್ಥಿರ u32
handle_pmm02(u16 *ವಾದಗಳು)
{
    u32 ಬಫರ್ = *(u32*)&ವಾದಗಳು[1];
    dprintf(3, "pmm02: buffer=%x\n", ಬಫರ್);
    ಪೂರ್ಣಾಂಕ ret = malloc_pfree(ಬಫರ್);
    ಆದರೆ (ret)
        // Error
        ಹಿಂತಿರುಗಿ 1;
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32
handle_pmmXX(u16 *ವಾದಗಳು)
{
    ಹಿಂತಿರುಗಿ PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *ವಾದಗಳು)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_PMM)
        ಹಿಂತಿರುಗಿ PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = ವಾದಗಳು[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    ಆಯ್ಕೆ (arg1) {
    ಸ್ಥಿತಿ 0x00: ret = handle_pmm00(ವಾದಗಳು); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: ret = handle_pmm01(ವಾದಗಳು); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: ret = handle_pmm02(ವಾದಗಳು); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   ret = handle_pmmXX(ವಾದಗಳು); ನಿಲ್ಲಿಸು;
    }

    ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ
pmm_init(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PMM)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init PMM\n");

    PMMHEADER.ನಮೂದು = FUNC16(ನಮೂದು_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, ಗಾತ್ರ(PMMHEADER));
}

ಶೂನ್ಯ
pmm_prepboot(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PMM)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.ನಮೂದು.segoff = 0;
}
