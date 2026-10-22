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
पूर्णांक PlatformRunningOn VARFSEG;
// cfg enabled
पूर्णांक cfg_enabled = 0;
// cfg_dma enabled
पूर्णांक cfg_dma_enabled = 0;

ओळीत पूर्णांक qemu_cfg_enabled(शून्य)
{
    परत cfg_enabled;
}

ओळीत पूर्णांक qemu_cfg_dma_enabled(शून्य)
{
    परत cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#define KVM_CPUID_SIGNATURE     0x40000000
स्थिर अचिन्हित पूर्णांक kvm_cpuid_base = 0;

स्थिर शून्य kvm_detect(शून्य)
{
    अचिन्हित पूर्णांक i, max = 0;
    अचिन्हित पूर्णांक eax, ebx, ecx, edx;
    अक्षर signature[13];

    साठी (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        जर (eax < i)
            थांबा;
        स_नकल(signature + 0, &ebx, 4);
        स_नकल(signature + 4, &ecx, 4);
        स_नकल(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        जर (म_भेद(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    जर (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        जर (max >= kvm_cpuid_base + 0x10) {
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
    अचिन्हित पूर्णांक eax, ebx, ecx, edx, msr;

    जर (!runningOnKVM())
        परत;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    जर (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    अन्यथा जर (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    अन्यथा
        परत;

    kvmclock = memalign_खालचा(आकार(*kvmclock), 32);
    स_रखो(kvmclock, 0, आकार(*kvmclock));
    u32 मूल्य = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", मूल्य, msr);
    wrmsr(msr, मूल्य | 0x01);

    जर (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        परत;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    जर (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    अन्यथा
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

स्थिर शून्य physbits(पूर्णांक qemu_quirk)
{
    अचिन्हित पूर्णांक max, eax, ebx, ecx, edx;
    अचिन्हित पूर्णांक physbits;
    अक्षर signature[13];
    पूर्णांक pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    स_नकल(signature + 0, &ebx, 4);
    स_नकल(signature + 4, &edx, 4);
    स_नकल(signature + 8, &ecx, 4);
    signature[12] = 0;
    जर (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    जर (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    जर (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        जर (!qemu_quirk) {
            valid = 1;
        } अन्यथा जर (physbits >= 41) {
            valid = 1;
        } अन्यथा जर (म_भेद(signature, "GenuineIntel") == 0) {
            जर ((physbits == 36) || (physbits == 39))
                valid = 1;
        } अन्यथा जर (म_भेद(signature, "AuthenticAMD") == 0) {
            जर (physbits == 40)
                valid = 1;
        }
    } अन्यथा {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    जर (valid)
        CPUPhysBits = physbits;
}

स्थिर शून्य qemu_detect(शून्य)
{
    जर (!CONFIG_QEMU_HARDWARE)
        परत;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    जर (v == 0x0000 || v == 0xffff)
        परत;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    जर (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        परत;

    PlatformRunningOn |= PF_QEMU;
    निवड (d) {
    स्थिती 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        थांबा;
    स्थिती 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        थांबा;
    पूर्वनिर्धारित:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        थांबा;
    }
    physbits(1);
}

स्थिर पूर्णांक qemu_early_e820(शून्य);

शून्य
qemu_preinit(शून्य)
{
    qemu_detect();
    kvm_detect();

    जर (!CONFIG_QEMU)
        परत;

    जर (runningOnXen()) {
        xen_ramsize_preinit();
        परत;
    }

    // try read e820 table first
    जर (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_खालचा) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_वरचा) << 24));
        जर (rs)
            rs += 16 * 1024 * 1024;
        अन्यथा
            rs = (((rtc_read(CMOS_MEM_EXTMEM_खालचा) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_वरचा) << 18))
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
    जर (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

शून्य
qemu_platform_setup(शून्य)
{
    जर (!CONFIG_QEMU)
        परत;

    जर (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        परत;
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
    जर (Max_मोजणी_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    जर (CONFIG_FW_ROMFILE_LOAD) {
        पूर्णांक loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        जर (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            परत;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        जर (!loader_err)
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

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_खालचा);

    पर्यंत(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

स्थिर शून्य
qemu_cfg_read(शून्य *buf, पूर्णांक len)
{
    जर (len == 0) {
        परत;
    }

    जर (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } अन्यथा {
        insb(PORT_QEMU_CFG_DATA, buf, len);
    }
}

स्थिर शून्य
qemu_cfg_write(शून्य *buf, पूर्णांक len)
{
    जर (len == 0) {
        परत;
    }

    जर (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } अन्यथा {
        warn_internalerror();
    }
}

स्थिर शून्य
qemu_cfg_skip(पूर्णांक len)
{
    जर (len == 0) {
        परत;
    }

    जर (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } अन्यथा {
        पर्यंत (len--)
            inb(PORT_QEMU_CFG_DATA);
    }
}

स्थिर शून्य
qemu_cfg_read_entry(शून्य *buf, पूर्णांक e, पूर्णांक len)
{
    जर (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } अन्यथा {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

स्थिर शून्य
qemu_cfg_write_entry(शून्य *buf, पूर्णांक e, पूर्णांक len)
{
    जर (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } अन्यथा {
        warn_internalerror();
    }
}

संरचना qemu_romfile_s {
    संरचना romfile_s file;
    पूर्णांक select, skip;
};

स्थिर पूर्णांक
qemu_cfg_read_file(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    जर (file->size > maxlen)
        परत -1;
    संरचना qemu_romfile_s *qfile;
    qfile = container_of(file, संरचना qemu_romfile_s, file);
    जर (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->size);
    } अन्यथा {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->size);
    }
    परत file->size;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
पूर्णांक
qemu_cfg_write_file_simple(शून्य *src, u16 किल्ली, u32 offset, u32 len)
{
    जर (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, किल्ली, len);
    } अन्यथा {
        qemu_cfg_select(किल्ली);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    परत len;
}

पूर्णांक
qemu_cfg_write_file(शून्य *src, संरचना romfile_s *file, u32 offset, u32 len)
{
    जर ((offset + len) > file->size)
        परत -1;

    जर (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        परत -1;
    }
    परत qemu_cfg_write_file_simple(src, qemu_get_romfile_किल्ली(file),
                                      offset, len);
}

स्थिर शून्य
qemu_romfile_add(अक्षर *name, पूर्णांक select, पूर्णांक skip, पूर्णांक size)
{
    संरचना qemu_romfile_s *qfile = malloc_tmp(आकार(*qfile));
    जर (!qfile) {
        warn_noalloc();
        परत;
    }
    स_रखो(qfile, 0, आकार(*qfile));
    strtcpy(qfile->file.name, name, आकार(qfile->file.name));
    qfile->file.size = size;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_किल्ली(संरचना romfile_s *file)
{
    संरचना qemu_romfile_s *qfile;
    जर (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        परत 0;
    }
    qfile = container_of(file, संरचना qemu_romfile_s, file);
    परत qfile->select;
}

स्थिर पूर्णांक rtc_present(शून्य)
{
    परत rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_मोजणी(शून्य)
{
    u16 smp_मोजणी = 0;
    जर (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_मोजणी, QEMU_CFG_NB_CPUS, आकार(smp_मोजणी));
    }
    जर (rtc_present()) {
        u16 cmos_cpu_मोजणी = rtc_read(CMOS_BIOS_SMP_मोजणी) + 1;
        जर (smp_मोजणी < cmos_cpu_मोजणी) {
            smp_मोजणी = cmos_cpu_मोजणी;
        }
    }
    परत smp_मोजणी;
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
    जर (!CONFIG_QEMU)
        परत;

    जर (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        परत;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, आकार(count32));
    जर (count32) {
        संरचना e820_reservation entry;
        पूर्णांक i;
        साठी (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, आकार(entry));
            e820_add(entry.address, entry.length, entry.type);
        }
    } अन्यथा जर (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 वरचा = ((rtc_read(CMOS_MEM_HIGHMEM_खालचा) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_वरचा) << 32));
    RamSizeOver4G = वरचा;
    e820_add(0x100000000ull, वरचा, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", RamSizeOver4G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
स्थिर शून्य
qemu_cfg_legacy(शून्य)
{
    जर (!CONFIG_QEMU)
        परत;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, आकार(numacount));
    पूर्णांक max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, आकार(numacount)
                     , max_cpu*आकार(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , आकार(numacount) + max_cpu*आकार(u64)
                     , numacount*आकार(u64));

    // ACPI tables
    अक्षर name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, आकार(cnt));
    पूर्णांक i, offset = आकार(cnt);
    साठी (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, आकार(len));
        offset += आकार(len);
        snprintf(name, आकार(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, आकार(cnt));
    offset = आकार(cnt);
    साठी (i = 0; i < cnt; i++) {
        संरचना qemu_smbios_header header;
        qemu_cfg_read(&header, आकार(header));
        जर (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, आकार(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + आकार(header)
                             , header.length - आकार(header));
        } अन्यथा {
            snprintf(name, आकार(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.length - 3);
        }
        qemu_cfg_skip(header.length - आकार(header));
        offset += header.length;
    }
}

संरचना QemuCfgFile {
    u32  size;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    अक्षर name[56];
};

स्थिर पूर्णांक qemu_cfg_detect(शून्य)
{
    जर (cfg_enabled)
        परत 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    अक्षर *sig = "QEMU";
    पूर्णांक i;
    साठी (i = 0; i < 4; i++)
        जर (inb(PORT_QEMU_CFG_DATA) != sig[i])
            परत 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, आकार(id));

    जर (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    परत 1;
}

शून्य qemu_cfg_init(शून्य)
{
    जर (!runningOnQEMU())
        परत;

    जर (!qemu_cfg_detect())
        परत;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 मोजणी;
    qemu_cfg_read_entry(&मोजणी, QEMU_CFG_FILE_DIR, आकार(मोजणी));
    मोजणी = be32_to_cpu(मोजणी);
    u32 e;
    साठी (e = 0; e < मोजणी; e++) {
        संरचना QemuCfgFile qfile;
        qemu_cfg_read(&qfile, आकार(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.size));
    }

    qemu_cfg_e820();

    जर (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, आकार(nogfx));
    जर (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
स्थिर पूर्णांक qemu_early_e820(शून्य)
{
    संरचना e820_reservation table;
    संरचना QemuCfgFile qfile;
    u32 select = 0, size = 0;
    u32 मोजणी, i;

    जर (!qemu_cfg_detect())
        परत 0;

    // find e820 table
    qemu_cfg_read_entry(&मोजणी, QEMU_CFG_FILE_DIR, आकार(मोजणी));
    मोजणी = be32_to_cpu(मोजणी);
    साठी (i = 0; i < मोजणी; i++) {
        qemu_cfg_read(&qfile, आकार(qfile));
        जर (स_भेद(qfile.name, "etc/e820", 9) != 0)
            सुरूठेवा;
        select = be16_to_cpu(qfile.select);
        size = be32_to_cpu(qfile.size);
        थांबा;
    }
    जर (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        परत 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    मोजणी = size/आकार(table);
    साठी (i = 0, select = 0; i < मोजणी; i++) {
        qemu_cfg_read(&table, आकार(table));
        निवड (table.type) {
        स्थिती E820_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.length);
            थांबा;
        स्थिती E820_SOFT_RESERVED:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.length);
            थांबा;
        स्थिती E820_RAM:
            e820_add(table.address, table.length, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.length);
            जर (table.address < 0x100000000LL) {
                // below 4g
                जर (RamSize < table.address + table.length)
                    RamSize = table.address + table.length;
            } अन्यथा {
                // above 4g
                जर (RamSizeOver4G < table.address + table.length - 0x100000000LL)
                    RamSizeOver4G = table.address + table.length - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", RamSize);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", RamSizeOver4G);
    परत 1;
}
