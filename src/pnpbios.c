// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // BUILD_BIOS_ADDR
#ಅನ್ತರ್ಭಾವ "farptr.h" // SET_FARVAR
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/pnpbios.h" // PNP_SIGNATURE
#ಅನ್ತರ್ಭಾವ "string.h" // checksum
#ಅನ್ತರ್ಭಾವ "util.h" // pnp_init

ಬಾಹ್ಯ ಸಂರಚನಾ pnpheader PNPHEADER;
ಬಾಹ್ಯ ವರ್ಣ pnp_string[];

#ಯದಿ CONFIG_PNPBIOS
ಸಂರಚನಾ pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .version = 0x10,
    .length = ಪರಿಮಾಣ(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_base = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#ಅನ್ಯಥಾ
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
ವರ್ಣ pnp_string[] __aligned(2) VARFSEG = " $PnP";
#endif

// BBS - Get Version and Installation Check
ಸ್ಥಿರ u16
handle_pnp60(u16 *args)
{
    u16 version_ptr = args[1];
    u16 version_seg = args[2];
    SET_FARVAR(version_seg, *(u16*)(version_ptr+0), 0x0101);
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u16
handle_pnpXX(u16 *args)
{
    ಪ್ರತ್ಯಾಗಮ FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *args)
{
    ಯದಿ (! CONFIG_PNPBIOS)
        ಪ್ರತ್ಯಾಗಮ FUNCTION_NOT_SUPPORTED;

    u16 arg1 = args[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    ವಿಕಲ್ಪ (arg1) {
    ಪಕ್ಷ 0x60: ಪ್ರತ್ಯಾಗಮ handle_pnp60(args);
    ಸ್ವಭಾವ:   ಪ್ರತ್ಯಾಗಮ handle_pnpXX(args);
    }
}

u16
get_pnp_offset(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PNPBIOS)
        ಪ್ರತ್ಯಾಗಮ (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    ಪ್ರತ್ಯಾಗಮ (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
ಬಾಹ್ಯ ಶೂನ್ಯ entry_pnp_real(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ entry_pnp_prot(ಶೂನ್ಯ);

ಶೂನ್ಯ
pnp_init(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PNPBIOS)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)entry_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)entry_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, ಪರಿಮಾಣ(PNPHEADER));
}
