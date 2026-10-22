#جيڪڏهن_وصف_نه_ٿيل __VBE_H
#وصف_ڪريو __VBE_H

#ضمن "types.h" // u8

#وصف_ڪريو VESA_SIGNATURE 0x41534556 // VESA
#وصف_ڪريو VBE2_SIGNATURE 0x32454256 // VBE2

بنية vbe_info {
    u32 signature;
    u16 نسخو_2;
    بنية segoff_s oem_string;
    u32 capabilities;
    بنية segoff_s video_گهڻائي;
    u16 ڪل_يادگيري;
    u16 oem_revision;
    بنية segoff_s oem_vendor_string;
    بنية segoff_s oem_حاصل_ضرب_string;
    بنية segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_معلومات[256];
} PACKED;

بنية vbe_گهڻائي_info {
    /* VBE */
    u16 گهڻائي_خاصيتون;
    u8 win_A_خاصيتون;
    u8 win_B_خاصيتون;
    u16 win_granularity;
    u16 win_سائيز;
    u16 winA_seg;
    u16 winB_seg;
    بنية segoff_s win_func_ptr;
    u16 بائيٽون_في_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 بٽ_في_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_سائيز;
    u8 صفحا;
    u8 reserved0;
    /* Direct Color */
    u8 red_سائيز;
    u8 red_pos;
    u8 green_سائيز;
    u8 green_pos;
    u8 blue_سائيز;
    u8 blue_pos;
    u8 alpha_سائيز;
    u8 alpha_pos;
    u8 directcolor_info;
    /* VBE 2.0 */
    u32 phys_بنياد;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 لڪيري_بائيٽون_في_scanline;
    u8 bank_صفحا;
    u8 لڪيري_صفحا;
    u8 لڪيري_red_سائيز;
    u8 لڪيري_red_pos;
    u8 لڪيري_green_سائيز;
    u8 لڪيري_green_pos;
    u8 لڪيري_blue_سائيز;
    u8 لڪيري_blue_pos;
    u8 لڪيري_alpha_سائيز;
    u8 لڪيري_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

بنية vbe_crtc_info {
    u16 horiz_ڪل;
    u16 horiz_sync_شروعات;
    u16 horiz_sync_پڇاڙي;
    u16 vert_ڪل;
    u16 vert_sync_شروعات;
    u16 vert_sync_پڇاڙي;
    u8 flags;
    u32 pixclock;
    u16 refresh_شرح;
    u8 reserved[40];
} PACKED;

بنية vbe_palette_داخلا {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#وصف_ڪريو VBE_RETURN_حالت_SUPPORTED                      0x4F
#وصف_ڪريو VBE_RETURN_حالت_UNSUPPORTED                    0x00
/* AH */
#وصف_ڪريو VBE_RETURN_حالت_SUCCESSFULL                    0x00
#وصف_ڪريو VBE_RETURN_حالت_FAILED                         0x01
#وصف_ڪريو VBE_RETURN_حالت_NOT_SUPPORTED                  0x02
#وصف_ڪريو VBE_RETURN_حالت_INVALID                        0x03

/* VBE Mode Numbers */

#وصف_ڪريو VBE_گهڻائي_VESA_DEFINED                            0x0100
#وصف_ڪريو VBE_گهڻائي_REFRESH_شرح_USE_CRTC                   0x0800
#وصف_ڪريو VBE_گهڻائي_لڪيري_چوکٺو_بفر                     0x4000
#وصف_ڪريو VBE_گهڻائي_PRESERVE_DISPLAY_يادگيري                 0x8000

#وصف_ڪريو VBE_VESA_گهڻائي_پڇاڙي_جو_LIST                        0xFFFF

/* Capabilities */

#وصف_ڪريو VBE_CAPABILITY_8_ٻٽو_انگ_DAC                          0x0001
#وصف_ڪريو VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#وصف_ڪريو VBE_CAPABILITY_RAMDAC_USE_BLANK_ٻٽو_انگ              0x0004
#وصف_ڪريو VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#وصف_ڪريو VBE_CAPABILITY_STEREO_ذريعي_VESA_EVC               0x0010

/* Mode Attributes */

#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_SUPPORTED                     0x0001
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_EXTENDED_INFORMATION_موجود  0x0002
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_COLOR_گهڻائي                    0x0008
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_GRAPHICS_گهڻائي                 0x0010
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_NO_VGA_COMPATIBLE_دري      0x0040
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_لڪيري_چوکٺو_بفر_گهڻائي      0x0080
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_DOUBLE_جاچ_گهڻائي              0x0100
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_INTERLACE_گهڻائي                0x0200
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_HARDWARE_TRIPLE_بفر        0x0400
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#وصف_ڪريو VBE_گهڻائي_ATTRIBUTE_DUAL_DISPLAY_شروعات_پتو    0x1000

#وصف_ڪريو VBE_گهڻائي_ATTTRIBUTE_LFB_ONLY                     ( VBE_گهڻائي_ATTRIBUTE_NO_VGA_COMPATIBLE_دري | VBE_گهڻائي_ATTRIBUTE_لڪيري_چوکٺو_بفر_گهڻائي )

/* Window attributes */

#وصف_ڪريو VBE_دري_ATTRIBUTE_RELOCATABLE                 0x01
#وصف_ڪريو VBE_دري_ATTRIBUTE_READABLE                    0x02
#وصف_ڪريو VBE_دري_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#وصف_ڪريو VBE_MEMORYMODEL_متن_گهڻائي                        0x00
#وصف_ڪريو VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#وصف_ڪريو VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#وصف_ڪريو VBE_MEMORYMODEL_PLANAR                           0x03
#وصف_ڪريو VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#وصف_ڪريو VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#وصف_ڪريو VBE_MEMORYMODEL_سڌو_COLOR                     0x06
#وصف_ڪريو VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#وصف_ڪريو VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#وصف_ڪريو VBE_DIRECTCOLOR_RESERVED_بٽ_موجود          0x02

#جيڪڏهن_پورو
