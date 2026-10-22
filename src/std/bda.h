// BIOS Data Area (and similar) definitions
#अगर_अघोषित __BDA_H
#घोषणा __BDA_H

#समावेश "disk.h" // struct fdpt_s
#समावेश "types.h" // u8


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

काष्ठा rmode_IVT {
    काष्ठा segoff_s ivec[256];
};


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

काष्ठा bios_तथ्य_area_s {
    // 40:00
    u16 port_com[4];
    u16 port_lpt[3];
    u16 ebda_seg;
    // 40:10
    u16 equipment_list_flags;
    u8 pad1;
    u16 mem_आकार_kb;
    u8 pad2;
    u8 ps2_ctrl_flag;
    u16 kbd_flag0;
    u8 alt_keypad;
    u16 kbd_buf_शीर्ष;
    u16 kbd_buf_पुच्छ;
    // 40:1e
    u8 kbd_buf[32];
    u8 floppy_recalibration_status;
    u8 floppy_motor_status;
    // 40:40
    u8 floppy_motor_गणक;
    u8 floppy_last_status;
    u8 floppy_return_status[7];
    u8 video_mode;
    u16 video_cols;
    u16 video_pagesize;
    u16 video_pagestart;
    // 40:50
    u16 cursor_pos[8];
    // 40:60
    u16 cursor_type;
    u8 video_page;
    u16 crtc_address;
    u8 video_msr;
    u8 video_pal;
    काष्ठा segoff_s jump;
    u8 other_6b;
    u32 timer_गणक;
    // 40:70
    u8 timer_rollover;
    u8 break_flag;
    u16 soft_reset_flag;
    u8 disk_last_status;
    u8 hdcount;
    u8 disk_control_byte;
    u8 port_disk;
    u8 lpt_timeout[4];
    u8 com_timeout[4];
    // 40:80
    u16 kbd_buf_start_offset;
    u16 kbd_buf_end_offset;
    u8 video_rows;
    u16 char_height;
    u8 video_ctl;
    u8 video_switches;
    u8 modeset_ctl;
    u8 dcc_सूचकांक;
    u8 floppy_last_तथ्य_rate;
    u8 disk_status_controller;
    u8 disk_error_controller;
    u8 disk_interrupt_flag;
    u8 floppy_harddisk_info;
    // 40:90
    u8 floppy_media_state[4];
    u8 floppy_track[2];
    u8 kbd_flag1;
    u8 kbd_led;
    काष्ठा segoff_s user_wait_complete_flag;
    u32 user_wait_timeout;
    // 40:A0
    u8 rtc_wait_flag;
    u8 other_a1[7];
    काष्ठा segoff_s video_savetable;
    u8 other_ac[4];
    // 40:B0
    u8 other_b0[5*16];
} PACKED;

// BDA floppy_recalibration_status bitdefs
#घोषणा FRS_IRQ (1<<7)

// BDA rtc_wait_flag bitdefs
#घोषणा RWS_WAIT_PENDING (1<<0)
#घोषणा RWS_WAIT_ELAPSED (1<<7)

// BDA floppy_media_state bitdefs
#घोषणा FMS_DRIVE_STATE_MASK        (0x07)
#घोषणा FMS_MEDIA_DRIVE_ESTABLISHED (1<<4)
#घोषणा FMS_DOUBLE_STEPPING         (1<<5)
#घोषणा FMS_तथ्य_RATE_MASK          (0xc0)

// BDA kbd_flag[01] bitdefs
#घोषणा KF0_RSHIFT       (1<<0)
#घोषणा KF0_LSHIFT       (1<<1)
#घोषणा KF0_CTRLACTIVE   (1<<2)
#घोषणा KF0_ALTACTIVE    (1<<3)
#घोषणा KF0_SCROLLACTIVE (1<<4)
#घोषणा KF0_NUMACTIVE    (1<<5)
#घोषणा KF0_CAPSACTIVE   (1<<6)
#घोषणा KF0_LCTRL        (1<<8)
#घोषणा KF0_LALT         (1<<9)
#घोषणा KF0_PAUSEACTIVE  (1<<11)
#घोषणा KF0_SCROLL       (1<<12)
#घोषणा KF0_अंक          (1<<13)
#घोषणा KF0_CAPS         (1<<14)

#घोषणा KF1_LAST_E1    (1<<0)
#घोषणा KF1_LAST_E0    (1<<1)
#घोषणा KF1_RCTRL      (1<<2)
#घोषणा KF1_RALT       (1<<3)
#घोषणा KF1_101KBD     (1<<4)

// Limit of BDA timer_counter field
#घोषणा TICKS_PER_DAY 1573040


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

काष्ठा extended_bios_तथ्य_area_s {
    u8 आकार;
    u8 reserved1[0x21];
    काष्ठा segoff_s far_call_pointer;
    u8 mouse_flag1;
    u8 mouse_flag2;
    u8 mouse_तथ्य[0x08];
    // 0x30
    u8 other1[0x0d];

    // 0x3d
    काष्ठा fdpt_s fdpt[2];

    // 0x5d
    u8 other2[0xC4];

    // 0x121 - Begin custom storage.
} PACKED;


/****************************************************************
 * Bios Config Table
 ****************************************************************/

काष्ठा bios_config_table_s {
    u16 आकार;
    u8 model;
    u8 submodel;
    u8 biosrev;
    u8 feature1, feature2, feature3, feature4, feature5;
} PACKED;

#पूर्ण_अगर // bda.h
