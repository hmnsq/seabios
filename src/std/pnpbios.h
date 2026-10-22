#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PNPHEADER_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PNPHEADER_H

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PNP_SIGNATURE 0x506e5024 // $PnP

ਬਣਤਰ pnpheader {
    u32 signature;
    u8 ਸੰਸਕਰਣ_2;
    u8 ਲੰਮਾਈ;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_ਅਧਾਰ;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FUNCTION_NOT_SUPPORTED 0x82

#ਜੇ_ਅੰਤ // pnpheader.h
