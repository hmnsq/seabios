#ifndef __VGA_H
#define __VGA_H
// Standard structure definitions for vgabios video tables

#ضمن "types.h" // u8

// standard BIOS Video Parameter Table
بنية video_param_s {
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
بنية video_save_pointer_s {
    بنية segoff_s videoparam;
    بنية segoff_s paramdynamicsave;
    بنية segoff_s textcharset;
    بنية segoff_s graphcharset;
    بنية segoff_s secsavepointer;
    u8 reserved[8];
} PACKED;

// Data returned by int101B
بنية video_func_static {
    u32 modes;
    u8 reserved_0x04[3];
    u8 scanlines;
    u8 cblocks;
    u8 active_cblocks;
    u16 misc_flags;
    u8 reserved_0x0c[2];
    u8 save_flags;
    u8 reserved_0x0f;
} PACKED;

بنية video_func_info {
    بنية segoff_s static_functionality;
    u8 bda_0x49[30];
    u8 bda_0x84[3];
    u8 dcc_index;
    u8 dcc_alt;
    u16 colors;
    u8 pages;
    u8 scan_ليوك;
    u8 primary_char;
    u8 secondar_char;
    u8 misc;
    u8 non_vga_mode;
    u8 reserved_2f[2];
    u8 video_mem;
    u8 save_flags;
    u8 disp_info;
    u8 reserved_34[12];
} PACKED;

#endif // vga.h
