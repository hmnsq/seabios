// Misc function and variable declarations.
#నిర్వచితం_కాకపోతే __UTIL_H
#నిర్వచించు __UTIL_H

#అన్తర్భావ "types.h" // u32

// apm.c
శూన్య apm_shutdown(శూన్య);
నిర్మాణం bregs;
శూన్య handle_1553(నిర్మాణం bregs *regs);

// bmp.c
నిర్మాణం bmp_decdata *bmp_alloc(శూన్య);
పూర్ణాంకం bmp_decode(నిర్మాణం bmp_decdata *bmp, గుర్తులేని అక్షరం *దత్తాంశం, పూర్ణాంకం దత్తాంశం_కొలత);
శూన్య bmp_get_info(నిర్మాణం bmp_decdata *bmp, పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు, పూర్ణాంకం *bpp);
పూర్ణాంకం bmp_show(నిర్మాణం bmp_decdata *bmp, గుర్తులేని అక్షరం *pic, పూర్ణాంకం వెడల్పు
             , పూర్ణాంకం ఎత్తు, పూర్ణాంకం లోతు, పూర్ణాంకం బైట్లు_చొప్పున_పంక్తి_dest);

// boot.c
శూన్య boot_init(శూన్య);
శూన్య boot_కూడు_bev(u16 seg, u16 bev, u16 desc, పూర్ణాంకం prio);
శూన్య boot_కూడు_bcv(u16 seg, u16 ip, u16 desc, పూర్ణాంకం prio);
నిర్మాణం drive_s;
శూన్య boot_కూడు_floppy(నిర్మాణం drive_s *drive_g, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio);
శూన్య boot_కూడు_hd(నిర్మాణం drive_s *drive_g, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio);
శూన్య boot_కూడు_cd(నిర్మాణం drive_s *drive_g, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio);
శూన్య boot_కూడు_cbfs(శూన్య *దత్తాంశం, స్థిరరాశి అక్షరం *desc, పూర్ణాంకం prio);
శూన్య interactive_bootmenu(శూన్య);
శూన్య bcv_prepboot(శూన్య);
u8 is_bootprio_strict(శూన్య);
నిర్మాణం pci_device;
పూర్ణాంకం bootprio_కనుగొను_pci_device(నిర్మాణం pci_device *pci);
పూర్ణాంకం bootprio_కనుగొను_mmio_device(శూన్య *mmio);
పూర్ణాంకం bootprio_కనుగొను_scsi_device(నిర్మాణం pci_device *pci, పూర్ణాంకం లక్ష్యం, పూర్ణాంకం lun);
పూర్ణాంకం bootprio_కనుగొను_scsi_mmio_device(శూన్య *mmio, పూర్ణాంకం లక్ష్యం, పూర్ణాంకం lun);
పూర్ణాంకం bootprio_కనుగొను_ata_device(నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం slave);
పూర్ణాంకం bootprio_కనుగొను_fdc_device(నిర్మాణం pci_device *pci, పూర్ణాంకం పోర్టు, పూర్ణాంకం fdid);
పూర్ణాంకం bootprio_కనుగొను_pci_rom(నిర్మాణం pci_device *pci, పూర్ణాంకం instance);
పూర్ణాంకం bootprio_కనుగొను_named_rom(స్థిరరాశి అక్షరం *name, పూర్ణాంకం instance);
నిర్మాణం usbdevice_s;
పూర్ణాంకం bootprio_కనుగొను_usb(నిర్మాణం usbdevice_s *usbdev, పూర్ణాంకం lun);
పూర్ణాంకం get_keystroke_నిండిన(పూర్ణాంకం msec);
పూర్ణాంకం get_keystroke(పూర్ణాంకం msec);
నిర్మాణం chs_s;
పూర్ణాంకం boot_lchs_కనుగొను_pci_device(నిర్మాణం pci_device *pci, నిర్మాణం chs_s *chs);
పూర్ణాంకం boot_lchs_కనుగొను_scsi_device(నిర్మాణం pci_device *pci, పూర్ణాంకం లక్ష్యం, పూర్ణాంకం lun,
                               నిర్మాణం chs_s *chs);
పూర్ణాంకం boot_lchs_కనుగొను_ata_device(నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం slave,
                              నిర్మాణం chs_s *chs);

// bootsplash.c
శూన్య enable_vga_console(శూన్య);
శూన్య enable_bootsplash(శూన్య);
శూన్య disable_bootsplash(శూన్య);

// cdrom.c
బాహ్య నిర్మాణం eltorito_s CDEmu;
బాహ్య నిర్మాణం drive_s *cdemu_drive_gf;
నిర్మాణం disk_op_s;
పూర్ణాంకం cdemu_process_op(నిర్మాణం disk_op_s *op);
శూన్య cdrom_prepboot(శూన్య);
పూర్ణాంకం cdrom_boot(నిర్మాణం drive_s *drive_g);
అక్షరం *cdrom_media_info(నిర్మాణం drive_s *drive_g);

// clock.c
శూన్య clock_అమరిక(శూన్య);
శూన్య handle_1583(నిర్మాణం bregs *regs);
శూన్య clock_poll_irq(శూన్య);
u32 irqtimer_calc_ticks(u32 లెక్క);
u32 irqtimer_calc(u32 msecs);
పూర్ణాంకం irqtimer_check(u32 చివర);
శూన్య handle_1586(నిర్మాణం bregs *regs);

// fw/acpi.c
శూన్య acpi_అమరిక(శూన్య);

// fw/biostable.c
శూన్య copy_pir(శూన్య *pos);
శూన్య copy_mptable(శూన్య *pos);
బాహ్య నిర్మాణం pir_శీర్షిక *PirAddr;
శూన్య copy_acpi_rsdp(శూన్య *pos);
బాహ్య నిర్మాణం rsdp_descriptor *RsdpAddr;
బాహ్య u32 acpi_pm1a_cnt;
బాహ్య u16 acpi_pm_పునాది;
శూన్య *కనుగొను_acpi_rsdp(శూన్య);
శూన్య *కనుగొను_acpi_పట్టిక(u32 signature);
u32 కనుగొను_resume_సదిశ(శూన్య);
శూన్య acpi_reboot(శూన్య);
శూన్య కనుగొను_acpi_features(శూన్య);
శూన్య *smbios_get_tables(u32 *నిడివి);
శూన్య copy_smbios_21(శూన్య *pos);
శూన్య display_uuid(శూన్య);
శూన్య copy_పట్టిక(శూన్య *pos);
శూన్య smbios_అమరిక(శూన్య);

// fw/dsdt_parser.c
నిర్మాణం acpi_device;
శూన్య acpi_dsdt_parse(శూన్య);
నిర్మాణం acpi_device *acpi_dsdt_కనుగొను_string(నిర్మాణం acpi_device *prev, స్థిరరాశి అక్షరం *hid);
నిర్మాణం acpi_device *acpi_dsdt_కనుగొను_eisaid(నిర్మాణం acpi_device *prev, u16 eisaid);
అక్షరం *acpi_dsdt_name(నిర్మాణం acpi_device *dev);
పూర్ణాంకం acpi_dsdt_present_eisaid(u16 eisaid);
పూర్ణాంకం acpi_dsdt_కనుగొను_io(నిర్మాణం acpi_device *dev, u64 *min, u64 *max);
పూర్ణాంకం acpi_dsdt_కనుగొను_mem(నిర్మాణం acpi_device *dev, u64 *min, u64 *max);
పూర్ణాంకం acpi_dsdt_కనుగొను_irq(నిర్మాణం acpi_device *dev, u64 *irq);

// fw/coreboot.c
బాహ్య స్థిరరాశి అక్షరం *CBvendor, *CBpart;
నిర్మాణం cbfs_file;
శూన్య coreboot_debug_putc(అక్షరం c);
శూన్య cbfs_నిరంతరం_payload(నిర్మాణం cbfs_file *file);
శూన్య coreboot_platform_అమరిక(శూన్య);
శూన్య cbfs_payload_అమరిక(శూన్య);
శూన్య coreboot_preinit(శూన్య);
శూన్య coreboot_cbfs_init(శూన్య);
నిర్మాణం cb_శీర్షిక;
శూన్య *కనుగొను_cb_subtable(నిర్మాణం cb_శీర్షిక *cbh, u32 ట్యాగ్);
నిర్మాణం cb_శీర్షిక *కనుగొను_cb_పట్టిక(శూన్య);

// fw/csm.c
పూర్ణాంకం csm_bootprio_fdc(నిర్మాణం pci_device *pci, పూర్ణాంకం పోర్టు, పూర్ణాంకం fdid);
పూర్ణాంకం csm_bootprio_ata(నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం slave);
పూర్ణాంకం csm_bootprio_pci(నిర్మాణం pci_device *pci);

// fw/mptable.c
శూన్య mptable_అమరిక(శూన్య);

// fw/mtrr.c
శూన్య mtrr_అమరిక(శూన్య);

// fw/multiboot.c
శూన్య multiboot_init(శూన్య);

// fw/pciinit.c
బాహ్య u64 pcimem_ప్రారంభం, pcimem_చివర;
బాహ్య u64 pcimem64_ప్రారంభం, pcimem64_చివర;
బాహ్య స్థిరరాశి u8 pci_irqs[4];
శూన్య pci_అమరిక(శూన్య);
శూన్య pci_resume(శూన్య);

// fw/pirtable.c
శూన్య pirtable_అమరిక(శూన్య);

// fw/shadow.c
శూన్య make_bios_writable(శూన్య);
శూన్య make_bios_readonly(శూన్య);
శూన్య qemu_reboot(శూన్య);

// fw/smbios.c
శూన్య smbios_legacy_అమరిక(శూన్య);

// fw/smm.c
శూన్య smm_device_అమరిక(శూన్య);
శూన్య smm_అమరిక(శూన్య);

// fw/smp.c
బాహ్య u32 Max_లెక్క_CPUs;
శూన్య wrmsr_smp(u32 సూచీ, u64 val);
శూన్య smp_అమరిక(శూన్య);
శూన్య smp_resume(శూన్య);
పూర్ణాంకం apic_id_is_present(u8 apic_id);

// hw/dma.c
పూర్ణాంకం dma_floppy(u32 addr, పూర్ణాంకం లెక్క, పూర్ణాంకం isWrite);
శూన్య dma_అమరిక(శూన్య);

// hw/floppy.c
బాహ్య నిర్మాణం floppy_ext_dbt_s diskette_param_table2;
శూన్య floppy_అమరిక(శూన్య);
నిర్మాణం drive_s *init_floppy(పూర్ణాంకం floppyid, పూర్ణాంకం ftype);
పూర్ణాంకం కనుగొను_floppy_type(u32 కొలత);
పూర్ణాంకం floppy_process_op(నిర్మాణం disk_op_s *op);
శూన్య floppy_tick(శూన్య);

// hw/ramdisk.c
శూన్య ramdisk_అమరిక(శూన్య);
పూర్ణాంకం ramdisk_process_op(నిర్మాణం disk_op_s *op);

// hw/sdcard.c
పూర్ణాంకం sdcard_process_op(నిర్మాణం disk_op_s *op);
శూన్య sdcard_అమరిక(శూన్య);

// hw/timer.c
శూన్య timer_అమరిక(శూన్య);
శూన్య pmtimer_అమరిక(u16 ioport);
శూన్య tsctimer_setfreq(u32 khz, స్థిరరాశి అక్షరం *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
పూర్ణాంకం timer_check(u32 చివర);
శూన్య ndelay(u32 లెక్క);
శూన్య udelay(u32 లెక్క);
శూన్య mdelay(u32 లెక్క);
శూన్య nsleep(u32 లెక్క);
శూన్య usleep(u32 లెక్క);
శూన్య msleep(u32 లెక్క);
u32 ticks_గమ్యం_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
శూన్య pit_అమరిక(శూన్య);

// jpeg.c
నిర్మాణం jpeg_decdata *jpeg_alloc(శూన్య);
పూర్ణాంకం jpeg_decode(నిర్మాణం jpeg_decdata *jpeg, గుర్తులేని అక్షరం *buf);
శూన్య jpeg_get_కొలత(నిర్మాణం jpeg_decdata *jpeg, పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు);
పూర్ణాంకం jpeg_show(నిర్మాణం jpeg_decdata *jpeg, గుర్తులేని అక్షరం *pic, పూర్ణాంకం వెడల్పు
              , పూర్ణాంకం ఎత్తు, పూర్ణాంకం లోతు, పూర్ణాంకం బైట్లు_చొప్పున_పంక్తి_dest);

// kbd.c
శూన్య kbd_init(శూన్య);
శూన్య handle_15c2(నిర్మాణం bregs *regs);
శూన్య process_కీలకం(u8 కీలకం);
u8 వరుసలో_చేర్చు_కీలకం(u16 keycode);
u16 ascii_గమ్యం_keycode(u8 ascii);

// misc.c
బాహ్య పూర్ణాంకం Have_నిరంతరం_Post;
బాహ్య నిర్మాణం bios_config_పట్టిక_s BIOS_CONFIG_పట్టిక __aligned(1);
బాహ్య నిర్మాణం floppy_dbt_s diskette_param_పట్టిక __aligned(1);
బాహ్య u8 BiosChecksum;
పూర్ణాంకం లోపల_post(శూన్య);
శూన్య mathcp_అమరిక(శూన్య);

// mouse.c
శూన్య mouse_init(శూన్య);
శూన్య process_mouse(u8 దత్తాంశం);

// optionroms.c
నిర్మాణం rom_శీర్షిక;
శూన్య callrom(నిర్మాణం rom_శీర్షిక *rom, u16 bdf);
శూన్య call_bcv(u16 seg, u16 ip);
పూర్ణాంకం is_pci_vga(నిర్మాణం pci_device *pci);
శూన్య optionrom_అమరిక(శూన్య);
శూన్య vgarom_అమరిక(శూన్య);
శూన్య s3_resume_vga(శూన్య);
బాహ్య పూర్ణాంకం ScreenAndDebug;

// pcibios.c
శూన్య handle_1ab1(నిర్మాణం bregs *regs);
శూన్య bios32_init(శూన్య);

// pmm.c
శూన్య pmm_init(శూన్య);
శూన్య pmm_prepboot(శూన్య);

// pnpbios.c
u16 get_pnp_స్థానభ్రంశం(శూన్య);
శూన్య pnp_init(శూన్య);

// post.c
శూన్య interface_init(శూన్య);
శూన్య device_hardware_అమరిక(శూన్య);
శూన్య prepareboot(శూన్య);
శూన్య ప్రారంభం_Boot(శూన్య);
శూన్య reloc_preinit(శూన్య *f, శూన్య *arg);
శూన్య సంకేతం_mutable_preinit(శూన్య);

// sercon.c
శూన్య sercon_అమరిక(శూన్య);
శూన్య sercon_check_event(శూన్య);

// serial.c
శూన్య serial_అమరిక(శూన్య);
శూన్య lpt_అమరిక(శూన్య);

// version.c
బాహ్య స్థిరరాశి అక్షరం సంస్కరణ[], BUILDINFO[];

// vgahooks.c
శూన్య handle_155f(నిర్మాణం bregs *regs);
శూన్య handle_157f(నిర్మాణం bregs *regs);
శూన్య vgahook_అమరిక(నిర్మాణం pci_device *pci);

#అయితే_ముగింపు // util.h
