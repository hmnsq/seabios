#که_نه_تعریف_شوی __VBE_H
#تعریف_کړه __VBE_H

#ضمن "types.h" // u8

#تعریف_کړه VESA_SIGNATURE 0x41534556 // VESA
#تعریف_کړه VBE2_SIGNATURE 0x32454256 // VBE2

بنية vbe_info {
    u32 signature;
    u16 نسخه_2;
    بنية segoff_s oem_string;
    u32 capabilities;
    بنية segoff_s video_ډېرپېښ;
    u16 ټول_حافظه;
    u16 oem_revision;
    بنية segoff_s oem_vendor_string;
    بنية segoff_s oem_حاصل_ضرب_string;
    بنية segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_معلومات[256];
} PACKED;

بنية vbe_ډېرپېښ_info {
    /* VBE */
    u16 ډېرپېښ_ځانګړتیاوې;
    u8 win_A_ځانګړتیاوې;
    u8 win_B_ځانګړتیاوې;
    u16 win_granularity;
    u16 win_اندازه;
    u16 winA_seg;
    u16 winB_seg;
    بنية segoff_s win_func_ptr;
    u16 بایټونه_فی_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 بټونه_فی_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_اندازه;
    u8 پاڼې;
    u8 reserved0;
    /* Direct Color */
    u8 red_اندازه;
    u8 red_pos;
    u8 green_اندازه;
    u8 green_pos;
    u8 blue_اندازه;
    u8 blue_pos;
    u8 alpha_اندازه;
    u8 alpha_pos;
    u8 directcolor_info;
    /* VBE 2.0 */
    u32 phys_بنسټ;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 خطي_بایټونه_فی_scanline;
    u8 bank_پاڼې;
    u8 خطي_پاڼې;
    u8 خطي_red_اندازه;
    u8 خطي_red_pos;
    u8 خطي_green_اندازه;
    u8 خطي_green_pos;
    u8 خطي_blue_اندازه;
    u8 خطي_blue_pos;
    u8 خطي_alpha_اندازه;
    u8 خطي_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

بنية vbe_crtc_info {
    u16 horiz_ټول;
    u16 horiz_sync_پیل;
    u16 horiz_sync_پای_ټکی;
    u16 vert_ټول;
    u16 vert_sync_پیل;
    u16 vert_sync_پای_ټکی;
    u8 flags;
    u32 pixclock;
    u16 refresh_شرح;
    u8 reserved[40];
} PACKED;

بنية vbe_palette_ثبت {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#تعریف_کړه VBE_RETURN_حالت_SUPPORTED                      0x4F
#تعریف_کړه VBE_RETURN_حالت_UNSUPPORTED                    0x00
/* AH */
#تعریف_کړه VBE_RETURN_حالت_SUCCESSFULL                    0x00
#تعریف_کړه VBE_RETURN_حالت_FAILED                         0x01
#تعریف_کړه VBE_RETURN_حالت_NOT_SUPPORTED                  0x02
#تعریف_کړه VBE_RETURN_حالت_INVALID                        0x03

/* VBE Mode Numbers */

#تعریف_کړه VBE_ډېرپېښ_VESA_DEFINED                            0x0100
#تعریف_کړه VBE_ډېرپېښ_REFRESH_شرح_USE_CRTC                   0x0800
#تعریف_کړه VBE_ډېرپېښ_خطي_چوکاټ_بفر                     0x4000
#تعریف_کړه VBE_ډېرپېښ_PRESERVE_DISPLAY_حافظه                 0x8000

#تعریف_کړه VBE_VESA_ډېرپېښ_پای_ټکی_د_LIST                        0xFFFF

/* Capabilities */

#تعریف_کړه VBE_CAPABILITY_8_بټ_DAC                          0x0001
#تعریف_کړه VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#تعریف_کړه VBE_CAPABILITY_RAMDAC_USE_BLANK_بټ              0x0004
#تعریف_کړه VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#تعریف_کړه VBE_CAPABILITY_STEREO_له_لارې_VESA_EVC               0x0010

/* Mode Attributes */

#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_SUPPORTED                     0x0001
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_EXTENDED_INFORMATION_شته  0x0002
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_COLOR_ډېرپېښ                    0x0008
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_GRAPHICS_ډېرپېښ                 0x0010
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_NO_VGA_COMPATIBLE_کړکۍ      0x0040
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_خطي_چوکاټ_بفر_ډېرپېښ      0x0080
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_DOUBLE_لټون_ډېرپېښ              0x0100
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_INTERLACE_ډېرپېښ                0x0200
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_HARDWARE_TRIPLE_بفر        0x0400
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#تعریف_کړه VBE_ډېرپېښ_ATTRIBUTE_DUAL_DISPLAY_پیل_پته    0x1000

#تعریف_کړه VBE_ډېرپېښ_ATTTRIBUTE_LFB_ONLY                     ( VBE_ډېرپېښ_ATTRIBUTE_NO_VGA_COMPATIBLE_کړکۍ | VBE_ډېرپېښ_ATTRIBUTE_خطي_چوکاټ_بفر_ډېرپېښ )

/* Window attributes */

#تعریف_کړه VBE_کړکۍ_ATTRIBUTE_RELOCATABLE                 0x01
#تعریف_کړه VBE_کړکۍ_ATTRIBUTE_READABLE                    0x02
#تعریف_کړه VBE_کړکۍ_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#تعریف_کړه VBE_MEMORYMODEL_متن_ډېرپېښ                        0x00
#تعریف_کړه VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#تعریف_کړه VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#تعریف_کړه VBE_MEMORYMODEL_PLANAR                           0x03
#تعریف_کړه VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#تعریف_کړه VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#تعریف_کړه VBE_MEMORYMODEL_مستقیم_COLOR                     0x06
#تعریف_کړه VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#تعریف_کړه VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#تعریف_کړه VBE_DIRECTCOLOR_RESERVED_بټونه_شته          0x02

#د_که_پای
