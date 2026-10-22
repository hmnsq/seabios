// Misc function and variable declarations.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __UTIL_H
#ವ್ಯಾಖ್ಯಾನಿಸು __UTIL_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// apm.c
ಶೂನ್ಯ apm_shutdown(ಶೂನ್ಯ);
ರಚನೆ bregs;
ಶೂನ್ಯ handle_1553(ರಚನೆ bregs *regs);

// bmp.c
ರಚನೆ bmp_decdata *bmp_alloc(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ bmp_decode(ರಚನೆ bmp_decdata *bmp, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ದತ್ತಾಂಶ_ಗಾತ್ರ);
ಶೂನ್ಯ bmp_get_info(ರಚನೆ bmp_decdata *bmp, ಪೂರ್ಣಾಂಕ *ಅಗಲ, ಪೂರ್ಣಾಂಕ *ಎತ್ತರ, ಪೂರ್ಣಾಂಕ *bpp);
ಪೂರ್ಣಾಂಕ bmp_show(ರಚನೆ bmp_decdata *bmp, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic, ಪೂರ್ಣಾಂಕ ಅಗಲ
             , ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಪೂರ್ಣಾಂಕ ಆಳ, ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಾಲು_dest);

// boot.c
ಶೂನ್ಯ boot_init(ಶೂನ್ಯ);
ಶೂನ್ಯ boot_ಕೂಡಿಸು_bev(u16 seg, u16 bev, u16 desc, ಪೂರ್ಣಾಂಕ prio);
ಶೂನ್ಯ boot_ಕೂಡಿಸು_bcv(u16 seg, u16 ip, u16 desc, ಪೂರ್ಣಾಂಕ prio);
ರಚನೆ drive_s;
ಶೂನ್ಯ boot_ಕೂಡಿಸು_floppy(ರಚನೆ drive_s *drive_g, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio);
ಶೂನ್ಯ boot_ಕೂಡಿಸು_hd(ರಚನೆ drive_s *drive_g, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio);
ಶೂನ್ಯ boot_ಕೂಡಿಸು_cd(ರಚನೆ drive_s *drive_g, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio);
ಶೂನ್ಯ boot_ಕೂಡಿಸು_cbfs(ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc, ಪೂರ್ಣಾಂಕ prio);
ಶೂನ್ಯ interactive_bootmenu(ಶೂನ್ಯ);
ಶೂನ್ಯ bcv_prepboot(ಶೂನ್ಯ);
u8 is_bootprio_strict(ಶೂನ್ಯ);
ರಚನೆ pci_device;
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_pci_device(ರಚನೆ pci_device *pci);
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_mmio_device(ಶೂನ್ಯ *mmio);
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_scsi_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಗುರಿ, ಪೂರ್ಣಾಂಕ lun);
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_scsi_mmio_device(ಶೂನ್ಯ *mmio, ಪೂರ್ಣಾಂಕ ಗುರಿ, ಪೂರ್ಣಾಂಕ lun);
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_ata_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ slave);
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_fdc_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ fdid);
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_pci_rom(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ instance);
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_named_rom(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ instance);
ರಚನೆ usbdevice_s;
ಪೂರ್ಣಾಂಕ bootprio_ಕಂಡುಹಿಡಿ_usb(ರಚನೆ usbdevice_s *usbdev, ಪೂರ್ಣಾಂಕ lun);
ಪೂರ್ಣಾಂಕ get_keystroke_ತುಂಬಿದ(ಪೂರ್ಣಾಂಕ msec);
ಪೂರ್ಣಾಂಕ get_keystroke(ಪೂರ್ಣಾಂಕ msec);
ರಚನೆ chs_s;
ಪೂರ್ಣಾಂಕ boot_lchs_ಕಂಡುಹಿಡಿ_pci_device(ರಚನೆ pci_device *pci, ರಚನೆ chs_s *chs);
ಪೂರ್ಣಾಂಕ boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಗುರಿ, ಪೂರ್ಣಾಂಕ lun,
                               ರಚನೆ chs_s *chs);
ಪೂರ್ಣಾಂಕ boot_lchs_ಕಂಡುಹಿಡಿ_ata_device(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ slave,
                              ರಚನೆ chs_s *chs);

// bootsplash.c
ಶೂನ್ಯ enable_vga_console(ಶೂನ್ಯ);
ಶೂನ್ಯ enable_bootsplash(ಶೂನ್ಯ);
ಶೂನ್ಯ disable_bootsplash(ಶೂನ್ಯ);

// cdrom.c
ಬಾಹ್ಯ ರಚನೆ eltorito_s CDEmu;
ಬಾಹ್ಯ ರಚನೆ drive_s *cdemu_drive_gf;
ರಚನೆ disk_op_s;
ಪೂರ್ಣಾಂಕ cdemu_process_op(ರಚನೆ disk_op_s *op);
ಶೂನ್ಯ cdrom_prepboot(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ cdrom_boot(ರಚನೆ drive_s *drive_g);
ಅಕ್ಷರ *cdrom_media_info(ರಚನೆ drive_s *drive_g);

// clock.c
ಶೂನ್ಯ clock_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ handle_1583(ರಚನೆ bregs *regs);
ಶೂನ್ಯ clock_poll_irq(ಶೂನ್ಯ);
u32 irqtimer_calc_ticks(u32 ಎಣಿಕೆ_2);
u32 irqtimer_calc(u32 msecs);
ಪೂರ್ಣಾಂಕ irqtimer_check(u32 ಕೊನೆ);
ಶೂನ್ಯ handle_1586(ರಚನೆ bregs *regs);

// fw/acpi.c
ಶೂನ್ಯ acpi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// fw/biostable.c
ಶೂನ್ಯ copy_pir(ಶೂನ್ಯ *pos);
ಶೂನ್ಯ copy_mptable(ಶೂನ್ಯ *pos);
ಬಾಹ್ಯ ರಚನೆ pir_ಶೀರ್ಷಿಕೆ *PirAddr;
ಶೂನ್ಯ copy_acpi_rsdp(ಶೂನ್ಯ *pos);
ಬಾಹ್ಯ ರಚನೆ rsdp_descriptor *RsdpAddr;
ಬಾಹ್ಯ u32 acpi_pm1a_cnt;
ಬಾಹ್ಯ u16 acpi_pm_ತಳ;
ಶೂನ್ಯ *ಕಂಡುಹಿಡಿ_acpi_rsdp(ಶೂನ್ಯ);
ಶೂನ್ಯ *ಕಂಡುಹಿಡಿ_acpi_ಕೋಷ್ಟಕ(u32 signature);
u32 ಕಂಡುಹಿಡಿ_resume_ಸದಿಶ(ಶೂನ್ಯ);
ಶೂನ್ಯ acpi_reboot(ಶೂನ್ಯ);
ಶೂನ್ಯ ಕಂಡುಹಿಡಿ_acpi_features(ಶೂನ್ಯ);
ಶೂನ್ಯ *smbios_get_tables(u32 *ಉದ್ದಳತೆ);
ಶೂನ್ಯ copy_smbios_21(ಶೂನ್ಯ *pos);
ಶೂನ್ಯ display_uuid(ಶೂನ್ಯ);
ಶೂನ್ಯ copy_ಕೋಷ್ಟಕ(ಶೂನ್ಯ *pos);
ಶೂನ್ಯ smbios_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// fw/dsdt_parser.c
ರಚನೆ acpi_device;
ಶೂನ್ಯ acpi_dsdt_parse(ಶೂನ್ಯ);
ರಚನೆ acpi_device *acpi_dsdt_ಕಂಡುಹಿಡಿ_string(ರಚನೆ acpi_device *prev, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hid);
ರಚನೆ acpi_device *acpi_dsdt_ಕಂಡುಹಿಡಿ_eisaid(ರಚನೆ acpi_device *prev, u16 eisaid);
ಅಕ್ಷರ *acpi_dsdt_name(ರಚನೆ acpi_device *dev);
ಪೂರ್ಣಾಂಕ acpi_dsdt_present_eisaid(u16 eisaid);
ಪೂರ್ಣಾಂಕ acpi_dsdt_ಕಂಡುಹಿಡಿ_io(ರಚನೆ acpi_device *dev, u64 *min, u64 *max);
ಪೂರ್ಣಾಂಕ acpi_dsdt_ಕಂಡುಹಿಡಿ_mem(ರಚನೆ acpi_device *dev, u64 *min, u64 *max);
ಪೂರ್ಣಾಂಕ acpi_dsdt_ಕಂಡುಹಿಡಿ_irq(ರಚನೆ acpi_device *dev, u64 *irq);

// fw/coreboot.c
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *CBvendor, *CBpart;
ರಚನೆ cbfs_file;
ಶೂನ್ಯ coreboot_debug_putc(ಅಕ್ಷರ c);
ಶೂನ್ಯ cbfs_ಸತತ_payload(ರಚನೆ cbfs_file *file);
ಶೂನ್ಯ coreboot_platform_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ cbfs_payload_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ coreboot_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ coreboot_cbfs_init(ಶೂನ್ಯ);
ರಚನೆ cb_ಶೀರ್ಷಿಕೆ;
ಶೂನ್ಯ *ಕಂಡುಹಿಡಿ_cb_subtable(ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *cbh, u32 ಟ್ಯಾಗ್);
ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *ಕಂಡುಹಿಡಿ_cb_ಕೋಷ್ಟಕ(ಶೂನ್ಯ);

// fw/csm.c
ಪೂರ್ಣಾಂಕ csm_bootprio_fdc(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ fdid);
ಪೂರ್ಣಾಂಕ csm_bootprio_ata(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ slave);
ಪೂರ್ಣಾಂಕ csm_bootprio_pci(ರಚನೆ pci_device *pci);

// fw/mptable.c
ಶೂನ್ಯ mptable_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// fw/mtrr.c
ಶೂನ್ಯ mtrr_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// fw/multiboot.c
ಶೂನ್ಯ multiboot_init(ಶೂನ್ಯ);

// fw/pciinit.c
ಬಾಹ್ಯ u64 pcimem_ಆರಂಭ, pcimem_ಕೊನೆ;
ಬಾಹ್ಯ u64 pcimem64_ಆರಂಭ, pcimem64_ಕೊನೆ;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ u8 pci_irqs[4];
ಶೂನ್ಯ pci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ pci_resume(ಶೂನ್ಯ);

// fw/pirtable.c
ಶೂನ್ಯ pirtable_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// fw/shadow.c
ಶೂನ್ಯ make_bios_writable(ಶೂನ್ಯ);
ಶೂನ್ಯ make_bios_readonly(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_reboot(ಶೂನ್ಯ);

// fw/smbios.c
ಶೂನ್ಯ smbios_legacy_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// fw/smm.c
ಶೂನ್ಯ smm_device_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ smm_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// fw/smp.c
ಬಾಹ್ಯ u32 Max_ಎಣಿಕೆ_CPUs;
ಶೂನ್ಯ wrmsr_smp(u32 ಸೂಚ್ಯಂಕ, u64 val);
ಶೂನ್ಯ smp_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ smp_resume(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ apic_id_is_present(u8 apic_id);

// hw/dma.c
ಪೂರ್ಣಾಂಕ dma_floppy(u32 addr, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಪೂರ್ಣಾಂಕ isWrite);
ಶೂನ್ಯ dma_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// hw/floppy.c
ಬಾಹ್ಯ ರಚನೆ floppy_ext_dbt_s diskette_param_table2;
ಶೂನ್ಯ floppy_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ರಚನೆ drive_s *init_floppy(ಪೂರ್ಣಾಂಕ floppyid, ಪೂರ್ಣಾಂಕ ftype);
ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ_floppy_type(u32 ಗಾತ್ರ_2);
ಪೂರ್ಣಾಂಕ floppy_process_op(ರಚನೆ disk_op_s *op);
ಶೂನ್ಯ floppy_tick(ಶೂನ್ಯ);

// hw/ramdisk.c
ಶೂನ್ಯ ramdisk_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ ramdisk_process_op(ರಚನೆ disk_op_s *op);

// hw/sdcard.c
ಪೂರ್ಣಾಂಕ sdcard_process_op(ರಚನೆ disk_op_s *op);
ಶೂನ್ಯ sdcard_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// hw/timer.c
ಶೂನ್ಯ timer_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ pmtimer_ಸಿದ್ಧತೆ(u16 ioport);
ಶೂನ್ಯ tsctimer_setfreq(u32 khz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
ಪೂರ್ಣಾಂಕ timer_check(u32 ಕೊನೆ);
ಶೂನ್ಯ ndelay(u32 ಎಣಿಕೆ_2);
ಶೂನ್ಯ udelay(u32 ಎಣಿಕೆ_2);
ಶೂನ್ಯ mdelay(u32 ಎಣಿಕೆ_2);
ಶೂನ್ಯ nsleep(u32 ಎಣಿಕೆ_2);
ಶೂನ್ಯ usleep(u32 ಎಣಿಕೆ_2);
ಶೂನ್ಯ msleep(u32 ಎಣಿಕೆ_2);
u32 ticks_ಗಮ್ಯ_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
ಶೂನ್ಯ pit_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// jpeg.c
ರಚನೆ jpeg_decdata *jpeg_alloc(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ jpeg_decode(ರಚನೆ jpeg_decdata *jpeg, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf);
ಶೂನ್ಯ jpeg_get_ಗಾತ್ರ(ರಚನೆ jpeg_decdata *jpeg, ಪೂರ್ಣಾಂಕ *ಅಗಲ, ಪೂರ್ಣಾಂಕ *ಎತ್ತರ);
ಪೂರ್ಣಾಂಕ jpeg_show(ರಚನೆ jpeg_decdata *jpeg, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic, ಪೂರ್ಣಾಂಕ ಅಗಲ
              , ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಪೂರ್ಣಾಂಕ ಆಳ, ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಾಲು_dest);

// kbd.c
ಶೂನ್ಯ kbd_init(ಶೂನ್ಯ);
ಶೂನ್ಯ handle_15c2(ರಚನೆ bregs *regs);
ಶೂನ್ಯ process_ಕೀಲಿ(u8 ಕೀಲಿ);
u8 ಸರತಿಗೆ_ಸೇರಿಸು_ಕೀಲಿ(u16 keycode);
u16 ascii_ಗಮ್ಯ_keycode(u8 ascii);

// misc.c
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ Have_ಸತತ_Post;
ಬಾಹ್ಯ ರಚನೆ bios_config_ಕೋಷ್ಟಕ_s BIOS_CONFIG_ಕೋಷ್ಟಕ __aligned(1);
ಬಾಹ್ಯ ರಚನೆ floppy_dbt_s diskette_param_ಕೋಷ್ಟಕ __aligned(1);
ಬಾಹ್ಯ u8 BiosChecksum;
ಪೂರ್ಣಾಂಕ ಒಳ_post(ಶೂನ್ಯ);
ಶೂನ್ಯ mathcp_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// mouse.c
ಶೂನ್ಯ mouse_init(ಶೂನ್ಯ);
ಶೂನ್ಯ process_mouse(u8 ದತ್ತಾಂಶ);

// optionroms.c
ರಚನೆ rom_ಶೀರ್ಷಿಕೆ;
ಶೂನ್ಯ callrom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom, u16 bdf);
ಶೂನ್ಯ call_bcv(u16 seg, u16 ip);
ಪೂರ್ಣಾಂಕ is_pci_vga(ರಚನೆ pci_device *pci);
ಶೂನ್ಯ optionrom_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ vgarom_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ s3_resume_vga(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ScreenAndDebug;

// pcibios.c
ಶೂನ್ಯ handle_1ab1(ರಚನೆ bregs *regs);
ಶೂನ್ಯ bios32_init(ಶೂನ್ಯ);

// pmm.c
ಶೂನ್ಯ pmm_init(ಶೂನ್ಯ);
ಶೂನ್ಯ pmm_prepboot(ಶೂನ್ಯ);

// pnpbios.c
u16 get_pnp_ಸ್ಥಾನಾಂತರ(ಶೂನ್ಯ);
ಶೂನ್ಯ pnp_init(ಶೂನ್ಯ);

// post.c
ಶೂನ್ಯ interface_init(ಶೂನ್ಯ);
ಶೂನ್ಯ device_hardware_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ prepareboot(ಶೂನ್ಯ);
ಶೂನ್ಯ ಆರಂಭ_Boot(ಶೂನ್ಯ);
ಶೂನ್ಯ reloc_preinit(ಶೂನ್ಯ *f, ಶೂನ್ಯ *arg);
ಶೂನ್ಯ ಸಂಕೇತ_mutable_preinit(ಶೂನ್ಯ);

// sercon.c
ಶೂನ್ಯ sercon_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ sercon_check_event(ಶೂನ್ಯ);

// serial.c
ಶೂನ್ಯ serial_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ lpt_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

// version.c
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ಆವೃತ್ತಿ[], BUILDINFO[];

// vgahooks.c
ಶೂನ್ಯ handle_155f(ರಚನೆ bregs *regs);
ಶೂನ್ಯ handle_157f(ರಚನೆ bregs *regs);
ಶೂನ್ಯ vgahook_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci);

#ಆದರೆ_ಅಂತ್ಯ // util.h
