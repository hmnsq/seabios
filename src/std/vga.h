#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __VGA_H
#ವ್ಯಾಖ್ಯಾನಿಸು __VGA_H
// Standard structure definitions for vgabios video tables

#ಅನ್ತರ್ಭಾವ "types.h" // u8

// standard BIOS Video Parameter Table
ರಚನೆ video_param_s {
    u8 twidth;
    u8 theightm1;
    u8 cheight;
    u16 slength;
    u8 sequ_regs[4];
    u8 miscreg;
    u8 crtc_regs[25];
    u8 actl_regs[20];
    u8 grdc_regs[9];
} PACKED;

// Standard Video Save Pointer Table
ರಚನೆ video_save_ಸೂಚಕ_s {
    ರಚನೆ segoff_s videoparam;
    ರಚನೆ segoff_s paramdynamicsave;
    ರಚನೆ segoff_s textcharset;
    ರಚನೆ segoff_s graphcharset;
    ರಚನೆ segoff_s secsavepointer;
    u8 reserved[8];
} PACKED;

// Data returned by int101B
ರಚನೆ video_func_static {
    u32 modes;
    u8 reserved_0x04[3];
    u8 scanlines;
    u8 cblocks;
    u8 ಸಕ್ರಿಯ_cblocks;
    u16 misc_flags;
    u8 reserved_0x0c[2];
    u8 save_flags;
    u8 reserved_0x0f;
} PACKED;

ರಚನೆ video_func_info {
    ರಚನೆ segoff_s static_functionality;
    u8 bda_0x49[30];
    u8 bda_0x84[3];
    u8 dcc_ಸೂಚ್ಯಂಕ;
    u8 dcc_alt;
    u16 colors;
    u8 ಪುಟಗಳು;
    u8 ಪರಿಶೀಲನೆ_ಸಾಲುಗಳು;
    u8 primary_char;
    u8 secondar_char;
    u8 misc;
    u8 non_vga_ಬಹುಲಕ;
    u8 reserved_2f[2];
    u8 video_mem;
    u8 save_flags;
    u8 disp_info;
    u8 reserved_34[12];
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // vga.h
