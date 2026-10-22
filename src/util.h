// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// apm.c
ಶೂನ್ಯ apm_shutdown(ಶೂನ್ಯ);
ಸಂರಚನಾ bregs;
ಶೂನ್ಯ handle_1553(ಸಂರಚನಾ bregs *regs);

// bmp.c
ಸಂರಚನಾ bmp_decdata *bmp_alloc(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ bmp_decode(ಸಂರಚನಾ bmp_decdata *bmp, unsigned ವರ್ಣ *data, ಪೂರ್ಣಾಙ್ಕ data_size);
ಶೂನ್ಯ bmp_get_info(ಸಂರಚನಾ bmp_decdata *bmp, ಪೂರ್ಣಾಙ್ಕ *width, ಪೂರ್ಣಾಙ್ಕ *height, ಪೂರ್ಣಾಙ್ಕ *bpp);
ಪೂರ್ಣಾಙ್ಕ bmp_show(ಸಂರಚನಾ bmp_decdata *bmp, unsigned ವರ್ಣ *pic, ಪೂರ್ಣಾಙ್ಕ width
             , ಪೂರ್ಣಾಙ್ಕ height, ಪೂರ್ಣಾಙ್ಕ depth, ಪೂರ್ಣಾಙ್ಕ bytes_per_ಸಾಲು_dest);

// boot.c
ಶೂನ್ಯ boot_init(ಶೂನ್ಯ);
ಶೂನ್ಯ boot_add_bev(u16 seg, u16 bev, u16 desc, ಪೂರ್ಣಾಙ್ಕ prio);
ಶೂನ್ಯ boot_add_bcv(u16 seg, u16 ip, u16 desc, ಪೂರ್ಣಾಙ್ಕ prio);
ಸಂರಚನಾ drive_s;
ಶೂನ್ಯ boot_add_floppy(ಸಂರಚನಾ drive_s *drive_g, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio);
ಶೂನ್ಯ boot_add_hd(ಸಂರಚನಾ drive_s *drive_g, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio);
ಶೂನ್ಯ boot_add_cd(ಸಂರಚನಾ drive_s *drive_g, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio);
ಶೂನ್ಯ boot_add_cbfs(ಶೂನ್ಯ *data, ಧ್ರುವ ವರ್ಣ *desc, ಪೂರ್ಣಾಙ್ಕ prio);
ಶೂನ್ಯ interactive_bootmenu(ಶೂನ್ಯ);
ಶೂನ್ಯ bcv_prepboot(ಶೂನ್ಯ);
u8 is_bootprio_strict(ಶೂನ್ಯ);
ಸಂರಚನಾ pci_device;
ಪೂರ್ಣಾಙ್ಕ bootprio_find_pci_device(ಸಂರಚನಾ pci_device *pci);
ಪೂರ್ಣಾಙ್ಕ bootprio_find_mmio_device(ಶೂನ್ಯ *mmio);
ಪೂರ್ಣಾಙ್ಕ bootprio_find_scsi_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ target, ಪೂರ್ಣಾಙ್ಕ lun);
ಪೂರ್ಣಾಙ್ಕ bootprio_find_scsi_mmio_device(ಶೂನ್ಯ *mmio, ಪೂರ್ಣಾಙ್ಕ target, ಪೂರ್ಣಾಙ್ಕ lun);
ಪೂರ್ಣಾಙ್ಕ bootprio_find_ata_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ slave);
ಪೂರ್ಣಾಙ್ಕ bootprio_find_fdc_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ port, ಪೂರ್ಣಾಙ್ಕ fdid);
ಪೂರ್ಣಾಙ್ಕ bootprio_find_pci_rom(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ instance);
ಪೂರ್ಣಾಙ್ಕ bootprio_find_named_rom(ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ instance);
ಸಂರಚನಾ usbdevice_s;
ಪೂರ್ಣಾಙ್ಕ bootprio_find_usb(ಸಂರಚನಾ usbdevice_s *usbdev, ಪೂರ್ಣಾಙ್ಕ lun);
ಪೂರ್ಣಾಙ್ಕ get_keystroke_full(ಪೂರ್ಣಾಙ್ಕ msec);
ಪೂರ್ಣಾಙ್ಕ get_keystroke(ಪೂರ್ಣಾಙ್ಕ msec);
ಸಂರಚನಾ chs_s;
ಪೂರ್ಣಾಙ್ಕ boot_lchs_find_pci_device(ಸಂರಚನಾ pci_device *pci, ಸಂರಚನಾ chs_s *chs);
ಪೂರ್ಣಾಙ್ಕ boot_lchs_find_scsi_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ target, ಪೂರ್ಣಾಙ್ಕ lun,
                               ಸಂರಚನಾ chs_s *chs);
ಪೂರ್ಣಾಙ್ಕ boot_lchs_find_ata_device(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ slave,
                              ಸಂರಚನಾ chs_s *chs);

// bootsplash.c
ಶೂನ್ಯ enable_vga_console(ಶೂನ್ಯ);
ಶೂನ್ಯ enable_bootsplash(ಶೂನ್ಯ);
ಶೂನ್ಯ disable_bootsplash(ಶೂನ್ಯ);

// cdrom.c
ಬಾಹ್ಯ ಸಂರಚನಾ eltorito_s CDEmu;
ಬಾಹ್ಯ ಸಂರಚನಾ drive_s *cdemu_drive_gf;
ಸಂರಚನಾ disk_op_s;
ಪೂರ್ಣಾಙ್ಕ cdemu_process_op(ಸಂರಚನಾ disk_op_s *op);
ಶೂನ್ಯ cdrom_prepboot(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ cdrom_boot(ಸಂರಚನಾ drive_s *drive_g);
ವರ್ಣ *cdrom_media_info(ಸಂರಚನಾ drive_s *drive_g);

// clock.c
ಶೂನ್ಯ clock_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ handle_1583(ಸಂರಚನಾ bregs *regs);
ಶೂನ್ಯ clock_poll_irq(ಶೂನ್ಯ);
u32 irqtimer_calc_ticks(u32 ಎಣಿಕೆ);
u32 irqtimer_calc(u32 msecs);
ಪೂರ್ಣಾಙ್ಕ irqtimer_check(u32 end);
ಶೂನ್ಯ handle_1586(ಸಂರಚನಾ bregs *regs);

// fw/acpi.c
ಶೂನ್ಯ acpi_setup(ಶೂನ್ಯ);

// fw/biostable.c
ಶೂನ್ಯ copy_pir(ಶೂನ್ಯ *pos);
ಶೂನ್ಯ copy_mptable(ಶೂನ್ಯ *pos);
ಬಾಹ್ಯ ಸಂರಚನಾ pir_header *PirAddr;
ಶೂನ್ಯ copy_acpi_rsdp(ಶೂನ್ಯ *pos);
ಬಾಹ್ಯ ಸಂರಚನಾ rsdp_descriptor *RsdpAddr;
ಬಾಹ್ಯ u32 acpi_pm1a_cnt;
ಬಾಹ್ಯ u16 acpi_pm_base;
ಶೂನ್ಯ *find_acpi_rsdp(ಶೂನ್ಯ);
ಶೂನ್ಯ *find_acpi_table(u32 signature);
u32 find_resume_vector(ಶೂನ್ಯ);
ಶೂನ್ಯ acpi_reboot(ಶೂನ್ಯ);
ಶೂನ್ಯ find_acpi_features(ಶೂನ್ಯ);
ಶೂನ್ಯ *smbios_get_tables(u32 *length);
ಶೂನ್ಯ copy_smbios_21(ಶೂನ್ಯ *pos);
ಶೂನ್ಯ display_uuid(ಶೂನ್ಯ);
ಶೂನ್ಯ copy_table(ಶೂನ್ಯ *pos);
ಶೂನ್ಯ smbios_setup(ಶೂನ್ಯ);

// fw/dsdt_parser.c
ಸಂರಚನಾ acpi_device;
ಶೂನ್ಯ acpi_dsdt_parse(ಶೂನ್ಯ);
ಸಂರಚನಾ acpi_device *acpi_dsdt_find_string(ಸಂರಚನಾ acpi_device *prev, ಧ್ರುವ ವರ್ಣ *hid);
ಸಂರಚನಾ acpi_device *acpi_dsdt_find_eisaid(ಸಂರಚನಾ acpi_device *prev, u16 eisaid);
ವರ್ಣ *acpi_dsdt_name(ಸಂರಚನಾ acpi_device *dev);
ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_present_eisaid(u16 eisaid);
ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_find_io(ಸಂರಚನಾ acpi_device *dev, u64 *min, u64 *max);
ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_find_mem(ಸಂರಚನಾ acpi_device *dev, u64 *min, u64 *max);
ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_find_irq(ಸಂರಚನಾ acpi_device *dev, u64 *irq);

// fw/coreboot.c
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *CBvendor, *CBpart;
ಸಂರಚನಾ cbfs_file;
ಶೂನ್ಯ coreboot_debug_putc(ವರ್ಣ c);
ಶೂನ್ಯ cbfs_run_payload(ಸಂರಚನಾ cbfs_file *file);
ಶೂನ್ಯ coreboot_platform_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ cbfs_payload_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ coreboot_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ coreboot_cbfs_init(ಶೂನ್ಯ);
ಸಂರಚನಾ cb_header;
ಶೂನ್ಯ *find_cb_subtable(ಸಂರಚನಾ cb_header *cbh, u32 tag);
ಸಂರಚನಾ cb_header *find_cb_table(ಶೂನ್ಯ);

// fw/csm.c
ಪೂರ್ಣಾಙ್ಕ csm_bootprio_fdc(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ port, ಪೂರ್ಣಾಙ್ಕ fdid);
ಪೂರ್ಣಾಙ್ಕ csm_bootprio_ata(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ slave);
ಪೂರ್ಣಾಙ್ಕ csm_bootprio_pci(ಸಂರಚನಾ pci_device *pci);

// fw/mptable.c
ಶೂನ್ಯ mptable_setup(ಶೂನ್ಯ);

// fw/mtrr.c
ಶೂನ್ಯ mtrr_setup(ಶೂನ್ಯ);

// fw/multiboot.c
ಶೂನ್ಯ multiboot_init(ಶೂನ್ಯ);

// fw/pciinit.c
ಬಾಹ್ಯ u64 pcimem_start, pcimem_end;
ಬಾಹ್ಯ u64 pcimem64_start, pcimem64_end;
ಬಾಹ್ಯ ಧ್ರುವ u8 pci_irqs[4];
ಶೂನ್ಯ pci_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ pci_resume(ಶೂನ್ಯ);

// fw/pirtable.c
ಶೂನ್ಯ pirtable_setup(ಶೂನ್ಯ);

// fw/shadow.c
ಶೂನ್ಯ make_bios_writable(ಶೂನ್ಯ);
ಶೂನ್ಯ make_bios_readonly(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_reboot(ಶೂನ್ಯ);

// fw/smbios.c
ಶೂನ್ಯ smbios_legacy_setup(ಶೂನ್ಯ);

// fw/smm.c
ಶೂನ್ಯ smm_device_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ smm_setup(ಶೂನ್ಯ);

// fw/smp.c
ಬಾಹ್ಯ u32 Max_ಎಣಿಕೆ_CPUs;
ಶೂನ್ಯ wrmsr_smp(u32 index, u64 val);
ಶೂನ್ಯ smp_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ smp_resume(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ apic_id_is_present(u8 apic_id);

// hw/dma.c
ಪೂರ್ಣಾಙ್ಕ dma_floppy(u32 addr, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ, ಪೂರ್ಣಾಙ್ಕ isWrite);
ಶೂನ್ಯ dma_setup(ಶೂನ್ಯ);

// hw/floppy.c
ಬಾಹ್ಯ ಸಂರಚನಾ floppy_ext_dbt_s diskette_param_table2;
ಶೂನ್ಯ floppy_setup(ಶೂನ್ಯ);
ಸಂರಚನಾ drive_s *init_floppy(ಪೂರ್ಣಾಙ್ಕ floppyid, ಪೂರ್ಣಾಙ್ಕ ftype);
ಪೂರ್ಣಾಙ್ಕ find_floppy_type(u32 size);
ಪೂರ್ಣಾಙ್ಕ floppy_process_op(ಸಂರಚನಾ disk_op_s *op);
ಶೂನ್ಯ floppy_tick(ಶೂನ್ಯ);

// hw/ramdisk.c
ಶೂನ್ಯ ramdisk_setup(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ ramdisk_process_op(ಸಂರಚನಾ disk_op_s *op);

// hw/sdcard.c
ಪೂರ್ಣಾಙ್ಕ sdcard_process_op(ಸಂರಚನಾ disk_op_s *op);
ಶೂನ್ಯ sdcard_setup(ಶೂನ್ಯ);

// hw/timer.c
ಶೂನ್ಯ timer_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ pmtimer_setup(u16 ioport);
ಶೂನ್ಯ tsctimer_setfreq(u32 khz, ಧ್ರುವ ವರ್ಣ *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
ಪೂರ್ಣಾಙ್ಕ timer_check(u32 end);
ಶೂನ್ಯ ndelay(u32 ಎಣಿಕೆ);
ಶೂನ್ಯ udelay(u32 ಎಣಿಕೆ);
ಶೂನ್ಯ mdelay(u32 ಎಣಿಕೆ);
ಶೂನ್ಯ nsleep(u32 ಎಣಿಕೆ);
ಶೂನ್ಯ usleep(u32 ಎಣಿಕೆ);
ಶೂನ್ಯ msleep(u32 ಎಣಿಕೆ);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
ಶೂನ್ಯ pit_setup(ಶೂನ್ಯ);

// jpeg.c
ಸಂರಚನಾ jpeg_decdata *jpeg_alloc(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ jpeg_decode(ಸಂರಚನಾ jpeg_decdata *jpeg, unsigned ವರ್ಣ *buf);
ಶೂನ್ಯ jpeg_get_size(ಸಂರಚನಾ jpeg_decdata *jpeg, ಪೂರ್ಣಾಙ್ಕ *width, ಪೂರ್ಣಾಙ್ಕ *height);
ಪೂರ್ಣಾಙ್ಕ jpeg_show(ಸಂರಚನಾ jpeg_decdata *jpeg, unsigned ವರ್ಣ *pic, ಪೂರ್ಣಾಙ್ಕ width
              , ಪೂರ್ಣಾಙ್ಕ height, ಪೂರ್ಣಾಙ್ಕ depth, ಪೂರ್ಣಾಙ್ಕ bytes_per_ಸಾಲು_dest);

// kbd.c
ಶೂನ್ಯ kbd_init(ಶೂನ್ಯ);
ಶೂನ್ಯ handle_15c2(ಸಂರಚನಾ bregs *regs);
ಶೂನ್ಯ process_ಕೀಲಿ(u8 ಕೀಲಿ);
u8 enqueue_ಕೀಲಿ(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ HaveRunPost;
ಬಾಹ್ಯ ಸಂರಚನಾ bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
ಬಾಹ್ಯ ಸಂರಚನಾ floppy_dbt_s diskette_param_table __aligned(1);
ಬಾಹ್ಯ u8 BiosChecksum;
ಪೂರ್ಣಾಙ್ಕ in_post(ಶೂನ್ಯ);
ಶೂನ್ಯ mathcp_setup(ಶೂನ್ಯ);

// mouse.c
ಶೂನ್ಯ mouse_init(ಶೂನ್ಯ);
ಶೂನ್ಯ process_mouse(u8 data);

// optionroms.c
ಸಂರಚನಾ rom_header;
ಶೂನ್ಯ callrom(ಸಂರಚನಾ rom_header *rom, u16 bdf);
ಶೂನ್ಯ call_bcv(u16 seg, u16 ip);
ಪೂರ್ಣಾಙ್ಕ is_pci_vga(ಸಂರಚನಾ pci_device *pci);
ಶೂನ್ಯ optionrom_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ vgarom_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ s3_resume_vga(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ScreenAndDebug;

// pcibios.c
ಶೂನ್ಯ handle_1ab1(ಸಂರಚನಾ bregs *regs);
ಶೂನ್ಯ bios32_init(ಶೂನ್ಯ);

// pmm.c
ಶೂನ್ಯ pmm_init(ಶೂನ್ಯ);
ಶೂನ್ಯ pmm_prepboot(ಶೂನ್ಯ);

// pnpbios.c
u16 get_pnp_offset(ಶೂನ್ಯ);
ಶೂನ್ಯ pnp_init(ಶೂನ್ಯ);

// post.c
ಶೂನ್ಯ interface_init(ಶೂನ್ಯ);
ಶೂನ್ಯ device_hardware_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ prepareboot(ಶೂನ್ಯ);
ಶೂನ್ಯ startBoot(ಶೂನ್ಯ);
ಶೂನ್ಯ reloc_preinit(ಶೂನ್ಯ *f, ಶೂನ್ಯ *arg);
ಶೂನ್ಯ code_mutable_preinit(ಶೂನ್ಯ);

// sercon.c
ಶೂನ್ಯ sercon_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ sercon_check_event(ಶೂನ್ಯ);

// serial.c
ಶೂನ್ಯ serial_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ lpt_setup(ಶೂನ್ಯ);

// version.c
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ VERSION[], BUILDINFO[];

// vgahooks.c
ಶೂನ್ಯ handle_155f(ಸಂರಚನಾ bregs *regs);
ಶೂನ್ಯ handle_157f(ಸಂರಚನಾ bregs *regs);
ಶೂನ್ಯ vgahook_setup(ಸಂರಚನಾ pci_device *pci);

#endif // util.h
