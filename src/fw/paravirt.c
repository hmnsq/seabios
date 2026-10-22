// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "byteorder.h" // be32_to_cpu
#include "config.h" // CONFIG_QEMU
#include "e820map.h" // e820_add
#include "hw/pci.h" // pci_config_readw
#include "hw/pcidevice.h" // pci_probe_devices
#include "hw/pci_regs.h" // PCI_DEVICE_ID
#include "hw/serialio.h" // PORT_SERIAL1
#include "hw/rtc.h" // CMOS_*
#include "hw/virtio-mmio.h" // virtio_mmio_acpi
#include "malloc.h" // malloc_tmp
#include "output.h" // dprintf
#include "paravirt.h" // qemu_cfg_preinit
#include "romfile.h" // romfile_loadint
#include "romfile_loader.h" // romfile_loader_execute
#include "string.h" // memset
#include "util.h" // pci_setup
#include "x86.h" // cpuid
#include "xen.h" // xen_biostable_setup
#include "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
정수 PlatformRunningOn VARFSEG;
// cfg enabled
정수 cfg_enabled = 0;
// cfg_dma enabled
정수 cfg_dma_enabled = 0;

inline 정수 qemu_cfg_enabled(void)
{
    반환 cfg_enabled;
}

inline 정수 qemu_cfg_dma_enabled(void)
{
    반환 cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
static unsigned 정수 kvm_cpuid_base = 0;

static void kvm_detect(void)
{
    unsigned 정수 i, max = 0;
    unsigned 정수 eax, ebx, ecx, edx;
    char signature[13];

    위해 (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        만약 (eax < i)
            break;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        만약 (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    만약 (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        만약 (max >= kvm_cpuid_base + 0x10) {
            cpuid(kvm_cpuid_base + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#define KVM_FEATURE_CLOCKSOURCE           0
#define KVM_FEATURE_CLOCKSOURCE2          3

#define MSR_KVM_SYSTEM_TIME            0x12
#define MSR_KVM_SYSTEM_TIME_NEW  0x4b564d01

#define PVCLOCK_TSC_STABLE_BIT     (1 << 0)

struct pvclock_vcpu_time_info *kvmclock;

static void kvmclock_init(void)
{
    unsigned 정수 eax, ebx, ecx, edx, msr;

    만약 (!runningOnKVM())
        반환;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    만약 (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    아니면 만약 (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    아니면
        반환;

    kvmclock = memalign_low(sizeof(*kvmclock), 32);
    memset(kvmclock, 0, sizeof(*kvmclock));
    u32 value = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", value, msr);
    wrmsr(msr, value | 0x01);

    만약 (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        반환;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    만약 (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    아니면
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

static void physbits(정수 qemu_quirk)
{
    unsigned 정수 max, eax, ebx, ecx, edx;
    unsigned 정수 physbits;
    char signature[13];
    정수 pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    만약 (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    만약 (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    만약 (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        만약 (!qemu_quirk) {
            valid = 1;
        } 아니면 만약 (physbits >= 41) {
            valid = 1;
        } 아니면 만약 (strcmp(signature, "GenuineIntel") == 0) {
            만약 ((physbits == 36) || (physbits == 39))
                valid = 1;
        } 아니면 만약 (strcmp(signature, "AuthenticAMD") == 0) {
            만약 (physbits == 40)
                valid = 1;
        }
    } 아니면 {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    만약 (valid)
        CPUPhysBits = physbits;
}

static void qemu_detect(void)
{
    만약 (!CONFIG_QEMU_HARDWARE)
        반환;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    만약 (v == 0x0000 || v == 0xffff)
        반환;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    만약 (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        반환;

    PlatformRunningOn |= PF_QEMU;
    switch (d) {
    case 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        break;
    case 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        break;
    default:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        break;
    }
    physbits(1);
}

static 정수 qemu_early_e820(void);

void
qemu_preinit(void)
{
    qemu_detect();
    kvm_detect();

    만약 (!CONFIG_QEMU)
        반환;

    만약 (runningOnXen()) {
        xen_ramsize_preinit();
        반환;
    }

    // try read e820 table first
    만약 (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_LOW) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_HIGH) << 24));
        만약 (rs)
            rs += 16 * 1024 * 1024;
        아니면
            rs = (((rtc_read(CMOS_MEM_EXTMEM_LOW) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_HIGH) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

static void msr_feature_control_setup(void)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    만약 (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

void
qemu_platform_setup(void)
{
    만약 (!CONFIG_QEMU)
        반환;

    만약 (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        반환;
    }

    kvmclock_init();

    // Initialize pci
    pci_setup();
    smm_device_setup();
    smm_setup();

    // Initialize mtrr, msr_feature_control and smp
    mtrr_setup();
    msr_feature_control_setup();
    smp_setup();

    // Create bios tables
    만약 (MaxCountCPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    만약 (CONFIG_FW_ROMFILE_LOAD) {
        정수 loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        만약 (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            반환;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        만약 (!loader_err)
            warn_internalerror();
    }

    acpi_setup();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#define QEMU_CFG_SIGNATURE              0x00
#define QEMU_CFG_ID                     0x01
#define QEMU_CFG_UUID                   0x02
#define QEMU_CFG_NOGRAPHIC              0x04
#define QEMU_CFG_NUMA                   0x0d
#define QEMU_CFG_BOOT_MENU              0x0e
#define QEMU_CFG_NB_CPUS                0x05
#define QEMU_CFG_MAX_CPUS               0x0f
#define QEMU_CFG_FILE_DIR               0x19
#define QEMU_CFG_ARCH_LOCAL             0x8000
#define QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#define QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#define QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#define QEMU_CFG_E820_TABLE             (QEMU_CFG_ARCH_LOCAL + 3)

static void
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

static void
qemu_cfg_dma_transfer(void *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_LOW);

    동안(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

static void
qemu_cfg_read(void *buf, 정수 len)
{
    만약 (len == 0) {
        반환;
    }

    만약 (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } 아니면 {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

static void
qemu_cfg_write(void *buf, 정수 len)
{
    만약 (len == 0) {
        반환;
    }

    만약 (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } 아니면 {
        warn_internalerror();
    }
}

static void
qemu_cfg_skip(정수 len)
{
    만약 (len == 0) {
        반환;
    }

    만약 (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } 아니면 {
        동안 (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

static void
qemu_cfg_read_entry(void *buf, 정수 e, 정수 len)
{
    만약 (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } 아니면 {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

static void
qemu_cfg_write_entry(void *buf, 정수 e, 정수 len)
{
    만약 (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } 아니면 {
        warn_internalerror();
    }
}

struct qemu_romfile_s {
    struct romfile_s file;
    정수 select, skip;
};

static 정수
qemu_cfg_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    만약 (file->size > maxlen)
        반환 -1;
    struct qemu_romfile_s *qfile;
    qfile = container_of(file, struct qemu_romfile_s, file);
    만약 (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } 아니면 {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    반환 file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
정수
qemu_cfg_write_file_simple(void *src, u16 key, u32 offset, u32 len)
{
    만약 (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, key, len);
    } 아니면 {
        qemu_cfg_select(key);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    반환 len;
}

정수
qemu_cfg_write_file(void *src, struct romfile_s *file, u32 offset, u32 len)
{
    만약 ((offset + len) > file->size)
        반환 -1;

    만약 (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        반환 -1;
    }
    반환 qemu_cfg_write_file_simple(src, qemu_get_romfile_key(file),
                                      offset, len);
}

static void
qemu_romfile_add(char *name, 정수 select, 정수 skip, 정수 size)
{
    struct qemu_romfile_s *qfile = malloc_tmp(sizeof(*qfile));
    만약 (!qfile) {
        warn_noalloc();
        반환;
    }
    memset(qfile, 0, sizeof(*qfile));
    strtcpy(qfile->file.name, name, sizeof(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_key(struct romfile_s *file)
{
    struct qemu_romfile_s *qfile;
    만약 (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        반환 0;
    }
    qfile = container_of(file, struct qemu_romfile_s, file);
    반환 qfile->select;
}

static 정수 rtc_present(void)
{
    반환 rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_count(void)
{
    u16 smp_count = 0;
    만약 (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_count, QEMU_CFG_NB_CPUS, sizeof(smp_count));
    }
    만약 (rtc_present()) {
        u16 cmos_cpu_count = rtc_read(CMOS_BIOS_SMP_COUNT) + 1;
        만약 (smp_count < cmos_cpu_count) {
            smp_count = cmos_cpu_count;
        }
    }
    반환 smp_count;
}

struct e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

struct qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

static void
qemu_cfg_e820(void)
{
    만약 (!CONFIG_QEMU)
        반환;

    만약 (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        반환;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, sizeof(count32));
    만약 (count32) {
        struct e820_reservation entry;
        정수 i;
        위해 (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, sizeof(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } 아니면 만약 (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 high = ((rtc_read(CMOS_MEM_HIGHMEM_LOW) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_HIGH) << 32));
    RamSizeOver4G = high;
    e820_add(0x100000000ull, high, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
static void
qemu_cfg_legacy(void)
{
    만약 (!CONFIG_QEMU)
        반환;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, sizeof(numacount));
    정수 max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, sizeof(numacount)
                     , max_cpu*sizeof(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , sizeof(numacount) + max_cpu*sizeof(u64)
                     , numacount*sizeof(u64));

    // ACPI tables
    char name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, sizeof(cnt));
    정수 i, offset = sizeof(cnt);
    위해 (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, sizeof(len));
        offset += sizeof(len);
        snprintf(name, sizeof(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, sizeof(cnt));
    offset = sizeof(cnt);
    위해 (i = 0; i < cnt; i++) {
        struct qemu_smbios_header header;
        qemu_cfg_read(&header, sizeof(header));
        만약 (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, sizeof(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + sizeof(header)
                             , header.length - sizeof(header));
        } 아니면 {
            snprintf(name, sizeof(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - sizeof(header));
        offset += header.length;
    }
}

struct QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    char name[56];
};

static 정수 qemu_cfg_detect(void)
{
    만약 (cfg_enabled)
        반환 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    char *sig = "QEMU";
    정수 i;
    위해 (i = 0; i < 4; i++)
        만약 (inb(PORT_QEMU_CFG_DATA) != sig[i])
            반환 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, sizeof(id));

    만약 (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    반환 1;
}

void qemu_cfg_init(void)
{
    만약 (!runningOnQEMU())
        반환;

    만약 (!qemu_cfg_detect())
        반환;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 count;
    qemu_cfg_read_entry(&count, QEMU_CFG_FILE_DIR, sizeof(count));
    count = be32_to_cpu(count);
    u32 e;
    위해 (e = 0; e < count; e++) {
        struct QemuCfgFile qfile;
        qemu_cfg_read(&qfile, sizeof(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    만약 (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, sizeof(nogfx));
    만약 (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
static 정수 qemu_early_e820(void)
{
    struct e820_reservation table;
    struct QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 count, i;

    만약 (!qemu_cfg_detect())
        반환 0;

    // find e820 table
    qemu_cfg_read_entry(&count, QEMU_CFG_FILE_DIR, sizeof(count));
    count = be32_to_cpu(count);
    위해 (i = 0; i < count; i++) {
        qemu_cfg_read(&qfile, sizeof(qfile));
        만약 (memcmp(qfile.name, "etc/e820", 9) != 0)
            continue;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        break;
    }
    만약 (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        반환 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    count = size/sizeof(table);
    위해 (i = 0, select = 0; i < count; i++) {
        qemu_cfg_read(&table, sizeof(table));
        switch (table.type) {
        case E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            break;
        case E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            break;
        case E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            만약 (table.address < 0x100000000LL) {
                // below 4g
                만약 (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } 아니면 {
                // above 4g
                만약 (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    반환 1;
}
