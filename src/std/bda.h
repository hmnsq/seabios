// BIOS Data Area (and similar) definitions
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __BDA_H
#ವ್ಯಾಖ್ಯಾನಿಸು __BDA_H

#ಅನ್ತರ್ಭಾವ "disk.h" // struct fdpt_s
#ಅನ್ತರ್ಭಾವ "types.h" // u8


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

ರಚನೆ rmode_IVT {
    ರಚನೆ segoff_s ivec[256];
};


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

ರಚನೆ bios_ದತ್ತಾಂಶ_area_s {
    // 40:00
    u16 ಪೋರ್ಟ್_com[4];
    u16 ಪೋರ್ಟ್_lpt[3];
    u16 ebda_seg;
    // 40:10
    u16 equipment_list_flags;
    u8 pad1;
    u16 mem_ಗಾತ್ರ_kb;
    u8 pad2;
    u8 ps2_ctrl_flag;
    u16 kbd_flag0;
    u8 alt_keypad;
    u16 kbd_buf_ತಲೆ;
    u16 kbd_buf_ಬಾಲ;
    // 40:1e
    u8 kbd_buf[32];
    u8 floppy_recalibration_ಸ್ಥಿತಿ;
    u8 floppy_motor_ಸ್ಥಿತಿ;
    // 40:40
    u8 floppy_motor_counter;
    u8 floppy_ಕೊನೆಯ_ಸ್ಥಿತಿ;
    u8 floppy_return_ಸ್ಥಿತಿ[7];
    u8 video_ಬಹುಲಕ;
    u16 video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
    u16 video_pagesize;
    u16 video_pagestart;
    // 40:50
    u16 cursor_pos[8];
    // 40:60
    u16 cursor_type;
    u8 video_ಪುಟ;
    u16 crtc_ವಿಳಾಸ;
    u8 video_msr;
    u8 video_pal;
    ರಚನೆ segoff_s jump;
    u8 other_6b;
    u32 timer_counter;
    // 40:70
    u8 timer_rollover;
    u8 break_flag;
    u16 soft_reset_flag;
    u8 disk_ಕೊನೆಯ_ಸ್ಥಿತಿ;
    u8 hdcount;
    u8 disk_control_byte;
    u8 ಪೋರ್ಟ್_disk;
    u8 lpt_timeout[4];
    u8 com_timeout[4];
    // 40:80
    u16 kbd_buf_ಆರಂಭ_ಸ್ಥಾನಾಂತರ;
    u16 kbd_buf_ಕೊನೆ_ಸ್ಥಾನಾಂತರ;
    u8 video_ಅಡ್ಡಸಾಲುಗಳು;
    u16 char_ಎತ್ತರ;
    u8 video_ctl;
    u8 video_switches;
    u8 modeset_ctl;
    u8 dcc_ಸೂಚ್ಯಂಕ;
    u8 floppy_ಕೊನೆಯ_ದತ್ತಾಂಶ_ದರ;
    u8 disk_ಸ್ಥಿತಿ_controller;
    u8 disk_error_controller;
    u8 disk_ಅಡಚಣೆ_flag;
    u8 floppy_harddisk_info;
    // 40:90
    u8 floppy_media_state[4];
    u8 floppy_track[2];
    u8 kbd_flag1;
    u8 kbd_led;
    ರಚನೆ segoff_s user_wait_complete_flag;
    u32 user_wait_timeout;
    // 40:A0
    u8 rtc_wait_flag;
    u8 other_a1[7];
    ರಚನೆ segoff_s video_savetable;
    u8 other_ac[4];
    // 40:B0
    u8 other_b0[5*16];
} PACKED;

// BDA floppy_recalibration_status bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು FRS_IRQ (1<<7)

// BDA rtc_wait_flag bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು RWS_WAIT_PENDING (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು RWS_WAIT_ELAPSED (1<<7)

// BDA floppy_media_state bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು FMS_DRIVE_STATE_ಮುಖವಾಡ        (0x07)
#ವ್ಯಾಖ್ಯಾನಿಸು FMS_MEDIA_DRIVE_ESTABLISHED (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು FMS_DOUBLE_STEPPING         (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು FMS_ದತ್ತಾಂಶ_ದರ_ಮುಖವಾಡ          (0xc0)

// BDA kbd_flag[01] bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_RSHIFT       (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_LSHIFT       (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_CTRLACTIVE   (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_ALTACTIVE    (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_SCROLLACTIVE (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_NUMACTIVE    (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_CAPSACTIVE   (1<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_LCTRL        (1<<8)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_LALT         (1<<9)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_PAUSEACTIVE  (1<<11)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_SCROLL       (1<<12)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_ಅಂಕೆ          (1<<13)
#ವ್ಯಾಖ್ಯಾನಿಸು KF0_CAPS         (1<<14)

#ವ್ಯಾಖ್ಯಾನಿಸು KF1_ಕೊನೆಯ_E1    (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು KF1_ಕೊನೆಯ_E0    (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು KF1_RCTRL      (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು KF1_RALT       (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು KF1_101KBD     (1<<4)

// Limit of BDA timer_counter field
#ವ್ಯಾಖ್ಯಾನಿಸು TICKS_ಪ್ರತಿ_DAY 1573040


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s {
    u8 ಗಾತ್ರ_2;
    u8 reserved1[0x21];
    ರಚನೆ segoff_s far_call_ಸೂಚಕ;
    u8 mouse_flag1;
    u8 mouse_flag2;
    u8 mouse_ದತ್ತಾಂಶ[0x08];
    // 0x30
    u8 other1[0x0d];

    // 0x3d
    ರಚನೆ fdpt_s fdpt[2];

    // 0x5d
    u8 other2[0xC4];

    // 0x121 - Begin custom storage.
} PACKED;


/****************************************************************
 * Bios Config Table
 ****************************************************************/

ರಚನೆ bios_config_ಕೋಷ್ಟಕ_s {
    u16 ಗಾತ್ರ_2;
    u8 model;
    u8 submodel;
    u8 biosrev;
    u8 feature1, feature2, feature3, feature4, feature5;
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // bda.h
