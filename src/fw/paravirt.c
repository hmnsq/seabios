// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "byteorder.h" // be32_to_cpu
#અન્તર્ભાવ "config.h" // CONFIG_QEMU
#અન્તર્ભાવ "e820map.h" // e820_add
#અન્તર્ભાવ "hw/pci.h" // pci_config_readw
#અન્તર્ભાવ "hw/pcidevice.h" // pci_probe_devices
#અન્તર્ભાવ "hw/pci_regs.h" // PCI_DEVICE_ID
#અન્તર્ભાવ "hw/serialio.h" // PORT_SERIAL1
#અન્તર્ભાવ "hw/rtc.h" // CMOS_*
#અન્તર્ભાવ "hw/virtio-mmio.h" // virtio_mmio_acpi
#અન્તર્ભાવ "malloc.h" // malloc_tmp
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "paravirt.h" // qemu_cfg_preinit
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "romfile_loader.h" // romfile_loader_execute
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // pci_setup
#અન્તર્ભાવ "x86.h" // cpuid
#અન્તર્ભાવ "xen.h" // xen_biostable_setup
#અન્તર્ભાવ "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
પૂર્ણાંક PlatformRunningOn VARFSEG;
// cfg enabled
પૂર્ણાંક cfg_enabled = 0;
// cfg_dma enabled
પૂર્ણાંક cfg_dma_enabled = 0;

અંતઃરેખ પૂર્ણાંક qemu_cfg_enabled(શૂન્ય)
{
    પાછા cfg_enabled;
}

અંતઃરેખ પૂર્ણાંક qemu_cfg_dma_enabled(શૂન્ય)
{
    પાછા cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
સ્થિર unsigned પૂર્ણાંક kvm_cpuid_base = 0;

સ્થિર શૂન્ય kvm_detect(શૂન્ય)
{
    unsigned પૂર્ણાંક i, max = 0;
    unsigned પૂર્ણાંક eax, ebx, ecx, edx;
    અક્ષર signature[13];

    માટે (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        જો (eax < i)
            તોડો;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        જો (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    જો (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        જો (max >= kvm_cpuid_base + 0x10) {
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

સંરચના pvclock_vcpu_time_info *kvmclock;

સ્થિર શૂન્ય kvmclock_init(શૂન્ય)
{
    unsigned પૂર્ણાંક eax, ebx, ecx, edx, msr;

    જો (!runningOnKVM())
        પાછા;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    જો (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    નહીંતર જો (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    નહીંતર
        પાછા;

    kvmclock = memalign_નીચલું(કદ(*kvmclock), 32);
    memset(kvmclock, 0, કદ(*kvmclock));
    u32 કિંમત = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", કિંમત, msr);
    wrmsr(msr, કિંમત | 0x01);

    જો (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        પાછા;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    જો (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    નહીંતર
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

સ્થિર શૂન્ય physbits(પૂર્ણાંક qemu_quirk)
{
    unsigned પૂર્ણાંક max, eax, ebx, ecx, edx;
    unsigned પૂર્ણાંક physbits;
    અક્ષર signature[13];
    પૂર્ણાંક pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    જો (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    જો (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    જો (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        જો (!qemu_quirk) {
            valid = 1;
        } નહીંતર જો (physbits >= 41) {
            valid = 1;
        } નહીંતર જો (strcmp(signature, "GenuineIntel") == 0) {
            જો ((physbits == 36) || (physbits == 39))
                valid = 1;
        } નહીંતર જો (strcmp(signature, "AuthenticAMD") == 0) {
            જો (physbits == 40)
                valid = 1;
        }
    } નહીંતર {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    જો (valid)
        CPUPhysBits = physbits;
}

સ્થિર શૂન્ય qemu_detect(શૂન્ય)
{
    જો (!CONFIG_QEMU_HARDWARE)
        પાછા;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    જો (v == 0x0000 || v == 0xffff)
        પાછા;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    જો (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        પાછા;

    PlatformRunningOn |= PF_QEMU;
    પસંદગી (d) {
    સ્થિતિ 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        તોડો;
    સ્થિતિ 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        તોડો;
    મૂળભૂત:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        તોડો;
    }
    physbits(1);
}

સ્થિર પૂર્ણાંક qemu_early_e820(શૂન્ય);

શૂન્ય
qemu_preinit(શૂન્ય)
{
    qemu_detect();
    kvm_detect();

    જો (!CONFIG_QEMU)
        પાછા;

    જો (runningOnXen()) {
        xen_ramsize_preinit();
        પાછા;
    }

    // try read e820 table first
    જો (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_નીચલું) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_ઉપલું) << 24));
        જો (rs)
            rs += 16 * 1024 * 1024;
        નહીંતર
            rs = (((rtc_read(CMOS_MEM_EXTMEM_નીચલું) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_ઉપલું) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

સ્થિર શૂન્ય msr_feature_control_setup(શૂન્ય)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    જો (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

શૂન્ય
qemu_platform_setup(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    જો (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        પાછા;
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
    જો (Max_ગણતરી_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    જો (CONFIG_FW_ROMFILE_LOAD) {
        પૂર્ણાંક loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        જો (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            પાછા;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        જો (!loader_err)
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

સ્થિર શૂન્ય
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

સ્થિર શૂન્ય
qemu_cfg_dma_transfer(શૂન્ય *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_નીચલું);

    જ્યારે(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

સ્થિર શૂન્ય
qemu_cfg_read(શૂન્ય *buf, પૂર્ણાંક len)
{
    જો (len == 0) {
        પાછા;
    }

    જો (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } નહીંતર {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

સ્થિર શૂન્ય
qemu_cfg_write(શૂન્ય *buf, પૂર્ણાંક len)
{
    જો (len == 0) {
        પાછા;
    }

    જો (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } નહીંતર {
        warn_internalerror();
    }
}

સ્થિર શૂન્ય
qemu_cfg_skip(પૂર્ણાંક len)
{
    જો (len == 0) {
        પાછા;
    }

    જો (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } નહીંતર {
        જ્યારે (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

સ્થિર શૂન્ય
qemu_cfg_read_entry(શૂન્ય *buf, પૂર્ણાંક e, પૂર્ણાંક len)
{
    જો (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } નહીંતર {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

સ્થિર શૂન્ય
qemu_cfg_write_entry(શૂન્ય *buf, પૂર્ણાંક e, પૂર્ણાંક len)
{
    જો (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } નહીંતર {
        warn_internalerror();
    }
}

સંરચના qemu_romfile_s {
    સંરચના romfile_s file;
    પૂર્ણાંક select, skip;
};

સ્થિર પૂર્ણાંક
qemu_cfg_read_file(સંરચના romfile_s *file, શૂન્ય *dst, u32 maxlen)
{
    જો (file->size > maxlen)
        પાછા -1;
    સંરચના qemu_romfile_s *qfile;
    qfile = container_of(file, સંરચના qemu_romfile_s, file);
    જો (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } નહીંતર {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    પાછા file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
પૂર્ણાંક
qemu_cfg_write_file_simple(શૂન્ય *src, u16 ચાવી, u32 offset, u32 len)
{
    જો (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, ચાવી, len);
    } નહીંતર {
        qemu_cfg_select(ચાવી);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    પાછા len;
}

પૂર્ણાંક
qemu_cfg_write_file(શૂન્ય *src, સંરચના romfile_s *file, u32 offset, u32 len)
{
    જો ((offset + len) > file->size)
        પાછા -1;

    જો (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        પાછા -1;
    }
    પાછા qemu_cfg_write_file_simple(src, qemu_get_romfile_ચાવી(file),
                                      offset, len);
}

સ્થિર શૂન્ય
qemu_romfile_add(અક્ષર *name, પૂર્ણાંક select, પૂર્ણાંક skip, પૂર્ણાંક size)
{
    સંરચના qemu_romfile_s *qfile = malloc_tmp(કદ(*qfile));
    જો (!qfile) {
        warn_noalloc();
        પાછા;
    }
    memset(qfile, 0, કદ(*qfile));
    strtcpy(qfile->file.name, name, કદ(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_ચાવી(સંરચના romfile_s *file)
{
    સંરચના qemu_romfile_s *qfile;
    જો (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        પાછા 0;
    }
    qfile = container_of(file, સંરચના qemu_romfile_s, file);
    પાછા qfile->select;
}

સ્થિર પૂર્ણાંક rtc_present(શૂન્ય)
{
    પાછા rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_ગણતરી(શૂન્ય)
{
    u16 smp_ગણતરી = 0;
    જો (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_ગણતરી, QEMU_CFG_NB_CPUS, કદ(smp_ગણતરી));
    }
    જો (rtc_present()) {
        u16 cmos_cpu_ગણતરી = rtc_read(CMOS_BIOS_SMP_ગણતરી) + 1;
        જો (smp_ગણતરી < cmos_cpu_ગણતરી) {
            smp_ગણતરી = cmos_cpu_ગણતરી;
        }
    }
    પાછા smp_ગણતરી;
}

સંરચના e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

સંરચના qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

સ્થિર શૂન્ય
qemu_cfg_e820(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    જો (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        પાછા;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, કદ(count32));
    જો (count32) {
        સંરચના e820_reservation entry;
        પૂર્ણાંક i;
        માટે (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, કદ(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } નહીંતર જો (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 ઉપલું = ((rtc_read(CMOS_MEM_HIGHMEM_નીચલું) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_ઉપલું) << 32));
    RamSizeOver4G = ઉપલું;
    e820_add(0x100000000ull, ઉપલું, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
સ્થિર શૂન્ય
qemu_cfg_legacy(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, કદ(numacount));
    પૂર્ણાંક max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, કદ(numacount)
                     , max_cpu*કદ(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , કદ(numacount) + max_cpu*કદ(u64)
                     , numacount*કદ(u64));

    // ACPI tables
    અક્ષર name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, કદ(cnt));
    પૂર્ણાંક i, offset = કદ(cnt);
    માટે (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, કદ(len));
        offset += કદ(len);
        snprintf(name, કદ(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, કદ(cnt));
    offset = કદ(cnt);
    માટે (i = 0; i < cnt; i++) {
        સંરચના qemu_smbios_header header;
        qemu_cfg_read(&header, કદ(header));
        જો (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, કદ(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + કદ(header)
                             , header.length - કદ(header));
        } નહીંતર {
            snprintf(name, કદ(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - કદ(header));
        offset += header.length;
    }
}

સંરચના QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    અક્ષર name[56];
};

સ્થિર પૂર્ણાંક qemu_cfg_detect(શૂન્ય)
{
    જો (cfg_enabled)
        પાછા 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    અક્ષર *sig = "QEMU";
    પૂર્ણાંક i;
    માટે (i = 0; i < 4; i++)
        જો (inb(PORT_QEMU_CFG_DATA) != sig[i])
            પાછા 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, કદ(id));

    જો (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    પાછા 1;
}

શૂન્ય qemu_cfg_init(શૂન્ય)
{
    જો (!runningOnQEMU())
        પાછા;

    જો (!qemu_cfg_detect())
        પાછા;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 ગણતરી;
    qemu_cfg_read_entry(&ગણતરી, QEMU_CFG_FILE_DIR, કદ(ગણતરી));
    ગણતરી = be32_to_cpu(ગણતરી);
    u32 e;
    માટે (e = 0; e < ગણતરી; e++) {
        સંરચના QemuCfgFile qfile;
        qemu_cfg_read(&qfile, કદ(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    જો (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, કદ(nogfx));
    જો (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
સ્થિર પૂર્ણાંક qemu_early_e820(શૂન્ય)
{
    સંરચના e820_reservation table;
    સંરચના QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 ગણતરી, i;

    જો (!qemu_cfg_detect())
        પાછા 0;

    // find e820 table
    qemu_cfg_read_entry(&ગણતરી, QEMU_CFG_FILE_DIR, કદ(ગણતરી));
    ગણતરી = be32_to_cpu(ગણતરી);
    માટે (i = 0; i < ગણતરી; i++) {
        qemu_cfg_read(&qfile, કદ(qfile));
        જો (memcmp(qfile.name, "etc/e820", 9) != 0)
            ચાલુ;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        તોડો;
    }
    જો (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        પાછા 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    ગણતરી = size/કદ(table);
    માટે (i = 0, select = 0; i < ગણતરી; i++) {
        qemu_cfg_read(&table, કદ(table));
        પસંદગી (table.type) {
        સ્થિતિ E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            તોડો;
        સ્થિતિ E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            તોડો;
        સ્થિતિ E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            જો (table.address < 0x100000000LL) {
                // below 4g
                જો (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } નહીંતર {
                // above 4g
                જો (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    પાછા 1;
}
