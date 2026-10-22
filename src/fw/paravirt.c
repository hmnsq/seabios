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
puur_naa_nka PlatformRunningOn VARFSEG;
// cfg enabled
puur_naa_nka cfg_enabled = 0;
// cfg_dma enabled
puur_naa_nka cfg_dma_enabled = 0;

anta_hstha puur_naa_nka qemu_cfg_enabled(void)
{
    ᤀᤡᤴ cfg_enabled;
}

anta_hstha puur_naa_nka qemu_cfg_dma_enabled(void)
{
    ᤀᤡᤴ cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
static unsigned puur_naa_nka kvm_cpuid_base = 0;

static void kvm_detect(void)
{
    unsigned puur_naa_nka i, max = 0;
    unsigned puur_naa_nka eax, ebx, ecx, edx;
    var_na signature[13];

    for (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        ᤀᤡ᤺ (eax < i)
            bha_nga;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        ᤀᤡ᤺ (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    ᤀᤡ᤺ (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        ᤀᤡ᤺ (max >= kvm_cpuid_base + 0x10) {
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

sa_nracanaa pvclock_vcpu_time_info *kvmclock;

static void kvmclock_init(void)
{
    unsigned puur_naa_nka eax, ebx, ecx, edx, msr;

    ᤀᤡ᤺ (!runningOnKVM())
        ᤀᤡᤴ;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    ᤀᤡ᤺ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    ᤀᤡ᤺ᤆᤡ
        ᤀᤡᤴ;

    kvmclock = memalign_low(parimaa_na(*kvmclock), 32);
    memset(kvmclock, 0, parimaa_na(*kvmclock));
    u32 value = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", value, msr);
    wrmsr(msr, value | 0x01);

    ᤀᤡ᤺ (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        ᤀᤡᤴ;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    ᤀᤡ᤺ (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    ᤀᤡ᤺ᤆᤡ
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

static void physbits(puur_naa_nka qemu_quirk)
{
    unsigned puur_naa_nka max, eax, ebx, ecx, edx;
    unsigned puur_naa_nka physbits;
    var_na signature[13];
    puur_naa_nka pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    ᤀᤡ᤺ (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    ᤀᤡ᤺ (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    ᤀᤡ᤺ (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        ᤀᤡ᤺ (!qemu_quirk) {
            valid = 1;
        } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (physbits >= 41) {
            valid = 1;
        } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(signature, "GenuineIntel") == 0) {
            ᤀᤡ᤺ ((physbits == 36) || (physbits == 39))
                valid = 1;
        } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(signature, "AuthenticAMD") == 0) {
            ᤀᤡ᤺ (physbits == 40)
                valid = 1;
        }
    } ᤀᤡ᤺ᤆᤡ {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    ᤀᤡ᤺ (valid)
        CPUPhysBits = physbits;
}

static void qemu_detect(void)
{
    ᤀᤡ᤺ (!CONFIG_QEMU_HARDWARE)
        ᤀᤡᤴ;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    ᤀᤡ᤺ (v == 0x0000 || v == 0xffff)
        ᤀᤡᤴ;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    ᤀᤡ᤺ (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        ᤀᤡᤴ;

    PlatformRunningOn |= PF_QEMU;
    switch (d) {
    pak_sa 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        bha_nga;
    pak_sa 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        bha_nga;
    default:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        bha_nga;
    }
    physbits(1);
}

static puur_naa_nka qemu_early_e820(void);

void
qemu_preinit(void)
{
    qemu_detect();
    kvm_detect();

    ᤀᤡ᤺ (!CONFIG_QEMU)
        ᤀᤡᤴ;

    ᤀᤡ᤺ (runningOnXen()) {
        xen_ramsize_preinit();
        ᤀᤡᤴ;
    }

    // try read e820 table first
    ᤀᤡ᤺ (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_LOW) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_HIGH) << 24));
        ᤀᤡ᤺ (rs)
            rs += 16 * 1024 * 1024;
        ᤀᤡ᤺ᤆᤡ
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
    ᤀᤡ᤺ (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

void
qemu_platform_setup(void)
{
    ᤀᤡ᤺ (!CONFIG_QEMU)
        ᤀᤡᤴ;

    ᤀᤡ᤺ (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        ᤀᤡᤴ;
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
    ᤀᤡ᤺ (MaxCountCPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    ᤀᤡ᤺ (CONFIG_FW_ROMFILE_LOAD) {
        puur_naa_nka loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        ᤀᤡ᤺ (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            ᤀᤡᤴ;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        ᤀᤡ᤺ (!loader_err)
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

    ᤀᤠ᤺(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

static void
qemu_cfg_read(void *buf, puur_naa_nka len)
{
    ᤀᤡ᤺ (len == 0) {
        ᤀᤡᤴ;
    }

    ᤀᤡ᤺ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } ᤀᤡ᤺ᤆᤡ {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

static void
qemu_cfg_write(void *buf, puur_naa_nka len)
{
    ᤀᤡ᤺ (len == 0) {
        ᤀᤡᤴ;
    }

    ᤀᤡ᤺ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } ᤀᤡ᤺ᤆᤡ {
        warn_internalerror();
    }
}

static void
qemu_cfg_skip(puur_naa_nka len)
{
    ᤀᤡ᤺ (len == 0) {
        ᤀᤡᤴ;
    }

    ᤀᤡ᤺ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } ᤀᤡ᤺ᤆᤡ {
        ᤀᤠ᤺ (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

static void
qemu_cfg_read_entry(void *buf, puur_naa_nka e, puur_naa_nka len)
{
    ᤀᤡ᤺ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } ᤀᤡ᤺ᤆᤡ {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

static void
qemu_cfg_write_entry(void *buf, puur_naa_nka e, puur_naa_nka len)
{
    ᤀᤡ᤺ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } ᤀᤡ᤺ᤆᤡ {
        warn_internalerror();
    }
}

sa_nracanaa qemu_romfile_s {
    sa_nracanaa romfile_s file;
    puur_naa_nka select, skip;
};

static puur_naa_nka
qemu_cfg_read_file(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    ᤀᤡ᤺ (file->size > maxlen)
        ᤀᤡᤴ -1;
    sa_nracanaa qemu_romfile_s *qfile;
    qfile = container_of(file, sa_nracanaa qemu_romfile_s, file);
    ᤀᤡ᤺ (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } ᤀᤡ᤺ᤆᤡ {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    ᤀᤡᤴ file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
puur_naa_nka
qemu_cfg_write_file_simple(void *src, u16 key, u32 offset, u32 len)
{
    ᤀᤡ᤺ (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, key, len);
    } ᤀᤡ᤺ᤆᤡ {
        qemu_cfg_select(key);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    ᤀᤡᤴ len;
}

puur_naa_nka
qemu_cfg_write_file(void *src, sa_nracanaa romfile_s *file, u32 offset, u32 len)
{
    ᤀᤡ᤺ ((offset + len) > file->size)
        ᤀᤡᤴ -1;

    ᤀᤡ᤺ (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        ᤀᤡᤴ -1;
    }
    ᤀᤡᤴ qemu_cfg_write_file_simple(src, qemu_get_romfile_key(file),
                                      offset, len);
}

static void
qemu_romfile_add(var_na *name, puur_naa_nka select, puur_naa_nka skip, puur_naa_nka size)
{
    sa_nracanaa qemu_romfile_s *qfile = malloc_tmp(parimaa_na(*qfile));
    ᤀᤡ᤺ (!qfile) {
        warn_noalloc();
        ᤀᤡᤴ;
    }
    memset(qfile, 0, parimaa_na(*qfile));
    strtcpy(qfile->file.name, name, parimaa_na(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_key(sa_nracanaa romfile_s *file)
{
    sa_nracanaa qemu_romfile_s *qfile;
    ᤀᤡ᤺ (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        ᤀᤡᤴ 0;
    }
    qfile = container_of(file, sa_nracanaa qemu_romfile_s, file);
    ᤀᤡᤴ qfile->select;
}

static puur_naa_nka rtc_present(void)
{
    ᤀᤡᤴ rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_count(void)
{
    u16 smp_count = 0;
    ᤀᤡ᤺ (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_count, QEMU_CFG_NB_CPUS, parimaa_na(smp_count));
    }
    ᤀᤡ᤺ (rtc_present()) {
        u16 cmos_cpu_count = rtc_read(CMOS_BIOS_SMP_COUNT) + 1;
        ᤀᤡ᤺ (smp_count < cmos_cpu_count) {
            smp_count = cmos_cpu_count;
        }
    }
    ᤀᤡᤴ smp_count;
}

sa_nracanaa e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

sa_nracanaa qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

static void
qemu_cfg_e820(void)
{
    ᤀᤡ᤺ (!CONFIG_QEMU)
        ᤀᤡᤴ;

    ᤀᤡ᤺ (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        ᤀᤡᤴ;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, parimaa_na(count32));
    ᤀᤡ᤺ (count32) {
        sa_nracanaa e820_reservation entry;
        puur_naa_nka i;
        for (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, parimaa_na(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (runningOnKVM()) {
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
    ᤀᤡ᤺ (!CONFIG_QEMU)
        ᤀᤡᤴ;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, parimaa_na(numacount));
    puur_naa_nka max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, parimaa_na(numacount)
                     , max_cpu*parimaa_na(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , parimaa_na(numacount) + max_cpu*parimaa_na(u64)
                     , numacount*parimaa_na(u64));

    // ACPI tables
    var_na name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, parimaa_na(cnt));
    puur_naa_nka i, offset = parimaa_na(cnt);
    for (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, parimaa_na(len));
        offset += parimaa_na(len);
        snprintf(name, parimaa_na(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, parimaa_na(cnt));
    offset = parimaa_na(cnt);
    for (i = 0; i < cnt; i++) {
        sa_nracanaa qemu_smbios_header header;
        qemu_cfg_read(&header, parimaa_na(header));
        ᤀᤡ᤺ (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, parimaa_na(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + parimaa_na(header)
                             , header.length - parimaa_na(header));
        } ᤀᤡ᤺ᤆᤡ {
            snprintf(name, parimaa_na(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - parimaa_na(header));
        offset += header.length;
    }
}

sa_nracanaa QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    var_na name[56];
};

static puur_naa_nka qemu_cfg_detect(void)
{
    ᤀᤡ᤺ (cfg_enabled)
        ᤀᤡᤴ 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    var_na *sig = "QEMU";
    puur_naa_nka i;
    for (i = 0; i < 4; i++)
        ᤀᤡ᤺ (inb(PORT_QEMU_CFG_DATA) != sig[i])
            ᤀᤡᤴ 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, parimaa_na(id));

    ᤀᤡ᤺ (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    ᤀᤡᤴ 1;
}

void qemu_cfg_init(void)
{
    ᤀᤡ᤺ (!runningOnQEMU())
        ᤀᤡᤴ;

    ᤀᤡ᤺ (!qemu_cfg_detect())
        ᤀᤡᤴ;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 count;
    qemu_cfg_read_entry(&count, QEMU_CFG_FILE_DIR, parimaa_na(count));
    count = be32_to_cpu(count);
    u32 e;
    for (e = 0; e < count; e++) {
        sa_nracanaa QemuCfgFile qfile;
        qemu_cfg_read(&qfile, parimaa_na(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    ᤀᤡ᤺ (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, parimaa_na(nogfx));
    ᤀᤡ᤺ (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
static puur_naa_nka qemu_early_e820(void)
{
    sa_nracanaa e820_reservation table;
    sa_nracanaa QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 count, i;

    ᤀᤡ᤺ (!qemu_cfg_detect())
        ᤀᤡᤴ 0;

    // find e820 table
    qemu_cfg_read_entry(&count, QEMU_CFG_FILE_DIR, parimaa_na(count));
    count = be32_to_cpu(count);
    for (i = 0; i < count; i++) {
        qemu_cfg_read(&qfile, parimaa_na(qfile));
        ᤀᤡ᤺ (memcmp(qfile.name, "etc/e820", 9) != 0)
            continue;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        bha_nga;
    }
    ᤀᤡ᤺ (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        ᤀᤡᤴ 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    count = size/parimaa_na(table);
    for (i = 0, select = 0; i < count; i++) {
        qemu_cfg_read(&table, parimaa_na(table));
        switch (table.type) {
        pak_sa E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            bha_nga;
        pak_sa E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            bha_nga;
        pak_sa E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            ᤀᤡ᤺ (table.address < 0x100000000LL) {
                // below 4g
                ᤀᤡ᤺ (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } ᤀᤡ᤺ᤆᤡ {
                // above 4g
                ᤀᤡ᤺ (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    ᤀᤡᤴ 1;
}
