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
    .نسخہ_2 = 0x01,
    .لمائی = حجم(PMMHEADER),
};
#جے_انت

// PMM - allocate
ساكن u32
handle_pmm00(u16 *دلیلاں)
{
    u32 لمائی = *(u32*)&دلیلاں[1], handle = *(u32*)&دلیلاں[3];
    u16 flags = دلیلاں[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , لمائی, handle, flags);
    بنية zone_s *lowzone = &Zone_Tmp_کم, *highzone = &Zone_Tmp_بلند;
    إذا (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_کم;
        highzone = &Zone_بلند;
    }
    إذا (!لمائی) {
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
    u32 ناپ = لمائی * 16;
    إذا ((s32)ناپ <= 0)
        أرجع 0;
    u32 align = MALLOC_MIN_ALIGN;
    إذا (flags & 4) {
        align = 1<<__ffs(ناپ);
        إذا (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 انکڑے;
    بدل (flags & 3) {
    افتراضي:
    حالة 0:
        أرجع 0;
    حالة 1:
        انکڑے = malloc_palloc(lowzone, ناپ, align);
        اكسر;
    حالة 2:
        انکڑے = malloc_palloc(highzone, ناپ, align);
        إذا (!انکڑے && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            انکڑے = malloc_palloc(&Zone_Tmp_بلند, ناپ, align);
            إذا (انکڑے)
                e820_جوڑو(انکڑے, ناپ, E820_RESERVED);
        }
        اكسر;
    حالة 3: {
        انکڑے = malloc_palloc(lowzone, ناپ, align);
        إذا (!انکڑے)
            انکڑے = malloc_palloc(highzone, ناپ, align);
    }
    }
    إذا (انکڑے && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(انکڑے, handle);
    أرجع انکڑے;
}

// PMM - find
ساكن u32
handle_pmm01(u16 *دلیلاں)
{
    u32 handle = *(u32*)&دلیلاں[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    إذا (handle == MALLOC_DEFAULT_HANDLE)
        أرجع 0;
    أرجع malloc_findhandle(handle);
}

// PMM - deallocate
ساكن u32
handle_pmm02(u16 *دلیلاں)
{
    u32 بفر = *(u32*)&دلیلاں[1];
    dprintf(3, "pmm02: buffer=%x\n", بفر);
    صحيح ret = malloc_pfree(بفر);
    إذا (ret)
        // Error
        أرجع 1;
    أرجع 0;
}

ساكن u32
handle_pmmXX(u16 *دلیلاں)
{
    أرجع PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *دلیلاں)
{
    ASSERT32FLAT();
    إذا (! CONFIG_PMM)
        أرجع PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = دلیلاں[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    بدل (arg1) {
    حالة 0x00: ret = handle_pmm00(دلیلاں); اكسر;
    حالة 0x01: ret = handle_pmm01(دلیلاں); اكسر;
    حالة 0x02: ret = handle_pmm02(دلیلاں); اكسر;
    افتراضي:   ret = handle_pmmXX(دلیلاں); اكسر;
    }

    أرجع ret;
}

فراغ
pmm_init(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "init PMM\n");

    PMMHEADER.اندراج = FUNC16(اندراج_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, حجم(PMMHEADER));
}

فراغ
pmm_prepboot(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.اندراج.segoff = 0;
}
