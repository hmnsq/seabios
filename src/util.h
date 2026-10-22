// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#অন্তর্ভাৱ "types.h" // u32

// apm.c
শূন্য apm_shutdown(শূন্য);
সংরচনা bregs;
শূন্য handle_1553(সংরচনা bregs *regs);

// bmp.c
সংরচনা bmp_decdata *bmp_alloc(শূন্য);
পূর্ণাঙ্ক bmp_decode(সংরচনা bmp_decdata *bmp, unsigned ৱর্ণ *data, পূর্ণাঙ্ক data_size);
শূন্য bmp_get_info(সংরচনা bmp_decdata *bmp, পূর্ণাঙ্ক *width, পূর্ণাঙ্ক *height, পূর্ণাঙ্ক *bpp);
পূর্ণাঙ্ক bmp_show(সংরচনা bmp_decdata *bmp, unsigned ৱর্ণ *pic, পূর্ণাঙ্ক width
             , পূর্ণাঙ্ক height, পূর্ণাঙ্ক depth, পূর্ণাঙ্ক bytes_per_সাৰি_dest);

// boot.c
শূন্য boot_init(শূন্য);
শূন্য boot_add_bev(u16 seg, u16 bev, u16 desc, পূর্ণাঙ্ক prio);
শূন্য boot_add_bcv(u16 seg, u16 ip, u16 desc, পূর্ণাঙ্ক prio);
সংরচনা drive_s;
শূন্য boot_add_floppy(সংরচনা drive_s *drive_g, ধ্রুৱ ৱর্ণ *desc, পূর্ণাঙ্ক prio);
শূন্য boot_add_hd(সংরচনা drive_s *drive_g, ধ্রুৱ ৱর্ণ *desc, পূর্ণাঙ্ক prio);
শূন্য boot_add_cd(সংরচনা drive_s *drive_g, ধ্রুৱ ৱর্ণ *desc, পূর্ণাঙ্ক prio);
শূন্য boot_add_cbfs(শূন্য *data, ধ্রুৱ ৱর্ণ *desc, পূর্ণাঙ্ক prio);
শূন্য interactive_bootmenu(শূন্য);
শূন্য bcv_prepboot(শূন্য);
u8 is_bootprio_strict(শূন্য);
সংরচনা pci_device;
পূর্ণাঙ্ক bootprio_find_pci_device(সংরচনা pci_device *pci);
পূর্ণাঙ্ক bootprio_find_mmio_device(শূন্য *mmio);
পূর্ণাঙ্ক bootprio_find_scsi_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক target, পূর্ণাঙ্ক lun);
পূর্ণাঙ্ক bootprio_find_scsi_mmio_device(শূন্য *mmio, পূর্ণাঙ্ক target, পূর্ণাঙ্ক lun);
পূর্ণাঙ্ক bootprio_find_ata_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক chanid, পূর্ণাঙ্ক slave);
পূর্ণাঙ্ক bootprio_find_fdc_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক port, পূর্ণাঙ্ক fdid);
পূর্ণাঙ্ক bootprio_find_pci_rom(সংরচনা pci_device *pci, পূর্ণাঙ্ক instance);
পূর্ণাঙ্ক bootprio_find_named_rom(ধ্রুৱ ৱর্ণ *name, পূর্ণাঙ্ক instance);
সংরচনা usbdevice_s;
পূর্ণাঙ্ক bootprio_find_usb(সংরচনা usbdevice_s *usbdev, পূর্ণাঙ্ক lun);
পূর্ণাঙ্ক get_keystroke_full(পূর্ণাঙ্ক msec);
পূর্ণাঙ্ক get_keystroke(পূর্ণাঙ্ক msec);
সংরচনা chs_s;
পূর্ণাঙ্ক boot_lchs_find_pci_device(সংরচনা pci_device *pci, সংরচনা chs_s *chs);
পূর্ণাঙ্ক boot_lchs_find_scsi_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক target, পূর্ণাঙ্ক lun,
                               সংরচনা chs_s *chs);
পূর্ণাঙ্ক boot_lchs_find_ata_device(সংরচনা pci_device *pci, পূর্ণাঙ্ক chanid, পূর্ণাঙ্ক slave,
                              সংরচনা chs_s *chs);

// bootsplash.c
শূন্য enable_vga_console(শূন্য);
শূন্য enable_bootsplash(শূন্য);
শূন্য disable_bootsplash(শূন্য);

// cdrom.c
বাহ্য সংরচনা eltorito_s CDEmu;
বাহ্য সংরচনা drive_s *cdemu_drive_gf;
সংরচনা disk_op_s;
পূর্ণাঙ্ক cdemu_process_op(সংরচনা disk_op_s *op);
শূন্য cdrom_prepboot(শূন্য);
পূর্ণাঙ্ক cdrom_boot(সংরচনা drive_s *drive_g);
ৱর্ণ *cdrom_media_info(সংরচনা drive_s *drive_g);

// clock.c
শূন্য clock_setup(শূন্য);
শূন্য handle_1583(সংরচনা bregs *regs);
শূন্য clock_poll_irq(শূন্য);
u32 irqtimer_calc_ticks(u32 গুনতি);
u32 irqtimer_calc(u32 msecs);
পূর্ণাঙ্ক irqtimer_check(u32 end);
শূন্য handle_1586(সংরচনা bregs *regs);

// fw/acpi.c
শূন্য acpi_setup(শূন্য);

// fw/biostable.c
শূন্য copy_pir(শূন্য *pos);
শূন্য copy_mptable(শূন্য *pos);
বাহ্য সংরচনা pir_header *PirAddr;
শূন্য copy_acpi_rsdp(শূন্য *pos);
বাহ্য সংরচনা rsdp_descriptor *RsdpAddr;
বাহ্য u32 acpi_pm1a_cnt;
বাহ্য u16 acpi_pm_base;
শূন্য *find_acpi_rsdp(শূন্য);
শূন্য *find_acpi_table(u32 signature);
u32 find_resume_vector(শূন্য);
শূন্য acpi_reboot(শূন্য);
শূন্য find_acpi_features(শূন্য);
শূন্য *smbios_get_tables(u32 *length);
শূন্য copy_smbios_21(শূন্য *pos);
শূন্য display_uuid(শূন্য);
শূন্য copy_table(শূন্য *pos);
শূন্য smbios_setup(শূন্য);

// fw/dsdt_parser.c
সংরচনা acpi_device;
শূন্য acpi_dsdt_parse(শূন্য);
সংরচনা acpi_device *acpi_dsdt_find_string(সংরচনা acpi_device *prev, ধ্রুৱ ৱর্ণ *hid);
সংরচনা acpi_device *acpi_dsdt_find_eisaid(সংরচনা acpi_device *prev, u16 eisaid);
ৱর্ণ *acpi_dsdt_name(সংরচনা acpi_device *dev);
পূর্ণাঙ্ক acpi_dsdt_present_eisaid(u16 eisaid);
পূর্ণাঙ্ক acpi_dsdt_find_io(সংরচনা acpi_device *dev, u64 *min, u64 *max);
পূর্ণাঙ্ক acpi_dsdt_find_mem(সংরচনা acpi_device *dev, u64 *min, u64 *max);
পূর্ণাঙ্ক acpi_dsdt_find_irq(সংরচনা acpi_device *dev, u64 *irq);

// fw/coreboot.c
বাহ্য ধ্রুৱ ৱর্ণ *CBvendor, *CBpart;
সংরচনা cbfs_file;
শূন্য coreboot_debug_putc(ৱর্ণ c);
শূন্য cbfs_run_payload(সংরচনা cbfs_file *file);
শূন্য coreboot_platform_setup(শূন্য);
শূন্য cbfs_payload_setup(শূন্য);
শূন্য coreboot_preinit(শূন্য);
শূন্য coreboot_cbfs_init(শূন্য);
সংরচনা cb_header;
শূন্য *find_cb_subtable(সংরচনা cb_header *cbh, u32 tag);
সংরচনা cb_header *find_cb_table(শূন্য);

// fw/csm.c
পূর্ণাঙ্ক csm_bootprio_fdc(সংরচনা pci_device *pci, পূর্ণাঙ্ক port, পূর্ণাঙ্ক fdid);
পূর্ণাঙ্ক csm_bootprio_ata(সংরচনা pci_device *pci, পূর্ণাঙ্ক chanid, পূর্ণাঙ্ক slave);
পূর্ণাঙ্ক csm_bootprio_pci(সংরচনা pci_device *pci);

// fw/mptable.c
শূন্য mptable_setup(শূন্য);

// fw/mtrr.c
শূন্য mtrr_setup(শূন্য);

// fw/multiboot.c
শূন্য multiboot_init(শূন্য);

// fw/pciinit.c
বাহ্য u64 pcimem_start, pcimem_end;
বাহ্য u64 pcimem64_start, pcimem64_end;
বাহ্য ধ্রুৱ u8 pci_irqs[4];
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
বাহ্য u32 Max_গুনতি_CPUs;
শূন্য wrmsr_smp(u32 index, u64 val);
শূন্য smp_setup(শূন্য);
শূন্য smp_resume(শূন্য);
পূর্ণাঙ্ক apic_id_is_present(u8 apic_id);

// hw/dma.c
পূর্ণাঙ্ক dma_floppy(u32 addr, পূর্ণাঙ্ক গুনতি, পূর্ণাঙ্ক isWrite);
শূন্য dma_setup(শূন্য);

// hw/floppy.c
বাহ্য সংরচনা floppy_ext_dbt_s diskette_param_table2;
শূন্য floppy_setup(শূন্য);
সংরচনা drive_s *init_floppy(পূর্ণাঙ্ক floppyid, পূর্ণাঙ্ক ftype);
পূর্ণাঙ্ক find_floppy_type(u32 size);
পূর্ণাঙ্ক floppy_process_op(সংরচনা disk_op_s *op);
শূন্য floppy_tick(শূন্য);

// hw/ramdisk.c
শূন্য ramdisk_setup(শূন্য);
পূর্ণাঙ্ক ramdisk_process_op(সংরচনা disk_op_s *op);

// hw/sdcard.c
পূর্ণাঙ্ক sdcard_process_op(সংরচনা disk_op_s *op);
শূন্য sdcard_setup(শূন্য);

// hw/timer.c
শূন্য timer_setup(শূন্য);
শূন্য pmtimer_setup(u16 ioport);
শূন্য tsctimer_setfreq(u32 khz, ধ্রুৱ ৱর্ণ *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
পূর্ণাঙ্ক timer_check(u32 end);
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
সংরচনা jpeg_decdata *jpeg_alloc(শূন্য);
পূর্ণাঙ্ক jpeg_decode(সংরচনা jpeg_decdata *jpeg, unsigned ৱর্ণ *buf);
শূন্য jpeg_get_size(সংরচনা jpeg_decdata *jpeg, পূর্ণাঙ্ক *width, পূর্ণাঙ্ক *height);
পূর্ণাঙ্ক jpeg_show(সংরচনা jpeg_decdata *jpeg, unsigned ৱর্ণ *pic, পূর্ণাঙ্ক width
              , পূর্ণাঙ্ক height, পূর্ণাঙ্ক depth, পূর্ণাঙ্ক bytes_per_সাৰি_dest);

// kbd.c
শূন্য kbd_init(শূন্য);
শূন্য handle_15c2(সংরচনা bregs *regs);
শূন্য process_চাবি(u8 চাবি);
u8 enqueue_চাবি(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
বাহ্য পূর্ণাঙ্ক HaveRunPost;
বাহ্য সংরচনা bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
বাহ্য সংরচনা floppy_dbt_s diskette_param_table __aligned(1);
বাহ্য u8 BiosChecksum;
পূর্ণাঙ্ক in_post(শূন্য);
শূন্য mathcp_setup(শূন্য);

// mouse.c
শূন্য mouse_init(শূন্য);
শূন্য process_mouse(u8 data);

// optionroms.c
সংরচনা rom_header;
শূন্য callrom(সংরচনা rom_header *rom, u16 bdf);
শূন্য call_bcv(u16 seg, u16 ip);
পূর্ণাঙ্ক is_pci_vga(সংরচনা pci_device *pci);
শূন্য optionrom_setup(শূন্য);
শূন্য vgarom_setup(শূন্য);
শূন্য s3_resume_vga(শূন্য);
বাহ্য পূর্ণাঙ্ক ScreenAndDebug;

// pcibios.c
শূন্য handle_1ab1(সংরচনা bregs *regs);
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
বাহ্য ধ্রুৱ ৱর্ণ VERSION[], BUILDINFO[];

// vgahooks.c
শূন্য handle_155f(সংরচনা bregs *regs);
শূন্য handle_157f(সংরচনা bregs *regs);
শূন্য vgahook_setup(সংরচনা pci_device *pci);

#endif // util.h
