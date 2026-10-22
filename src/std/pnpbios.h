#جيڪڏهن_وصف_نه_ٿيل __PNPHEADER_H
#وصف_ڪريو __PNPHEADER_H

#وصف_ڪريو PNP_SIGNATURE 0x506e5024 // $PnP

بنية pnpheader {
    u32 signature;
    u8 نسخو_2;
    u8 ڊيگهائي;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_بنياد;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#وصف_ڪريو FUNCTION_NOT_SUPPORTED 0x82

#جيڪڏهن_پورو // pnpheader.h
