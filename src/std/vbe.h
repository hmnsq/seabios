#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __VBE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __VBE_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VESA_SIGNATURE 0x41534556 // VESA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE2_SIGNATURE 0x32454256 // VBE2

ਬਣਤਰ vbe_info {
    u32 signature;
    u16 ਸੰਸਕਰਣ_2;
    ਬਣਤਰ segoff_s oem_string;
    u32 capabilities;
    ਬਣਤਰ segoff_s video_ਬਹੁਲਕ;
    u16 ਕੁੱਲ_ਸਮ੍ਰਿਤੀ;
    u16 oem_revision;
    ਬਣਤਰ segoff_s oem_vendor_string;
    ਬਣਤਰ segoff_s oem_ਗੁਣਨਫਲ_string;
    ਬਣਤਰ segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_ਅੰਕੜੇ[256];
} PACKED;

ਬਣਤਰ vbe_ਬਹੁਲਕ_info {
    /* VBE */
    u16 ਬਹੁਲਕ_ਗੁਣ;
    u8 win_A_ਗੁਣ;
    u8 win_B_ਗੁਣ;
    u16 win_granularity;
    u16 win_ਮਾਪ;
    u16 winA_seg;
    u16 winB_seg;
    ਬਣਤਰ segoff_s win_func_ptr;
    u16 ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 ਬਿੱਟ_ਪ੍ਰਤੀ_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_ਮਾਪ;
    u8 ਪੰਨੇ;
    u8 reserved0;
    /* Direct Color */
    u8 red_ਮਾਪ;
    u8 red_pos;
    u8 green_ਮਾਪ;
    u8 green_pos;
    u8 blue_ਮਾਪ;
    u8 blue_pos;
    u8 alpha_ਮਾਪ;
    u8 alpha_pos;
    u8 directcolor_info;
    /* VBE 2.0 */
    u32 phys_ਅਧਾਰ;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 ਰੇਖਿਕ_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_scanline;
    u8 bank_ਪੰਨੇ;
    u8 ਰੇਖਿਕ_ਪੰਨੇ;
    u8 ਰੇਖਿਕ_red_ਮਾਪ;
    u8 ਰੇਖਿਕ_red_pos;
    u8 ਰੇਖਿਕ_green_ਮਾਪ;
    u8 ਰੇਖਿਕ_green_pos;
    u8 ਰੇਖਿਕ_blue_ਮਾਪ;
    u8 ਰੇਖਿਕ_blue_pos;
    u8 ਰੇਖਿਕ_alpha_ਮਾਪ;
    u8 ਰੇਖਿਕ_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

ਬਣਤਰ vbe_crtc_info {
    u16 horiz_ਕੁੱਲ;
    u16 horiz_sync_ਸ਼ੁਰੂ;
    u16 horiz_sync_ਸਿਰਾ;
    u16 vert_ਕੁੱਲ;
    u16 vert_sync_ਸ਼ੁਰੂ;
    u16 vert_sync_ਸਿਰਾ;
    u8 flags;
    u32 pixclock;
    u16 refresh_ਦਰ;
    u8 reserved[40];
} PACKED;

ਬਣਤਰ vbe_palette_ਇੰਦਰਾਜ {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_RETURN_ਸਥਿਤੀ_SUPPORTED                      0x4F
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_RETURN_ਸਥਿਤੀ_UNSUPPORTED                    0x00
/* AH */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_RETURN_ਸਥਿਤੀ_SUCCESSFULL                    0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_RETURN_ਸਥਿਤੀ_FAILED                         0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_RETURN_ਸਥਿਤੀ_NOT_SUPPORTED                  0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_RETURN_ਸਥਿਤੀ_INVALID                        0x03

/* VBE Mode Numbers */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_VESA_DEFINED                            0x0100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_REFRESH_ਦਰ_USE_CRTC                   0x0800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ਰੇਖਿਕ_ਚੌਖਟਾ_ਬਫਰ                     0x4000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_PRESERVE_DISPLAY_ਸਮ੍ਰਿਤੀ                 0x8000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_VESA_ਬਹੁਲਕ_ਸਿਰਾ_ਦਾ_LIST                        0xFFFF

/* Capabilities */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_CAPABILITY_8_ਦੋਆਧਾਰੀ_ਅੰਕ_DAC                          0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_CAPABILITY_RAMDAC_USE_BLANK_ਦੋਆਧਾਰੀ_ਅੰਕ              0x0004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_CAPABILITY_STEREO_ਰਾਹੀਂ_VESA_EVC               0x0010

/* Mode Attributes */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_SUPPORTED                     0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_EXTENDED_INFORMATION_ਉਪਲਬਧ  0x0002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_COLOR_ਬਹੁਲਕ                    0x0008
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_GRAPHICS_ਬਹੁਲਕ                 0x0010
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_NO_VGA_COMPATIBLE_ਖਿੜਕੀ      0x0040
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_ਰੇਖਿਕ_ਚੌਖਟਾ_ਬਫਰ_ਬਹੁਲਕ      0x0080
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_DOUBLE_ਜਾਂਚ_ਬਹੁਲਕ              0x0100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_INTERLACE_ਬਹੁਲਕ                0x0200
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_HARDWARE_TRIPLE_ਬਫਰ        0x0400
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTRIBUTE_DUAL_DISPLAY_ਸ਼ੁਰੂ_ਪਤਾ    0x1000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਬਹੁਲਕ_ATTTRIBUTE_LFB_ONLY                     ( VBE_ਬਹੁਲਕ_ATTRIBUTE_NO_VGA_COMPATIBLE_ਖਿੜਕੀ | VBE_ਬਹੁਲਕ_ATTRIBUTE_ਰੇਖਿਕ_ਚੌਖਟਾ_ਬਫਰ_ਬਹੁਲਕ )

/* Window attributes */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਖਿੜਕੀ_ATTRIBUTE_RELOCATABLE                 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਖਿੜਕੀ_ATTRIBUTE_READABLE                    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_ਖਿੜਕੀ_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_ਪਾਠ_ਬਹੁਲਕ                        0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_PLANAR                           0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_ਸਿੱਧਾ_COLOR                     0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VBE_DIRECTCOLOR_RESERVED_ਬਿੱਟ_ਉਪਲਬਧ          0x02

#ਜੇ_ਅੰਤ
