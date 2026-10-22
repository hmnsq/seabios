// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // BUILD_BIOS_ADDR
#অন্তর্ভাৱ "farptr.h" // SET_FARVAR
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "std/pnpbios.h" // PNP_SIGNATURE
#অন্তর্ভাৱ "string.h" // checksum
#অন্তর্ভাৱ "util.h" // pnp_init

বাহ্য সংরচনা pnpheader PNPHEADER;
বাহ্য ৱর্ণ pnp_string[];

#যদি CONFIG_PNPBIOS
সংরচনা pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = পরিমাণ(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#অন্যথা
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
ৱর্ণ pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
স্থির u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    ঘূৰাই 0;
}

স্থির u16
handle_pnpXX(u16 *args)
{
    ঘূৰাই FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    যদি (! CONFIG_PNPBIOS)
        ঘূৰাই FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    ৱিকল্প (arg1) {
    পক্ষ 0x60: ঘূৰাই handle_pnp60(args);
    স্ৱভাৱ:   ঘূৰাই handle_pnpXX(args);
    }
}

u16
get_pnp_offset(শূন্য)
{
    যদি (! CONFIG_PNPBIOS)
        ঘূৰাই (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    ঘূৰাই (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
বাহ্য শূন্য entry_pnp_real(শূন্য);
বাহ্য শূন্য entry_pnp_prot(শূন্য);

শূন্য
pnp_init(শূন্য)
{
    যদি (! CONFIG_PNPBIOS)
        ঘূৰাই;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, পরিমাণ(PNPHEADER));
}
