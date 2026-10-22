// PNP BIOS calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // BUILD_BIOS_ADDR
#అన్తర్భావ "farptr.h" // SET_FARVAR
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "std/pnpbios.h" // PNP_SIGNATURE
#అన్తర్భావ "string.h" // checksum
#అన్తర్భావ "util.h" // pnp_init

బాహ్య నిర్మాణం pnpheader PNPHEADER;
బాహ్య అక్షరం pnp_string[];

#ఒకవేళ CONFIG_PNPBIOS
నిర్మాణం pnpheader PNPHEADER __aligned(16) VARFSEG = {
    .signature = PNP_SIGNATURE,
    .సంస్కరణ_2 = 0x10,
    .నిడివి = పరిమాణం(PNPHEADER),
    .real_cs = SEG_BIOS,
    .prot_పునాది = BUILD_BIOS_ADDR,
    .real_ds = SEG_BIOS,
    .prot_database = BUILD_BIOS_ADDR,
};
#లేకపోతే
// We need a copy of this string in the 0xf000 segment, but we are not
// actually a PnP BIOS, so make sure it is *not* aligned, so OSes will
// not see it if they scan.
అక్షరం pnp_string[] __aligned(2) VARFSEG = " $PnP";
#అయితే_ముగింపు

// BBS - Get Version and Installation Check
స్థిర u16
handle_pnp60(u16 *వాదనలు)
{
    u16 సంస్కరణ_ptr = వాదనలు[1];
    u16 సంస్కరణ_seg = వాదనలు[2];
    సమితి_FARVAR(సంస్కరణ_seg, *(u16*)(సంస్కరణ_ptr+0), 0x0101);
    తిరిగి 0;
}

స్థిర u16
handle_pnpXX(u16 *వాదనలు)
{
    తిరిగి FUNCTION_NOT_SUPPORTED;
}

u16 VISIBLE16
handle_pnp(u16 *వాదనలు)
{
    ఒకవేళ (! CONFIG_PNPBIOS)
        తిరిగి FUNCTION_NOT_SUPPORTED;

    u16 arg1 = వాదనలు[0];
    dprintf(DEBUG_HDL_pnp, "pnp call arg1=%x\n", arg1);

    ఎంపిక (arg1) {
    స్థితి 0x60: తిరిగి handle_pnp60(వాదనలు);
    అప్రమేయ:   తిరిగి handle_pnpXX(వాదనలు);
    }
}

u16
get_pnp_స్థానభ్రంశం(శూన్య)
{
    ఒకవేళ (! CONFIG_PNPBIOS)
        తిరిగి (u32)pnp_string + 1 - BUILD_BIOS_ADDR;
    తిరిగి (u32)&PNPHEADER - BUILD_BIOS_ADDR;
}

// romlayout.S
బాహ్య శూన్య నమోదు_pnp_real(శూన్య);
బాహ్య శూన్య నమోదు_pnp_prot(శూన్య);

శూన్య
pnp_init(శూన్య)
{
    ఒకవేళ (! CONFIG_PNPBIOS)
        తిరిగి;

    dprintf(3, "init PNPBIOS table\n");

    PNPHEADER.real_ip = (u32)నమోదు_pnp_real - BUILD_BIOS_ADDR;
    PNPHEADER.prot_ip = (u32)నమోదు_pnp_prot - BUILD_BIOS_ADDR;
    PNPHEADER.checksum -= checksum(&PNPHEADER, పరిమాణం(PNPHEADER));
}
