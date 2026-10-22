// Misc function and variable declarations.
#اگر_غیر_تعریف_شدہ __UTIL_H
#تعریف_کرو __UTIL_H

#شامل "types.h" // u32

// apm.c
خالی apm_shutdown(خالی);
ساخت bregs;
خالی handle_1553(ساخت bregs *regs);

// bmp.c
ساخت bmp_decdata *bmp_alloc(خالی);
صحیح bmp_decode(ساخت bmp_decdata *bmp, غیرعلامتی حرف *ڈیٹا, صحیح ڈیٹا_سائز);
خالی bmp_get_info(ساخت bmp_decdata *bmp, صحیح *چوڑائی, صحیح *اونچائی, صحیح *bpp);
صحیح bmp_show(ساخت bmp_decdata *bmp, غیرعلامتی حرف *pic, صحیح چوڑائی
             , صحیح اونچائی, صحیح گہرائی, صحیح بائٹیں_فی_سطر_dest);

// boot.c
خالی boot_init(خالی);
خالی boot_جوڑو_bev(u16 seg, u16 bev, u16 desc, صحیح prio);
خالی boot_جوڑو_bcv(u16 seg, u16 ip, u16 desc, صحیح prio);
ساخت drive_s;
خالی boot_جوڑو_floppy(ساخت drive_s *drive_g, ثابت حرف *desc, صحیح prio);
خالی boot_جوڑو_hd(ساخت drive_s *drive_g, ثابت حرف *desc, صحیح prio);
خالی boot_جوڑو_cd(ساخت drive_s *drive_g, ثابت حرف *desc, صحیح prio);
خالی boot_جوڑو_cbfs(خالی *ڈیٹا, ثابت حرف *desc, صحیح prio);
خالی interactive_bootmenu(خالی);
خالی bcv_prepboot(خالی);
u8 is_bootprio_strict(خالی);
ساخت pci_device;
صحیح bootprio_ڈھونڈو_pci_device(ساخت pci_device *pci);
صحیح bootprio_ڈھونڈو_mmio_device(خالی *mmio);
صحیح bootprio_ڈھونڈو_scsi_device(ساخت pci_device *pci, صحیح ہدف, صحیح lun);
صحیح bootprio_ڈھونڈو_scsi_mmio_device(خالی *mmio, صحیح ہدف, صحیح lun);
صحیح bootprio_ڈھونڈو_ata_device(ساخت pci_device *pci, صحیح chanid, صحیح slave);
صحیح bootprio_ڈھونڈو_fdc_device(ساخت pci_device *pci, صحیح پورٹ, صحیح fdid);
صحیح bootprio_ڈھونڈو_pci_rom(ساخت pci_device *pci, صحیح instance);
صحیح bootprio_ڈھونڈو_named_rom(ثابت حرف *name, صحیح instance);
ساخت usbdevice_s;
صحیح bootprio_ڈھونڈو_usb(ساخت usbdevice_s *usbdev, صحیح lun);
صحیح get_keystroke_بھرا(صحیح msec);
صحیح get_keystroke(صحیح msec);
ساخت chs_s;
صحیح boot_lchs_ڈھونڈو_pci_device(ساخت pci_device *pci, ساخت chs_s *chs);
صحیح boot_lchs_ڈھونڈو_scsi_device(ساخت pci_device *pci, صحیح ہدف, صحیح lun,
                               ساخت chs_s *chs);
صحیح boot_lchs_ڈھونڈو_ata_device(ساخت pci_device *pci, صحیح chanid, صحیح slave,
                              ساخت chs_s *chs);

// bootsplash.c
خالی enable_vga_console(خالی);
خالی enable_bootsplash(خالی);
خالی disable_bootsplash(خالی);

// cdrom.c
خارجی ساخت eltorito_s CDEmu;
خارجی ساخت drive_s *cdemu_drive_gf;
ساخت disk_op_s;
صحیح cdemu_process_op(ساخت disk_op_s *op);
خالی cdrom_prepboot(خالی);
صحیح cdrom_boot(ساخت drive_s *drive_g);
حرف *cdrom_media_info(ساخت drive_s *drive_g);

// clock.c
خالی clock_تیاری(خالی);
خالی handle_1583(ساخت bregs *regs);
خالی clock_poll_irq(خالی);
u32 irqtimer_calc_ticks(u32 گنتی);
u32 irqtimer_calc(u32 msecs);
صحیح irqtimer_check(u32 انتہا);
خالی handle_1586(ساخت bregs *regs);

// fw/acpi.c
خالی acpi_تیاری(خالی);

// fw/biostable.c
خالی copy_pir(خالی *pos);
خالی copy_mptable(خالی *pos);
خارجی ساخت pir_سرخی *PirAddr;
خالی copy_acpi_rsdp(خالی *pos);
خارجی ساخت rsdp_descriptor *RsdpAddr;
خارجی u32 acpi_pm1a_cnt;
خارجی u16 acpi_pm_بنیاد;
خالی *ڈھونڈو_acpi_rsdp(خالی);
خالی *ڈھونڈو_acpi_جدول(u32 signature);
u32 ڈھونڈو_resume_سمتیہ(خالی);
خالی acpi_reboot(خالی);
خالی ڈھونڈو_acpi_features(خالی);
خالی *smbios_get_tables(u32 *لمبائی);
خالی copy_smbios_21(خالی *pos);
خالی display_uuid(خالی);
خالی copy_جدول(خالی *pos);
خالی smbios_تیاری(خالی);

// fw/dsdt_parser.c
ساخت acpi_device;
خالی acpi_dsdt_parse(خالی);
ساخت acpi_device *acpi_dsdt_ڈھونڈو_string(ساخت acpi_device *پچھلا, ثابت حرف *hid);
ساخت acpi_device *acpi_dsdt_ڈھونڈو_eisaid(ساخت acpi_device *پچھلا, u16 eisaid);
حرف *acpi_dsdt_name(ساخت acpi_device *dev);
صحیح acpi_dsdt_present_eisaid(u16 eisaid);
صحیح acpi_dsdt_ڈھونڈو_io(ساخت acpi_device *dev, u64 *min, u64 *max);
صحیح acpi_dsdt_ڈھونڈو_mem(ساخت acpi_device *dev, u64 *min, u64 *max);
صحیح acpi_dsdt_ڈھونڈو_irq(ساخت acpi_device *dev, u64 *irq);

// fw/coreboot.c
خارجی ثابت حرف *CBvendor, *CBpart;
ساخت cbfs_file;
خالی coreboot_debug_putc(حرف c);
خالی cbfs_تسلسل_payload(ساخت cbfs_file *file);
خالی coreboot_platform_تیاری(خالی);
خالی cbfs_payload_تیاری(خالی);
خالی coreboot_preinit(خالی);
خالی coreboot_cbfs_init(خالی);
ساخت cb_سرخی;
خالی *ڈھونڈو_cb_subtable(ساخت cb_سرخی *cbh, u32 لیبل);
ساخت cb_سرخی *ڈھونڈو_cb_جدول(خالی);

// fw/csm.c
صحیح csm_bootprio_fdc(ساخت pci_device *pci, صحیح پورٹ, صحیح fdid);
صحیح csm_bootprio_ata(ساخت pci_device *pci, صحیح chanid, صحیح slave);
صحیح csm_bootprio_pci(ساخت pci_device *pci);

// fw/mptable.c
خالی mptable_تیاری(خالی);

// fw/mtrr.c
خالی mtrr_تیاری(خالی);

// fw/multiboot.c
خالی multiboot_init(خالی);

// fw/pciinit.c
خارجی u64 pcimem_آغاز, pcimem_انتہا;
خارجی u64 pcimem64_آغاز, pcimem64_انتہا;
خارجی ثابت u8 pci_irqs[4];
خالی pci_تیاری(خالی);
خالی pci_resume(خالی);

// fw/pirtable.c
خالی pirtable_تیاری(خالی);

// fw/shadow.c
خالی make_bios_writable(خالی);
خالی make_bios_readonly(خالی);
خالی qemu_reboot(خالی);

// fw/smbios.c
خالی smbios_legacy_تیاری(خالی);

// fw/smm.c
خالی smm_device_تیاری(خالی);
خالی smm_تیاری(خالی);

// fw/smp.c
خارجی u32 Max_گنتی_CPUs;
خالی wrmsr_smp(u32 اشاریہ, u64 val);
خالی smp_تیاری(خالی);
خالی smp_resume(خالی);
صحیح apic_id_is_present(u8 apic_id);

// hw/dma.c
صحیح dma_floppy(u32 addr, صحیح گنتی, صحیح isWrite);
خالی dma_تیاری(خالی);

// hw/floppy.c
خارجی ساخت floppy_ext_dbt_s diskette_param_table2;
خالی floppy_تیاری(خالی);
ساخت drive_s *init_floppy(صحیح floppyid, صحیح ftype);
صحیح ڈھونڈو_floppy_type(u32 سائز);
صحیح floppy_process_op(ساخت disk_op_s *op);
خالی floppy_tick(خالی);

// hw/ramdisk.c
خالی ramdisk_تیاری(خالی);
صحیح ramdisk_process_op(ساخت disk_op_s *op);

// hw/sdcard.c
صحیح sdcard_process_op(ساخت disk_op_s *op);
خالی sdcard_تیاری(خالی);

// hw/timer.c
خالی timer_تیاری(خالی);
خالی pmtimer_تیاری(u16 ioport);
خالی tsctimer_setfreq(u32 khz, ثابت حرف *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
صحیح timer_check(u32 انتہا);
خالی ndelay(u32 گنتی);
خالی udelay(u32 گنتی);
خالی mdelay(u32 گنتی);
خالی nsleep(u32 گنتی);
خالی usleep(u32 گنتی);
خالی msleep(u32 گنتی);
u32 ticks_مقصد_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
خالی pit_تیاری(خالی);

// jpeg.c
ساخت jpeg_decdata *jpeg_alloc(خالی);
صحیح jpeg_decode(ساخت jpeg_decdata *jpeg, غیرعلامتی حرف *buf);
خالی jpeg_get_سائز(ساخت jpeg_decdata *jpeg, صحیح *چوڑائی, صحیح *اونچائی);
صحیح jpeg_show(ساخت jpeg_decdata *jpeg, غیرعلامتی حرف *pic, صحیح چوڑائی
              , صحیح اونچائی, صحیح گہرائی, صحیح بائٹیں_فی_سطر_dest);

// kbd.c
خالی kbd_init(خالی);
خالی handle_15c2(ساخت bregs *regs);
خالی process_کلید(u8 کلید);
u8 قطار_میں_ڈالو_کلید(u16 keycode);
u16 ascii_مقصد_keycode(u8 ascii);

// misc.c
خارجی صحیح Have_تسلسل_Post;
خارجی ساخت bios_config_جدول_s BIOS_CONFIG_جدول __aligned(1);
خارجی ساخت floppy_dbt_s diskette_param_جدول __aligned(1);
خارجی u8 BiosChecksum;
صحیح اندر_post(خالی);
خالی mathcp_تیاری(خالی);

// mouse.c
خالی mouse_init(خالی);
خالی process_mouse(u8 ڈیٹا);

// optionroms.c
ساخت rom_سرخی;
خالی callrom(ساخت rom_سرخی *rom, u16 bdf);
خالی call_bcv(u16 seg, u16 ip);
صحیح is_pci_vga(ساخت pci_device *pci);
خالی optionrom_تیاری(خالی);
خالی vgarom_تیاری(خالی);
خالی s3_resume_vga(خالی);
خارجی صحیح ScreenAndDebug;

// pcibios.c
خالی handle_1ab1(ساخت bregs *regs);
خالی bios32_init(خالی);

// pmm.c
خالی pmm_init(خالی);
خالی pmm_prepboot(خالی);

// pnpbios.c
u16 get_pnp_انحراف(خالی);
خالی pnp_init(خالی);

// post.c
خالی interface_init(خالی);
خالی device_hardware_تیاری(خالی);
خالی prepareboot(خالی);
خالی آغاز_Boot(خالی);
خالی reloc_preinit(خالی *f, خالی *arg);
خالی رمز_mutable_preinit(خالی);

// sercon.c
خالی sercon_تیاری(خالی);
خالی sercon_check_event(خالی);

// serial.c
خالی serial_تیاری(خالی);
خالی lpt_تیاری(خالی);

// version.c
خارجی ثابت حرف نسخہ[], BUILDINFO[];

// vgahooks.c
خالی handle_155f(ساخت bregs *regs);
خالی handle_157f(ساخت bregs *regs);
خالی vgahook_تیاری(ساخت pci_device *pci);

#اگر_ختم // util.h
