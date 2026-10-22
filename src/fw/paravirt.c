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
u32 Ram_आकार;
// Amount of continuous ram >4Gig
u64 Ram_आकार_Over4_G;
// physical address space bits
u8 CPUPhys_द्वयङ्काः;
// 64bit processor
u8 CPULong_बहुलकम्;
// Type of emulator platform.
पूर्णाङ्क PlatformRunningOn VARFSEG;
// cfg enabled
पूर्णाङ्क cfg_enabled = 0;
// cfg_dma enabled
पूर्णाङ्क cfg_dma_enabled = 0;

अन्तःस्थ पूर्णाङ्क qemu_cfg_enabled(शून्य)
{
    प्रत्यागम cfg_enabled;
}

अन्तःस्थ पूर्णाङ्क qemu_cfg_dma_enabled(शून्य)
{
    प्रत्यागम cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#परिभाषय KVM_CPUID_SIGNATURE     0x40000000
स्थिर अचिह्नित पूर्णाङ्क kvm_cpuid_आधारः = 0;

स्थिर शून्य kvm_detect(शून्य)
{
    अचिह्नित पूर्णाङ्क i, max = 0;
    अचिह्नित पूर्णाङ्क eax, ebx, ecx, edx;
    वर्ण signature[13];

    प्रति (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        यदि (eax < i)
            भङ्ग;
        स्मृतिं_प्रतिलिख(signature + 0, &ebx, 4);
        स्मृतिं_प्रतिलिख(signature + 4, &ecx, 4);
        स्मृतिं_प्रतिलिख(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        यदि (वाक्यं_तुलय(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_आधारः = i;
            max = eax;
        }
    }

    यदि (kvm_cpuid_आधारः) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        यदि (max >= kvm_cpuid_आधारः + 0x10) {
            cpuid(kvm_cpuid_आधारः + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#परिभाषय KVM_FEATURE_CLOCKSOURCE           0
#परिभाषय KVM_FEATURE_CLOCKSOURCE2          3

#परिभाषय MSR_KVM_SYSTEM_TIME            0x12
#परिभाषय MSR_KVM_SYSTEM_TIME_NEW  0x4b564d01

#परिभाषय PVCLOCK_TSC_STABLE_BIT     (1 << 0)

संरचना pvclock_vcpu_time_info *kvmclock;

स्थिर शून्य kvmclock_init(शून्य)
{
    अचिह्नित पूर्णाङ्क eax, ebx, ecx, edx, msr;

    यदि (!runningOnKVM())
        प्रत्यागम;

    cpuid(kvm_cpuid_आधारः + 0x01, &eax, &ebx, &ecx, &edx);
    यदि (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_TIME_NEW;
    अन्यथा यदि (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_TIME;
    अन्यथा
        प्रत्यागम;

    kvmclock = memalign_नीचः(परिमाण(*kvmclock), 32);
    स्मृतिं_पूरय(kvmclock, 0, परिमाण(*kvmclock));
    u32 मानम् = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", मानम्, msr);
    wrmsr(msr, मानम् | 0x01);

    यदि (!(kvmclock->flags & PVCLOCK_TSC_STABLE_BIT))
        प्रत्यागम;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_गन्तव्यम्_system_mul >> 16);
    यदि (kvmclock->tsc_सरणम् < 0)
        MHz <<= -kvmclock->tsc_सरणम्;
    अन्यथा
        MHz >>= kvmclock->tsc_सरणम्;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

स्थिर शून्य physbits(पूर्णाङ्क qemu_quirk)
{
    अचिह्नित पूर्णाङ्क max, eax, ebx, ecx, edx;
    अचिह्नित पूर्णाङ्क physbits;
    वर्ण signature[13];
    पूर्णाङ्क pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    स्मृतिं_प्रतिलिख(signature + 0, &ebx, 4);
    स्मृतिं_प्रतिलिख(signature + 4, &edx, 4);
    स्मृतिं_प्रतिलिख(signature + 8, &ecx, 4);
    signature[12] = 0;
    यदि (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    यदि (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULong_बहुलकम् = !!(edx & (1 << 29));
    }

    यदि (pae && CPULong_बहुलकम् && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        यदि (!qemu_quirk) {
            valid = 1;
        } अन्यथा यदि (physbits >= 41) {
            valid = 1;
        } अन्यथा यदि (वाक्यं_तुलय(signature, "GenuineIntel") == 0) {
            यदि ((physbits == 36) || (physbits == 39))
                valid = 1;
        } अन्यथा यदि (वाक्यं_तुलय(signature, "AuthenticAMD") == 0) {
            यदि (physbits == 40)
                valid = 1;
        }
    } अन्यथा {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULong_बहुलकम् ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    यदि (valid)
        CPUPhys_द्वयङ्काः = physbits;
}

स्थिर शून्य qemu_detect(शून्य)
{
    यदि (!CONFIG_QEMU_HARDWARE)
        प्रत्यागम;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    यदि (v == 0x0000 || v == 0xffff)
        प्रत्यागम;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    यदि (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        प्रत्यागम;

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

    यदि (!CONFIG_QEMU)
        प्रत्यागम;

    यदि (runningOnXen()) {
        xen_ramsize_preinit();
        प्रत्यागम;
    }

    // try read e820 table first
    यदि (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_नीचः) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_उच्चः) << 24));
        यदि (rs)
            rs += 16 * 1024 * 1024;
        अन्यथा
            rs = (((rtc_read(CMOS_MEM_EXTMEM_नीचः) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_उच्चः) << 18))
                  + 1 * 1024 * 1024);
        Ram_आकार = rs;
        e820_सङ्कलय(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_आकार);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_सङ्कलय(0xfffc0000, 256*1024, E820_RESERVED);
}

#परिभाषय MSR_IA32_FEATURE_CONTROL 0x0000003a

स्थिर शून्य msr_feature_control_सज्जता(शून्य)
{
    u64 feature_control_द्वयङ्काः = romfile_loadint("etc/msr_feature_control", 0);
    यदि (feature_control_द्वयङ्काः)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_द्वयङ्काः);
}

शून्य
qemu_platform_सज्जता(शून्य)
{
    यदि (!CONFIG_QEMU)
        प्रत्यागम;

    यदि (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_सज्जता();
        xen_biostable_सज्जता();
        प्रत्यागम;
    }

    kvmclock_init();

    // Initialize pci
    pci_सज्जता();
    smm_device_सज्जता();
    smm_सज्जता();

    // Initialize mtrr, msr_feature_control and smp
    mtrr_सज्जता();
    msr_feature_control_सज्जता();
    smp_सज्जता();

    // Create bios tables
    यदि (Max_गणनम्_CPUs <= 255) {
        pirtable_सज्जता();
        mptable_सज्जता();
    }
    smbios_सज्जता();

    यदि (CONFIG_FW_ROMFILE_आरोपय) {
        पूर्णाङ्क loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = find_acpi_rsdp();

        यदि (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_सज्जता_acpi();
            प्रत्यागम;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        यदि (!loader_err)
            warn_internalerror();
    }

    acpi_सज्जता();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#परिभाषय QEMU_CFG_SIGNATURE              0x00
#परिभाषय QEMU_CFG_ID                     0x01
#परिभाषय QEMU_CFG_UUID                   0x02
#परिभाषय QEMU_CFG_NOGRAPHIC              0x04
#परिभाषय QEMU_CFG_NUMA                   0x0d
#परिभाषय QEMU_CFG_BOOT_MENU              0x0e
#परिभाषय QEMU_CFG_NB_CPUS                0x05
#परिभाषय QEMU_CFG_MAX_CPUS               0x0f
#परिभाषय QEMU_CFG_FILE_DIR               0x19
#परिभाषय QEMU_CFG_ARCH_LOCAL             0x8000
#परिभाषय QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#परिभाषय QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#परिभाषय QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#परिभाषय QEMU_CFG_E820_सारणी             (QEMU_CFG_ARCH_LOCAL + 3)

स्थिर शून्य
qemu_cfg_select(u16 f)
{
    outw(f, पत्तनम्_QEMU_CFG_CTL);
}

स्थिर शून्य
qemu_cfg_dma_transfer(शून्य *स्थानाङ्कः, u32 दीर्घता, u32 control)
{
    QemuCfgDmaAccess access;

    access.स्थानाङ्कः = cpu_गन्तव्यम्_be64((u64)(u32)स्थानाङ्कः);
    access.दीर्घता = cpu_गन्तव्यम्_be32(दीर्घता);
    access.control = cpu_गन्तव्यम्_be32(control);

    barrier();

    outl(cpu_गन्तव्यम्_be32((u32)&access), पत्तनम्_QEMU_CFG_DMA_ADDR_नीचः);

    यावत्(be32_गन्तव्यम्_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

स्थिर शून्य
qemu_cfg_read(शून्य *buf, पूर्णाङ्क len)
{
    यदि (len == 0) {
        प्रत्यागम;
    }

    यदि (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } अन्यथा {
        insb(पत्तनम्_QEMU_CFG_दत्त, buf, len);
    }
}

स्थिर शून्य
qemu_cfg_write(शून्य *buf, पूर्णाङ्क len)
{
    यदि (len == 0) {
        प्रत्यागम;
    }

    यदि (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } अन्यथा {
        warn_internalerror();
    }
}

स्थिर शून्य
qemu_cfg_skip(पूर्णाङ्क len)
{
    यदि (len == 0) {
        प्रत्यागम;
    }

    यदि (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } अन्यथा {
        यावत् (len--)
            inb(पत्तनम्_QEMU_CFG_दत्त);
    }
}

स्थिर शून्य
qemu_cfg_read_प्रविष्टिः(शून्य *buf, पूर्णाङ्क e, पूर्णाङ्क len)
{
    यदि (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } अन्यथा {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

स्थिर शून्य
qemu_cfg_write_प्रविष्टिः(शून्य *buf, पूर्णाङ्क e, पूर्णाङ्क len)
{
    यदि (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } अन्यथा {
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
    यदि (file->आकार > maxlen)
        प्रत्यागम -1;
    संरचना qemu_romfile_s *qfile;
    qfile = container_सम्बन्धि(file, संरचना qemu_romfile_s, file);
    यदि (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_प्रविष्टिः(dst, qfile->select, file->आकार);
    } अन्यथा {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->आकार);
    }
    प्रत्यागम file->आकार;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
पूर्णाङ्क
qemu_cfg_write_file_simple(शून्य *src, u16 कुञ्चिका, u32 विस्थापनम्_2, u32 len)
{
    यदि (विस्थापनम्_2 == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_प्रविष्टिः(src, कुञ्चिका, len);
    } अन्यथा {
        qemu_cfg_select(कुञ्चिका);
        qemu_cfg_skip(विस्थापनम्_2);
        qemu_cfg_write(src, len);
    }
    प्रत्यागम len;
}

पूर्णाङ्क
qemu_cfg_write_file(शून्य *src, संरचना romfile_s *file, u32 विस्थापनम्_2, u32 len)
{
    यदि ((विस्थापनम्_2 + len) > file->आकार)
        प्रत्यागम -1;

    यदि (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        प्रत्यागम -1;
    }
    प्रत्यागम qemu_cfg_write_file_simple(src, qemu_get_romfile_कुञ्चिका(file),
                                      विस्थापनम्_2, len);
}

स्थिर शून्य
qemu_romfile_सङ्कलय(वर्ण *name, पूर्णाङ्क select, पूर्णाङ्क skip, पूर्णाङ्क आकार)
{
    संरचना qemu_romfile_s *qfile = malloc_tmp(परिमाण(*qfile));
    यदि (!qfile) {
        warn_noalloc();
        प्रत्यागम;
    }
    स्मृतिं_पूरय(qfile, 0, परिमाण(*qfile));
    strtcpy(qfile->file.name, name, परिमाण(qfile->file.name));
    qfile->file.आकार = आकार;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_सङ्कलय(&qfile->file);
}

u16
qemu_get_romfile_कुञ्चिका(संरचना romfile_s *file)
{
    संरचना qemu_romfile_s *qfile;
    यदि (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        प्रत्यागम 0;
    }
    qfile = container_सम्बन्धि(file, संरचना qemu_romfile_s, file);
    प्रत्यागम qfile->select;
}

स्थिर पूर्णाङ्क rtc_present(शून्य)
{
    प्रत्यागम rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_गणनम्(शून्य)
{
    u16 smp_गणनम् = 0;
    यदि (qemu_cfg_enabled()) {
        qemu_cfg_read_प्रविष्टिः(&smp_गणनम्, QEMU_CFG_NB_CPUS, परिमाण(smp_गणनम्));
    }
    यदि (rtc_present()) {
        u16 cmos_cpu_गणनम् = rtc_read(CMOS_BIOS_SMP_गणनम्) + 1;
        यदि (smp_गणनम् < cmos_cpu_गणनम्) {
            smp_गणनम् = cmos_cpu_गणनम्;
        }
    }
    प्रत्यागम smp_गणनम्;
}

संरचना e820_reservation {
    u64 स्थानाङ्कः;
    u64 दीर्घता;
    u32 type;
};

#परिभाषय SMBIOS_क्षेत्रम्_प्रविष्टिः 0
#परिभाषय SMBIOS_सारणी_प्रविष्टिः 1

संरचना qemu_smbios_शीर्षकम् {
    u16 दीर्घता;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

स्थिर शून्य
qemu_cfg_e820(शून्य)
{
    यदि (!CONFIG_QEMU)
        प्रत्यागम;

    यदि (romfile_find("etc/e820")) {
        // qemu_early_e820() has handled everything
        प्रत्यागम;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_प्रविष्टिः(&count32, QEMU_CFG_E820_सारणी, परिमाण(count32));
    यदि (count32) {
        संरचना e820_reservation प्रविष्टिः;
        पूर्णाङ्क i;
        प्रति (i = 0; i < count32; i++) {
            qemu_cfg_read(&प्रविष्टिः, परिमाण(प्रविष्टिः));
            e820_सङ्कलय(प्रविष्टिः.स्थानाङ्कः, प्रविष्टिः.दीर्घता, प्रविष्टिः.type);
        }
    } अन्यथा यदि (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_सङ्कलय(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 उच्चः = ((rtc_read(CMOS_MEM_HIGHMEM_नीचः) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_उच्चः) << 32));
    Ram_आकार_Over4_G = उच्चः;
    e820_सङ्कलय(0x100000000ull, उच्चः, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_आकार_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
स्थिर शून्य
qemu_cfg_legacy(शून्य)
{
    यदि (!CONFIG_QEMU)
        प्रत्यागम;

    // Misc config items.
    qemu_romfile_सङ्कलय("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_सङ्कलय("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_सङ्कलय("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_प्रविष्टिः(&numacount, QEMU_CFG_NUMA, परिमाण(numacount));
    पूर्णाङ्क max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_सङ्कलय("etc/numa-cpu-map", QEMU_CFG_NUMA, परिमाण(numacount)
                     , max_cpu*परिमाण(u64));
    qemu_romfile_सङ्कलय("etc/numa-nodes", QEMU_CFG_NUMA
                     , परिमाण(numacount) + max_cpu*परिमाण(u64)
                     , numacount*परिमाण(u64));

    // ACPI tables
    वर्ण name[128];
    u16 cnt;
    qemu_cfg_read_प्रविष्टिः(&cnt, QEMU_CFG_ACPI_TABLES, परिमाण(cnt));
    पूर्णाङ्क i, विस्थापनम्_2 = परिमाण(cnt);
    प्रति (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, परिमाण(len));
        विस्थापनम्_2 += परिमाण(len);
        snprintf(name, परिमाण(name), "acpi/table%d", i);
        qemu_romfile_सङ्कलय(name, QEMU_CFG_ACPI_TABLES, विस्थापनम्_2, len);
        qemu_cfg_skip(len);
        विस्थापनम्_2 += len;
    }

    // SMBIOS info
    qemu_cfg_read_प्रविष्टिः(&cnt, QEMU_CFG_SMBIOS_ENTRIES, परिमाण(cnt));
    विस्थापनम्_2 = परिमाण(cnt);
    प्रति (i = 0; i < cnt; i++) {
        संरचना qemu_smbios_शीर्षकम् शीर्षकम्;
        qemu_cfg_read(&शीर्षकम्, परिमाण(शीर्षकम्));
        यदि (शीर्षकम्.headertype == SMBIOS_क्षेत्रम्_प्रविष्टिः) {
            snprintf(name, परिमाण(name), "smbios/field%d-%d"
                     , शीर्षकम्.tabletype, शीर्षकम्.fieldoffset);
            qemu_romfile_सङ्कलय(name, QEMU_CFG_SMBIOS_ENTRIES
                             , विस्थापनम्_2 + परिमाण(शीर्षकम्)
                             , शीर्षकम्.दीर्घता - परिमाण(शीर्षकम्));
        } अन्यथा {
            snprintf(name, परिमाण(name), "smbios/table%d-%d"
                     , शीर्षकम्.tabletype, i);
            qemu_romfile_सङ्कलय(name, QEMU_CFG_SMBIOS_ENTRIES
                             , विस्थापनम्_2 + 3, शीर्षकम्.दीर्घता - 3);
        }
        qemu_cfg_skip(शीर्षकम्.दीर्घता - परिमाण(शीर्षकम्));
        विस्थापनम्_2 += शीर्षकम्.दीर्घता;
    }
}

संरचना QemuCfgFile {
    u32  आकार;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    वर्ण name[56];
};

स्थिर पूर्णाङ्क qemu_cfg_detect(शून्य)
{
    यदि (cfg_enabled)
        प्रत्यागम 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    वर्ण *sig = "QEMU";
    पूर्णाङ्क i;
    प्रति (i = 0; i < 4; i++)
        यदि (inb(पत्तनम्_QEMU_CFG_दत्त) != sig[i])
            प्रत्यागम 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_प्रविष्टिः(&id, QEMU_CFG_ID, परिमाण(id));

    यदि (id & QEMU_CFG_संस्करणम्_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    प्रत्यागम 1;
}

शून्य qemu_cfg_init(शून्य)
{
    यदि (!runningOnQEMU())
        प्रत्यागम;

    यदि (!qemu_cfg_detect())
        प्रत्यागम;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 गणनम्;
    qemu_cfg_read_प्रविष्टिः(&गणनम्, QEMU_CFG_FILE_DIR, परिमाण(गणनम्));
    गणनम् = be32_गन्तव्यम्_cpu(गणनम्);
    u32 e;
    प्रति (e = 0; e < गणनम्; e++) {
        संरचना QemuCfgFile qfile;
        qemu_cfg_read(&qfile, परिमाण(qfile));
        qemu_romfile_सङ्कलय(qfile.name, be16_गन्तव्यम्_cpu(qfile.select)
                         , 0, be32_गन्तव्यम्_cpu(qfile.आकार));
    }

    qemu_cfg_e820();

    यदि (romfile_find("etc/table-loader")) {
        acpi_pm_आधारः = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_आधारः);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_प्रविष्टिः(&nogfx, QEMU_CFG_NOGRAPHIC, परिमाण(nogfx));
    यदि (nogfx && !romfile_find("etc/sercon-port")
        && !romfile_find("vgaroms/sgabios.bin"))
        const_romfile_सङ्कलय_int("etc/sercon-port", पत्तनम्_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
स्थिर पूर्णाङ्क qemu_early_e820(शून्य)
{
    संरचना e820_reservation सारणी;
    संरचना QemuCfgFile qfile;
    u32 select = 0, आकार = 0;
    u32 गणनम्, i;

    यदि (!qemu_cfg_detect())
        प्रत्यागम 0;

    // find e820 table
    qemu_cfg_read_प्रविष्टिः(&गणनम्, QEMU_CFG_FILE_DIR, परिमाण(गणनम्));
    गणनम् = be32_गन्तव्यम्_cpu(गणनम्);
    प्रति (i = 0; i < गणनम्; i++) {
        qemu_cfg_read(&qfile, परिमाण(qfile));
        यदि (memcmp(qfile.name, "etc/e820", 9) != 0)
            अनुवर्त;
        select = be16_गन्तव्यम्_cpu(qfile.select);
        आकार = be32_गन्तव्यम्_cpu(qfile.आकार);
        भङ्ग;
    }
    यदि (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        प्रत्यागम 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    गणनम् = आकार/परिमाण(सारणी);
    प्रति (i = 0, select = 0; i < गणनम्; i++) {
        qemu_cfg_read(&सारणी, परिमाण(सारणी));
        विकल्प (सारणी.type) {
        पक्ष E820_RESERVED:
            e820_सङ्कलय(सारणी.स्थानाङ्कः, सारणी.दीर्घता, सारणी.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    सारणी.स्थानाङ्कः, सारणी.दीर्घता);
            भङ्ग;
        पक्ष E820_SOFT_RESERVED:
            e820_सङ्कलय(सारणी.स्थानाङ्कः, सारणी.दीर्घता, सारणी.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    सारणी.स्थानाङ्कः, सारणी.दीर्घता);
            भङ्ग;
        पक्ष E820_RAM:
            e820_सङ्कलय(सारणी.स्थानाङ्कः, सारणी.दीर्घता, सारणी.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    सारणी.स्थानाङ्कः, सारणी.दीर्घता);
            यदि (सारणी.स्थानाङ्कः < 0x100000000LL) {
                // below 4g
                यदि (Ram_आकार < सारणी.स्थानाङ्कः + सारणी.दीर्घता)
                    Ram_आकार = सारणी.स्थानाङ्कः + सारणी.दीर्घता;
            } अन्यथा {
                // above 4g
                यदि (Ram_आकार_Over4_G < सारणी.स्थानाङ्कः + सारणी.दीर्घता - 0x100000000LL)
                    Ram_आकार_Over4_G = सारणी.स्थानाङ्कः + सारणी.दीर्घता - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_आकार);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_आकार_Over4_G);
    प्रत्यागम 1;
}
