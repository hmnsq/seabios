// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // FUNC16
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // _malloc
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "e820map.h" // struct e820entry
#അന്തര്ഭാവ "std/pmm.h" // PMM_SIGNATURE
#അന്തര്ഭാവ "string.h" // checksum
#അന്തര്ഭാവ "util.h" // pmm_init
#അന്തര്ഭാവ "x86.h" // __ffs

ബാഹ്യ ഘടന pmmheader PMMHEADER;

#എങ്കിൽ CONFIG_PMM
ഘടന pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = വലുപ്പം(PMMHEADER),
};
#endif

// PMM - allocate
സ്ഥിതം u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    ഘടന zone_s *lowzone = &Zone_Tmp_താഴെയുള്ള, *highzone = &Zone_Tmp_മുകളിലെ;
    എങ്കിൽ (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_താഴെയുള്ള;
        highzone = &Zone_മുകളിലെ;
    }
    എങ്കിൽ (!length) {
        // Memory size request
        തിരഞ്ഞെടുപ്പ് (flags & 3) {
        സ്ഥിരസ്ഥിതി:
        അവസ്ഥ 0:
            തിരികെ 0;
        അവസ്ഥ 1:
            തിരികെ malloc_getspace(lowzone);
        അവസ്ഥ 2:
            തിരികെ malloc_getspace(highzone);
        അവസ്ഥ 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            എങ്കിൽ (spacelow > spacehigh)
                തിരികെ spacelow;
            തിരികെ spacehigh;
        }
        }
    }
    u32 size = length * 16;
    എങ്കിൽ ((s32)size <= 0)
        തിരികെ 0;
    u32 align = MALLOC_MIN_ALIGN;
    എങ്കിൽ (flags & 4) {
        align = 1<<__ffs(size);
        എങ്കിൽ (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    തിരഞ്ഞെടുപ്പ് (flags & 3) {
    സ്ഥിരസ്ഥിതി:
    അവസ്ഥ 0:
        തിരികെ 0;
    അവസ്ഥ 1:
        data = malloc_palloc(lowzone, size, align);
        നിർത്തുക;
    അവസ്ഥ 2:
        data = malloc_palloc(highzone, size, align);
        എങ്കിൽ (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_മുകളിലെ, size, align);
            എങ്കിൽ (data)
                e820_add(data, size, E820_RESERVED);
        }
        നിർത്തുക;
    അവസ്ഥ 3: {
        data = malloc_palloc(lowzone, size, align);
        എങ്കിൽ (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    എങ്കിൽ (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    തിരികെ data;
}

// PMM - find
സ്ഥിതം u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    എങ്കിൽ (handle == MALLOC_DEFAULT_HANDLE)
        തിരികെ 0;
    തിരികെ malloc_findhandle(handle);
}

// PMM - deallocate
സ്ഥിതം u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    പൂർണ്ണസംഖ്യ ret = malloc_pfree(buffer);
    എങ്കിൽ (ret)
        // Error
        തിരികെ 1;
    തിരികെ 0;
}

സ്ഥിതം u32
handle_pmmXX(u16 *args)
{
    തിരികെ PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_PMM)
        തിരികെ PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    തിരഞ്ഞെടുപ്പ് (arg1) {
    അവസ്ഥ 0x00: ret = handle_pmm00(args); നിർത്തുക;
    അവസ്ഥ 0x01: ret = handle_pmm01(args); നിർത്തുക;
    അവസ്ഥ 0x02: ret = handle_pmm02(args); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   ret = handle_pmmXX(args); നിർത്തുക;
    }

    തിരികെ ret;
}

ശൂന്യം
pmm_init(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PMM)
        തിരികെ;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, വലുപ്പം(PMMHEADER));
}

ശൂന്യം
pmm_prepboot(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PMM)
        തിരികെ;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
