// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // FUNC16
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // _malloc
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "e820map.h" // struct e820entry
#অন্তর্ভাব "std/pmm.h" // PMM_SIGNATURE
#অন্তর্ভাব "string.h" // checksum
#অন্তর্ভাব "util.h" // pmm_init
#অন্তর্ভাব "x86.h" // __ffs

বাহ্য সংরচনা pmmheader PMMHEADER;

#যদি CONFIG_PMM
সংরচনা pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = পরিমাণ(PMMHEADER),
};
#endif

// PMM - allocate
স্থির u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    সংরচনা zone_s *lowzone = &Zone_Tmp_নিচের, *highzone = &Zone_Tmp_উপরের;
    যদি (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_নিচের;
        highzone = &Zone_উপরের;
    }
    যদি (!length) {
        // Memory size request
        বিকল্প (flags & 3) {
        স্বভাব:
        পক্ষ 0:
            প্রত্যাগম 0;
        পক্ষ 1:
            প্রত্যাগম malloc_getspace(lowzone);
        পক্ষ 2:
            প্রত্যাগম malloc_getspace(highzone);
        পক্ষ 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            যদি (spacelow > spacehigh)
                প্রত্যাগম spacelow;
            প্রত্যাগম spacehigh;
        }
        }
    }
    u32 size = length * 16;
    যদি ((s32)size <= 0)
        প্রত্যাগম 0;
    u32 align = MALLOC_MIN_ALIGN;
    যদি (flags & 4) {
        align = 1<<__ffs(size);
        যদি (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    বিকল্প (flags & 3) {
    স্বভাব:
    পক্ষ 0:
        প্রত্যাগম 0;
    পক্ষ 1:
        data = malloc_palloc(lowzone, size, align);
        ভঙ্গ;
    পক্ষ 2:
        data = malloc_palloc(highzone, size, align);
        যদি (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_উপরের, size, align);
            যদি (data)
                e820_add(data, size, E820_RESERVED);
        }
        ভঙ্গ;
    পক্ষ 3: {
        data = malloc_palloc(lowzone, size, align);
        যদি (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    যদি (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    প্রত্যাগম data;
}

// PMM - find
স্থির u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    যদি (handle == MALLOC_DEFAULT_HANDLE)
        প্রত্যাগম 0;
    প্রত্যাগম malloc_findhandle(handle);
}

// PMM - deallocate
স্থির u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    পূর্ণাঙ্ক ret = malloc_pfree(buffer);
    যদি (ret)
        // Error
        প্রত্যাগম 1;
    প্রত্যাগম 0;
}

স্থির u32
handle_pmmXX(u16 *args)
{
    প্রত্যাগম PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    যদি (! CONFIG_PMM)
        প্রত্যাগম PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    বিকল্প (arg1) {
    পক্ষ 0x00: ret = handle_pmm00(args); ভঙ্গ;
    পক্ষ 0x01: ret = handle_pmm01(args); ভঙ্গ;
    পক্ষ 0x02: ret = handle_pmm02(args); ভঙ্গ;
    স্বভাব:   ret = handle_pmmXX(args); ভঙ্গ;
    }

    প্রত্যাগম ret;
}

শূন্য
pmm_init(শূন্য)
{
    যদি (! CONFIG_PMM)
        প্রত্যাগম;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, পরিমাণ(PMMHEADER));
}

শূন্য
pmm_prepboot(শূন্য)
{
    যদি (! CONFIG_PMM)
        প্রত্যাগম;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
