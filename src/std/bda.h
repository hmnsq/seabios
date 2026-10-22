// BIOS Data Area (and similar) definitions
#اگر_غیر_تعریف_شدہ __BDA_H
#تعریف_کرو __BDA_H

#شامل "disk.h" // struct fdpt_s
#شامل "types.h" // u8


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

ساخت rmode_IVT {
    ساخت segoff_s ivec[256];
};


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

ساخت bios_ڈیٹا_area_s {
    // 40:00
    u16 پورٹ_com[4];
    u16 پورٹ_lpt[3];
    u16 ebda_seg;
    // 40:10
    u16 equipment_list_flags;
    u8 pad1;
    u16 mem_سائز_kb;
    u8 pad2;
    u8 ps2_ctrl_flag;
    u16 kbd_flag0;
    u8 alt_keypad;
    u16 kbd_buf_سر;
    u16 kbd_buf_دم;
    // 40:1e
    u8 kbd_buf[32];
    u8 floppy_recalibration_حالت;
    u8 floppy_motor_حالت;
    // 40:40
    u8 floppy_motor_شمار;
    u8 floppy_آخری_حالت;
    u8 floppy_return_حالت[7];
    u8 video_منوال;
    u16 video_ستون_تعداد;
    u16 video_pagesize;
    u16 video_pagestart;
    // 40:50
    u16 cursor_pos[8];
    // 40:60
    u16 cursor_type;
    u8 video_صفحہ;
    u16 crtc_پتہ;
    u8 video_msr;
    u8 video_pal;
    ساخت segoff_s jump;
    u8 other_6b;
    u32 timer_شمار;
    // 40:70
    u8 timer_rollover;
    u8 break_flag;
    u16 soft_reset_flag;
    u8 disk_آخری_حالت;
    u8 hdcount;
    u8 disk_control_byte;
    u8 پورٹ_disk;
    u8 lpt_timeout[4];
    u8 com_timeout[4];
    // 40:80
    u16 kbd_buf_آغاز_انحراف;
    u16 kbd_buf_انتہا_انحراف;
    u8 video_صفیں;
    u16 char_اونچائی;
    u8 video_ctl;
    u8 video_switches;
    u8 modeset_ctl;
    u8 dcc_اشاریہ;
    u8 floppy_آخری_ڈیٹا_شرح;
    u8 disk_حالت_controller;
    u8 disk_error_controller;
    u8 disk_مداخلت_flag;
    u8 floppy_harddisk_info;
    // 40:90
    u8 floppy_media_state[4];
    u8 floppy_track[2];
    u8 kbd_flag1;
    u8 kbd_led;
    ساخت segoff_s user_wait_complete_flag;
    u32 user_wait_timeout;
    // 40:A0
    u8 rtc_wait_flag;
    u8 other_a1[7];
    ساخت segoff_s video_savetable;
    u8 other_ac[4];
    // 40:B0
    u8 other_b0[5*16];
} PACKED;

// BDA floppy_recalibration_status bitdefs
#تعریف_کرو FRS_IRQ (1<<7)

// BDA rtc_wait_flag bitdefs
#تعریف_کرو RWS_WAIT_PENDING (1<<0)
#تعریف_کرو RWS_WAIT_ELAPSED (1<<7)

// BDA floppy_media_state bitdefs
#تعریف_کرو FMS_DRIVE_STATE_نقاب        (0x07)
#تعریف_کرو FMS_MEDIA_DRIVE_ESTABLISHED (1<<4)
#تعریف_کرو FMS_DOUBLE_STEPPING         (1<<5)
#تعریف_کرو FMS_ڈیٹا_شرح_نقاب          (0xc0)

// BDA kbd_flag[01] bitdefs
#تعریف_کرو KF0_RSHIFT       (1<<0)
#تعریف_کرو KF0_LSHIFT       (1<<1)
#تعریف_کرو KF0_CTRLACTIVE   (1<<2)
#تعریف_کرو KF0_ALTACTIVE    (1<<3)
#تعریف_کرو KF0_SCROLLACTIVE (1<<4)
#تعریف_کرو KF0_NUMACTIVE    (1<<5)
#تعریف_کرو KF0_CAPSACTIVE   (1<<6)
#تعریف_کرو KF0_LCTRL        (1<<8)
#تعریف_کرو KF0_LALT         (1<<9)
#تعریف_کرو KF0_PAUSEACTIVE  (1<<11)
#تعریف_کرو KF0_SCROLL       (1<<12)
#تعریف_کرو KF0_عدد          (1<<13)
#تعریف_کرو KF0_CAPS         (1<<14)

#تعریف_کرو KF1_آخری_E1    (1<<0)
#تعریف_کرو KF1_آخری_E0    (1<<1)
#تعریف_کرو KF1_RCTRL      (1<<2)
#تعریف_کرو KF1_RALT       (1<<3)
#تعریف_کرو KF1_101KBD     (1<<4)

// Limit of BDA timer_counter field
#تعریف_کرو TICKS_فی_DAY 1573040


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

ساخت extended_bios_ڈیٹا_area_s {
    u8 سائز;
    u8 reserved1[0x21];
    ساخت segoff_s far_call_اشارہ_گر;
    u8 mouse_flag1;
    u8 mouse_flag2;
    u8 mouse_ڈیٹا[0x08];
    // 0x30
    u8 other1[0x0d];

    // 0x3d
    ساخت fdpt_s fdpt[2];

    // 0x5d
    u8 other2[0xC4];

    // 0x121 - Begin custom storage.
} PACKED;


/****************************************************************
 * Bios Config Table
 ****************************************************************/

ساخت bios_config_جدول_s {
    u16 سائز;
    u8 model;
    u8 submodel;
    u8 biosrev;
    u8 feature1, feature2, feature3, feature4, feature5;
} PACKED;

#اگر_ختم // bda.h
