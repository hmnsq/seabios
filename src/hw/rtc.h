#که_نه_تعریف_شوی __RTC_H
#تعریف_کړه __RTC_H

#تعریف_کړه پورټ_CMOS_شاخص        0x0070
#تعریف_کړه پورټ_CMOS_معلومات         0x0071

// PORT_CMOS_INDEX nmi disable bit
#تعریف_کړه NMI_DISABLE_بټ 0x80

// Standard BIOS RTC chip entries
#تعریف_کړه CMOS_RTC_SECONDS         0x00
#تعریف_کړه CMOS_RTC_SECONDS_ALARM   0x01
#تعریف_کړه CMOS_RTC_MINUTES         0x02
#تعریف_کړه CMOS_RTC_MINUTES_ALARM   0x03
#تعریف_کړه CMOS_RTC_HOURS           0x04
#تعریف_کړه CMOS_RTC_HOURS_ALARM     0x05
#تعریف_کړه CMOS_RTC_DAY_WEEK        0x06
#تعریف_کړه CMOS_RTC_DAY_MONTH       0x07
#تعریف_کړه CMOS_RTC_MONTH           0x08
#تعریف_کړه CMOS_RTC_YEAR            0x09
#تعریف_کړه CMOS_حالت_A            0x0a
#تعریف_کړه CMOS_حالت_B            0x0b
#تعریف_کړه CMOS_حالت_C            0x0c
#تعریف_کړه CMOS_حالت_D            0x0d
#تعریف_کړه CMOS_RESET_کوډ          0x0f

// QEMU cmos config fields.  DO NOT ADD MORE.  (All new content should
// be passed via the fw_cfg "file" interface.)
#تعریف_کړه CMOS_FLOPPY_DRIVE_TYPE   0x10
#تعریف_کړه CMOS_DISK_معلومات           0x12
#تعریف_کړه CMOS_EQUIPMENT_INFO      0x14
#تعریف_کړه CMOS_DISK_DRIVE1_TYPE    0x19
#تعریف_کړه CMOS_DISK_DRIVE2_TYPE    0x1a
#تعریف_کړه CMOS_DISK_DRIVE1_CYL     0x1b
#تعریف_کړه CMOS_DISK_DRIVE2_CYL     0x24
#تعریف_کړه CMOS_MEM_EXTMEM_ټیټ      0x30
#تعریف_کړه CMOS_MEM_EXTMEM_لوړ     0x31
#تعریف_کړه CMOS_CENTURY             0x32
#تعریف_کړه CMOS_MEM_EXTMEM2_ټیټ     0x34
#تعریف_کړه CMOS_MEM_EXTMEM2_لوړ    0x35
#تعریف_کړه CMOS_BIOS_BOOTFLAG1      0x38
#تعریف_کړه CMOS_BIOS_DISKTRANSFLAG  0x39
#تعریف_کړه CMOS_BIOS_BOOTFLAG2      0x3d
#تعریف_کړه CMOS_MEM_HIGHMEM_ټیټ     0x5b
#تعریف_کړه CMOS_MEM_HIGHMEM_MID     0x5c
#تعریف_کړه CMOS_MEM_HIGHMEM_لوړ    0x5d
#تعریف_کړه CMOS_BIOS_SMP_شمېر      0x5f

// RTC register flags
#تعریف_کړه RTC_A_UIP 0x80

#تعریف_کړه RTC_B_سېټ  0x80
#تعریف_کړه RTC_B_PIE  0x40
#تعریف_کړه RTC_B_AIE  0x20
#تعریف_کړه RTC_B_UIE  0x10
#تعریف_کړه RTC_B_BIN  0x04
#تعریف_کړه RTC_B_24HR 0x02
#تعریف_کړه RTC_B_DSE  0x01

#که_نه_تعریف_شوی __ASSEMBLY__

#ضمن "types.h" // u8

// rtc.c
u8 rtc_read(u8 شاخص);
فراغ rtc_write(u8 شاخص, u8 val);
فراغ rtc_ماسک(u8 شاخص, u8 off, u8 on);
صحيح rtc_updating(فراغ);
فراغ rtc_تیاری(فراغ);
فراغ rtc_use(فراغ);
فراغ rtc_release(فراغ);

#د_که_پای // !__ASSEMBLY__

#د_که_پای // rtc.h
