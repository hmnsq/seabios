#నిర్వచితం_కాకపోతే __PNPHEADER_H
#నిర్వచించు __PNPHEADER_H

#నిర్వచించు PNP_SIGNATURE 0x506e5024 // $PnP

నిర్మాణం pnpheader {
    u32 signature;
    u8 సంస్కరణ_2;
    u8 నిడివి;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_పునాది;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#నిర్వచించు FUNCTION_NOT_SUPPORTED 0x82

#అయితే_ముగింపు // pnpheader.h
