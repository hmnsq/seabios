// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // BUILD_BIOS_ADDR
#شامل "farptr.h" // SET_FARVAR
#شامل "output.h" // dprintf
#شامل "std/pnpbios.h" // PNP_SIGNATURE
#شامل "string.h" // checksum
#شامل "util.h" // pnp_init

بیرونی ساختار pnpheader PNPHEADER;
بیرونی نویسه pnp_string[];

#اگر CONFIG_PNPBIOS
ساختار pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = اندازه(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#وگرنه
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
نویسه pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
ایستا u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    بازگشت 0;
}

ایستا u16
handle_pnpXX(u16 *args)
{
    بازگشت FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    اگر (! CONFIG_PNPBIOS)
        بازگشت FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    انتخاب (arg1) {
    حالت 0x60: بازگشت handle_pnp60(args);
    پیشفرض:   بازگشت handle_pnpXX(args);
    }
}

u16
get_pnp_offset(تهی)
{
    اگر (! CONFIG_PNPBIOS)
        بازگشت (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    بازگشت (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
بیرونی تهی entry_pnp_real(تهی);
بیرونی تهی entry_pnp_prot(تهی);

تهی
pnp_init(تهی)
{
    اگر (! CONFIG_PNPBIOS)
        بازگشت;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, اندازه(PNPHEADER));
}
