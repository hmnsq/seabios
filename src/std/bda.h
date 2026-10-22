// BIOS Data Area (and similar) definitions
#నిర్వచితం_కాకపోతే __BDA_H
#నిర్వచించు __BDA_H

#అన్తర్భావ "disk.h" // struct fdpt_s
#అన్తర్భావ "types.h" // u8


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

నిర్మాణం rmode_IVT {
    నిర్మాణం segoff_s ivec[256];
};


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

నిర్మాణం bios_దత్తాంశం_area_s {
    // 40:00
    u16 పోర్టు_com[4];
    u16 పోర్టు_lpt[3];
    u16 ebda_seg;
    // 40:10
    u16 equipment_list_flags;
    u8 pad1;
    u16 mem_కొలత_kb;
    u8 pad2;
    u8 ps2_ctrl_flag;
    u16 kbd_flag0;
    u8 alt_keypad;
    u16 kbd_buf_తల;
    u16 kbd_buf_తోక;
    // 40:1e
    u8 kbd_buf[32];
    u8 floppy_recalibration_స్థితి;
    u8 floppy_motor_స్థితి;
    // 40:40
    u8 floppy_motor_counter;
    u8 floppy_చివరి_స్థితి;
    u8 floppy_return_స్థితి[7];
    u8 video_బహుళకం;
    u16 video_నిలువుల_సంఖ్య;
    u16 video_pagesize;
    u16 video_pagestart;
    // 40:50
    u16 cursor_pos[8];
    // 40:60
    u16 cursor_type;
    u8 video_పుట;
    u16 crtc_చిరునామా;
    u8 video_msr;
    u8 video_pal;
    నిర్మాణం segoff_s jump;
    u8 other_6b;
    u32 timer_counter;
    // 40:70
    u8 timer_rollover;
    u8 break_flag;
    u16 soft_reset_flag;
    u8 disk_చివరి_స్థితి;
    u8 hdcount;
    u8 disk_control_byte;
    u8 పోర్టు_disk;
    u8 lpt_timeout[4];
    u8 com_timeout[4];
    // 40:80
    u16 kbd_buf_ప్రారంభం_స్థానభ్రంశం;
    u16 kbd_buf_చివర_స్థానభ్రంశం;
    u8 video_అడ్డువరుసలు;
    u16 char_ఎత్తు;
    u8 video_ctl;
    u8 video_switches;
    u8 modeset_ctl;
    u8 dcc_సూచీ;
    u8 floppy_చివరి_దత్తాంశం_రేటు;
    u8 disk_స్థితి_controller;
    u8 disk_error_controller;
    u8 disk_అంతరాయం_flag;
    u8 floppy_harddisk_info;
    // 40:90
    u8 floppy_media_state[4];
    u8 floppy_track[2];
    u8 kbd_flag1;
    u8 kbd_led;
    నిర్మాణం segoff_s user_wait_complete_flag;
    u32 user_wait_timeout;
    // 40:A0
    u8 rtc_wait_flag;
    u8 other_a1[7];
    నిర్మాణం segoff_s video_savetable;
    u8 other_ac[4];
    // 40:B0
    u8 other_b0[5*16];
} PACKED;

// BDA floppy_recalibration_status bitdefs
#నిర్వచించు FRS_IRQ (1<<7)

// BDA rtc_wait_flag bitdefs
#నిర్వచించు RWS_WAIT_PENDING (1<<0)
#నిర్వచించు RWS_WAIT_ELAPSED (1<<7)

// BDA floppy_media_state bitdefs
#నిర్వచించు FMS_DRIVE_STATE_ముసుగు        (0x07)
#నిర్వచించు FMS_MEDIA_DRIVE_ESTABLISHED (1<<4)
#నిర్వచించు FMS_DOUBLE_STEPPING         (1<<5)
#నిర్వచించు FMS_దత్తాంశం_రేటు_ముసుగు          (0xc0)

// BDA kbd_flag[01] bitdefs
#నిర్వచించు KF0_RSHIFT       (1<<0)
#నిర్వచించు KF0_LSHIFT       (1<<1)
#నిర్వచించు KF0_CTRLACTIVE   (1<<2)
#నిర్వచించు KF0_ALTACTIVE    (1<<3)
#నిర్వచించు KF0_SCROLLACTIVE (1<<4)
#నిర్వచించు KF0_NUMACTIVE    (1<<5)
#నిర్వచించు KF0_CAPSACTIVE   (1<<6)
#నిర్వచించు KF0_LCTRL        (1<<8)
#నిర్వచించు KF0_LALT         (1<<9)
#నిర్వచించు KF0_PAUSEACTIVE  (1<<11)
#నిర్వచించు KF0_SCROLL       (1<<12)
#నిర్వచించు KF0_అంకె          (1<<13)
#నిర్వచించు KF0_CAPS         (1<<14)

#నిర్వచించు KF1_చివరి_E1    (1<<0)
#నిర్వచించు KF1_చివరి_E0    (1<<1)
#నిర్వచించు KF1_RCTRL      (1<<2)
#నిర్వచించు KF1_RALT       (1<<3)
#నిర్వచించు KF1_101KBD     (1<<4)

// Limit of BDA timer_counter field
#నిర్వచించు TICKS_చొప్పున_DAY 1573040


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

నిర్మాణం extended_bios_దత్తాంశం_area_s {
    u8 కొలత;
    u8 reserved1[0x21];
    నిర్మాణం segoff_s far_call_సూచిక;
    u8 mouse_flag1;
    u8 mouse_flag2;
    u8 mouse_దత్తాంశం[0x08];
    // 0x30
    u8 other1[0x0d];

    // 0x3d
    నిర్మాణం fdpt_s fdpt[2];

    // 0x5d
    u8 other2[0xC4];

    // 0x121 - Begin custom storage.
} PACKED;


/****************************************************************
 * Bios Config Table
 ****************************************************************/

నిర్మాణం bios_config_పట్టిక_s {
    u16 కొలత;
    u8 model;
    u8 submodel;
    u8 biosrev;
    u8 feature1, feature2, feature3, feature4, feature5;
} PACKED;

#అయితే_ముగింపు // bda.h
