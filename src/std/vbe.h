#اگر_تعریف_نشده __VBE_H
#تعریف_کن __VBE_H

#ضمن "types.h" // u8

#تعریف_کن VESA_SIGNATURE 0x41534556 // VESA
#تعریف_کن VBE2_SIGNATURE 0x32454256 // VBE2

بنية vbe_info {
    u32 signature;
    u16 نسخه_2;
    بنية segoff_s oem_string;
    u32 capabilities;
    بنية segoff_s video_نما;
    u16 مجموع_حافظه;
    u16 oem_revision;
    بنية segoff_s oem_vendor_string;
    بنية segoff_s oem_حاصلضرب_string;
    بنية segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_داده[256];
} PACKED;

بنية vbe_نما_info {
    /* VBE */
    u16 نما_صفات;
    u8 win_A_صفات;
    u8 win_B_صفات;
    u16 win_granularity;
    u16 win_اندازه;
    u16 winA_seg;
    u16 winB_seg;
    بنية segoff_s win_func_ptr;
    u16 بایتها_در_هر_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 بیتها_در_هر_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_اندازه;
    u8 صفحات;
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
    u32 phys_پایه;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 خطی_بایتها_در_هر_scanline;
    u8 bank_صفحات;
    u8 خطی_صفحات;
    u8 خطی_red_اندازه;
    u8 خطی_red_pos;
    u8 خطی_green_اندازه;
    u8 خطی_green_pos;
    u8 خطی_blue_اندازه;
    u8 خطی_blue_pos;
    u8 خطی_alpha_اندازه;
    u8 خطی_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

بنية vbe_crtc_info {
    u16 horiz_مجموع;
    u16 horiz_sync_شروع;
    u16 horiz_sync_انتها;
    u16 vert_مجموع;
    u16 vert_sync_شروع;
    u16 vert_sync_انتها;
    u8 flags;
    u32 pixclock;
    u16 refresh_نرخ;
    u8 reserved[40];
} PACKED;

بنية vbe_palette_مدخل {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#تعریف_کن VBE_RETURN_وضعیت_SUPPORTED                      0x4F
#تعریف_کن VBE_RETURN_وضعیت_UNSUPPORTED                    0x00
/* AH */
#تعریف_کن VBE_RETURN_وضعیت_SUCCESSFULL                    0x00
#تعریف_کن VBE_RETURN_وضعیت_FAILED                         0x01
#تعریف_کن VBE_RETURN_وضعیت_NOT_SUPPORTED                  0x02
#تعریف_کن VBE_RETURN_وضعیت_INVALID                        0x03

/* VBE Mode Numbers */

#تعریف_کن VBE_نما_VESA_DEFINED                            0x0100
#تعریف_کن VBE_نما_REFRESH_نرخ_USE_CRTC                   0x0800
#تعریف_کن VBE_نما_خطی_قاب_میانگیر                     0x4000
#تعریف_کن VBE_نما_PRESERVE_DISPLAY_حافظه                 0x8000

#تعریف_کن VBE_VESA_نما_انتها_از_LIST                        0xFFFF

/* Capabilities */

#تعریف_کن VBE_CAPABILITY_8_بیت_DAC                          0x0001
#تعریف_کن VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#تعریف_کن VBE_CAPABILITY_RAMDAC_USE_BLANK_بیت              0x0004
#تعریف_کن VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#تعریف_کن VBE_CAPABILITY_STEREO_از_طریق_VESA_EVC               0x0010

/* Mode Attributes */

#تعریف_کن VBE_نما_ATTRIBUTE_SUPPORTED                     0x0001
#تعریف_کن VBE_نما_ATTRIBUTE_EXTENDED_INFORMATION_موجود  0x0002
#تعریف_کن VBE_نما_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#تعریف_کن VBE_نما_ATTRIBUTE_COLOR_نما                    0x0008
#تعریف_کن VBE_نما_ATTRIBUTE_GRAPHICS_نما                 0x0010
#تعریف_کن VBE_نما_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#تعریف_کن VBE_نما_ATTRIBUTE_NO_VGA_COMPATIBLE_پنجره      0x0040
#تعریف_کن VBE_نما_ATTRIBUTE_خطی_قاب_میانگیر_نما      0x0080
#تعریف_کن VBE_نما_ATTRIBUTE_DOUBLE_پیمایش_نما              0x0100
#تعریف_کن VBE_نما_ATTRIBUTE_INTERLACE_نما                0x0200
#تعریف_کن VBE_نما_ATTRIBUTE_HARDWARE_TRIPLE_میانگیر        0x0400
#تعریف_کن VBE_نما_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#تعریف_کن VBE_نما_ATTRIBUTE_DUAL_DISPLAY_شروع_آدرس    0x1000

#تعریف_کن VBE_نما_ATTTRIBUTE_LFB_ONLY                     ( VBE_نما_ATTRIBUTE_NO_VGA_COMPATIBLE_پنجره | VBE_نما_ATTRIBUTE_خطی_قاب_میانگیر_نما )

/* Window attributes */

#تعریف_کن VBE_پنجره_ATTRIBUTE_RELOCATABLE                 0x01
#تعریف_کن VBE_پنجره_ATTRIBUTE_READABLE                    0x02
#تعریف_کن VBE_پنجره_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#تعریف_کن VBE_MEMORYMODEL_متن_نما                        0x00
#تعریف_کن VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#تعریف_کن VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#تعریف_کن VBE_MEMORYMODEL_PLANAR                           0x03
#تعریف_کن VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#تعریف_کن VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#تعریف_کن VBE_MEMORYMODEL_مستقیم_COLOR                     0x06
#تعریف_کن VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#تعریف_کن VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#تعریف_کن VBE_DIRECTCOLOR_RESERVED_بیتها_موجود          0x02

#پایان_اگر
