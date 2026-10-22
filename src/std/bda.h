// BIOS Data Area (and similar) definitions
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __BDA_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __BDA_H

#ਅਨ੍ਤਰ੍ਭਾਵ "disk.h" // struct fdpt_s
#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

ਬਣਤਰ rmode_IVT {
    ਬਣਤਰ segoff_s ivec[256];
};


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

ਬਣਤਰ bios_ਅੰਕੜੇ_area_s {
    // 40:00
    u16 ਪੋਰਟ_com[4];
    u16 ਪੋਰਟ_lpt[3];
    u16 ebda_seg;
    // 40:10
    u16 equipment_list_flags;
    u8 pad1;
    u16 mem_ਮਾਪ_kb;
    u8 pad2;
    u8 ps2_ctrl_flag;
    u16 kbd_flag0;
    u8 alt_keypad;
    u16 kbd_buf_ਸਿਰ;
    u16 kbd_buf_ਪੂਛ;
    // 40:1e
    u8 kbd_buf[32];
    u8 floppy_recalibration_ਸਥਿਤੀ;
    u8 floppy_motor_ਸਥਿਤੀ;
    // 40:40
    u8 floppy_motor_counter;
    u8 floppy_ਆਖਰੀ_ਸਥਿਤੀ;
    u8 floppy_return_ਸਥਿਤੀ[7];
    u8 video_ਬਹੁਲਕ;
    u16 video_ਖਾਨੇ_ਗਿਣਤੀ;
    u16 video_pagesize;
    u16 video_pagestart;
    // 40:50
    u16 cursor_pos[8];
    // 40:60
    u16 cursor_type;
    u8 video_ਪੰਨਾ;
    u16 crtc_ਪਤਾ;
    u8 video_msr;
    u8 video_pal;
    ਬਣਤਰ segoff_s jump;
    u8 other_6b;
    u32 timer_counter;
    // 40:70
    u8 timer_rollover;
    u8 break_flag;
    u16 soft_reset_flag;
    u8 disk_ਆਖਰੀ_ਸਥਿਤੀ;
    u8 hdcount;
    u8 disk_control_byte;
    u8 ਪੋਰਟ_disk;
    u8 lpt_timeout[4];
    u8 com_timeout[4];
    // 40:80
    u16 kbd_buf_ਸ਼ੁਰੂ_ਵਿੱਥ;
    u16 kbd_buf_ਸਿਰਾ_ਵਿੱਥ;
    u8 video_ਪੰਕਤੀਆਂ;
    u16 char_ਉਚਾਈ;
    u8 video_ctl;
    u8 video_switches;
    u8 modeset_ctl;
    u8 dcc_ਸੂਚਕਾਂਕ;
    u8 floppy_ਆਖਰੀ_ਅੰਕੜੇ_ਦਰ;
    u8 disk_ਸਥਿਤੀ_controller;
    u8 disk_error_controller;
    u8 disk_ਰੁਕਾਵਟ_flag;
    u8 floppy_harddisk_info;
    // 40:90
    u8 floppy_media_state[4];
    u8 floppy_track[2];
    u8 kbd_flag1;
    u8 kbd_led;
    ਬਣਤਰ segoff_s user_wait_complete_flag;
    u32 user_wait_timeout;
    // 40:A0
    u8 rtc_wait_flag;
    u8 other_a1[7];
    ਬਣਤਰ segoff_s video_savetable;
    u8 other_ac[4];
    // 40:B0
    u8 other_b0[5*16];
} PACKED;

// BDA floppy_recalibration_status bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FRS_IRQ (1<<7)

// BDA rtc_wait_flag bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RWS_WAIT_PENDING (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RWS_WAIT_ELAPSED (1<<7)

// BDA floppy_media_state bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FMS_DRIVE_STATE_ਨਕਾਬ        (0x07)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FMS_MEDIA_DRIVE_ESTABLISHED (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FMS_DOUBLE_STEPPING         (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FMS_ਅੰਕੜੇ_ਦਰ_ਨਕਾਬ          (0xc0)

// BDA kbd_flag[01] bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_RSHIFT       (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_LSHIFT       (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_CTRLACTIVE   (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_ALTACTIVE    (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_SCROLLACTIVE (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_NUMACTIVE    (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_CAPSACTIVE   (1<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_LCTRL        (1<<8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_LALT         (1<<9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_PAUSEACTIVE  (1<<11)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_SCROLL       (1<<12)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_ਅੰਕ          (1<<13)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF0_CAPS         (1<<14)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF1_ਆਖਰੀ_E1    (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF1_ਆਖਰੀ_E0    (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF1_RCTRL      (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF1_RALT       (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KF1_101KBD     (1<<4)

// Limit of BDA timer_counter field
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TICKS_ਪ੍ਰਤੀ_DAY 1573040


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s {
    u8 ਮਾਪ;
    u8 reserved1[0x21];
    ਬਣਤਰ segoff_s far_call_ਸੂਚਕ;
    u8 mouse_flag1;
    u8 mouse_flag2;
    u8 mouse_ਅੰਕੜੇ[0x08];
    // 0x30
    u8 other1[0x0d];

    // 0x3d
    ਬਣਤਰ fdpt_s fdpt[2];

    // 0x5d
    u8 other2[0xC4];

    // 0x121 - Begin custom storage.
} PACKED;


/****************************************************************
 * Bios Config Table
 ****************************************************************/

ਬਣਤਰ bios_config_ਸਾਰਣੀ_s {
    u16 ਮਾਪ;
    u8 model;
    u8 submodel;
    u8 biosrev;
    u8 feature1, feature2, feature3, feature4, feature5;
} PACKED;

#ਜੇ_ਅੰਤ // bda.h
