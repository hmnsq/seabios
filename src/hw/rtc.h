#నిర్వచితం_కాకపోతే __RTC_H
#నిర్వచించు __RTC_H

#నిర్వచించు పోర్టు_CMOS_సూచీ        0x0070
#నిర్వచించు పోర్టు_CMOS_దత్తాంశం         0x0071

// PORT_CMOS_INDEX nmi disable bit
#నిర్వచించు NMI_DISABLE_బిట్టు 0x80

// Standard BIOS RTC chip entries
#నిర్వచించు CMOS_RTC_SECONDS         0x00
#నిర్వచించు CMOS_RTC_SECONDS_ALARM   0x01
#నిర్వచించు CMOS_RTC_MINUTES         0x02
#నిర్వచించు CMOS_RTC_MINUTES_ALARM   0x03
#నిర్వచించు CMOS_RTC_HOURS           0x04
#నిర్వచించు CMOS_RTC_HOURS_ALARM     0x05
#నిర్వచించు CMOS_RTC_DAY_WEEK        0x06
#నిర్వచించు CMOS_RTC_DAY_MONTH       0x07
#నిర్వచించు CMOS_RTC_MONTH           0x08
#నిర్వచించు CMOS_RTC_YEAR            0x09
#నిర్వచించు CMOS_స్థితి_A            0x0a
#నిర్వచించు CMOS_స్థితి_B            0x0b
#నిర్వచించు CMOS_స్థితి_C            0x0c
#నిర్వచించు CMOS_స్థితి_D            0x0d
#నిర్వచించు CMOS_RESET_సంకేతం          0x0f

// QEMU cmos config fields.  DO NOT ADD MORE.  (All new content should
// be passed via the fw_cfg "file" interface.)
#నిర్వచించు CMOS_FLOPPY_DRIVE_TYPE   0x10
#నిర్వచించు CMOS_DISK_దత్తాంశం           0x12
#నిర్వచించు CMOS_EQUIPMENT_INFO      0x14
#నిర్వచించు CMOS_DISK_DRIVE1_TYPE    0x19
#నిర్వచించు CMOS_DISK_DRIVE2_TYPE    0x1a
#నిర్వచించు CMOS_DISK_DRIVE1_CYL     0x1b
#నిర్వచించు CMOS_DISK_DRIVE2_CYL     0x24
#నిర్వచించు CMOS_MEM_EXTMEM_దిగువ      0x30
#నిర్వచించు CMOS_MEM_EXTMEM_ఎగువ     0x31
#నిర్వచించు CMOS_CENTURY             0x32
#నిర్వచించు CMOS_MEM_EXTMEM2_దిగువ     0x34
#నిర్వచించు CMOS_MEM_EXTMEM2_ఎగువ    0x35
#నిర్వచించు CMOS_BIOS_BOOTFLAG1      0x38
#నిర్వచించు CMOS_BIOS_DISKTRANSFLAG  0x39
#నిర్వచించు CMOS_BIOS_BOOTFLAG2      0x3d
#నిర్వచించు CMOS_MEM_HIGHMEM_దిగువ     0x5b
#నిర్వచించు CMOS_MEM_HIGHMEM_MID     0x5c
#నిర్వచించు CMOS_MEM_HIGHMEM_ఎగువ    0x5d
#నిర్వచించు CMOS_BIOS_SMP_లెక్క      0x5f

// RTC register flags
#నిర్వచించు RTC_A_UIP 0x80

#నిర్వచించు RTC_B_సమితి  0x80
#నిర్వచించు RTC_B_PIE  0x40
#నిర్వచించు RTC_B_AIE  0x20
#నిర్వచించు RTC_B_UIE  0x10
#నిర్వచించు RTC_B_BIN  0x04
#నిర్వచించు RTC_B_24HR 0x02
#నిర్వచించు RTC_B_DSE  0x01

#నిర్వచితం_కాకపోతే __ASSEMBLY__

#అన్తర్భావ "types.h" // u8

// rtc.c
u8 rtc_read(u8 సూచీ);
శూన్య rtc_write(u8 సూచీ, u8 val);
శూన్య rtc_ముసుగు(u8 సూచీ, u8 off, u8 on);
పూర్ణాంకం rtc_updating(శూన్య);
శూన్య rtc_అమరిక(శూన్య);
శూన్య rtc_use(శూన్య);
శూన్య rtc_release(శూన్య);

#అయితే_ముగింపు // !__ASSEMBLY__

#అయితే_ముగింపు // rtc.h
