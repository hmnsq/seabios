#جے_بنا_تعریف __RTC_H
#تعریف_کرو __RTC_H

#تعریف_کرو پورٹ_CMOS_اشاریہ        0x0070
#تعریف_کرو پورٹ_CMOS_انکڑے         0x0071

// PORT_CMOS_INDEX nmi disable bit
#تعریف_کرو NMI_DISABLE_ثنائی_ہندسہ 0x80

// Standard BIOS RTC chip entries
#تعریف_کرو CMOS_RTC_SECONDS         0x00
#تعریف_کرو CMOS_RTC_SECONDS_ALARM   0x01
#تعریف_کرو CMOS_RTC_MINUTES         0x02
#تعریف_کرو CMOS_RTC_MINUTES_ALARM   0x03
#تعریف_کرو CMOS_RTC_HOURS           0x04
#تعریف_کرو CMOS_RTC_HOURS_ALARM     0x05
#تعریف_کرو CMOS_RTC_DAY_WEEK        0x06
#تعریف_کرو CMOS_RTC_DAY_MONTH       0x07
#تعریف_کرو CMOS_RTC_MONTH           0x08
#تعریف_کرو CMOS_RTC_YEAR            0x09
#تعریف_کرو CMOS_حالت_A            0x0a
#تعریف_کرو CMOS_حالت_B            0x0b
#تعریف_کرو CMOS_حالت_C            0x0c
#تعریف_کرو CMOS_حالت_D            0x0d
#تعریف_کرو CMOS_RESET_کوڈ          0x0f

// QEMU cmos config fields.  DO NOT ADD MORE.  (All new content should
// be passed via the fw_cfg "file" interface.)
#تعریف_کرو CMOS_FLOPPY_DRIVE_TYPE   0x10
#تعریف_کرو CMOS_DISK_انکڑے           0x12
#تعریف_کرو CMOS_EQUIPMENT_INFO      0x14
#تعریف_کرو CMOS_DISK_DRIVE1_TYPE    0x19
#تعریف_کرو CMOS_DISK_DRIVE2_TYPE    0x1a
#تعریف_کرو CMOS_DISK_DRIVE1_CYL     0x1b
#تعریف_کرو CMOS_DISK_DRIVE2_CYL     0x24
#تعریف_کرو CMOS_MEM_EXTMEM_کم      0x30
#تعریف_کرو CMOS_MEM_EXTMEM_بلند     0x31
#تعریف_کرو CMOS_CENTURY             0x32
#تعریف_کرو CMOS_MEM_EXTMEM2_کم     0x34
#تعریف_کرو CMOS_MEM_EXTMEM2_بلند    0x35
#تعریف_کرو CMOS_BIOS_BOOTFLAG1      0x38
#تعریف_کرو CMOS_BIOS_DISKTRANSFLAG  0x39
#تعریف_کرو CMOS_BIOS_BOOTFLAG2      0x3d
#تعریف_کرو CMOS_MEM_HIGHMEM_کم     0x5b
#تعریف_کرو CMOS_MEM_HIGHMEM_MID     0x5c
#تعریف_کرو CMOS_MEM_HIGHMEM_بلند    0x5d
#تعریف_کرو CMOS_BIOS_SMP_گنتی      0x5f

// RTC register flags
#تعریف_کرو RTC_A_UIP 0x80

#تعریف_کرو RTC_B_سیٹ  0x80
#تعریف_کرو RTC_B_PIE  0x40
#تعریف_کرو RTC_B_AIE  0x20
#تعریف_کرو RTC_B_UIE  0x10
#تعریف_کرو RTC_B_BIN  0x04
#تعریف_کرو RTC_B_24HR 0x02
#تعریف_کرو RTC_B_DSE  0x01

#جے_بنا_تعریف __ASSEMBLY__

#ضمن "types.h" // u8

// rtc.c
u8 rtc_read(u8 اشاریہ);
فراغ rtc_write(u8 اشاریہ, u8 val);
فراغ rtc_نقاب(u8 اشاریہ, u8 off, u8 on);
صحيح rtc_updating(فراغ);
فراغ rtc_تیاری(فراغ);
فراغ rtc_use(فراغ);
فراغ rtc_release(فراغ);

#جے_انت // !__ASSEMBLY__

#جے_انت // rtc.h
