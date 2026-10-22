#यदि_अपरिभाषितम् __VBE_H
#परिभाषय __VBE_H

#अन्तर्भाव "types.h" // u8

#परिभाषय VESA_SIGNATURE 0x41534556 // VESA
#परिभाषय VBE2_SIGNATURE 0x32454256 // VBE2

संरचना vbe_info {
    u32 signature;
    u16 संस्करणम्_2;
    संरचना segoff_s oem_string;
    u32 capabilities;
    संरचना segoff_s video_बहुलकम्;
    u16 योग_स्मृतिः;
    u16 oem_revision;
    संरचना segoff_s oem_vendor_string;
    संरचना segoff_s oem_गुणनफलम्_string;
    संरचना segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_दत्त[256];
} PACKED;

संरचना vbe_बहुलकम्_info {
    /* VBE */
    u16 बहुलकम्_गुणाः;
    u8 win_A_गुणाः;
    u8 win_B_गुणाः;
    u16 win_granularity;
    u16 win_आकार;
    u16 winA_seg;
    u16 winB_seg;
    संरचना segoff_s win_func_ptr;
    u16 अष्टकानि_प्रत्येकम्_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 द्वयङ्काः_प्रत्येकम्_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_आकार;
    u8 पृष्ठानि;
    u8 reserved0;
    /* Direct Color */
    u8 red_आकार;
    u8 red_pos;
    u8 green_आकार;
    u8 green_pos;
    u8 blue_आकार;
    u8 blue_pos;
    u8 alpha_आकार;
    u8 alpha_pos;
    u8 directcolor_info;
    /* VBE 2.0 */
    u32 phys_आधारः;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 रैखिकम्_अष्टकानि_प्रत्येकम्_scanline;
    u8 bank_पृष्ठानि;
    u8 रैखिकम्_पृष्ठानि;
    u8 रैखिकम्_red_आकार;
    u8 रैखिकम्_red_pos;
    u8 रैखिकम्_green_आकार;
    u8 रैखिकम्_green_pos;
    u8 रैखिकम्_blue_आकार;
    u8 रैखिकम्_blue_pos;
    u8 रैखिकम्_alpha_आकार;
    u8 रैखिकम्_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

संरचना vbe_crtc_info {
    u16 horiz_योग;
    u16 horiz_sync_start;
    u16 horiz_sync_अन्तः;
    u16 vert_योग;
    u16 vert_sync_start;
    u16 vert_sync_अन्तः;
    u8 flags;
    u32 pixclock;
    u16 refresh_दरः;
    u8 reserved[40];
} PACKED;

संरचना vbe_palette_प्रविष्टिः {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#परिभाषय VBE_RETURN_स्थितिः_SUPPORTED                      0x4F
#परिभाषय VBE_RETURN_स्थितिः_UNSUPPORTED                    0x00
/* AH */
#परिभाषय VBE_RETURN_स्थितिः_SUCCESSFULL                    0x00
#परिभाषय VBE_RETURN_स्थितिः_FAILED                         0x01
#परिभाषय VBE_RETURN_स्थितिः_NOT_SUPPORTED                  0x02
#परिभाषय VBE_RETURN_स्थितिः_INVALID                        0x03

/* VBE Mode Numbers */

#परिभाषय VBE_बहुलकम्_VESA_DEFINED                            0x0100
#परिभाषय VBE_बहुलकम्_REFRESH_दरः_USE_CRTC                   0x0800
#परिभाषय VBE_बहुलकम्_रैखिकम्_FRAME_बफर                     0x4000
#परिभाषय VBE_बहुलकम्_PRESERVE_DISPLAY_स्मृतिः                 0x8000

#परिभाषय VBE_VESA_बहुलकम्_अन्तः_सम्बन्धि_LIST                        0xFFFF

/* Capabilities */

#परिभाषय VBE_CAPABILITY_8BIT_DAC                          0x0001
#परिभाषय VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#परिभाषय VBE_CAPABILITY_RAMDAC_USE_BLANK_BIT              0x0004
#परिभाषय VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#परिभाषय VBE_CAPABILITY_STEREO_द्वारा_VESA_EVC               0x0010

/* Mode Attributes */

#परिभाषय VBE_बहुलकम्_ATTRIBUTE_SUPPORTED                     0x0001
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_EXTENDED_INFORMATION_उपलब्धम्  0x0002
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_COLOR_बहुलकम्                    0x0008
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_GRAPHICS_बहुलकम्                 0x0010
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_NO_VGA_COMPATIBLE_गवाक्षः      0x0040
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_रैखिकम्_FRAME_बफर_बहुलकम्      0x0080
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_DOUBLE_अवलोकनम्_बहुलकम्              0x0100
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_INTERLACE_बहुलकम्                0x0200
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_HARDWARE_TRIPLE_बफर        0x0400
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#परिभाषय VBE_बहुलकम्_ATTRIBUTE_DUAL_DISPLAY_START_स्थानाङ्कः    0x1000

#परिभाषय VBE_बहुलकम्_ATTTRIBUTE_LFB_ONLY                     ( VBE_बहुलकम्_ATTRIBUTE_NO_VGA_COMPATIBLE_गवाक्षः | VBE_बहुलकम्_ATTRIBUTE_रैखिकम्_FRAME_बफर_बहुलकम् )

/* Window attributes */

#परिभाषय VBE_गवाक्षः_ATTRIBUTE_RELOCATABLE                 0x01
#परिभाषय VBE_गवाक्षः_ATTRIBUTE_READABLE                    0x02
#परिभाषय VBE_गवाक्षः_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#परिभाषय VBE_MEMORYMODEL_TEXT_बहुलकम्                        0x00
#परिभाषय VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#परिभाषय VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#परिभाषय VBE_MEMORYMODEL_PLANAR                           0x03
#परिभाषय VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#परिभाषय VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#परिभाषय VBE_MEMORYMODEL_प्रत्यक्षम्_COLOR                     0x06
#परिभाषय VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#परिभाषय VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#परिभाषय VBE_DIRECTCOLOR_RESERVED_द्वयङ्काः_उपलब्धम्          0x02

#यदि_अन्तः
