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

خارجی ساخت pnpheader PNPHEADER;
خارجی حرف pnp_string[];

#اگر CONFIG_PNPBIOS
ساخت pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .نسخہ_2 = 0x10,
    .لمبائی = حجم(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_بنیاد = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#ورنہ
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
حرف pnp_string[] __aligned(2) VARFSEG = " $PnP";
#اگر_ختم

// BBS - Get Version and Installation Check
ساکن u16
handle_pnp60(u16 *دلائل)
{
    u16 نسخہ_ptr = دلائل[1];
    u16 نسخہ_seg = دلائل[2];
    سیٹ_FARVAR(نسخہ_seg, *(u16*)(نسخہ_ptr+0), 0x0101);
    واپس 0;
}

ساکن u16
handle_pnpXX(u16 *دلائل)
{
    واپس FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *دلائل)
{
    اگر (! CONFIG_PNPBIOS)
        واپس FUNCTION_NOT_SUPPORTED;

    u16 arg1 = دلائل[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    انتخاب (arg1) {
    حالت 0x60: واپس handle_pnp60(دلائل);
    طےشدہ:   واپس handle_pnpXX(دلائل);
    }
}

u16
get_pnp_انحراف(خالی)
{
    اگر (! CONFIG_PNPBIOS)
        واپس (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    واپس (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
خارجی خالی اندراج_pnp_real(خالی);
خارجی خالی اندراج_pnp_prot(خالی);

خالی
pnp_init(خالی)
{
    اگر (! CONFIG_PNPBIOS)
        واپس;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)اندراج_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)اندراج_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, حجم(PNPHEADER));
}
