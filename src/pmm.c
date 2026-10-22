// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // FUNC16
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // _malloc
#شامل "output.h" // dprintf
#شامل "e820map.h" // struct e820entry
#شامل "std/pmm.h" // PMM_SIGNATURE
#شامل "string.h" // checksum
#شامل "util.h" // pmm_init
#شامل "x86.h" // __ffs

بیرونی ساختار pmmheader PMMHEADER;

#اگر CONFIG_PMM
ساختار pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = اندازه(PMMHEADER),
};
#endif

// PMM - allocate
ایستا u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    ساختار zone_s *lowzone = &Zone_Tmp_کم, *highzone = &Zone_Tmp_بالا;
    اگر (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_کم;
        highzone = &Zone_بالا;
    }
    اگر (!length) {
        // Memory size request
        انتخاب (flags & 3) {
        پیشفرض:
        حالت 0:
            بازگشت 0;
        حالت 1:
            بازگشت malloc_getspace(lowzone);
        حالت 2:
            بازگشت malloc_getspace(highzone);
        حالت 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            اگر (spacelow > spacehigh)
                بازگشت spacelow;
            بازگشت spacehigh;
        }
        }
    }
    u32 size = length * 16;
    اگر ((s32)size <= 0)
        بازگشت 0;
    u32 align = MALLOC_MIN_ALIGN;
    اگر (flags & 4) {
        align = 1<<__ffs(size);
        اگر (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    انتخاب (flags & 3) {
    پیشفرض:
    حالت 0:
        بازگشت 0;
    حالت 1:
        data = malloc_palloc(lowzone, size, align);
        بشکن;
    حالت 2:
        data = malloc_palloc(highzone, size, align);
        اگر (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_بالا, size, align);
            اگر (data)
                e820_add(data, size, E820_RESERVED);
        }
        بشکن;
    حالت 3: {
        data = malloc_palloc(lowzone, size, align);
        اگر (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    اگر (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    بازگشت data;
}

// PMM - find
ایستا u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    اگر (handle == MALLOC_DEFAULT_HANDLE)
        بازگشت 0;
    بازگشت malloc_findhandle(handle);
}

// PMM - deallocate
ایستا u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    صحیح ret = malloc_pfree(buffer);
    اگر (ret)
        // Error
        بازگشت 1;
    بازگشت 0;
}

ایستا u32
handle_pmmXX(u16 *args)
{
    بازگشت PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    اگر (! CONFIG_PMM)
        بازگشت PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    انتخاب (arg1) {
    حالت 0x00: ret = handle_pmm00(args); بشکن;
    حالت 0x01: ret = handle_pmm01(args); بشکن;
    حالت 0x02: ret = handle_pmm02(args); بشکن;
    پیشفرض:   ret = handle_pmmXX(args); بشکن;
    }

    بازگشت ret;
}

تهی
pmm_init(تهی)
{
    اگر (! CONFIG_PMM)
        بازگشت;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, اندازه(PMMHEADER));
}

تهی
pmm_prepboot(تهی)
{
    اگر (! CONFIG_PMM)
        بازگشت;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
