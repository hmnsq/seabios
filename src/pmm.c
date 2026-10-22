// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // FUNC16
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // _malloc
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "e820map.h" // struct e820entry
#అన్తర్భావ "std/pmm.h" // PMM_SIGNATURE
#అన్తర్భావ "string.h" // checksum
#అన్తర్భావ "util.h" // pmm_init
#అన్తర్భావ "x86.h" // __ffs

బాహ్య నిర్మాణం pmmheader PMMHEADER;

#ఒకవేళ CONFIG_PMM
నిర్మాణం pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .సంస్కరణ_2 = 0x01,
    .నిడివి = పరిమాణం(PMMHEADER),
};
#అయితే_ముగింపు

// PMM - allocate
స్థిర u32
handle_pmm00(u16 *వాదనలు)
{
    u32 నిడివి = *(u32*)&వాదనలు[1], handle = *(u32*)&వాదనలు[3];
    u16 flags = వాదనలు[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , నిడివి, handle, flags);
    నిర్మాణం zone_s *lowzone = &Zone_Tmp_దిగువ, *highzone = &Zone_Tmp_ఎగువ;
    ఒకవేళ (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_దిగువ;
        highzone = &Zone_ఎగువ;
    }
    ఒకవేళ (!నిడివి) {
        // Memory size request
        ఎంపిక (flags & 3) {
        అప్రమేయ:
        స్థితి 0:
            తిరిగి 0;
        స్థితి 1:
            తిరిగి malloc_getspace(lowzone);
        స్థితి 2:
            తిరిగి malloc_getspace(highzone);
        స్థితి 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            ఒకవేళ (spacelow > spacehigh)
                తిరిగి spacelow;
            తిరిగి spacehigh;
        }
        }
    }
    u32 కొలత = నిడివి * 16;
    ఒకవేళ ((s32)కొలత <= 0)
        తిరిగి 0;
    u32 align = MALLOC_MIN_ALIGN;
    ఒకవేళ (flags & 4) {
        align = 1<<__ffs(కొలత);
        ఒకవేళ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 దత్తాంశం;
    ఎంపిక (flags & 3) {
    అప్రమేయ:
    స్థితి 0:
        తిరిగి 0;
    స్థితి 1:
        దత్తాంశం = malloc_palloc(lowzone, కొలత, align);
        ఆపు;
    స్థితి 2:
        దత్తాంశం = malloc_palloc(highzone, కొలత, align);
        ఒకవేళ (!దత్తాంశం && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            దత్తాంశం = malloc_palloc(&Zone_Tmp_ఎగువ, కొలత, align);
            ఒకవేళ (దత్తాంశం)
                e820_కూడు(దత్తాంశం, కొలత, E820_RESERVED);
        }
        ఆపు;
    స్థితి 3: {
        దత్తాంశం = malloc_palloc(lowzone, కొలత, align);
        ఒకవేళ (!దత్తాంశం)
            దత్తాంశం = malloc_palloc(highzone, కొలత, align);
    }
    }
    ఒకవేళ (దత్తాంశం && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(దత్తాంశం, handle);
    తిరిగి దత్తాంశం;
}

// PMM - find
స్థిర u32
handle_pmm01(u16 *వాదనలు)
{
    u32 handle = *(u32*)&వాదనలు[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    ఒకవేళ (handle == MALLOC_DEFAULT_HANDLE)
        తిరిగి 0;
    తిరిగి malloc_findhandle(handle);
}

// PMM - deallocate
స్థిర u32
handle_pmm02(u16 *వాదనలు)
{
    u32 బఫరు = *(u32*)&వాదనలు[1];
    dprintf(3, "pmm02: buffer=%x\n", బఫరు);
    పూర్ణాంకం ret = malloc_pfree(బఫరు);
    ఒకవేళ (ret)
        // Error
        తిరిగి 1;
    తిరిగి 0;
}

స్థిర u32
handle_pmmXX(u16 *వాదనలు)
{
    తిరిగి PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *వాదనలు)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_PMM)
        తిరిగి PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = వాదనలు[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    ఎంపిక (arg1) {
    స్థితి 0x00: ret = handle_pmm00(వాదనలు); ఆపు;
    స్థితి 0x01: ret = handle_pmm01(వాదనలు); ఆపు;
    స్థితి 0x02: ret = handle_pmm02(వాదనలు); ఆపు;
    అప్రమేయ:   ret = handle_pmmXX(వాదనలు); ఆపు;
    }

    తిరిగి ret;
}

శూన్య
pmm_init(శూన్య)
{
    ఒకవేళ (! CONFIG_PMM)
        తిరిగి;

    dprintf(3, "init PMM\n");

    PMMHEADER.నమోదు_2 = FUNC16(నమోదు_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, పరిమాణం(PMMHEADER));
}

శూన్య
pmm_prepboot(శూన్య)
{
    ఒకవేళ (! CONFIG_PMM)
        తిరిగి;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.నమోదు_2.segoff = 0;
}
