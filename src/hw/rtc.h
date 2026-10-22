#यदि_अपरिभाषितम् __RTC_H
#परिभाषय __RTC_H

#परिभाषय पत्तनम्_CMOS_सूचकांक        0x0070
#परिभाषय पत्तनम्_CMOS_दत्त         0x0071

// PORT_CMOS_INDEX nmi disable bit
#परिभाषय NMI_DISABLE_BIT 0x80

// Standard BIOS RTC chip entries
#परिभाषय CMOS_RTC_SECONDS         0x00
#परिभाषय CMOS_RTC_SECONDS_ALARM   0x01
#परिभाषय CMOS_RTC_MINUTES         0x02
#परिभाषय CMOS_RTC_MINUTES_ALARM   0x03
#परिभाषय CMOS_RTC_HOURS           0x04
#परिभाषय CMOS_RTC_HOURS_ALARM     0x05
#परिभाषय CMOS_RTC_DAY_WEEK        0x06
#परिभाषय CMOS_RTC_DAY_MONTH       0x07
#परिभाषय CMOS_RTC_MONTH           0x08
#परिभाषय CMOS_RTC_YEAR            0x09
#परिभाषय CMOS_स्थितिः_A            0x0a
#परिभाषय CMOS_स्थितिः_B            0x0b
#परिभाषय CMOS_स्थितिः_C            0x0c
#परिभाषय CMOS_स्थितिः_D            0x0d
#परिभाषय CMOS_RESET_संहिता          0x0f

// QEMU cmos config fields.  DO NOT ADD MORE.  (All new content should
// be passed via the fw_cfg "file" interface.)
#परिभाषय CMOS_FLOPPY_DRIVE_TYPE   0x10
#परिभाषय CMOS_DISK_दत्त           0x12
#परिभाषय CMOS_EQUIPMENT_INFO      0x14
#परिभाषय CMOS_DISK_DRIVE1_TYPE    0x19
#परिभाषय CMOS_DISK_DRIVE2_TYPE    0x1a
#परिभाषय CMOS_DISK_DRIVE1_CYL     0x1b
#परिभाषय CMOS_DISK_DRIVE2_CYL     0x24
#परिभाषय CMOS_MEM_EXTMEM_नीचः      0x30
#परिभाषय CMOS_MEM_EXTMEM_उच्चः     0x31
#परिभाषय CMOS_CENTURY             0x32
#परिभाषय CMOS_MEM_EXTMEM2_नीचः     0x34
#परिभाषय CMOS_MEM_EXTMEM2_उच्चः    0x35
#परिभाषय CMOS_BIOS_BOOTFLAG1      0x38
#परिभाषय CMOS_BIOS_DISKTRANSFLAG  0x39
#परिभाषय CMOS_BIOS_BOOTFLAG2      0x3d
#परिभाषय CMOS_MEM_HIGHMEM_नीचः     0x5b
#परिभाषय CMOS_MEM_HIGHMEM_MID     0x5c
#परिभाषय CMOS_MEM_HIGHMEM_उच्चः    0x5d
#परिभाषय CMOS_BIOS_SMP_गणनम्      0x5f

// RTC register flags
#परिभाषय RTC_A_UIP 0x80

#परिभाषय RTC_B_समुच्चयः  0x80
#परिभाषय RTC_B_PIE  0x40
#परिभाषय RTC_B_AIE  0x20
#परिभाषय RTC_B_UIE  0x10
#परिभाषय RTC_B_BIN  0x04
#परिभाषय RTC_B_24HR 0x02
#परिभाषय RTC_B_DSE  0x01

#यदि_अपरिभाषितम् __ASSEMBLY__

#अन्तर्भाव "types.h" // u8

// rtc.c
u8 rtc_read(u8 सूचकांक);
शून्य rtc_write(u8 सूचकांक, u8 val);
शून्य rtc_आवरणम्(u8 सूचकांक, u8 off, u8 on);
पूर्णाङ्क rtc_updating(शून्य);
शून्य rtc_सज्जता(शून्य);
शून्य rtc_use(शून्य);
शून्य rtc_release(शून्य);

#यदि_अन्तः // !__ASSEMBLY__

#यदि_अन्तः // rtc.h
