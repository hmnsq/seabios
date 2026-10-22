// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // FUNC16
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // _malloc
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "e820map.h" // struct e820entry
#ଅନ୍ତର୍ଭାଵ "std/pmm.h" // PMM_SIGNATURE
#ଅନ୍ତର୍ଭାଵ "string.h" // checksum
#ଅନ୍ତର୍ଭାଵ "util.h" // pmm_init
#ଅନ୍ତର୍ଭାଵ "x86.h" // __ffs

ବାହ୍ୟ ସଂରଚନା pmmheader PMMHEADER;

#ଯଦି CONFIG_PMM
ସଂରଚନା pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = ଆକାର(PMMHEADER),
};
#endif

// PMM - allocate
ସ୍ଥିର u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    ସଂରଚନା zone_s *lowzone = &Zone_Tmp_ତଳ, *highzone = &Zone_Tmp_ଉପର;
    ଯଦି (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_ତଳ;
        highzone = &Zone_ଉପର;
    }
    ଯଦି (!length) {
        // Memory size request
        ବାଛ (flags & 3) {
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ସ୍ଥିତି 0:
            ଫେରନ୍ତୁ 0;
        ସ୍ଥିତି 1:
            ଫେରନ୍ତୁ malloc_getspace(lowzone);
        ସ୍ଥିତି 2:
            ଫେରନ୍ତୁ malloc_getspace(highzone);
        ସ୍ଥିତି 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            ଯଦି (spacelow > spacehigh)
                ଫେରନ୍ତୁ spacelow;
            ଫେରନ୍ତୁ spacehigh;
        }
        }
    }
    u32 size = length * 16;
    ଯଦି ((s32)size <= 0)
        ଫେରନ୍ତୁ 0;
    u32 align = MALLOC_MIN_ALIGN;
    ଯଦି (flags & 4) {
        align = 1<<__ffs(size);
        ଯଦି (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    ବାଛ (flags & 3) {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ସ୍ଥିତି 0:
        ଫେରନ୍ତୁ 0;
    ସ୍ଥିତି 1:
        data = malloc_palloc(lowzone, size, align);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 2:
        data = malloc_palloc(highzone, size, align);
        ଯଦି (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_ଉପର, size, align);
            ଯଦି (data)
                e820_add(data, size, E820_RESERVED);
        }
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 3: {
        data = malloc_palloc(lowzone, size, align);
        ଯଦି (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    ଯଦି (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    ଫେରନ୍ତୁ data;
}

// PMM - find
ସ୍ଥିର u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    ଯଦି (handle == MALLOC_DEFAULT_HANDLE)
        ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ malloc_findhandle(handle);
}

// PMM - deallocate
ସ୍ଥିର u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = malloc_pfree(buffer);
    ଯଦି (ret)
        // Error
        ଫେରନ୍ତୁ 1;
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32
handle_pmmXX(u16 *args)
{
    ଫେରନ୍ତୁ PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_PMM)
        ଫେରନ୍ତୁ PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    ବାଛ (arg1) {
    ସ୍ଥିତି 0x00: ret = handle_pmm00(args); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: ret = handle_pmm01(args); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: ret = handle_pmm02(args); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   ret = handle_pmmXX(args); ଭାଙ୍ଗ;
    }

    ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ
pmm_init(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PMM)
        ଫେରନ୍ତୁ;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, ଆକାର(PMMHEADER));
}

ଶୂନ୍ୟ
pmm_prepboot(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PMM)
        ଫେରନ୍ତୁ;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
