// BIOS Data Area (and similar) definitions
#جيڪڏهن_وصف_نه_ٿيل __BDA_H
#وصف_ڪريو __BDA_H

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

بنية bios_معلومات_area_s {
    // 40:00
    u16 پورٽ_com[4];
    u16 پورٽ_lpt[3];
    u16 ebda_seg;
    // 40:10
    u16 equipment_list_flags;
    u8 pad1;
    u16 mem_سائيز_kb;
    u8 pad2;
    u8 ps2_ctrl_flag;
    u16 kbd_flag0;
    u8 alt_keypad;
    u16 kbd_buf_سر;
    u16 kbd_buf_پڇ;
    // 40:1e
    u8 kbd_buf[32];
    u8 floppy_recalibration_حالت;
    u8 floppy_motor_حالت;
    // 40:40
    u8 floppy_motor_counter;
    u8 floppy_آخري_حالت;
    u8 floppy_return_حالت[7];
    u8 video_گهڻائي;
    u16 video_کانن_ڳڻپ;
    u16 video_pagesize;
    u16 video_pagestart;
    // 40:50
    u16 cursor_pos[8];
    // 40:60
    u16 cursor_type;
    u8 video_صفحو;
    u16 crtc_پتو;
    u8 video_msr;
    u8 video_pal;
    بنية segoff_s jump;
    u8 other_6b;
    u32 timer_counter;
    // 40:70
    u8 timer_rollover;
    u8 break_flag;
    u16 soft_reset_flag;
    u8 disk_آخري_حالت;
    u8 hdcount;
    u8 disk_control_byte;
    u8 پورٽ_disk;
    u8 lpt_timeout[4];
    u8 com_timeout[4];
    // 40:80
    u16 kbd_buf_شروعات_وٿي;
    u16 kbd_buf_پڇاڙي_وٿي;
    u8 video_صفون;
    u16 char_اوچائي;
    u8 video_ctl;
    u8 video_switches;
    u8 modeset_ctl;
    u8 dcc_اشاريو;
    u8 floppy_آخري_معلومات_شرح;
    u8 disk_حالت_controller;
    u8 disk_error_controller;
    u8 disk_مداخلت_flag;
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
#وصف_ڪريو FRS_IRQ (1<<7)

// BDA rtc_wait_flag bitdefs
#وصف_ڪريو RWS_WAIT_PENDING (1<<0)
#وصف_ڪريو RWS_WAIT_ELAPSED (1<<7)

// BDA floppy_media_state bitdefs
#وصف_ڪريو FMS_DRIVE_STATE_نقاب        (0x07)
#وصف_ڪريو FMS_MEDIA_DRIVE_ESTABLISHED (1<<4)
#وصف_ڪريو FMS_DOUBLE_STEPPING         (1<<5)
#وصف_ڪريو FMS_معلومات_شرح_نقاب          (0xc0)

// BDA kbd_flag[01] bitdefs
#وصف_ڪريو KF0_RSHIFT       (1<<0)
#وصف_ڪريو KF0_LSHIFT       (1<<1)
#وصف_ڪريو KF0_CTRLACTIVE   (1<<2)
#وصف_ڪريو KF0_ALTACTIVE    (1<<3)
#وصف_ڪريو KF0_SCROLLACTIVE (1<<4)
#وصف_ڪريو KF0_NUMACTIVE    (1<<5)
#وصف_ڪريو KF0_CAPSACTIVE   (1<<6)
#وصف_ڪريو KF0_LCTRL        (1<<8)
#وصف_ڪريو KF0_LALT         (1<<9)
#وصف_ڪريو KF0_PAUSEACTIVE  (1<<11)
#وصف_ڪريو KF0_SCROLL       (1<<12)
#وصف_ڪريو KF0_انگ          (1<<13)
#وصف_ڪريو KF0_CAPS         (1<<14)

#وصف_ڪريو KF1_آخري_E1    (1<<0)
#وصف_ڪريو KF1_آخري_E0    (1<<1)
#وصف_ڪريو KF1_RCTRL      (1<<2)
#وصف_ڪريو KF1_RALT       (1<<3)
#وصف_ڪريو KF1_101KBD     (1<<4)

// Limit of BDA timer_counter field
#وصف_ڪريو TICKS_في_DAY 1573040


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

بنية extended_bios_معلومات_area_s {
    u8 سائيز;
    u8 reserved1[0x21];
    بنية segoff_s far_call_اشارو;
    u8 mouse_flag1;
    u8 mouse_flag2;
    u8 mouse_معلومات[0x08];
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
    u16 سائيز;
    u8 model;
    u8 submodel;
    u8 biosrev;
    u8 feature1, feature2, feature3, feature4, feature5;
} PACKED;

#جيڪڏهن_پورو // bda.h
