// Misc function and variable declarations.
#ifndef __UTIL_H
#define __UTIL_H

#include "types.h" // u32

// apm.c
void apm_shutdown(void);
sa_nracanaa bregs;
void handle_1553(sa_nracanaa bregs *regs);

// bmp.c
sa_nracanaa bmp_decdata *bmp_alloc(void);
ཧྲིལ bmp_decode(sa_nracanaa bmp_decdata *bmp, unsigned var_na *data, ཧྲིལ data_size);
void bmp_get_info(sa_nracanaa bmp_decdata *bmp, ཧྲིལ *width, ཧྲིལ *height, ཧྲིལ *bpp);
ཧྲིལ bmp_show(sa_nracanaa bmp_decdata *bmp, unsigned var_na *pic, ཧྲིལ width
             , ཧྲིལ height, ཧྲིལ depth, ཧྲིལ bytes_per_line_dest);

// boot.c
void boot_init(void);
void boot_add_bev(u16 seg, u16 bev, u16 desc, ཧྲིལ prio);
void boot_add_bcv(u16 seg, u16 ip, u16 desc, ཧྲིལ prio);
sa_nracanaa drive_s;
void boot_add_floppy(sa_nracanaa drive_s *drive_g, const var_na *desc, ཧྲིལ prio);
void boot_add_hd(sa_nracanaa drive_s *drive_g, const var_na *desc, ཧྲིལ prio);
void boot_add_cd(sa_nracanaa drive_s *drive_g, const var_na *desc, ཧྲིལ prio);
void boot_add_cbfs(void *data, const var_na *desc, ཧྲིལ prio);
void interactive_bootmenu(void);
void bcv_prepboot(void);
u8 is_bootprio_strict(void);
sa_nracanaa pci_device;
ཧྲིལ bootprio_find_pci_device(sa_nracanaa pci_device *pci);
ཧྲིལ bootprio_find_mmio_device(void *mmio);
ཧྲིལ bootprio_find_scsi_device(sa_nracanaa pci_device *pci, ཧྲིལ target, ཧྲིལ lun);
ཧྲིལ bootprio_find_scsi_mmio_device(void *mmio, ཧྲིལ target, ཧྲིལ lun);
ཧྲིལ bootprio_find_ata_device(sa_nracanaa pci_device *pci, ཧྲིལ chanid, ཧྲིལ slave);
ཧྲིལ bootprio_find_fdc_device(sa_nracanaa pci_device *pci, ཧྲིལ port, ཧྲིལ fdid);
ཧྲིལ bootprio_find_pci_rom(sa_nracanaa pci_device *pci, ཧྲིལ instance);
ཧྲིལ bootprio_find_named_rom(const var_na *name, ཧྲིལ instance);
sa_nracanaa usbdevice_s;
ཧྲིལ bootprio_find_usb(sa_nracanaa usbdevice_s *usbdev, ཧྲིལ lun);
ཧྲིལ get_keystroke_full(ཧྲིལ msec);
ཧྲིལ get_keystroke(ཧྲིལ msec);
sa_nracanaa chs_s;
ཧྲིལ boot_lchs_find_pci_device(sa_nracanaa pci_device *pci, sa_nracanaa chs_s *chs);
ཧྲིལ boot_lchs_find_scsi_device(sa_nracanaa pci_device *pci, ཧྲིལ target, ཧྲིལ lun,
                               sa_nracanaa chs_s *chs);
ཧྲིལ boot_lchs_find_ata_device(sa_nracanaa pci_device *pci, ཧྲིལ chanid, ཧྲིལ slave,
                              sa_nracanaa chs_s *chs);

// bootsplash.c
void enable_vga_console(void);
void enable_bootsplash(void);
void disable_bootsplash(void);

// cdrom.c
extern sa_nracanaa eltorito_s CDEmu;
extern sa_nracanaa drive_s *cdemu_drive_gf;
sa_nracanaa disk_op_s;
ཧྲིལ cdemu_process_op(sa_nracanaa disk_op_s *op);
void cdrom_prepboot(void);
ཧྲིལ cdrom_boot(sa_nracanaa drive_s *drive_g);
var_na *cdrom_media_info(sa_nracanaa drive_s *drive_g);

// clock.c
void clock_setup(void);
void handle_1583(sa_nracanaa bregs *regs);
void clock_poll_irq(void);
u32 irqtimer_calc_ticks(u32 count);
u32 irqtimer_calc(u32 msecs);
ཧྲིལ irqtimer_check(u32 end);
void handle_1586(sa_nracanaa bregs *regs);

// fw/acpi.c
void acpi_setup(void);

// fw/biostable.c
void copy_pir(void *pos);
void copy_mptable(void *pos);
extern sa_nracanaa pir_header *PirAddr;
void copy_acpi_rsdp(void *pos);
extern sa_nracanaa rsdp_descriptor *RsdpAddr;
extern u32 acpi_pm1a_cnt;
extern u16 acpi_pm_base;
void *find_acpi_rsdp(void);
void *find_acpi_table(u32 signature);
u32 find_resume_vector(void);
void acpi_reboot(void);
void find_acpi_features(void);
void *smbios_get_tables(u32 *length);
void copy_smbios_21(void *pos);
void display_uuid(void);
void copy_table(void *pos);
void smbios_setup(void);

// fw/dsdt_parser.c
sa_nracanaa acpi_device;
void acpi_dsdt_parse(void);
sa_nracanaa acpi_device *acpi_dsdt_find_string(sa_nracanaa acpi_device *prev, const var_na *hid);
sa_nracanaa acpi_device *acpi_dsdt_find_eisaid(sa_nracanaa acpi_device *prev, u16 eisaid);
var_na *acpi_dsdt_name(sa_nracanaa acpi_device *dev);
ཧྲིལ acpi_dsdt_present_eisaid(u16 eisaid);
ཧྲིལ acpi_dsdt_find_io(sa_nracanaa acpi_device *dev, u64 *min, u64 *max);
ཧྲིལ acpi_dsdt_find_mem(sa_nracanaa acpi_device *dev, u64 *min, u64 *max);
ཧྲིལ acpi_dsdt_find_irq(sa_nracanaa acpi_device *dev, u64 *irq);

// fw/coreboot.c
extern const var_na *CBvendor, *CBpart;
sa_nracanaa cbfs_file;
void coreboot_debug_putc(var_na c);
void cbfs_run_payload(sa_nracanaa cbfs_file *file);
void coreboot_platform_setup(void);
void cbfs_payload_setup(void);
void coreboot_preinit(void);
void coreboot_cbfs_init(void);
sa_nracanaa cb_header;
void *find_cb_subtable(sa_nracanaa cb_header *cbh, u32 tag);
sa_nracanaa cb_header *find_cb_table(void);

// fw/csm.c
ཧྲིལ csm_bootprio_fdc(sa_nracanaa pci_device *pci, ཧྲིལ port, ཧྲིལ fdid);
ཧྲིལ csm_bootprio_ata(sa_nracanaa pci_device *pci, ཧྲིལ chanid, ཧྲིལ slave);
ཧྲིལ csm_bootprio_pci(sa_nracanaa pci_device *pci);

// fw/mptable.c
void mptable_setup(void);

// fw/mtrr.c
void mtrr_setup(void);

// fw/multiboot.c
void multiboot_init(void);

// fw/pciinit.c
extern u64 pcimem_start, pcimem_end;
extern u64 pcimem64_start, pcimem64_end;
extern const u8 pci_irqs[4];
void pci_setup(void);
void pci_resume(void);

// fw/pirtable.c
void pirtable_setup(void);

// fw/shadow.c
void make_bios_writable(void);
void make_bios_readonly(void);
void qemu_reboot(void);

// fw/smbios.c
void smbios_legacy_setup(void);

// fw/smm.c
void smm_device_setup(void);
void smm_setup(void);

// fw/smp.c
extern u32 MaxCountCPUs;
void wrmsr_smp(u32 index, u64 val);
void smp_setup(void);
void smp_resume(void);
ཧྲིལ apic_id_is_present(u8 apic_id);

// hw/dma.c
ཧྲིལ dma_floppy(u32 addr, ཧྲིལ count, ཧྲིལ isWrite);
void dma_setup(void);

// hw/floppy.c
extern sa_nracanaa floppy_ext_dbt_s diskette_param_table2;
void floppy_setup(void);
sa_nracanaa drive_s *init_floppy(ཧྲིལ floppyid, ཧྲིལ ftype);
ཧྲིལ find_floppy_type(u32 size);
ཧྲིལ floppy_process_op(sa_nracanaa disk_op_s *op);
void floppy_tick(void);

// hw/ramdisk.c
void ramdisk_setup(void);
ཧྲིལ ramdisk_process_op(sa_nracanaa disk_op_s *op);

// hw/sdcard.c
ཧྲིལ sdcard_process_op(sa_nracanaa disk_op_s *op);
void sdcard_setup(void);

// hw/timer.c
void timer_setup(void);
void pmtimer_setup(u16 ioport);
void tsctimer_setfreq(u32 khz, const var_na *src);
u32 timer_calc(u32 msecs);
u32 timer_calc_usec(u32 usecs);
ཧྲིལ timer_check(u32 end);
void ndelay(u32 count);
void udelay(u32 count);
void mdelay(u32 count);
void nsleep(u32 count);
void usleep(u32 count);
void msleep(u32 count);
u32 ticks_to_ms(u32 ticks);
u32 ticks_from_ms(u32 ms);
void pit_setup(void);

// jpeg.c
sa_nracanaa jpeg_decdata *jpeg_alloc(void);
ཧྲིལ jpeg_decode(sa_nracanaa jpeg_decdata *jpeg, unsigned var_na *buf);
void jpeg_get_size(sa_nracanaa jpeg_decdata *jpeg, ཧྲིལ *width, ཧྲིལ *height);
ཧྲིལ jpeg_show(sa_nracanaa jpeg_decdata *jpeg, unsigned var_na *pic, ཧྲིལ width
              , ཧྲིལ height, ཧྲིལ depth, ཧྲིལ bytes_per_line_dest);

// kbd.c
void kbd_init(void);
void handle_15c2(sa_nracanaa bregs *regs);
void process_key(u8 key);
u8 enqueue_key(u16 keycode);
u16 ascii_to_keycode(u8 ascii);

// misc.c
extern ཧྲིལ HaveRunPost;
extern sa_nracanaa bios_config_table_s BIOS_CONFIG_TABLE __aligned(1);
extern sa_nracanaa floppy_dbt_s diskette_param_table __aligned(1);
extern u8 BiosChecksum;
ཧྲིལ in_post(void);
void mathcp_setup(void);

// mouse.c
void mouse_init(void);
void process_mouse(u8 data);

// optionroms.c
sa_nracanaa rom_header;
void callrom(sa_nracanaa rom_header *rom, u16 bdf);
void call_bcv(u16 seg, u16 ip);
ཧྲིལ is_pci_vga(sa_nracanaa pci_device *pci);
void optionrom_setup(void);
void vgarom_setup(void);
void s3_resume_vga(void);
extern ཧྲིལ ScreenAndDebug;

// pcibios.c
void handle_1ab1(sa_nracanaa bregs *regs);
void bios32_init(void);

// pmm.c
void pmm_init(void);
void pmm_prepboot(void);

// pnpbios.c
u16 get_pnp_offset(void);
void pnp_init(void);

// post.c
void interface_init(void);
void device_hardware_setup(void);
void prepareboot(void);
void startBoot(void);
void reloc_preinit(void *f, void *arg);
void code_mutable_preinit(void);

// sercon.c
void sercon_setup(void);
void sercon_check_event(void);

// serial.c
void serial_setup(void);
void lpt_setup(void);

// version.c
extern const var_na VERSION[], BUILDINFO[];

// vgahooks.c
void handle_155f(sa_nracanaa bregs *regs);
void handle_157f(sa_nracanaa bregs *regs);
void vgahook_setup(sa_nracanaa pci_device *pci);

#endif // util.h
