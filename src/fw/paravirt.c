// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "byteorder.h" // be32_to_cpu
#شامل "config.h" // CONFIG_QEMU
#شامل "e820map.h" // e820_add
#شامل "hw/pci.h" // pci_config_readw
#شامل "hw/pcidevice.h" // pci_probe_devices
#شامل "hw/pci_regs.h" // PCI_DEVICE_ID
#شامل "hw/serialio.h" // PORT_SERIAL1
#شامل "hw/rtc.h" // CMOS_*
#شامل "hw/virtio-mmio.h" // virtio_mmio_acpi
#شامل "malloc.h" // malloc_tmp
#شامل "output.h" // dprintf
#شامل "paravirt.h" // qemu_cfg_preinit
#شامل "romfile.h" // romfile_loadint
#شامل "romfile_loader.h" // romfile_loader_execute
#شامل "string.h" // memset
#شامل "util.h" // pci_setup
#شامل "x86.h" // cpuid
#شامل "xen.h" // xen_biostable_setup
#شامل "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
صحیح PlatformRunningOn VARFSEG;
// cfg enabled
صحیح cfg_enabled = 0;
// cfg_dma enabled
صحیح cfg_dma_enabled = 0;

درونخطی صحیح qemu_cfg_enabled(تهی)
{
    بازگشت cfg_enabled;
}

درونخطی صحیح qemu_cfg_dma_enabled(تهی)
{
    بازگشت cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
ایستا unsigned صحیح kvm_cpuid_base = 0;

ایستا تهی kvm_detect(تهی)
{
    unsigned صحیح i, max = 0;
    unsigned صحیح eax, ebx, ecx, edx;
    نویسه signature[13];

    برای (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        اگر (eax < i)
            بشکن;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        اگر (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    اگر (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        اگر (max >= kvm_cpuid_base + 0x10) {
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

ساختار pvclock_vcpu_time_info *kvmclock;

ایستا تهی kvmclock_init(تهی)
{
    unsigned صحیح eax, ebx, ecx, edx, msr;

    اگر (!runningOnKVM())
        بازگشت;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    اگر (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    وگرنه اگر (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    وگرنه
        بازگشت;

    kvmclock = memalign_کم(اندازه(*kvmclock), 32);
    memset(kvmclock, 0, اندازه(*kvmclock));
    u32 مقدار = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", مقدار, msr);
    wrmsr(msr, مقدار | 0x01);

    اگر (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        بازگشت;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    اگر (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    وگرنه
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ایستا تهی physbits(صحیح qemu_quirk)
{
    unsigned صحیح max, eax, ebx, ecx, edx;
    unsigned صحیح physbits;
    نویسه signature[13];
    صحیح pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    اگر (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    اگر (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    اگر (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        اگر (!qemu_quirk) {
            valid = 1;
        } وگرنه اگر (physbits >= 41) {
            valid = 1;
        } وگرنه اگر (strcmp(signature, "GenuineIntel") == 0) {
            اگر ((physbits == 36) || (physbits == 39))
                valid = 1;
        } وگرنه اگر (strcmp(signature, "AuthenticAMD") == 0) {
            اگر (physbits == 40)
                valid = 1;
        }
    } وگرنه {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    اگر (valid)
        CPUPhysBits = physbits;
}

ایستا تهی qemu_detect(تهی)
{
    اگر (!CONFIG_QEMU_HARDWARE)
        بازگشت;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    اگر (v == 0x0000 || v == 0xffff)
        بازگشت;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    اگر (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        بازگشت;

    PlatformRunningOn |= PF_QEMU;
    انتخاب (d) {
    حالت 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        بشکن;
    حالت 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        بشکن;
    پیشفرض:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        بشکن;
    }
    physbits(1);
}

ایستا صحیح qemu_early_e820(تهی);

تهی
qemu_preinit(تهی)
{
    qemu_detect();
    kvm_detect();

    اگر (!CONFIG_QEMU)
        بازگشت;

    اگر (runningOnXen()) {
        xen_ramsize_preinit();
        بازگشت;
    }

    // try read e820 table first
    اگر (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_کم) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_بالا) << 24));
        اگر (rs)
            rs += 16 * 1024 * 1024;
        وگرنه
            rs = (((rtc_read(CMOS_MEM_EXTMEM_کم) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_بالا) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

ایستا تهی msr_feature_control_setup(تهی)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    اگر (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

تهی
qemu_platform_setup(تهی)
{
    اگر (!CONFIG_QEMU)
        بازگشت;

    اگر (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        بازگشت;
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
    اگر (Max_شمار_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    اگر (CONFIG_FW_ROMFILE_LOAD) {
        صحیح loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        اگر (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            بازگشت;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        اگر (!loader_err)
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

ایستا تهی
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

ایستا تهی
qemu_cfg_dma_transfer(تهی *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_کم);

    وقتی(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

ایستا تهی
qemu_cfg_read(تهی *buf, صحیح len)
{
    اگر (len == 0) {
        بازگشت;
    }

    اگر (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } وگرنه {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

ایستا تهی
qemu_cfg_write(تهی *buf, صحیح len)
{
    اگر (len == 0) {
        بازگشت;
    }

    اگر (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } وگرنه {
        warn_internalerror();
    }
}

ایستا تهی
qemu_cfg_skip(صحیح len)
{
    اگر (len == 0) {
        بازگشت;
    }

    اگر (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } وگرنه {
        وقتی (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

ایستا تهی
qemu_cfg_read_entry(تهی *buf, صحیح e, صحیح len)
{
    اگر (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } وگرنه {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

ایستا تهی
qemu_cfg_write_entry(تهی *buf, صحیح e, صحیح len)
{
    اگر (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } وگرنه {
        warn_internalerror();
    }
}

ساختار qemu_romfile_s {
    ساختار romfile_s file;
    صحیح select, skip;
};

ایستا صحیح
qemu_cfg_read_file(ساختار romfile_s *file, تهی *dst, u32 maxlen)
{
    اگر (file->size > maxlen)
        بازگشت -1;
    ساختار qemu_romfile_s *qfile;
    qfile = container_of(file, ساختار qemu_romfile_s, file);
    اگر (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } وگرنه {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    بازگشت file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
صحیح
qemu_cfg_write_file_simple(تهی *src, u16 کلید, u32 offset, u32 len)
{
    اگر (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, کلید, len);
    } وگرنه {
        qemu_cfg_select(کلید);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    بازگشت len;
}

صحیح
qemu_cfg_write_file(تهی *src, ساختار romfile_s *file, u32 offset, u32 len)
{
    اگر ((offset + len) > file->size)
        بازگشت -1;

    اگر (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        بازگشت -1;
    }
    بازگشت qemu_cfg_write_file_simple(src, qemu_get_romfile_کلید(file),
                                      offset, len);
}

ایستا تهی
qemu_romfile_add(نویسه *name, صحیح select, صحیح skip, صحیح size)
{
    ساختار qemu_romfile_s *qfile = malloc_tmp(اندازه(*qfile));
    اگر (!qfile) {
        warn_noalloc();
        بازگشت;
    }
    memset(qfile, 0, اندازه(*qfile));
    strtcpy(qfile->file.name, name, اندازه(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_کلید(ساختار romfile_s *file)
{
    ساختار qemu_romfile_s *qfile;
    اگر (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        بازگشت 0;
    }
    qfile = container_of(file, ساختار qemu_romfile_s, file);
    بازگشت qfile->select;
}

ایستا صحیح rtc_present(تهی)
{
    بازگشت rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_شمار(تهی)
{
    u16 smp_شمار = 0;
    اگر (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_شمار, QEMU_CFG_NB_CPUS, اندازه(smp_شمار));
    }
    اگر (rtc_present()) {
        u16 cmos_cpu_شمار = rtc_read(CMOS_BIOS_SMP_شمار) + 1;
        اگر (smp_شمار < cmos_cpu_شمار) {
            smp_شمار = cmos_cpu_شمار;
        }
    }
    بازگشت smp_شمار;
}

ساختار e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

ساختار qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ایستا تهی
qemu_cfg_e820(تهی)
{
    اگر (!CONFIG_QEMU)
        بازگشت;

    اگر (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        بازگشت;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, اندازه(count32));
    اگر (count32) {
        ساختار e820_reservation entry;
        صحیح i;
        برای (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, اندازه(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } وگرنه اگر (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 بالا = ((rtc_read(CMOS_MEM_HIGHMEM_کم) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_بالا) << 32));
    RamSizeOver4G = بالا;
    e820_add(0x100000000ull, بالا, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ایستا تهی
qemu_cfg_legacy(تهی)
{
    اگر (!CONFIG_QEMU)
        بازگشت;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, اندازه(numacount));
    صحیح max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, اندازه(numacount)
                     , max_cpu*اندازه(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , اندازه(numacount) + max_cpu*اندازه(u64)
                     , numacount*اندازه(u64));

    // ACPI tables
    نویسه name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, اندازه(cnt));
    صحیح i, offset = اندازه(cnt);
    برای (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, اندازه(len));
        offset += اندازه(len);
        snprintf(name, اندازه(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, اندازه(cnt));
    offset = اندازه(cnt);
    برای (i = 0; i < cnt; i++) {
        ساختار qemu_smbios_header header;
        qemu_cfg_read(&header, اندازه(header));
        اگر (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, اندازه(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + اندازه(header)
                             , header.length - اندازه(header));
        } وگرنه {
            snprintf(name, اندازه(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - اندازه(header));
        offset += header.length;
    }
}

ساختار QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    نویسه name[56];
};

ایستا صحیح qemu_cfg_detect(تهی)
{
    اگر (cfg_enabled)
        بازگشت 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    نویسه *sig = "QEMU";
    صحیح i;
    برای (i = 0; i < 4; i++)
        اگر (inb(PORT_QEMU_CFG_DATA) != sig[i])
            بازگشت 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, اندازه(id));

    اگر (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    بازگشت 1;
}

تهی qemu_cfg_init(تهی)
{
    اگر (!runningOnQEMU())
        بازگشت;

    اگر (!qemu_cfg_detect())
        بازگشت;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 شمار;
    qemu_cfg_read_entry(&شمار, QEMU_CFG_FILE_DIR, اندازه(شمار));
    شمار = be32_to_cpu(شمار);
    u32 e;
    برای (e = 0; e < شمار; e++) {
        ساختار QemuCfgFile qfile;
        qemu_cfg_read(&qfile, اندازه(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    اگر (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, اندازه(nogfx));
    اگر (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ایستا صحیح qemu_early_e820(تهی)
{
    ساختار e820_reservation table;
    ساختار QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 شمار, i;

    اگر (!qemu_cfg_detect())
        بازگشت 0;

    // find e820 table
    qemu_cfg_read_entry(&شمار, QEMU_CFG_FILE_DIR, اندازه(شمار));
    شمار = be32_to_cpu(شمار);
    برای (i = 0; i < شمار; i++) {
        qemu_cfg_read(&qfile, اندازه(qfile));
        اگر (memcmp(qfile.name, "etc/e820", 9) != 0)
            ادامه;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        بشکن;
    }
    اگر (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        بازگشت 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    شمار = size/اندازه(table);
    برای (i = 0, select = 0; i < شمار; i++) {
        qemu_cfg_read(&table, اندازه(table));
        انتخاب (table.type) {
        حالت E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            بشکن;
        حالت E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            بشکن;
        حالت E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            اگر (table.address < 0x100000000LL) {
                // below 4g
                اگر (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } وگرنه {
                // above 4g
                اگر (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    بازگشت 1;
}
