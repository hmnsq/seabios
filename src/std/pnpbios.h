#यदि_अपरिभाषितम् __PNPHEADER_H
#परिभाषय __PNPHEADER_H

#परिभाषय PNP_SIGNATURE 0x506e5024 // $PnP

संरचना pnpheader {
    u32 signature;
    u8 संस्करणम्_2;
    u8 दीर्घता;
    u16 control;
    u8 checksum;
    u32 eventloc;
    u16 real_ip;
    u16 real_cs;
    u16 prot_ip;
    u32 prot_आधारः;
    u32 oemid;
    u16 real_ds;
    u32 prot_database;
} PACKED;

#परिभाषय FUNCTION_NOT_SUPPORTED 0x82

#यदि_अन्तः // pnpheader.h
