// Post memory manager (PMM) calls
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // FUNC16
#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // _malloc
#समावेश "output.h" // dprintf
#समावेश "e820map.h" // struct e820entry
#समावेश "std/pmm.h" // PMM_SIGNATURE
#समावेश "string.h" // checksum
#समावेश "util.h" // pmm_init
#समावेश "x86.h" // __ffs

बाह्य काष्ठा pmmheader PMMHEADER;

#अगर CONFIG_PMM
काष्ठा pmmheader PMMHEADER __aligned(16) VARFSEG = {
    .signature = PMM_SIGNATURE,
    .version = 0x01,
    .लंबाई = माप(PMMHEADER),
};
#पूर्ण_अगर

// PMM - allocate
अटल u32
handle_pmm00(u16 *args)
{
    u32 लंबाई = *(u32*)&args[1], handle = *(u32*)&args[3];
    u16 flags = args[5];
    dprintf(3, "pmm00: length=%x handle=%x flags=%x\n"
            , लंबाई, handle, flags);
    काष्ठा zone_s *lowzone = &Zone_Tmp_निचला, *highzone = &Zone_Tmp_ऊपरी;
    अगर (flags & 8) {
        // Permanent memory request.
        lowzone = &Zone_निचला;
        highzone = &Zone_ऊपरी;
    }
    अगर (!लंबाई) {
        // Memory size request
        चयन (flags & 3) {
        शेष:
        हाल 0:
            वापस 0;
        हाल 1:
            वापस malloc_getspace(lowzone);
        हाल 2:
            वापस malloc_getspace(highzone);
        हाल 3: {
            u32 spacelow = malloc_getspace(lowzone);
            u32 spacehigh = malloc_getspace(highzone);
            अगर (spacelow > spacehigh)
                वापस spacelow;
            वापस spacehigh;
        }
        }
    }
    u32 आकार = लंबाई * 16;
    अगर ((s32)आकार <= 0)
        वापस 0;
    u32 align = MALLOC_MIN_ALIGN;
    अगर (flags & 4) {
        align = 1<<__ffs(आकार);
        अगर (align < MALLOC_MIN_ALIGN)
            align = MALLOC_MIN_ALIGN;
    }
    u32 तथ्य;
    चयन (flags & 3) {
    शेष:
    हाल 0:
        वापस 0;
    हाल 1:
        तथ्य = malloc_palloc(lowzone, आकार, align);
        अवरोध;
    हाल 2:
        तथ्य = malloc_palloc(highzone, आकार, align);
        अगर (!तथ्य && (flags & 8)) {
            /*
             * We are out of meory.  So go allocate from the (big)
             * ZoneTmpHigh instead and reserve the block in the e820
             * map so the OS will not override it.  That way we can
             * handle big permanent allocations without needing a big
             * ZoneHigh.
             */
            तथ्य = malloc_palloc(&Zone_Tmp_ऊपरी, आकार, align);
            अगर (तथ्य)
                e820_add(तथ्य, आकार, E820_RESERVED);
        }
        अवरोध;
    हाल 3: {
        तथ्य = malloc_palloc(lowzone, आकार, align);
        अगर (!तथ्य)
            तथ्य = malloc_palloc(highzone, आकार, align);
    }
    }
    अगर (तथ्य && handle != MALLOC_DEFAULT_HANDLE)
        malloc_sethandle(तथ्य, handle);
    वापस तथ्य;
}

// PMM - find
अटल u32
handle_pmm01(u16 *args)
{
    u32 handle = *(u32*)&args[1];
    dprintf(3, "pmm01: handle=%x\n", handle);
    अगर (handle == MALLOC_DEFAULT_HANDLE)
        वापस 0;
    वापस malloc_findhandle(handle);
}

// PMM - deallocate
अटल u32
handle_pmm02(u16 *args)
{
    u32 बफर = *(u32*)&args[1];
    dprintf(3, "pmm02: buffer=%x\n", बफर);
    पूर्णांक ret = malloc_pfree(बफर);
    अगर (ret)
        // Error
        वापस 1;
    वापस 0;
}

अटल u32
handle_pmmXX(u16 *args)
{
    वापस PMM_FUNCTION_NOT_SUPPORTED;
}

u32 VISIBLE32INIT
handle_pmm(u16 *args)
{
    ASSERT32FLAT();
    अगर (! CONFIG_PMM)
        वापस PMM_FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pmm, "pmm call arg1=%x\n", arg1);

    u32 ret;
    चयन (arg1) {
    हाल 0x00: ret = handle_pmm00(args); अवरोध;
    हाल 0x01: ret = handle_pmm01(args); अवरोध;
    हाल 0x02: ret = handle_pmm02(args); अवरोध;
    शेष:   ret = handle_pmmXX(args); अवरोध;
    }

    वापस ret;
}

व्योम
pmm_init(व्योम)
{
    अगर (! CONFIG_PMM)
        वापस;

    dprintf(3, "init PMM\n");

    PMMHEADER.entry = FUNC16(entry_pmm);
    PMMHEADER.checksum -= checksum(&PMMHEADER, माप(PMMHEADER));
}

व्योम
pmm_prepboot(व्योम)
{
    अगर (! CONFIG_PMM)
        वापस;

    dprintf(3, "finalize PMM\n");

    PMMHEADER.signature = 0;
    PMMHEADER.entry.segoff = 0;
}
