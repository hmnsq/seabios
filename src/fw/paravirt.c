// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "byteorder.h" // be32_to_cpu
#അന്തര്ഭാവ "config.h" // CONFIG_QEMU
#അന്തര്ഭാവ "e820map.h" // e820_add
#അന്തര്ഭാവ "hw/pci.h" // pci_config_readw
#അന്തര്ഭാവ "hw/pcidevice.h" // pci_probe_devices
#അന്തര്ഭാവ "hw/pci_regs.h" // PCI_DEVICE_ID
#അന്തര്ഭാവ "hw/serialio.h" // PORT_SERIAL1
#അന്തര്ഭാവ "hw/rtc.h" // CMOS_*
#അന്തര്ഭാവ "hw/virtio-mmio.h" // virtio_mmio_acpi
#അന്തര്ഭാവ "malloc.h" // malloc_tmp
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "paravirt.h" // qemu_cfg_preinit
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "romfile_loader.h" // romfile_loader_execute
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // pci_setup
#അന്തര്ഭാവ "x86.h" // cpuid
#അന്തര്ഭാവ "xen.h" // xen_biostable_setup
#അന്തര്ഭാവ "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 RamSize;
// Amount of continuous ram >4Gig
u64 RamSizeOver4G;
// physical address space bits
u8 CPUPhysBits;
// 64bit processor
u8 CPULongMode;
// Type of emulator platform.
പൂർണ്ണസംഖ്യ PlatformRunningOn VARFSEG;
// cfg enabled
പൂർണ്ണസംഖ്യ cfg_enabled = 0;
// cfg_dma enabled
പൂർണ്ണസംഖ്യ cfg_dma_enabled = 0;

വരിയിൽ പൂർണ്ണസംഖ്യ qemu_cfg_enabled(ശൂന്യം)
{
    തിരികെ cfg_enabled;
}

വരിയിൽ പൂർണ്ണസംഖ്യ qemu_cfg_dma_enabled(ശൂന്യം)
{
    തിരികെ cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ kvm_cpuid_base = 0;

സ്ഥിതം ശൂന്യം kvm_detect(ശൂന്യം)
{
    unsigned പൂർണ്ണസംഖ്യ i, max = 0;
    unsigned പൂർണ്ണസംഖ്യ eax, ebx, ecx, edx;
    അക്ഷരം signature[13];

    വേണ്ടി (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        എങ്കിൽ (eax < i)
            നിർത്തുക;
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        എങ്കിൽ (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    എങ്കിൽ (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        എങ്കിൽ (max >= kvm_cpuid_base + 0x10) {
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

ഘടന pvclock_vcpu_time_info *kvmclock;

സ്ഥിതം ശൂന്യം kvmclock_init(ശൂന്യം)
{
    unsigned പൂർണ്ണസംഖ്യ eax, ebx, ecx, edx, msr;

    എങ്കിൽ (!runningOnKVM())
        തിരികെ;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    എങ്കിൽ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    അല്ലെങ്കിൽ എങ്കിൽ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    അല്ലെങ്കിൽ
        തിരികെ;

    kvmclock = memalign_താഴെയുള്ള(വലുപ്പം(*kvmclock), 32);
    memset(kvmclock, 0, വലുപ്പം(*kvmclock));
    u32 മൂല്യം = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", മൂല്യം, msr);
    wrmsr(msr, മൂല്യം | 0x01);

    എങ്കിൽ (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        തിരികെ;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    എങ്കിൽ (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    അല്ലെങ്കിൽ
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

സ്ഥിതം ശൂന്യം physbits(പൂർണ്ണസംഖ്യ qemu_quirk)
{
    unsigned പൂർണ്ണസംഖ്യ max, eax, ebx, ecx, edx;
    unsigned പൂർണ്ണസംഖ്യ physbits;
    അക്ഷരം signature[13];
    പൂർണ്ണസംഖ്യ pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(signature + 0, &ebx, 4);
    memcpy(signature + 4, &edx, 4);
    memcpy(signature + 8, &ecx, 4);
    signature[12] = 0;
    എങ്കിൽ (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    എങ്കിൽ (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    എങ്കിൽ (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        എങ്കിൽ (!qemu_quirk) {
            valid = 1;
        } അല്ലെങ്കിൽ എങ്കിൽ (physbits >= 41) {
            valid = 1;
        } അല്ലെങ്കിൽ എങ്കിൽ (strcmp(signature, "GenuineIntel") == 0) {
            എങ്കിൽ ((physbits == 36) || (physbits == 39))
                valid = 1;
        } അല്ലെങ്കിൽ എങ്കിൽ (strcmp(signature, "AuthenticAMD") == 0) {
            എങ്കിൽ (physbits == 40)
                valid = 1;
        }
    } അല്ലെങ്കിൽ {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    എങ്കിൽ (valid)
        CPUPhysBits = physbits;
}

സ്ഥിതം ശൂന്യം qemu_detect(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU_HARDWARE)
        തിരികെ;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    എങ്കിൽ (v == 0x0000 || v == 0xffff)
        തിരികെ;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    എങ്കിൽ (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        തിരികെ;

    PlatformRunningOn |= PF_QEMU;
    തിരഞ്ഞെടുപ്പ് (d) {
    അവസ്ഥ 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        നിർത്തുക;
    അവസ്ഥ 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        നിർത്തുക;
    }
    physbits(1);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ qemu_early_e820(ശൂന്യം);

ശൂന്യം
qemu_preinit(ശൂന്യം)
{
    qemu_detect();
    kvm_detect();

    എങ്കിൽ (!CONFIG_QEMU)
        തിരികെ;

    എങ്കിൽ (runningOnXen()) {
        xen_ramsize_preinit();
        തിരികെ;
    }

    // try read e820 table first
    എങ്കിൽ (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_താഴെയുള്ള) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_മുകളിലെ) << 24));
        എങ്കിൽ (rs)
            rs += 16 * 1024 * 1024;
        അല്ലെങ്കിൽ
            rs = (((rtc_read(CMOS_MEM_EXTMEM_താഴെയുള്ള) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_മുകളിലെ) << 18))
                  + 1 * 1024 * 1024);
        RamSize = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", RamSize);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#define MSR_IA32_FEATURE_CONTROL 0x0000003a

സ്ഥിതം ശൂന്യം msr_feature_control_setup(ശൂന്യം)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    എങ്കിൽ (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

ശൂന്യം
qemu_platform_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU)
        തിരികെ;

    എങ്കിൽ (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        തിരികെ;
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
    എങ്കിൽ (Max_എണ്ണം_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    എങ്കിൽ (CONFIG_FW_ROMFILE_LOAD) {
        പൂർണ്ണസംഖ്യ loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        എങ്കിൽ (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            തിരികെ;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        എങ്കിൽ (!loader_err)
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

സ്ഥിതം ശൂന്യം
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

സ്ഥിതം ശൂന്യം
qemu_cfg_dma_transfer(ശൂന്യം *address, u32 length, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.length = cpu_to_be32(length);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_താഴെയുള്ള);

    ആകുമ്പോൾ(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

സ്ഥിതം ശൂന്യം
qemu_cfg_read(ശൂന്യം *buf, പൂർണ്ണസംഖ്യ len)
{
    എങ്കിൽ (len == 0) {
        തിരികെ;
    }

    എങ്കിൽ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } അല്ലെങ്കിൽ {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

സ്ഥിതം ശൂന്യം
qemu_cfg_write(ശൂന്യം *buf, പൂർണ്ണസംഖ്യ len)
{
    എങ്കിൽ (len == 0) {
        തിരികെ;
    }

    എങ്കിൽ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } അല്ലെങ്കിൽ {
        warn_internalerror();
    }
}

സ്ഥിതം ശൂന്യം
qemu_cfg_skip(പൂർണ്ണസംഖ്യ len)
{
    എങ്കിൽ (len == 0) {
        തിരികെ;
    }

    എങ്കിൽ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } അല്ലെങ്കിൽ {
        ആകുമ്പോൾ (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

സ്ഥിതം ശൂന്യം
qemu_cfg_read_entry(ശൂന്യം *buf, പൂർണ്ണസംഖ്യ e, പൂർണ്ണസംഖ്യ len)
{
    എങ്കിൽ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } അല്ലെങ്കിൽ {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

സ്ഥിതം ശൂന്യം
qemu_cfg_write_entry(ശൂന്യം *buf, പൂർണ്ണസംഖ്യ e, പൂർണ്ണസംഖ്യ len)
{
    എങ്കിൽ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } അല്ലെങ്കിൽ {
        warn_internalerror();
    }
}

ഘടന qemu_romfile_s {
    ഘടന romfile_s file;
    പൂർണ്ണസംഖ്യ select, skip;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ
qemu_cfg_read_file(ഘടന romfile_s *file, ശൂന്യം *dst, u32 maxlen)
{
    എങ്കിൽ (file->size > maxlen)
        തിരികെ -1;
    ഘടന qemu_romfile_s *qfile;
    qfile = container_of(file, ഘടന qemu_romfile_s, file);
    എങ്കിൽ (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } അല്ലെങ്കിൽ {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    തിരികെ file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
പൂർണ്ണസംഖ്യ
qemu_cfg_write_file_simple(ശൂന്യം *src, u16 താക്കോൽ, u32 offset, u32 len)
{
    എങ്കിൽ (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, താക്കോൽ, len);
    } അല്ലെങ്കിൽ {
        qemu_cfg_select(താക്കോൽ);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    തിരികെ len;
}

പൂർണ്ണസംഖ്യ
qemu_cfg_write_file(ശൂന്യം *src, ഘടന romfile_s *file, u32 offset, u32 len)
{
    എങ്കിൽ ((offset + len) > file->size)
        തിരികെ -1;

    എങ്കിൽ (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        തിരികെ -1;
    }
    തിരികെ qemu_cfg_write_file_simple(src, qemu_get_romfile_താക്കോൽ(file),
                                      offset, len);
}

സ്ഥിതം ശൂന്യം
qemu_romfile_add(അക്ഷരം *name, പൂർണ്ണസംഖ്യ select, പൂർണ്ണസംഖ്യ skip, പൂർണ്ണസംഖ്യ size)
{
    ഘടന qemu_romfile_s *qfile = malloc_tmp(വലുപ്പം(*qfile));
    എങ്കിൽ (!qfile) {
        warn_noalloc();
        തിരികെ;
    }
    memset(qfile, 0, വലുപ്പം(*qfile));
    strtcpy(qfile->file.name, name, വലുപ്പം(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_താക്കോൽ(ഘടന romfile_s *file)
{
    ഘടന qemu_romfile_s *qfile;
    എങ്കിൽ (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        തിരികെ 0;
    }
    qfile = container_of(file, ഘടന qemu_romfile_s, file);
    തിരികെ qfile->select;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ rtc_present(ശൂന്യം)
{
    തിരികെ rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_എണ്ണം(ശൂന്യം)
{
    u16 smp_എണ്ണം = 0;
    എങ്കിൽ (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_എണ്ണം, QEMU_CFG_NB_CPUS, വലുപ്പം(smp_എണ്ണം));
    }
    എങ്കിൽ (rtc_present()) {
        u16 cmos_cpu_എണ്ണം = rtc_read(CMOS_BIOS_SMP_എണ്ണം) + 1;
        എങ്കിൽ (smp_എണ്ണം < cmos_cpu_എണ്ണം) {
            smp_എണ്ണം = cmos_cpu_എണ്ണം;
        }
    }
    തിരികെ smp_എണ്ണം;
}

ഘടന e820_reservation {
    u64 address;
    u64 length;
    u32 type;
};

#define SMBIOS_FIELD_ENTRY 0
#define SMBIOS_TABLE_ENTRY 1

ഘടന qemu_smbios_header {
    u16 length;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

സ്ഥിതം ശൂന്യം
qemu_cfg_e820(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU)
        തിരികെ;

    എങ്കിൽ (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        തിരികെ;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, വലുപ്പം(count32));
    എങ്കിൽ (count32) {
        ഘടന e820_reservation entry;
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, വലുപ്പം(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } അല്ലെങ്കിൽ എങ്കിൽ (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 മുകളിലെ = ((rtc_read(CMOS_MEM_HIGHMEM_താഴെയുള്ള) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_മുകളിലെ) << 32));
    RamSizeOver4G = മുകളിലെ;
    e820_add(0x100000000ull, മുകളിലെ, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
സ്ഥിതം ശൂന്യം
qemu_cfg_legacy(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU)
        തിരികെ;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, വലുപ്പം(numacount));
    പൂർണ്ണസംഖ്യ max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, വലുപ്പം(numacount)
                     , max_cpu*വലുപ്പം(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , വലുപ്പം(numacount) + max_cpu*വലുപ്പം(u64)
                     , numacount*വലുപ്പം(u64));

    // ACPI tables
    അക്ഷരം name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, വലുപ്പം(cnt));
    പൂർണ്ണസംഖ്യ i, offset = വലുപ്പം(cnt);
    വേണ്ടി (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, വലുപ്പം(len));
        offset += വലുപ്പം(len);
        snprintf(name, വലുപ്പം(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, വലുപ്പം(cnt));
    offset = വലുപ്പം(cnt);
    വേണ്ടി (i = 0; i < cnt; i++) {
        ഘടന qemu_smbios_header header;
        qemu_cfg_read(&header, വലുപ്പം(header));
        എങ്കിൽ (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, വലുപ്പം(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + വലുപ്പം(header)
                             , header.length - വലുപ്പം(header));
        } അല്ലെങ്കിൽ {
            snprintf(name, വലുപ്പം(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - വലുപ്പം(header));
        offset += header.length;
    }
}

ഘടന QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    അക്ഷരം name[56];
};

സ്ഥിതം പൂർണ്ണസംഖ്യ qemu_cfg_detect(ശൂന്യം)
{
    എങ്കിൽ (cfg_enabled)
        തിരികെ 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    അക്ഷരം *sig = "QEMU";
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < 4; i++)
        എങ്കിൽ (inb(PORT_QEMU_CFG_DATA) != sig[i])
            തിരികെ 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, വലുപ്പം(id));

    എങ്കിൽ (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    തിരികെ 1;
}

ശൂന്യം qemu_cfg_init(ശൂന്യം)
{
    എങ്കിൽ (!runningOnQEMU())
        തിരികെ;

    എങ്കിൽ (!qemu_cfg_detect())
        തിരികെ;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 എണ്ണം;
    qemu_cfg_read_entry(&എണ്ണം, QEMU_CFG_FILE_DIR, വലുപ്പം(എണ്ണം));
    എണ്ണം = be32_to_cpu(എണ്ണം);
    u32 e;
    വേണ്ടി (e = 0; e < എണ്ണം; e++) {
        ഘടന QemuCfgFile qfile;
        qemu_cfg_read(&qfile, വലുപ്പം(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    എങ്കിൽ (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, വലുപ്പം(nogfx));
    എങ്കിൽ (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ qemu_early_e820(ശൂന്യം)
{
    ഘടന e820_reservation table;
    ഘടന QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 എണ്ണം, i;

    എങ്കിൽ (!qemu_cfg_detect())
        തിരികെ 0;

    // find e820 table
    qemu_cfg_read_entry(&എണ്ണം, QEMU_CFG_FILE_DIR, വലുപ്പം(എണ്ണം));
    എണ്ണം = be32_to_cpu(എണ്ണം);
    വേണ്ടി (i = 0; i < എണ്ണം; i++) {
        qemu_cfg_read(&qfile, വലുപ്പം(qfile));
        എങ്കിൽ (memcmp(qfile.name, "etc/e820", 9) != 0)
            തുടരുക;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        നിർത്തുക;
    }
    എങ്കിൽ (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        തിരികെ 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    എണ്ണം = size/വലുപ്പം(table);
    വേണ്ടി (i = 0, select = 0; i < എണ്ണം; i++) {
        qemu_cfg_read(&table, വലുപ്പം(table));
        തിരഞ്ഞെടുപ്പ് (table.type) {
        അവസ്ഥ E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            നിർത്തുക;
        അവസ്ഥ E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            നിർത്തുക;
        അവസ്ഥ E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            എങ്കിൽ (table.address < 0x100000000LL) {
                // below 4g
                എങ്കിൽ (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } അല്ലെങ്കിൽ {
                // above 4g
                എങ്കിൽ (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    തിരികെ 1;
}
