#اگر_غیر_تعریف_شدہ __PNPHEADER_H
#تعریف_کرو __PNPHEADER_H

#تعریف_کرو PNP_SIGNATURE 0x506e5024 // $PnP

ساخت pnpheader {
    u32 signature;
    u8 نسخہ_2;
    u8 لمبائی;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_بنیاد;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#تعریف_کرو FUNCTION_NOT_SUPPORTED 0x82

#اگر_ختم // pnpheader.h
