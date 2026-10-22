// Misc function and variable declarations.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __UTIL_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __UTIL_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

// apm.c
ਖਾਲੀ apm_shutdown(ਖਾਲੀ);
ਬਣਤਰ bregs;
ਖਾਲੀ handle_1553(ਬਣਤਰ bregs *regs);

// bmp.c
ਬਣਤਰ bmp_decdata *bmp_alloc(ਖਾਲੀ);
ਪੂਰਨ bmp_decode(ਬਣਤਰ bmp_decdata *bmp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅੰਕੜੇ, ਪੂਰਨ ਅੰਕੜੇ_ਮਾਪ);
ਖਾਲੀ bmp_get_info(ਬਣਤਰ bmp_decdata *bmp, ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ, ਪੂਰਨ *bpp);
ਪੂਰਨ bmp_show(ਬਣਤਰ bmp_decdata *bmp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic, ਪੂਰਨ ਚੌੜਾਈ
             , ਪੂਰਨ ਉਚਾਈ, ਪੂਰਨ ਡੂੰਘਾਈ, ਪੂਰਨ ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest);

// boot.c
ਖਾਲੀ boot_init(ਖਾਲੀ);
ਖਾਲੀ boot_ਜੋੜੋ_bev(u16 seg, u16 bev, u16 desc, ਪੂਰਨ prio);
ਖਾਲੀ boot_ਜੋੜੋ_bcv(u16 seg, u16 ip, u16 desc, ਪੂਰਨ prio);
ਬਣਤਰ drive_s;
ਖਾਲੀ boot_ਜੋੜੋ_floppy(ਬਣਤਰ drive_s *drive_g, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio);
ਖਾਲੀ boot_ਜੋੜੋ_hd(ਬਣਤਰ drive_s *drive_g, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio);
ਖਾਲੀ boot_ਜੋੜੋ_cd(ਬਣਤਰ drive_s *drive_g, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio);
ਖਾਲੀ boot_ਜੋੜੋ_cbfs(ਖਾਲੀ *ਅੰਕੜੇ, ਅਚਲ ਅੱਖਰ *desc, ਪੂਰਨ prio);
ਖਾਲੀ interactive_bootmenu(ਖਾਲੀ);
ਖਾਲੀ bcv_prepboot(ਖਾਲੀ);
u8 is_bootprio_strict(ਖਾਲੀ);
ਬਣਤਰ pci_device;
ਪੂਰਨ bootprio_ਲੱਭੋ_pci_device(ਬਣਤਰ pci_device *pci);
ਪੂਰਨ bootprio_ਲੱਭੋ_mmio_device(ਖਾਲੀ *mmio);
ਪੂਰਨ bootprio_ਲੱਭੋ_scsi_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਨਿਸ਼ਾਨਾ, ਪੂਰਨ lun);
ਪੂਰਨ bootprio_ਲੱਭੋ_scsi_mmio_device(ਖਾਲੀ *mmio, ਪੂਰਨ ਨਿਸ਼ਾਨਾ, ਪੂਰਨ lun);
ਪੂਰਨ bootprio_ਲੱਭੋ_ata_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ slave);
ਪੂਰਨ bootprio_ਲੱਭੋ_fdc_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ fdid);
ਪੂਰਨ bootprio_ਲੱਭੋ_pci_rom(ਬਣਤਰ pci_device *pci, ਪੂਰਨ instance);
ਪੂਰਨ bootprio_ਲੱਭੋ_named_rom(ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ instance);
ਬਣਤਰ usbdevice_s;
ਪੂਰਨ bootprio_ਲੱਭੋ_usb(ਬਣਤਰ usbdevice_s *usbdev, ਪੂਰਨ lun);
ਪੂਰਨ get_keystroke_ਭਰਿਆ(ਪੂਰਨ msec);
ਪੂਰਨ get_keystroke(ਪੂਰਨ msec);
ਬਣਤਰ chs_s;
ਪੂਰਨ boot_lchs_ਲੱਭੋ_pci_device(ਬਣਤਰ pci_device *pci, ਬਣਤਰ chs_s *chs);
ਪੂਰਨ boot_lchs_ਲੱਭੋ_scsi_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਨਿਸ਼ਾਨਾ, ਪੂਰਨ lun,
                               ਬਣਤਰ chs_s *chs);
ਪੂਰਨ boot_lchs_ਲੱਭੋ_ata_device(ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ slave,
                              ਬਣਤਰ chs_s *chs);

// bootsplash.c
ਖਾਲੀ enable_vga_console(ਖਾਲੀ);
ਖਾਲੀ enable_bootsplash(ਖਾਲੀ);
ਖਾਲੀ disable_bootsplash(ਖਾਲੀ);

// cdrom.c
ਬਾਹਰੀ ਬਣਤਰ eltorito_s CDEmu;
ਬਾਹਰੀ ਬਣਤਰ drive_s *cdemu_drive_gf;
ਬਣਤਰ disk_op_s;
ਪੂਰਨ cdemu_process_op(ਬਣਤਰ disk_op_s *op);
ਖਾਲੀ cdrom_prepboot(ਖਾਲੀ);
ਪੂਰਨ cdrom_boot(ਬਣਤਰ drive_s *drive_g);
ਅੱਖਰ *cdrom_media_info(ਬਣਤਰ drive_s *drive_g);

// clock.c
ਖਾਲੀ clock_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ handle_1583(ਬਣਤਰ bregs *regs);
ਖਾਲੀ clock_poll_irq(ਖਾਲੀ);
u32 irqtimer_calc_ticks(u32 ਗਿਣਤੀ_2);
u32 irqtimer_calc(u32 msecs);
ਪੂਰਨ irqtimer_check(u32 ਸਿਰਾ);
ਖਾਲੀ handle_1586(ਬਣਤਰ bregs *regs);

// fw/acpi.c
ਖਾਲੀ acpi_ਤਿਆਰੀ(ਖਾਲੀ);

// fw/biostable.c
ਖਾਲੀ copy_pir(ਖਾਲੀ *pos);
ਖਾਲੀ copy_mptable(ਖਾਲੀ *pos);
ਬਾਹਰੀ ਬਣਤਰ pir_ਸਿਰਲੇਖ *PirAddr;
ਖਾਲੀ copy_acpi_rsdp(ਖਾਲੀ *pos);
ਬਾਹਰੀ ਬਣਤਰ rsdp_descriptor *RsdpAddr;
ਬਾਹਰੀ u32 acpi_pm1a_cnt;
ਬਾਹਰੀ u16 acpi_pm_ਅਧਾਰ;
ਖਾਲੀ *ਲੱਭੋ_acpi_rsdp(ਖਾਲੀ);
ਖਾਲੀ *ਲੱਭੋ_acpi_ਸਾਰਣੀ(u32 signature);
u32 ਲੱਭੋ_resume_ਸਦਿਸ਼(ਖਾਲੀ);
ਖਾਲੀ acpi_reboot(ਖਾਲੀ);
ਖਾਲੀ ਲੱਭੋ_acpi_features(ਖਾਲੀ);
ਖਾਲੀ *smbios_get_tables(u32 *ਲੰਮਾਈ);
ਖਾਲੀ copy_smbios_21(ਖਾਲੀ *pos);
ਖਾਲੀ display_uuid(ਖਾਲੀ);
ਖਾਲੀ copy_ਸਾਰਣੀ(ਖਾਲੀ *pos);
ਖਾਲੀ smbios_ਤਿਆਰੀ(ਖਾਲੀ);

// fw/dsdt_parser.c
ਬਣਤਰ acpi_device;
ਖਾਲੀ acpi_dsdt_parse(ਖਾਲੀ);
ਬਣਤਰ acpi_device *acpi_dsdt_ਲੱਭੋ_string(ਬਣਤਰ acpi_device *prev, ਅਚਲ ਅੱਖਰ *hid);
ਬਣਤਰ acpi_device *acpi_dsdt_ਲੱਭੋ_eisaid(ਬਣਤਰ acpi_device *prev, u16 eisaid);
ਅੱਖਰ *acpi_dsdt_name(ਬਣਤਰ acpi_device *dev);
ਪੂਰਨ acpi_dsdt_present_eisaid(u16 eisaid);
ਪੂਰਨ acpi_dsdt_ਲੱਭੋ_io(ਬਣਤਰ acpi_device *dev, u64 *min, u64 *max);
ਪੂਰਨ acpi_dsdt_ਲੱਭੋ_mem(ਬਣਤਰ acpi_device *dev, u64 *min, u64 *max);
ਪੂਰਨ acpi_dsdt_ਲੱਭੋ_irq(ਬਣਤਰ acpi_device *dev, u64 *irq);

// fw/coreboot.c
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *CBvendor, *CBpart;
ਬਣਤਰ cbfs_file;
ਖਾਲੀ coreboot_debug_putc(ਅੱਖਰ c);
ਖਾਲੀ cbfs_ਲੜੀਵਾਰ_payload(ਬਣਤਰ cbfs_file *file);
ਖਾਲੀ coreboot_platform_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ cbfs_payload_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ coreboot_preinit(ਖਾਲੀ);
ਖਾਲੀ coreboot_cbfs_init(ਖਾਲੀ);
ਬਣਤਰ cb_ਸਿਰਲੇਖ;
ਖਾਲੀ *ਲੱਭੋ_cb_subtable(ਬਣਤਰ cb_ਸਿਰਲੇਖ *cbh, u32 ਟੈਗ);
ਬਣਤਰ cb_ਸਿਰਲੇਖ *ਲੱਭੋ_cb_ਸਾਰਣੀ(ਖਾਲੀ);

// fw/csm.c
ਪੂਰਨ csm_bootprio_fdc(ਬਣਤਰ pci_device *pci, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ fdid);
ਪੂਰਨ csm_bootprio_ata(ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ slave);
ਪੂਰਨ csm_bootprio_pci(ਬਣਤਰ pci_device *pci);

// fw/mptable.c
ਖਾਲੀ mptable_ਤਿਆਰੀ(ਖਾਲੀ);

// fw/mtrr.c
ਖਾਲੀ mtrr_ਤਿਆਰੀ(ਖਾਲੀ);

// fw/multiboot.c
ਖਾਲੀ multiboot_init(ਖਾਲੀ);

// fw/pciinit.c
ਬਾਹਰੀ u64 pcimem_ਸ਼ੁਰੂ, pcimem_ਸਿਰਾ;
ਬਾਹਰੀ u64 pcimem64_ਸ਼ੁਰੂ, pcimem64_ਸਿਰਾ;
ਬਾਹਰੀ ਅਚਲ u8 pci_irqs[4];
ਖਾਲੀ pci_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ pci_resume(ਖਾਲੀ);

// fw/pirtable.c
ਖਾਲੀ pirtable_ਤਿਆਰੀ(ਖਾਲੀ);

// fw/shadow.c
ਖਾਲੀ make_bios_writable(ਖਾਲੀ);
ਖਾਲੀ make_bios_readonly(ਖਾਲੀ);
ਖਾਲੀ qemu_reboot(ਖਾਲੀ);

// fw/smbios.c
ਖਾਲੀ smbios_legacy_ਤਿਆਰੀ(ਖਾਲੀ);

// fw/smm.c
ਖਾਲੀ smm_device_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ smm_ਤਿਆਰੀ(ਖਾਲੀ);

// fw/smp.c
ਬਾਹਰੀ u32 Max_ਗਿਣਤੀ_CPUs;
ਖਾਲੀ wrmsr_smp(u32 ਸੂਚਕਾਂਕ, u64 val);
ਖਾਲੀ smp_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ smp_resume(ਖਾਲੀ);
ਪੂਰਨ apic_id_is_present(u8 apic_id);

// hw/dma.c
ਪੂਰਨ dma_floppy(u32 addr, ਪੂਰਨ ਗਿਣਤੀ_2, ਪੂਰਨ isWrite);
ਖਾਲੀ dma_ਤਿਆਰੀ(ਖਾਲੀ);

// hw/floppy.c
ਬਾਹਰੀ ਬਣਤਰ floppy_ext_dbt_s diskette_param_table2;
ਖਾਲੀ floppy_ਤਿਆਰੀ(ਖਾਲੀ);
ਬਣਤਰ drive_s *init_floppy(ਪੂਰਨ floppyid, ਪੂਰਨ ftype);
ਪੂਰਨ ਲੱਭੋ_floppy_type(u32 ਮਾਪ);
ਪੂਰਨ floppy_process_op(ਬਣਤਰ disk_op_s *op);
ਖਾਲੀ floppy_tick(ਖਾਲੀ);

// hw/ramdisk.c
ਖਾਲੀ ramdisk_ਤਿਆਰੀ(ਖਾਲੀ);
ਪੂਰਨ ramdisk_process_op(ਬਣਤਰ disk_op_s *op);

// hw/sdcard.c
ਪੂਰਨ sdcard_process_op(ਬਣਤਰ disk_op_s *op);
ਖਾਲੀ sdcard_ਤਿਆਰੀ(ਖਾਲੀ);

// hw/timer.c
ਖਾਲੀ timer_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ pmtimer_ਤਿਆਰੀ(u16 ioport);
ਖਾਲੀ tsctimer_setfreq(u32 khz, ਅਚਲ ਅੱਖਰ *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
ਪੂਰਨ timer_check(u32 ਸਿਰਾ);
ਖਾਲੀ ndelay(u32 ਗਿਣਤੀ_2);
ਖਾਲੀ udelay(u32 ਗਿਣਤੀ_2);
ਖਾਲੀ mdelay(u32 ਗਿਣਤੀ_2);
ਖਾਲੀ nsleep(u32 ਗਿਣਤੀ_2);
ਖਾਲੀ usleep(u32 ਗਿਣਤੀ_2);
ਖਾਲੀ msleep(u32 ਗਿਣਤੀ_2);
u32 ticks_ਮੰਜ਼ਿਲ_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
ਖਾਲੀ pit_ਤਿਆਰੀ(ਖਾਲੀ);

// jpeg.c
ਬਣਤਰ jpeg_decdata *jpeg_alloc(ਖਾਲੀ);
ਪੂਰਨ jpeg_decode(ਬਣਤਰ jpeg_decdata *jpeg, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf);
ਖਾਲੀ jpeg_get_ਮਾਪ(ਬਣਤਰ jpeg_decdata *jpeg, ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ);
ਪੂਰਨ jpeg_show(ਬਣਤਰ jpeg_decdata *jpeg, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic, ਪੂਰਨ ਚੌੜਾਈ
              , ਪੂਰਨ ਉਚਾਈ, ਪੂਰਨ ਡੂੰਘਾਈ, ਪੂਰਨ ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest);

// kbd.c
ਖਾਲੀ kbd_init(ਖਾਲੀ);
ਖਾਲੀ handle_15c2(ਬਣਤਰ bregs *regs);
ਖਾਲੀ process_ਕੁੰਜੀ(u8 ਕੁੰਜੀ);
u8 ਕਤਾਰ_ਵਿੱਚ_ਪਾਓ_ਕੁੰਜੀ(u16 keycode);
u16 ascii_ਮੰਜ਼ਿਲ_keycode(u8 ascii);

// misc.c
ਬਾਹਰੀ ਪੂਰਨ Have_ਲੜੀਵਾਰ_Post;
ਬਾਹਰੀ ਬਣਤਰ bios_config_ਸਾਰਣੀ_s BIOS_CONFIG_ਸਾਰਣੀ __aligned(1);
ਬਾਹਰੀ ਬਣਤਰ floppy_dbt_s diskette_param_ਸਾਰਣੀ __aligned(1);
ਬਾਹਰੀ u8 BiosChecksum;
ਪੂਰਨ ਵਿੱਚ_post(ਖਾਲੀ);
ਖਾਲੀ mathcp_ਤਿਆਰੀ(ਖਾਲੀ);

// mouse.c
ਖਾਲੀ mouse_init(ਖਾਲੀ);
ਖਾਲੀ process_mouse(u8 ਅੰਕੜੇ);

// optionroms.c
ਬਣਤਰ rom_ਸਿਰਲੇਖ;
ਖਾਲੀ callrom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom, u16 bdf);
ਖਾਲੀ call_bcv(u16 seg, u16 ip);
ਪੂਰਨ is_pci_vga(ਬਣਤਰ pci_device *pci);
ਖਾਲੀ optionrom_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ vgarom_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ s3_resume_vga(ਖਾਲੀ);
ਬਾਹਰੀ ਪੂਰਨ ScreenAndDebug;

// pcibios.c
ਖਾਲੀ handle_1ab1(ਬਣਤਰ bregs *regs);
ਖਾਲੀ bios32_init(ਖਾਲੀ);

// pmm.c
ਖਾਲੀ pmm_init(ਖਾਲੀ);
ਖਾਲੀ pmm_prepboot(ਖਾਲੀ);

// pnpbios.c
u16 get_pnp_ਵਿੱਥ(ਖਾਲੀ);
ਖਾਲੀ pnp_init(ਖਾਲੀ);

// post.c
ਖਾਲੀ interface_init(ਖਾਲੀ);
ਖਾਲੀ device_hardware_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ prepareboot(ਖਾਲੀ);
ਖਾਲੀ ਸ਼ੁਰੂ_Boot(ਖਾਲੀ);
ਖਾਲੀ reloc_preinit(ਖਾਲੀ *f, ਖਾਲੀ *arg);
ਖਾਲੀ ਕੋਡ_mutable_preinit(ਖਾਲੀ);

// sercon.c
ਖਾਲੀ sercon_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ sercon_check_event(ਖਾਲੀ);

// serial.c
ਖਾਲੀ serial_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ lpt_ਤਿਆਰੀ(ਖਾਲੀ);

// version.c
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ ਸੰਸਕਰਣ[], BUILDINFO[];

// vgahooks.c
ਖਾਲੀ handle_155f(ਬਣਤਰ bregs *regs);
ਖਾਲੀ handle_157f(ਬਣਤਰ bregs *regs);
ਖਾਲੀ vgahook_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci);

#ਜੇ_ਅੰਤ // util.h
