// Misc function and variable declarations.
#که_نه_تعریف_شوی __UTIL_H
#تعریف_کړه __UTIL_H

#ضمن "types.h" // u32

// apm.c
فراغ apm_shutdown(فراغ);
بنية bregs;
فراغ handle_1553(بنية bregs *regs);

// bmp.c
بنية bmp_decdata *bmp_alloc(فراغ);
صحيح bmp_decode(بنية bmp_decdata *bmp, بینښه حرف *معلومات, صحيح معلومات_اندازه);
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *پلنوالی, صحيح *لوړوالی, صحيح *bpp);
صحيح bmp_show(بنية bmp_decdata *bmp, بینښه حرف *pic, صحيح پلنوالی
             , صحيح لوړوالی, صحيح ژوروالی, صحيح بایټونه_فی_کرښه_dest);

// boot.c
فراغ boot_init(فراغ);
فراغ boot_جمع_کړه_bev(u16 seg, u16 bev, u16 desc, صحيح prio);
فراغ boot_جمع_کړه_bcv(u16 seg, u16 ip, u16 desc, صحيح prio);
بنية drive_s;
فراغ boot_جمع_کړه_floppy(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_جمع_کړه_hd(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_جمع_کړه_cd(بنية drive_s *drive_g, ثابت حرف *desc, صحيح prio);
فراغ boot_جمع_کړه_cbfs(فراغ *معلومات, ثابت حرف *desc, صحيح prio);
فراغ interactive_bootmenu(فراغ);
فراغ bcv_prepboot(فراغ);
u8 is_bootprio_strict(فراغ);
بنية pci_device;
صحيح bootprio_پیدا_کړه_pci_device(بنية pci_device *pci);
صحيح bootprio_پیدا_کړه_mmio_device(فراغ *mmio);
صحيح bootprio_پیدا_کړه_scsi_device(بنية pci_device *pci, صحيح موخه, صحيح lun);
صحيح bootprio_پیدا_کړه_scsi_mmio_device(فراغ *mmio, صحيح موخه, صحيح lun);
صحيح bootprio_پیدا_کړه_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave);
صحيح bootprio_پیدا_کړه_fdc_device(بنية pci_device *pci, صحيح پورټ, صحيح fdid);
صحيح bootprio_پیدا_کړه_pci_rom(بنية pci_device *pci, صحيح instance);
صحيح bootprio_پیدا_کړه_named_rom(ثابت حرف *name, صحيح instance);
بنية usbdevice_s;
صحيح bootprio_پیدا_کړه_usb(بنية usbdevice_s *usbdev, صحيح lun);
صحيح get_keystroke_ډک(صحيح msec);
صحيح get_keystroke(صحيح msec);
بنية chs_s;
صحيح boot_lchs_پیدا_کړه_pci_device(بنية pci_device *pci, بنية chs_s *chs);
صحيح boot_lchs_پیدا_کړه_scsi_device(بنية pci_device *pci, صحيح موخه, صحيح lun,
                               بنية chs_s *chs);
صحيح boot_lchs_پیدا_کړه_ata_device(بنية pci_device *pci, صحيح chanid, صحيح slave,
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
فراغ clock_تیاری(فراغ);
فراغ handle_1583(بنية bregs *regs);
فراغ clock_poll_irq(فراغ);
u32 irqtimer_calc_ticks(u32 شمېر);
u32 irqtimer_calc(u32 msecs);
صحيح irqtimer_check(u32 پای_ټکی);
فراغ handle_1586(بنية bregs *regs);

// fw/acpi.c
فراغ acpi_تیاری(فراغ);

// fw/biostable.c
فراغ copy_pir(فراغ *pos);
فراغ copy_mptable(فراغ *pos);
خارجي بنية pir_سرلیک *PirAddr;
فراغ copy_acpi_rsdp(فراغ *pos);
خارجي بنية rsdp_descriptor *RsdpAddr;
خارجي u32 acpi_pm1a_cnt;
خارجي u16 acpi_pm_بنسټ;
فراغ *پیدا_کړه_acpi_rsdp(فراغ);
فراغ *پیدا_کړه_acpi_جدول(u32 signature);
u32 پیدا_کړه_resume_ویکټور(فراغ);
فراغ acpi_reboot(فراغ);
فراغ پیدا_کړه_acpi_features(فراغ);
فراغ *smbios_get_tables(u32 *اوږدتیا);
فراغ copy_smbios_21(فراغ *pos);
فراغ display_uuid(فراغ);
فراغ copy_جدول(فراغ *pos);
فراغ smbios_تیاری(فراغ);

// fw/dsdt_parser.c
بنية acpi_device;
فراغ acpi_dsdt_parse(فراغ);
بنية acpi_device *acpi_dsdt_پیدا_کړه_string(بنية acpi_device *prev, ثابت حرف *hid);
بنية acpi_device *acpi_dsdt_پیدا_کړه_eisaid(بنية acpi_device *prev, u16 eisaid);
حرف *acpi_dsdt_name(بنية acpi_device *dev);
صحيح acpi_dsdt_present_eisaid(u16 eisaid);
صحيح acpi_dsdt_پیدا_کړه_io(بنية acpi_device *dev, u64 *min, u64 *max);
صحيح acpi_dsdt_پیدا_کړه_mem(بنية acpi_device *dev, u64 *min, u64 *max);
صحيح acpi_dsdt_پیدا_کړه_irq(بنية acpi_device *dev, u64 *irq);

// fw/coreboot.c
خارجي ثابت حرف *CBvendor, *CBpart;
بنية cbfs_file;
فراغ coreboot_debug_putc(حرف c);
فراغ cbfs_پرلپسې_payload(بنية cbfs_file *file);
فراغ coreboot_platform_تیاری(فراغ);
فراغ cbfs_payload_تیاری(فراغ);
فراغ coreboot_preinit(فراغ);
فراغ coreboot_cbfs_init(فراغ);
بنية cb_سرلیک;
فراغ *پیدا_کړه_cb_subtable(بنية cb_سرلیک *cbh, u32 نښان);
بنية cb_سرلیک *پیدا_کړه_cb_جدول(فراغ);

// fw/csm.c
صحيح csm_bootprio_fdc(بنية pci_device *pci, صحيح پورټ, صحيح fdid);
صحيح csm_bootprio_ata(بنية pci_device *pci, صحيح chanid, صحيح slave);
صحيح csm_bootprio_pci(بنية pci_device *pci);

// fw/mptable.c
فراغ mptable_تیاری(فراغ);

// fw/mtrr.c
فراغ mtrr_تیاری(فراغ);

// fw/multiboot.c
فراغ multiboot_init(فراغ);

// fw/pciinit.c
خارجي u64 pcimem_پیل, pcimem_پای_ټکی;
خارجي u64 pcimem64_پیل, pcimem64_پای_ټکی;
خارجي ثابت u8 pci_irqs[4];
فراغ pci_تیاری(فراغ);
فراغ pci_resume(فراغ);

// fw/pirtable.c
فراغ pirtable_تیاری(فراغ);

// fw/shadow.c
فراغ make_bios_writable(فراغ);
فراغ make_bios_readonly(فراغ);
فراغ qemu_reboot(فراغ);

// fw/smbios.c
فراغ smbios_legacy_تیاری(فراغ);

// fw/smm.c
فراغ smm_device_تیاری(فراغ);
فراغ smm_تیاری(فراغ);

// fw/smp.c
خارجي u32 Max_شمېر_CPUs;
فراغ wrmsr_smp(u32 شاخص, u64 val);
فراغ smp_تیاری(فراغ);
فراغ smp_resume(فراغ);
صحيح apic_id_is_present(u8 apic_id);

// hw/dma.c
صحيح dma_floppy(u32 addr, صحيح شمېر, صحيح isWrite);
فراغ dma_تیاری(فراغ);

// hw/floppy.c
خارجي بنية floppy_ext_dbt_s diskette_param_table2;
فراغ floppy_تیاری(فراغ);
بنية drive_s *init_floppy(صحيح floppyid, صحيح ftype);
صحيح پیدا_کړه_floppy_type(u32 اندازه);
صحيح floppy_process_op(بنية disk_op_s *op);
فراغ floppy_tick(فراغ);

// hw/ramdisk.c
فراغ ramdisk_تیاری(فراغ);
صحيح ramdisk_process_op(بنية disk_op_s *op);

// hw/sdcard.c
صحيح sdcard_process_op(بنية disk_op_s *op);
فراغ sdcard_تیاری(فراغ);

// hw/timer.c
فراغ timer_تیاری(فراغ);
فراغ pmtimer_تیاری(u16 ioport);
فراغ tsctimer_setfreq(u32 khz, ثابت حرف *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
صحيح timer_check(u32 پای_ټکی);
فراغ ndelay(u32 شمېر);
فراغ udelay(u32 شمېر);
فراغ mdelay(u32 شمېر);
فراغ nsleep(u32 شمېر);
فراغ usleep(u32 شمېر);
فراغ msleep(u32 شمېر);
u32 ticks_منزل_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
فراغ pit_تیاری(فراغ);

// jpeg.c
بنية jpeg_decdata *jpeg_alloc(فراغ);
صحيح jpeg_decode(بنية jpeg_decdata *jpeg, بینښه حرف *buf);
فراغ jpeg_get_اندازه(بنية jpeg_decdata *jpeg, صحيح *پلنوالی, صحيح *لوړوالی);
صحيح jpeg_show(بنية jpeg_decdata *jpeg, بینښه حرف *pic, صحيح پلنوالی
              , صحيح لوړوالی, صحيح ژوروالی, صحيح بایټونه_فی_کرښه_dest);

// kbd.c
فراغ kbd_init(فراغ);
فراغ handle_15c2(بنية bregs *regs);
فراغ process_کلی(u8 کلی);
u8 کتار_ته_ورزیاتول_کلی(u16 keycode);
u16 ascii_منزل_keycode(u8 ascii);

// misc.c
خارجي صحيح Have_پرلپسې_Post;
خارجي بنية bios_config_جدول_s BIOS_CONFIG_جدول __aligned(1);
خارجي بنية floppy_dbt_s diskette_param_جدول __aligned(1);
خارجي u8 BiosChecksum;
صحيح دننه_post(فراغ);
فراغ mathcp_تیاری(فراغ);

// mouse.c
فراغ mouse_init(فراغ);
فراغ process_mouse(u8 معلومات);

// optionroms.c
بنية rom_سرلیک;
فراغ callrom(بنية rom_سرلیک *rom, u16 bdf);
فراغ call_bcv(u16 seg, u16 ip);
صحيح is_pci_vga(بنية pci_device *pci);
فراغ optionrom_تیاری(فراغ);
فراغ vgarom_تیاری(فراغ);
فراغ s3_resume_vga(فراغ);
خارجي صحيح ScreenAndDebug;

// pcibios.c
فراغ handle_1ab1(بنية bregs *regs);
فراغ bios32_init(فراغ);

// pmm.c
فراغ pmm_init(فراغ);
فراغ pmm_prepboot(فراغ);

// pnpbios.c
u16 get_pnp_دننه_واټن(فراغ);
فراغ pnp_init(فراغ);

// post.c
فراغ interface_init(فراغ);
فراغ device_hardware_تیاری(فراغ);
فراغ prepareboot(فراغ);
فراغ پیل_Boot(فراغ);
فراغ reloc_preinit(فراغ *f, فراغ *arg);
فراغ کوډ_mutable_preinit(فراغ);

// sercon.c
فراغ sercon_تیاری(فراغ);
فراغ sercon_check_event(فراغ);

// serial.c
فراغ serial_تیاری(فراغ);
فراغ lpt_تیاری(فراغ);

// version.c
خارجي ثابت حرف نسخه[], BUILDINFO[];

// vgahooks.c
فراغ handle_155f(بنية bregs *regs);
فراغ handle_157f(بنية bregs *regs);
فراغ vgahook_تیاری(بنية pci_device *pci);

#د_که_پای // util.h
