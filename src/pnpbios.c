// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // BUILD_BIOS_ADDR
#अन्तर्भाव "farptr.h" // SET_FARVAR
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/pnpbios.h" // PNP_SIGNATURE
#अन्तर्भाव "string.h" // checksum
#अन्तर्भाव "util.h" // pnp_init

बाह्य संरचना pnpheader PNPHEADER;
बाह्य वर्ण pnp_string[];

#यदि CONFIG_PNPBIOS
संरचना pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = परिमाण(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#अन्यथा
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
वर्ण pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
स्थिर u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    प्रत्यागम 0;
}

स्थिर u16
handle_pnpXX(u16 *args)
{
    प्रत्यागम FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    यदि (! CONFIG_PNPBIOS)
        प्रत्यागम FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    विकल्प (arg1) {
    पक्ष 0x60: प्रत्यागम handle_pnp60(args);
    स्वभाव:   प्रत्यागम handle_pnpXX(args);
    }
}

u16
get_pnp_offset(शून्य)
{
    यदि (! CONFIG_PNPBIOS)
        प्रत्यागम (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    प्रत्यागम (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
बाह्य शून्य entry_pnp_real(शून्य);
बाह्य शून्य entry_pnp_prot(शून्य);

शून्य
pnp_init(शून्य)
{
    यदि (! CONFIG_PNPBIOS)
        प्रत्यागम;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, परिमाण(PNPHEADER));
}
