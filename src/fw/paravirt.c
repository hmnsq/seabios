// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "byteorder.h" // be32_to_cpu
#அந்தர்பாவ "config.h" // CONFIG_QEMU
#அந்தர்பாவ "e820map.h" // e820_add
#அந்தர்பாவ "hw/pci.h" // pci_config_readw
#அந்தர்பாவ "hw/pcidevice.h" // pci_probe_devices
#அந்தர்பாவ "hw/pci_regs.h" // PCI_DEVICE_ID
#அந்தர்பாவ "hw/serialio.h" // PORT_SERIAL1
#அந்தர்பாவ "hw/rtc.h" // CMOS_*
#அந்தர்பாவ "hw/virtio-mmio.h" // virtio_mmio_acpi
#அந்தர்பாவ "malloc.h" // malloc_tmp
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "paravirt.h" // qemu_cfg_preinit
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "romfile_loader.h" // romfile_loader_execute
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // pci_setup
#அந்தர்பாவ "x86.h" // cpuid
#அந்தர்பாவ "xen.h" // xen_biostable_setup
#அந்தர்பாவ "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
முழு PlatformRunningOn VARFSEG;
// cfg enabled
முழு cfg_enabled = 0;
// cfg_dma enabled
முழு cfg_dma_enabled = 0;

உள்வரி முழு qemu_cfg_enabled(வெறுமை)
{
    திரும்பு cfg_enabled;
}

உள்வரி முழு qemu_cfg_dma_enabled(வெறுமை)
{
    திரும்பு cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
நிலையான unsigned முழு kvm_cpuid_base = 0;

நிலையான வெறுமை kvm_detect(வெறுமை)
{
    unsigned முழு i, max = 0;
    unsigned முழு eax, ebx, ecx, edx;
    எழுத்து signature[13];

    க்கு (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        எனில் (eax < i)
            நிறுத்து;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        எனில் (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    எனில் (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        எனில் (max >= kvm_cpuid_base + 0x10) {
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

கட்டமைப்பு pvclock_vcpu_time_info *kvmclock;

நிலையான வெறுமை kvmclock_init(வெறுமை)
{
    unsigned முழு eax, ebx, ecx, edx, msr;

    எனில் (!runningOnKVM())
        திரும்பு;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    எனில் (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    இல்லையெனில் எனில் (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    இல்லையெனில்
        திரும்பு;

    kvmclock = memalign_கீழ்(அளவு(*kvmclock), 32);
    memset(kvmclock, 0, அளவு(*kvmclock));
    u32 மதிப்பு = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", மதிப்பு, msr);
    wrmsr(msr, மதிப்பு | 0x01);

    எனில் (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        திரும்பு;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    எனில் (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    இல்லையெனில்
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

நிலையான வெறுமை physbits(முழு qemu_quirk)
{
    unsigned முழு max, eax, ebx, ecx, edx;
    unsigned முழு physbits;
    எழுத்து signature[13];
    முழு pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    எனில் (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    எனில் (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    எனில் (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        எனில் (!qemu_quirk) {
            valid = 1;
        } இல்லையெனில் எனில் (physbits >= 41) {
            valid = 1;
        } இல்லையெனில் எனில் (strcmp(signature, "GenuineIntel") == 0) {
            எனில் ((physbits == 36) || (physbits == 39))
                valid = 1;
        } இல்லையெனில் எனில் (strcmp(signature, "AuthenticAMD") == 0) {
            எனில் (physbits == 40)
                valid = 1;
        }
    } இல்லையெனில் {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    எனில் (valid)
        CPUPhysBits = physbits;
}

நிலையான வெறுமை qemu_detect(வெறுமை)
{
    எனில் (!CONFIG_QEMU_HARDWARE)
        திரும்பு;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    எனில் (v == 0x0000 || v == 0xffff)
        திரும்பு;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    எனில் (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        திரும்பு;

    PlatformRunningOn |= PF_QEMU;
    தேர்வு (d) {
    நிலை 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        நிறுத்து;
    நிலை 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        நிறுத்து;
    இயல்பு:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        நிறுத்து;
    }
    physbits(1);
}

நிலையான முழு qemu_early_e820(வெறுமை);

வெறுமை
qemu_preinit(வெறுமை)
{
    qemu_detect();
    kvm_detect();

    எனில் (!CONFIG_QEMU)
        திரும்பு;

    எனில் (runningOnXen()) {
        xen_ramsize_preinit();
        திரும்பு;
    }

    // try read e820 table first
    எனில் (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_கீழ்) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_உயர்வு) << 24));
        எனில் (rs)
            rs += 16 * 1024 * 1024;
        இல்லையெனில்
            rs = (((rtc_read(CMOS_MEM_EXTMEM_கீழ்) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_உயர்வு) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

நிலையான வெறுமை msr_feature_control_setup(வெறுமை)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    எனில் (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

வெறுமை
qemu_platform_setup(வெறுமை)
{
    எனில் (!CONFIG_QEMU)
        திரும்பு;

    எனில் (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        திரும்பு;
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
    எனில் (Max_மொத்தம்_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    எனில் (CONFIG_FW_ROMFILE_LOAD) {
        முழு loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        எனில் (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            திரும்பு;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        எனில் (!loader_err)
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

நிலையான வெறுமை
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

நிலையான வெறுமை
qemu_cfg_dma_transfer(வெறுமை *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_கீழ்);

    போது(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

நிலையான வெறுமை
qemu_cfg_read(வெறுமை *buf, முழு len)
{
    எனில் (len == 0) {
        திரும்பு;
    }

    எனில் (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } இல்லையெனில் {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

நிலையான வெறுமை
qemu_cfg_write(வெறுமை *buf, முழு len)
{
    எனில் (len == 0) {
        திரும்பு;
    }

    எனில் (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } இல்லையெனில் {
        warn_internalerror();
    }
}

நிலையான வெறுமை
qemu_cfg_skip(முழு len)
{
    எனில் (len == 0) {
        திரும்பு;
    }

    எனில் (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } இல்லையெனில் {
        போது (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

நிலையான வெறுமை
qemu_cfg_read_entry(வெறுமை *buf, முழு e, முழு len)
{
    எனில் (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } இல்லையெனில் {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

நிலையான வெறுமை
qemu_cfg_write_entry(வெறுமை *buf, முழு e, முழு len)
{
    எனில் (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } இல்லையெனில் {
        warn_internalerror();
    }
}

கட்டமைப்பு qemu_romfile_s {
    கட்டமைப்பு romfile_s file;
    முழு select, skip;
};

நிலையான முழு
qemu_cfg_read_file(கட்டமைப்பு romfile_s *file, வெறுமை *dst, u32 maxlen)
{
    எனில் (file->size > maxlen)
        திரும்பு -1;
    கட்டமைப்பு qemu_romfile_s *qfile;
    qfile = container_of(file, கட்டமைப்பு qemu_romfile_s, file);
    எனில் (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } இல்லையெனில் {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    திரும்பு file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
முழு
qemu_cfg_write_file_simple(வெறுமை *src, u16 சாவி, u32 offset, u32 len)
{
    எனில் (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, சாவி, len);
    } இல்லையெனில் {
        qemu_cfg_select(சாவி);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    திரும்பு len;
}

முழு
qemu_cfg_write_file(வெறுமை *src, கட்டமைப்பு romfile_s *file, u32 offset, u32 len)
{
    எனில் ((offset + len) > file->size)
        திரும்பு -1;

    எனில் (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        திரும்பு -1;
    }
    திரும்பு qemu_cfg_write_file_simple(src, qemu_get_romfile_சாவி(file),
                                      offset, len);
}

நிலையான வெறுமை
qemu_romfile_add(எழுத்து *name, முழு select, முழு skip, முழு size)
{
    கட்டமைப்பு qemu_romfile_s *qfile = malloc_tmp(அளவு(*qfile));
    எனில் (!qfile) {
        warn_noalloc();
        திரும்பு;
    }
    memset(qfile, 0, அளவு(*qfile));
    strtcpy(qfile->file.name, name, அளவு(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_சாவி(கட்டமைப்பு romfile_s *file)
{
    கட்டமைப்பு qemu_romfile_s *qfile;
    எனில் (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        திரும்பு 0;
    }
    qfile = container_of(file, கட்டமைப்பு qemu_romfile_s, file);
    திரும்பு qfile->select;
}

நிலையான முழு rtc_present(வெறுமை)
{
    திரும்பு rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_மொத்தம்(வெறுமை)
{
    u16 smp_மொத்தம் = 0;
    எனில் (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_மொத்தம், QEMU_CFG_NB_CPUS, அளவு(smp_மொத்தம்));
    }
    எனில் (rtc_present()) {
        u16 cmos_cpu_மொத்தம் = rtc_read(CMOS_BIOS_SMP_மொத்தம்) + 1;
        எனில் (smp_மொத்தம் < cmos_cpu_மொத்தம்) {
            smp_மொத்தம் = cmos_cpu_மொத்தம்;
        }
    }
    திரும்பு smp_மொத்தம்;
}

கட்டமைப்பு e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

கட்டமைப்பு qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

நிலையான வெறுமை
qemu_cfg_e820(வெறுமை)
{
    எனில் (!CONFIG_QEMU)
        திரும்பு;

    எனில் (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        திரும்பு;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, அளவு(count32));
    எனில் (count32) {
        கட்டமைப்பு e820_reservation entry;
        முழு i;
        க்கு (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, அளவு(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } இல்லையெனில் எனில் (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 உயர்வு = ((rtc_read(CMOS_MEM_HIGHMEM_கீழ்) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_உயர்வு) << 32));
    RamSizeOver4G = உயர்வு;
    e820_add(0x100000000ull, உயர்வு, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
நிலையான வெறுமை
qemu_cfg_legacy(வெறுமை)
{
    எனில் (!CONFIG_QEMU)
        திரும்பு;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, அளவு(numacount));
    முழு max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, அளவு(numacount)
                     , max_cpu*அளவு(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , அளவு(numacount) + max_cpu*அளவு(u64)
                     , numacount*அளவு(u64));

    // ACPI tables
    எழுத்து name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, அளவு(cnt));
    முழு i, offset = அளவு(cnt);
    க்கு (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, அளவு(len));
        offset += அளவு(len);
        snprintf(name, அளவு(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, அளவு(cnt));
    offset = அளவு(cnt);
    க்கு (i = 0; i < cnt; i++) {
        கட்டமைப்பு qemu_smbios_header header;
        qemu_cfg_read(&header, அளவு(header));
        எனில் (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, அளவு(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + அளவு(header)
                             , header.length - அளவு(header));
        } இல்லையெனில் {
            snprintf(name, அளவு(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - அளவு(header));
        offset += header.length;
    }
}

கட்டமைப்பு QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    எழுத்து name[56];
};

நிலையான முழு qemu_cfg_detect(வெறுமை)
{
    எனில் (cfg_enabled)
        திரும்பு 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    எழுத்து *sig = "QEMU";
    முழு i;
    க்கு (i = 0; i < 4; i++)
        எனில் (inb(PORT_QEMU_CFG_DATA) != sig[i])
            திரும்பு 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, அளவு(id));

    எனில் (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    திரும்பு 1;
}

வெறுமை qemu_cfg_init(வெறுமை)
{
    எனில் (!runningOnQEMU())
        திரும்பு;

    எனில் (!qemu_cfg_detect())
        திரும்பு;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 மொத்தம்;
    qemu_cfg_read_entry(&மொத்தம், QEMU_CFG_FILE_DIR, அளவு(மொத்தம்));
    மொத்தம் = be32_to_cpu(மொத்தம்);
    u32 e;
    க்கு (e = 0; e < மொத்தம்; e++) {
        கட்டமைப்பு QemuCfgFile qfile;
        qemu_cfg_read(&qfile, அளவு(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    எனில் (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, அளவு(nogfx));
    எனில் (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
நிலையான முழு qemu_early_e820(வெறுமை)
{
    கட்டமைப்பு e820_reservation table;
    கட்டமைப்பு QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 மொத்தம், i;

    எனில் (!qemu_cfg_detect())
        திரும்பு 0;

    // find e820 table
    qemu_cfg_read_entry(&மொத்தம், QEMU_CFG_FILE_DIR, அளவு(மொத்தம்));
    மொத்தம் = be32_to_cpu(மொத்தம்);
    க்கு (i = 0; i < மொத்தம்; i++) {
        qemu_cfg_read(&qfile, அளவு(qfile));
        எனில் (memcmp(qfile.name, "etc/e820", 9) != 0)
            தொடர்;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        நிறுத்து;
    }
    எனில் (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        திரும்பு 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    மொத்தம் = size/அளவு(table);
    க்கு (i = 0, select = 0; i < மொத்தம்; i++) {
        qemu_cfg_read(&table, அளவு(table));
        தேர்வு (table.type) {
        நிலை E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            நிறுத்து;
        நிலை E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            நிறுத்து;
        நிலை E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            எனில் (table.address < 0x100000000LL) {
                // below 4g
                எனில் (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } இல்லையெனில் {
                // above 4g
                எனில் (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    திரும்பு 1;
}
