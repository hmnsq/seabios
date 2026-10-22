// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#અન્તર્ભાવ "types.h" // u32

// apm.c
શૂન્ય apm_shutdown(શૂન્ય);
સંરચના bregs;
શૂન્ય handle_1553(સંરચના bregs *regs);

// bmp.c
સંરચના bmp_decdata *bmp_alloc(શૂન્ય);
પૂર્ણાંક bmp_decode(સંરચના bmp_decdata *bmp, unsigned અક્ષર *data, પૂર્ણાંક data_size);
શૂન્ય bmp_get_info(સંરચના bmp_decdata *bmp, પૂર્ણાંક *width, પૂર્ણાંક *height, પૂર્ણાંક *bpp);
પૂર્ણાંક bmp_show(સંરચના bmp_decdata *bmp, unsigned અક્ષર *pic, પૂર્ણાંક width
             , પૂર્ણાંક height, પૂર્ણાંક depth, પૂર્ણાંક bytes_per_લીટી_dest);

// boot.c
શૂન્ય boot_init(શૂન્ય);
શૂન્ય boot_add_bev(u16 seg, u16 bev, u16 desc, પૂર્ણાંક prio);
શૂન્ય boot_add_bcv(u16 seg, u16 ip, u16 desc, પૂર્ણાંક prio);
સંરચના drive_s;
શૂન્ય boot_add_floppy(સંરચના drive_s *drive_g, અચલ અક્ષર *desc, પૂર્ણાંક prio);
શૂન્ય boot_add_hd(સંરચના drive_s *drive_g, અચલ અક્ષર *desc, પૂર્ણાંક prio);
શૂન્ય boot_add_cd(સંરચના drive_s *drive_g, અચલ અક્ષર *desc, પૂર્ણાંક prio);
શૂન્ય boot_add_cbfs(શૂન્ય *data, અચલ અક્ષર *desc, પૂર્ણાંક prio);
શૂન્ય interactive_bootmenu(શૂન્ય);
શૂન્ય bcv_prepboot(શૂન્ય);
u8 is_bootprio_strict(શૂન્ય);
સંરચના pci_device;
પૂર્ણાંક bootprio_find_pci_device(સંરચના pci_device *pci);
પૂર્ણાંક bootprio_find_mmio_device(શૂન્ય *mmio);
પૂર્ણાંક bootprio_find_scsi_device(સંરચના pci_device *pci, પૂર્ણાંક target, પૂર્ણાંક lun);
પૂર્ણાંક bootprio_find_scsi_mmio_device(શૂન્ય *mmio, પૂર્ણાંક target, પૂર્ણાંક lun);
પૂર્ણાંક bootprio_find_ata_device(સંરચના pci_device *pci, પૂર્ણાંક chanid, પૂર્ણાંક slave);
પૂર્ણાંક bootprio_find_fdc_device(સંરચના pci_device *pci, પૂર્ણાંક port, પૂર્ણાંક fdid);
પૂર્ણાંક bootprio_find_pci_rom(સંરચના pci_device *pci, પૂર્ણાંક instance);
પૂર્ણાંક bootprio_find_named_rom(અચલ અક્ષર *name, પૂર્ણાંક instance);
સંરચના usbdevice_s;
પૂર્ણાંક bootprio_find_usb(સંરચના usbdevice_s *usbdev, પૂર્ણાંક lun);
પૂર્ણાંક get_keystroke_full(પૂર્ણાંક msec);
પૂર્ણાંક get_keystroke(પૂર્ણાંક msec);
સંરચના chs_s;
પૂર્ણાંક boot_lchs_find_pci_device(સંરચના pci_device *pci, સંરચના chs_s *chs);
પૂર્ણાંક boot_lchs_find_scsi_device(સંરચના pci_device *pci, પૂર્ણાંક target, પૂર્ણાંક lun,
                               સંરચના chs_s *chs);
પૂર્ણાંક boot_lchs_find_ata_device(સંરચના pci_device *pci, પૂર્ણાંક chanid, પૂર્ણાંક slave,
                              સંરચના chs_s *chs);

// bootsplash.c
શૂન્ય enable_vga_console(શૂન્ય);
શૂન્ય enable_bootsplash(શૂન્ય);
શૂન્ય disable_bootsplash(શૂન્ય);

// cdrom.c
બાહ્ય સંરચના eltorito_s CDEmu;
બાહ્ય સંરચના drive_s *cdemu_drive_gf;
સંરચના disk_op_s;
પૂર્ણાંક cdemu_process_op(સંરચના disk_op_s *op);
શૂન્ય cdrom_prepboot(શૂન્ય);
પૂર્ણાંક cdrom_boot(સંરચના drive_s *drive_g);
અક્ષર *cdrom_media_info(સંરચના drive_s *drive_g);

// clock.c
શૂન્ય clock_setup(શૂન્ય);
શૂન્ય handle_1583(સંરચના bregs *regs);
શૂન્ય clock_poll_irq(શૂન્ય);
u32 irqtimer_calc_ticks(u32 ગણતરી);
u32 irqtimer_calc(u32 msecs);
પૂર્ણાંક irqtimer_check(u32 end);
શૂન્ય handle_1586(સંરચના bregs *regs);

// fw/acpi.c
શૂન્ય acpi_setup(શૂન્ય);

// fw/biostable.c
શૂન્ય copy_pir(શૂન્ય *pos);
શૂન્ય copy_mptable(શૂન્ય *pos);
બાહ્ય સંરચના pir_header *PirAddr;
શૂન્ય copy_acpi_rsdp(શૂન્ય *pos);
બાહ્ય સંરચના rsdp_descriptor *RsdpAddr;
બાહ્ય u32 acpi_pm1a_cnt;
બાહ્ય u16 acpi_pm_base;
શૂન્ય *find_acpi_rsdp(શૂન્ય);
શૂન્ય *find_acpi_table(u32 signature);
u32 find_resume_vector(શૂન્ય);
શૂન્ય acpi_reboot(શૂન્ય);
શૂન્ય find_acpi_features(શૂન્ય);
શૂન્ય *smbios_get_tables(u32 *length);
શૂન્ય copy_smbios_21(શૂન્ય *pos);
શૂન્ય display_uuid(શૂન્ય);
શૂન્ય copy_table(શૂન્ય *pos);
શૂન્ય smbios_setup(શૂન્ય);

// fw/dsdt_parser.c
સંરચના acpi_device;
શૂન્ય acpi_dsdt_parse(શૂન્ય);
સંરચના acpi_device *acpi_dsdt_find_string(સંરચના acpi_device *prev, અચલ અક્ષર *hid);
સંરચના acpi_device *acpi_dsdt_find_eisaid(સંરચના acpi_device *prev, u16 eisaid);
અક્ષર *acpi_dsdt_name(સંરચના acpi_device *dev);
પૂર્ણાંક acpi_dsdt_present_eisaid(u16 eisaid);
પૂર્ણાંક acpi_dsdt_find_io(સંરચના acpi_device *dev, u64 *min, u64 *max);
પૂર્ણાંક acpi_dsdt_find_mem(સંરચના acpi_device *dev, u64 *min, u64 *max);
પૂર્ણાંક acpi_dsdt_find_irq(સંરચના acpi_device *dev, u64 *irq);

// fw/coreboot.c
બાહ્ય અચલ અક્ષર *CBvendor, *CBpart;
સંરચના cbfs_file;
શૂન્ય coreboot_debug_putc(અક્ષર c);
શૂન્ય cbfs_run_payload(સંરચના cbfs_file *file);
શૂન્ય coreboot_platform_setup(શૂન્ય);
શૂન્ય cbfs_payload_setup(શૂન્ય);
શૂન્ય coreboot_preinit(શૂન્ય);
શૂન્ય coreboot_cbfs_init(શૂન્ય);
સંરચના cb_header;
શૂન્ય *find_cb_subtable(સંરચના cb_header *cbh, u32 tag);
સંરચના cb_header *find_cb_table(શૂન્ય);

// fw/csm.c
પૂર્ણાંક csm_bootprio_fdc(સંરચના pci_device *pci, પૂર્ણાંક port, પૂર્ણાંક fdid);
પૂર્ણાંક csm_bootprio_ata(સંરચના pci_device *pci, પૂર્ણાંક chanid, પૂર્ણાંક slave);
પૂર્ણાંક csm_bootprio_pci(સંરચના pci_device *pci);

// fw/mptable.c
શૂન્ય mptable_setup(શૂન્ય);

// fw/mtrr.c
શૂન્ય mtrr_setup(શૂન્ય);

// fw/multiboot.c
શૂન્ય multiboot_init(શૂન્ય);

// fw/pciinit.c
બાહ્ય u64 pcimem_start, pcimem_end;
બાહ્ય u64 pcimem64_start, pcimem64_end;
બાહ્ય અચલ u8 pci_irqs[4];
શૂન્ય pci_setup(શૂન્ય);
શૂન્ય pci_resume(શૂન્ય);

// fw/pirtable.c
શૂન્ય pirtable_setup(શૂન્ય);

// fw/shadow.c
શૂન્ય make_bios_writable(શૂન્ય);
શૂન્ય make_bios_readonly(શૂન્ય);
શૂન્ય qemu_reboot(શૂન્ય);

// fw/smbios.c
શૂન્ય smbios_legacy_setup(શૂન્ય);

// fw/smm.c
શૂન્ય smm_device_setup(શૂન્ય);
શૂન્ય smm_setup(શૂન્ય);

// fw/smp.c
બાહ્ય u32 Max_ગણતરી_CPUs;
શૂન્ય wrmsr_smp(u32 index, u64 val);
શૂન્ય smp_setup(શૂન્ય);
શૂન્ય smp_resume(શૂન્ય);
પૂર્ણાંક apic_id_is_present(u8 apic_id);

// hw/dma.c
પૂર્ણાંક dma_floppy(u32 addr, પૂર્ણાંક ગણતરી, પૂર્ણાંક isWrite);
શૂન્ય dma_setup(શૂન્ય);

// hw/floppy.c
બાહ્ય સંરચના floppy_ext_dbt_s diskette_param_table2;
શૂન્ય floppy_setup(શૂન્ય);
સંરચના drive_s *init_floppy(પૂર્ણાંક floppyid, પૂર્ણાંક ftype);
પૂર્ણાંક find_floppy_type(u32 size);
પૂર્ણાંક floppy_process_op(સંરચના disk_op_s *op);
શૂન્ય floppy_tick(શૂન્ય);

// hw/ramdisk.c
શૂન્ય ramdisk_setup(શૂન્ય);
પૂર્ણાંક ramdisk_process_op(સંરચના disk_op_s *op);

// hw/sdcard.c
પૂર્ણાંક sdcard_process_op(સંરચના disk_op_s *op);
શૂન્ય sdcard_setup(શૂન્ય);

// hw/timer.c
શૂન્ય timer_setup(શૂન્ય);
શૂન્ય pmtimer_setup(u16 ioport);
શૂન્ય tsctimer_setfreq(u32 khz, અચલ અક્ષર *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
પૂર્ણાંક timer_check(u32 end);
શૂન્ય ndelay(u32 ગણતરી);
શૂન્ય udelay(u32 ગણતરી);
શૂન્ય mdelay(u32 ગણતરી);
શૂન્ય nsleep(u32 ગણતરી);
શૂન્ય usleep(u32 ગણતરી);
શૂન્ય msleep(u32 ગણતરી);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
શૂન્ય pit_setup(શૂન્ય);

// jpeg.c
સંરચના jpeg_decdata *jpeg_alloc(શૂન્ય);
પૂર્ણાંક jpeg_decode(સંરચના jpeg_decdata *jpeg, unsigned અક્ષર *buf);
શૂન્ય jpeg_get_size(સંરચના jpeg_decdata *jpeg, પૂર્ણાંક *width, પૂર્ણાંક *height);
પૂર્ણાંક jpeg_show(સંરચના jpeg_decdata *jpeg, unsigned અક્ષર *pic, પૂર્ણાંક width
              , પૂર્ણાંક height, પૂર્ણાંક depth, પૂર્ણાંક bytes_per_લીટી_dest);

// kbd.c
શૂન્ય kbd_init(શૂન્ય);
શૂન્ય handle_15c2(સંરચના bregs *regs);
શૂન્ય process_ચાવી(u8 ચાવી);
u8 enqueue_ચાવી(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
બાહ્ય પૂર્ણાંક HaveRunPost;
બાહ્ય સંરચના bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
બાહ્ય સંરચના floppy_dbt_s diskette_param_table __aligned(1);
બાહ્ય u8 BiosChecksum;
પૂર્ણાંક in_post(શૂન્ય);
શૂન્ય mathcp_setup(શૂન્ય);

// mouse.c
શૂન્ય mouse_init(શૂન્ય);
શૂન્ય process_mouse(u8 data);

// optionroms.c
સંરચના rom_header;
શૂન્ય callrom(સંરચના rom_header *rom, u16 bdf);
શૂન્ય call_bcv(u16 seg, u16 ip);
પૂર્ણાંક is_pci_vga(સંરચના pci_device *pci);
શૂન્ય optionrom_setup(શૂન્ય);
શૂન્ય vgarom_setup(શૂન્ય);
શૂન્ય s3_resume_vga(શૂન્ય);
બાહ્ય પૂર્ણાંક ScreenAndDebug;

// pcibios.c
શૂન્ય handle_1ab1(સંરચના bregs *regs);
શૂન્ય bios32_init(શૂન્ય);

// pmm.c
શૂન્ય pmm_init(શૂન્ય);
શૂન્ય pmm_prepboot(શૂન્ય);

// pnpbios.c
u16 get_pnp_offset(શૂન્ય);
શૂન્ય pnp_init(શૂન્ય);

// post.c
શૂન્ય interface_init(શૂન્ય);
શૂન્ય device_hardware_setup(શૂન્ય);
શૂન્ય prepareboot(શૂન્ય);
શૂન્ય startBoot(શૂન્ય);
શૂન્ય reloc_preinit(શૂન્ય *f, શૂન્ય *arg);
શૂન્ય code_mutable_preinit(શૂન્ય);

// sercon.c
શૂન્ય sercon_setup(શૂન્ય);
શૂન્ય sercon_check_event(શૂન્ય);

// serial.c
શૂન્ય serial_setup(શૂન્ય);
શૂન્ય lpt_setup(શૂન્ય);

// version.c
બાહ્ય અચલ અક્ષર VERSION[], BUILDINFO[];

// vgahooks.c
શૂન્ય handle_155f(સંરચના bregs *regs);
શૂન્ય handle_157f(સંરચના bregs *regs);
શૂન્ય vgahook_setup(સંરચના pci_device *pci);

#endif // util.h
