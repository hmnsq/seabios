#अगर_अघोषित __VBE_H
#घोषणा __VBE_H

#समावेश "types.h" // u8

#घोषणा VESA_SIGNATURE 0x41534556 // VESA
#घोषणा VBE2_SIGNATURE 0x32454256 // VBE2

काष्ठा vbe_info {
    u32 signature;
    u16 version;
    काष्ठा segoff_s oem_string;
    u32 capabilities;
    काष्ठा segoff_s video_mode;
    u16 योग_memory;
    u16 oem_revision;
    काष्ठा segoff_s oem_vendor_string;
    काष्ठा segoff_s oem_product_string;
    काष्ठा segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_तथ्य[256];
} PACKED;

काष्ठा vbe_mode_info {
    /* VBE */
    u16 mode_attributes;
    u8 winA_attributes;
    u8 winB_attributes;
    u16 win_granularity;
    u16 win_आकार;
    u16 winA_seg;
    u16 winB_seg;
    काष्ठा segoff_s win_func_ptr;
    u16 bytes_per_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 bits_per_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_आकार;
    u8 pages;
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
    u32 phys_base;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 linear_bytes_per_scanline;
    u8 bank_pages;
    u8 linear_pages;
    u8 linear_red_आकार;
    u8 linear_red_pos;
    u8 linear_green_आकार;
    u8 linear_green_pos;
    u8 linear_blue_आकार;
    u8 linear_blue_pos;
    u8 linear_alpha_आकार;
    u8 linear_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

काष्ठा vbe_crtc_info {
    u16 horiz_योग;
    u16 horiz_sync_start;
    u16 horiz_sync_end;
    u16 vert_योग;
    u16 vert_sync_start;
    u16 vert_sync_end;
    u8 flags;
    u32 pixclock;
    u16 refresh_rate;
    u8 reserved[40];
} PACKED;

काष्ठा vbe_palette_entry {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#घोषणा VBE_RETURN_STATUS_SUPPORTED                      0x4F
#घोषणा VBE_RETURN_STATUS_UNSUPPORTED                    0x00
/* AH */
#घोषणा VBE_RETURN_STATUS_SUCCESSFULL                    0x00
#घोषणा VBE_RETURN_STATUS_FAILED                         0x01
#घोषणा VBE_RETURN_STATUS_NOT_SUPPORTED                  0x02
#घोषणा VBE_RETURN_STATUS_INVALID                        0x03

/* VBE Mode Numbers */

#घोषणा VBE_MODE_VESA_DEFINED                            0x0100
#घोषणा VBE_MODE_REFRESH_RATE_USE_CRTC                   0x0800
#घोषणा VBE_MODE_LINEAR_FRAME_बफर                     0x4000
#घोषणा VBE_MODE_PRESERVE_DISPLAY_MEMORY                 0x8000

#घोषणा VBE_VESA_MODE_END_OF_LIST                        0xFFFF

/* Capabilities */

#घोषणा VBE_CAPABILITY_8BIT_DAC                          0x0001
#घोषणा VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#घोषणा VBE_CAPABILITY_RAMDAC_USE_BLANK_BIT              0x0004
#घोषणा VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#घोषणा VBE_CAPABILITY_STEREO_VIA_VESA_EVC               0x0010

/* Mode Attributes */

#घोषणा VBE_MODE_ATTRIBUTE_SUPPORTED                     0x0001
#घोषणा VBE_MODE_ATTRIBUTE_EXTENDED_INFORMATION_AVAILABLE  0x0002
#घोषणा VBE_MODE_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#घोषणा VBE_MODE_ATTRIBUTE_COLOR_MODE                    0x0008
#घोषणा VBE_MODE_ATTRIBUTE_GRAPHICS_MODE                 0x0010
#घोषणा VBE_MODE_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#घोषणा VBE_MODE_ATTRIBUTE_NO_VGA_COMPATIBLE_WINDOW      0x0040
#घोषणा VBE_MODE_ATTRIBUTE_LINEAR_FRAME_बफर_MODE      0x0080
#घोषणा VBE_MODE_ATTRIBUTE_DOUBLE_SCAN_MODE              0x0100
#घोषणा VBE_MODE_ATTRIBUTE_INTERLACE_MODE                0x0200
#घोषणा VBE_MODE_ATTRIBUTE_HARDWARE_TRIPLE_बफर        0x0400
#घोषणा VBE_MODE_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#घोषणा VBE_MODE_ATTRIBUTE_DUAL_DISPLAY_START_ADDRESS    0x1000

#घोषणा VBE_MODE_ATTTRIBUTE_LFB_ONLY                     ( VBE_MODE_ATTRIBUTE_NO_VGA_COMPATIBLE_WINDOW | VBE_MODE_ATTRIBUTE_LINEAR_FRAME_बफर_MODE )

/* Window attributes */

#घोषणा VBE_WINDOW_ATTRIBUTE_RELOCATABLE                 0x01
#घोषणा VBE_WINDOW_ATTRIBUTE_READABLE                    0x02
#घोषणा VBE_WINDOW_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#घोषणा VBE_MEMORYMODEL_TEXT_MODE                        0x00
#घोषणा VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#घोषणा VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#घोषणा VBE_MEMORYMODEL_PLANAR                           0x03
#घोषणा VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#घोषणा VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#घोषणा VBE_MEMORYMODEL_DIRECT_COLOR                     0x06
#घोषणा VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#घोषणा VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#घोषणा VBE_DIRECTCOLOR_RESERVED_BITS_AVAILABLE          0x02

#पूर्ण_अगर
