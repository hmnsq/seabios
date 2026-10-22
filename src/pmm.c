// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // FUNC16
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // _malloc
#ضمن "output.h" // dprintf
#ضمن "e820map.h" // struct e820entry
#ضمن "std/pmm.h" // PMM_SIGNATURE
#ضمن "string.h" // checksum
#ضمن "util.h" // pmm_init
#ضمن "x86.h" // __ffs

خارجي بنية pmmheader PMMHEADER;

#إذا CONFIG_PMM
بنية pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = حجم(PMMHEADER),
};
#endif

// PMM - allocate
ساكن u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    بنية zone_s *lowzone = &Zone_Tmp_تۆۋەن, *highzone = &Zone_Tmp_يۇقىرى;
    إذا (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_تۆۋەن;
        highzone = &Zone_يۇقىرى;
    }
    إذا (!length) {
        // Memory size request
        بدل (flags & 3) {
        افتراضي:
        حالة 0:
            أرجع 0;
        حالة 1:
            أرجع malloc_getspace(lowzone);
        حالة 2:
            أرجع malloc_getspace(highzone);
        حالة 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            إذا (spacelow > spacehigh)
                أرجع spacelow;
            أرجع spacehigh;
        }
        }
    }
    u32 size = length * 16;
    إذا ((s32)size <= 0)
        أرجع 0;
    u32 align = MALLOC_MIN_ALIGN;
    إذا (flags & 4) {
        align = 1<<__ffs(size);
        إذا (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    بدل (flags & 3) {
    افتراضي:
    حالة 0:
        أرجع 0;
    حالة 1:
        data = malloc_palloc(lowzone, size, align);
        اكسر;
    حالة 2:
        data = malloc_palloc(highzone, size, align);
        إذا (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_يۇقىرى, size, align);
            إذا (data)
                e820_add(data, size, E820_RESERVED);
        }
        اكسر;
    حالة 3: {
        data = malloc_palloc(lowzone, size, align);
        إذا (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    إذا (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    أرجع data;
}

// PMM - find
ساكن u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    إذا (handle == MALLOC_DEFAULT_HANDLE)
        أرجع 0;
    أرجع malloc_findhandle(handle);
}

// PMM - deallocate
ساكن u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    صحيح ret = malloc_pfree(buffer);
    إذا (ret)
        // Error
        أرجع 1;
    أرجع 0;
}

ساكن u32
handle_pmmXX(u16 *args)
{
    أرجع PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    إذا (! CONFIG_PMM)
        أرجع PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    بدل (arg1) {
    حالة 0x00: ret = handle_pmm00(args); اكسر;
    حالة 0x01: ret = handle_pmm01(args); اكسر;
    حالة 0x02: ret = handle_pmm02(args); اكسر;
    افتراضي:   ret = handle_pmmXX(args); اكسر;
    }

    أرجع ret;
}

فراغ
pmm_init(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, حجم(PMMHEADER));
}

فراغ
pmm_prepboot(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
