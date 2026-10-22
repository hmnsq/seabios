// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // FUNC16
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // _malloc
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "e820map.h" // struct e820entry
#अन्तर्भाव "std/pmm.h" // PMM_SIGNATURE
#अन्तर्भाव "string.h" // checksum
#अन्तर्भाव "util.h" // pmm_init
#अन्तर्भाव "x86.h" // __ffs

बाह्य संरचना pmmheader PMMHEADER;

#जइ CONFIG_PMM
संरचना pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = परिमाण(PMMHEADER),
};
#endif

// PMM - allocate
स्थिर u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    संरचना zone_s *lowzone = &Zone_Tmp_नीचः, *highzone = &Zone_Tmp_उच्चः;
    जइ (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_नीचः;
        highzone = &Zone_उच्चः;
    }
    जइ (!length) {
        // Memory size request
        विकल्प (flags & 3) {
        स्वभाव:
        पक्ष 0:
            प्रत्यागम 0;
        पक्ष 1:
            प्रत्यागम malloc_getspace(lowzone);
        पक्ष 2:
            प्रत्यागम malloc_getspace(highzone);
        पक्ष 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            जइ (spacelow > spacehigh)
                प्रत्यागम spacelow;
            प्रत्यागम spacehigh;
        }
        }
    }
    u32 size = length * 16;
    जइ ((s32)size <= 0)
        प्रत्यागम 0;
    u32 align = MALLOC_MIN_ALIGN;
    जइ (flags & 4) {
        align = 1<<__ffs(size);
        जइ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    विकल्प (flags & 3) {
    स्वभाव:
    पक्ष 0:
        प्रत्यागम 0;
    पक्ष 1:
        data = malloc_palloc(lowzone, size, align);
        भङ्ग;
    पक्ष 2:
        data = malloc_palloc(highzone, size, align);
        जइ (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_उच्चः, size, align);
            जइ (data)
                e820_add(data, size, E820_RESERVED);
        }
        भङ्ग;
    पक्ष 3: {
        data = malloc_palloc(lowzone, size, align);
        जइ (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    जइ (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    प्रत्यागम data;
}

// PMM - find
स्थिर u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    जइ (handle == MALLOC_DEFAULT_HANDLE)
        प्रत्यागम 0;
    प्रत्यागम malloc_findhandle(handle);
}

// PMM - deallocate
स्थिर u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    पूर्णाङ्क ret = malloc_pfree(buffer);
    जइ (ret)
        // Error
        प्रत्यागम 1;
    प्रत्यागम 0;
}

स्थिर u32
handle_pmmXX(u16 *args)
{
    प्रत्यागम PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    जइ (! CONFIG_PMM)
        प्रत्यागम PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    विकल्प (arg1) {
    पक्ष 0x00: ret = handle_pmm00(args); भङ्ग;
    पक्ष 0x01: ret = handle_pmm01(args); भङ्ग;
    पक्ष 0x02: ret = handle_pmm02(args); भङ्ग;
    स्वभाव:   ret = handle_pmmXX(args); भङ्ग;
    }

    प्रत्यागम ret;
}

शून्य
pmm_init(शून्य)
{
    जइ (! CONFIG_PMM)
        प्रत्यागम;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, परिमाण(PMMHEADER));
}

शून्य
pmm_prepboot(शून्य)
{
    जइ (! CONFIG_PMM)
        प्रत्यागम;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
