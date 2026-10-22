#اگر_تعریف_نشده __PNPHEADER_H
#تعریف_کن __PNPHEADER_H

#تعریف_کن PNP_SIGNATURE 0x506e5024 // $PnP

بنية pnpheader {
    u32 signature;
    u8 نسخه_2;
    u8 درازی;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_پایه;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#تعریف_کن FUNCTION_NOT_SUPPORTED 0x82

#پایان_اگر // pnpheader.h
