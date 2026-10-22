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
u32 Ram_ಗಾತ್ರ;
// Amount of continuous ram >4Gig
u64 Ram_ಗಾತ್ರ_Over4_G;
// physical address space bits
u8 CPUPhys_ಬಿಟ್ಟುಗಳು;
// 64bit processor
u8 CPULong_ಬಹುಲಕ;
// Type of emulator platform.
ಪೂರ್ಣಾಂಕ PlatformRunningOn VARFSEG;
// cfg enabled
ಪೂರ್ಣಾಂಕ cfg_enabled = 0;
// cfg_dma enabled
ಪೂರ್ಣಾಂಕ cfg_dma_enabled = 0;

ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ qemu_cfg_enabled(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ cfg_enabled;
}

ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ qemu_cfg_dma_enabled(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು KVM_CPUID_SIGNATURE     0x40000000
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ kvm_cpuid_ತಳ = 0;

ಸ್ಥಿರ ಶೂನ್ಯ kvm_detect(ಶೂನ್ಯ)
{
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, max = 0;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ eax, ebx, ecx, edx;
    ಅಕ್ಷರ signature[13];

    ಗಾಗಿ (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        ಆದರೆ (eax < i)
            ನಿಲ್ಲಿಸು;
        ಸ್ಮರಣೆ_ನಕಲು(signature + 0, &ebx, 4);
        ಸ್ಮರಣೆ_ನಕಲು(signature + 4, &ecx, 4);
        ಸ್ಮರಣೆ_ನಕಲು(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        ಆದರೆ (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_ತಳ = i;
            max = eax;
        }
    }

    ಆದರೆ (kvm_cpuid_ತಳ) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        ಆದರೆ (max >= kvm_cpuid_ತಳ + 0x10) {
            cpuid(kvm_cpuid_ತಳ + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#ವ್ಯಾಖ್ಯಾನಿಸು KVM_FEATURE_CLOCKSOURCE           0
#ವ್ಯಾಖ್ಯಾನಿಸು KVM_FEATURE_CLOCKSOURCE2          3

#ವ್ಯಾಖ್ಯಾನಿಸು MSR_KVM_SYSTEM_ಸಮಯ            0x12
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_KVM_SYSTEM_ಸಮಯ_NEW  0x4b564d01

#ವ್ಯಾಖ್ಯಾನಿಸು PVCLOCK_TSC_STABLE_ಬಿಟ್     (1 << 0)

ರಚನೆ pvclock_vcpu_ಸಮಯ_info *kvmclock;

ಸ್ಥಿರ ಶೂನ್ಯ kvmclock_init(ಶೂನ್ಯ)
{
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ eax, ebx, ecx, edx, msr;

    ಆದರೆ (!runningOnKVM())
        ಹಿಂತಿರುಗಿ;

    cpuid(kvm_cpuid_ತಳ + 0x01, &eax, &ebx, &ecx, &edx);
    ಆದರೆ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_ಸಮಯ_NEW;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_ಸಮಯ;
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ;

    kvmclock = memalign_ಕೆಳಗಿನ(ಗಾತ್ರ(*kvmclock), 32);
    memset(kvmclock, 0, ಗಾತ್ರ(*kvmclock));
    u32 ಮೌಲ್ಯ = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", ಮೌಲ್ಯ, msr);
    wrmsr(msr, ಮೌಲ್ಯ | 0x01);

    ಆದರೆ (!(kvmclock->flags & PVCLOCK_TSC_STABLE_ಬಿಟ್))
        ಹಿಂತಿರುಗಿ;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_ಗಮ್ಯ_system_mul >> 16);
    ಆದರೆ (kvmclock->tsc_ಸರಿಸು < 0)
        MHz <<= -kvmclock->tsc_ಸರಿಸು;
    ಇಲ್ಲದಿದ್ದರೆ
        MHz >>= kvmclock->tsc_ಸರಿಸು;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ಸ್ಥಿರ ಶೂನ್ಯ physbits(ಪೂರ್ಣಾಂಕ qemu_quirk)
{
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ max, eax, ebx, ecx, edx;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ physbits;
    ಅಕ್ಷರ signature[13];
    ಪೂರ್ಣಾಂಕ pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    ಸ್ಮರಣೆ_ನಕಲು(signature + 0, &ebx, 4);
    ಸ್ಮರಣೆ_ನಕಲು(signature + 4, &edx, 4);
    ಸ್ಮರಣೆ_ನಕಲು(signature + 8, &ecx, 4);
    signature[12] = 0;
    ಆದರೆ (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    ಆದರೆ (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULong_ಬಹುಲಕ = !!(edx & (1 << 29));
    }

    ಆದರೆ (pae && CPULong_ಬಹುಲಕ && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        ಆದರೆ (!qemu_quirk) {
            valid = 1;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (physbits >= 41) {
            valid = 1;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(signature, "GenuineIntel") == 0) {
            ಆದರೆ ((physbits == 36) || (physbits == 39))
                valid = 1;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(signature, "AuthenticAMD") == 0) {
            ಆದರೆ (physbits == 40)
                valid = 1;
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULong_ಬಹುಲಕ ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    ಆದರೆ (valid)
        CPUPhys_ಬಿಟ್ಟುಗಳು = physbits;
}

ಸ್ಥಿರ ಶೂನ್ಯ qemu_detect(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU_HARDWARE)
        ಹಿಂತಿರುಗಿ;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    ಆದರೆ (v == 0x0000 || v == 0xffff)
        ಹಿಂತಿರುಗಿ;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    ಆದರೆ (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        ಹಿಂತಿರುಗಿ;

    PlatformRunningOn |= PF_QEMU;
    ಆಯ್ಕೆ (d) {
    ಸ್ಥಿತಿ 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        ನಿಲ್ಲಿಸು;
    }
    physbits(1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ qemu_early_e820(ಶೂನ್ಯ);

ಶೂನ್ಯ
qemu_preinit(ಶೂನ್ಯ)
{
    qemu_detect();
    kvm_detect();

    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (runningOnXen()) {
        xen_ramsize_preinit();
        ಹಿಂತಿರುಗಿ;
    }

    // try read e820 table first
    ಆದರೆ (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_ಕೆಳಗಿನ) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_ಮೇಲಿನ) << 24));
        ಆದರೆ (rs)
            rs += 16 * 1024 * 1024;
        ಇಲ್ಲದಿದ್ದರೆ
            rs = (((rtc_read(CMOS_MEM_EXTMEM_ಕೆಳಗಿನ) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_ಮೇಲಿನ) << 18))
                  + 1 * 1024 * 1024);
        Ram_ಗಾತ್ರ = rs;
        e820_ಕೂಡಿಸು(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_ಗಾತ್ರ);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_ಕೂಡಿಸು(0xfffc0000, 256*1024, E820_RESERVED);
}

#ವ್ಯಾಖ್ಯಾನಿಸು MSR_IA32_FEATURE_CONTROL 0x0000003a

ಸ್ಥಿರ ಶೂನ್ಯ msr_feature_control_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    u64 feature_control_ಬಿಟ್ಟುಗಳು = romfile_loadint("etc/msr_feature_control", 0);
    ಆದರೆ (feature_control_ಬಿಟ್ಟುಗಳು)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_ಬಿಟ್ಟುಗಳು);
}

ಶೂನ್ಯ
qemu_platform_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_ಸಿದ್ಧತೆ();
        xen_biostable_ಸಿದ್ಧತೆ();
        ಹಿಂತಿರುಗಿ;
    }

    kvmclock_init();

    // Initialize pci
    pci_ಸಿದ್ಧತೆ();
    smm_device_ಸಿದ್ಧತೆ();
    smm_ಸಿದ್ಧತೆ();

    // Initialize mtrr, msr_feature_control and smp
    mtrr_ಸಿದ್ಧತೆ();
    msr_feature_control_ಸಿದ್ಧತೆ();
    smp_ಸಿದ್ಧತೆ();

    // Create bios tables
    ಆದರೆ (Max_ಎಣಿಕೆ_CPUs <= 255) {
        pirtable_ಸಿದ್ಧತೆ();
        mptable_ಸಿದ್ಧತೆ();
    }
    smbios_ಸಿದ್ಧತೆ();

    ಆದರೆ (CONFIG_FW_ROMFILE_ತುಂಬು) {
        ಪೂರ್ಣಾಂಕ loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = ಕಂಡುಹಿಡಿ_acpi_rsdp();

        ಆದರೆ (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_ಸಿದ್ಧತೆ_acpi();
            ಹಿಂತಿರುಗಿ;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        ಆದರೆ (!loader_err)
            warn_internalerror();
    }

    acpi_ಸಿದ್ಧತೆ();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_SIGNATURE              0x00
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_ID                     0x01
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_UUID                   0x02
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_NOGRAPHIC              0x04
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_NUMA                   0x0d
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_BOOT_MENU              0x0e
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_NB_CPUS                0x05
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_MAX_CPUS               0x0f
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_FILE_DIR               0x19
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_ARCH_LOCAL             0x8000
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_E820_ಕೋಷ್ಟಕ             (QEMU_CFG_ARCH_LOCAL + 3)

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_select(u16 f)
{
    outw(f, ಪೋರ್ಟ್_QEMU_CFG_CTL);
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_dma_transfer(ಶೂನ್ಯ *ವಿಳಾಸ, u32 ಉದ್ದಳತೆ, u32 control)
{
    QemuCfgDmaAccess access;

    access.ವಿಳಾಸ = cpu_ಗಮ್ಯ_be64((u64)(u32)ವಿಳಾಸ);
    access.ಉದ್ದಳತೆ = cpu_ಗಮ್ಯ_be32(ಉದ್ದಳತೆ);
    access.control = cpu_ಗಮ್ಯ_be32(control);

    barrier();

    outl(cpu_ಗಮ್ಯ_be32((u32)&access), ಪೋರ್ಟ್_QEMU_CFG_DMA_ADDR_ಕೆಳಗಿನ);

    ಆಗ(be32_ಗಮ್ಯ_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_read(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ len)
{
    ಆದರೆ (len == 0) {
        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        insb(ಪೋರ್ಟ್_QEMU_CFG_ದತ್ತಾಂಶ, buf, len);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_write(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ len)
{
    ಆದರೆ (len == 0) {
        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } ಇಲ್ಲದಿದ್ದರೆ {
        warn_internalerror();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_skip(ಪೂರ್ಣಾಂಕ len)
{
    ಆದರೆ (len == 0) {
        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಆಗ (len--)
            inb(ಪೋರ್ಟ್_QEMU_CFG_ದತ್ತಾಂಶ);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_read_ನಮೂದು(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ e, ಪೂರ್ಣಾಂಕ len)
{
    ಆದರೆ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } ಇಲ್ಲದಿದ್ದರೆ {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_write_ನಮೂದು(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ e, ಪೂರ್ಣಾಂಕ len)
{
    ಆದರೆ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } ಇಲ್ಲದಿದ್ದರೆ {
        warn_internalerror();
    }
}

ರಚನೆ qemu_romfile_s {
    ರಚನೆ romfile_s file;
    ಪೂರ್ಣಾಂಕ select, skip;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
qemu_cfg_read_file(ರಚನೆ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ಆದರೆ (file->ಗಾತ್ರ_2 > maxlen)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ qemu_romfile_s *qfile;
    qfile = container_ದ(file, ರಚನೆ qemu_romfile_s, file);
    ಆದರೆ (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_ನಮೂದು(dst, qfile->select, file->ಗಾತ್ರ_2);
    } ಇಲ್ಲದಿದ್ದರೆ {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->ಗಾತ್ರ_2);
    }
    ಹಿಂತಿರುಗಿ file->ಗಾತ್ರ_2;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
ಪೂರ್ಣಾಂಕ
qemu_cfg_write_file_simple(ಶೂನ್ಯ *src, u16 ಕೀಲಿ, u32 ಸ್ಥಾನಾಂತರ_2, u32 len)
{
    ಆದರೆ (ಸ್ಥಾನಾಂತರ_2 == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_ನಮೂದು(src, ಕೀಲಿ, len);
    } ಇಲ್ಲದಿದ್ದರೆ {
        qemu_cfg_select(ಕೀಲಿ);
        qemu_cfg_skip(ಸ್ಥಾನಾಂತರ_2);
        qemu_cfg_write(src, len);
    }
    ಹಿಂತಿರುಗಿ len;
}

ಪೂರ್ಣಾಂಕ
qemu_cfg_write_file(ಶೂನ್ಯ *src, ರಚನೆ romfile_s *file, u32 ಸ್ಥಾನಾಂತರ_2, u32 len)
{
    ಆದರೆ ((ಸ್ಥಾನಾಂತರ_2 + len) > file->ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ qemu_cfg_write_file_simple(src, qemu_get_romfile_ಕೀಲಿ(file),
                                      ಸ್ಥಾನಾಂತರ_2, len);
}

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_romfile_ಕೂಡಿಸು(ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ select, ಪೂರ್ಣಾಂಕ skip, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
    ರಚನೆ qemu_romfile_s *qfile = malloc_tmp(ಗಾತ್ರ(*qfile));
    ಆದರೆ (!qfile) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(qfile, 0, ಗಾತ್ರ(*qfile));
    strtcpy(qfile->file.name, name, ಗಾತ್ರ(qfile->file.name));
    qfile->file.ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_ಕೂಡಿಸು(&qfile->file);
}

u16
qemu_get_romfile_ಕೀಲಿ(ರಚನೆ romfile_s *file)
{
    ರಚನೆ qemu_romfile_s *qfile;
    ಆದರೆ (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        ಹಿಂತಿರುಗಿ 0;
    }
    qfile = container_ದ(file, ರಚನೆ qemu_romfile_s, file);
    ಹಿಂತಿರುಗಿ qfile->select;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rtc_present(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_ಎಣಿಕೆ(ಶೂನ್ಯ)
{
    u16 smp_ಎಣಿಕೆ = 0;
    ಆದರೆ (qemu_cfg_enabled()) {
        qemu_cfg_read_ನಮೂದು(&smp_ಎಣಿಕೆ, QEMU_CFG_NB_CPUS, ಗಾತ್ರ(smp_ಎಣಿಕೆ));
    }
    ಆದರೆ (rtc_present()) {
        u16 cmos_cpu_ಎಣಿಕೆ = rtc_read(CMOS_BIOS_SMP_ಎಣಿಕೆ) + 1;
        ಆದರೆ (smp_ಎಣಿಕೆ < cmos_cpu_ಎಣಿಕೆ) {
            smp_ಎಣಿಕೆ = cmos_cpu_ಎಣಿಕೆ;
        }
    }
    ಹಿಂತಿರುಗಿ smp_ಎಣಿಕೆ;
}

ರಚನೆ e820_reservation {
    u64 ವಿಳಾಸ;
    u64 ಉದ್ದಳತೆ;
    u32 type;
};

#ವ್ಯಾಖ್ಯಾನಿಸು SMBIOS_ಕ್ಷೇತ್ರ_ನಮೂದು 0
#ವ್ಯಾಖ್ಯಾನಿಸು SMBIOS_ಕೋಷ್ಟಕ_ನಮೂದು 1

ರಚನೆ qemu_smbios_ಶೀರ್ಷಿಕೆ {
    u16 ಉದ್ದಳತೆ;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_e820(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (romfile_ಕಂಡುಹಿಡಿ("etc/e820")) {
        // qemu_early_e820() has handled everything
        ಹಿಂತಿರುಗಿ;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_ನಮೂದು(&count32, QEMU_CFG_E820_ಕೋಷ್ಟಕ, ಗಾತ್ರ(count32));
    ಆದರೆ (count32) {
        ರಚನೆ e820_reservation ನಮೂದು;
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i = 0; i < count32; i++) {
            qemu_cfg_read(&ನಮೂದು, ಗಾತ್ರ(ನಮೂದು));
            e820_ಕೂಡಿಸು(ನಮೂದು.ವಿಳಾಸ, ನಮೂದು.ಉದ್ದಳತೆ, ನಮೂದು.type);
        }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_ಕೂಡಿಸು(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 ಮೇಲಿನ = ((rtc_read(CMOS_MEM_HIGHMEM_ಕೆಳಗಿನ) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_ಮೇಲಿನ) << 32));
    Ram_ಗಾತ್ರ_Over4_G = ಮೇಲಿನ;
    e820_ಕೂಡಿಸು(0x100000000ull, ಮೇಲಿನ, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_ಗಾತ್ರ_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ಸ್ಥಿರ ಶೂನ್ಯ
qemu_cfg_legacy(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    // Misc config items.
    qemu_romfile_ಕೂಡಿಸು("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_ಕೂಡಿಸು("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_ಕೂಡಿಸು("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_ನಮೂದು(&numacount, QEMU_CFG_NUMA, ಗಾತ್ರ(numacount));
    ಪೂರ್ಣಾಂಕ max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_ಕೂಡಿಸು("etc/numa-cpu-map", QEMU_CFG_NUMA, ಗಾತ್ರ(numacount)
                     , max_cpu*ಗಾತ್ರ(u64));
    qemu_romfile_ಕೂಡಿಸು("etc/numa-nodes", QEMU_CFG_NUMA
                     , ಗಾತ್ರ(numacount) + max_cpu*ಗಾತ್ರ(u64)
                     , numacount*ಗಾತ್ರ(u64));

    // ACPI tables
    ಅಕ್ಷರ name[128];
    u16 cnt;
    qemu_cfg_read_ನಮೂದು(&cnt, QEMU_CFG_ACPI_TABLES, ಗಾತ್ರ(cnt));
    ಪೂರ್ಣಾಂಕ i, ಸ್ಥಾನಾಂತರ_2 = ಗಾತ್ರ(cnt);
    ಗಾಗಿ (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, ಗಾತ್ರ(len));
        ಸ್ಥಾನಾಂತರ_2 += ಗಾತ್ರ(len);
        snprintf(name, ಗಾತ್ರ(name), "acpi/table%d", i);
        qemu_romfile_ಕೂಡಿಸು(name, QEMU_CFG_ACPI_TABLES, ಸ್ಥಾನಾಂತರ_2, len);
        qemu_cfg_skip(len);
        ಸ್ಥಾನಾಂತರ_2 += len;
    }

    // SMBIOS info
    qemu_cfg_read_ನಮೂದು(&cnt, QEMU_CFG_SMBIOS_ENTRIES, ಗಾತ್ರ(cnt));
    ಸ್ಥಾನಾಂತರ_2 = ಗಾತ್ರ(cnt);
    ಗಾಗಿ (i = 0; i < cnt; i++) {
        ರಚನೆ qemu_smbios_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
        qemu_cfg_read(&ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ));
        ಆದರೆ (ಶೀರ್ಷಿಕೆ.headertype == SMBIOS_ಕ್ಷೇತ್ರ_ನಮೂದು) {
            snprintf(name, ಗಾತ್ರ(name), "smbios/field%d-%d"
                     , ಶೀರ್ಷಿಕೆ.tabletype, ಶೀರ್ಷಿಕೆ.fieldoffset);
            qemu_romfile_ಕೂಡಿಸು(name, QEMU_CFG_SMBIOS_ENTRIES
                             , ಸ್ಥಾನಾಂತರ_2 + ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ)
                             , ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ - ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ));
        } ಇಲ್ಲದಿದ್ದರೆ {
            snprintf(name, ಗಾತ್ರ(name), "smbios/table%d-%d"
                     , ಶೀರ್ಷಿಕೆ.tabletype, i);
            qemu_romfile_ಕೂಡಿಸು(name, QEMU_CFG_SMBIOS_ENTRIES
                             , ಸ್ಥಾನಾಂತರ_2 + 3, ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ - 3);
        }
        qemu_cfg_skip(ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ - ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ));
        ಸ್ಥಾನಾಂತರ_2 += ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ;
    }
}

ರಚನೆ QemuCfgFile {
    u32  ಗಾತ್ರ_2;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    ಅಕ್ಷರ name[56];
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ qemu_cfg_detect(ಶೂನ್ಯ)
{
    ಆದರೆ (cfg_enabled)
        ಹಿಂತಿರುಗಿ 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    ಅಕ್ಷರ *sig = "QEMU";
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < 4; i++)
        ಆದರೆ (inb(ಪೋರ್ಟ್_QEMU_CFG_ದತ್ತಾಂಶ) != sig[i])
            ಹಿಂತಿರುಗಿ 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_ನಮೂದು(&id, QEMU_CFG_ID, ಗಾತ್ರ(id));

    ಆದರೆ (id & QEMU_CFG_ಆವೃತ್ತಿ_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    ಹಿಂತಿರುಗಿ 1;
}

ಶೂನ್ಯ qemu_cfg_init(ಶೂನ್ಯ)
{
    ಆದರೆ (!runningOnQEMU())
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (!qemu_cfg_detect())
        ಹಿಂತಿರುಗಿ;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 ಎಣಿಕೆ_2;
    qemu_cfg_read_ನಮೂದು(&ಎಣಿಕೆ_2, QEMU_CFG_FILE_DIR, ಗಾತ್ರ(ಎಣಿಕೆ_2));
    ಎಣಿಕೆ_2 = be32_ಗಮ್ಯ_cpu(ಎಣಿಕೆ_2);
    u32 e;
    ಗಾಗಿ (e = 0; e < ಎಣಿಕೆ_2; e++) {
        ರಚನೆ QemuCfgFile qfile;
        qemu_cfg_read(&qfile, ಗಾತ್ರ(qfile));
        qemu_romfile_ಕೂಡಿಸು(qfile.name, be16_ಗಮ್ಯ_cpu(qfile.select)
                         , 0, be32_ಗಮ್ಯ_cpu(qfile.ಗಾತ್ರ_2));
    }

    qemu_cfg_e820();

    ಆದರೆ (romfile_ಕಂಡುಹಿಡಿ("etc/table-loader")) {
        acpi_pm_ತಳ = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_ತಳ);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_ನಮೂದು(&nogfx, QEMU_CFG_NOGRAPHIC, ಗಾತ್ರ(nogfx));
    ಆದರೆ (nogfx && !romfile_ಕಂಡುಹಿಡಿ("etc/sercon-port")
        && !romfile_ಕಂಡುಹಿಡಿ("vgaroms/sgabios.bin"))
        const_romfile_ಕೂಡಿಸು_int("etc/sercon-port", ಪೋರ್ಟ್_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ qemu_early_e820(ಶೂನ್ಯ)
{
    ರಚನೆ e820_reservation ಕೋಷ್ಟಕ;
    ರಚನೆ QemuCfgFile qfile;
    u32 select = 0, ಗಾತ್ರ_2 = 0;
    u32 ಎಣಿಕೆ_2, i;

    ಆದರೆ (!qemu_cfg_detect())
        ಹಿಂತಿರುಗಿ 0;

    // find e820 table
    qemu_cfg_read_ನಮೂದು(&ಎಣಿಕೆ_2, QEMU_CFG_FILE_DIR, ಗಾತ್ರ(ಎಣಿಕೆ_2));
    ಎಣಿಕೆ_2 = be32_ಗಮ್ಯ_cpu(ಎಣಿಕೆ_2);
    ಗಾಗಿ (i = 0; i < ಎಣಿಕೆ_2; i++) {
        qemu_cfg_read(&qfile, ಗಾತ್ರ(qfile));
        ಆದರೆ (memcmp(qfile.name, "etc/e820", 9) != 0)
            ಮುಂದುವರಿ;
        select = be16_ಗಮ್ಯ_cpu(qfile.select);
        ಗಾತ್ರ_2 = be32_ಗಮ್ಯ_cpu(qfile.ಗಾತ್ರ_2);
        ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    ಎಣಿಕೆ_2 = ಗಾತ್ರ_2/ಗಾತ್ರ(ಕೋಷ್ಟಕ);
    ಗಾಗಿ (i = 0, select = 0; i < ಎಣಿಕೆ_2; i++) {
        qemu_cfg_read(&ಕೋಷ್ಟಕ, ಗಾತ್ರ(ಕೋಷ್ಟಕ));
        ಆಯ್ಕೆ (ಕೋಷ್ಟಕ.type) {
        ಸ್ಥಿತಿ E820_RESERVED:
            e820_ಕೂಡಿಸು(ಕೋಷ್ಟಕ.ವಿಳಾಸ, ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ, ಕೋಷ್ಟಕ.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    ಕೋಷ್ಟಕ.ವಿಳಾಸ, ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ E820_SOFT_RESERVED:
            e820_ಕೂಡಿಸು(ಕೋಷ್ಟಕ.ವಿಳಾಸ, ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ, ಕೋಷ್ಟಕ.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    ಕೋಷ್ಟಕ.ವಿಳಾಸ, ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ E820_RAM:
            e820_ಕೂಡಿಸು(ಕೋಷ್ಟಕ.ವಿಳಾಸ, ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ, ಕೋಷ್ಟಕ.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    ಕೋಷ್ಟಕ.ವಿಳಾಸ, ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ);
            ಆದರೆ (ಕೋಷ್ಟಕ.ವಿಳಾಸ < 0x100000000LL) {
                // below 4g
                ಆದರೆ (Ram_ಗಾತ್ರ < ಕೋಷ್ಟಕ.ವಿಳಾಸ + ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ)
                    Ram_ಗಾತ್ರ = ಕೋಷ್ಟಕ.ವಿಳಾಸ + ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ;
            } ಇಲ್ಲದಿದ್ದರೆ {
                // above 4g
                ಆದರೆ (Ram_ಗಾತ್ರ_Over4_G < ಕೋಷ್ಟಕ.ವಿಳಾಸ + ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ - 0x100000000LL)
                    Ram_ಗಾತ್ರ_Over4_G = ಕೋಷ್ಟಕ.ವಿಳಾಸ + ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_ಗಾತ್ರ);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_ಗಾತ್ರ_Over4_G);
    ಹಿಂತಿರುಗಿ 1;
}
