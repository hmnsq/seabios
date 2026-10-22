#నిర్వచితం_కాకపోతే __VBE_H
#నిర్వచించు __VBE_H

#అన్తర్భావ "types.h" // u8

#నిర్వచించు VESA_SIGNATURE 0x41534556 // VESA
#నిర్వచించు VBE2_SIGNATURE 0x32454256 // VBE2

నిర్మాణం vbe_info {
    u32 signature;
    u16 సంస్కరణ_2;
    నిర్మాణం segoff_s oem_string;
    u32 capabilities;
    నిర్మాణం segoff_s video_బహుళకం;
    u16 మొత్తం_జ్ఞాపకం;
    u16 oem_revision;
    నిర్మాణం segoff_s oem_vendor_string;
    నిర్మాణం segoff_s oem_లబ్ధం_string;
    నిర్మాణం segoff_s oem_revision_string;
    u8 reserved[222];
    /* VBE 2.0 */
    u8 oem_దత్తాంశం[256];
} PACKED;

నిర్మాణం vbe_బహుళకం_info {
    /* VBE */
    u16 బహుళకం_లక్షణాలు;
    u8 win_A_లక్షణాలు;
    u8 win_B_లక్షణాలు;
    u16 win_granularity;
    u16 win_కొలత;
    u16 winA_seg;
    u16 winB_seg;
    నిర్మాణం segoff_s win_func_ptr;
    u16 బైట్లు_చొప్పున_scanline;
    /* VBE 1.2 */
    u16 xres;
    u16 yres;
    u8 xcharsize;
    u8 ycharsize;
    u8 planes;
    u8 బిట్లు_చొప్పున_pixel;
    u8 banks;
    u8 mem_model;
    u8 bank_కొలత;
    u8 పుటలు;
    u8 reserved0;
    /* Direct Color */
    u8 red_కొలత;
    u8 red_pos;
    u8 green_కొలత;
    u8 green_pos;
    u8 blue_కొలత;
    u8 blue_pos;
    u8 alpha_కొలత;
    u8 alpha_pos;
    u8 directcolor_info;
    /* VBE 2.0 */
    u32 phys_పునాది;
    u32 reserved1;
    u16 reserved2;
    /* VBE 3.0 */
    u16 రేఖీయ_బైట్లు_చొప్పున_scanline;
    u8 bank_పుటలు;
    u8 రేఖీయ_పుటలు;
    u8 రేఖీయ_red_కొలత;
    u8 రేఖీయ_red_pos;
    u8 రేఖీయ_green_కొలత;
    u8 రేఖీయ_green_pos;
    u8 రేఖీయ_blue_కొలత;
    u8 రేఖీయ_blue_pos;
    u8 రేఖీయ_alpha_కొలత;
    u8 రేఖీయ_alpha_pos;
    u32 pixclock_max;
    u8 reserved[190];
} PACKED;

నిర్మాణం vbe_crtc_info {
    u16 horiz_మొత్తం;
    u16 horiz_sync_ప్రారంభం;
    u16 horiz_sync_చివర;
    u16 vert_మొత్తం;
    u16 vert_sync_ప్రారంభం;
    u16 vert_sync_చివర;
    u8 flags;
    u32 pixclock;
    u16 refresh_రేటు;
    u8 reserved[40];
} PACKED;

నిర్మాణం vbe_palette_నమోదు {
    u8 blue;
    u8 green;
    u8 red;
    u8 align;
} PACKED;

/* VBE Return Status Info */
/* AL */
#నిర్వచించు VBE_RETURN_స్థితి_SUPPORTED                      0x4F
#నిర్వచించు VBE_RETURN_స్థితి_UNSUPPORTED                    0x00
/* AH */
#నిర్వచించు VBE_RETURN_స్థితి_SUCCESSFULL                    0x00
#నిర్వచించు VBE_RETURN_స్థితి_FAILED                         0x01
#నిర్వచించు VBE_RETURN_స్థితి_NOT_SUPPORTED                  0x02
#నిర్వచించు VBE_RETURN_స్థితి_INVALID                        0x03

/* VBE Mode Numbers */

#నిర్వచించు VBE_బహుళకం_VESA_DEFINED                            0x0100
#నిర్వచించు VBE_బహుళకం_REFRESH_రేటు_USE_CRTC                   0x0800
#నిర్వచించు VBE_బహుళకం_రేఖీయ_చట్రం_బఫరు                     0x4000
#నిర్వచించు VBE_బహుళకం_PRESERVE_DISPLAY_జ్ఞాపకం                 0x8000

#నిర్వచించు VBE_VESA_బహుళకం_చివర_యొక్క_LIST                        0xFFFF

/* Capabilities */

#నిర్వచించు VBE_CAPABILITY_8_బిట్టు_DAC                          0x0001
#నిర్వచించు VBE_CAPABILITY_NOT_VGA_COMPATIBLE                0x0002
#నిర్వచించు VBE_CAPABILITY_RAMDAC_USE_BLANK_బిట్టు              0x0004
#నిర్వచించు VBE_CAPABILITY_STEREOSCOPIC_SUPPORT              0x0008
#నిర్వచించు VBE_CAPABILITY_STEREO_ద్వారా_VESA_EVC               0x0010

/* Mode Attributes */

#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_SUPPORTED                     0x0001
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_EXTENDED_INFORMATION_అందుబాటు  0x0002
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_TTY_BIOS_SUPPORT              0x0004
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_COLOR_బహుళకం                    0x0008
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_GRAPHICS_బహుళకం                 0x0010
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_NOT_VGA_COMPATIBLE            0x0020
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_NO_VGA_COMPATIBLE_కిటికీ      0x0040
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_రేఖీయ_చట్రం_బఫరు_బహుళకం      0x0080
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_DOUBLE_పరిశీలన_బహుళకం              0x0100
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_INTERLACE_బహుళకం                0x0200
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_HARDWARE_TRIPLE_బఫరు        0x0400
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_HARDWARE_STEREOSCOPIC_DISPLAY 0x0800
#నిర్వచించు VBE_బహుళకం_ATTRIBUTE_DUAL_DISPLAY_ప్రారంభం_చిరునామా    0x1000

#నిర్వచించు VBE_బహుళకం_ATTTRIBUTE_LFB_ONLY                     ( VBE_బహుళకం_ATTRIBUTE_NO_VGA_COMPATIBLE_కిటికీ | VBE_బహుళకం_ATTRIBUTE_రేఖీయ_చట్రం_బఫరు_బహుళకం )

/* Window attributes */

#నిర్వచించు VBE_కిటికీ_ATTRIBUTE_RELOCATABLE                 0x01
#నిర్వచించు VBE_కిటికీ_ATTRIBUTE_READABLE                    0x02
#నిర్వచించు VBE_కిటికీ_ATTRIBUTE_WRITEABLE                   0x04

/* Memory model */

#నిర్వచించు VBE_MEMORYMODEL_పాఠ్యం_బహుళకం                        0x00
#నిర్వచించు VBE_MEMORYMODEL_CGA_GRAPHICS                     0x01
#నిర్వచించు VBE_MEMORYMODEL_HERCULES_GRAPHICS                0x02
#నిర్వచించు VBE_MEMORYMODEL_PLANAR                           0x03
#నిర్వచించు VBE_MEMORYMODEL_PACKED_PIXEL                     0x04
#నిర్వచించు VBE_MEMORYMODEL_NON_CHAIN_4_256                  0x05
#నిర్వచించు VBE_MEMORYMODEL_ప్రత్యక్ష_COLOR                     0x06
#నిర్వచించు VBE_MEMORYMODEL_YUV                              0x07

/* DirectColorModeInfo */

#నిర్వచించు VBE_DIRECTCOLOR_COLOR_RAMP_PROGRAMMABLE          0x01
#నిర్వచించు VBE_DIRECTCOLOR_RESERVED_బిట్లు_అందుబాటు          0x02

#అయితే_ముగింపు
