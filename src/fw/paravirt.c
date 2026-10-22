// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // be32_to_cpu
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_QEMU
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // e820_add
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_readw
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // pci_probe_devices
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_DEVICE_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/serialio.h" // PORT_SERIAL1
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // CMOS_*
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/virtio-mmio.h" // virtio_mmio_acpi
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_tmp
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // qemu_cfg_preinit
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile_loader.h" // romfile_loader_execute
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // pci_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // cpuid
#ਅਨ੍ਤਰ੍ਭਾਵ "xen.h" // xen_biostable_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 Ram_ਮਾਪ;
// Amount of continuous ram >4Gig
u64 Ram_ਮਾਪ_Over4_G;
// physical address space bits
u8 CPUPhys_ਬਿੱਟ;
// 64bit processor
u8 CPULong_ਬਹੁਲਕ;
// Type of emulator platform.
ਪੂਰਨ PlatformRunningOn VARFSEG;
// cfg enabled
ਪੂਰਨ cfg_enabled = 0;
// cfg_dma enabled
ਪੂਰਨ cfg_dma_enabled = 0;

ਸਤਰਅੰਦਰ ਪੂਰਨ qemu_cfg_enabled(ਖਾਲੀ)
{
    ਵਾਪਸ cfg_enabled;
}

ਸਤਰਅੰਦਰ ਪੂਰਨ qemu_cfg_dma_enabled(ਖਾਲੀ)
{
    ਵਾਪਸ cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KVM_CPUID_SIGNATURE     0x40000000
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ kvm_cpuid_ਅਧਾਰ = 0;

ਸਥਿਰ ਖਾਲੀ kvm_detect(ਖਾਲੀ)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, max = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ eax, ebx, ecx, edx;
    ਅੱਖਰ signature[13];

    ਲਈ (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        ਜੇ (eax < i)
            ਰੋਕੋ;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 0, &ebx, 4);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 4, &ecx, 4);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        ਜੇ (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_ਅਧਾਰ = i;
            max = eax;
        }
    }

    ਜੇ (kvm_cpuid_ਅਧਾਰ) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        ਜੇ (max >= kvm_cpuid_ਅਧਾਰ + 0x10) {
            cpuid(kvm_cpuid_ਅਧਾਰ + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KVM_FEATURE_CLOCKSOURCE           0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KVM_FEATURE_CLOCKSOURCE2          3

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_KVM_SYSTEM_ਸਮਾਂ            0x12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_KVM_SYSTEM_ਸਮਾਂ_NEW  0x4b564d01

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVCLOCK_TSC_STABLE_ਦੋਆਧਾਰੀ_ਅੰਕ     (1 << 0)

ਬਣਤਰ pvclock_vcpu_ਸਮਾਂ_info *kvmclock;

ਸਥਿਰ ਖਾਲੀ kvmclock_init(ਖਾਲੀ)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ eax, ebx, ecx, edx, msr;

    ਜੇ (!runningOnKVM())
        ਵਾਪਸ;

    cpuid(kvm_cpuid_ਅਧਾਰ + 0x01, &eax, &ebx, &ecx, &edx);
    ਜੇ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_ਸਮਾਂ_NEW;
    ਨਹੀਂਤਾਂ ਜੇ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_ਸਮਾਂ;
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ;

    kvmclock = memalign_ਨੀਵਾਂ(ਆਕਾਰ(*kvmclock), 32);
    memset(kvmclock, 0, ਆਕਾਰ(*kvmclock));
    u32 ਮੁੱਲ = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", ਮੁੱਲ, msr);
    wrmsr(msr, ਮੁੱਲ | 0x01);

    ਜੇ (!(kvmclock->flags & PVCLOCK_TSC_STABLE_ਦੋਆਧਾਰੀ_ਅੰਕ))
        ਵਾਪਸ;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_ਮੰਜ਼ਿਲ_system_mul >> 16);
    ਜੇ (kvmclock->tsc_ਖਿਸਕਾਓ < 0)
        MHz <<= -kvmclock->tsc_ਖਿਸਕਾਓ;
    ਨਹੀਂਤਾਂ
        MHz >>= kvmclock->tsc_ਖਿਸਕਾਓ;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ਸਥਿਰ ਖਾਲੀ physbits(ਪੂਰਨ qemu_quirk)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ max, eax, ebx, ecx, edx;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ physbits;
    ਅੱਖਰ signature[13];
    ਪੂਰਨ pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 0, &ebx, 4);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 4, &edx, 4);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 8, &ecx, 4);
    signature[12] = 0;
    ਜੇ (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    ਜੇ (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULong_ਬਹੁਲਕ = !!(edx & (1 << 29));
    }

    ਜੇ (pae && CPULong_ਬਹੁਲਕ && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        ਜੇ (!qemu_quirk) {
            valid = 1;
        } ਨਹੀਂਤਾਂ ਜੇ (physbits >= 41) {
            valid = 1;
        } ਨਹੀਂਤਾਂ ਜੇ (strcmp(signature, "GenuineIntel") == 0) {
            ਜੇ ((physbits == 36) || (physbits == 39))
                valid = 1;
        } ਨਹੀਂਤਾਂ ਜੇ (strcmp(signature, "AuthenticAMD") == 0) {
            ਜੇ (physbits == 40)
                valid = 1;
        }
    } ਨਹੀਂਤਾਂ {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULong_ਬਹੁਲਕ ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    ਜੇ (valid)
        CPUPhys_ਬਿੱਟ = physbits;
}

ਸਥਿਰ ਖਾਲੀ qemu_detect(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU_HARDWARE)
        ਵਾਪਸ;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    ਜੇ (v == 0x0000 || v == 0xffff)
        ਵਾਪਸ;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    ਜੇ (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        ਵਾਪਸ;

    PlatformRunningOn |= PF_QEMU;
    ਚੋਣ (d) {
    ਹਾਲਤ 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        ਰੋਕੋ;
    ਹਾਲਤ 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        ਰੋਕੋ;
    ਮੂਲ:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        ਰੋਕੋ;
    }
    physbits(1);
}

ਸਥਿਰ ਪੂਰਨ qemu_early_e820(ਖਾਲੀ);

ਖਾਲੀ
qemu_preinit(ਖਾਲੀ)
{
    qemu_detect();
    kvm_detect();

    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    ਜੇ (runningOnXen()) {
        xen_ramsize_preinit();
        ਵਾਪਸ;
    }

    // try read e820 table first
    ਜੇ (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_ਨੀਵਾਂ) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_ਉੱਚਾ) << 24));
        ਜੇ (rs)
            rs += 16 * 1024 * 1024;
        ਨਹੀਂਤਾਂ
            rs = (((rtc_read(CMOS_MEM_EXTMEM_ਨੀਵਾਂ) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_ਉੱਚਾ) << 18))
                  + 1 * 1024 * 1024);
        Ram_ਮਾਪ = rs;
        e820_ਜੋੜੋ(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_ਮਾਪ);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_ਜੋੜੋ(0xfffc0000, 256*1024, E820_RESERVED);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_IA32_FEATURE_CONTROL 0x0000003a

ਸਥਿਰ ਖਾਲੀ msr_feature_control_ਤਿਆਰੀ(ਖਾਲੀ)
{
    u64 feature_control_ਬਿੱਟ = romfile_loadint("etc/msr_feature_control", 0);
    ਜੇ (feature_control_ਬਿੱਟ)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_ਬਿੱਟ);
}

ਖਾਲੀ
qemu_platform_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    ਜੇ (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_ਤਿਆਰੀ();
        xen_biostable_ਤਿਆਰੀ();
        ਵਾਪਸ;
    }

    kvmclock_init();

    // Initialize pci
    pci_ਤਿਆਰੀ();
    smm_device_ਤਿਆਰੀ();
    smm_ਤਿਆਰੀ();

    // Initialize mtrr, msr_feature_control and smp
    mtrr_ਤਿਆਰੀ();
    msr_feature_control_ਤਿਆਰੀ();
    smp_ਤਿਆਰੀ();

    // Create bios tables
    ਜੇ (Max_ਗਿਣਤੀ_CPUs <= 255) {
        pirtable_ਤਿਆਰੀ();
        mptable_ਤਿਆਰੀ();
    }
    smbios_ਤਿਆਰੀ();

    ਜੇ (CONFIG_FW_ROMFILE_ਲੱਦੋ) {
        ਪੂਰਨ loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = ਲੱਭੋ_acpi_rsdp();

        ਜੇ (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_ਤਿਆਰੀ_acpi();
            ਵਾਪਸ;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        ਜੇ (!loader_err)
            warn_internalerror();
    }

    acpi_ਤਿਆਰੀ();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_SIGNATURE              0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_ID                     0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_UUID                   0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_NOGRAPHIC              0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_NUMA                   0x0d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_BOOT_MENU              0x0e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_NB_CPUS                0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_MAX_CPUS               0x0f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_FILE_DIR               0x19
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_ARCH_LOCAL             0x8000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_E820_ਸਾਰਣੀ             (QEMU_CFG_ARCH_LOCAL + 3)

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_select(u16 f)
{
    outw(f, ਪੋਰਟ_QEMU_CFG_CTL);
}

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_dma_transfer(ਖਾਲੀ *ਪਤਾ, u32 ਲੰਮਾਈ, u32 control)
{
    QemuCfgDmaAccess access;

    access.ਪਤਾ = cpu_ਮੰਜ਼ਿਲ_be64((u64)(u32)ਪਤਾ);
    access.ਲੰਮਾਈ = cpu_ਮੰਜ਼ਿਲ_be32(ਲੰਮਾਈ);
    access.control = cpu_ਮੰਜ਼ਿਲ_be32(control);

    barrier();

    outl(cpu_ਮੰਜ਼ਿਲ_be32((u32)&access), ਪੋਰਟ_QEMU_CFG_DMA_ADDR_ਨੀਵਾਂ);

    ਜਦੋਂ(be32_ਮੰਜ਼ਿਲ_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_read(ਖਾਲੀ *buf, ਪੂਰਨ len)
{
    ਜੇ (len == 0) {
        ਵਾਪਸ;
    }

    ਜੇ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } ਨਹੀਂਤਾਂ {
        insb(ਪੋਰਟ_QEMU_CFG_ਅੰਕੜੇ, buf, len);
    }
}

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_write(ਖਾਲੀ *buf, ਪੂਰਨ len)
{
    ਜੇ (len == 0) {
        ਵਾਪਸ;
    }

    ਜੇ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } ਨਹੀਂਤਾਂ {
        warn_internalerror();
    }
}

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_skip(ਪੂਰਨ len)
{
    ਜੇ (len == 0) {
        ਵਾਪਸ;
    }

    ਜੇ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } ਨਹੀਂਤਾਂ {
        ਜਦੋਂ (len--)
            inb(ਪੋਰਟ_QEMU_CFG_ਅੰਕੜੇ);
    }
}

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_read_ਇੰਦਰਾਜ(ਖਾਲੀ *buf, ਪੂਰਨ e, ਪੂਰਨ len)
{
    ਜੇ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } ਨਹੀਂਤਾਂ {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_write_ਇੰਦਰਾਜ(ਖਾਲੀ *buf, ਪੂਰਨ e, ਪੂਰਨ len)
{
    ਜੇ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } ਨਹੀਂਤਾਂ {
        warn_internalerror();
    }
}

ਬਣਤਰ qemu_romfile_s {
    ਬਣਤਰ romfile_s file;
    ਪੂਰਨ select, skip;
};

ਸਥਿਰ ਪੂਰਨ
qemu_cfg_read_file(ਬਣਤਰ romfile_s *file, ਖਾਲੀ *dst, u32 maxlen)
{
    ਜੇ (file->ਮਾਪ > maxlen)
        ਵਾਪਸ -1;
    ਬਣਤਰ qemu_romfile_s *qfile;
    qfile = container_ਦਾ(file, ਬਣਤਰ qemu_romfile_s, file);
    ਜੇ (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_ਇੰਦਰਾਜ(dst, qfile->select, file->ਮਾਪ);
    } ਨਹੀਂਤਾਂ {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->ਮਾਪ);
    }
    ਵਾਪਸ file->ਮਾਪ;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
ਪੂਰਨ
qemu_cfg_write_file_simple(ਖਾਲੀ *src, u16 ਕੁੰਜੀ, u32 ਵਿੱਥ_2, u32 len)
{
    ਜੇ (ਵਿੱਥ_2 == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_ਇੰਦਰਾਜ(src, ਕੁੰਜੀ, len);
    } ਨਹੀਂਤਾਂ {
        qemu_cfg_select(ਕੁੰਜੀ);
        qemu_cfg_skip(ਵਿੱਥ_2);
        qemu_cfg_write(src, len);
    }
    ਵਾਪਸ len;
}

ਪੂਰਨ
qemu_cfg_write_file(ਖਾਲੀ *src, ਬਣਤਰ romfile_s *file, u32 ਵਿੱਥ_2, u32 len)
{
    ਜੇ ((ਵਿੱਥ_2 + len) > file->ਮਾਪ)
        ਵਾਪਸ -1;

    ਜੇ (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        ਵਾਪਸ -1;
    }
    ਵਾਪਸ qemu_cfg_write_file_simple(src, qemu_get_romfile_ਕੁੰਜੀ(file),
                                      ਵਿੱਥ_2, len);
}

ਸਥਿਰ ਖਾਲੀ
qemu_romfile_ਜੋੜੋ(ਅੱਖਰ *name, ਪੂਰਨ select, ਪੂਰਨ skip, ਪੂਰਨ ਮਾਪ)
{
    ਬਣਤਰ qemu_romfile_s *qfile = malloc_tmp(ਆਕਾਰ(*qfile));
    ਜੇ (!qfile) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(qfile, 0, ਆਕਾਰ(*qfile));
    strtcpy(qfile->file.name, name, ਆਕਾਰ(qfile->file.name));
    qfile->file.ਮਾਪ = ਮਾਪ;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_ਜੋੜੋ(&qfile->file);
}

u16
qemu_get_romfile_ਕੁੰਜੀ(ਬਣਤਰ romfile_s *file)
{
    ਬਣਤਰ qemu_romfile_s *qfile;
    ਜੇ (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        ਵਾਪਸ 0;
    }
    qfile = container_ਦਾ(file, ਬਣਤਰ qemu_romfile_s, file);
    ਵਾਪਸ qfile->select;
}

ਸਥਿਰ ਪੂਰਨ rtc_present(ਖਾਲੀ)
{
    ਵਾਪਸ rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_ਗਿਣਤੀ(ਖਾਲੀ)
{
    u16 smp_ਗਿਣਤੀ = 0;
    ਜੇ (qemu_cfg_enabled()) {
        qemu_cfg_read_ਇੰਦਰਾਜ(&smp_ਗਿਣਤੀ, QEMU_CFG_NB_CPUS, ਆਕਾਰ(smp_ਗਿਣਤੀ));
    }
    ਜੇ (rtc_present()) {
        u16 cmos_cpu_ਗਿਣਤੀ = rtc_read(CMOS_BIOS_SMP_ਗਿਣਤੀ) + 1;
        ਜੇ (smp_ਗਿਣਤੀ < cmos_cpu_ਗਿਣਤੀ) {
            smp_ਗਿਣਤੀ = cmos_cpu_ਗਿਣਤੀ;
        }
    }
    ਵਾਪਸ smp_ਗਿਣਤੀ;
}

ਬਣਤਰ e820_reservation {
    u64 ਪਤਾ;
    u64 ਲੰਮਾਈ;
    u32 type;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMBIOS_ਖੇਤਰ_ਇੰਦਰਾਜ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMBIOS_ਸਾਰਣੀ_ਇੰਦਰਾਜ 1

ਬਣਤਰ qemu_smbios_ਸਿਰਲੇਖ {
    u16 ਲੰਮਾਈ;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ਸਥਿਰ ਖਾਲੀ
qemu_cfg_e820(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    ਜੇ (romfile_ਲੱਭੋ("etc/e820")) {
        // qemu_early_e820() has handled everything
        ਵਾਪਸ;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_ਇੰਦਰਾਜ(&count32, QEMU_CFG_E820_ਸਾਰਣੀ, ਆਕਾਰ(count32));
    ਜੇ (count32) {
        ਬਣਤਰ e820_reservation ਇੰਦਰਾਜ;
        ਪੂਰਨ i;
        ਲਈ (i = 0; i < count32; i++) {
            qemu_cfg_read(&ਇੰਦਰਾਜ, ਆਕਾਰ(ਇੰਦਰਾਜ));
            e820_ਜੋੜੋ(ਇੰਦਰਾਜ.ਪਤਾ, ਇੰਦਰਾਜ.ਲੰਮਾਈ, ਇੰਦਰਾਜ.type);
        }
    } ਨਹੀਂਤਾਂ ਜੇ (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_ਜੋੜੋ(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 ਉੱਚਾ = ((rtc_read(CMOS_MEM_HIGHMEM_ਨੀਵਾਂ) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_ਉੱਚਾ) << 32));
    Ram_ਮਾਪ_Over4_G = ਉੱਚਾ;
    e820_ਜੋੜੋ(0x100000000ull, ਉੱਚਾ, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_ਮਾਪ_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ਸਥਿਰ ਖਾਲੀ
qemu_cfg_legacy(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    // Misc config items.
    qemu_romfile_ਜੋੜੋ("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_ਜੋੜੋ("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_ਜੋੜੋ("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_ਇੰਦਰਾਜ(&numacount, QEMU_CFG_NUMA, ਆਕਾਰ(numacount));
    ਪੂਰਨ max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_ਜੋੜੋ("etc/numa-cpu-map", QEMU_CFG_NUMA, ਆਕਾਰ(numacount)
                     , max_cpu*ਆਕਾਰ(u64));
    qemu_romfile_ਜੋੜੋ("etc/numa-nodes", QEMU_CFG_NUMA
                     , ਆਕਾਰ(numacount) + max_cpu*ਆਕਾਰ(u64)
                     , numacount*ਆਕਾਰ(u64));

    // ACPI tables
    ਅੱਖਰ name[128];
    u16 cnt;
    qemu_cfg_read_ਇੰਦਰਾਜ(&cnt, QEMU_CFG_ACPI_TABLES, ਆਕਾਰ(cnt));
    ਪੂਰਨ i, ਵਿੱਥ_2 = ਆਕਾਰ(cnt);
    ਲਈ (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, ਆਕਾਰ(len));
        ਵਿੱਥ_2 += ਆਕਾਰ(len);
        snprintf(name, ਆਕਾਰ(name), "acpi/table%d", i);
        qemu_romfile_ਜੋੜੋ(name, QEMU_CFG_ACPI_TABLES, ਵਿੱਥ_2, len);
        qemu_cfg_skip(len);
        ਵਿੱਥ_2 += len;
    }

    // SMBIOS info
    qemu_cfg_read_ਇੰਦਰਾਜ(&cnt, QEMU_CFG_SMBIOS_ENTRIES, ਆਕਾਰ(cnt));
    ਵਿੱਥ_2 = ਆਕਾਰ(cnt);
    ਲਈ (i = 0; i < cnt; i++) {
        ਬਣਤਰ qemu_smbios_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
        qemu_cfg_read(&ਸਿਰਲੇਖ, ਆਕਾਰ(ਸਿਰਲੇਖ));
        ਜੇ (ਸਿਰਲੇਖ.headertype == SMBIOS_ਖੇਤਰ_ਇੰਦਰਾਜ) {
            snprintf(name, ਆਕਾਰ(name), "smbios/field%d-%d"
                     , ਸਿਰਲੇਖ.tabletype, ਸਿਰਲੇਖ.fieldoffset);
            qemu_romfile_ਜੋੜੋ(name, QEMU_CFG_SMBIOS_ENTRIES
                             , ਵਿੱਥ_2 + ਆਕਾਰ(ਸਿਰਲੇਖ)
                             , ਸਿਰਲੇਖ.ਲੰਮਾਈ - ਆਕਾਰ(ਸਿਰਲੇਖ));
        } ਨਹੀਂਤਾਂ {
            snprintf(name, ਆਕਾਰ(name), "smbios/table%d-%d"
                     , ਸਿਰਲੇਖ.tabletype, i);
            qemu_romfile_ਜੋੜੋ(name, QEMU_CFG_SMBIOS_ENTRIES
                             , ਵਿੱਥ_2 + 3, ਸਿਰਲੇਖ.ਲੰਮਾਈ - 3);
        }
        qemu_cfg_skip(ਸਿਰਲੇਖ.ਲੰਮਾਈ - ਆਕਾਰ(ਸਿਰਲੇਖ));
        ਵਿੱਥ_2 += ਸਿਰਲੇਖ.ਲੰਮਾਈ;
    }
}

ਬਣਤਰ QemuCfgFile {
    u32  ਮਾਪ;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    ਅੱਖਰ name[56];
};

ਸਥਿਰ ਪੂਰਨ qemu_cfg_detect(ਖਾਲੀ)
{
    ਜੇ (cfg_enabled)
        ਵਾਪਸ 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    ਅੱਖਰ *sig = "QEMU";
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < 4; i++)
        ਜੇ (inb(ਪੋਰਟ_QEMU_CFG_ਅੰਕੜੇ) != sig[i])
            ਵਾਪਸ 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_ਇੰਦਰਾਜ(&id, QEMU_CFG_ID, ਆਕਾਰ(id));

    ਜੇ (id & QEMU_CFG_ਸੰਸਕਰਣ_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    ਵਾਪਸ 1;
}

ਖਾਲੀ qemu_cfg_init(ਖਾਲੀ)
{
    ਜੇ (!runningOnQEMU())
        ਵਾਪਸ;

    ਜੇ (!qemu_cfg_detect())
        ਵਾਪਸ;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 ਗਿਣਤੀ_2;
    qemu_cfg_read_ਇੰਦਰਾਜ(&ਗਿਣਤੀ_2, QEMU_CFG_FILE_DIR, ਆਕਾਰ(ਗਿਣਤੀ_2));
    ਗਿਣਤੀ_2 = be32_ਮੰਜ਼ਿਲ_cpu(ਗਿਣਤੀ_2);
    u32 e;
    ਲਈ (e = 0; e < ਗਿਣਤੀ_2; e++) {
        ਬਣਤਰ QemuCfgFile qfile;
        qemu_cfg_read(&qfile, ਆਕਾਰ(qfile));
        qemu_romfile_ਜੋੜੋ(qfile.name, be16_ਮੰਜ਼ਿਲ_cpu(qfile.select)
                         , 0, be32_ਮੰਜ਼ਿਲ_cpu(qfile.ਮਾਪ));
    }

    qemu_cfg_e820();

    ਜੇ (romfile_ਲੱਭੋ("etc/table-loader")) {
        acpi_pm_ਅਧਾਰ = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_ਅਧਾਰ);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_ਇੰਦਰਾਜ(&nogfx, QEMU_CFG_NOGRAPHIC, ਆਕਾਰ(nogfx));
    ਜੇ (nogfx && !romfile_ਲੱਭੋ("etc/sercon-port")
        && !romfile_ਲੱਭੋ("vgaroms/sgabios.bin"))
        const_romfile_ਜੋੜੋ_int("etc/sercon-port", ਪੋਰਟ_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ਸਥਿਰ ਪੂਰਨ qemu_early_e820(ਖਾਲੀ)
{
    ਬਣਤਰ e820_reservation ਸਾਰਣੀ;
    ਬਣਤਰ QemuCfgFile qfile;
    u32 select = 0, ਮਾਪ = 0;
    u32 ਗਿਣਤੀ_2, i;

    ਜੇ (!qemu_cfg_detect())
        ਵਾਪਸ 0;

    // find e820 table
    qemu_cfg_read_ਇੰਦਰਾਜ(&ਗਿਣਤੀ_2, QEMU_CFG_FILE_DIR, ਆਕਾਰ(ਗਿਣਤੀ_2));
    ਗਿਣਤੀ_2 = be32_ਮੰਜ਼ਿਲ_cpu(ਗਿਣਤੀ_2);
    ਲਈ (i = 0; i < ਗਿਣਤੀ_2; i++) {
        qemu_cfg_read(&qfile, ਆਕਾਰ(qfile));
        ਜੇ (memcmp(qfile.name, "etc/e820", 9) != 0)
            ਜਾਰੀ;
        select = be16_ਮੰਜ਼ਿਲ_cpu(qfile.select);
        ਮਾਪ = be32_ਮੰਜ਼ਿਲ_cpu(qfile.ਮਾਪ);
        ਰੋਕੋ;
    }
    ਜੇ (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        ਵਾਪਸ 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    ਗਿਣਤੀ_2 = ਮਾਪ/ਆਕਾਰ(ਸਾਰਣੀ);
    ਲਈ (i = 0, select = 0; i < ਗਿਣਤੀ_2; i++) {
        qemu_cfg_read(&ਸਾਰਣੀ, ਆਕਾਰ(ਸਾਰਣੀ));
        ਚੋਣ (ਸਾਰਣੀ.type) {
        ਹਾਲਤ E820_RESERVED:
            e820_ਜੋੜੋ(ਸਾਰਣੀ.ਪਤਾ, ਸਾਰਣੀ.ਲੰਮਾਈ, ਸਾਰਣੀ.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    ਸਾਰਣੀ.ਪਤਾ, ਸਾਰਣੀ.ਲੰਮਾਈ);
            ਰੋਕੋ;
        ਹਾਲਤ E820_SOFT_RESERVED:
            e820_ਜੋੜੋ(ਸਾਰਣੀ.ਪਤਾ, ਸਾਰਣੀ.ਲੰਮਾਈ, ਸਾਰਣੀ.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    ਸਾਰਣੀ.ਪਤਾ, ਸਾਰਣੀ.ਲੰਮਾਈ);
            ਰੋਕੋ;
        ਹਾਲਤ E820_RAM:
            e820_ਜੋੜੋ(ਸਾਰਣੀ.ਪਤਾ, ਸਾਰਣੀ.ਲੰਮਾਈ, ਸਾਰਣੀ.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    ਸਾਰਣੀ.ਪਤਾ, ਸਾਰਣੀ.ਲੰਮਾਈ);
            ਜੇ (ਸਾਰਣੀ.ਪਤਾ < 0x100000000LL) {
                // below 4g
                ਜੇ (Ram_ਮਾਪ < ਸਾਰਣੀ.ਪਤਾ + ਸਾਰਣੀ.ਲੰਮਾਈ)
                    Ram_ਮਾਪ = ਸਾਰਣੀ.ਪਤਾ + ਸਾਰਣੀ.ਲੰਮਾਈ;
            } ਨਹੀਂਤਾਂ {
                // above 4g
                ਜੇ (Ram_ਮਾਪ_Over4_G < ਸਾਰਣੀ.ਪਤਾ + ਸਾਰਣੀ.ਲੰਮਾਈ - 0x100000000LL)
                    Ram_ਮਾਪ_Over4_G = ਸਾਰਣੀ.ਪਤਾ + ਸਾਰਣੀ.ਲੰਮਾਈ - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_ਮਾਪ);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_ਮਾਪ_Over4_G);
    ਵਾਪਸ 1;
}
