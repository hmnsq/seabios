#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __VBE_H
#ವ್ಯಾಖ್ಯಾನಿಸು __VBE_H

#ಅನ್ತರ್ಭಾವ "types.h" // u8

#ವ್ಯಾಖ್ಯಾನಿಸು VESA_SIGNATURE 0x41534556 // VESA
#ವ್ಯಾಖ್ಯಾನಿಸು VBE2_SIGNATURE 0x32454256 // VBE2

ರಚನೆ vbe_info {
    u32 signature;
    u16 ಆವೃತ್ತಿ_2;
    ರಚನೆ segoff_s oem_string;
    u32 capabilities;
    ರಚನೆ segoff_s video_ಬಹುಲಕ;
    u16 ಒಟ್ಟು_ಸ್ಮರಣೆ;
    u16 oem_revision;
    ರಚನೆ segoff_s oem_vendor_string;
    ರಚನೆ segoff_s oem_ಗುಣಲಬ್ಧ_string;
    ರಚನೆ segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_ದತ್ತಾಂಶ[256];
} PACKED;

ರಚನೆ vbe_ಬಹುಲಕ_info {
    /* VBE */
    u16 ಬಹುಲಕ_ಗುಣಲಕ್ಷಣಗಳು;
    u8 win_A_ಗುಣಲಕ್ಷಣಗಳು;
    u8 win_B_ಗುಣಲಕ್ಷಣಗಳು;
    u16 win_granularity;
    u16 win_ಗಾತ್ರ;
    u16 winA_seg;
    u16 winB_seg;
    ರಚನೆ segoff_s win_func_ptr;
    u16 ಬೈಟುಗಳು_ಪ್ರತಿ_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_ಗಾತ್ರ;
    u8 ಪುಟಗಳು;
    u8 reserved0;
    /* Direct Color */
    u8 red_ಗಾತ್ರ;
    u8 red_pos;
    u8 green_ಗಾತ್ರ;
    u8 green_pos;
    u8 blue_ಗಾತ್ರ;
    u8 blue_pos;
    u8 alpha_ಗಾತ್ರ;
    u8 alpha_pos;
    u8 directcolor_info;
    /* VBE 2.0 */
    u32 phys_ತಳ;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 ರೇಖೀಯ_ಬೈಟುಗಳು_ಪ್ರತಿ_scanline;
    u8 bank_ಪುಟಗಳು;
    u8 ರೇಖೀಯ_ಪುಟಗಳು;
    u8 ರೇಖೀಯ_red_ಗಾತ್ರ;
    u8 ರೇಖೀಯ_red_pos;
    u8 ರೇಖೀಯ_green_ಗಾತ್ರ;
    u8 ರೇಖೀಯ_green_pos;
    u8 ರೇಖೀಯ_blue_ಗಾತ್ರ;
    u8 ರೇಖೀಯ_blue_pos;
    u8 ರೇಖೀಯ_alpha_ಗಾತ್ರ;
    u8 ರೇಖೀಯ_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

ರಚನೆ vbe_crtc_info {
    u16 horiz_ಒಟ್ಟು;
    u16 horiz_sync_ಆರಂಭ;
    u16 horiz_sync_ಕೊನೆ;
    u16 vert_ಒಟ್ಟು;
    u16 vert_sync_ಆರಂಭ;
    u16 vert_sync_ಕೊನೆ;
    u8 flags;
    u32 pixclock;
    u16 refresh_ದರ;
    u8 reserved[40];
} PACKED;

ರಚನೆ vbe_palette_ನಮೂದು {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_RETURN_ಸ್ಥಿತಿ_SUPPORTED                      0x4F
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_RETURN_ಸ್ಥಿತಿ_UNSUPPORTED                    0x00
/* AH */
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_RETURN_ಸ್ಥಿತಿ_SUCCESSFULL                    0x00
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_RETURN_ಸ್ಥಿತಿ_FAILED                         0x01
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_RETURN_ಸ್ಥಿತಿ_NOT_SUPPORTED                  0x02
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_RETURN_ಸ್ಥಿತಿ_INVALID                        0x03

/* VBE Mode Numbers */

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_VESA_DEFINED                            0x0100
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_REFRESH_ದರ_USE_CRTC                   0x0800
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ರೇಖೀಯ_ಚೌಕಟ್ಟು_ಬಫರ್                     0x4000
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_PRESERVE_DISPLAY_ಸ್ಮರಣೆ                 0x8000

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_VESA_ಬಹುಲಕ_ಕೊನೆ_ದ_LIST                        0xFFFF

/* Capabilities */

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_CAPABILITY_8_ಬಿಟ್_DAC                          0x0001
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_CAPABILITY_RAMDAC_USE_BLANK_ಬಿಟ್              0x0004
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_CAPABILITY_STEREO_ಮೂಲಕ_VESA_EVC               0x0010

/* Mode Attributes */

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_SUPPORTED                     0x0001
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_EXTENDED_INFORMATION_ಲಭ್ಯ  0x0002
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_COLOR_ಬಹುಲಕ                    0x0008
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_GRAPHICS_ಬಹುಲಕ                 0x0010
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_NO_VGA_COMPATIBLE_ಕಿಟಕಿ      0x0040
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_ರೇಖೀಯ_ಚೌಕಟ್ಟು_ಬಫರ್_ಬಹುಲಕ      0x0080
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_DOUBLE_ಪರಿಶೀಲನೆ_ಬಹುಲಕ              0x0100
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_INTERLACE_ಬಹುಲಕ                0x0200
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_HARDWARE_TRIPLE_ಬಫರ್        0x0400
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTRIBUTE_DUAL_DISPLAY_ಆರಂಭ_ವಿಳಾಸ    0x1000

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಬಹುಲಕ_ATTTRIBUTE_LFB_ONLY                     ( VBE_ಬಹುಲಕ_ATTRIBUTE_NO_VGA_COMPATIBLE_ಕಿಟಕಿ | VBE_ಬಹುಲಕ_ATTRIBUTE_ರೇಖೀಯ_ಚೌಕಟ್ಟು_ಬಫರ್_ಬಹುಲಕ )

/* Window attributes */

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಕಿಟಕಿ_ATTRIBUTE_RELOCATABLE                 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಕಿಟಕಿ_ATTRIBUTE_READABLE                    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_ಕಿಟಕಿ_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_ಪಠ್ಯ_ಬಹುಲಕ                        0x00
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_PLANAR                           0x03
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_ನೇರ_COLOR                     0x06
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#ವ್ಯಾಖ್ಯಾನಿಸು VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#ವ್ಯಾಖ್ಯಾನಿಸು VBE_DIRECTCOLOR_RESERVED_ಬಿಟ್ಟುಗಳು_ಲಭ್ಯ          0x02

#ಆದರೆ_ಅಂತ್ಯ
