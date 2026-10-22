// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // BUILD_BIOS_ADDR
#ଅନ୍ତର୍ଭାଵ "farptr.h" // SET_FARVAR
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "std/pnpbios.h" // PNP_SIGNATURE
#ଅନ୍ତର୍ଭାଵ "string.h" // checksum
#ଅନ୍ତର୍ଭାଵ "util.h" // pnp_init

ବାହ୍ୟ ସଂରଚନା pnpheader PNPHEADER;
ବାହ୍ୟ ଅକ୍ଷର pnp_string[];

#ଯଦି CONFIG_PNPBIOS
ସଂରଚନା pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = ଆକାର(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#ଅନ୍ୟଥା
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
ଅକ୍ଷର pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
ସ୍ଥିର u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u16
handle_pnpXX(u16 *args)
{
    ଫେରନ୍ତୁ FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    ଯଦି (! CONFIG_PNPBIOS)
        ଫେରନ୍ତୁ FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    ବାଛ (arg1) {
    ସ୍ଥିତି 0x60: ଫେରନ୍ତୁ handle_pnp60(args);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   ଫେରନ୍ତୁ handle_pnpXX(args);
    }
}

u16
get_pnp_offset(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PNPBIOS)
        ଫେରନ୍ତୁ (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    ଫେରନ୍ତୁ (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
ବାହ୍ୟ ଶୂନ୍ୟ entry_pnp_real(ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ entry_pnp_prot(ଶୂନ୍ୟ);

ଶୂନ୍ୟ
pnp_init(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PNPBIOS)
        ଫେରନ୍ତୁ;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, ଆକାର(PNPHEADER));
}
