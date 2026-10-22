// Misc function and variable declarations.
#اگر_تعریف_نشده __UTIL_H
#تعریف_کن __UTIL_H

#ضمن "types.h" // u32

// apm.c
فراغ apm_shutdown(فراغ);
بنية bregs;
فراغ handle_1553(بنية bregs *regs);

// bmp.c
بنية bmp_decdata *bmp_alloc(فراغ);
صحيح bmp_decode(بنية bmp_decdata *bmp, بیعلامت حرف *داده, صحيح داده_اندازه);
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *عرض, صحيح *ارتفاع, صحيح *bpp);
صحيح bmp_show(بنية bmp_decdata *bmp, بیعلامت حرف *pic, صحيح عرض
             , صحيح ارتفاع, صحيح عمق, صحيح بایتها_در_هر_سطر_dest);

// boot.c
فراغ boot_init(فراغ);
فراغ boot_جمع_کن_bev(u16 seg, u16 bev, u16 desc, صحيح prio);
فراغ boot_جمع_کن_bcv(u16 seg, u16 ip, u16 desc, صحيح prio);
بنية drive_s;
فراغ boot_جمع_کن_floppy(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_جمع_کن_hd(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_جمع_کن_cd(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_جمع_کن_cbfs(فراغ *داده, ثابت حرف *desc, صحيح prio);
فراغ interactive_bootmenu(فراغ);
فراغ bcv_prepboot(فراغ);
u8 is_bootprio_strict(فراغ);
بنية pci_device;
صحيح bootprio_پیدا_کن_pci_device(بنية pci_device *pci);
صحيح bootprio_پیدا_کن_mmio_device(فراغ *mmio);
صحيح bootprio_پیدا_کن_scsi_device(بنية pci_device *pci, صحيح هدف, صحيح lun);
صحيح bootprio_پیدا_کن_scsi_mmio_device(فراغ *mmio, صحيح هدف, صحيح lun);
صحيح bootprio_پیدا_کن_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave);
صحيح bootprio_پیدا_کن_fdc_device(بنية pci_device *pci, صحيح پورت, صحيح fdid);
صحيح bootprio_پیدا_کن_pci_rom(بنية pci_device *pci, صحيح instance);
صحيح bootprio_پیدا_کن_named_rom(ثابت حرف *name, صحيح instance);
بنية usbdevice_s;
صحيح bootprio_پیدا_کن_usb(بنية usbdevice_s *usbdev, صحيح lun);
صحيح get_keystroke_پر(صحيح msec);
صحيح get_keystroke(صحيح msec);
بنية chs_s;
صحيح boot_lchs_پیدا_کن_pci_device(بنية pci_device *pci, بنية chs_s *chs);
صحيح boot_lchs_پیدا_کن_scsi_device(بنية pci_device *pci, صحيح هدف, صحيح lun,
                               بنية chs_s *chs);
صحيح boot_lchs_پیدا_کن_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave,
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
فراغ clock_راهاندازی(فراغ);
فراغ handle_1583(بنية bregs *regs);
فراغ clock_poll_irq(فراغ);
u32 irqtimer_calc_ticks(u32 شمار);
u32 irqtimer_calc(u32 msecs);
صحيح irqtimer_check(u32 انتها);
فراغ handle_1586(بنية bregs *regs);

// fw/acpi.c
فراغ acpi_راهاندازی(فراغ);

// fw/biostable.c
فراغ copy_pir(فراغ *pos);
فراغ copy_mptable(فراغ *pos);
خارجي بنية pir_سرآیند *PirAddr;
فراغ copy_acpi_rsdp(فراغ *pos);
خارجي بنية rsdp_descriptor *RsdpAddr;
خارجي u32 acpi_pm1a_cnt;
خارجي u16 acpi_pm_پایه;
فراغ *پیدا_کن_acpi_rsdp(فراغ);
فراغ *پیدا_کن_acpi_جدول(u32 signature);
u32 پیدا_کن_resume_وکتور(فراغ);
فراغ acpi_reboot(فراغ);
فراغ پیدا_کن_acpi_features(فراغ);
فراغ *smbios_get_tables(u32 *درازی);
فراغ copy_smbios_21(فراغ *pos);
فراغ display_uuid(فراغ);
فراغ copy_جدول(فراغ *pos);
فراغ smbios_راهاندازی(فراغ);

// fw/dsdt_parser.c
بنية acpi_device;
فراغ acpi_dsdt_parse(فراغ);
بنية acpi_device *acpi_dsdt_پیدا_کن_string(بنية acpi_device *prev, ثابت حرف *hid);
بنية acpi_device *acpi_dsdt_پیدا_کن_eisaid(بنية acpi_device *prev, u16 eisaid);
حرف *acpi_dsdt_name(بنية acpi_device *dev);
صحيح acpi_dsdt_present_eisaid(u16 eisaid);
صحيح acpi_dsdt_پیدا_کن_io(بنية acpi_device *dev, u64 *min, u64 *max);
صحيح acpi_dsdt_پیدا_کن_mem(بنية acpi_device *dev, u64 *min, u64 *max);
صحيح acpi_dsdt_پیدا_کن_irq(بنية acpi_device *dev, u64 *irq);

// fw/coreboot.c
خارجي ثابت حرف *CBvendor, *CBpart;
بنية cbfs_file;
فراغ coreboot_debug_putc(حرف c);
فراغ cbfs_پیاپی_payload(بنية cbfs_file *file);
فراغ coreboot_platform_راهاندازی(فراغ);
فراغ cbfs_payload_راهاندازی(فراغ);
فراغ coreboot_preinit(فراغ);
فراغ coreboot_cbfs_init(فراغ);
بنية cb_سرآیند;
فراغ *پیدا_کن_cb_subtable(بنية cb_سرآیند *cbh, u32 برچسب);
بنية cb_سرآیند *پیدا_کن_cb_جدول(فراغ);

// fw/csm.c
صحيح csm_bootprio_fdc(بنية pci_device *pci, صحيح پورت, صحيح fdid);
صحيح csm_bootprio_ata(بنية pci_device *pci, صحيح chanid, صحيح slave);
صحيح csm_bootprio_pci(بنية pci_device *pci);

// fw/mptable.c
فراغ mptable_راهاندازی(فراغ);

// fw/mtrr.c
فراغ mtrr_راهاندازی(فراغ);

// fw/multiboot.c
فراغ multiboot_init(فراغ);

// fw/pciinit.c
خارجي u64 pcimem_شروع, pcimem_انتها;
خارجي u64 pcimem64_شروع, pcimem64_انتها;
خارجي ثابت u8 pci_irqs[4];
فراغ pci_راهاندازی(فراغ);
فراغ pci_resume(فراغ);

// fw/pirtable.c
فراغ pirtable_راهاندازی(فراغ);

// fw/shadow.c
فراغ make_bios_writable(فراغ);
فراغ make_bios_readonly(فراغ);
فراغ qemu_reboot(فراغ);

// fw/smbios.c
فراغ smbios_legacy_راهاندازی(فراغ);

// fw/smm.c
فراغ smm_device_راهاندازی(فراغ);
فراغ smm_راهاندازی(فراغ);

// fw/smp.c
خارجي u32 Max_شمار_CPUs;
فراغ wrmsr_smp(u32 شاخص, u64 val);
فراغ smp_راهاندازی(فراغ);
فراغ smp_resume(فراغ);
صحيح apic_id_is_present(u8 apic_id);

// hw/dma.c
صحيح dma_floppy(u32 addr, صحيح شمار, صحيح isWrite);
فراغ dma_راهاندازی(فراغ);

// hw/floppy.c
خارجي بنية floppy_ext_dbt_s diskette_param_table2;
فراغ floppy_راهاندازی(فراغ);
بنية drive_s *init_floppy(صحيح floppyid, صحيح ftype);
صحيح پیدا_کن_floppy_type(u32 اندازه);
صحيح floppy_process_op(بنية disk_op_s *op);
فراغ floppy_tick(فراغ);

// hw/ramdisk.c
فراغ ramdisk_راهاندازی(فراغ);
صحيح ramdisk_process_op(بنية disk_op_s *op);

// hw/sdcard.c
صحيح sdcard_process_op(بنية disk_op_s *op);
فراغ sdcard_راهاندازی(فراغ);

// hw/timer.c
فراغ timer_راهاندازی(فراغ);
فراغ pmtimer_راهاندازی(u16 ioport);
فراغ tsctimer_setfreq(u32 khz, ثابت حرف *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
صحيح timer_check(u32 انتها);
فراغ ndelay(u32 شمار);
فراغ udelay(u32 شمار);
فراغ mdelay(u32 شمار);
فراغ nsleep(u32 شمار);
فراغ usleep(u32 شمار);
فراغ msleep(u32 شمار);
u32 ticks_مقصد_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
فراغ pit_راهاندازی(فراغ);

// jpeg.c
بنية jpeg_decdata *jpeg_alloc(فراغ);
صحيح jpeg_decode(بنية jpeg_decdata *jpeg, بیعلامت حرف *buf);
فراغ jpeg_get_اندازه(بنية jpeg_decdata *jpeg, صحيح *عرض, صحيح *ارتفاع);
صحيح jpeg_show(بنية jpeg_decdata *jpeg, بیعلامت حرف *pic, صحيح عرض
              , صحيح ارتفاع, صحيح عمق, صحيح بایتها_در_هر_سطر_dest);

// kbd.c
فراغ kbd_init(فراغ);
فراغ handle_15c2(بنية bregs *regs);
فراغ process_کلید(u8 کلید);
u8 افزودن_به_صف_کلید(u16 keycode);
u16 ascii_مقصد_keycode(u8 ascii);

// misc.c
خارجي صحيح Have_پیاپی_Post;
خارجي بنية bios_config_جدول_s BIOS_CONFIG_جدول __aligned(1);
خارجي بنية floppy_dbt_s diskette_param_جدول __aligned(1);
خارجي u8 BiosChecksum;
صحيح درون_post(فراغ);
فراغ mathcp_راهاندازی(فراغ);

// mouse.c
فراغ mouse_init(فراغ);
فراغ process_mouse(u8 داده);

// optionroms.c
بنية rom_سرآیند;
فراغ callrom(بنية rom_سرآیند *rom, u16 bdf);
فراغ call_bcv(u16 seg, u16 ip);
صحيح is_pci_vga(بنية pci_device *pci);
فراغ optionrom_راهاندازی(فراغ);
فراغ vgarom_راهاندازی(فراغ);
فراغ s3_resume_vga(فراغ);
خارجي صحيح ScreenAndDebug;

// pcibios.c
فراغ handle_1ab1(بنية bregs *regs);
فراغ bios32_init(فراغ);

// pmm.c
فراغ pmm_init(فراغ);
فراغ pmm_prepboot(فراغ);

// pnpbios.c
u16 get_pnp_جابه_جایی(فراغ);
فراغ pnp_init(فراغ);

// post.c
فراغ interface_init(فراغ);
فراغ device_hardware_راهاندازی(فراغ);
فراغ prepareboot(فراغ);
فراغ شروع_Boot(فراغ);
فراغ reloc_preinit(فراغ *f, فراغ *arg);
فراغ کد_mutable_preinit(فراغ);

// sercon.c
فراغ sercon_راهاندازی(فراغ);
فراغ sercon_check_event(فراغ);

// serial.c
فراغ serial_راهاندازی(فراغ);
فراغ lpt_راهاندازی(فراغ);

// version.c
خارجي ثابت حرف نسخه[], BUILDINFO[];

// vgahooks.c
فراغ handle_155f(بنية bregs *regs);
فراغ handle_157f(بنية bregs *regs);
فراغ vgahook_راهاندازی(بنية pci_device *pci);

#پایان_اگر // util.h
