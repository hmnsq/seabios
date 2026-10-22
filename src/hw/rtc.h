#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __RTC_H
#ವ್ಯಾಖ್ಯಾನಿಸು __RTC_H

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMOS_ಸೂಚ್ಯಂಕ        0x0070
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMOS_ದತ್ತಾಂಶ         0x0071

// PORT_CMOS_INDEX nmi disable bit
#ವ್ಯಾಖ್ಯಾನಿಸು NMI_DISABLE_ಬಿಟ್ 0x80

// Standard BIOS RTC chip entries
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_SECONDS         0x00
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_SECONDS_ALARM   0x01
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_MINUTES         0x02
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_MINUTES_ALARM   0x03
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_HOURS           0x04
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_HOURS_ALARM     0x05
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_DAY_WEEK        0x06
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_DAY_MONTH       0x07
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_MONTH           0x08
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RTC_YEAR            0x09
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_ಸ್ಥಿತಿ_A            0x0a
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_ಸ್ಥಿತಿ_B            0x0b
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_ಸ್ಥಿತಿ_C            0x0c
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_ಸ್ಥಿತಿ_D            0x0d
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_RESET_ಸಂಕೇತ          0x0f

// QEMU cmos config fields.  DO NOT ADD MORE.  (All new content should
// be passed via the fw_cfg "file" interface.)
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_FLOPPY_DRIVE_TYPE   0x10
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_DISK_ದತ್ತಾಂಶ           0x12
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_EQUIPMENT_INFO      0x14
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_DISK_DRIVE1_TYPE    0x19
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_DISK_DRIVE2_TYPE    0x1a
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_DISK_DRIVE1_CYL     0x1b
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_DISK_DRIVE2_CYL     0x24
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_MEM_EXTMEM_ಕೆಳಗಿನ      0x30
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_MEM_EXTMEM_ಮೇಲಿನ     0x31
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_CENTURY             0x32
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_MEM_EXTMEM2_ಕೆಳಗಿನ     0x34
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_MEM_EXTMEM2_ಮೇಲಿನ    0x35
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_BIOS_BOOTFLAG1      0x38
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_BIOS_DISKTRANSFLAG  0x39
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_BIOS_BOOTFLAG2      0x3d
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_MEM_HIGHMEM_ಕೆಳಗಿನ     0x5b
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_MEM_HIGHMEM_MID     0x5c
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_MEM_HIGHMEM_ಮೇಲಿನ    0x5d
#ವ್ಯಾಖ್ಯಾನಿಸು CMOS_BIOS_SMP_ಎಣಿಕೆ      0x5f

// RTC register flags
#ವ್ಯಾಖ್ಯಾನಿಸು RTC_A_UIP 0x80

#ವ್ಯಾಖ್ಯಾನಿಸು RTC_B_ಗಣ  0x80
#ವ್ಯಾಖ್ಯಾನಿಸು RTC_B_PIE  0x40
#ವ್ಯಾಖ್ಯಾನಿಸು RTC_B_AIE  0x20
#ವ್ಯಾಖ್ಯಾನಿಸು RTC_B_UIE  0x10
#ವ್ಯಾಖ್ಯಾನಿಸು RTC_B_BIN  0x04
#ವ್ಯಾಖ್ಯಾನಿಸು RTC_B_24HR 0x02
#ವ್ಯಾಖ್ಯಾನಿಸು RTC_B_DSE  0x01

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ASSEMBLY__

#ಅನ್ತರ್ಭಾವ "types.h" // u8

// rtc.c
u8 rtc_read(u8 ಸೂಚ್ಯಂಕ);
ಶೂನ್ಯ rtc_write(u8 ಸೂಚ್ಯಂಕ, u8 val);
ಶೂನ್ಯ rtc_ಮುಖವಾಡ(u8 ಸೂಚ್ಯಂಕ, u8 off, u8 on);
ಪೂರ್ಣಾಂಕ rtc_updating(ಶೂನ್ಯ);
ಶೂನ್ಯ rtc_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ rtc_use(ಶೂನ್ಯ);
ಶೂನ್ಯ rtc_release(ಶೂನ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ // !__ASSEMBLY__

#ಆದರೆ_ಅಂತ್ಯ // rtc.h
