// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "byteorder.h" // be32_to_cpu
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_QEMU
#ಅನ್ತರ್ಭಾವ "e820map.h" // e820_add
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_readw
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_probe_devices
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_DEVICE_ID
#ಅನ್ತರ್ಭಾವ "hw/serialio.h" // PORT_SERIAL1
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // CMOS_*
#ಅನ್ತರ್ಭಾವ "hw/virtio-mmio.h" // virtio_mmio_acpi
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_tmp
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // qemu_cfg_preinit
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "romfile_loader.h" // romfile_loader_execute
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // pci_setup
#ಅನ್ತರ್ಭಾವ "x86.h" // cpuid
#ಅನ್ತರ್ಭಾವ "xen.h" // xen_biostable_setup
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
ಪೂರ್ಣಾಙ್ಕ PlatformRunningOn VARFSEG;
// cfg enabled
ಪೂರ್ಣಾಙ್ಕ cfg_enabled = 0;
// cfg_dma enabled
ಪೂರ್ಣಾಙ್ಕ cfg_dma_enabled = 0;

ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ qemu_cfg_enabled(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ cfg_enabled;
}

ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ qemu_cfg_dma_enabled(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ kvm_cpuid_base = 0;

ಸ್ಥಿರ ಶೂನ್ಯ kvm_detect(ಶೂನ್ಯ)
{
    unsigned ಪೂರ್ಣಾಙ್ಕ i, max = 0;
    unsigned ಪೂರ್ಣಾಙ್ಕ eax, ebx, ecx, edx;
    ವರ್ಣ signature[13];

    ಪ್ರತಿ (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        ಯದಿ (eax < i)
            ಭಙ್ಗ;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        ಯದಿ (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    ಯದಿ (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        ಯದಿ (max >= kvm_cpuid_base + 0x10) {
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

ಸಂರಚನಾ pvclock_vcpu_time_info *kvmclock;

ಸ್ಥಿರ ಶೂನ್ಯ kvmclock_init(ಶೂನ್ಯ)
{
    unsigned ಪೂರ್ಣಾಙ್ಕ eax, ebx, ecx, edx, msr;

    ಯದಿ (!runningOnKVM())
        ಪ್ರತ್ಯಾಗಮ;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    ಯದಿ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    ಅನ್ಯಥಾ ಯದಿ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    ಅನ್ಯಥಾ
        ಪ್ರತ್ಯಾಗಮ;

    kvmclock = memalign_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*kvmclock), 32);
    memset(kvmclock, 0, ಪರಿಮಾಣ(*kvmclock));
    u32 ಮೌಲ್ಯ = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", ಮೌಲ್ಯ, msr);
    wrmsr(msr, ಮೌಲ್ಯ | 0x01);

    ಯದಿ (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        ಪ್ರತ್ಯಾಗಮ;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    ಯದಿ (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    ಅನ್ಯಥಾ
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ಸ್ಥಿರ ಶೂನ್ಯ physbits(ಪೂರ್ಣಾಙ್ಕ qemu_quirk)
{
    unsigned ಪೂರ್ಣಾಙ್ಕ max, eax, ebx, ecx, edx;
    unsigned ಪೂರ್ಣಾಙ್ಕ physbits;
    ವರ್ಣ signature[13];
    ಪೂರ್ಣಾಙ್ಕ pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    ಯದಿ (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    ಯದಿ (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    ಯದಿ (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        ಯದಿ (!qemu_quirk) {
            valid = 1;
        } ಅನ್ಯಥಾ ಯದಿ (physbits >= 41) {
            valid = 1;
        } ಅನ್ಯಥಾ ಯದಿ (strcmp(signature, "GenuineIntel") == 0) {
            ಯದಿ ((physbits == 36) || (physbits == 39))
                valid = 1;
        } ಅನ್ಯಥಾ ಯದಿ (strcmp(signature, "AuthenticAMD") == 0) {
            ಯದಿ (physbits == 40)
                valid = 1;
        }
    } ಅನ್ಯಥಾ {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    ಯದಿ (valid)
        CPUPhysBits = physbits;
}

ಸ್ಥಿರ ಶೂನ್ಯ qemu_detect(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU_HARDWARE)
        ಪ್ರತ್ಯಾಗಮ;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    ಯದಿ (v == 0x0000 || v == 0xffff)
        ಪ್ರತ್ಯಾಗಮ;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    ಯದಿ (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        ಪ್ರತ್ಯಾಗಮ;

    PlatformRunningOn |= PF_QEMU;
    ವಿಕಲ್ಪ (d) {
    ಪಕ್ಷ 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        ಭಙ್ಗ;
    }
    physbits(1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ qemu_early_e820(ಶೂನ್ಯ);

ಶೂನ್ಯ
qemu_preinit(ಶೂನ್ಯ)
{
    qemu_detect();
    kvm_detect();

    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (runningOnXen()) {
        xen_ramsize_preinit();
        ಪ್ರತ್ಯಾಗಮ;
    }

    // try read e820 table first
    ಯದಿ (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_ಕೆಳಗಿನ) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_ಮೇಲಿನ) << 24));
        ಯದಿ (rs)
            rs += 16 * 1024 * 1024;
        ಅನ್ಯಥಾ
            rs = (((rtc_read(CMOS_MEM_EXTMEM_ಕೆಳಗಿನ) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_ಮೇಲಿನ) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

ಸ್ಥಿರ ಶೂನ್ಯ msr_feature_control_setup(ಶೂನ್ಯ)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    ಯದಿ (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

ಶೂನ್ಯ
qemu_platform_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        ಪ್ರತ್ಯಾಗಮ;
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
    ಯದಿ (Max_ಎಣಿಕೆ_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    ಯದಿ (CONFIG_FW_ROMFILE_LOAD) {
        ಪೂರ್ಣಾಙ್ಕ loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        ಯದಿ (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            ಪ್ರತ್ಯಾಗಮ;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        ಯದಿ (!loader_err)
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

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_dma_transfer(ಶೂನ್ಯ *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_ಕೆಳಗಿನ);

    ಯಾವತ್(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_read(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ len)
{
    ಯದಿ (len == 0) {
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } ಅನ್ಯಥಾ {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_write(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ len)
{
    ಯದಿ (len == 0) {
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } ಅನ್ಯಥಾ {
        warn_internalerror();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_skip(ಪೂರ್ಣಾಙ್ಕ len)
{
    ಯದಿ (len == 0) {
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } ಅನ್ಯಥಾ {
        ಯಾವತ್ (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_read_entry(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ e, ಪೂರ್ಣಾಙ್ಕ len)
{
    ಯದಿ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } ಅನ್ಯಥಾ {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_write_entry(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ e, ಪೂರ್ಣಾಙ್ಕ len)
{
    ಯದಿ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } ಅನ್ಯಥಾ {
        warn_internalerror();
    }
}

ಸಂರಚನಾ qemu_romfile_s {
    ಸಂರಚನಾ romfile_s file;
    ಪೂರ್ಣಾಙ್ಕ select, skip;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
qemu_cfg_read_file(ಸಂರಚನಾ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ಯದಿ (file->size > maxlen)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ qemu_romfile_s *qfile;
    qfile = container_of(file, ಸಂರಚನಾ qemu_romfile_s, file);
    ಯದಿ (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } ಅನ್ಯಥಾ {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    ಪ್ರತ್ಯಾಗಮ file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
ಪೂರ್ಣಾಙ್ಕ
qemu_cfg_write_file_simple(ಶೂನ್ಯ *src, u16 ಕೀಲಿ, u32 offset, u32 len)
{
    ಯದಿ (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, ಕೀಲಿ, len);
    } ಅನ್ಯಥಾ {
        qemu_cfg_select(ಕೀಲಿ);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    ಪ್ರತ್ಯಾಗಮ len;
}

ಪೂರ್ಣಾಙ್ಕ
qemu_cfg_write_file(ಶೂನ್ಯ *src, ಸಂರಚನಾ romfile_s *file, u32 offset, u32 len)
{
    ಯದಿ ((offset + len) > file->size)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ qemu_cfg_write_file_simple(src, qemu_get_romfile_ಕೀಲಿ(file),
                                      offset, len);
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_romfile_add(ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ select, ಪೂರ್ಣಾಙ್ಕ skip, ಪೂರ್ಣಾಙ್ಕ size)
{
    ಸಂರಚನಾ qemu_romfile_s *qfile = malloc_tmp(ಪರಿಮಾಣ(*qfile));
    ಯದಿ (!qfile) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(qfile, 0, ಪರಿಮಾಣ(*qfile));
    strtcpy(qfile->file.name, name, ಪರಿಮಾಣ(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_ಕೀಲಿ(ಸಂರಚನಾ romfile_s *file)
{
    ಸಂರಚನಾ qemu_romfile_s *qfile;
    ಯದಿ (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        ಪ್ರತ್ಯಾಗಮ 0;
    }
    qfile = container_of(file, ಸಂರಚನಾ qemu_romfile_s, file);
    ಪ್ರತ್ಯಾಗಮ qfile->select;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rtc_present(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_ಎಣಿಕೆ(ಶೂನ್ಯ)
{
    u16 smp_ಎಣಿಕೆ = 0;
    ಯದಿ (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_ಎಣಿಕೆ, QEMU_CFG_NB_CPUS, ಪರಿಮಾಣ(smp_ಎಣಿಕೆ));
    }
    ಯದಿ (rtc_present()) {
        u16 cmos_cpu_ಎಣಿಕೆ = rtc_read(CMOS_BIOS_SMP_ಎಣಿಕೆ) + 1;
        ಯದಿ (smp_ಎಣಿಕೆ < cmos_cpu_ಎಣಿಕೆ) {
            smp_ಎಣಿಕೆ = cmos_cpu_ಎಣಿಕೆ;
        }
    }
    ಪ್ರತ್ಯಾಗಮ smp_ಎಣಿಕೆ;
}

ಸಂರಚನಾ e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

ಸಂರಚನಾ qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_e820(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        ಪ್ರತ್ಯಾಗಮ;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, ಪರಿಮಾಣ(count32));
    ಯದಿ (count32) {
        ಸಂರಚನಾ e820_reservation entry;
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, ಪರಿಮಾಣ(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } ಅನ್ಯಥಾ ಯದಿ (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 ಮೇಲಿನ = ((rtc_read(CMOS_MEM_HIGHMEM_ಕೆಳಗಿನ) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_ಮೇಲಿನ) << 32));
    RamSizeOver4G = ಮೇಲಿನ;
    e820_add(0x100000000ull, ಮೇಲಿನ, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_legacy(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, ಪರಿಮಾಣ(numacount));
    ಪೂರ್ಣಾಙ್ಕ max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, ಪರಿಮಾಣ(numacount)
                     , max_cpu*ಪರಿಮಾಣ(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , ಪರಿಮಾಣ(numacount) + max_cpu*ಪರಿಮಾಣ(u64)
                     , numacount*ಪರಿಮಾಣ(u64));

    // ACPI tables
    ವರ್ಣ name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, ಪರಿಮಾಣ(cnt));
    ಪೂರ್ಣಾಙ್ಕ i, offset = ಪರಿಮಾಣ(cnt);
    ಪ್ರತಿ (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, ಪರಿಮಾಣ(len));
        offset += ಪರಿಮಾಣ(len);
        snprintf(name, ಪರಿಮಾಣ(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, ಪರಿಮಾಣ(cnt));
    offset = ಪರಿಮಾಣ(cnt);
    ಪ್ರತಿ (i = 0; i < cnt; i++) {
        ಸಂರಚನಾ qemu_smbios_header header;
        qemu_cfg_read(&header, ಪರಿಮಾಣ(header));
        ಯದಿ (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, ಪರಿಮಾಣ(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + ಪರಿಮಾಣ(header)
                             , header.length - ಪರಿಮಾಣ(header));
        } ಅನ್ಯಥಾ {
            snprintf(name, ಪರಿಮಾಣ(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - ಪರಿಮಾಣ(header));
        offset += header.length;
    }
}

ಸಂರಚನಾ QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    ವರ್ಣ name[56];
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ qemu_cfg_detect(ಶೂನ್ಯ)
{
    ಯದಿ (cfg_enabled)
        ಪ್ರತ್ಯಾಗಮ 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    ವರ್ಣ *sig = "QEMU";
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < 4; i++)
        ಯದಿ (inb(PORT_QEMU_CFG_DATA) != sig[i])
            ಪ್ರತ್ಯಾಗಮ 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, ಪರಿಮಾಣ(id));

    ಯದಿ (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    ಪ್ರತ್ಯಾಗಮ 1;
}

ಶೂನ್ಯ qemu_cfg_init(ಶೂನ್ಯ)
{
    ಯದಿ (!runningOnQEMU())
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (!qemu_cfg_detect())
        ಪ್ರತ್ಯಾಗಮ;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 ಎಣಿಕೆ;
    qemu_cfg_read_entry(&ಎಣಿಕೆ, QEMU_CFG_FILE_DIR, ಪರಿಮಾಣ(ಎಣಿಕೆ));
    ಎಣಿಕೆ = be32_to_cpu(ಎಣಿಕೆ);
    u32 e;
    ಪ್ರತಿ (e = 0; e < ಎಣಿಕೆ; e++) {
        ಸಂರಚನಾ QemuCfgFile qfile;
        qemu_cfg_read(&qfile, ಪರಿಮಾಣ(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    ಯದಿ (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, ಪರಿಮಾಣ(nogfx));
    ಯದಿ (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ qemu_early_e820(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ e820_reservation table;
    ಸಂರಚನಾ QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 ಎಣಿಕೆ, i;

    ಯದಿ (!qemu_cfg_detect())
        ಪ್ರತ್ಯಾಗಮ 0;

    // find e820 table
    qemu_cfg_read_entry(&ಎಣಿಕೆ, QEMU_CFG_FILE_DIR, ಪರಿಮಾಣ(ಎಣಿಕೆ));
    ಎಣಿಕೆ = be32_to_cpu(ಎಣಿಕೆ);
    ಪ್ರತಿ (i = 0; i < ಎಣಿಕೆ; i++) {
        qemu_cfg_read(&qfile, ಪರಿಮಾಣ(qfile));
        ಯದಿ (memcmp(qfile.name, "etc/e820", 9) != 0)
            ಅನುವರ್ತ;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        ಭಙ್ಗ;
    }
    ಯದಿ (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    ಎಣಿಕೆ = size/ಪರಿಮಾಣ(table);
    ಪ್ರತಿ (i = 0, select = 0; i < ಎಣಿಕೆ; i++) {
        qemu_cfg_read(&table, ಪರಿಮಾಣ(table));
        ವಿಕಲ್ಪ (table.type) {
        ಪಕ್ಷ E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            ಭಙ್ಗ;
        ಪಕ್ಷ E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            ಭಙ್ಗ;
        ಪಕ್ಷ E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            ಯದಿ (table.address < 0x100000000LL) {
                // below 4g
                ಯದಿ (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } ಅನ್ಯಥಾ {
                // above 4g
                ಯದಿ (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    ಪ್ರತ್ಯಾಗಮ 1;
}
