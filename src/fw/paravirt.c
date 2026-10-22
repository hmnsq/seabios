// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "byteorder.h" // be32_to_cpu
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_QEMU
#ଅନ୍ତର୍ଭାଵ "e820map.h" // e820_add
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_config_readw
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // pci_probe_devices
#ଅନ୍ତର୍ଭାଵ "hw/pci_regs.h" // PCI_DEVICE_ID
#ଅନ୍ତର୍ଭାଵ "hw/serialio.h" // PORT_SERIAL1
#ଅନ୍ତର୍ଭାଵ "hw/rtc.h" // CMOS_*
#ଅନ୍ତର୍ଭାଵ "hw/virtio-mmio.h" // virtio_mmio_acpi
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_tmp
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "paravirt.h" // qemu_cfg_preinit
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "romfile_loader.h" // romfile_loader_execute
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // pci_setup
#ଅନ୍ତର୍ଭାଵ "x86.h" // cpuid
#ଅନ୍ତର୍ଭାଵ "xen.h" // xen_biostable_setup
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା PlatformRunningOn VARFSEG;
// cfg enabled
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfg_enabled = 0;
// cfg_dma enabled
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfg_dma_enabled = 0;

ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା qemu_cfg_enabled(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ cfg_enabled;
}

ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା qemu_cfg_dma_enabled(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvm_cpuid_base = 0;

ସ୍ଥିର ଶୂନ୍ୟ kvm_detect(ଶୂନ୍ୟ)
{
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, max = 0;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା eax, ebx, ecx, edx;
    ଅକ୍ଷର signature[13];

    ପାଇଁ (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        ଯଦି (eax < i)
            ଭାଙ୍ଗ;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        ଯଦି (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    ଯଦି (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        ଯଦି (max >= kvm_cpuid_base + 0x10) {
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

ସଂରଚନା pvclock_vcpu_time_info *kvmclock;

ସ୍ଥିର ଶୂନ୍ୟ kvmclock_init(ଶୂନ୍ୟ)
{
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା eax, ebx, ecx, edx, msr;

    ଯଦି (!runningOnKVM())
        ଫେରନ୍ତୁ;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    ଯଦି (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    ଅନ୍ୟଥା ଯଦି (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    ଅନ୍ୟଥା
        ଫେରନ୍ତୁ;

    kvmclock = memalign_ତଳ(ଆକାର(*kvmclock), 32);
    memset(kvmclock, 0, ଆକାର(*kvmclock));
    u32 ମୂଲ୍ୟ = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", ମୂଲ୍ୟ, msr);
    wrmsr(msr, ମୂଲ୍ୟ | 0x01);

    ଯଦି (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        ଫେରନ୍ତୁ;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    ଯଦି (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    ଅନ୍ୟଥା
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ସ୍ଥିର ଶୂନ୍ୟ physbits(ପୂର୍ଣ୍ଣସଂଖ୍ୟା qemu_quirk)
{
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା max, eax, ebx, ecx, edx;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା physbits;
    ଅକ୍ଷର signature[13];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    ଯଦି (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    ଯଦି (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    ଯଦି (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        ଯଦି (!qemu_quirk) {
            valid = 1;
        } ଅନ୍ୟଥା ଯଦି (physbits >= 41) {
            valid = 1;
        } ଅନ୍ୟଥା ଯଦି (strcmp(signature, "GenuineIntel") == 0) {
            ଯଦି ((physbits == 36) || (physbits == 39))
                valid = 1;
        } ଅନ୍ୟଥା ଯଦି (strcmp(signature, "AuthenticAMD") == 0) {
            ଯଦି (physbits == 40)
                valid = 1;
        }
    } ଅନ୍ୟଥା {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    ଯଦି (valid)
        CPUPhysBits = physbits;
}

ସ୍ଥିର ଶୂନ୍ୟ qemu_detect(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU_HARDWARE)
        ଫେରନ୍ତୁ;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    ଯଦି (v == 0x0000 || v == 0xffff)
        ଫେରନ୍ତୁ;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    ଯଦି (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        ଫେରନ୍ତୁ;

    PlatformRunningOn |= PF_QEMU;
    ବାଛ (d) {
    ସ୍ଥିତି 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        ଭାଙ୍ଗ;
    }
    physbits(1);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା qemu_early_e820(ଶୂନ୍ୟ);

ଶୂନ୍ୟ
qemu_preinit(ଶୂନ୍ୟ)
{
    qemu_detect();
    kvm_detect();

    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    ଯଦି (runningOnXen()) {
        xen_ramsize_preinit();
        ଫେରନ୍ତୁ;
    }

    // try read e820 table first
    ଯଦି (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_ତଳ) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_ଉପର) << 24));
        ଯଦି (rs)
            rs += 16 * 1024 * 1024;
        ଅନ୍ୟଥା
            rs = (((rtc_read(CMOS_MEM_EXTMEM_ତଳ) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_ଉପର) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

ସ୍ଥିର ଶୂନ୍ୟ msr_feature_control_setup(ଶୂନ୍ୟ)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    ଯଦି (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

ଶୂନ୍ୟ
qemu_platform_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    ଯଦି (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        ଫେରନ୍ତୁ;
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
    ଯଦି (Max_ଗଣନା_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    ଯଦି (CONFIG_FW_ROMFILE_LOAD) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        ଯଦି (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            ଫେରନ୍ତୁ;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        ଯଦି (!loader_err)
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

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_dma_transfer(ଶୂନ୍ୟ *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_ତଳ);

    ଯେତେବେଳେ(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_read(ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    ଯଦି (len == 0) {
        ଫେରନ୍ତୁ;
    }

    ଯଦି (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } ଅନ୍ୟଥା {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_write(ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    ଯଦି (len == 0) {
        ଫେରନ୍ତୁ;
    }

    ଯଦି (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } ଅନ୍ୟଥା {
        warn_internalerror();
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_skip(ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    ଯଦି (len == 0) {
        ଫେରନ୍ତୁ;
    }

    ଯଦି (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } ଅନ୍ୟଥା {
        ଯେତେବେଳେ (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_read_entry(ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା e, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    ଯଦି (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } ଅନ୍ୟଥା {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_write_entry(ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା e, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    ଯଦି (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } ଅନ୍ୟଥା {
        warn_internalerror();
    }
}

ସଂରଚନା qemu_romfile_s {
    ସଂରଚନା romfile_s file;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା select, skip;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
qemu_cfg_read_file(ସଂରଚନା romfile_s *file, ଶୂନ୍ୟ *dst, u32 maxlen)
{
    ଯଦି (file->size > maxlen)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା qemu_romfile_s *qfile;
    qfile = container_of(file, ସଂରଚନା qemu_romfile_s, file);
    ଯଦି (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } ଅନ୍ୟଥା {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    ଫେରନ୍ତୁ file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
qemu_cfg_write_file_simple(ଶୂନ୍ୟ *src, u16 ଚାବି, u32 offset, u32 len)
{
    ଯଦି (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, ଚାବି, len);
    } ଅନ୍ୟଥା {
        qemu_cfg_select(ଚାବି);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    ଫେରନ୍ତୁ len;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
qemu_cfg_write_file(ଶୂନ୍ୟ *src, ସଂରଚନା romfile_s *file, u32 offset, u32 len)
{
    ଯଦି ((offset + len) > file->size)
        ଫେରନ୍ତୁ -1;

    ଯଦି (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ qemu_cfg_write_file_simple(src, qemu_get_romfile_ଚାବି(file),
                                      offset, len);
}

ସ୍ଥିର ଶୂନ୍ୟ
qemu_romfile_add(ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା select, ପୂର୍ଣ୍ଣସଂଖ୍ୟା skip, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
    ସଂରଚନା qemu_romfile_s *qfile = malloc_tmp(ଆକାର(*qfile));
    ଯଦି (!qfile) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(qfile, 0, ଆକାର(*qfile));
    strtcpy(qfile->file.name, name, ଆକାର(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_ଚାବି(ସଂରଚନା romfile_s *file)
{
    ସଂରଚନା qemu_romfile_s *qfile;
    ଯଦି (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        ଫେରନ୍ତୁ 0;
    }
    qfile = container_of(file, ସଂରଚନା qemu_romfile_s, file);
    ଫେରନ୍ତୁ qfile->select;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rtc_present(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_ଗଣନା(ଶୂନ୍ୟ)
{
    u16 smp_ଗଣନା = 0;
    ଯଦି (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_ଗଣନା, QEMU_CFG_NB_CPUS, ଆକାର(smp_ଗଣନା));
    }
    ଯଦି (rtc_present()) {
        u16 cmos_cpu_ଗଣନା = rtc_read(CMOS_BIOS_SMP_ଗଣନା) + 1;
        ଯଦି (smp_ଗଣନା < cmos_cpu_ଗଣନା) {
            smp_ଗଣନା = cmos_cpu_ଗଣନା;
        }
    }
    ଫେରନ୍ତୁ smp_ଗଣନା;
}

ସଂରଚନା e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

ସଂରଚନା qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_e820(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    ଯଦି (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        ଫେରନ୍ତୁ;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, ଆକାର(count32));
    ଯଦି (count32) {
        ସଂରଚନା e820_reservation entry;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, ଆକାର(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } ଅନ୍ୟଥା ଯଦି (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 ଉପର = ((rtc_read(CMOS_MEM_HIGHMEM_ତଳ) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_ଉପର) << 32));
    RamSizeOver4G = ଉପର;
    e820_add(0x100000000ull, ଉପର, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ସ୍ଥିର ଶୂନ୍ୟ
qemu_cfg_legacy(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, ଆକାର(numacount));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, ଆକାର(numacount)
                     , max_cpu*ଆକାର(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , ଆକାର(numacount) + max_cpu*ଆକାର(u64)
                     , numacount*ଆକାର(u64));

    // ACPI tables
    ଅକ୍ଷର name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, ଆକାର(cnt));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, offset = ଆକାର(cnt);
    ପାଇଁ (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, ଆକାର(len));
        offset += ଆକାର(len);
        snprintf(name, ଆକାର(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, ଆକାର(cnt));
    offset = ଆକାର(cnt);
    ପାଇଁ (i = 0; i < cnt; i++) {
        ସଂରଚନା qemu_smbios_header header;
        qemu_cfg_read(&header, ଆକାର(header));
        ଯଦି (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, ଆକାର(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + ଆକାର(header)
                             , header.length - ଆକାର(header));
        } ଅନ୍ୟଥା {
            snprintf(name, ଆକାର(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - ଆକାର(header));
        offset += header.length;
    }
}

ସଂରଚନା QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    ଅକ୍ଷର name[56];
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା qemu_cfg_detect(ଶୂନ୍ୟ)
{
    ଯଦି (cfg_enabled)
        ଫେରନ୍ତୁ 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    ଅକ୍ଷର *sig = "QEMU";
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < 4; i++)
        ଯଦି (inb(PORT_QEMU_CFG_DATA) != sig[i])
            ଫେରନ୍ତୁ 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, ଆକାର(id));

    ଯଦି (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    ଫେରନ୍ତୁ 1;
}

ଶୂନ୍ୟ qemu_cfg_init(ଶୂନ୍ୟ)
{
    ଯଦି (!runningOnQEMU())
        ଫେରନ୍ତୁ;

    ଯଦି (!qemu_cfg_detect())
        ଫେରନ୍ତୁ;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 ଗଣନା_2;
    qemu_cfg_read_entry(&ଗଣନା_2, QEMU_CFG_FILE_DIR, ଆକାର(ଗଣନା_2));
    ଗଣନା_2 = be32_to_cpu(ଗଣନା_2);
    u32 e;
    ପାଇଁ (e = 0; e < ଗଣନା_2; e++) {
        ସଂରଚନା QemuCfgFile qfile;
        qemu_cfg_read(&qfile, ଆକାର(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    ଯଦି (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, ଆକାର(nogfx));
    ଯଦି (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା qemu_early_e820(ଶୂନ୍ୟ)
{
    ସଂରଚନା e820_reservation table;
    ସଂରଚନା QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 ଗଣନା_2, i;

    ଯଦି (!qemu_cfg_detect())
        ଫେରନ୍ତୁ 0;

    // find e820 table
    qemu_cfg_read_entry(&ଗଣନା_2, QEMU_CFG_FILE_DIR, ଆକାର(ଗଣନା_2));
    ଗଣନା_2 = be32_to_cpu(ଗଣନା_2);
    ପାଇଁ (i = 0; i < ଗଣନା_2; i++) {
        qemu_cfg_read(&qfile, ଆକାର(qfile));
        ଯଦି (memcmp(qfile.name, "etc/e820", 9) != 0)
            ଜାରି;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        ଭାଙ୍ଗ;
    }
    ଯଦି (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        ଫେରନ୍ତୁ 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    ଗଣନା_2 = size/ଆକାର(table);
    ପାଇଁ (i = 0, select = 0; i < ଗଣନା_2; i++) {
        qemu_cfg_read(&table, ଆକାର(table));
        ବାଛ (table.type) {
        ସ୍ଥିତି E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            ଯଦି (table.address < 0x100000000LL) {
                // below 4g
                ଯଦି (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } ଅନ୍ୟଥା {
                // above 4g
                ଯଦି (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    ଫେରନ୍ତୁ 1;
}
