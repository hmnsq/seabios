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

ಬಾಹ್ಯ ರಚನೆ pnpheader PNPHEADER;
ಬಾಹ್ಯ ಅಕ್ಷರ pnp_string[];

#ಆದರೆ CONFIG_PNPBIOS
ರಚನೆ pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .ಆವೃತ್ತಿ_2 = 0x10,
    .ಉದ್ದಳತೆ = ಗಾತ್ರ(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_ತಳ = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#ಇಲ್ಲದಿದ್ದರೆ
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
ಅಕ್ಷರ pnp_string[] __aligned(2) VARFSEG = " $PnP";
#ಆದರೆ_ಅಂತ್ಯ

// BBS - Get Version and Installation Check
ಸ್ಥಿರ u16
handle_pnp60(u16 *ವಾದಗಳು)
{
    u16 ಆವೃತ್ತಿ_ptr = ವಾದಗಳು[1];
    u16 ಆವೃತ್ತಿ_seg = ವಾದಗಳು[2];
    ಗಣ_FARVAR(ಆವೃತ್ತಿ_seg, *(u16*)(ಆವೃತ್ತಿ_ptr+0), 0x0101);
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u16
handle_pnpXX(u16 *ವಾದಗಳು)
{
    ಹಿಂತಿರುಗಿ FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *ವಾದಗಳು)
{
    ಆದರೆ (! CONFIG_PNPBIOS)
        ಹಿಂತಿರುಗಿ FUNCTION_NOT_SUPPORTED;

    u16 arg1 = ವಾದಗಳು[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    ಆಯ್ಕೆ (arg1) {
    ಸ್ಥಿತಿ 0x60: ಹಿಂತಿರುಗಿ handle_pnp60(ವಾದಗಳು);
    ಪೂರ್ವನಿಯೋಜಿತ:   ಹಿಂತಿರುಗಿ handle_pnpXX(ವಾದಗಳು);
    }
}

u16
get_pnp_ಸ್ಥಾನಾಂತರ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PNPBIOS)
        ಹಿಂತಿರುಗಿ (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    ಹಿಂತಿರುಗಿ (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_pnp_real(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_pnp_prot(ಶೂನ್ಯ);

ಶೂನ್ಯ
pnp_init(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PNPBIOS)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)ನಮೂದು_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)ನಮೂದು_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, ಗಾತ್ರ(PNPHEADER));
}
