// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // BUILD_BIOS_ADDR
#ضمن "farptr.h" // SET_FARVAR
#ضمن "output.h" // dprintf
#ضمن "std/pnpbios.h" // PNP_SIGNATURE
#ضمن "string.h" // checksum
#ضمن "util.h" // pnp_init

خارجي بنية pnpheader PNPHEADER;
خارجي حرف pnp_string[];

#إذا CONFIG_PNPBIOS
بنية pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .نسخو_2 = 0x10,
    .ڊيگهائي = حجم(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_بنياد = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#وإلا
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
حرف pnp_string[] __aligned(2) VARFSEG = " $PnP";
#جيڪڏهن_پورو

// BBS - Get Version and Installation Check
ساكن u16
handle_pnp60(u16 *دليلون)
{
    u16 نسخو_ptr = دليلون[1];
    u16 نسخو_seg = دليلون[2];
    سيٽ_FARVAR(نسخو_seg, *(u16*)(نسخو_ptr+0), 0x0101);
    أرجع 0;
}

ساكن u16
handle_pnpXX(u16 *دليلون)
{
    أرجع FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *دليلون)
{
    إذا (! CONFIG_PNPBIOS)
        أرجع FUNCTION_NOT_SUPPORTED;

    u16 arg1 = دليلون[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    بدل (arg1) {
    حالة 0x60: أرجع handle_pnp60(دليلون);
    افتراضي:   أرجع handle_pnpXX(دليلون);
    }
}

u16
get_pnp_وٿي(فراغ)
{
    إذا (! CONFIG_PNPBIOS)
        أرجع (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    أرجع (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
خارجي فراغ داخلا_pnp_real(فراغ);
خارجي فراغ داخلا_pnp_prot(فراغ);

فراغ
pnp_init(فراغ)
{
    إذا (! CONFIG_PNPBIOS)
        أرجع;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)داخلا_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)داخلا_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, حجم(PNPHEADER));
}
