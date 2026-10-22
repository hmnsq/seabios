// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "byteorder.h" // be32_to_cpu
#ضمن "config.h" // CONFIG_QEMU
#ضمن "e820map.h" // e820_add
#ضمن "hw/pci.h" // pci_config_readw
#ضمن "hw/pcidevice.h" // pci_probe_devices
#ضمن "hw/pci_regs.h" // PCI_DEVICE_ID
#ضمن "hw/serialio.h" // PORT_SERIAL1
#ضمن "hw/rtc.h" // CMOS_*
#ضمن "hw/virtio-mmio.h" // virtio_mmio_acpi
#ضمن "malloc.h" // malloc_tmp
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // qemu_cfg_preinit
#ضمن "romfile.h" // romfile_loadint
#ضمن "romfile_loader.h" // romfile_loader_execute
#ضمن "string.h" // memset
#ضمن "util.h" // pci_setup
#ضمن "x86.h" // cpuid
#ضمن "xen.h" // xen_biostable_setup
#ضمن "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
صحيح PlatformRunningOn VARFSEG;
// cfg enabled
صحيح cfg_enabled = 0;
// cfg_dma enabled
صحيح cfg_dma_enabled = 0;

مضمن صحيح qemu_cfg_enabled(فراغ)
{
    أرجع cfg_enabled;
}

مضمن صحيح qemu_cfg_dma_enabled(فراغ)
{
    أرجع cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
ساكن unsigned صحيح kvm_cpuid_base = 0;

ساكن فراغ kvm_detect(فراغ)
{
    unsigned صحيح i, max = 0;
    unsigned صحيح eax, ebx, ecx, edx;
    حرف signature[13];

    لكل (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        إذا (eax < i)
            اكسر;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        إذا (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    إذا (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        إذا (max >= kvm_cpuid_base + 0x10) {
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

بنية pvclock_vcpu_time_info *kvmclock;

ساكن فراغ kvmclock_init(فراغ)
{
    unsigned صحيح eax, ebx, ecx, edx, msr;

    إذا (!runningOnKVM())
        أرجع;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    إذا (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    وإلا إذا (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    وإلا
        أرجع;

    kvmclock = memalign_نزم(حجم(*kvmclock), 32);
    memset(kvmclock, 0, حجم(*kvmclock));
    u32 بەها = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", بەها, msr);
    wrmsr(msr, بەها | 0x01);

    إذا (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        أرجع;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    إذا (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    وإلا
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ساكن فراغ physbits(صحيح qemu_quirk)
{
    unsigned صحيح max, eax, ebx, ecx, edx;
    unsigned صحيح physbits;
    حرف signature[13];
    صحيح pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    إذا (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    إذا (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    إذا (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        إذا (!qemu_quirk) {
            valid = 1;
        } وإلا إذا (physbits >= 41) {
            valid = 1;
        } وإلا إذا (strcmp(signature, "GenuineIntel") == 0) {
            إذا ((physbits == 36) || (physbits == 39))
                valid = 1;
        } وإلا إذا (strcmp(signature, "AuthenticAMD") == 0) {
            إذا (physbits == 40)
                valid = 1;
        }
    } وإلا {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    إذا (valid)
        CPUPhysBits = physbits;
}

ساكن فراغ qemu_detect(فراغ)
{
    إذا (!CONFIG_QEMU_HARDWARE)
        أرجع;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    إذا (v == 0x0000 || v == 0xffff)
        أرجع;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    إذا (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        أرجع;

    PlatformRunningOn |= PF_QEMU;
    بدل (d) {
    حالة 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        اكسر;
    حالة 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        اكسر;
    افتراضي:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        اكسر;
    }
    physbits(1);
}

ساكن صحيح qemu_early_e820(فراغ);

فراغ
qemu_preinit(فراغ)
{
    qemu_detect();
    kvm_detect();

    إذا (!CONFIG_QEMU)
        أرجع;

    إذا (runningOnXen()) {
        xen_ramsize_preinit();
        أرجع;
    }

    // try read e820 table first
    إذا (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_نزم) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_بەرز) << 24));
        إذا (rs)
            rs += 16 * 1024 * 1024;
        وإلا
            rs = (((rtc_read(CMOS_MEM_EXTMEM_نزم) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_بەرز) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

ساكن فراغ msr_feature_control_setup(فراغ)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    إذا (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

فراغ
qemu_platform_setup(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    إذا (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        أرجع;
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
    إذا (Max_ژماردن_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    إذا (CONFIG_FW_ROMFILE_LOAD) {
        صحيح loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        إذا (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            أرجع;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        إذا (!loader_err)
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

ساكن فراغ
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

ساكن فراغ
qemu_cfg_dma_transfer(فراغ *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_نزم);

    طالما(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

ساكن فراغ
qemu_cfg_read(فراغ *buf, صحيح len)
{
    إذا (len == 0) {
        أرجع;
    }

    إذا (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } وإلا {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

ساكن فراغ
qemu_cfg_write(فراغ *buf, صحيح len)
{
    إذا (len == 0) {
        أرجع;
    }

    إذا (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } وإلا {
        warn_internalerror();
    }
}

ساكن فراغ
qemu_cfg_skip(صحيح len)
{
    إذا (len == 0) {
        أرجع;
    }

    إذا (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } وإلا {
        طالما (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

ساكن فراغ
qemu_cfg_read_entry(فراغ *buf, صحيح e, صحيح len)
{
    إذا (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } وإلا {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

ساكن فراغ
qemu_cfg_write_entry(فراغ *buf, صحيح e, صحيح len)
{
    إذا (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } وإلا {
        warn_internalerror();
    }
}

بنية qemu_romfile_s {
    بنية romfile_s file;
    صحيح select, skip;
};

ساكن صحيح
qemu_cfg_read_file(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    إذا (file->size > maxlen)
        أرجع -1;
    بنية qemu_romfile_s *qfile;
    qfile = container_of(file, بنية qemu_romfile_s, file);
    إذا (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } وإلا {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    أرجع file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
صحيح
qemu_cfg_write_file_simple(فراغ *src, u16 کلیل, u32 offset, u32 len)
{
    إذا (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, کلیل, len);
    } وإلا {
        qemu_cfg_select(کلیل);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    أرجع len;
}

صحيح
qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 offset, u32 len)
{
    إذا ((offset + len) > file->size)
        أرجع -1;

    إذا (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        أرجع -1;
    }
    أرجع qemu_cfg_write_file_simple(src, qemu_get_romfile_کلیل(file),
                                      offset, len);
}

ساكن فراغ
qemu_romfile_add(حرف *name, صحيح select, صحيح skip, صحيح size)
{
    بنية qemu_romfile_s *qfile = malloc_tmp(حجم(*qfile));
    إذا (!qfile) {
        warn_noalloc();
        أرجع;
    }
    memset(qfile, 0, حجم(*qfile));
    strtcpy(qfile->file.name, name, حجم(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_کلیل(بنية romfile_s *file)
{
    بنية qemu_romfile_s *qfile;
    إذا (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        أرجع 0;
    }
    qfile = container_of(file, بنية qemu_romfile_s, file);
    أرجع qfile->select;
}

ساكن صحيح rtc_present(فراغ)
{
    أرجع rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_ژماردن(فراغ)
{
    u16 smp_ژماردن = 0;
    إذا (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_ژماردن, QEMU_CFG_NB_CPUS, حجم(smp_ژماردن));
    }
    إذا (rtc_present()) {
        u16 cmos_cpu_ژماردن = rtc_read(CMOS_BIOS_SMP_ژماردن) + 1;
        إذا (smp_ژماردن < cmos_cpu_ژماردن) {
            smp_ژماردن = cmos_cpu_ژماردن;
        }
    }
    أرجع smp_ژماردن;
}

بنية e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

بنية qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ساكن فراغ
qemu_cfg_e820(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    إذا (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        أرجع;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, حجم(count32));
    إذا (count32) {
        بنية e820_reservation entry;
        صحيح i;
        لكل (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, حجم(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } وإلا إذا (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 بەرز = ((rtc_read(CMOS_MEM_HIGHMEM_نزم) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_بەرز) << 32));
    RamSizeOver4G = بەرز;
    e820_add(0x100000000ull, بەرز, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ساكن فراغ
qemu_cfg_legacy(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, حجم(numacount));
    صحيح max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, حجم(numacount)
                     , max_cpu*حجم(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , حجم(numacount) + max_cpu*حجم(u64)
                     , numacount*حجم(u64));

    // ACPI tables
    حرف name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, حجم(cnt));
    صحيح i, offset = حجم(cnt);
    لكل (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, حجم(len));
        offset += حجم(len);
        snprintf(name, حجم(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, حجم(cnt));
    offset = حجم(cnt);
    لكل (i = 0; i < cnt; i++) {
        بنية qemu_smbios_header header;
        qemu_cfg_read(&header, حجم(header));
        إذا (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, حجم(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + حجم(header)
                             , header.length - حجم(header));
        } وإلا {
            snprintf(name, حجم(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - حجم(header));
        offset += header.length;
    }
}

بنية QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    حرف name[56];
};

ساكن صحيح qemu_cfg_detect(فراغ)
{
    إذا (cfg_enabled)
        أرجع 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    حرف *sig = "QEMU";
    صحيح i;
    لكل (i = 0; i < 4; i++)
        إذا (inb(PORT_QEMU_CFG_DATA) != sig[i])
            أرجع 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, حجم(id));

    إذا (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    أرجع 1;
}

فراغ qemu_cfg_init(فراغ)
{
    إذا (!runningOnQEMU())
        أرجع;

    إذا (!qemu_cfg_detect())
        أرجع;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 ژماردن;
    qemu_cfg_read_entry(&ژماردن, QEMU_CFG_FILE_DIR, حجم(ژماردن));
    ژماردن = be32_to_cpu(ژماردن);
    u32 e;
    لكل (e = 0; e < ژماردن; e++) {
        بنية QemuCfgFile qfile;
        qemu_cfg_read(&qfile, حجم(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    إذا (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, حجم(nogfx));
    إذا (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ساكن صحيح qemu_early_e820(فراغ)
{
    بنية e820_reservation table;
    بنية QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 ژماردن, i;

    إذا (!qemu_cfg_detect())
        أرجع 0;

    // find e820 table
    qemu_cfg_read_entry(&ژماردن, QEMU_CFG_FILE_DIR, حجم(ژماردن));
    ژماردن = be32_to_cpu(ژماردن);
    لكل (i = 0; i < ژماردن; i++) {
        qemu_cfg_read(&qfile, حجم(qfile));
        إذا (memcmp(qfile.name, "etc/e820", 9) != 0)
            استمر;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        اكسر;
    }
    إذا (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        أرجع 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    ژماردن = size/حجم(table);
    لكل (i = 0, select = 0; i < ژماردن; i++) {
        qemu_cfg_read(&table, حجم(table));
        بدل (table.type) {
        حالة E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            اكسر;
        حالة E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            اكسر;
        حالة E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            إذا (table.address < 0x100000000LL) {
                // below 4g
                إذا (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } وإلا {
                // above 4g
                إذا (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    أرجع 1;
}
