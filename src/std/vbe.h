#جے_بنا_تعریف __VBE_H
#تعریف_کرو __VBE_H

#ضمن "types.h" // u8

#تعریف_کرو VESA_SIGNATURE 0x41534556 // VESA
#تعریف_کرو VBE2_SIGNATURE 0x32454256 // VBE2

بنية vbe_info {
    u32 signature;
    u16 نسخہ_2;
    بنية segoff_s oem_string;
    u32 capabilities;
    بنية segoff_s video_منوال;
    u16 کل_حافظہ;
    u16 oem_revision;
    بنية segoff_s oem_vendor_string;
    بنية segoff_s oem_ضرب_حاصل_string;
    بنية segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_انکڑے[256];
} PACKED;

بنية vbe_منوال_info {
    /* VBE */
    u16 منوال_گن;
    u8 win_A_گن;
    u8 win_B_گن;
    u16 win_granularity;
    u16 win_ناپ;
    u16 winA_seg;
    u16 winB_seg;
    بنية segoff_s win_func_ptr;
    u16 بائٹاں_فی_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 بٹ_فی_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_ناپ;
    u8 ورقے;
    u8 reserved0;
    /* Direct Color */
    u8 red_ناپ;
    u8 red_pos;
    u8 green_ناپ;
    u8 green_pos;
    u8 blue_ناپ;
    u8 blue_pos;
    u8 alpha_ناپ;
    u8 alpha_pos;
    u8 directcolor_info;
    /* VBE 2.0 */
    u32 phys_ادھار;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 خطی_بائٹاں_فی_scanline;
    u8 bank_ورقے;
    u8 خطی_ورقے;
    u8 خطی_red_ناپ;
    u8 خطی_red_pos;
    u8 خطی_green_ناپ;
    u8 خطی_green_pos;
    u8 خطی_blue_ناپ;
    u8 خطی_blue_pos;
    u8 خطی_alpha_ناپ;
    u8 خطی_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

بنية vbe_crtc_info {
    u16 horiz_کل;
    u16 horiz_sync_شروع;
    u16 horiz_sync_سرا;
    u16 vert_کل;
    u16 vert_sync_شروع;
    u16 vert_sync_سرا;
    u8 flags;
    u32 pixclock;
    u16 refresh_شرح;
    u8 reserved[40];
} PACKED;

بنية vbe_palette_اندراج {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#تعریف_کرو VBE_RETURN_حالت_SUPPORTED                      0x4F
#تعریف_کرو VBE_RETURN_حالت_UNSUPPORTED                    0x00
/* AH */
#تعریف_کرو VBE_RETURN_حالت_SUCCESSFULL                    0x00
#تعریف_کرو VBE_RETURN_حالت_FAILED                         0x01
#تعریف_کرو VBE_RETURN_حالت_NOT_SUPPORTED                  0x02
#تعریف_کرو VBE_RETURN_حالت_INVALID                        0x03

/* VBE Mode Numbers */

#تعریف_کرو VBE_منوال_VESA_DEFINED                            0x0100
#تعریف_کرو VBE_منوال_REFRESH_شرح_USE_CRTC                   0x0800
#تعریف_کرو VBE_منوال_خطی_چوکھٹا_بفر                     0x4000
#تعریف_کرو VBE_منوال_PRESERVE_DISPLAY_حافظہ                 0x8000

#تعریف_کرو VBE_VESA_منوال_سرا_دا_LIST                        0xFFFF

/* Capabilities */

#تعریف_کرو VBE_CAPABILITY_8_ثنائی_ہندسہ_DAC                          0x0001
#تعریف_کرو VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#تعریف_کرو VBE_CAPABILITY_RAMDAC_USE_BLANK_ثنائی_ہندسہ              0x0004
#تعریف_کرو VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#تعریف_کرو VBE_CAPABILITY_STEREO_راہیں_VESA_EVC               0x0010

/* Mode Attributes */

#تعریف_کرو VBE_منوال_ATTRIBUTE_SUPPORTED                     0x0001
#تعریف_کرو VBE_منوال_ATTRIBUTE_EXTENDED_INFORMATION_دستیاب  0x0002
#تعریف_کرو VBE_منوال_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#تعریف_کرو VBE_منوال_ATTRIBUTE_COLOR_منوال                    0x0008
#تعریف_کرو VBE_منوال_ATTRIBUTE_GRAPHICS_منوال                 0x0010
#تعریف_کرو VBE_منوال_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#تعریف_کرو VBE_منوال_ATTRIBUTE_NO_VGA_COMPATIBLE_کھڑکی      0x0040
#تعریف_کرو VBE_منوال_ATTRIBUTE_خطی_چوکھٹا_بفر_منوال      0x0080
#تعریف_کرو VBE_منوال_ATTRIBUTE_DOUBLE_جانچ_منوال              0x0100
#تعریف_کرو VBE_منوال_ATTRIBUTE_INTERLACE_منوال                0x0200
#تعریف_کرو VBE_منوال_ATTRIBUTE_HARDWARE_TRIPLE_بفر        0x0400
#تعریف_کرو VBE_منوال_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#تعریف_کرو VBE_منوال_ATTRIBUTE_DUAL_DISPLAY_شروع_پتہ    0x1000

#تعریف_کرو VBE_منوال_ATTTRIBUTE_LFB_ONLY                     ( VBE_منوال_ATTRIBUTE_NO_VGA_COMPATIBLE_کھڑکی | VBE_منوال_ATTRIBUTE_خطی_چوکھٹا_بفر_منوال )

/* Window attributes */

#تعریف_کرو VBE_کھڑکی_ATTRIBUTE_RELOCATABLE                 0x01
#تعریف_کرو VBE_کھڑکی_ATTRIBUTE_READABLE                    0x02
#تعریف_کرو VBE_کھڑکی_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#تعریف_کرو VBE_MEMORYMODEL_متن_منوال                        0x00
#تعریف_کرو VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#تعریف_کرو VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#تعریف_کرو VBE_MEMORYMODEL_PLANAR                           0x03
#تعریف_کرو VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#تعریف_کرو VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#تعریف_کرو VBE_MEMORYMODEL_سدھا_COLOR                     0x06
#تعریف_کرو VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#تعریف_کرو VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#تعریف_کرو VBE_DIRECTCOLOR_RESERVED_بٹ_دستیاب          0x02

#جے_انت
