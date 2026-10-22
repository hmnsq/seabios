// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // BUILD_BIOS_ADDR
#અન્તર્ભાવ "farptr.h" // SET_FARVAR
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "std/pnpbios.h" // PNP_SIGNATURE
#અન્તર્ભાવ "string.h" // checksum
#અન્તર્ભાવ "util.h" // pnp_init

બાહ્ય સંરચના pnpheader PNPHEADER;
બાહ્ય અક્ષર pnp_string[];

#જો CONFIG_PNPBIOS
સંરચના pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = કદ(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#નહીંતર
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
અક્ષર pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
સ્થિર u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    પાછા 0;
}

સ્થિર u16
handle_pnpXX(u16 *args)
{
    પાછા FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    જો (! CONFIG_PNPBIOS)
        પાછા FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    પસંદગી (arg1) {
    સ્થિતિ 0x60: પાછા handle_pnp60(args);
    મૂળભૂત:   પાછા handle_pnpXX(args);
    }
}

u16
get_pnp_offset(શૂન્ય)
{
    જો (! CONFIG_PNPBIOS)
        પાછા (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    પાછા (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
બાહ્ય શૂન્ય entry_pnp_real(શૂન્ય);
બાહ્ય શૂન્ય entry_pnp_prot(શૂન્ય);

શૂન્ય
pnp_init(શૂન્ય)
{
    જો (! CONFIG_PNPBIOS)
        પાછા;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, કદ(PNPHEADER));
}
