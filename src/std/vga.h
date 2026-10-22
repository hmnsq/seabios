#اگر_غیر_تعریف_شدہ __VGA_H
#تعریف_کرو __VGA_H
// Standard structure definitions for vgabios video tables

#شامل "types.h" // u8

// standard BIOS Video Parameter Table
ساخت video_param_s {
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
ساخت video_save_اشارہ_گر_s {
    ساخت segoff_s videoparam;
    ساخت segoff_s paramdynamicsave;
    ساخت segoff_s textcharset;
    ساخت segoff_s graphcharset;
    ساخت segoff_s secsavepointer;
    u8 reserved[8];
} PACKED;

// Data returned by int101B
ساخت video_func_static {
    u32 modes;
    u8 reserved_0x04[3];
    u8 scanlines;
    u8 cblocks;
    u8 فعال_cblocks;
    u16 misc_flags;
    u8 reserved_0x0c[2];
    u8 save_flags;
    u8 reserved_0x0f;
} PACKED;

ساخت video_func_info {
    ساخت segoff_s static_functionality;
    u8 bda_0x49[30];
    u8 bda_0x84[3];
    u8 dcc_اشاریہ;
    u8 dcc_alt;
    u16 colors;
    u8 صفحات;
    u8 جائزہ_سطریں;
    u8 primary_char;
    u8 secondar_char;
    u8 misc;
    u8 non_vga_منوال;
    u8 reserved_2f[2];
    u8 video_mem;
    u8 save_flags;
    u8 disp_info;
    u8 reserved_34[12];
} PACKED;

#اگر_ختم // vga.h
