#که_نه_تعریف_شوی __PNPHEADER_H
#تعریف_کړه __PNPHEADER_H

#تعریف_کړه PNP_SIGNATURE 0x506e5024 // $PnP

بنية pnpheader {
    u32 signature;
    u8 نسخه_2;
    u8 اوږدتیا;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_بنسټ;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#تعریف_کړه FUNCTION_NOT_SUPPORTED 0x82

#د_که_پای // pnpheader.h
