// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // FUNC16
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // _malloc
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "e820map.h" // struct e820entry
#अन्तर्भाव "std/pmm.h" // PMM_SIGNATURE
#अन्तर्भाव "string.h" // checksum
#अन्तर्भाव "util.h" // pmm_init
#अन्तर्भाव "x86.h" // __ffs

बाह्य संरचना pmmheader PMMHEADER;

#जर CONFIG_PMM
संरचना pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = आकार(PMMHEADER),
};
#endif

// PMM - allocate
स्थिर u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    संरचना zone_s *lowzone = &Zone_Tmp_खालचा, *highzone = &Zone_Tmp_वरचा;
    जर (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_खालचा;
        highzone = &Zone_वरचा;
    }
    जर (!length) {
        // Memory size request
        निवड (flags & 3) {
        पूर्वनिर्धारित:
        स्थिती 0:
            परत 0;
        स्थिती 1:
            परत malloc_getspace(lowzone);
        स्थिती 2:
            परत malloc_getspace(highzone);
        स्थिती 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            जर (spacelow > spacehigh)
                परत spacelow;
            परत spacehigh;
        }
        }
    }
    u32 size = length * 16;
    जर ((s32)size <= 0)
        परत 0;
    u32 align = MALLOC_MIN_ALIGN;
    जर (flags & 4) {
        align = 1<<__ffs(size);
        जर (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    निवड (flags & 3) {
    पूर्वनिर्धारित:
    स्थिती 0:
        परत 0;
    स्थिती 1:
        data = malloc_palloc(lowzone, size, align);
        थांबा;
    स्थिती 2:
        data = malloc_palloc(highzone, size, align);
        जर (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_वरचा, size, align);
            जर (data)
                e820_add(data, size, E820_RESERVED);
        }
        थांबा;
    स्थिती 3: {
        data = malloc_palloc(lowzone, size, align);
        जर (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    जर (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    परत data;
}

// PMM - find
स्थिर u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    जर (handle == MALLOC_DEFAULT_HANDLE)
        परत 0;
    परत malloc_findhandle(handle);
}

// PMM - deallocate
स्थिर u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    पूर्णांक ret = malloc_pfree(buffer);
    जर (ret)
        // Error
        परत 1;
    परत 0;
}

स्थिर u32
handle_pmmXX(u16 *args)
{
    परत PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    जर (! CONFIG_PMM)
        परत PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    निवड (arg1) {
    स्थिती 0x00: ret = handle_pmm00(args); थांबा;
    स्थिती 0x01: ret = handle_pmm01(args); थांबा;
    स्थिती 0x02: ret = handle_pmm02(args); थांबा;
    पूर्वनिर्धारित:   ret = handle_pmmXX(args); थांबा;
    }

    परत ret;
}

शून्य
pmm_init(शून्य)
{
    जर (! CONFIG_PMM)
        परत;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, आकार(PMMHEADER));
}

शून्य
pmm_prepboot(शून्य)
{
    जर (! CONFIG_PMM)
        परत;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
