// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // BUILD_BIOS_ADDR
#অন্তর্ভাব "farptr.h" // SET_FARVAR
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "std/pnpbios.h" // PNP_SIGNATURE
#অন্তর্ভাব "string.h" // checksum
#অন্তর্ভাব "util.h" // pnp_init

বাহ্যিক কাঠামো pnpheader PNPHEADER;
বাহ্যিক অক্ষর pnp_string[];

#যদি CONFIG_PNPBIOS
কাঠামো pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .দৈর্ঘ্য = আকার(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#অন্যথা
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
অক্ষর pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
স্থির u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    ফেরত 0;
}

স্থির u16
handle_pnpXX(u16 *args)
{
    ফেরত FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    যদি (! CONFIG_PNPBIOS)
        ফেরত FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    নির্বাচন (arg1) {
    ক্ষেত্র 0x60: ফেরত handle_pnp60(args);
    পূর্বনির্ধারিত:   ফেরত handle_pnpXX(args);
    }
}

u16
get_pnp_offset(শূন্য)
{
    যদি (! CONFIG_PNPBIOS)
        ফেরত (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    ফেরত (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
বাহ্যিক শূন্য entry_pnp_real(শূন্য);
বাহ্যিক শূন্য entry_pnp_prot(শূন্য);

শূন্য
pnp_init(শূন্য)
{
    যদি (! CONFIG_PNPBIOS)
        ফেরত;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, আকার(PNPHEADER));
}
