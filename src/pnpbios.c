// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // BUILD_BIOS_ADDR
#അന്തര്ഭാവ "farptr.h" // SET_FARVAR
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "std/pnpbios.h" // PNP_SIGNATURE
#അന്തര്ഭാവ "string.h" // checksum
#അന്തര്ഭാവ "util.h" // pnp_init

ബാഹ്യ ഘടന pnpheader PNPHEADER;
ബാഹ്യ അക്ഷരം pnp_string[];

#എങ്കിൽ CONFIG_PNPBIOS
ഘടന pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = വലുപ്പം(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#അല്ലെങ്കിൽ
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
അക്ഷരം pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
സ്ഥിതം u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    തിരികെ 0;
}

സ്ഥിതം u16
handle_pnpXX(u16 *args)
{
    തിരികെ FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    എങ്കിൽ (! CONFIG_PNPBIOS)
        തിരികെ FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    തിരഞ്ഞെടുപ്പ് (arg1) {
    അവസ്ഥ 0x60: തിരികെ handle_pnp60(args);
    സ്ഥിരസ്ഥിതി:   തിരികെ handle_pnpXX(args);
    }
}

u16
get_pnp_offset(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PNPBIOS)
        തിരികെ (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    തിരികെ (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
ബാഹ്യ ശൂന്യം entry_pnp_real(ശൂന്യം);
ബാഹ്യ ശൂന്യം entry_pnp_prot(ശൂന്യം);

ശൂന്യം
pnp_init(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PNPBIOS)
        തിരികെ;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, വലുപ്പം(PNPHEADER));
}
