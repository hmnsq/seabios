// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // BUILD_BIOS_ADDR
#அந்தர்பாவ "farptr.h" // SET_FARVAR
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "std/pnpbios.h" // PNP_SIGNATURE
#அந்தர்பாவ "string.h" // checksum
#அந்தர்பாவ "util.h" // pnp_init

வெளி கட்டமைப்பு pnpheader PNPHEADER;
வெளி எழுத்து pnp_string[];

#எனில் CONFIG_PNPBIOS
கட்டமைப்பு pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = அளவு(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#இல்லையெனில்
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
எழுத்து pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
நிலையான u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    திரும்பு 0;
}

நிலையான u16
handle_pnpXX(u16 *args)
{
    திரும்பு FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    எனில் (! CONFIG_PNPBIOS)
        திரும்பு FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    தேர்வு (arg1) {
    நிலை 0x60: திரும்பு handle_pnp60(args);
    இயல்பு:   திரும்பு handle_pnpXX(args);
    }
}

u16
get_pnp_offset(வெறுமை)
{
    எனில் (! CONFIG_PNPBIOS)
        திரும்பு (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    திரும்பு (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
வெளி வெறுமை entry_pnp_real(வெறுமை);
வெளி வெறுமை entry_pnp_prot(வெறுமை);

வெறுமை
pnp_init(வெறுமை)
{
    எனில் (! CONFIG_PNPBIOS)
        திரும்பு;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, அளவு(PNPHEADER));
}
