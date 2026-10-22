// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#ضمن "types.h" // u32

// apm.c
فراغ apm_shutdown(فراغ);
بنية bregs;
فراغ handle_1553(بنية bregs *regs);

// bmp.c
بنية bmp_decdata *bmp_alloc(فراغ);
صحيح bmp_decode(بنية bmp_decdata *bmp, unsigned حرف *data, صحيح data_size);
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *width, صحيح *height, صحيح *bpp);
صحيح bmp_show(بنية bmp_decdata *bmp, unsigned حرف *pic, صحيح width
             , صحيح height, صحيح depth, صحيح bytes_per_قۇر_dest);

// boot.c
فراغ boot_init(فراغ);
فراغ boot_add_bev(u16 seg, u16 bev, u16 desc, صحيح prio);
فراغ boot_add_bcv(u16 seg, u16 ip, u16 desc, صحيح prio);
بنية drive_s;
فراغ boot_add_floppy(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_add_hd(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_add_cd(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_add_cbfs(فراغ *data, ثابت حرف *desc, صحيح prio);
فراغ interactive_bootmenu(فراغ);
فراغ bcv_prepboot(فراغ);
u8 is_bootprio_strict(فراغ);
بنية pci_device;
صحيح bootprio_find_pci_device(بنية pci_device *pci);
صحيح bootprio_find_mmio_device(فراغ *mmio);
صحيح bootprio_find_scsi_device(بنية pci_device *pci, صحيح target, صحيح lun);
صحيح bootprio_find_scsi_mmio_device(فراغ *mmio, صحيح target, صحيح lun);
صحيح bootprio_find_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave);
صحيح bootprio_find_fdc_device(بنية pci_device *pci, صحيح port, صحيح fdid);
صحيح bootprio_find_pci_rom(بنية pci_device *pci, صحيح instance);
صحيح bootprio_find_named_rom(ثابت حرف *name, صحيح instance);
بنية usbdevice_s;
صحيح bootprio_find_usb(بنية usbdevice_s *usbdev, صحيح lun);
صحيح get_keystroke_full(صحيح msec);
صحيح get_keystroke(صحيح msec);
بنية chs_s;
صحيح boot_lchs_find_pci_device(بنية pci_device *pci, بنية chs_s *chs);
صحيح boot_lchs_find_scsi_device(بنية pci_device *pci, صحيح target, صحيح lun,
                               بنية chs_s *chs);
صحيح boot_lchs_find_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave,
                              بنية chs_s *chs);

// bootsplash.c
فراغ enable_vga_console(فراغ);
فراغ enable_bootsplash(فراغ);
فراغ disable_bootsplash(فراغ);

// cdrom.c
خارجي بنية eltorito_s CDEmu;
خارجي بنية drive_s *cdemu_drive_gf;
بنية disk_op_s;
صحيح cdemu_process_op(بنية disk_op_s *op);
فراغ cdrom_prepboot(فراغ);
صحيح cdrom_boot(بنية drive_s *drive_g);
حرف *cdrom_media_info(بنية drive_s *drive_g);

// clock.c
فراغ clock_setup(فراغ);
فراغ handle_1583(بنية bregs *regs);
فراغ clock_poll_irq(فراغ);
u32 irqtimer_calc_ticks(u32 سانى);
u32 irqtimer_calc(u32 msecs);
صحيح irqtimer_check(u32 end);
فراغ handle_1586(بنية bregs *regs);

// fw/acpi.c
فراغ acpi_setup(فراغ);

// fw/biostable.c
فراغ copy_pir(فراغ *pos);
فراغ copy_mptable(فراغ *pos);
خارجي بنية pir_header *PirAddr;
فراغ copy_acpi_rsdp(فراغ *pos);
خارجي بنية rsdp_descriptor *RsdpAddr;
خارجي u32 acpi_pm1a_cnt;
خارجي u16 acpi_pm_base;
فراغ *find_acpi_rsdp(فراغ);
فراغ *find_acpi_table(u32 signature);
u32 find_resume_vector(فراغ);
فراغ acpi_reboot(فراغ);
فراغ find_acpi_features(فراغ);
فراغ *smbios_get_tables(u32 *length);
فراغ copy_smbios_21(فراغ *pos);
فراغ display_uuid(فراغ);
فراغ copy_table(فراغ *pos);
فراغ smbios_setup(فراغ);

// fw/dsdt_parser.c
بنية acpi_device;
فراغ acpi_dsdt_parse(فراغ);
بنية acpi_device *acpi_dsdt_find_string(بنية acpi_device *prev, ثابت حرف *hid);
بنية acpi_device *acpi_dsdt_find_eisaid(بنية acpi_device *prev, u16 eisaid);
حرف *acpi_dsdt_name(بنية acpi_device *dev);
صحيح acpi_dsdt_present_eisaid(u16 eisaid);
صحيح acpi_dsdt_find_io(بنية acpi_device *dev, u64 *min, u64 *max);
صحيح acpi_dsdt_find_mem(بنية acpi_device *dev, u64 *min, u64 *max);
صحيح acpi_dsdt_find_irq(بنية acpi_device *dev, u64 *irq);

// fw/coreboot.c
خارجي ثابت حرف *CBvendor, *CBpart;
بنية cbfs_file;
فراغ coreboot_debug_putc(حرف c);
فراغ cbfs_run_payload(بنية cbfs_file *file);
فراغ coreboot_platform_setup(فراغ);
فراغ cbfs_payload_setup(فراغ);
فراغ coreboot_preinit(فراغ);
فراغ coreboot_cbfs_init(فراغ);
بنية cb_header;
فراغ *find_cb_subtable(بنية cb_header *cbh, u32 tag);
بنية cb_header *find_cb_table(فراغ);

// fw/csm.c
صحيح csm_bootprio_fdc(بنية pci_device *pci, صحيح port, صحيح fdid);
صحيح csm_bootprio_ata(بنية pci_device *pci, صحيح chanid, صحيح slave);
صحيح csm_bootprio_pci(بنية pci_device *pci);

// fw/mptable.c
فراغ mptable_setup(فراغ);

// fw/mtrr.c
فراغ mtrr_setup(فراغ);

// fw/multiboot.c
فراغ multiboot_init(فراغ);

// fw/pciinit.c
خارجي u64 pcimem_start, pcimem_end;
خارجي u64 pcimem64_start, pcimem64_end;
خارجي ثابت u8 pci_irqs[4];
فراغ pci_setup(فراغ);
فراغ pci_resume(فراغ);

// fw/pirtable.c
فراغ pirtable_setup(فراغ);

// fw/shadow.c
فراغ make_bios_writable(فراغ);
فراغ make_bios_readonly(فراغ);
فراغ qemu_reboot(فراغ);

// fw/smbios.c
فراغ smbios_legacy_setup(فراغ);

// fw/smm.c
فراغ smm_device_setup(فراغ);
فراغ smm_setup(فراغ);

// fw/smp.c
خارجي u32 Max_سانى_CPUs;
فراغ wrmsr_smp(u32 index, u64 val);
فراغ smp_setup(فراغ);
فراغ smp_resume(فراغ);
صحيح apic_id_is_present(u8 apic_id);

// hw/dma.c
صحيح dma_floppy(u32 addr, صحيح سانى, صحيح isWrite);
فراغ dma_setup(فراغ);

// hw/floppy.c
خارجي بنية floppy_ext_dbt_s diskette_param_table2;
فراغ floppy_setup(فراغ);
بنية drive_s *init_floppy(صحيح floppyid, صحيح ftype);
صحيح find_floppy_type(u32 size);
صحيح floppy_process_op(بنية disk_op_s *op);
فراغ floppy_tick(فراغ);

// hw/ramdisk.c
فراغ ramdisk_setup(فراغ);
صحيح ramdisk_process_op(بنية disk_op_s *op);

// hw/sdcard.c
صحيح sdcard_process_op(بنية disk_op_s *op);
فراغ sdcard_setup(فراغ);

// hw/timer.c
فراغ timer_setup(فراغ);
فراغ pmtimer_setup(u16 ioport);
فراغ tsctimer_setfreq(u32 khz, ثابت حرف *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
صحيح timer_check(u32 end);
فراغ ndelay(u32 سانى);
فراغ udelay(u32 سانى);
فراغ mdelay(u32 سانى);
فراغ nsleep(u32 سانى);
فراغ usleep(u32 سانى);
فراغ msleep(u32 سانى);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
فراغ pit_setup(فراغ);

// jpeg.c
بنية jpeg_decdata *jpeg_alloc(فراغ);
صحيح jpeg_decode(بنية jpeg_decdata *jpeg, unsigned حرف *buf);
فراغ jpeg_get_size(بنية jpeg_decdata *jpeg, صحيح *width, صحيح *height);
صحيح jpeg_show(بنية jpeg_decdata *jpeg, unsigned حرف *pic, صحيح width
              , صحيح height, صحيح depth, صحيح bytes_per_قۇر_dest);

// kbd.c
فراغ kbd_init(فراغ);
فراغ handle_15c2(بنية bregs *regs);
فراغ process_ئاچقۇچ(u8 ئاچقۇچ);
u8 enqueue_ئاچقۇچ(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
خارجي صحيح HaveRunPost;
خارجي بنية bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
خارجي بنية floppy_dbt_s diskette_param_table __aligned(1);
خارجي u8 BiosChecksum;
صحيح in_post(فراغ);
فراغ mathcp_setup(فراغ);

// mouse.c
فراغ mouse_init(فراغ);
فراغ process_mouse(u8 data);

// optionroms.c
بنية rom_header;
فراغ callrom(بنية rom_header *rom, u16 bdf);
فراغ call_bcv(u16 seg, u16 ip);
صحيح is_pci_vga(بنية pci_device *pci);
فراغ optionrom_setup(فراغ);
فراغ vgarom_setup(فراغ);
فراغ s3_resume_vga(فراغ);
خارجي صحيح ScreenAndDebug;

// pcibios.c
فراغ handle_1ab1(بنية bregs *regs);
فراغ bios32_init(فراغ);

// pmm.c
فراغ pmm_init(فراغ);
فراغ pmm_prepboot(فراغ);

// pnpbios.c
u16 get_pnp_offset(فراغ);
فراغ pnp_init(فراغ);

// post.c
فراغ interface_init(فراغ);
فراغ device_hardware_setup(فراغ);
فراغ prepareboot(فراغ);
فراغ startBoot(فراغ);
فراغ reloc_preinit(فراغ *f, فراغ *arg);
فراغ code_mutable_preinit(فراغ);

// sercon.c
فراغ sercon_setup(فراغ);
فراغ sercon_check_event(فراغ);

// serial.c
فراغ serial_setup(فراغ);
فراغ lpt_setup(فراغ);

// version.c
خارجي ثابت حرف VERSION[], BUILDINFO[];

// vgahooks.c
فراغ handle_155f(بنية bregs *regs);
فراغ handle_157f(بنية bregs *regs);
فراغ vgahook_setup(بنية pci_device *pci);

#endif // util.h
