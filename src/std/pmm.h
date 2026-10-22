#यदि_अपरिभाषितम् __PMM_H
#परिभाषय __PMM_H

#अन्तर्भाव "types.h" // u32

#परिभाषय PMM_SIGNATURE 0x4d4d5024 // $PMM

संरचना pmmheader {
    u32 signature;
    u8 संस्करणम्_2;
    u8 दीर्घता;
    u8 checksum;
    संरचना segoff_s प्रविष्टिः;
    u8 reserved[5];
} PACKED;

#परिभाषय PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#यदि_अन्तः // pmm.h
