#جيڪڏهن_وصف_نه_ٿيل __RTC_H
#وصف_ڪريو __RTC_H

#وصف_ڪريو پورٽ_CMOS_اشاريو        0x0070
#وصف_ڪريو پورٽ_CMOS_معلومات         0x0071

// PORT_CMOS_INDEX nmi disable bit
#وصف_ڪريو NMI_DISABLE_ٻٽو_انگ 0x80

// Standard BIOS RTC chip entries
#وصف_ڪريو CMOS_RTC_SECONDS         0x00
#وصف_ڪريو CMOS_RTC_SECONDS_ALARM   0x01
#وصف_ڪريو CMOS_RTC_MINUTES         0x02
#وصف_ڪريو CMOS_RTC_MINUTES_ALARM   0x03
#وصف_ڪريو CMOS_RTC_HOURS           0x04
#وصف_ڪريو CMOS_RTC_HOURS_ALARM     0x05
#وصف_ڪريو CMOS_RTC_DAY_WEEK        0x06
#وصف_ڪريو CMOS_RTC_DAY_MONTH       0x07
#وصف_ڪريو CMOS_RTC_MONTH           0x08
#وصف_ڪريو CMOS_RTC_YEAR            0x09
#وصف_ڪريو CMOS_حالت_A            0x0a
#وصف_ڪريو CMOS_حالت_B            0x0b
#وصف_ڪريو CMOS_حالت_C            0x0c
#وصف_ڪريو CMOS_حالت_D            0x0d
#وصف_ڪريو CMOS_RESET_ڪوڊ          0x0f

// QEMU cmos config fields.  DO NOT ADD MORE.  (All new content should
// be passed via the fw_cfg "file" interface.)
#وصف_ڪريو CMOS_FLOPPY_DRIVE_TYPE   0x10
#وصف_ڪريو CMOS_DISK_معلومات           0x12
#وصف_ڪريو CMOS_EQUIPMENT_INFO      0x14
#وصف_ڪريو CMOS_DISK_DRIVE1_TYPE    0x19
#وصف_ڪريو CMOS_DISK_DRIVE2_TYPE    0x1a
#وصف_ڪريو CMOS_DISK_DRIVE1_CYL     0x1b
#وصف_ڪريو CMOS_DISK_DRIVE2_CYL     0x24
#وصف_ڪريو CMOS_MEM_EXTMEM_هيٺيون      0x30
#وصف_ڪريو CMOS_MEM_EXTMEM_مٿاهون     0x31
#وصف_ڪريو CMOS_CENTURY             0x32
#وصف_ڪريو CMOS_MEM_EXTMEM2_هيٺيون     0x34
#وصف_ڪريو CMOS_MEM_EXTMEM2_مٿاهون    0x35
#وصف_ڪريو CMOS_BIOS_BOOTFLAG1      0x38
#وصف_ڪريو CMOS_BIOS_DISKTRANSFLAG  0x39
#وصف_ڪريو CMOS_BIOS_BOOTFLAG2      0x3d
#وصف_ڪريو CMOS_MEM_HIGHMEM_هيٺيون     0x5b
#وصف_ڪريو CMOS_MEM_HIGHMEM_MID     0x5c
#وصف_ڪريو CMOS_MEM_HIGHMEM_مٿاهون    0x5d
#وصف_ڪريو CMOS_BIOS_SMP_ڳڻپ      0x5f

// RTC register flags
#وصف_ڪريو RTC_A_UIP 0x80

#وصف_ڪريو RTC_B_سيٽ  0x80
#وصف_ڪريو RTC_B_PIE  0x40
#وصف_ڪريو RTC_B_AIE  0x20
#وصف_ڪريو RTC_B_UIE  0x10
#وصف_ڪريو RTC_B_BIN  0x04
#وصف_ڪريو RTC_B_24HR 0x02
#وصف_ڪريو RTC_B_DSE  0x01

#جيڪڏهن_وصف_نه_ٿيل __ASSEMBLY__

#ضمن "types.h" // u8

// rtc.c
u8 rtc_read(u8 اشاريو);
فراغ rtc_write(u8 اشاريو, u8 val);
فراغ rtc_نقاب(u8 اشاريو, u8 off, u8 on);
صحيح rtc_updating(فراغ);
فراغ rtc_تياري(فراغ);
فراغ rtc_use(فراغ);
فراغ rtc_release(فراغ);

#جيڪڏهن_پورو // !__ASSEMBLY__

#جيڪڏهن_پورو // rtc.h
