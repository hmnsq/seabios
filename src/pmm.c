// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // FUNC16
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // _malloc
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "e820map.h" // struct e820entry
#அந்தர்பாவ "std/pmm.h" // PMM_SIGNATURE
#அந்தர்பாவ "string.h" // checksum
#அந்தர்பாவ "util.h" // pmm_init
#அந்தர்பாவ "x86.h" // __ffs

வெளி கட்டமைப்பு pmmheader PMMHEADER;

#எனில் CONFIG_PMM
கட்டமைப்பு pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .length = அளவு(PMMHEADER),
};
#endif

// PMM - allocate
நிலையான u32
handle_pmm00(u16 *args)
{
    u32 length = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , length, handle, flags);
    கட்டமைப்பு zone_s *lowzone = &Zone_Tmp_கீழ், *highzone = &Zone_Tmp_உயர்வு;
    எனில் (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_கீழ்;
        highzone = &Zone_உயர்வு;
    }
    எனில் (!length) {
        // Memory size request
        தேர்வு (flags & 3) {
        இயல்பு:
        நிலை 0:
            திரும்பு 0;
        நிலை 1:
            திரும்பு malloc_getspace(lowzone);
        நிலை 2:
            திரும்பு malloc_getspace(highzone);
        நிலை 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            எனில் (spacelow > spacehigh)
                திரும்பு spacelow;
            திரும்பு spacehigh;
        }
        }
    }
    u32 size = length * 16;
    எனில் ((s32)size <= 0)
        திரும்பு 0;
    u32 align = MALLOC_MIN_ALIGN;
    எனில் (flags & 4) {
        align = 1<<__ffs(size);
        எனில் (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 data;
    தேர்வு (flags & 3) {
    இயல்பு:
    நிலை 0:
        திரும்பு 0;
    நிலை 1:
        data = malloc_palloc(lowzone, size, align);
        நிறுத்து;
    நிலை 2:
        data = malloc_palloc(highzone, size, align);
        எனில் (!data && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            data = malloc_palloc(&Zone_Tmp_உயர்வு, size, align);
            எனில் (data)
                e820_add(data, size, E820_RESERVED);
        }
        நிறுத்து;
    நிலை 3: {
        data = malloc_palloc(lowzone, size, align);
        எனில் (!data)
            data = malloc_palloc(highzone, size, align);
    }
    }
    எனில் (data && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(data, handle);
    திரும்பு data;
}

// PMM - find
நிலையான u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    எனில் (handle == MALLOC_DEFAULT_HANDLE)
        திரும்பு 0;
    திரும்பு malloc_findhandle(handle);
}

// PMM - deallocate
நிலையான u32
handle_pmm02(u16 *args)
{
    u32 buffer = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", buffer);
    முழு ret = malloc_pfree(buffer);
    எனில் (ret)
        // Error
        திரும்பு 1;
    திரும்பு 0;
}

நிலையான u32
handle_pmmXX(u16 *args)
{
    திரும்பு PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_PMM)
        திரும்பு PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    தேர்வு (arg1) {
    நிலை 0x00: ret = handle_pmm00(args); நிறுத்து;
    நிலை 0x01: ret = handle_pmm01(args); நிறுத்து;
    நிலை 0x02: ret = handle_pmm02(args); நிறுத்து;
    இயல்பு:   ret = handle_pmmXX(args); நிறுத்து;
    }

    திரும்பு ret;
}

வெறுமை
pmm_init(வெறுமை)
{
    எனில் (! CONFIG_PMM)
        திரும்பு;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, அளவு(PMMHEADER));
}

வெறுமை
pmm_prepboot(வெறுமை)
{
    எனில் (! CONFIG_PMM)
        திரும்பு;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
