// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "byteorder.h" // be32_to_cpu
#समावेश "config.h" // CONFIG_QEMU
#समावेश "e820map.h" // e820_add
#समावेश "hw/pci.h" // pci_config_readw
#समावेश "hw/pcidevice.h" // pci_probe_devices
#समावेश "hw/pci_regs.h" // PCI_DEVICE_ID
#समावेश "hw/serialio.h" // PORT_SERIAL1
#समावेश "hw/rtc.h" // CMOS_*
#समावेश "hw/virtio-mmio.h" // virtio_mmio_acpi
#समावेश "malloc.h" // malloc_tmp
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // qemu_cfg_preinit
#समावेश "romfile.h" // romfile_loadint
#समावेश "romfile_loader.h" // romfile_loader_execute
#समावेश "string.h" // memset
#समावेश "util.h" // pci_setup
#समावेश "x86.h" // cpuid
#समावेश "xen.h" // xen_biostable_setup
#समावेश "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 Ram_आकार;
// Amount of continuous ram >4Gig
u64 Ram_आकार_Over4_G;
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

अंतरभूत पूर्णांक qemu_cfg_enabled(व्योम)
{
    वापस cfg_enabled;
}

अंतरभूत पूर्णांक qemu_cfg_dma_enabled(व्योम)
{
    वापस cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#घोषणा KVM_CPUID_SIGNATURE     0x40000000
अटल अचिन्हित पूर्णांक kvm_cpuid_base = 0;

अटल व्योम kvm_detect(व्योम)
{
    अचिन्हित पूर्णांक i, max = 0;
    अचिन्हित पूर्णांक eax, ebx, ecx, edx;
    अक्षर signature[13];

    क्रम (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        अगर (eax < i)
            अवरोध;
        स_नकल(signature + 0, &ebx, 4);
        स_नकल(signature + 4, &ecx, 4);
        स_नकल(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        अगर (म_भेद(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_base = i;
            max = eax;
        }
    }

    अगर (kvm_cpuid_base) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        अगर (max >= kvm_cpuid_base + 0x10) {
            cpuid(kvm_cpuid_base + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#घोषणा KVM_FEATURE_CLOCKSOURCE           0
#घोषणा KVM_FEATURE_CLOCKSOURCE2          3

#घोषणा MSR_KVM_SYSTEM_TIME            0x12
#घोषणा MSR_KVM_SYSTEM_TIME_NEW  0x4b564d01

#घोषणा PVCLOCK_TSC_STABLE_BIT     (1 << 0)

काष्ठा pvclock_vcpu_time_info *kvmclock;

अटल व्योम kvmclock_init(व्योम)
{
    अचिन्हित पूर्णांक eax, ebx, ecx, edx, msr;

    अगर (!runningOnKVM())
        वापस;

    cpuid(kvm_cpuid_base + 0x01, &eax, &ebx, &ecx, &edx);
    अगर (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    अन्यथा अगर (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    अन्यथा
        वापस;

    kvmclock = memalign_निचला(माप(*kvmclock), 32);
    स_रखो(kvmclock, 0, माप(*kvmclock));
    u32 मूल्य = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", मूल्य, msr);
    wrmsr(msr, मूल्य | 0x01);

    अगर (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        वापस;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_to_system_mul >> 16);
    अगर (kvmclock->tsc_shift < 0)
        MHz <<= -kvmclock->tsc_shift;
    अन्यथा
        MHz >>= kvmclock->tsc_shift;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

अटल व्योम physbits(पूर्णांक qemu_quirk)
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
    अगर (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    अगर (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULongMode = !!(edx & (1 << 29));
    }

    अगर (pae && CPULongMode && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        अगर (!qemu_quirk) {
            valid = 1;
        } अन्यथा अगर (physbits >= 41) {
            valid = 1;
        } अन्यथा अगर (म_भेद(signature, "GenuineIntel") == 0) {
            अगर ((physbits == 36) || (physbits == 39))
                valid = 1;
        } अन्यथा अगर (म_भेद(signature, "AuthenticAMD") == 0) {
            अगर (physbits == 40)
                valid = 1;
        }
    } अन्यथा {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULongMode ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    अगर (valid)
        CPUPhysBits = physbits;
}

अटल व्योम qemu_detect(व्योम)
{
    अगर (!CONFIG_QEMU_HARDWARE)
        वापस;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    अगर (v == 0x0000 || v == 0xffff)
        वापस;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    अगर (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        वापस;

    PlatformRunningOn |= PF_QEMU;
    चयन (d) {
    हाल 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        अवरोध;
    हाल 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        अवरोध;
    शेष:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        अवरोध;
    }
    physbits(1);
}

अटल पूर्णांक qemu_early_e820(व्योम);

व्योम
qemu_preinit(व्योम)
{
    qemu_detect();
    kvm_detect();

    अगर (!CONFIG_QEMU)
        वापस;

    अगर (runningOnXen()) {
        xen_ramsize_preinit();
        वापस;
    }

    // try read e820 table first
    अगर (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_निचला) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_ऊपरी) << 24));
        अगर (rs)
            rs += 16 * 1024 * 1024;
        अन्यथा
            rs = (((rtc_read(CMOS_MEM_EXTMEM_निचला) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_ऊपरी) << 18))
                  + 1 * 1024 * 1024);
        Ram_आकार = rs;
        e820_add(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_आकार);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_add(0xfffc0000, 256*1024, E820_RESERVED);
}

#घोषणा MSR_IA32_FEATURE_CONTROL 0x0000003a

अटल व्योम msr_feature_control_setup(व्योम)
{
    u64 feature_control_bits = romfile_loadint("etc/msr_feature_control", 0);
    अगर (feature_control_bits)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_bits);
}

व्योम
qemu_platform_setup(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    अगर (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_setup();
        xen_biostable_setup();
        वापस;
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
    अगर (Max_गिनती_CPUs <= 255) {
        pirtable_setup();
        mptable_setup();
    }
    smbios_setup();

    अगर (CONFIG_FW_ROMFILE_LOAD) {
        पूर्णांक loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        अगर (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_setup_acpi();
            वापस;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        अगर (!loader_err)
            warn_internalerror();
    }

    acpi_setup();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#घोषणा QEMU_CFG_SIGNATURE              0x00
#घोषणा QEMU_CFG_ID                     0x01
#घोषणा QEMU_CFG_UUID                   0x02
#घोषणा QEMU_CFG_NOGRAPHIC              0x04
#घोषणा QEMU_CFG_NUMA                   0x0d
#घोषणा QEMU_CFG_BOOT_MENU              0x0e
#घोषणा QEMU_CFG_NB_CPUS                0x05
#घोषणा QEMU_CFG_MAX_CPUS               0x0f
#घोषणा QEMU_CFG_FILE_DIR               0x19
#घोषणा QEMU_CFG_ARCH_LOCAL             0x8000
#घोषणा QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#घोषणा QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#घोषणा QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#घोषणा QEMU_CFG_E820_TABLE             (QEMU_CFG_ARCH_LOCAL + 3)

अटल व्योम
qemu_cfg_select(u16 f)
{
    outw(f, PORT_QEMU_CFG_CTL);
}

अटल व्योम
qemu_cfg_dma_transfer(व्योम *address, u32 लंबाई, u32 control)
{
    QemuCfgDmaAccess access;

    access.address = cpu_to_be64((u64)(u32)address);
    access.लंबाई = cpu_to_be32(लंबाई);
    access.control = cpu_to_be32(control);

    barrier();

    outl(cpu_to_be32((u32)&access), PORT_QEMU_CFG_DMA_ADDR_निचला);

    जबतक(be32_to_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

अटल व्योम
qemu_cfg_read(व्योम *buf, पूर्णांक len)
{
    अगर (len == 0) {
        वापस;
    }

    अगर (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } अन्यथा {
        insb(PORT_QEMU_CFG_तथ्य, buf, len);
    }
}

अटल व्योम
qemu_cfg_write(व्योम *buf, पूर्णांक len)
{
    अगर (len == 0) {
        वापस;
    }

    अगर (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } अन्यथा {
        warn_internalerror();
    }
}

अटल व्योम
qemu_cfg_skip(पूर्णांक len)
{
    अगर (len == 0) {
        वापस;
    }

    अगर (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } अन्यथा {
        जबतक (len--)
            inb(PORT_QEMU_CFG_तथ्य);
    }
}

अटल व्योम
qemu_cfg_read_entry(व्योम *buf, पूर्णांक e, पूर्णांक len)
{
    अगर (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } अन्यथा {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

अटल व्योम
qemu_cfg_write_entry(व्योम *buf, पूर्णांक e, पूर्णांक len)
{
    अगर (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } अन्यथा {
        warn_internalerror();
    }
}

काष्ठा qemu_romfile_s {
    काष्ठा romfile_s file;
    पूर्णांक select, skip;
};

अटल पूर्णांक
qemu_cfg_read_file(काष्ठा romfile_s *file, व्योम *dst, u32 maxlen)
{
    अगर (file->आकार > maxlen)
        वापस -1;
    काष्ठा qemu_romfile_s *qfile;
    qfile = container_of(file, काष्ठा qemu_romfile_s, file);
    अगर (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_entry(dst, qfile->select, file->आकार);
    } अन्यथा {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->आकार);
    }
    वापस file->आकार;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
पूर्णांक
qemu_cfg_write_file_simple(व्योम *src, u16 कुंजी, u32 offset, u32 len)
{
    अगर (offset == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_entry(src, कुंजी, len);
    } अन्यथा {
        qemu_cfg_select(कुंजी);
        qemu_cfg_skip(offset);
        qemu_cfg_write(src, len);
    }
    वापस len;
}

पूर्णांक
qemu_cfg_write_file(व्योम *src, काष्ठा romfile_s *file, u32 offset, u32 len)
{
    अगर ((offset + len) > file->आकार)
        वापस -1;

    अगर (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        वापस -1;
    }
    वापस qemu_cfg_write_file_simple(src, qemu_get_romfile_कुंजी(file),
                                      offset, len);
}

अटल व्योम
qemu_romfile_add(अक्षर *name, पूर्णांक select, पूर्णांक skip, पूर्णांक आकार)
{
    काष्ठा qemu_romfile_s *qfile = malloc_tmp(माप(*qfile));
    अगर (!qfile) {
        warn_noalloc();
        वापस;
    }
    स_रखो(qfile, 0, माप(*qfile));
    strtcpy(qfile->file.name, name, माप(qfile->file.name));
    qfile->file.आकार = आकार;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_add(&qfile->file);
}

u16
qemu_get_romfile_कुंजी(काष्ठा romfile_s *file)
{
    काष्ठा qemu_romfile_s *qfile;
    अगर (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        वापस 0;
    }
    qfile = container_of(file, काष्ठा qemu_romfile_s, file);
    वापस qfile->select;
}

अटल पूर्णांक rtc_present(व्योम)
{
    वापस rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_गिनती(व्योम)
{
    u16 smp_गिनती = 0;
    अगर (qemu_cfg_enabled()) {
        qemu_cfg_read_entry(&smp_गिनती, QEMU_CFG_NB_CPUS, माप(smp_गिनती));
    }
    अगर (rtc_present()) {
        u16 cmos_cpu_गिनती = rtc_read(CMOS_BIOS_SMP_गिनती) + 1;
        अगर (smp_गिनती < cmos_cpu_गिनती) {
            smp_गिनती = cmos_cpu_गिनती;
        }
    }
    वापस smp_गिनती;
}

काष्ठा e820_reservation {
    u64 address;
    u64 लंबाई;
    u32 type;
};

#घोषणा SMBIOS_FIELD_ENTRY 0
#घोषणा SMBIOS_TABLE_ENTRY 1

काष्ठा qemu_smbios_header {
    u16 लंबाई;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

अटल व्योम
qemu_cfg_e820(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    अगर (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        वापस;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_entry(&count32, QEMU_CFG_E820_TABLE, माप(count32));
    अगर (count32) {
        काष्ठा e820_reservation entry;
        पूर्णांक i;
        क्रम (i = 0; i < count32; i++) {
            qemu_cfg_read(&entry, माप(entry));
            e820_add(entry.address, entry.लंबाई, entry.type);
        }
    } अन्यथा अगर (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_add(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 ऊपरी = ((rtc_read(CMOS_MEM_HIGHMEM_निचला) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_ऊपरी) << 32));
    Ram_आकार_Over4_G = ऊपरी;
    e820_add(0x100000000ull, ऊपरी, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_आकार_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
अटल व्योम
qemu_cfg_legacy(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    // Misc config items.
    qemu_romfile_add("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_add("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_add("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_entry(&numacount, QEMU_CFG_NUMA, माप(numacount));
    पूर्णांक max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_add("etc/numa-cpu-map", QEMU_CFG_NUMA, माप(numacount)
                     , max_cpu*माप(u64));
    qemu_romfile_add("etc/numa-nodes", QEMU_CFG_NUMA
                     , माप(numacount) + max_cpu*माप(u64)
                     , numacount*माप(u64));

    // ACPI tables
    अक्षर name[128];
    u16 cnt;
    qemu_cfg_read_entry(&cnt, QEMU_CFG_ACPI_TABLES, माप(cnt));
    पूर्णांक i, offset = माप(cnt);
    क्रम (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, माप(len));
        offset += माप(len);
        snprintf(name, माप(name), "acpi/table%d", i);
        qemu_romfile_add(name, QEMU_CFG_ACPI_TABLES, offset, len);
        qemu_cfg_skip(len);
        offset += len;
    }

    // SMBIOS info
    qemu_cfg_read_entry(&cnt, QEMU_CFG_SMBIOS_ENTRIES, माप(cnt));
    offset = माप(cnt);
    क्रम (i = 0; i < cnt; i++) {
        काष्ठा qemu_smbios_header header;
        qemu_cfg_read(&header, माप(header));
        अगर (header.headertype == SMBIOS_FIELD_ENTRY) {
            snprintf(name, माप(name), "smbios/field%d-%d"
                     , header.tabletype, header.fieldoffset);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + माप(header)
                             , header.लंबाई - माप(header));
        } अन्यथा {
            snprintf(name, माप(name), "smbios/table%d-%d"
                     , header.tabletype, i);
            qemu_romfile_add(name, QEMU_CFG_SMBIOS_ENTRIES
                             , offset + 3, header.लंबाई - 3);
        }
        qemu_cfg_skip(header.लंबाई - माप(header));
        offset += header.लंबाई;
    }
}

काष्ठा QemuCfgFile {
    u32  आकार;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    अक्षर name[56];
};

अटल पूर्णांक qemu_cfg_detect(व्योम)
{
    अगर (cfg_enabled)
        वापस 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    अक्षर *sig = "QEMU";
    पूर्णांक i;
    क्रम (i = 0; i < 4; i++)
        अगर (inb(PORT_QEMU_CFG_तथ्य) != sig[i])
            वापस 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_entry(&id, QEMU_CFG_ID, माप(id));

    अगर (id & QEMU_CFG_VERSION_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    वापस 1;
}

व्योम qemu_cfg_init(व्योम)
{
    अगर (!runningOnQEMU())
        वापस;

    अगर (!qemu_cfg_detect())
        वापस;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 गिनती;
    qemu_cfg_read_entry(&गिनती, QEMU_CFG_FILE_DIR, माप(गिनती));
    गिनती = be32_to_cpu(गिनती);
    u32 e;
    क्रम (e = 0; e < गिनती; e++) {
        काष्ठा QemuCfgFile qfile;
        qemu_cfg_read(&qfile, माप(qfile));
        qemu_romfile_add(qfile.name, be16_to_cpu(qfile.select)
                         , 0, be32_to_cpu(qfile.आकार));
    }

    qemu_cfg_e820();

    अगर (romfile_find("etc/table-loader")) {
        acpi_pm_base = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_base);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_entry(&nogfx, QEMU_CFG_NOGRAPHIC, माप(nogfx));
    अगर (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_add_int("etc/sercon-port", PORT_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
अटल पूर्णांक qemu_early_e820(व्योम)
{
    काष्ठा e820_reservation table;
    काष्ठा QemuCfgFile qfile;
    u32 select = 0, आकार = 0;
    u32 गिनती, i;

    अगर (!qemu_cfg_detect())
        वापस 0;

    // find e820 table
    qemu_cfg_read_entry(&गिनती, QEMU_CFG_FILE_DIR, माप(गिनती));
    गिनती = be32_to_cpu(गिनती);
    क्रम (i = 0; i < गिनती; i++) {
        qemu_cfg_read(&qfile, माप(qfile));
        अगर (स_भेद(qfile.name, "etc/e820", 9) != 0)
            जारी;
        select = be16_to_cpu(qfile.select);
        आकार = be32_to_cpu(qfile.आकार);
        अवरोध;
    }
    अगर (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        वापस 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    गिनती = आकार/माप(table);
    क्रम (i = 0, select = 0; i < गिनती; i++) {
        qemu_cfg_read(&table, माप(table));
        चयन (table.type) {
        हाल E820_RESERVED:
            e820_add(table.address, table.लंबाई, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    table.address, table.लंबाई);
            अवरोध;
        हाल E820_SOFT_RESERVED:
            e820_add(table.address, table.लंबाई, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    table.address, table.लंबाई);
            अवरोध;
        हाल E820_RAM:
            e820_add(table.address, table.लंबाई, table.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    table.address, table.लंबाई);
            अगर (table.address < 0x100000000LL) {
                // below 4g
                अगर (Ram_आकार < table.address + table.लंबाई)
                    Ram_आकार = table.address + table.लंबाई;
            } अन्यथा {
                // above 4g
                अगर (Ram_आकार_Over4_G < table.address + table.लंबाई - 0x100000000LL)
                    Ram_आकार_Over4_G = table.address + table.लंबाई - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_आकार);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_आकार_Over4_G);
    वापस 1;
}
