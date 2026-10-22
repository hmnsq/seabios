// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "byteorder.h" // be32_to_cpu
#অন্তর্ভাব "config.h" // CONFIG_QEMU
#অন্তর্ভাব "e820map.h" // e820_add
#অন্তর্ভাব "hw/pci.h" // pci_config_readw
#অন্তর্ভাব "hw/pcidevice.h" // pci_probe_devices
#অন্তর্ভাব "hw/pci_regs.h" // PCI_DEVICE_ID
#অন্তর্ভাব "hw/serialio.h" // PORT_SERIAL1
#অন্তর্ভাব "hw/rtc.h" // CMOS_*
#অন্তর্ভাব "hw/virtio-mmio.h" // virtio_mmio_acpi
#অন্তর্ভাব "malloc.h" // malloc_tmp
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "paravirt.h" // qemu_cfg_preinit
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "romfile_loader.h" // romfile_loader_execute
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // pci_setup
#অন্তর্ভাব "x86.h" // cpuid
#অন্তর্ভাব "xen.h" // xen_biostable_setup
#অন্তর্ভাব "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 Ram_আকার;
// Amount of continuous ram >4Gig
u64 Ram_আকার_Over4_G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
পূর্ণসংখ্যা PlatformRunningOn VARFSEG;
// cfg enabled
পূর্ণসংখ্যা cfg_enabled = 0;
// cfg_dma enabled
পূর্ণসংখ্যা cfg_dma_enabled = 0;

অন্তঃরেখ পূর্ণসংখ্যা qemu_cfg_enabled(শূন্য)
{
    ফেরত cfg_enabled;
}

অন্তঃরেখ পূর্ণসংখ্যা qemu_cfg_dma_enabled(শূন্য)
{
    ফেরত cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
স্থির unsigned পূর্ণসংখ্যা kvm_cpuid_base = 0;

স্থির শূন্য kvm_detect(শূন্য)
{
    unsigned পূর্ণসংখ্যা i, max = 0;
    unsigned পূর্ণসংখ্যা eax, ebx, ecx, edx;
    অক্ষর signature[13];

    জন্য (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        যদি (eax < i)
            বিরতি;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        যদি (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    যদি (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        যদি (max >= kvm_cpuid_base + 0x10) {
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

কাঠামো pvclock_vcpu_time_info *kvmclock;

স্থির শূন্য kvmclock_init(শূন্য)
{
    unsigned পূর্ণসংখ্যা eax, ebx, ecx, edx, msr;

    যদি (!runningOnKVM())
        ফেরত;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    যদি (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    অন্যথা যদি (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    অন্যথা
        ফেরত;

    kvmclock = memalign_নিচের(আকার(*kvmclock), 32);
    memset(kvmclock, 0, আকার(*kvmclock));
    u32 মান = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", মান, msr);
    wrmsr(msr, মান | 0x01);

    যদি (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        ফেরত;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    যদি (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    অন্যথা
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

স্থির শূন্য physbits(পূর্ণসংখ্যা qemu_quirk)
{
    unsigned পূর্ণসংখ্যা max, eax, ebx, ecx, edx;
    unsigned পূর্ণসংখ্যা physbits;
    অক্ষর signature[13];
    পূর্ণসংখ্যা pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    যদি (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    যদি (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    যদি (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        যদি (!qemu_quirk) {
            valid = 1;
        } অন্যথা যদি (physbits >= 41) {
            valid = 1;
        } অন্যথা যদি (strcmp(signature, "GenuineIntel") == 0) {
            যদি ((physbits == 36) || (physbits == 39))
                valid = 1;
        } অন্যথা যদি (strcmp(signature, "AuthenticAMD") == 0) {
            যদি (physbits == 40)
                valid = 1;
        }
    } অন্যথা {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    যদি (valid)
        CPUPhysBits = physbits;
}

স্থির শূন্য qemu_detect(শূন্য)
{
    যদি (!CONFIG_QEMU_HARDWARE)
        ফেরত;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    যদি (v == 0x0000 || v == 0xffff)
        ফেরত;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    যদি (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        ফেরত;

    PlatformRunningOn |= PF_QEMU;
    নির্বাচন (d) {
    ক্ষেত্র 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        বিরতি;
    ক্ষেত্র 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        বিরতি;
    পূর্বনির্ধারিত:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        বিরতি;
    }
    physbits(1);
}

স্থির পূর্ণসংখ্যা qemu_early_e820(শূন্য);

শূন্য
qemu_preinit(শূন্য)
{
    qemu_detect();
    kvm_detect();

    যদি (!CONFIG_QEMU)
        ফেরত;

    যদি (runningOnXen()) {
        xen_ramsize_preinit();
        ফেরত;
    }

    // try read e820 table first
    যদি (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_নিচের) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_উপরের) << 24));
        যদি (rs)
            rs += 16 * 1024 * 1024;
        অন্যথা
            rs = (((rtc_read(CMOS_MEM_EXTMEM_নিচের) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_উপরের) << 18))
                  + 1 * 1024 * 1024);
        Ram_আকার = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_আকার);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

স্থির শূন্য msr_feature_control_setup(শূন্য)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    যদি (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

শূন্য
qemu_platform_setup(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    যদি (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        ফেরত;
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
    যদি (Max_গুনতি_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    যদি (CONFIG_FW_ROMFILE_LOAD) {
        পূর্ণসংখ্যা loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        যদি (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            ফেরত;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        যদি (!loader_err)
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

স্থির শূন্য
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

স্থির শূন্য
qemu_cfg_dma_transfer(শূন্য *address, u32 দৈর্ঘ্য, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.দৈর্ঘ্য = cpu_to_be32(দৈর্ঘ্য);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_নিচের);

    যখন(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

স্থির শূন্য
qemu_cfg_read(শূন্য *buf, পূর্ণসংখ্যা len)
{
    যদি (len == 0) {
        ফেরত;
    }

    যদি (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } অন্যথা {
        insb(PORT_QEMU_CFG_তথ্য, buf, len);
    }
}

স্থির শূন্য
qemu_cfg_write(শূন্য *buf, পূর্ণসংখ্যা len)
{
    যদি (len == 0) {
        ফেরত;
    }

    যদি (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } অন্যথা {
        warn_internalerror();
    }
}

স্থির শূন্য
qemu_cfg_skip(পূর্ণসংখ্যা len)
{
    যদি (len == 0) {
        ফেরত;
    }

    যদি (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } অন্যথা {
        যখন (len--)
            inb(PORT_QEMU_CFG_তথ্য);
    }
}

স্থির শূন্য
qemu_cfg_read_entry(শূন্য *buf, পূর্ণসংখ্যা e, পূর্ণসংখ্যা len)
{
    যদি (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } অন্যথা {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

স্থির শূন্য
qemu_cfg_write_entry(শূন্য *buf, পূর্ণসংখ্যা e, পূর্ণসংখ্যা len)
{
    যদি (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } অন্যথা {
        warn_internalerror();
    }
}

কাঠামো qemu_romfile_s {
    কাঠামো romfile_s file;
    পূর্ণসংখ্যা select, skip;
};

স্থির পূর্ণসংখ্যা
qemu_cfg_read_file(কাঠামো romfile_s *file, শূন্য *dst, u32 maxlen)
{
    যদি (file->আকার_2 > maxlen)
        ফেরত -1;
    কাঠামো qemu_romfile_s *qfile;
    qfile = container_of(file, কাঠামো qemu_romfile_s, file);
    যদি (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->আকার_2);
    } অন্যথা {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->আকার_2);
    }
    ফেরত file->আকার_2;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
পূর্ণসংখ্যা
qemu_cfg_write_file_simple(শূন্য *src, u16 চাবি, u32 offset, u32 len)
{
    যদি (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, চাবি, len);
    } অন্যথা {
        qemu_cfg_select(চাবি);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    ফেরত len;
}

পূর্ণসংখ্যা
qemu_cfg_write_file(শূন্য *src, কাঠামো romfile_s *file, u32 offset, u32 len)
{
    যদি ((offset + len) > file->আকার_2)
        ফেরত -1;

    যদি (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        ফেরত -1;
    }
    ফেরত qemu_cfg_write_file_simple(src, qemu_get_romfile_চাবি(file),
                                      offset, len);
}

স্থির শূন্য
qemu_romfile_add(অক্ষর *name, পূর্ণসংখ্যা select, পূর্ণসংখ্যা skip, পূর্ণসংখ্যা আকার_2)
{
    কাঠামো qemu_romfile_s *qfile = malloc_tmp(আকার(*qfile));
    যদি (!qfile) {
        warn_noalloc();
        ফেরত;
    }
    memset(qfile, 0, আকার(*qfile));
    strtcpy(qfile->file.name, name, আকার(qfile->file.name));
    qfile->file.আকার_2 = আকার_2;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_চাবি(কাঠামো romfile_s *file)
{
    কাঠামো qemu_romfile_s *qfile;
    যদি (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        ফেরত 0;
    }
    qfile = container_of(file, কাঠামো qemu_romfile_s, file);
    ফেরত qfile->select;
}

স্থির পূর্ণসংখ্যা rtc_present(শূন্য)
{
    ফেরত rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_গুনতি(শূন্য)
{
    u16 smp_গুনতি = 0;
    যদি (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_গুনতি, QEMU_CFG_NB_CPUS, আকার(smp_গুনতি));
    }
    যদি (rtc_present()) {
        u16 cmos_cpu_গুনতি = rtc_read(CMOS_BIOS_SMP_গুনতি) + 1;
        যদি (smp_গুনতি < cmos_cpu_গুনতি) {
            smp_গুনতি = cmos_cpu_গুনতি;
        }
    }
    ফেরত smp_গুনতি;
}

কাঠামো e820_reservation {
    u64 address;
    u64 দৈর্ঘ্য;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

কাঠামো qemu_smbios_header {
    u16 দৈর্ঘ্য;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

স্থির শূন্য
qemu_cfg_e820(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    যদি (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        ফেরত;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, আকার(count32));
    যদি (count32) {
        কাঠামো e820_reservation entry;
        পূর্ণসংখ্যা i;
        জন্য (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, আকার(entry));
            e820_add(entry.address, entry.দৈর্ঘ্য, entry.type);
        }
    } অন্যথা যদি (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 উপরের = ((rtc_read(CMOS_MEM_HIGHMEM_নিচের) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_উপরের) << 32));
    Ram_আকার_Over4_G = উপরের;
    e820_add(0x100000000ull, উপরের, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_আকার_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
স্থির শূন্য
qemu_cfg_legacy(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, আকার(numacount));
    পূর্ণসংখ্যা max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, আকার(numacount)
                     , max_cpu*আকার(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , আকার(numacount) + max_cpu*আকার(u64)
                     , numacount*আকার(u64));

    // ACPI tables
    অক্ষর name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, আকার(cnt));
    পূর্ণসংখ্যা i, offset = আকার(cnt);
    জন্য (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, আকার(len));
        offset += আকার(len);
        snprintf(name, আকার(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, আকার(cnt));
    offset = আকার(cnt);
    জন্য (i = 0; i < cnt; i++) {
        কাঠামো qemu_smbios_header header;
        qemu_cfg_read(&header, আকার(header));
        যদি (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, আকার(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + আকার(header)
                             , header.দৈর্ঘ্য - আকার(header));
        } অন্যথা {
            snprintf(name, আকার(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.দৈর্ঘ্য - 3);
        }
        qemu_cfg_skip(header.দৈর্ঘ্য - আকার(header));
        offset += header.দৈর্ঘ্য;
    }
}

কাঠামো QemuCfgFile {
    u32  আকার_2;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    অক্ষর name[56];
};

স্থির পূর্ণসংখ্যা qemu_cfg_detect(শূন্য)
{
    যদি (cfg_enabled)
        ফেরত 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    অক্ষর *sig = "QEMU";
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < 4; i++)
        যদি (inb(PORT_QEMU_CFG_তথ্য) != sig[i])
            ফেরত 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, আকার(id));

    যদি (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    ফেরত 1;
}

শূন্য qemu_cfg_init(শূন্য)
{
    যদি (!runningOnQEMU())
        ফেরত;

    যদি (!qemu_cfg_detect())
        ফেরত;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 গুনতি;
    qemu_cfg_read_entry(&গুনতি, QEMU_CFG_FILE_DIR, আকার(গুনতি));
    গুনতি = be32_to_cpu(গুনতি);
    u32 e;
    জন্য (e = 0; e < গুনতি; e++) {
        কাঠামো QemuCfgFile qfile;
        qemu_cfg_read(&qfile, আকার(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.আকার_2));
    }

    qemu_cfg_e820();

    যদি (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, আকার(nogfx));
    যদি (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
স্থির পূর্ণসংখ্যা qemu_early_e820(শূন্য)
{
    কাঠামো e820_reservation table;
    কাঠামো QemuCfgFile qfile;
    u32 select = 0, আকার_2 = 0;
    u32 গুনতি, i;

    যদি (!qemu_cfg_detect())
        ফেরত 0;

    // find e820 table
    qemu_cfg_read_entry(&গুনতি, QEMU_CFG_FILE_DIR, আকার(গুনতি));
    গুনতি = be32_to_cpu(গুনতি);
    জন্য (i = 0; i < গুনতি; i++) {
        qemu_cfg_read(&qfile, আকার(qfile));
        যদি (memcmp(qfile.name, "etc/e820", 9) != 0)
            চালু;
        select = be16_to_cpu(qfile.select);
        আকার_2 = be32_to_cpu(qfile.আকার_2);
        বিরতি;
    }
    যদি (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        ফেরত 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    গুনতি = আকার_2/আকার(table);
    জন্য (i = 0, select = 0; i < গুনতি; i++) {
        qemu_cfg_read(&table, আকার(table));
        নির্বাচন (table.type) {
        ক্ষেত্র E820_RESERVED:
            e820_add(table.address, table.দৈর্ঘ্য, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.দৈর্ঘ্য);
            বিরতি;
        ক্ষেত্র E820_SOFT_RESERVED:
            e820_add(table.address, table.দৈর্ঘ্য, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.দৈর্ঘ্য);
            বিরতি;
        ক্ষেত্র E820_RAM:
            e820_add(table.address, table.দৈর্ঘ্য, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.দৈর্ঘ্য);
            যদি (table.address < 0x100000000LL) {
                // below 4g
                যদি (Ram_আকার < table.address + table.দৈর্ঘ্য)
                    Ram_আকার = table.address + table.দৈর্ঘ্য;
            } অন্যথা {
                // above 4g
                যদি (Ram_আকার_Over4_G < table.address + table.দৈর্ঘ্য - 0x100000000LL)
                    Ram_আকার_Over4_G = table.address + table.দৈর্ঘ্য - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_আকার);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_আকার_Over4_G);
    ফেরত 1;
}
