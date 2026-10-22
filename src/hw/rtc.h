#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __RTC_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __RTC_H

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMOS_ਸੂਚਕਾਂਕ        0x0070
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMOS_ਅੰਕੜੇ         0x0071

// PORT_CMOS_INDEX nmi disable bit
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NMI_DISABLE_ਦੋਆਧਾਰੀ_ਅੰਕ 0x80

// Standard BIOS RTC chip entries
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_SECONDS         0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_SECONDS_ALARM   0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_MINUTES         0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_MINUTES_ALARM   0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_HOURS           0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_HOURS_ALARM     0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_DAY_WEEK        0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_DAY_MONTH       0x07
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_MONTH           0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RTC_YEAR            0x09
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_ਸਥਿਤੀ_A            0x0a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_ਸਥਿਤੀ_B            0x0b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_ਸਥਿਤੀ_C            0x0c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_ਸਥਿਤੀ_D            0x0d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_RESET_ਕੋਡ          0x0f

// QEMU cmos config fields.  DO NOT ADD MORE.  (All new content should
// be passed via the fw_cfg "file" interface.)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_FLOPPY_DRIVE_TYPE   0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_DISK_ਅੰਕੜੇ           0x12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_EQUIPMENT_INFO      0x14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_DISK_DRIVE1_TYPE    0x19
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_DISK_DRIVE2_TYPE    0x1a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_DISK_DRIVE1_CYL     0x1b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_DISK_DRIVE2_CYL     0x24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_MEM_EXTMEM_ਨੀਵਾਂ      0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_MEM_EXTMEM_ਉੱਚਾ     0x31
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_CENTURY             0x32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_MEM_EXTMEM2_ਨੀਵਾਂ     0x34
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_MEM_EXTMEM2_ਉੱਚਾ    0x35
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_BIOS_BOOTFLAG1      0x38
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_BIOS_DISKTRANSFLAG  0x39
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_BIOS_BOOTFLAG2      0x3d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_MEM_HIGHMEM_ਨੀਵਾਂ     0x5b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_MEM_HIGHMEM_MID     0x5c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_MEM_HIGHMEM_ਉੱਚਾ    0x5d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMOS_BIOS_SMP_ਗਿਣਤੀ      0x5f

// RTC register flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_A_UIP 0x80

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_B_ਸੈੱਟ  0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_B_PIE  0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_B_AIE  0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_B_UIE  0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_B_BIN  0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_B_24HR 0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTC_B_DSE  0x01

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ASSEMBLY__

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8

// rtc.c
u8 rtc_read(u8 ਸੂਚਕਾਂਕ);
ਖਾਲੀ rtc_write(u8 ਸੂਚਕਾਂਕ, u8 val);
ਖਾਲੀ rtc_ਨਕਾਬ(u8 ਸੂਚਕਾਂਕ, u8 off, u8 on);
ਪੂਰਨ rtc_updating(ਖਾਲੀ);
ਖਾਲੀ rtc_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ rtc_use(ਖਾਲੀ);
ਖਾਲੀ rtc_release(ਖਾਲੀ);

#ਜੇ_ਅੰਤ // !__ASSEMBLY__

#ਜੇ_ਅੰਤ // rtc.h
