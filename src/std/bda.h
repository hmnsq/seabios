// BIOS Data Area (and similar) definitions
#اگر_تعریف_نشده __BDA_H
#تعریف_کن __BDA_H

#ضمن "disk.h" // struct fdpt_s
#ضمن "types.h" // u8


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

بنية rmode_IVT {
    بنية segoff_s ivec[256];
};


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

بنية bios_داده_area_s {
    // 40:00
    u16 پورت_com[4];
    u16 پورت_lpt[3];
    u16 ebda_seg;
    // 40:10
    u16 equipment_list_flags;
    u8 pad1;
    u16 mem_اندازه_kb;
    u8 pad2;
    u8 ps2_ctrl_flag;
    u16 kbd_flag0;
    u8 alt_keypad;
    u16 kbd_buf_سر;
    u16 kbd_buf_دم;
    // 40:1e
    u8 kbd_buf[32];
    u8 floppy_recalibration_وضعیت;
    u8 floppy_motor_وضعیت;
    // 40:40
    u8 floppy_motor_counter;
    u8 floppy_آخرین_وضعیت;
    u8 floppy_return_وضعیت[7];
    u8 video_نما;
    u16 video_تعداد_ستون;
    u16 video_pagesize;
    u16 video_pagestart;
    // 40:50
    u16 cursor_pos[8];
    // 40:60
    u16 cursor_type;
    u8 video_صفحه;
    u16 crtc_آدرس;
    u8 video_msr;
    u8 video_pal;
    بنية segoff_s jump;
    u8 other_6b;
    u32 timer_counter;
    // 40:70
    u8 timer_rollover;
    u8 break_flag;
    u16 soft_reset_flag;
    u8 disk_آخرین_وضعیت;
    u8 hdcount;
    u8 disk_control_byte;
    u8 پورت_disk;
    u8 lpt_timeout[4];
    u8 com_timeout[4];
    // 40:80
    u16 kbd_buf_شروع_جابه_جایی;
    u16 kbd_buf_انتها_جابه_جایی;
    u8 video_ردیفها;
    u16 char_ارتفاع;
    u8 video_ctl;
    u8 video_switches;
    u8 modeset_ctl;
    u8 dcc_شاخص;
    u8 floppy_آخرین_داده_نرخ;
    u8 disk_وضعیت_controller;
    u8 disk_error_controller;
    u8 disk_وقفه_flag;
    u8 floppy_harddisk_info;
    // 40:90
    u8 floppy_media_state[4];
    u8 floppy_track[2];
    u8 kbd_flag1;
    u8 kbd_led;
    بنية segoff_s user_wait_complete_flag;
    u32 user_wait_timeout;
    // 40:A0
    u8 rtc_wait_flag;
    u8 other_a1[7];
    بنية segoff_s video_savetable;
    u8 other_ac[4];
    // 40:B0
    u8 other_b0[5*16];
} PACKED;

// BDA floppy_recalibration_status bitdefs
#تعریف_کن FRS_IRQ (1<<7)

// BDA rtc_wait_flag bitdefs
#تعریف_کن RWS_WAIT_PENDING (1<<0)
#تعریف_کن RWS_WAIT_ELAPSED (1<<7)

// BDA floppy_media_state bitdefs
#تعریف_کن FMS_DRIVE_STATE_ماسک        (0x07)
#تعریف_کن FMS_MEDIA_DRIVE_ESTABLISHED (1<<4)
#تعریف_کن FMS_DOUBLE_STEPPING         (1<<5)
#تعریف_کن FMS_داده_نرخ_ماسک          (0xc0)

// BDA kbd_flag[01] bitdefs
#تعریف_کن KF0_RSHIFT       (1<<0)
#تعریف_کن KF0_LSHIFT       (1<<1)
#تعریف_کن KF0_CTRLACTIVE   (1<<2)
#تعریف_کن KF0_ALTACTIVE    (1<<3)
#تعریف_کن KF0_SCROLLACTIVE (1<<4)
#تعریف_کن KF0_NUMACTIVE    (1<<5)
#تعریف_کن KF0_CAPSACTIVE   (1<<6)
#تعریف_کن KF0_LCTRL        (1<<8)
#تعریف_کن KF0_LALT         (1<<9)
#تعریف_کن KF0_PAUSEACTIVE  (1<<11)
#تعریف_کن KF0_SCROLL       (1<<12)
#تعریف_کن KF0_رقم          (1<<13)
#تعریف_کن KF0_CAPS         (1<<14)

#تعریف_کن KF1_آخرین_E1    (1<<0)
#تعریف_کن KF1_آخرین_E0    (1<<1)
#تعریف_کن KF1_RCTRL      (1<<2)
#تعریف_کن KF1_RALT       (1<<3)
#تعریف_کن KF1_101KBD     (1<<4)

// Limit of BDA timer_counter field
#تعریف_کن TICKS_در_هر_DAY 1573040


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

بنية extended_bios_داده_area_s {
    u8 اندازه;
    u8 reserved1[0x21];
    بنية segoff_s far_call_اشاره_گر;
    u8 mouse_flag1;
    u8 mouse_flag2;
    u8 mouse_داده[0x08];
    // 0x30
    u8 other1[0x0d];

    // 0x3d
    بنية fdpt_s fdpt[2];

    // 0x5d
    u8 other2[0xC4];

    // 0x121 - Begin custom storage.
} PACKED;


/****************************************************************
 * Bios Config Table
 ****************************************************************/

بنية bios_config_جدول_s {
    u16 اندازه;
    u8 model;
    u8 submodel;
    u8 biosrev;
    u8 feature1, feature2, feature3, feature4, feature5;
} PACKED;

#پایان_اگر // bda.h
