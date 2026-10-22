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

خارجی ساخت pmmheader PMMHEADER;

#اگر CONFIG_PMM
ساخت pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .نسخہ_2 = 0x01,
    .لمبائی = حجم(PMMHEADER),
};
#اگر_ختم

// PMM - allocate
ساکن u32
handle_pmm00(u16 *دلائل)
{
    u32 لمبائی = *(u32*)&دلائل[1], handle = *(u32*)&دلائل[3];
    u16 flags = دلائل[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , لمبائی, handle, flags);
    ساخت zone_s *lowzone = &Zone_Tmp_کم, *highzone = &Zone_Tmp_بلند;
    اگر (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_کم;
        highzone = &Zone_بلند;
    }
    اگر (!لمبائی) {
        // Memory size request
        انتخاب (flags & 3) {
        طےشدہ:
        حالت 0:
            واپس 0;
        حالت 1:
            واپس malloc_getspace(lowzone);
        حالت 2:
            واپس malloc_getspace(highzone);
        حالت 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            اگر (spacelow > spacehigh)
                واپس spacelow;
            واپس spacehigh;
        }
        }
    }
    u32 سائز = لمبائی * 16;
    اگر ((s32)سائز <= 0)
        واپس 0;
    u32 align = MALLOC_MIN_ALIGN;
    اگر (flags & 4) {
        align = 1<<__ffs(سائز);
        اگر (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 ڈیٹا;
    انتخاب (flags & 3) {
    طےشدہ:
    حالت 0:
        واپس 0;
    حالت 1:
        ڈیٹا = malloc_palloc(lowzone, سائز, align);
        توڑو;
    حالت 2:
        ڈیٹا = malloc_palloc(highzone, سائز, align);
        اگر (!ڈیٹا && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            ڈیٹا = malloc_palloc(&Zone_Tmp_بلند, سائز, align);
            اگر (ڈیٹا)
                e820_جوڑو(ڈیٹا, سائز, E820_RESERVED);
        }
        توڑو;
    حالت 3: {
        ڈیٹا = malloc_palloc(lowzone, سائز, align);
        اگر (!ڈیٹا)
            ڈیٹا = malloc_palloc(highzone, سائز, align);
    }
    }
    اگر (ڈیٹا && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(ڈیٹا, handle);
    واپس ڈیٹا;
}

// PMM - find
ساکن u32
handle_pmm01(u16 *دلائل)
{
    u32 handle = *(u32*)&دلائل[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    اگر (handle == MALLOC_DEFAULT_HANDLE)
        واپس 0;
    واپس malloc_findhandle(handle);
}

// PMM - deallocate
ساکن u32
handle_pmm02(u16 *دلائل)
{
    u32 بفر = *(u32*)&دلائل[1];
    dprintf(3, "pmm02: buffer=%x\n", بفر);
    صحیح ret = malloc_pfree(بفر);
    اگر (ret)
        // Error
        واپس 1;
    واپس 0;
}

ساکن u32
handle_pmmXX(u16 *دلائل)
{
    واپس PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *دلائل)
{
    ASSERT32FLAT();
    اگر (! CONFIG_PMM)
        واپس PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = دلائل[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    انتخاب (arg1) {
    حالت 0x00: ret = handle_pmm00(دلائل); توڑو;
    حالت 0x01: ret = handle_pmm01(دلائل); توڑو;
    حالت 0x02: ret = handle_pmm02(دلائل); توڑو;
    طےشدہ:   ret = handle_pmmXX(دلائل); توڑو;
    }

    واپس ret;
}

خالی
pmm_init(خالی)
{
    اگر (! CONFIG_PMM)
        واپس;

    dprintf(3, "init PMM\n");

    PMMHEADER.اندراج_2 = FUNC16(اندراج_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, حجم(PMMHEADER));
}

خالی
pmm_prepboot(خالی)
{
    اگر (! CONFIG_PMM)
        واپس;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.اندراج_2.segoff = 0;
}
