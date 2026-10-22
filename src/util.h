// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#شامل "types.h" // u32

// apm.c
تهی apm_shutdown(تهی);
ساختار bregs;
تهی handle_1553(ساختار bregs *regs);

// bmp.c
ساختار bmp_decdata *bmp_alloc(تهی);
صحیح bmp_decode(ساختار bmp_decdata *bmp, unsigned نویسه *data, صحیح data_size);
تهی bmp_get_info(ساختار bmp_decdata *bmp, صحیح *width, صحیح *height, صحیح *bpp);
صحیح bmp_show(ساختار bmp_decdata *bmp, unsigned نویسه *pic, صحیح width
             , صحیح height, صحیح depth, صحیح bytes_per_سطر_dest);

// boot.c
تهی boot_init(تهی);
تهی boot_add_bev(u16 seg, u16 bev, u16 desc, صحیح prio);
تهی boot_add_bcv(u16 seg, u16 ip, u16 desc, صحیح prio);
ساختار drive_s;
تهی boot_add_floppy(ساختار drive_s *drive_g, ثابت نویسه *desc, صحیح prio);
تهی boot_add_hd(ساختار drive_s *drive_g, ثابت نویسه *desc, صحیح prio);
تهی boot_add_cd(ساختار drive_s *drive_g, ثابت نویسه *desc, صحیح prio);
تهی boot_add_cbfs(تهی *data, ثابت نویسه *desc, صحیح prio);
تهی interactive_bootmenu(تهی);
تهی bcv_prepboot(تهی);
u8 is_bootprio_strict(تهی);
ساختار pci_device;
صحیح bootprio_find_pci_device(ساختار pci_device *pci);
صحیح bootprio_find_mmio_device(تهی *mmio);
صحیح bootprio_find_scsi_device(ساختار pci_device *pci, صحیح target, صحیح lun);
صحیح bootprio_find_scsi_mmio_device(تهی *mmio, صحیح target, صحیح lun);
صحیح bootprio_find_ata_device(ساختار pci_device *pci, صحیح chanid, صحیح slave);
صحیح bootprio_find_fdc_device(ساختار pci_device *pci, صحیح port, صحیح fdid);
صحیح bootprio_find_pci_rom(ساختار pci_device *pci, صحیح instance);
صحیح bootprio_find_named_rom(ثابت نویسه *name, صحیح instance);
ساختار usbdevice_s;
صحیح bootprio_find_usb(ساختار usbdevice_s *usbdev, صحیح lun);
صحیح get_keystroke_full(صحیح msec);
صحیح get_keystroke(صحیح msec);
ساختار chs_s;
صحیح boot_lchs_find_pci_device(ساختار pci_device *pci, ساختار chs_s *chs);
صحیح boot_lchs_find_scsi_device(ساختار pci_device *pci, صحیح target, صحیح lun,
                               ساختار chs_s *chs);
صحیح boot_lchs_find_ata_device(ساختار pci_device *pci, صحیح chanid, صحیح slave,
                              ساختار chs_s *chs);

// bootsplash.c
تهی enable_vga_console(تهی);
تهی enable_bootsplash(تهی);
تهی disable_bootsplash(تهی);

// cdrom.c
بیرونی ساختار eltorito_s CDEmu;
بیرونی ساختار drive_s *cdemu_drive_gf;
ساختار disk_op_s;
صحیح cdemu_process_op(ساختار disk_op_s *op);
تهی cdrom_prepboot(تهی);
صحیح cdrom_boot(ساختار drive_s *drive_g);
نویسه *cdrom_media_info(ساختار drive_s *drive_g);

// clock.c
تهی clock_setup(تهی);
تهی handle_1583(ساختار bregs *regs);
تهی clock_poll_irq(تهی);
u32 irqtimer_calc_ticks(u32 شمار);
u32 irqtimer_calc(u32 msecs);
صحیح irqtimer_check(u32 end);
تهی handle_1586(ساختار bregs *regs);

// fw/acpi.c
تهی acpi_setup(تهی);

// fw/biostable.c
تهی copy_pir(تهی *pos);
تهی copy_mptable(تهی *pos);
بیرونی ساختار pir_header *PirAddr;
تهی copy_acpi_rsdp(تهی *pos);
بیرونی ساختار rsdp_descriptor *RsdpAddr;
بیرونی u32 acpi_pm1a_cnt;
بیرونی u16 acpi_pm_base;
تهی *find_acpi_rsdp(تهی);
تهی *find_acpi_table(u32 signature);
u32 find_resume_vector(تهی);
تهی acpi_reboot(تهی);
تهی find_acpi_features(تهی);
تهی *smbios_get_tables(u32 *length);
تهی copy_smbios_21(تهی *pos);
تهی display_uuid(تهی);
تهی copy_table(تهی *pos);
تهی smbios_setup(تهی);

// fw/dsdt_parser.c
ساختار acpi_device;
تهی acpi_dsdt_parse(تهی);
ساختار acpi_device *acpi_dsdt_find_string(ساختار acpi_device *prev, ثابت نویسه *hid);
ساختار acpi_device *acpi_dsdt_find_eisaid(ساختار acpi_device *prev, u16 eisaid);
نویسه *acpi_dsdt_name(ساختار acpi_device *dev);
صحیح acpi_dsdt_present_eisaid(u16 eisaid);
صحیح acpi_dsdt_find_io(ساختار acpi_device *dev, u64 *min, u64 *max);
صحیح acpi_dsdt_find_mem(ساختار acpi_device *dev, u64 *min, u64 *max);
صحیح acpi_dsdt_find_irq(ساختار acpi_device *dev, u64 *irq);

// fw/coreboot.c
بیرونی ثابت نویسه *CBvendor, *CBpart;
ساختار cbfs_file;
تهی coreboot_debug_putc(نویسه c);
تهی cbfs_run_payload(ساختار cbfs_file *file);
تهی coreboot_platform_setup(تهی);
تهی cbfs_payload_setup(تهی);
تهی coreboot_preinit(تهی);
تهی coreboot_cbfs_init(تهی);
ساختار cb_header;
تهی *find_cb_subtable(ساختار cb_header *cbh, u32 tag);
ساختار cb_header *find_cb_table(تهی);

// fw/csm.c
صحیح csm_bootprio_fdc(ساختار pci_device *pci, صحیح port, صحیح fdid);
صحیح csm_bootprio_ata(ساختار pci_device *pci, صحیح chanid, صحیح slave);
صحیح csm_bootprio_pci(ساختار pci_device *pci);

// fw/mptable.c
تهی mptable_setup(تهی);

// fw/mtrr.c
تهی mtrr_setup(تهی);

// fw/multiboot.c
تهی multiboot_init(تهی);

// fw/pciinit.c
بیرونی u64 pcimem_start, pcimem_end;
بیرونی u64 pcimem64_start, pcimem64_end;
بیرونی ثابت u8 pci_irqs[4];
تهی pci_setup(تهی);
تهی pci_resume(تهی);

// fw/pirtable.c
تهی pirtable_setup(تهی);

// fw/shadow.c
تهی make_bios_writable(تهی);
تهی make_bios_readonly(تهی);
تهی qemu_reboot(تهی);

// fw/smbios.c
تهی smbios_legacy_setup(تهی);

// fw/smm.c
تهی smm_device_setup(تهی);
تهی smm_setup(تهی);

// fw/smp.c
بیرونی u32 Max_شمار_CPUs;
تهی wrmsr_smp(u32 index, u64 val);
تهی smp_setup(تهی);
تهی smp_resume(تهی);
صحیح apic_id_is_present(u8 apic_id);

// hw/dma.c
صحیح dma_floppy(u32 addr, صحیح شمار, صحیح isWrite);
تهی dma_setup(تهی);

// hw/floppy.c
بیرونی ساختار floppy_ext_dbt_s diskette_param_table2;
تهی floppy_setup(تهی);
ساختار drive_s *init_floppy(صحیح floppyid, صحیح ftype);
صحیح find_floppy_type(u32 size);
صحیح floppy_process_op(ساختار disk_op_s *op);
تهی floppy_tick(تهی);

// hw/ramdisk.c
تهی ramdisk_setup(تهی);
صحیح ramdisk_process_op(ساختار disk_op_s *op);

// hw/sdcard.c
صحیح sdcard_process_op(ساختار disk_op_s *op);
تهی sdcard_setup(تهی);

// hw/timer.c
تهی timer_setup(تهی);
تهی pmtimer_setup(u16 ioport);
تهی tsctimer_setfreq(u32 khz, ثابت نویسه *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
صحیح timer_check(u32 end);
تهی ndelay(u32 شمار);
تهی udelay(u32 شمار);
تهی mdelay(u32 شمار);
تهی nsleep(u32 شمار);
تهی usleep(u32 شمار);
تهی msleep(u32 شمار);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
تهی pit_setup(تهی);

// jpeg.c
ساختار jpeg_decdata *jpeg_alloc(تهی);
صحیح jpeg_decode(ساختار jpeg_decdata *jpeg, unsigned نویسه *buf);
تهی jpeg_get_size(ساختار jpeg_decdata *jpeg, صحیح *width, صحیح *height);
صحیح jpeg_show(ساختار jpeg_decdata *jpeg, unsigned نویسه *pic, صحیح width
              , صحیح height, صحیح depth, صحیح bytes_per_سطر_dest);

// kbd.c
تهی kbd_init(تهی);
تهی handle_15c2(ساختار bregs *regs);
تهی process_کلید(u8 کلید);
u8 enqueue_کلید(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
بیرونی صحیح HaveRunPost;
بیرونی ساختار bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
بیرونی ساختار floppy_dbt_s diskette_param_table __aligned(1);
بیرونی u8 BiosChecksum;
صحیح in_post(تهی);
تهی mathcp_setup(تهی);

// mouse.c
تهی mouse_init(تهی);
تهی process_mouse(u8 data);

// optionroms.c
ساختار rom_header;
تهی callrom(ساختار rom_header *rom, u16 bdf);
تهی call_bcv(u16 seg, u16 ip);
صحیح is_pci_vga(ساختار pci_device *pci);
تهی optionrom_setup(تهی);
تهی vgarom_setup(تهی);
تهی s3_resume_vga(تهی);
بیرونی صحیح ScreenAndDebug;

// pcibios.c
تهی handle_1ab1(ساختار bregs *regs);
تهی bios32_init(تهی);

// pmm.c
تهی pmm_init(تهی);
تهی pmm_prepboot(تهی);

// pnpbios.c
u16 get_pnp_offset(تهی);
تهی pnp_init(تهی);

// post.c
تهی interface_init(تهی);
تهی device_hardware_setup(تهی);
تهی prepareboot(تهی);
تهی startBoot(تهی);
تهی reloc_preinit(تهی *f, تهی *arg);
تهی code_mutable_preinit(تهی);

// sercon.c
تهی sercon_setup(تهی);
تهی sercon_check_event(تهی);

// serial.c
تهی serial_setup(تهی);
تهی lpt_setup(تهی);

// version.c
بیرونی ثابت نویسه VERSION[], BUILDINFO[];

// vgahooks.c
تهی handle_155f(ساختار bregs *regs);
تهی handle_157f(ساختار bregs *regs);
تهی vgahook_setup(ساختار pci_device *pci);

#endif // util.h
