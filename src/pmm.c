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
    .نسخه_2 = 0x01,
    .درازی = حجم(PMMHEADER),
};
#پایان_اگر

// PMM - allocate
ساكن u32
handle_pmm00(u16 *آرگومانها)
{
    u32 درازی = *(u32*)&آرگومانها[1], handle = *(u32*)&آرگومانها[3];
    u16 flags = آرگومانها[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , درازی, handle, flags);
    بنية zone_s *lowzone = &Zone_Tmp_کم, *highzone = &Zone_Tmp_بالا;
    إذا (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_کم;
        highzone = &Zone_بالا;
    }
    إذا (!درازی) {
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
    u32 اندازه = درازی * 16;
    إذا ((s32)اندازه <= 0)
        أرجع 0;
    u32 align = MALLOC_MIN_ALIGN;
    إذا (flags & 4) {
        align = 1<<__ffs(اندازه);
        إذا (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 داده;
    بدل (flags & 3) {
    افتراضي:
    حالة 0:
        أرجع 0;
    حالة 1:
        داده = malloc_palloc(lowzone, اندازه, align);
        اكسر;
    حالة 2:
        داده = malloc_palloc(highzone, اندازه, align);
        إذا (!داده && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            داده = malloc_palloc(&Zone_Tmp_بالا, اندازه, align);
            إذا (داده)
                e820_جمع_کن(داده, اندازه, E820_RESERVED);
        }
        اكسر;
    حالة 3: {
        داده = malloc_palloc(lowzone, اندازه, align);
        إذا (!داده)
            داده = malloc_palloc(highzone, اندازه, align);
    }
    }
    إذا (داده && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(داده, handle);
    أرجع داده;
}

// PMM - find
ساكن u32
handle_pmm01(u16 *آرگومانها)
{
    u32 handle = *(u32*)&آرگومانها[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    إذا (handle == MALLOC_DEFAULT_HANDLE)
        أرجع 0;
    أرجع malloc_findhandle(handle);
}

// PMM - deallocate
ساكن u32
handle_pmm02(u16 *آرگومانها)
{
    u32 میانگیر = *(u32*)&آرگومانها[1];
    dprintf(3, "pmm02: buffer=%x\n", میانگیر);
    صحيح ret = malloc_pfree(میانگیر);
    إذا (ret)
        // Error
        أرجع 1;
    أرجع 0;
}

ساكن u32
handle_pmmXX(u16 *آرگومانها)
{
    أرجع PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *آرگومانها)
{
    ASSERT32FLAT();
    إذا (! CONFIG_PMM)
        أرجع PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = آرگومانها[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    بدل (arg1) {
    حالة 0x00: ret = handle_pmm00(آرگومانها); اكسر;
    حالة 0x01: ret = handle_pmm01(آرگومانها); اكسر;
    حالة 0x02: ret = handle_pmm02(آرگومانها); اكسر;
    افتراضي:   ret = handle_pmmXX(آرگومانها); اكسر;
    }

    أرجع ret;
}

فراغ
pmm_init(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "init PMM\n");

    PMMHEADER.مدخل = FUNC16(مدخل_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, حجم(PMMHEADER));
}

فراغ
pmm_prepboot(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.مدخل.segoff = 0;
}
