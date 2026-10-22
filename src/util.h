// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#অন্তর্ভাব "types.h" // u32

// apm.c
শূন্য apm_shutdown(শূন্য);
কাঠামো bregs;
শূন্য handle_1553(কাঠামো bregs *regs);

// bmp.c
কাঠামো bmp_decdata *bmp_alloc(শূন্য);
পূর্ণসংখ্যা bmp_decode(কাঠামো bmp_decdata *bmp, unsigned অক্ষর *তথ্য, পূর্ণসংখ্যা তথ্য_আকার);
শূন্য bmp_get_info(কাঠামো bmp_decdata *bmp, পূর্ণসংখ্যা *width, পূর্ণসংখ্যা *height, পূর্ণসংখ্যা *bpp);
পূর্ণসংখ্যা bmp_show(কাঠামো bmp_decdata *bmp, unsigned অক্ষর *pic, পূর্ণসংখ্যা width
             , পূর্ণসংখ্যা height, পূর্ণসংখ্যা depth, পূর্ণসংখ্যা bytes_per_সারি_dest);

// boot.c
শূন্য boot_init(শূন্য);
শূন্য boot_add_bev(u16 seg, u16 bev, u16 desc, পূর্ণসংখ্যা prio);
শূন্য boot_add_bcv(u16 seg, u16 ip, u16 desc, পূর্ণসংখ্যা prio);
কাঠামো drive_s;
শূন্য boot_add_floppy(কাঠামো drive_s *drive_g, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio);
শূন্য boot_add_hd(কাঠামো drive_s *drive_g, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio);
শূন্য boot_add_cd(কাঠামো drive_s *drive_g, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio);
শূন্য boot_add_cbfs(শূন্য *তথ্য, ধ্রুবক অক্ষর *desc, পূর্ণসংখ্যা prio);
শূন্য interactive_bootmenu(শূন্য);
শূন্য bcv_prepboot(শূন্য);
u8 is_bootprio_strict(শূন্য);
কাঠামো pci_device;
পূর্ণসংখ্যা bootprio_find_pci_device(কাঠামো pci_device *pci);
পূর্ণসংখ্যা bootprio_find_mmio_device(শূন্য *mmio);
পূর্ণসংখ্যা bootprio_find_scsi_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা target, পূর্ণসংখ্যা lun);
পূর্ণসংখ্যা bootprio_find_scsi_mmio_device(শূন্য *mmio, পূর্ণসংখ্যা target, পূর্ণসংখ্যা lun);
পূর্ণসংখ্যা bootprio_find_ata_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা chanid, পূর্ণসংখ্যা slave);
পূর্ণসংখ্যা bootprio_find_fdc_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা port, পূর্ণসংখ্যা fdid);
পূর্ণসংখ্যা bootprio_find_pci_rom(কাঠামো pci_device *pci, পূর্ণসংখ্যা instance);
পূর্ণসংখ্যা bootprio_find_named_rom(ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা instance);
কাঠামো usbdevice_s;
পূর্ণসংখ্যা bootprio_find_usb(কাঠামো usbdevice_s *usbdev, পূর্ণসংখ্যা lun);
পূর্ণসংখ্যা get_keystroke_full(পূর্ণসংখ্যা msec);
পূর্ণসংখ্যা get_keystroke(পূর্ণসংখ্যা msec);
কাঠামো chs_s;
পূর্ণসংখ্যা boot_lchs_find_pci_device(কাঠামো pci_device *pci, কাঠামো chs_s *chs);
পূর্ণসংখ্যা boot_lchs_find_scsi_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা target, পূর্ণসংখ্যা lun,
                               কাঠামো chs_s *chs);
পূর্ণসংখ্যা boot_lchs_find_ata_device(কাঠামো pci_device *pci, পূর্ণসংখ্যা chanid, পূর্ণসংখ্যা slave,
                              কাঠামো chs_s *chs);

// bootsplash.c
শূন্য enable_vga_console(শূন্য);
শূন্য enable_bootsplash(শূন্য);
শূন্য disable_bootsplash(শূন্য);

// cdrom.c
বাহ্যিক কাঠামো eltorito_s CDEmu;
বাহ্যিক কাঠামো drive_s *cdemu_drive_gf;
কাঠামো disk_op_s;
পূর্ণসংখ্যা cdemu_process_op(কাঠামো disk_op_s *op);
শূন্য cdrom_prepboot(শূন্য);
পূর্ণসংখ্যা cdrom_boot(কাঠামো drive_s *drive_g);
অক্ষর *cdrom_media_info(কাঠামো drive_s *drive_g);

// clock.c
শূন্য clock_setup(শূন্য);
শূন্য handle_1583(কাঠামো bregs *regs);
শূন্য clock_poll_irq(শূন্য);
u32 irqtimer_calc_ticks(u32 গুনতি);
u32 irqtimer_calc(u32 msecs);
পূর্ণসংখ্যা irqtimer_check(u32 end);
শূন্য handle_1586(কাঠামো bregs *regs);

// fw/acpi.c
শূন্য acpi_setup(শূন্য);

// fw/biostable.c
শূন্য copy_pir(শূন্য *pos);
শূন্য copy_mptable(শূন্য *pos);
বাহ্যিক কাঠামো pir_header *PirAddr;
শূন্য copy_acpi_rsdp(শূন্য *pos);
বাহ্যিক কাঠামো rsdp_descriptor *RsdpAddr;
বাহ্যিক u32 acpi_pm1a_cnt;
বাহ্যিক u16 acpi_pm_base;
শূন্য *find_acpi_rsdp(শূন্য);
শূন্য *find_acpi_table(u32 signature);
u32 find_resume_vector(শূন্য);
শূন্য acpi_reboot(শূন্য);
শূন্য find_acpi_features(শূন্য);
শূন্য *smbios_get_tables(u32 *দৈর্ঘ্য);
শূন্য copy_smbios_21(শূন্য *pos);
শূন্য display_uuid(শূন্য);
শূন্য copy_table(শূন্য *pos);
শূন্য smbios_setup(শূন্য);

// fw/dsdt_parser.c
কাঠামো acpi_device;
শূন্য acpi_dsdt_parse(শূন্য);
কাঠামো acpi_device *acpi_dsdt_find_string(কাঠামো acpi_device *পূর্ব, ধ্রুবক অক্ষর *hid);
কাঠামো acpi_device *acpi_dsdt_find_eisaid(কাঠামো acpi_device *পূর্ব, u16 eisaid);
অক্ষর *acpi_dsdt_name(কাঠামো acpi_device *dev);
পূর্ণসংখ্যা acpi_dsdt_present_eisaid(u16 eisaid);
পূর্ণসংখ্যা acpi_dsdt_find_io(কাঠামো acpi_device *dev, u64 *min, u64 *max);
পূর্ণসংখ্যা acpi_dsdt_find_mem(কাঠামো acpi_device *dev, u64 *min, u64 *max);
পূর্ণসংখ্যা acpi_dsdt_find_irq(কাঠামো acpi_device *dev, u64 *irq);

// fw/coreboot.c
বাহ্যিক ধ্রুবক অক্ষর *CBvendor, *CBpart;
কাঠামো cbfs_file;
শূন্য coreboot_debug_putc(অক্ষর c);
শূন্য cbfs_run_payload(কাঠামো cbfs_file *file);
শূন্য coreboot_platform_setup(শূন্য);
শূন্য cbfs_payload_setup(শূন্য);
শূন্য coreboot_preinit(শূন্য);
শূন্য coreboot_cbfs_init(শূন্য);
কাঠামো cb_header;
শূন্য *find_cb_subtable(কাঠামো cb_header *cbh, u32 tag);
কাঠামো cb_header *find_cb_table(শূন্য);

// fw/csm.c
পূর্ণসংখ্যা csm_bootprio_fdc(কাঠামো pci_device *pci, পূর্ণসংখ্যা port, পূর্ণসংখ্যা fdid);
পূর্ণসংখ্যা csm_bootprio_ata(কাঠামো pci_device *pci, পূর্ণসংখ্যা chanid, পূর্ণসংখ্যা slave);
পূর্ণসংখ্যা csm_bootprio_pci(কাঠামো pci_device *pci);

// fw/mptable.c
শূন্য mptable_setup(শূন্য);

// fw/mtrr.c
শূন্য mtrr_setup(শূন্য);

// fw/multiboot.c
শূন্য multiboot_init(শূন্য);

// fw/pciinit.c
বাহ্যিক u64 pcimem_start, pcimem_end;
বাহ্যিক u64 pcimem64_start, pcimem64_end;
বাহ্যিক ধ্রুবক u8 pci_irqs[4];
শূন্য pci_setup(শূন্য);
শূন্য pci_resume(শূন্য);

// fw/pirtable.c
শূন্য pirtable_setup(শূন্য);

// fw/shadow.c
শূন্য make_bios_writable(শূন্য);
শূন্য make_bios_readonly(শূন্য);
শূন্য qemu_reboot(শূন্য);

// fw/smbios.c
শূন্য smbios_legacy_setup(শূন্য);

// fw/smm.c
শূন্য smm_device_setup(শূন্য);
শূন্য smm_setup(শূন্য);

// fw/smp.c
বাহ্যিক u32 Max_গুনতি_CPUs;
শূন্য wrmsr_smp(u32 সূচক, u64 val);
শূন্য smp_setup(শূন্য);
শূন্য smp_resume(শূন্য);
পূর্ণসংখ্যা apic_id_is_present(u8 apic_id);

// hw/dma.c
পূর্ণসংখ্যা dma_floppy(u32 addr, পূর্ণসংখ্যা গুনতি, পূর্ণসংখ্যা isWrite);
শূন্য dma_setup(শূন্য);

// hw/floppy.c
বাহ্যিক কাঠামো floppy_ext_dbt_s diskette_param_table2;
শূন্য floppy_setup(শূন্য);
কাঠামো drive_s *init_floppy(পূর্ণসংখ্যা floppyid, পূর্ণসংখ্যা ftype);
পূর্ণসংখ্যা find_floppy_type(u32 আকার_2);
পূর্ণসংখ্যা floppy_process_op(কাঠামো disk_op_s *op);
শূন্য floppy_tick(শূন্য);

// hw/ramdisk.c
শূন্য ramdisk_setup(শূন্য);
পূর্ণসংখ্যা ramdisk_process_op(কাঠামো disk_op_s *op);

// hw/sdcard.c
পূর্ণসংখ্যা sdcard_process_op(কাঠামো disk_op_s *op);
শূন্য sdcard_setup(শূন্য);

// hw/timer.c
শূন্য timer_setup(শূন্য);
শূন্য pmtimer_setup(u16 ioport);
শূন্য tsctimer_setfreq(u32 khz, ধ্রুবক অক্ষর *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
পূর্ণসংখ্যা timer_check(u32 end);
শূন্য ndelay(u32 গুনতি);
শূন্য udelay(u32 গুনতি);
শূন্য mdelay(u32 গুনতি);
শূন্য nsleep(u32 গুনতি);
শূন্য usleep(u32 গুনতি);
শূন্য msleep(u32 গুনতি);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
শূন্য pit_setup(শূন্য);

// jpeg.c
কাঠামো jpeg_decdata *jpeg_alloc(শূন্য);
পূর্ণসংখ্যা jpeg_decode(কাঠামো jpeg_decdata *jpeg, unsigned অক্ষর *buf);
শূন্য jpeg_get_আকার(কাঠামো jpeg_decdata *jpeg, পূর্ণসংখ্যা *width, পূর্ণসংখ্যা *height);
পূর্ণসংখ্যা jpeg_show(কাঠামো jpeg_decdata *jpeg, unsigned অক্ষর *pic, পূর্ণসংখ্যা width
              , পূর্ণসংখ্যা height, পূর্ণসংখ্যা depth, পূর্ণসংখ্যা bytes_per_সারি_dest);

// kbd.c
শূন্য kbd_init(শূন্য);
শূন্য handle_15c2(কাঠামো bregs *regs);
শূন্য process_চাবি(u8 চাবি);
u8 enqueue_চাবি(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
বাহ্যিক পূর্ণসংখ্যা HaveRunPost;
বাহ্যিক কাঠামো bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
বাহ্যিক কাঠামো floppy_dbt_s diskette_param_table __aligned(1);
বাহ্যিক u8 BiosChecksum;
পূর্ণসংখ্যা in_post(শূন্য);
শূন্য mathcp_setup(শূন্য);

// mouse.c
শূন্য mouse_init(শূন্য);
শূন্য process_mouse(u8 তথ্য);

// optionroms.c
কাঠামো rom_header;
শূন্য callrom(কাঠামো rom_header *rom, u16 bdf);
শূন্য call_bcv(u16 seg, u16 ip);
পূর্ণসংখ্যা is_pci_vga(কাঠামো pci_device *pci);
শূন্য optionrom_setup(শূন্য);
শূন্য vgarom_setup(শূন্য);
শূন্য s3_resume_vga(শূন্য);
বাহ্যিক পূর্ণসংখ্যা ScreenAndDebug;

// pcibios.c
শূন্য handle_1ab1(কাঠামো bregs *regs);
শূন্য bios32_init(শূন্য);

// pmm.c
শূন্য pmm_init(শূন্য);
শূন্য pmm_prepboot(শূন্য);

// pnpbios.c
u16 get_pnp_offset(শূন্য);
শূন্য pnp_init(শূন্য);

// post.c
শূন্য interface_init(শূন্য);
শূন্য device_hardware_setup(শূন্য);
শূন্য prepareboot(শূন্য);
শূন্য startBoot(শূন্য);
শূন্য reloc_preinit(শূন্য *f, শূন্য *arg);
শূন্য code_mutable_preinit(শূন্য);

// sercon.c
শূন্য sercon_setup(শূন্য);
শূন্য sercon_check_event(শূন্য);

// serial.c
শূন্য serial_setup(শূন্য);
শূন্য lpt_setup(শূন্য);

// version.c
বাহ্যিক ধ্রুবক অক্ষর VERSION[], BUILDINFO[];

// vgahooks.c
শূন্য handle_155f(কাঠামো bregs *regs);
শূন্য handle_157f(কাঠামো bregs *regs);
শূন্য vgahook_setup(কাঠামো pci_device *pci);

#endif // util.h
