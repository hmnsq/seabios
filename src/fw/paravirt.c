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
整数 PlatformRunningOn VARFSEG;
// cfg enabled
整数 cfg_enabled = 0;
// cfg_dma enabled
整数 cfg_dma_enabled = 0;

inline 整数 qemu_cfg_enabled(void)
{
    返回 cfg_enabled;
}

inline 整数 qemu_cfg_dma_enabled(void)
{
    返回 cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
static unsigned 整数 kvm_cpuid_base = 0;

static void kvm_detect(void)
{
    unsigned 整数 i, max = 0;
    unsigned 整数 eax, ebx, ecx, edx;
    char signature[13];

    为 (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        如果 (eax < i)
            break;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        如果 (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    如果 (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        如果 (max >= kvm_cpuid_base + 0x10) {
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
    unsigned 整数 eax, ebx, ecx, edx, msr;

    如果 (!runningOnKVM())
        返回;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    如果 (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    否则 如果 (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    否则
        返回;

    kvmclock = memalign_low(sizeof(*kvmclock), 32);
    memset(kvmclock, 0, sizeof(*kvmclock));
    u32 value = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", value, msr);
    wrmsr(msr, value | 0x01);

    如果 (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        返回;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    如果 (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    否则
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

static void physbits(整数 qemu_quirk)
{
    unsigned 整数 max, eax, ebx, ecx, edx;
    unsigned 整数 physbits;
    char signature[13];
    整数 pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    如果 (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    如果 (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    如果 (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        如果 (!qemu_quirk) {
            valid = 1;
        } 否则 如果 (physbits >= 41) {
            valid = 1;
        } 否则 如果 (strcmp(signature, "GenuineIntel") == 0) {
            如果 ((physbits == 36) || (physbits == 39))
                valid = 1;
        } 否则 如果 (strcmp(signature, "AuthenticAMD") == 0) {
            如果 (physbits == 40)
                valid = 1;
        }
    } 否则 {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    如果 (valid)
        CPUPhysBits = physbits;
}

static void qemu_detect(void)
{
    如果 (!CONFIG_QEMU_HARDWARE)
        返回;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    如果 (v == 0x0000 || v == 0xffff)
        返回;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    如果 (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        返回;

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

static 整数 qemu_early_e820(void);

void
qemu_preinit(void)
{
    qemu_detect();
    kvm_detect();

    如果 (!CONFIG_QEMU)
        返回;

    如果 (runningOnXen()) {
        xen_ramsize_preinit();
        返回;
    }

    // try read e820 table first
    如果 (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_LOW) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_HIGH) << 24));
        如果 (rs)
            rs += 16 * 1024 * 1024;
        否则
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
    如果 (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

void
qemu_platform_setup(void)
{
    如果 (!CONFIG_QEMU)
        返回;

    如果 (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        返回;
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
    如果 (MaxCountCPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    如果 (CONFIG_FW_ROMFILE_LOAD) {
        整数 loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        如果 (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            返回;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        如果 (!loader_err)
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

    当(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

static void
qemu_cfg_read(void *buf, 整数 len)
{
    如果 (len == 0) {
        返回;
    }

    如果 (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } 否则 {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

static void
qemu_cfg_write(void *buf, 整数 len)
{
    如果 (len == 0) {
        返回;
    }

    如果 (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } 否则 {
        warn_internalerror();
    }
}

static void
qemu_cfg_skip(整数 len)
{
    如果 (len == 0) {
        返回;
    }

    如果 (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } 否则 {
        当 (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

static void
qemu_cfg_read_entry(void *buf, 整数 e, 整数 len)
{
    如果 (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } 否则 {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

static void
qemu_cfg_write_entry(void *buf, 整数 e, 整数 len)
{
    如果 (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } 否则 {
        warn_internalerror();
    }
}

struct qemu_romfile_s {
    struct romfile_s file;
    整数 select, skip;
};

static 整数
qemu_cfg_read_file(struct romfile_s *file, void *dst, u32 maxlen)
{
    如果 (file->size > maxlen)
        返回 -1;
    struct qemu_romfile_s *qfile;
    qfile = container_of(file, struct qemu_romfile_s, file);
    如果 (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } 否则 {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    返回 file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
整数
qemu_cfg_write_file_simple(void *src, u16 key, u32 offset, u32 len)
{
    如果 (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, key, len);
    } 否则 {
        qemu_cfg_select(key);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    返回 len;
}

整数
qemu_cfg_write_file(void *src, struct romfile_s *file, u32 offset, u32 len)
{
    如果 ((offset + len) > file->size)
        返回 -1;

    如果 (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        返回 -1;
    }
    返回 qemu_cfg_write_file_simple(src, qemu_get_romfile_key(file),
                                      offset, len);
}

static void
qemu_romfile_add(char *name, 整数 select, 整数 skip, 整数 size)
{
    struct qemu_romfile_s *qfile = malloc_tmp(sizeof(*qfile));
    如果 (!qfile) {
        warn_noalloc();
        返回;
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
    如果 (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        返回 0;
    }
    qfile = container_of(file, struct qemu_romfile_s, file);
    返回 qfile->select;
}

static 整数 rtc_present(void)
{
    返回 rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_count(void)
{
    u16 smp_count = 0;
    如果 (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_count, QEMU_CFG_NB_CPUS, sizeof(smp_count));
    }
    如果 (rtc_present()) {
        u16 cmos_cpu_count = rtc_read(CMOS_BIOS_SMP_COUNT) + 1;
        如果 (smp_count < cmos_cpu_count) {
            smp_count = cmos_cpu_count;
        }
    }
    返回 smp_count;
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
    如果 (!CONFIG_QEMU)
        返回;

    如果 (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        返回;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, sizeof(count32));
    如果 (count32) {
        struct e820_reservation entry;
        整数 i;
        为 (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, sizeof(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } 否则 如果 (runningOnKVM()) {
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
    如果 (!CONFIG_QEMU)
        返回;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, sizeof(numacount));
    整数 max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, sizeof(numacount)
                     , max_cpu*sizeof(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , sizeof(numacount) + max_cpu*sizeof(u64)
                     , numacount*sizeof(u64));

    // ACPI tables
    char name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, sizeof(cnt));
    整数 i, offset = sizeof(cnt);
    为 (i = 0; i < cnt; i++) {
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
    为 (i = 0; i < cnt; i++) {
        struct qemu_smbios_header header;
        qemu_cfg_read(&header, sizeof(header));
        如果 (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, sizeof(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + sizeof(header)
                             , header.length - sizeof(header));
        } 否则 {
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

static 整数 qemu_cfg_detect(void)
{
    如果 (cfg_enabled)
        返回 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    char *sig = "QEMU";
    整数 i;
    为 (i = 0; i < 4; i++)
        如果 (inb(PORT_QEMU_CFG_DATA) != sig[i])
            返回 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, sizeof(id));

    如果 (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    返回 1;
}

void qemu_cfg_init(void)
{
    如果 (!runningOnQEMU())
        返回;

    如果 (!qemu_cfg_detect())
        返回;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 count;
    qemu_cfg_read_entry(&count, QEMU_CFG_FILE_DIR, sizeof(count));
    count = be32_to_cpu(count);
    u32 e;
    为 (e = 0; e < count; e++) {
        struct QemuCfgFile qfile;
        qemu_cfg_read(&qfile, sizeof(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    如果 (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, sizeof(nogfx));
    如果 (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
static 整数 qemu_early_e820(void)
{
    struct e820_reservation table;
    struct QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 count, i;

    如果 (!qemu_cfg_detect())
        返回 0;

    // find e820 table
    qemu_cfg_read_entry(&count, QEMU_CFG_FILE_DIR, sizeof(count));
    count = be32_to_cpu(count);
    为 (i = 0; i < count; i++) {
        qemu_cfg_read(&qfile, sizeof(qfile));
        如果 (memcmp(qfile.name, "etc/e820", 9) != 0)
            continue;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        break;
    }
    如果 (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        返回 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    count = size/sizeof(table);
    为 (i = 0, select = 0; i < count; i++) {
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
            如果 (table.address < 0x100000000LL) {
                // below 4g
                如果 (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } 否则 {
                // above 4g
                如果 (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    返回 1;
}
