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

বাহ্যিক কাঠামো pmmheader PMMHEADER;

#যদি CONFIG_PMM
কাঠামো pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .দৈর্ঘ্য = আকার(PMMHEADER),
};
#endif

// PMM - allocate
স্থির u32
handle_pmm00(u16 *args)
{
    u32 দৈর্ঘ্য = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , দৈর্ঘ্য, handle, flags);
    কাঠামো zone_s *lowzone = &Zone_Tmp_নিচের, *highzone = &Zone_Tmp_উপরের;
    যদি (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_নিচের;
        highzone = &Zone_উপরের;
    }
    যদি (!দৈর্ঘ্য) {
        // Memory size request
        নির্বাচন (flags & 3) {
        পূর্বনির্ধারিত:
        ক্ষেত্র 0:
            ফেরত 0;
        ক্ষেত্র 1:
            ফেরত malloc_getspace(lowzone);
        ক্ষেত্র 2:
            ফেরত malloc_getspace(highzone);
        ক্ষেত্র 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            যদি (spacelow > spacehigh)
                ফেরত spacelow;
            ফেরত spacehigh;
        }
        }
    }
    u32 আকার_2 = দৈর্ঘ্য * 16;
    যদি ((s32)আকার_2 <= 0)
        ফেরত 0;
    u32 align = MALLOC_MIN_ALIGN;
    যদি (flags & 4) {
        align = 1<<__ffs(আকার_2);
        যদি (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 তথ্য;
    নির্বাচন (flags & 3) {
    পূর্বনির্ধারিত:
    ক্ষেত্র 0:
        ফেরত 0;
    ক্ষেত্র 1:
        তথ্য = malloc_palloc(lowzone, আকার_2, align);
        বিরতি;
    ক্ষেত্র 2:
        তথ্য = malloc_palloc(highzone, আকার_2, align);
        যদি (!তথ্য && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            তথ্য = malloc_palloc(&Zone_Tmp_উপরের, আকার_2, align);
            যদি (তথ্য)
                e820_add(তথ্য, আকার_2, E820_RESERVED);
        }
        বিরতি;
    ক্ষেত্র 3: {
        তথ্য = malloc_palloc(lowzone, আকার_2, align);
        যদি (!তথ্য)
            তথ্য = malloc_palloc(highzone, আকার_2, align);
    }
    }
    যদি (তথ্য && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(তথ্য, handle);
    ফেরত তথ্য;
}

// PMM - find
স্থির u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    যদি (handle == MALLOC_DEFAULT_HANDLE)
        ফেরত 0;
    ফেরত malloc_findhandle(handle);
}

// PMM - deallocate
স্থির u32
handle_pmm02(u16 *args)
{
    u32 বাফার = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", বাফার);
    পূর্ণসংখ্যা ret = malloc_pfree(বাফার);
    যদি (ret)
        // Error
        ফেরত 1;
    ফেরত 0;
}

স্থির u32
handle_pmmXX(u16 *args)
{
    ফেরত PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    যদি (! CONFIG_PMM)
        ফেরত PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    নির্বাচন (arg1) {
    ক্ষেত্র 0x00: ret = handle_pmm00(args); বিরতি;
    ক্ষেত্র 0x01: ret = handle_pmm01(args); বিরতি;
    ক্ষেত্র 0x02: ret = handle_pmm02(args); বিরতি;
    পূর্বনির্ধারিত:   ret = handle_pmmXX(args); বিরতি;
    }

    ফেরত ret;
}

শূন্য
pmm_init(শূন্য)
{
    যদি (! CONFIG_PMM)
        ফেরত;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, আকার(PMMHEADER));
}

শূন্য
pmm_prepboot(শূন্য)
{
    যদি (! CONFIG_PMM)
        ফেরত;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
