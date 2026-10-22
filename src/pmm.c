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
    .نسخو_2 = 0x01,
    .ڊيگهائي = حجم(PMMHEADER),
};
#جيڪڏهن_پورو

// PMM - allocate
ساكن u32
handle_pmm00(u16 *دليلون)
{
    u32 ڊيگهائي = *(u32*)&دليلون[1], handle = *(u32*)&دليلون[3];
    u16 flags = دليلون[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , ڊيگهائي, handle, flags);
    بنية zone_s *lowzone = &Zone_Tmp_هيٺيون, *highzone = &Zone_Tmp_مٿاهون;
    إذا (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_هيٺيون;
        highzone = &Zone_مٿاهون;
    }
    إذا (!ڊيگهائي) {
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
    u32 سائيز = ڊيگهائي * 16;
    إذا ((s32)سائيز <= 0)
        أرجع 0;
    u32 align = MALLOC_MIN_ALIGN;
    إذا (flags & 4) {
        align = 1<<__ffs(سائيز);
        إذا (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 معلومات;
    بدل (flags & 3) {
    افتراضي:
    حالة 0:
        أرجع 0;
    حالة 1:
        معلومات = malloc_palloc(lowzone, سائيز, align);
        اكسر;
    حالة 2:
        معلومات = malloc_palloc(highzone, سائيز, align);
        إذا (!معلومات && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            معلومات = malloc_palloc(&Zone_Tmp_مٿاهون, سائيز, align);
            إذا (معلومات)
                e820_جوڙيو(معلومات, سائيز, E820_RESERVED);
        }
        اكسر;
    حالة 3: {
        معلومات = malloc_palloc(lowzone, سائيز, align);
        إذا (!معلومات)
            معلومات = malloc_palloc(highzone, سائيز, align);
    }
    }
    إذا (معلومات && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(معلومات, handle);
    أرجع معلومات;
}

// PMM - find
ساكن u32
handle_pmm01(u16 *دليلون)
{
    u32 handle = *(u32*)&دليلون[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    إذا (handle == MALLOC_DEFAULT_HANDLE)
        أرجع 0;
    أرجع malloc_findhandle(handle);
}

// PMM - deallocate
ساكن u32
handle_pmm02(u16 *دليلون)
{
    u32 بفر = *(u32*)&دليلون[1];
    dprintf(3, "pmm02: buffer=%x\n", بفر);
    صحيح ret = malloc_pfree(بفر);
    إذا (ret)
        // Error
        أرجع 1;
    أرجع 0;
}

ساكن u32
handle_pmmXX(u16 *دليلون)
{
    أرجع PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *دليلون)
{
    ASSERT32FLAT();
    إذا (! CONFIG_PMM)
        أرجع PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = دليلون[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    بدل (arg1) {
    حالة 0x00: ret = handle_pmm00(دليلون); اكسر;
    حالة 0x01: ret = handle_pmm01(دليلون); اكسر;
    حالة 0x02: ret = handle_pmm02(دليلون); اكسر;
    افتراضي:   ret = handle_pmmXX(دليلون); اكسر;
    }

    أرجع ret;
}

فراغ
pmm_init(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "init PMM\n");

    PMMHEADER.داخلا = FUNC16(داخلا_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, حجم(PMMHEADER));
}

فراغ
pmm_prepboot(فراغ)
{
    إذا (! CONFIG_PMM)
        أرجع;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.داخلا.segoff = 0;
}
