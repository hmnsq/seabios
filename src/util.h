// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

// apm.c
ଶୂନ୍ୟ apm_shutdown(ଶୂନ୍ୟ);
ସଂରଚନା bregs;
ଶୂନ୍ୟ handle_1553(ସଂରଚନା bregs *regs);

// bmp.c
ସଂରଚନା bmp_decdata *bmp_alloc(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bmp_decode(ସଂରଚନା bmp_decdata *bmp, unsigned ଅକ୍ଷର *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା data_size);
ଶୂନ୍ୟ bmp_get_info(ସଂରଚନା bmp_decdata *bmp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *bpp);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bmp_show(ସଂରଚନା bmp_decdata *bmp, unsigned ଅକ୍ଷର *pic, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width
             , ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes_per_ଧାଡ଼ି_dest);

// boot.c
ଶୂନ୍ୟ boot_init(ଶୂନ୍ୟ);
ଶୂନ୍ୟ boot_add_bev(u16 seg, u16 bev, u16 desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio);
ଶୂନ୍ୟ boot_add_bcv(u16 seg, u16 ip, u16 desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio);
ସଂରଚନା drive_s;
ଶୂନ୍ୟ boot_add_floppy(ସଂରଚନା drive_s *drive_g, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio);
ଶୂନ୍ୟ boot_add_hd(ସଂରଚନା drive_s *drive_g, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio);
ଶୂନ୍ୟ boot_add_cd(ସଂରଚନା drive_s *drive_g, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio);
ଶୂନ୍ୟ boot_add_cbfs(ଶୂନ୍ୟ *data, ଅଚଳ ଅକ୍ଷର *desc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio);
ଶୂନ୍ୟ interactive_bootmenu(ଶୂନ୍ୟ);
ଶୂନ୍ୟ bcv_prepboot(ଶୂନ୍ୟ);
u8 is_bootprio_strict(ଶୂନ୍ୟ);
ସଂରଚନା pci_device;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_pci_device(ସଂରଚନା pci_device *pci);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_mmio_device(ଶୂନ୍ୟ *mmio);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_scsi_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_scsi_mmio_device(ଶୂନ୍ୟ *mmio, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_ata_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chanid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_fdc_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fdid);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_pci_rom(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_named_rom(ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance);
ସଂରଚନା usbdevice_s;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootprio_find_usb(ସଂରଚନା usbdevice_s *usbdev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_keystroke_full(ପୂର୍ଣ୍ଣସଂଖ୍ୟା msec);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_keystroke(ପୂର୍ଣ୍ଣସଂଖ୍ୟା msec);
ସଂରଚନା chs_s;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା boot_lchs_find_pci_device(ସଂରଚନା pci_device *pci, ସଂରଚନା chs_s *chs);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା boot_lchs_find_scsi_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun,
                               ସଂରଚନା chs_s *chs);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା boot_lchs_find_ata_device(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chanid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave,
                              ସଂରଚନା chs_s *chs);

// bootsplash.c
ଶୂନ୍ୟ enable_vga_console(ଶୂନ୍ୟ);
ଶୂନ୍ୟ enable_bootsplash(ଶୂନ୍ୟ);
ଶୂନ୍ୟ disable_bootsplash(ଶୂନ୍ୟ);

// cdrom.c
ବାହ୍ୟ ସଂରଚନା eltorito_s CDEmu;
ବାହ୍ୟ ସଂରଚନା drive_s *cdemu_drive_gf;
ସଂରଚନା disk_op_s;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cdemu_process_op(ସଂରଚନା disk_op_s *op);
ଶୂନ୍ୟ cdrom_prepboot(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cdrom_boot(ସଂରଚନା drive_s *drive_g);
ଅକ୍ଷର *cdrom_media_info(ସଂରଚନା drive_s *drive_g);

// clock.c
ଶୂନ୍ୟ clock_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ handle_1583(ସଂରଚନା bregs *regs);
ଶୂନ୍ୟ clock_poll_irq(ଶୂନ୍ୟ);
u32 irqtimer_calc_ticks(u32 ଗଣନା_2);
u32 irqtimer_calc(u32 msecs);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା irqtimer_check(u32 end);
ଶୂନ୍ୟ handle_1586(ସଂରଚନା bregs *regs);

// fw/acpi.c
ଶୂନ୍ୟ acpi_setup(ଶୂନ୍ୟ);

// fw/biostable.c
ଶୂନ୍ୟ copy_pir(ଶୂନ୍ୟ *pos);
ଶୂନ୍ୟ copy_mptable(ଶୂନ୍ୟ *pos);
ବାହ୍ୟ ସଂରଚନା pir_header *PirAddr;
ଶୂନ୍ୟ copy_acpi_rsdp(ଶୂନ୍ୟ *pos);
ବାହ୍ୟ ସଂରଚନା rsdp_descriptor *RsdpAddr;
ବାହ୍ୟ u32 acpi_pm1a_cnt;
ବାହ୍ୟ u16 acpi_pm_base;
ଶୂନ୍ୟ *find_acpi_rsdp(ଶୂନ୍ୟ);
ଶୂନ୍ୟ *find_acpi_table(u32 signature);
u32 find_resume_vector(ଶୂନ୍ୟ);
ଶୂନ୍ୟ acpi_reboot(ଶୂନ୍ୟ);
ଶୂନ୍ୟ find_acpi_features(ଶୂନ୍ୟ);
ଶୂନ୍ୟ *smbios_get_tables(u32 *length);
ଶୂନ୍ୟ copy_smbios_21(ଶୂନ୍ୟ *pos);
ଶୂନ୍ୟ display_uuid(ଶୂନ୍ୟ);
ଶୂନ୍ୟ copy_table(ଶୂନ୍ୟ *pos);
ଶୂନ୍ୟ smbios_setup(ଶୂନ୍ୟ);

// fw/dsdt_parser.c
ସଂରଚନା acpi_device;
ଶୂନ୍ୟ acpi_dsdt_parse(ଶୂନ୍ୟ);
ସଂରଚନା acpi_device *acpi_dsdt_find_string(ସଂରଚନା acpi_device *prev, ଅଚଳ ଅକ୍ଷର *hid);
ସଂରଚନା acpi_device *acpi_dsdt_find_eisaid(ସଂରଚନା acpi_device *prev, u16 eisaid);
ଅକ୍ଷର *acpi_dsdt_name(ସଂରଚନା acpi_device *dev);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_present_eisaid(u16 eisaid);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_find_io(ସଂରଚନା acpi_device *dev, u64 *min, u64 *max);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_find_mem(ସଂରଚନା acpi_device *dev, u64 *min, u64 *max);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_find_irq(ସଂରଚନା acpi_device *dev, u64 *irq);

// fw/coreboot.c
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *CBvendor, *CBpart;
ସଂରଚନା cbfs_file;
ଶୂନ୍ୟ coreboot_debug_putc(ଅକ୍ଷର c);
ଶୂନ୍ୟ cbfs_run_payload(ସଂରଚନା cbfs_file *file);
ଶୂନ୍ୟ coreboot_platform_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ cbfs_payload_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ coreboot_preinit(ଶୂନ୍ୟ);
ଶୂନ୍ୟ coreboot_cbfs_init(ଶୂନ୍ୟ);
ସଂରଚନା cb_header;
ଶୂନ୍ୟ *find_cb_subtable(ସଂରଚନା cb_header *cbh, u32 tag);
ସଂରଚନା cb_header *find_cb_table(ଶୂନ୍ୟ);

// fw/csm.c
ପୂର୍ଣ୍ଣସଂଖ୍ୟା csm_bootprio_fdc(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fdid);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା csm_bootprio_ata(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chanid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା csm_bootprio_pci(ସଂରଚନା pci_device *pci);

// fw/mptable.c
ଶୂନ୍ୟ mptable_setup(ଶୂନ୍ୟ);

// fw/mtrr.c
ଶୂନ୍ୟ mtrr_setup(ଶୂନ୍ୟ);

// fw/multiboot.c
ଶୂନ୍ୟ multiboot_init(ଶୂନ୍ୟ);

// fw/pciinit.c
ବାହ୍ୟ u64 pcimem_start, pcimem_end;
ବାହ୍ୟ u64 pcimem64_start, pcimem64_end;
ବାହ୍ୟ ଅଚଳ u8 pci_irqs[4];
ଶୂନ୍ୟ pci_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ pci_resume(ଶୂନ୍ୟ);

// fw/pirtable.c
ଶୂନ୍ୟ pirtable_setup(ଶୂନ୍ୟ);

// fw/shadow.c
ଶୂନ୍ୟ make_bios_writable(ଶୂନ୍ୟ);
ଶୂନ୍ୟ make_bios_readonly(ଶୂନ୍ୟ);
ଶୂନ୍ୟ qemu_reboot(ଶୂନ୍ୟ);

// fw/smbios.c
ଶୂନ୍ୟ smbios_legacy_setup(ଶୂନ୍ୟ);

// fw/smm.c
ଶୂନ୍ୟ smm_device_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ smm_setup(ଶୂନ୍ୟ);

// fw/smp.c
ବାହ୍ୟ u32 Max_ଗଣନା_CPUs;
ଶୂନ୍ୟ wrmsr_smp(u32 index, u64 val);
ଶୂନ୍ୟ smp_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ smp_resume(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା apic_id_is_present(u8 apic_id);

// hw/dma.c
ପୂର୍ଣ୍ଣସଂଖ୍ୟା dma_floppy(u32 addr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isWrite);
ଶୂନ୍ୟ dma_setup(ଶୂନ୍ୟ);

// hw/floppy.c
ବାହ୍ୟ ସଂରଚନା floppy_ext_dbt_s diskette_param_table2;
ଶୂନ୍ୟ floppy_setup(ଶୂନ୍ୟ);
ସଂରଚନା drive_s *init_floppy(ପୂର୍ଣ୍ଣସଂଖ୍ୟା floppyid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ftype);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା find_floppy_type(u32 size);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା floppy_process_op(ସଂରଚନା disk_op_s *op);
ଶୂନ୍ୟ floppy_tick(ଶୂନ୍ୟ);

// hw/ramdisk.c
ଶୂନ୍ୟ ramdisk_setup(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ramdisk_process_op(ସଂରଚନା disk_op_s *op);

// hw/sdcard.c
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sdcard_process_op(ସଂରଚନା disk_op_s *op);
ଶୂନ୍ୟ sdcard_setup(ଶୂନ୍ୟ);

// hw/timer.c
ଶୂନ୍ୟ timer_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ pmtimer_setup(u16 ioport);
ଶୂନ୍ୟ tsctimer_setfreq(u32 khz, ଅଚଳ ଅକ୍ଷର *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା timer_check(u32 end);
ଶୂନ୍ୟ ndelay(u32 ଗଣନା_2);
ଶୂନ୍ୟ udelay(u32 ଗଣନା_2);
ଶୂନ୍ୟ mdelay(u32 ଗଣନା_2);
ଶୂନ୍ୟ nsleep(u32 ଗଣନା_2);
ଶୂନ୍ୟ usleep(u32 ଗଣନା_2);
ଶୂନ୍ୟ msleep(u32 ଗଣନା_2);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
ଶୂନ୍ୟ pit_setup(ଶୂନ୍ୟ);

// jpeg.c
ସଂରଚନା jpeg_decdata *jpeg_alloc(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା jpeg_decode(ସଂରଚନା jpeg_decdata *jpeg, unsigned ଅକ୍ଷର *buf);
ଶୂନ୍ୟ jpeg_get_size(ସଂରଚନା jpeg_decdata *jpeg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା jpeg_show(ସଂରଚନା jpeg_decdata *jpeg, unsigned ଅକ୍ଷର *pic, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width
              , ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes_per_ଧାଡ଼ି_dest);

// kbd.c
ଶୂନ୍ୟ kbd_init(ଶୂନ୍ୟ);
ଶୂନ୍ୟ handle_15c2(ସଂରଚନା bregs *regs);
ଶୂନ୍ୟ process_ଚାବି(u8 ଚାବି);
u8 enqueue_ଚାବି(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା HaveRunPost;
ବାହ୍ୟ ସଂରଚନା bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
ବାହ୍ୟ ସଂରଚନା floppy_dbt_s diskette_param_table __aligned(1);
ବାହ୍ୟ u8 BiosChecksum;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା in_post(ଶୂନ୍ୟ);
ଶୂନ୍ୟ mathcp_setup(ଶୂନ୍ୟ);

// mouse.c
ଶୂନ୍ୟ mouse_init(ଶୂନ୍ୟ);
ଶୂନ୍ୟ process_mouse(u8 data);

// optionroms.c
ସଂରଚନା rom_header;
ଶୂନ୍ୟ callrom(ସଂରଚନା rom_header *rom, u16 bdf);
ଶୂନ୍ୟ call_bcv(u16 seg, u16 ip);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_pci_vga(ସଂରଚନା pci_device *pci);
ଶୂନ୍ୟ optionrom_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ vgarom_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ s3_resume_vga(ଶୂନ୍ୟ);
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ScreenAndDebug;

// pcibios.c
ଶୂନ୍ୟ handle_1ab1(ସଂରଚନା bregs *regs);
ଶୂନ୍ୟ bios32_init(ଶୂନ୍ୟ);

// pmm.c
ଶୂନ୍ୟ pmm_init(ଶୂନ୍ୟ);
ଶୂନ୍ୟ pmm_prepboot(ଶୂନ୍ୟ);

// pnpbios.c
u16 get_pnp_offset(ଶୂନ୍ୟ);
ଶୂନ୍ୟ pnp_init(ଶୂନ୍ୟ);

// post.c
ଶୂନ୍ୟ interface_init(ଶୂନ୍ୟ);
ଶୂନ୍ୟ device_hardware_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ prepareboot(ଶୂନ୍ୟ);
ଶୂନ୍ୟ startBoot(ଶୂନ୍ୟ);
ଶୂନ୍ୟ reloc_preinit(ଶୂନ୍ୟ *f, ଶୂନ୍ୟ *arg);
ଶୂନ୍ୟ code_mutable_preinit(ଶୂନ୍ୟ);

// sercon.c
ଶୂନ୍ୟ sercon_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ sercon_check_event(ଶୂନ୍ୟ);

// serial.c
ଶୂନ୍ୟ serial_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ lpt_setup(ଶୂନ୍ୟ);

// version.c
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର VERSION[], BUILDINFO[];

// vgahooks.c
ଶୂନ୍ୟ handle_155f(ସଂରଚନା bregs *regs);
ଶୂନ୍ୟ handle_157f(ସଂରଚନା bregs *regs);
ଶୂନ୍ୟ vgahook_setup(ସଂରଚନା pci_device *pci);

#endif // util.h
