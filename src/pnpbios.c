// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // BUILD_BIOS_ADDR
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // SET_FARVAR
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "std/pnpbios.h" // PNP_SIGNATURE
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // checksum
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // pnp_init

ਬਾਹਰੀ ਬਣਤਰ pnpheader PNPHEADER;
ਬਾਹਰੀ ਅੱਖਰ pnp_string[];

#ਜੇ CONFIG_PNPBIOS
ਬਣਤਰ pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .ਸੰਸਕਰਣ_2 = 0x10,
    .ਲੰਮਾਈ = ਆਕਾਰ(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_ਅਧਾਰ = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#ਨਹੀਂਤਾਂ
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
ਅੱਖਰ pnp_string[] __aligned(2) VARFSEG = " $PnP";
#ਜੇ_ਅੰਤ

// BBS - Get Version and Installation Check
ਸਥਿਰ u16
handle_pnp60(u16 *ਦਲੀਲਾਂ)
{
    u16 ਸੰਸਕਰਣ_ptr = ਦਲੀਲਾਂ[1];
    u16 ਸੰਸਕਰਣ_seg = ਦਲੀਲਾਂ[2];
    ਸੈੱਟ_FARVAR(ਸੰਸਕਰਣ_seg, *(u16*)(ਸੰਸਕਰਣ_ptr+0), 0x0101);
    ਵਾਪਸ 0;
}

ਸਥਿਰ u16
handle_pnpXX(u16 *ਦਲੀਲਾਂ)
{
    ਵਾਪਸ FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *ਦਲੀਲਾਂ)
{
    ਜੇ (! CONFIG_PNPBIOS)
        ਵਾਪਸ FUNCTION_NOT_SUPPORTED;

    u16 arg1 = ਦਲੀਲਾਂ[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    ਚੋਣ (arg1) {
    ਹਾਲਤ 0x60: ਵਾਪਸ handle_pnp60(ਦਲੀਲਾਂ);
    ਮੂਲ:   ਵਾਪਸ handle_pnpXX(ਦਲੀਲਾਂ);
    }
}

u16
get_pnp_ਵਿੱਥ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PNPBIOS)
        ਵਾਪਸ (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    ਵਾਪਸ (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_pnp_real(ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_pnp_prot(ਖਾਲੀ);

ਖਾਲੀ
pnp_init(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PNPBIOS)
        ਵਾਪਸ;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)ਇੰਦਰਾਜ_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)ਇੰਦਰਾਜ_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, ਆਕਾਰ(PNPHEADER));
}
