// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#अन्तर्भाव "types.h" // u32

// apm.c
शून्य apm_shutdown(शून्य);
संरचना bregs;
शून्य handle_1553(संरचना bregs *regs);

// bmp.c
संरचना bmp_decdata *bmp_alloc(शून्य);
पूर्णाङ्क bmp_decode(संरचना bmp_decdata *bmp, unsigned वर्ण *data, पूर्णाङ्क data_size);
शून्य bmp_get_info(संरचना bmp_decdata *bmp, पूर्णाङ्क *width, पूर्णाङ्क *height, पूर्णाङ्क *bpp);
पूर्णाङ्क bmp_show(संरचना bmp_decdata *bmp, unsigned वर्ण *pic, पूर्णाङ्क width
             , पूर्णाङ्क height, पूर्णाङ्क depth, पूर्णाङ्क bytes_per_पंक्ति_dest);

// boot.c
शून्य boot_init(शून्य);
शून्य boot_add_bev(u16 seg, u16 bev, u16 desc, पूर्णाङ्क prio);
शून्य boot_add_bcv(u16 seg, u16 ip, u16 desc, पूर्णाङ्क prio);
संरचना drive_s;
शून्य boot_add_floppy(संरचना drive_s *drive_g, ध्रुव वर्ण *desc, पूर्णाङ्क prio);
शून्य boot_add_hd(संरचना drive_s *drive_g, ध्रुव वर्ण *desc, पूर्णाङ्क prio);
शून्य boot_add_cd(संरचना drive_s *drive_g, ध्रुव वर्ण *desc, पूर्णाङ्क prio);
शून्य boot_add_cbfs(शून्य *data, ध्रुव वर्ण *desc, पूर्णाङ्क prio);
शून्य interactive_bootmenu(शून्य);
शून्य bcv_prepboot(शून्य);
u8 is_bootprio_strict(शून्य);
संरचना pci_device;
पूर्णाङ्क bootprio_find_pci_device(संरचना pci_device *pci);
पूर्णाङ्क bootprio_find_mmio_device(शून्य *mmio);
पूर्णाङ्क bootprio_find_scsi_device(संरचना pci_device *pci, पूर्णाङ्क target, पूर्णाङ्क lun);
पूर्णाङ्क bootprio_find_scsi_mmio_device(शून्य *mmio, पूर्णाङ्क target, पूर्णाङ्क lun);
पूर्णाङ्क bootprio_find_ata_device(संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क slave);
पूर्णाङ्क bootprio_find_fdc_device(संरचना pci_device *pci, पूर्णाङ्क port, पूर्णाङ्क fdid);
पूर्णाङ्क bootprio_find_pci_rom(संरचना pci_device *pci, पूर्णाङ्क instance);
पूर्णाङ्क bootprio_find_named_rom(ध्रुव वर्ण *name, पूर्णाङ्क instance);
संरचना usbdevice_s;
पूर्णाङ्क bootprio_find_usb(संरचना usbdevice_s *usbdev, पूर्णाङ्क lun);
पूर्णाङ्क get_keystroke_full(पूर्णाङ्क msec);
पूर्णाङ्क get_keystroke(पूर्णाङ्क msec);
संरचना chs_s;
पूर्णाङ्क boot_lchs_find_pci_device(संरचना pci_device *pci, संरचना chs_s *chs);
पूर्णाङ्क boot_lchs_find_scsi_device(संरचना pci_device *pci, पूर्णाङ्क target, पूर्णाङ्क lun,
                               संरचना chs_s *chs);
पूर्णाङ्क boot_lchs_find_ata_device(संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क slave,
                              संरचना chs_s *chs);

// bootsplash.c
शून्य enable_vga_console(शून्य);
शून्य enable_bootsplash(शून्य);
शून्य disable_bootsplash(शून्य);

// cdrom.c
बाह्य संरचना eltorito_s CDEmu;
बाह्य संरचना drive_s *cdemu_drive_gf;
संरचना disk_op_s;
पूर्णाङ्क cdemu_process_op(संरचना disk_op_s *op);
शून्य cdrom_prepboot(शून्य);
पूर्णाङ्क cdrom_boot(संरचना drive_s *drive_g);
वर्ण *cdrom_media_info(संरचना drive_s *drive_g);

// clock.c
शून्य clock_setup(शून्य);
शून्य handle_1583(संरचना bregs *regs);
शून्य clock_poll_irq(शून्य);
u32 irqtimer_calc_ticks(u32 गिनती);
u32 irqtimer_calc(u32 msecs);
पूर्णाङ्क irqtimer_check(u32 end);
शून्य handle_1586(संरचना bregs *regs);

// fw/acpi.c
शून्य acpi_setup(शून्य);

// fw/biostable.c
शून्य copy_pir(शून्य *pos);
शून्य copy_mptable(शून्य *pos);
बाह्य संरचना pir_header *PirAddr;
शून्य copy_acpi_rsdp(शून्य *pos);
बाह्य संरचना rsdp_descriptor *RsdpAddr;
बाह्य u32 acpi_pm1a_cnt;
बाह्य u16 acpi_pm_base;
शून्य *find_acpi_rsdp(शून्य);
शून्य *find_acpi_table(u32 signature);
u32 find_resume_vector(शून्य);
शून्य acpi_reboot(शून्य);
शून्य find_acpi_features(शून्य);
शून्य *smbios_get_tables(u32 *length);
शून्य copy_smbios_21(शून्य *pos);
शून्य display_uuid(शून्य);
शून्य copy_table(शून्य *pos);
शून्य smbios_setup(शून्य);

// fw/dsdt_parser.c
संरचना acpi_device;
शून्य acpi_dsdt_parse(शून्य);
संरचना acpi_device *acpi_dsdt_find_string(संरचना acpi_device *prev, ध्रुव वर्ण *hid);
संरचना acpi_device *acpi_dsdt_find_eisaid(संरचना acpi_device *prev, u16 eisaid);
वर्ण *acpi_dsdt_name(संरचना acpi_device *dev);
पूर्णाङ्क acpi_dsdt_present_eisaid(u16 eisaid);
पूर्णाङ्क acpi_dsdt_find_io(संरचना acpi_device *dev, u64 *min, u64 *max);
पूर्णाङ्क acpi_dsdt_find_mem(संरचना acpi_device *dev, u64 *min, u64 *max);
पूर्णाङ्क acpi_dsdt_find_irq(संरचना acpi_device *dev, u64 *irq);

// fw/coreboot.c
बाह्य ध्रुव वर्ण *CBvendor, *CBpart;
संरचना cbfs_file;
शून्य coreboot_debug_putc(वर्ण c);
शून्य cbfs_run_payload(संरचना cbfs_file *file);
शून्य coreboot_platform_setup(शून्य);
शून्य cbfs_payload_setup(शून्य);
शून्य coreboot_preinit(शून्य);
शून्य coreboot_cbfs_init(शून्य);
संरचना cb_header;
शून्य *find_cb_subtable(संरचना cb_header *cbh, u32 tag);
संरचना cb_header *find_cb_table(शून्य);

// fw/csm.c
पूर्णाङ्क csm_bootprio_fdc(संरचना pci_device *pci, पूर्णाङ्क port, पूर्णाङ्क fdid);
पूर्णाङ्क csm_bootprio_ata(संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क slave);
पूर्णाङ्क csm_bootprio_pci(संरचना pci_device *pci);

// fw/mptable.c
शून्य mptable_setup(शून्य);

// fw/mtrr.c
शून्य mtrr_setup(शून्य);

// fw/multiboot.c
शून्य multiboot_init(शून्य);

// fw/pciinit.c
बाह्य u64 pcimem_start, pcimem_end;
बाह्य u64 pcimem64_start, pcimem64_end;
बाह्य ध्रुव u8 pci_irqs[4];
शून्य pci_setup(शून्य);
शून्य pci_resume(शून्य);

// fw/pirtable.c
शून्य pirtable_setup(शून्य);

// fw/shadow.c
शून्य make_bios_writable(शून्य);
शून्य make_bios_readonly(शून्य);
शून्य qemu_reboot(शून्य);

// fw/smbios.c
शून्य smbios_legacy_setup(शून्य);

// fw/smm.c
शून्य smm_device_setup(शून्य);
शून्य smm_setup(शून्य);

// fw/smp.c
बाह्य u32 Max_गिनती_CPUs;
शून्य wrmsr_smp(u32 index, u64 val);
शून्य smp_setup(शून्य);
शून्य smp_resume(शून्य);
पूर्णाङ्क apic_id_is_present(u8 apic_id);

// hw/dma.c
पूर्णाङ्क dma_floppy(u32 addr, पूर्णाङ्क गिनती, पूर्णाङ्क isWrite);
शून्य dma_setup(शून्य);

// hw/floppy.c
बाह्य संरचना floppy_ext_dbt_s diskette_param_table2;
शून्य floppy_setup(शून्य);
संरचना drive_s *init_floppy(पूर्णाङ्क floppyid, पूर्णाङ्क ftype);
पूर्णाङ्क find_floppy_type(u32 size);
पूर्णाङ्क floppy_process_op(संरचना disk_op_s *op);
शून्य floppy_tick(शून्य);

// hw/ramdisk.c
शून्य ramdisk_setup(शून्य);
पूर्णाङ्क ramdisk_process_op(संरचना disk_op_s *op);

// hw/sdcard.c
पूर्णाङ्क sdcard_process_op(संरचना disk_op_s *op);
शून्य sdcard_setup(शून्य);

// hw/timer.c
शून्य timer_setup(शून्य);
शून्य pmtimer_setup(u16 ioport);
शून्य tsctimer_setfreq(u32 khz, ध्रुव वर्ण *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
पूर्णाङ्क timer_check(u32 end);
शून्य ndelay(u32 गिनती);
शून्य udelay(u32 गिनती);
शून्य mdelay(u32 गिनती);
शून्य nsleep(u32 गिनती);
शून्य usleep(u32 गिनती);
शून्य msleep(u32 गिनती);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
शून्य pit_setup(शून्य);

// jpeg.c
संरचना jpeg_decdata *jpeg_alloc(शून्य);
पूर्णाङ्क jpeg_decode(संरचना jpeg_decdata *jpeg, unsigned वर्ण *buf);
शून्य jpeg_get_size(संरचना jpeg_decdata *jpeg, पूर्णाङ्क *width, पूर्णाङ्क *height);
पूर्णाङ्क jpeg_show(संरचना jpeg_decdata *jpeg, unsigned वर्ण *pic, पूर्णाङ्क width
              , पूर्णाङ्क height, पूर्णाङ्क depth, पूर्णाङ्क bytes_per_पंक्ति_dest);

// kbd.c
शून्य kbd_init(शून्य);
शून्य handle_15c2(संरचना bregs *regs);
शून्य process_कुंजी(u8 कुंजी);
u8 enqueue_कुंजी(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
बाह्य पूर्णाङ्क HaveRunPost;
बाह्य संरचना bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
बाह्य संरचना floppy_dbt_s diskette_param_table __aligned(1);
बाह्य u8 BiosChecksum;
पूर्णाङ्क in_post(शून्य);
शून्य mathcp_setup(शून्य);

// mouse.c
शून्य mouse_init(शून्य);
शून्य process_mouse(u8 data);

// optionroms.c
संरचना rom_header;
शून्य callrom(संरचना rom_header *rom, u16 bdf);
शून्य call_bcv(u16 seg, u16 ip);
पूर्णाङ्क is_pci_vga(संरचना pci_device *pci);
शून्य optionrom_setup(शून्य);
शून्य vgarom_setup(शून्य);
शून्य s3_resume_vga(शून्य);
बाह्य पूर्णाङ्क ScreenAndDebug;

// pcibios.c
शून्य handle_1ab1(संरचना bregs *regs);
शून्य bios32_init(शून्य);

// pmm.c
शून्य pmm_init(शून्य);
शून्य pmm_prepboot(शून्य);

// pnpbios.c
u16 get_pnp_offset(शून्य);
शून्य pnp_init(शून्य);

// post.c
शून्य interface_init(शून्य);
शून्य device_hardware_setup(शून्य);
शून्य prepareboot(शून्य);
शून्य startBoot(शून्य);
शून्य reloc_preinit(शून्य *f, शून्य *arg);
शून्य code_mutable_preinit(शून्य);

// sercon.c
शून्य sercon_setup(शून्य);
शून्य sercon_check_event(शून्य);

// serial.c
शून्य serial_setup(शून्य);
शून्य lpt_setup(शून्य);

// version.c
बाह्य ध्रुव वर्ण VERSION[], BUILDINFO[];

// vgahooks.c
शून्य handle_155f(संरचना bregs *regs);
शून्य handle_157f(संरचना bregs *regs);
शून्य vgahook_setup(संरचना pci_device *pci);

#endif // util.h
