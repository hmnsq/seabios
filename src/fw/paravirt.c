// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "byteorder.h" // be32_to_cpu
#अन्तर्भाव "config.h" // CONFIG_QEMU
#अन्तर्भाव "e820map.h" // e820_add
#अन्तर्भाव "hw/pci.h" // pci_config_readw
#अन्तर्भाव "hw/pcidevice.h" // pci_probe_devices
#अन्तर्भाव "hw/pci_regs.h" // PCI_DEVICE_ID
#अन्तर्भाव "hw/serialio.h" // PORT_SERIAL1
#अन्तर्भाव "hw/rtc.h" // CMOS_*
#अन्तर्भाव "hw/virtio-mmio.h" // virtio_mmio_acpi
#अन्तर्भाव "malloc.h" // malloc_tmp
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // qemu_cfg_preinit
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "romfile_loader.h" // romfile_loader_execute
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // pci_setup
#अन्तर्भाव "x86.h" // cpuid
#अन्तर्भाव "xen.h" // xen_biostable_setup
#अन्तर्भाव "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
पूर्णाङ्क PlatformRunningOn VARFSEG;
// cfg enabled
पूर्णाङ्क cfg_enabled = 0;
// cfg_dma enabled
पूर्णाङ्क cfg_dma_enabled = 0;

अन्तःस्थ पूर्णाङ्क qemu_cfg_enabled(शून्य)
{
    पटिनिवत्त cfg_enabled;
}

अन्तःस्थ पूर्णाङ्क qemu_cfg_dma_enabled(शून्य)
{
    पटिनिवत्त cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
स्थिर unsigned पूर्णाङ्क kvm_cpuid_base = 0;

स्थिर शून्य kvm_detect(शून्य)
{
    unsigned पूर्णाङ्क i, max = 0;
    unsigned पूर्णाङ्क eax, ebx, ecx, edx;
    वर्ण signature[13];

    अत्थाय (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        सचे (eax < i)
            भङ्ग;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        सचे (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    सचे (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        सचे (max >= kvm_cpuid_base + 0x10) {
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

संरचना pvclock_vcpu_time_info *kvmclock;

स्थिर शून्य kvmclock_init(शून्य)
{
    unsigned पूर्णाङ्क eax, ebx, ecx, edx, msr;

    सचे (!runningOnKVM())
        पटिनिवत्त;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    सचे (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    अञ्ञथा सचे (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    अञ्ञथा
        पटिनिवत्त;

    kvmclock = memalign_नीचः(परिमाण(*kvmclock), 32);
    memset(kvmclock, 0, परिमाण(*kvmclock));
    u32 मानम् = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", मानम्, msr);
    wrmsr(msr, मानम् | 0x01);

    सचे (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        पटिनिवत्त;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    सचे (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    अञ्ञथा
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

स्थिर शून्य physbits(पूर्णाङ्क qemu_quirk)
{
    unsigned पूर्णाङ्क max, eax, ebx, ecx, edx;
    unsigned पूर्णाङ्क physbits;
    वर्ण signature[13];
    पूर्णाङ्क pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    सचे (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    सचे (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    सचे (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        सचे (!qemu_quirk) {
            valid = 1;
        } अञ्ञथा सचे (physbits >= 41) {
            valid = 1;
        } अञ्ञथा सचे (strcmp(signature, "GenuineIntel") == 0) {
            सचे ((physbits == 36) || (physbits == 39))
                valid = 1;
        } अञ्ञथा सचे (strcmp(signature, "AuthenticAMD") == 0) {
            सचे (physbits == 40)
                valid = 1;
        }
    } अञ्ञथा {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    सचे (valid)
        CPUPhysBits = physbits;
}

स्थिर शून्य qemu_detect(शून्य)
{
    सचे (!CONFIG_QEMU_HARDWARE)
        पटिनिवत्त;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    सचे (v == 0x0000 || v == 0xffff)
        पटिनिवत्त;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    सचे (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        पटिनिवत्त;

    PlatformRunningOn |= PF_QEMU;
    विकल्प (d) {
    पक्ष 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        भङ्ग;
    पक्ष 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        भङ्ग;
    स्वभाव:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        भङ्ग;
    }
    physbits(1);
}

स्थिर पूर्णाङ्क qemu_early_e820(शून्य);

शून्य
qemu_preinit(शून्य)
{
    qemu_detect();
    kvm_detect();

    सचे (!CONFIG_QEMU)
        पटिनिवत्त;

    सचे (runningOnXen()) {
        xen_ramsize_preinit();
        पटिनिवत्त;
    }

    // try read e820 table first
    सचे (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_नीचः) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_उच्चः) << 24));
        सचे (rs)
            rs += 16 * 1024 * 1024;
        अञ्ञथा
            rs = (((rtc_read(CMOS_MEM_EXTMEM_नीचः) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_उच्चः) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

स्थिर शून्य msr_feature_control_setup(शून्य)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    सचे (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

शून्य
qemu_platform_setup(शून्य)
{
    सचे (!CONFIG_QEMU)
        पटिनिवत्त;

    सचे (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        पटिनिवत्त;
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
    सचे (Max_गणनम्_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    सचे (CONFIG_FW_ROMFILE_LOAD) {
        पूर्णाङ्क loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        सचे (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            पटिनिवत्त;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        सचे (!loader_err)
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

स्थिर शून्य
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

स्थिर शून्य
qemu_cfg_dma_transfer(शून्य *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_नीचः);

    याव(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

स्थिर शून्य
qemu_cfg_read(शून्य *buf, पूर्णाङ्क len)
{
    सचे (len == 0) {
        पटिनिवत्त;
    }

    सचे (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } अञ्ञथा {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

स्थिर शून्य
qemu_cfg_write(शून्य *buf, पूर्णाङ्क len)
{
    सचे (len == 0) {
        पटिनिवत्त;
    }

    सचे (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } अञ्ञथा {
        warn_internalerror();
    }
}

स्थिर शून्य
qemu_cfg_skip(पूर्णाङ्क len)
{
    सचे (len == 0) {
        पटिनिवत्त;
    }

    सचे (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } अञ्ञथा {
        याव (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

स्थिर शून्य
qemu_cfg_read_entry(शून्य *buf, पूर्णाङ्क e, पूर्णाङ्क len)
{
    सचे (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } अञ्ञथा {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

स्थिर शून्य
qemu_cfg_write_entry(शून्य *buf, पूर्णाङ्क e, पूर्णाङ्क len)
{
    सचे (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } अञ्ञथा {
        warn_internalerror();
    }
}

संरचना qemu_romfile_s {
    संरचना romfile_s file;
    पूर्णाङ्क select, skip;
};

स्थिर पूर्णाङ्क
qemu_cfg_read_file(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    सचे (file->size > maxlen)
        पटिनिवत्त -1;
    संरचना qemu_romfile_s *qfile;
    qfile = container_of(file, संरचना qemu_romfile_s, file);
    सचे (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } अञ्ञथा {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    पटिनिवत्त file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
पूर्णाङ्क
qemu_cfg_write_file_simple(शून्य *src, u16 कुञ्चिका, u32 offset, u32 len)
{
    सचे (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, कुञ्चिका, len);
    } अञ्ञथा {
        qemu_cfg_select(कुञ्चिका);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    पटिनिवत्त len;
}

पूर्णाङ्क
qemu_cfg_write_file(शून्य *src, संरचना romfile_s *file, u32 offset, u32 len)
{
    सचे ((offset + len) > file->size)
        पटिनिवत्त -1;

    सचे (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        पटिनिवत्त -1;
    }
    पटिनिवत्त qemu_cfg_write_file_simple(src, qemu_get_romfile_कुञ्चिका(file),
                                      offset, len);
}

स्थिर शून्य
qemu_romfile_add(वर्ण *name, पूर्णाङ्क select, पूर्णाङ्क skip, पूर्णाङ्क size)
{
    संरचना qemu_romfile_s *qfile = malloc_tmp(परिमाण(*qfile));
    सचे (!qfile) {
        warn_noalloc();
        पटिनिवत्त;
    }
    memset(qfile, 0, परिमाण(*qfile));
    strtcpy(qfile->file.name, name, परिमाण(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_कुञ्चिका(संरचना romfile_s *file)
{
    संरचना qemu_romfile_s *qfile;
    सचे (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        पटिनिवत्त 0;
    }
    qfile = container_of(file, संरचना qemu_romfile_s, file);
    पटिनिवत्त qfile->select;
}

स्थिर पूर्णाङ्क rtc_present(शून्य)
{
    पटिनिवत्त rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_गणनम्(शून्य)
{
    u16 smp_गणनम् = 0;
    सचे (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_गणनम्, QEMU_CFG_NB_CPUS, परिमाण(smp_गणनम्));
    }
    सचे (rtc_present()) {
        u16 cmos_cpu_गणनम् = rtc_read(CMOS_BIOS_SMP_गणनम्) + 1;
        सचे (smp_गणनम् < cmos_cpu_गणनम्) {
            smp_गणनम् = cmos_cpu_गणनम्;
        }
    }
    पटिनिवत्त smp_गणनम्;
}

संरचना e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

संरचना qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

स्थिर शून्य
qemu_cfg_e820(शून्य)
{
    सचे (!CONFIG_QEMU)
        पटिनिवत्त;

    सचे (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        पटिनिवत्त;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, परिमाण(count32));
    सचे (count32) {
        संरचना e820_reservation entry;
        पूर्णाङ्क i;
        अत्थाय (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, परिमाण(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } अञ्ञथा सचे (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 उच्चः = ((rtc_read(CMOS_MEM_HIGHMEM_नीचः) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_उच्चः) << 32));
    RamSizeOver4G = उच्चः;
    e820_add(0x100000000ull, उच्चः, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
स्थिर शून्य
qemu_cfg_legacy(शून्य)
{
    सचे (!CONFIG_QEMU)
        पटिनिवत्त;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, परिमाण(numacount));
    पूर्णाङ्क max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, परिमाण(numacount)
                     , max_cpu*परिमाण(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , परिमाण(numacount) + max_cpu*परिमाण(u64)
                     , numacount*परिमाण(u64));

    // ACPI tables
    वर्ण name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, परिमाण(cnt));
    पूर्णाङ्क i, offset = परिमाण(cnt);
    अत्थाय (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, परिमाण(len));
        offset += परिमाण(len);
        snprintf(name, परिमाण(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, परिमाण(cnt));
    offset = परिमाण(cnt);
    अत्थाय (i = 0; i < cnt; i++) {
        संरचना qemu_smbios_header header;
        qemu_cfg_read(&header, परिमाण(header));
        सचे (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, परिमाण(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + परिमाण(header)
                             , header.length - परिमाण(header));
        } अञ्ञथा {
            snprintf(name, परिमाण(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - परिमाण(header));
        offset += header.length;
    }
}

संरचना QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    वर्ण name[56];
};

स्थिर पूर्णाङ्क qemu_cfg_detect(शून्य)
{
    सचे (cfg_enabled)
        पटिनिवत्त 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    वर्ण *sig = "QEMU";
    पूर्णाङ्क i;
    अत्थाय (i = 0; i < 4; i++)
        सचे (inb(PORT_QEMU_CFG_DATA) != sig[i])
            पटिनिवत्त 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, परिमाण(id));

    सचे (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    पटिनिवत्त 1;
}

शून्य qemu_cfg_init(शून्य)
{
    सचे (!runningOnQEMU())
        पटिनिवत्त;

    सचे (!qemu_cfg_detect())
        पटिनिवत्त;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 गणनम्;
    qemu_cfg_read_entry(&गणनम्, QEMU_CFG_FILE_DIR, परिमाण(गणनम्));
    गणनम् = be32_to_cpu(गणनम्);
    u32 e;
    अत्थाय (e = 0; e < गणनम्; e++) {
        संरचना QemuCfgFile qfile;
        qemu_cfg_read(&qfile, परिमाण(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    सचे (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, परिमाण(nogfx));
    सचे (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
स्थिर पूर्णाङ्क qemu_early_e820(शून्य)
{
    संरचना e820_reservation table;
    संरचना QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 गणनम्, i;

    सचे (!qemu_cfg_detect())
        पटिनिवत्त 0;

    // find e820 table
    qemu_cfg_read_entry(&गणनम्, QEMU_CFG_FILE_DIR, परिमाण(गणनम्));
    गणनम् = be32_to_cpu(गणनम्);
    अत्थाय (i = 0; i < गणनम्; i++) {
        qemu_cfg_read(&qfile, परिमाण(qfile));
        सचे (memcmp(qfile.name, "etc/e820", 9) != 0)
            अनुवर्त;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        भङ्ग;
    }
    सचे (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        पटिनिवत्त 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    गणनम् = size/परिमाण(table);
    अत्थाय (i = 0, select = 0; i < गणनम्; i++) {
        qemu_cfg_read(&table, परिमाण(table));
        विकल्प (table.type) {
        पक्ष E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            भङ्ग;
        पक्ष E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            भङ्ग;
        पक्ष E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            सचे (table.address < 0x100000000LL) {
                // below 4g
                सचे (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } अञ्ञथा {
                // above 4g
                सचे (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    पटिनिवत्त 1;
}
