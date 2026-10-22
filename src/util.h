// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#അന്തര്ഭാവ "types.h" // u32

// apm.c
ശൂന്യം apm_shutdown(ശൂന്യം);
ഘടന bregs;
ശൂന്യം handle_1553(ഘടന bregs *regs);

// bmp.c
ഘടന bmp_decdata *bmp_alloc(ശൂന്യം);
പൂർണ്ണസംഖ്യ bmp_decode(ഘടന bmp_decdata *bmp, unsigned അക്ഷരം *data, പൂർണ്ണസംഖ്യ data_size);
ശൂന്യം bmp_get_info(ഘടന bmp_decdata *bmp, പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height, പൂർണ്ണസംഖ്യ *bpp);
പൂർണ്ണസംഖ്യ bmp_show(ഘടന bmp_decdata *bmp, unsigned അക്ഷരം *pic, പൂർണ്ണസംഖ്യ width
             , പൂർണ്ണസംഖ്യ height, പൂർണ്ണസംഖ്യ depth, പൂർണ്ണസംഖ്യ bytes_per_വരി_dest);

// boot.c
ശൂന്യം boot_init(ശൂന്യം);
ശൂന്യം boot_add_bev(u16 seg, u16 bev, u16 desc, പൂർണ്ണസംഖ്യ prio);
ശൂന്യം boot_add_bcv(u16 seg, u16 ip, u16 desc, പൂർണ്ണസംഖ്യ prio);
ഘടന drive_s;
ശൂന്യം boot_add_floppy(ഘടന drive_s *drive_g, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio);
ശൂന്യം boot_add_hd(ഘടന drive_s *drive_g, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio);
ശൂന്യം boot_add_cd(ഘടന drive_s *drive_g, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio);
ശൂന്യം boot_add_cbfs(ശൂന്യം *data, സ്ഥിരം അക്ഷരം *desc, പൂർണ്ണസംഖ്യ prio);
ശൂന്യം interactive_bootmenu(ശൂന്യം);
ശൂന്യം bcv_prepboot(ശൂന്യം);
u8 is_bootprio_strict(ശൂന്യം);
ഘടന pci_device;
പൂർണ്ണസംഖ്യ bootprio_find_pci_device(ഘടന pci_device *pci);
പൂർണ്ണസംഖ്യ bootprio_find_mmio_device(ശൂന്യം *mmio);
പൂർണ്ണസംഖ്യ bootprio_find_scsi_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ target, പൂർണ്ണസംഖ്യ lun);
പൂർണ്ണസംഖ്യ bootprio_find_scsi_mmio_device(ശൂന്യം *mmio, പൂർണ്ണസംഖ്യ target, പൂർണ്ണസംഖ്യ lun);
പൂർണ്ണസംഖ്യ bootprio_find_ata_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ chanid, പൂർണ്ണസംഖ്യ slave);
പൂർണ്ണസംഖ്യ bootprio_find_fdc_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ port, പൂർണ്ണസംഖ്യ fdid);
പൂർണ്ണസംഖ്യ bootprio_find_pci_rom(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ instance);
പൂർണ്ണസംഖ്യ bootprio_find_named_rom(സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ instance);
ഘടന usbdevice_s;
പൂർണ്ണസംഖ്യ bootprio_find_usb(ഘടന usbdevice_s *usbdev, പൂർണ്ണസംഖ്യ lun);
പൂർണ്ണസംഖ്യ get_keystroke_full(പൂർണ്ണസംഖ്യ msec);
പൂർണ്ണസംഖ്യ get_keystroke(പൂർണ്ണസംഖ്യ msec);
ഘടന chs_s;
പൂർണ്ണസംഖ്യ boot_lchs_find_pci_device(ഘടന pci_device *pci, ഘടന chs_s *chs);
പൂർണ്ണസംഖ്യ boot_lchs_find_scsi_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ target, പൂർണ്ണസംഖ്യ lun,
                               ഘടന chs_s *chs);
പൂർണ്ണസംഖ്യ boot_lchs_find_ata_device(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ chanid, പൂർണ്ണസംഖ്യ slave,
                              ഘടന chs_s *chs);

// bootsplash.c
ശൂന്യം enable_vga_console(ശൂന്യം);
ശൂന്യം enable_bootsplash(ശൂന്യം);
ശൂന്യം disable_bootsplash(ശൂന്യം);

// cdrom.c
ബാഹ്യ ഘടന eltorito_s CDEmu;
ബാഹ്യ ഘടന drive_s *cdemu_drive_gf;
ഘടന disk_op_s;
പൂർണ്ണസംഖ്യ cdemu_process_op(ഘടന disk_op_s *op);
ശൂന്യം cdrom_prepboot(ശൂന്യം);
പൂർണ്ണസംഖ്യ cdrom_boot(ഘടന drive_s *drive_g);
അക്ഷരം *cdrom_media_info(ഘടന drive_s *drive_g);

// clock.c
ശൂന്യം clock_setup(ശൂന്യം);
ശൂന്യം handle_1583(ഘടന bregs *regs);
ശൂന്യം clock_poll_irq(ശൂന്യം);
u32 irqtimer_calc_ticks(u32 എണ്ണം);
u32 irqtimer_calc(u32 msecs);
പൂർണ്ണസംഖ്യ irqtimer_check(u32 end);
ശൂന്യം handle_1586(ഘടന bregs *regs);

// fw/acpi.c
ശൂന്യം acpi_setup(ശൂന്യം);

// fw/biostable.c
ശൂന്യം copy_pir(ശൂന്യം *pos);
ശൂന്യം copy_mptable(ശൂന്യം *pos);
ബാഹ്യ ഘടന pir_header *PirAddr;
ശൂന്യം copy_acpi_rsdp(ശൂന്യം *pos);
ബാഹ്യ ഘടന rsdp_descriptor *RsdpAddr;
ബാഹ്യ u32 acpi_pm1a_cnt;
ബാഹ്യ u16 acpi_pm_base;
ശൂന്യം *find_acpi_rsdp(ശൂന്യം);
ശൂന്യം *find_acpi_table(u32 signature);
u32 find_resume_vector(ശൂന്യം);
ശൂന്യം acpi_reboot(ശൂന്യം);
ശൂന്യം find_acpi_features(ശൂന്യം);
ശൂന്യം *smbios_get_tables(u32 *length);
ശൂന്യം copy_smbios_21(ശൂന്യം *pos);
ശൂന്യം display_uuid(ശൂന്യം);
ശൂന്യം copy_table(ശൂന്യം *pos);
ശൂന്യം smbios_setup(ശൂന്യം);

// fw/dsdt_parser.c
ഘടന acpi_device;
ശൂന്യം acpi_dsdt_parse(ശൂന്യം);
ഘടന acpi_device *acpi_dsdt_find_string(ഘടന acpi_device *prev, സ്ഥിരം അക്ഷരം *hid);
ഘടന acpi_device *acpi_dsdt_find_eisaid(ഘടന acpi_device *prev, u16 eisaid);
അക്ഷരം *acpi_dsdt_name(ഘടന acpi_device *dev);
പൂർണ്ണസംഖ്യ acpi_dsdt_present_eisaid(u16 eisaid);
പൂർണ്ണസംഖ്യ acpi_dsdt_find_io(ഘടന acpi_device *dev, u64 *min, u64 *max);
പൂർണ്ണസംഖ്യ acpi_dsdt_find_mem(ഘടന acpi_device *dev, u64 *min, u64 *max);
പൂർണ്ണസംഖ്യ acpi_dsdt_find_irq(ഘടന acpi_device *dev, u64 *irq);

// fw/coreboot.c
ബാഹ്യ സ്ഥിരം അക്ഷരം *CBvendor, *CBpart;
ഘടന cbfs_file;
ശൂന്യം coreboot_debug_putc(അക്ഷരം c);
ശൂന്യം cbfs_run_payload(ഘടന cbfs_file *file);
ശൂന്യം coreboot_platform_setup(ശൂന്യം);
ശൂന്യം cbfs_payload_setup(ശൂന്യം);
ശൂന്യം coreboot_preinit(ശൂന്യം);
ശൂന്യം coreboot_cbfs_init(ശൂന്യം);
ഘടന cb_header;
ശൂന്യം *find_cb_subtable(ഘടന cb_header *cbh, u32 tag);
ഘടന cb_header *find_cb_table(ശൂന്യം);

// fw/csm.c
പൂർണ്ണസംഖ്യ csm_bootprio_fdc(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ port, പൂർണ്ണസംഖ്യ fdid);
പൂർണ്ണസംഖ്യ csm_bootprio_ata(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ chanid, പൂർണ്ണസംഖ്യ slave);
പൂർണ്ണസംഖ്യ csm_bootprio_pci(ഘടന pci_device *pci);

// fw/mptable.c
ശൂന്യം mptable_setup(ശൂന്യം);

// fw/mtrr.c
ശൂന്യം mtrr_setup(ശൂന്യം);

// fw/multiboot.c
ശൂന്യം multiboot_init(ശൂന്യം);

// fw/pciinit.c
ബാഹ്യ u64 pcimem_start, pcimem_end;
ബാഹ്യ u64 pcimem64_start, pcimem64_end;
ബാഹ്യ സ്ഥിരം u8 pci_irqs[4];
ശൂന്യം pci_setup(ശൂന്യം);
ശൂന്യം pci_resume(ശൂന്യം);

// fw/pirtable.c
ശൂന്യം pirtable_setup(ശൂന്യം);

// fw/shadow.c
ശൂന്യം make_bios_writable(ശൂന്യം);
ശൂന്യം make_bios_readonly(ശൂന്യം);
ശൂന്യം qemu_reboot(ശൂന്യം);

// fw/smbios.c
ശൂന്യം smbios_legacy_setup(ശൂന്യം);

// fw/smm.c
ശൂന്യം smm_device_setup(ശൂന്യം);
ശൂന്യം smm_setup(ശൂന്യം);

// fw/smp.c
ബാഹ്യ u32 Max_എണ്ണം_CPUs;
ശൂന്യം wrmsr_smp(u32 index, u64 val);
ശൂന്യം smp_setup(ശൂന്യം);
ശൂന്യം smp_resume(ശൂന്യം);
പൂർണ്ണസംഖ്യ apic_id_is_present(u8 apic_id);

// hw/dma.c
പൂർണ്ണസംഖ്യ dma_floppy(u32 addr, പൂർണ്ണസംഖ്യ എണ്ണം, പൂർണ്ണസംഖ്യ isWrite);
ശൂന്യം dma_setup(ശൂന്യം);

// hw/floppy.c
ബാഹ്യ ഘടന floppy_ext_dbt_s diskette_param_table2;
ശൂന്യം floppy_setup(ശൂന്യം);
ഘടന drive_s *init_floppy(പൂർണ്ണസംഖ്യ floppyid, പൂർണ്ണസംഖ്യ ftype);
പൂർണ്ണസംഖ്യ find_floppy_type(u32 size);
പൂർണ്ണസംഖ്യ floppy_process_op(ഘടന disk_op_s *op);
ശൂന്യം floppy_tick(ശൂന്യം);

// hw/ramdisk.c
ശൂന്യം ramdisk_setup(ശൂന്യം);
പൂർണ്ണസംഖ്യ ramdisk_process_op(ഘടന disk_op_s *op);

// hw/sdcard.c
പൂർണ്ണസംഖ്യ sdcard_process_op(ഘടന disk_op_s *op);
ശൂന്യം sdcard_setup(ശൂന്യം);

// hw/timer.c
ശൂന്യം timer_setup(ശൂന്യം);
ശൂന്യം pmtimer_setup(u16 ioport);
ശൂന്യം tsctimer_setfreq(u32 khz, സ്ഥിരം അക്ഷരം *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
പൂർണ്ണസംഖ്യ timer_check(u32 end);
ശൂന്യം ndelay(u32 എണ്ണം);
ശൂന്യം udelay(u32 എണ്ണം);
ശൂന്യം mdelay(u32 എണ്ണം);
ശൂന്യം nsleep(u32 എണ്ണം);
ശൂന്യം usleep(u32 എണ്ണം);
ശൂന്യം msleep(u32 എണ്ണം);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
ശൂന്യം pit_setup(ശൂന്യം);

// jpeg.c
ഘടന jpeg_decdata *jpeg_alloc(ശൂന്യം);
പൂർണ്ണസംഖ്യ jpeg_decode(ഘടന jpeg_decdata *jpeg, unsigned അക്ഷരം *buf);
ശൂന്യം jpeg_get_size(ഘടന jpeg_decdata *jpeg, പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height);
പൂർണ്ണസംഖ്യ jpeg_show(ഘടന jpeg_decdata *jpeg, unsigned അക്ഷരം *pic, പൂർണ്ണസംഖ്യ width
              , പൂർണ്ണസംഖ്യ height, പൂർണ്ണസംഖ്യ depth, പൂർണ്ണസംഖ്യ bytes_per_വരി_dest);

// kbd.c
ശൂന്യം kbd_init(ശൂന്യം);
ശൂന്യം handle_15c2(ഘടന bregs *regs);
ശൂന്യം process_താക്കോൽ(u8 താക്കോൽ);
u8 enqueue_താക്കോൽ(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
ബാഹ്യ പൂർണ്ണസംഖ്യ HaveRunPost;
ബാഹ്യ ഘടന bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
ബാഹ്യ ഘടന floppy_dbt_s diskette_param_table __aligned(1);
ബാഹ്യ u8 BiosChecksum;
പൂർണ്ണസംഖ്യ in_post(ശൂന്യം);
ശൂന്യം mathcp_setup(ശൂന്യം);

// mouse.c
ശൂന്യം mouse_init(ശൂന്യം);
ശൂന്യം process_mouse(u8 data);

// optionroms.c
ഘടന rom_header;
ശൂന്യം callrom(ഘടന rom_header *rom, u16 bdf);
ശൂന്യം call_bcv(u16 seg, u16 ip);
പൂർണ്ണസംഖ്യ is_pci_vga(ഘടന pci_device *pci);
ശൂന്യം optionrom_setup(ശൂന്യം);
ശൂന്യം vgarom_setup(ശൂന്യം);
ശൂന്യം s3_resume_vga(ശൂന്യം);
ബാഹ്യ പൂർണ്ണസംഖ്യ ScreenAndDebug;

// pcibios.c
ശൂന്യം handle_1ab1(ഘടന bregs *regs);
ശൂന്യം bios32_init(ശൂന്യം);

// pmm.c
ശൂന്യം pmm_init(ശൂന്യം);
ശൂന്യം pmm_prepboot(ശൂന്യം);

// pnpbios.c
u16 get_pnp_offset(ശൂന്യം);
ശൂന്യം pnp_init(ശൂന്യം);

// post.c
ശൂന്യം interface_init(ശൂന്യം);
ശൂന്യം device_hardware_setup(ശൂന്യം);
ശൂന്യം prepareboot(ശൂന്യം);
ശൂന്യം startBoot(ശൂന്യം);
ശൂന്യം reloc_preinit(ശൂന്യം *f, ശൂന്യം *arg);
ശൂന്യം code_mutable_preinit(ശൂന്യം);

// sercon.c
ശൂന്യം sercon_setup(ശൂന്യം);
ശൂന്യം sercon_check_event(ശൂന്യം);

// serial.c
ശൂന്യം serial_setup(ശൂന്യം);
ശൂന്യം lpt_setup(ശൂന്യം);

// version.c
ബാഹ്യ സ്ഥിരം അക്ഷരം VERSION[], BUILDINFO[];

// vgahooks.c
ശൂന്യം handle_155f(ഘടന bregs *regs);
ശൂന്യം handle_157f(ഘടന bregs *regs);
ശൂന്യം vgahook_setup(ഘടന pci_device *pci);

#endif // util.h
