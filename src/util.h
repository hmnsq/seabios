// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#அந்தர்பாவ "types.h" // u32

// apm.c
வெறுமை apm_shutdown(வெறுமை);
கட்டமைப்பு bregs;
வெறுமை handle_1553(கட்டமைப்பு bregs *regs);

// bmp.c
கட்டமைப்பு bmp_decdata *bmp_alloc(வெறுமை);
முழு bmp_decode(கட்டமைப்பு bmp_decdata *bmp, unsigned எழுத்து *data, முழு data_size);
வெறுமை bmp_get_info(கட்டமைப்பு bmp_decdata *bmp, முழு *width, முழு *height, முழு *bpp);
முழு bmp_show(கட்டமைப்பு bmp_decdata *bmp, unsigned எழுத்து *pic, முழு width
             , முழு height, முழு depth, முழு bytes_per_வரி_dest);

// boot.c
வெறுமை boot_init(வெறுமை);
வெறுமை boot_add_bev(u16 seg, u16 bev, u16 desc, முழு prio);
வெறுமை boot_add_bcv(u16 seg, u16 ip, u16 desc, முழு prio);
கட்டமைப்பு drive_s;
வெறுமை boot_add_floppy(கட்டமைப்பு drive_s *drive_g, மாறிலி எழுத்து *desc, முழு prio);
வெறுமை boot_add_hd(கட்டமைப்பு drive_s *drive_g, மாறிலி எழுத்து *desc, முழு prio);
வெறுமை boot_add_cd(கட்டமைப்பு drive_s *drive_g, மாறிலி எழுத்து *desc, முழு prio);
வெறுமை boot_add_cbfs(வெறுமை *data, மாறிலி எழுத்து *desc, முழு prio);
வெறுமை interactive_bootmenu(வெறுமை);
வெறுமை bcv_prepboot(வெறுமை);
u8 is_bootprio_strict(வெறுமை);
கட்டமைப்பு pci_device;
முழு bootprio_find_pci_device(கட்டமைப்பு pci_device *pci);
முழு bootprio_find_mmio_device(வெறுமை *mmio);
முழு bootprio_find_scsi_device(கட்டமைப்பு pci_device *pci, முழு target, முழு lun);
முழு bootprio_find_scsi_mmio_device(வெறுமை *mmio, முழு target, முழு lun);
முழு bootprio_find_ata_device(கட்டமைப்பு pci_device *pci, முழு chanid, முழு slave);
முழு bootprio_find_fdc_device(கட்டமைப்பு pci_device *pci, முழு port, முழு fdid);
முழு bootprio_find_pci_rom(கட்டமைப்பு pci_device *pci, முழு instance);
முழு bootprio_find_named_rom(மாறிலி எழுத்து *name, முழு instance);
கட்டமைப்பு usbdevice_s;
முழு bootprio_find_usb(கட்டமைப்பு usbdevice_s *usbdev, முழு lun);
முழு get_keystroke_full(முழு msec);
முழு get_keystroke(முழு msec);
கட்டமைப்பு chs_s;
முழு boot_lchs_find_pci_device(கட்டமைப்பு pci_device *pci, கட்டமைப்பு chs_s *chs);
முழு boot_lchs_find_scsi_device(கட்டமைப்பு pci_device *pci, முழு target, முழு lun,
                               கட்டமைப்பு chs_s *chs);
முழு boot_lchs_find_ata_device(கட்டமைப்பு pci_device *pci, முழு chanid, முழு slave,
                              கட்டமைப்பு chs_s *chs);

// bootsplash.c
வெறுமை enable_vga_console(வெறுமை);
வெறுமை enable_bootsplash(வெறுமை);
வெறுமை disable_bootsplash(வெறுமை);

// cdrom.c
வெளி கட்டமைப்பு eltorito_s CDEmu;
வெளி கட்டமைப்பு drive_s *cdemu_drive_gf;
கட்டமைப்பு disk_op_s;
முழு cdemu_process_op(கட்டமைப்பு disk_op_s *op);
வெறுமை cdrom_prepboot(வெறுமை);
முழு cdrom_boot(கட்டமைப்பு drive_s *drive_g);
எழுத்து *cdrom_media_info(கட்டமைப்பு drive_s *drive_g);

// clock.c
வெறுமை clock_setup(வெறுமை);
வெறுமை handle_1583(கட்டமைப்பு bregs *regs);
வெறுமை clock_poll_irq(வெறுமை);
u32 irqtimer_calc_ticks(u32 மொத்தம்);
u32 irqtimer_calc(u32 msecs);
முழு irqtimer_check(u32 end);
வெறுமை handle_1586(கட்டமைப்பு bregs *regs);

// fw/acpi.c
வெறுமை acpi_setup(வெறுமை);

// fw/biostable.c
வெறுமை copy_pir(வெறுமை *pos);
வெறுமை copy_mptable(வெறுமை *pos);
வெளி கட்டமைப்பு pir_header *PirAddr;
வெறுமை copy_acpi_rsdp(வெறுமை *pos);
வெளி கட்டமைப்பு rsdp_descriptor *RsdpAddr;
வெளி u32 acpi_pm1a_cnt;
வெளி u16 acpi_pm_base;
வெறுமை *find_acpi_rsdp(வெறுமை);
வெறுமை *find_acpi_table(u32 signature);
u32 find_resume_vector(வெறுமை);
வெறுமை acpi_reboot(வெறுமை);
வெறுமை find_acpi_features(வெறுமை);
வெறுமை *smbios_get_tables(u32 *length);
வெறுமை copy_smbios_21(வெறுமை *pos);
வெறுமை display_uuid(வெறுமை);
வெறுமை copy_table(வெறுமை *pos);
வெறுமை smbios_setup(வெறுமை);

// fw/dsdt_parser.c
கட்டமைப்பு acpi_device;
வெறுமை acpi_dsdt_parse(வெறுமை);
கட்டமைப்பு acpi_device *acpi_dsdt_find_string(கட்டமைப்பு acpi_device *prev, மாறிலி எழுத்து *hid);
கட்டமைப்பு acpi_device *acpi_dsdt_find_eisaid(கட்டமைப்பு acpi_device *prev, u16 eisaid);
எழுத்து *acpi_dsdt_name(கட்டமைப்பு acpi_device *dev);
முழு acpi_dsdt_present_eisaid(u16 eisaid);
முழு acpi_dsdt_find_io(கட்டமைப்பு acpi_device *dev, u64 *min, u64 *max);
முழு acpi_dsdt_find_mem(கட்டமைப்பு acpi_device *dev, u64 *min, u64 *max);
முழு acpi_dsdt_find_irq(கட்டமைப்பு acpi_device *dev, u64 *irq);

// fw/coreboot.c
வெளி மாறிலி எழுத்து *CBvendor, *CBpart;
கட்டமைப்பு cbfs_file;
வெறுமை coreboot_debug_putc(எழுத்து c);
வெறுமை cbfs_run_payload(கட்டமைப்பு cbfs_file *file);
வெறுமை coreboot_platform_setup(வெறுமை);
வெறுமை cbfs_payload_setup(வெறுமை);
வெறுமை coreboot_preinit(வெறுமை);
வெறுமை coreboot_cbfs_init(வெறுமை);
கட்டமைப்பு cb_header;
வெறுமை *find_cb_subtable(கட்டமைப்பு cb_header *cbh, u32 tag);
கட்டமைப்பு cb_header *find_cb_table(வெறுமை);

// fw/csm.c
முழு csm_bootprio_fdc(கட்டமைப்பு pci_device *pci, முழு port, முழு fdid);
முழு csm_bootprio_ata(கட்டமைப்பு pci_device *pci, முழு chanid, முழு slave);
முழு csm_bootprio_pci(கட்டமைப்பு pci_device *pci);

// fw/mptable.c
வெறுமை mptable_setup(வெறுமை);

// fw/mtrr.c
வெறுமை mtrr_setup(வெறுமை);

// fw/multiboot.c
வெறுமை multiboot_init(வெறுமை);

// fw/pciinit.c
வெளி u64 pcimem_start, pcimem_end;
வெளி u64 pcimem64_start, pcimem64_end;
வெளி மாறிலி u8 pci_irqs[4];
வெறுமை pci_setup(வெறுமை);
வெறுமை pci_resume(வெறுமை);

// fw/pirtable.c
வெறுமை pirtable_setup(வெறுமை);

// fw/shadow.c
வெறுமை make_bios_writable(வெறுமை);
வெறுமை make_bios_readonly(வெறுமை);
வெறுமை qemu_reboot(வெறுமை);

// fw/smbios.c
வெறுமை smbios_legacy_setup(வெறுமை);

// fw/smm.c
வெறுமை smm_device_setup(வெறுமை);
வெறுமை smm_setup(வெறுமை);

// fw/smp.c
வெளி u32 Max_மொத்தம்_CPUs;
வெறுமை wrmsr_smp(u32 index, u64 val);
வெறுமை smp_setup(வெறுமை);
வெறுமை smp_resume(வெறுமை);
முழு apic_id_is_present(u8 apic_id);

// hw/dma.c
முழு dma_floppy(u32 addr, முழு மொத்தம், முழு isWrite);
வெறுமை dma_setup(வெறுமை);

// hw/floppy.c
வெளி கட்டமைப்பு floppy_ext_dbt_s diskette_param_table2;
வெறுமை floppy_setup(வெறுமை);
கட்டமைப்பு drive_s *init_floppy(முழு floppyid, முழு ftype);
முழு find_floppy_type(u32 size);
முழு floppy_process_op(கட்டமைப்பு disk_op_s *op);
வெறுமை floppy_tick(வெறுமை);

// hw/ramdisk.c
வெறுமை ramdisk_setup(வெறுமை);
முழு ramdisk_process_op(கட்டமைப்பு disk_op_s *op);

// hw/sdcard.c
முழு sdcard_process_op(கட்டமைப்பு disk_op_s *op);
வெறுமை sdcard_setup(வெறுமை);

// hw/timer.c
வெறுமை timer_setup(வெறுமை);
வெறுமை pmtimer_setup(u16 ioport);
வெறுமை tsctimer_setfreq(u32 khz, மாறிலி எழுத்து *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
முழு timer_check(u32 end);
வெறுமை ndelay(u32 மொத்தம்);
வெறுமை udelay(u32 மொத்தம்);
வெறுமை mdelay(u32 மொத்தம்);
வெறுமை nsleep(u32 மொத்தம்);
வெறுமை usleep(u32 மொத்தம்);
வெறுமை msleep(u32 மொத்தம்);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
வெறுமை pit_setup(வெறுமை);

// jpeg.c
கட்டமைப்பு jpeg_decdata *jpeg_alloc(வெறுமை);
முழு jpeg_decode(கட்டமைப்பு jpeg_decdata *jpeg, unsigned எழுத்து *buf);
வெறுமை jpeg_get_size(கட்டமைப்பு jpeg_decdata *jpeg, முழு *width, முழு *height);
முழு jpeg_show(கட்டமைப்பு jpeg_decdata *jpeg, unsigned எழுத்து *pic, முழு width
              , முழு height, முழு depth, முழு bytes_per_வரி_dest);

// kbd.c
வெறுமை kbd_init(வெறுமை);
வெறுமை handle_15c2(கட்டமைப்பு bregs *regs);
வெறுமை process_சாவி(u8 சாவி);
u8 enqueue_சாவி(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
வெளி முழு HaveRunPost;
வெளி கட்டமைப்பு bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
வெளி கட்டமைப்பு floppy_dbt_s diskette_param_table __aligned(1);
வெளி u8 BiosChecksum;
முழு in_post(வெறுமை);
வெறுமை mathcp_setup(வெறுமை);

// mouse.c
வெறுமை mouse_init(வெறுமை);
வெறுமை process_mouse(u8 data);

// optionroms.c
கட்டமைப்பு rom_header;
வெறுமை callrom(கட்டமைப்பு rom_header *rom, u16 bdf);
வெறுமை call_bcv(u16 seg, u16 ip);
முழு is_pci_vga(கட்டமைப்பு pci_device *pci);
வெறுமை optionrom_setup(வெறுமை);
வெறுமை vgarom_setup(வெறுமை);
வெறுமை s3_resume_vga(வெறுமை);
வெளி முழு ScreenAndDebug;

// pcibios.c
வெறுமை handle_1ab1(கட்டமைப்பு bregs *regs);
வெறுமை bios32_init(வெறுமை);

// pmm.c
வெறுமை pmm_init(வெறுமை);
வெறுமை pmm_prepboot(வெறுமை);

// pnpbios.c
u16 get_pnp_offset(வெறுமை);
வெறுமை pnp_init(வெறுமை);

// post.c
வெறுமை interface_init(வெறுமை);
வெறுமை device_hardware_setup(வெறுமை);
வெறுமை prepareboot(வெறுமை);
வெறுமை startBoot(வெறுமை);
வெறுமை reloc_preinit(வெறுமை *f, வெறுமை *arg);
வெறுமை code_mutable_preinit(வெறுமை);

// sercon.c
வெறுமை sercon_setup(வெறுமை);
வெறுமை sercon_check_event(வெறுமை);

// serial.c
வெறுமை serial_setup(வெறுமை);
வெறுமை lpt_setup(வெறுமை);

// version.c
வெளி மாறிலி எழுத்து VERSION[], BUILDINFO[];

// vgahooks.c
வெறுமை handle_155f(கட்டமைப்பு bregs *regs);
வெறுமை handle_157f(கட்டமைப்பு bregs *regs);
வெறுமை vgahook_setup(கட்டமைப்பு pci_device *pci);

#endif // util.h
