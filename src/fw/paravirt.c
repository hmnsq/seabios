// Paravirtualization support.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2009 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "byteorder.h" // be32_to_cpu
#అన్తర్భావ "config.h" // CONFIG_QEMU
#అన్తర్భావ "e820map.h" // e820_add
#అన్తర్భావ "hw/pci.h" // pci_config_readw
#అన్తర్భావ "hw/pcidevice.h" // pci_probe_devices
#అన్తర్భావ "hw/pci_regs.h" // PCI_DEVICE_ID
#అన్తర్భావ "hw/serialio.h" // PORT_SERIAL1
#అన్తర్భావ "hw/rtc.h" // CMOS_*
#అన్తర్భావ "hw/virtio-mmio.h" // virtio_mmio_acpi
#అన్తర్భావ "malloc.h" // malloc_tmp
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // qemu_cfg_preinit
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "romfile_loader.h" // romfile_loader_execute
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // pci_setup
#అన్తర్భావ "x86.h" // cpuid
#అన్తర్భావ "xen.h" // xen_biostable_setup
#అన్తర్భావ "stacks.h" // yield

// Amount of continuous ram under 4Gig
u32 Ram_కొలత;
// Amount of continuous ram >4Gig
u64 Ram_కొలత_Over4_G;
// physical address space bits
u8 CPUPhys_బిట్లు;
// 64bit processor
u8 CPULong_బహుళకం;
// Type of emulator platform.
పూర్ణాంకం PlatformRunningOn VARFSEG;
// cfg enabled
పూర్ణాంకం cfg_enabled = 0;
// cfg_dma enabled
పూర్ణాంకం cfg_dma_enabled = 0;

అంతఃవరుస పూర్ణాంకం qemu_cfg_enabled(శూన్య)
{
    తిరిగి cfg_enabled;
}

అంతఃవరుస పూర్ణాంకం qemu_cfg_dma_enabled(శూన్య)
{
    తిరిగి cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#నిర్వచించు KVM_CPUID_SIGNATURE     0x40000000
స్థిర గుర్తులేని పూర్ణాంకం kvm_cpuid_పునాది = 0;

స్థిర శూన్య kvm_detect(శూన్య)
{
    గుర్తులేని పూర్ణాంకం i, max = 0;
    గుర్తులేని పూర్ణాంకం eax, ebx, ecx, edx;
    అక్షరం signature[13];

    కొరకు (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        ఒకవేళ (eax < i)
            ఆపు;
        జ్ఞాపకం_నకలు(signature + 0, &ebx, 4);
        జ్ఞాపకం_నకలు(signature + 4, &ecx, 4);
        జ్ఞాపకం_నకలు(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        ఒకవేళ (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_పునాది = i;
            max = eax;
        }
    }

    ఒకవేళ (kvm_cpuid_పునాది) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        ఒకవేళ (max >= kvm_cpuid_పునాది + 0x10) {
            cpuid(kvm_cpuid_పునాది + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#నిర్వచించు KVM_FEATURE_CLOCKSOURCE           0
#నిర్వచించు KVM_FEATURE_CLOCKSOURCE2          3

#నిర్వచించు MSR_KVM_SYSTEM_సమయం            0x12
#నిర్వచించు MSR_KVM_SYSTEM_సమయం_NEW  0x4b564d01

#నిర్వచించు PVCLOCK_TSC_STABLE_బిట్టు     (1 << 0)

నిర్మాణం pvclock_vcpu_సమయం_info *kvmclock;

స్థిర శూన్య kvmclock_init(శూన్య)
{
    గుర్తులేని పూర్ణాంకం eax, ebx, ecx, edx, msr;

    ఒకవేళ (!runningOnKVM())
        తిరిగి;

    cpuid(kvm_cpuid_పునాది + 0x01, &eax, &ebx, &ecx, &edx);
    ఒకవేళ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_సమయం_NEW;
    లేకపోతే ఒకవేళ (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_సమయం;
    లేకపోతే
        తిరిగి;

    kvmclock = memalign_దిగువ(పరిమాణం(*kvmclock), 32);
    memset(kvmclock, 0, పరిమాణం(*kvmclock));
    u32 విలువ = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", విలువ, msr);
    wrmsr(msr, విలువ | 0x01);

    ఒకవేళ (!(kvmclock->flags & PVCLOCK_TSC_STABLE_బిట్టు))
        తిరిగి;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_గమ్యం_system_mul >> 16);
    ఒకవేళ (kvmclock->tsc_జరుపు < 0)
        MHz <<= -kvmclock->tsc_జరుపు;
    లేకపోతే
        MHz >>= kvmclock->tsc_జరుపు;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

స్థిర శూన్య physbits(పూర్ణాంకం qemu_quirk)
{
    గుర్తులేని పూర్ణాంకం max, eax, ebx, ecx, edx;
    గుర్తులేని పూర్ణాంకం physbits;
    అక్షరం signature[13];
    పూర్ణాంకం pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    జ్ఞాపకం_నకలు(signature + 0, &ebx, 4);
    జ్ఞాపకం_నకలు(signature + 4, &edx, 4);
    జ్ఞాపకం_నకలు(signature + 8, &ecx, 4);
    signature[12] = 0;
    ఒకవేళ (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    ఒకవేళ (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULong_బహుళకం = !!(edx & (1 << 29));
    }

    ఒకవేళ (pae && CPULong_బహుళకం && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        ఒకవేళ (!qemu_quirk) {
            valid = 1;
        } లేకపోతే ఒకవేళ (physbits >= 41) {
            valid = 1;
        } లేకపోతే ఒకవేళ (strcmp(signature, "GenuineIntel") == 0) {
            ఒకవేళ ((physbits == 36) || (physbits == 39))
                valid = 1;
        } లేకపోతే ఒకవేళ (strcmp(signature, "AuthenticAMD") == 0) {
            ఒకవేళ (physbits == 40)
                valid = 1;
        }
    } లేకపోతే {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULong_బహుళకం ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    ఒకవేళ (valid)
        CPUPhys_బిట్లు = physbits;
}

స్థిర శూన్య qemu_detect(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU_HARDWARE)
        తిరిగి;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    ఒకవేళ (v == 0x0000 || v == 0xffff)
        తిరిగి;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    ఒకవేళ (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        తిరిగి;

    PlatformRunningOn |= PF_QEMU;
    ఎంపిక (d) {
    స్థితి 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        ఆపు;
    స్థితి 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        ఆపు;
    అప్రమేయ:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        ఆపు;
    }
    physbits(1);
}

స్థిర పూర్ణాంకం qemu_early_e820(శూన్య);

శూన్య
qemu_preinit(శూన్య)
{
    qemu_detect();
    kvm_detect();

    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    ఒకవేళ (runningOnXen()) {
        xen_ramsize_preinit();
        తిరిగి;
    }

    // try read e820 table first
    ఒకవేళ (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_దిగువ) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_ఎగువ) << 24));
        ఒకవేళ (rs)
            rs += 16 * 1024 * 1024;
        లేకపోతే
            rs = (((rtc_read(CMOS_MEM_EXTMEM_దిగువ) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_ఎగువ) << 18))
                  + 1 * 1024 * 1024);
        Ram_కొలత = rs;
        e820_కూడు(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_కొలత);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_కూడు(0xfffc0000, 256*1024, E820_RESERVED);
}

#నిర్వచించు MSR_IA32_FEATURE_CONTROL 0x0000003a

స్థిర శూన్య msr_feature_control_అమరిక(శూన్య)
{
    u64 feature_control_బిట్లు = romfile_loadint("etc/msr_feature_control", 0);
    ఒకవేళ (feature_control_బిట్లు)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_బిట్లు);
}

శూన్య
qemu_platform_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    ఒకవేళ (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_అమరిక();
        xen_biostable_అమరిక();
        తిరిగి;
    }

    kvmclock_init();

    // Initialize pci
    pci_అమరిక();
    smm_device_అమరిక();
    smm_అమరిక();

    // Initialize mtrr, msr_feature_control and smp
    mtrr_అమరిక();
    msr_feature_control_అమరిక();
    smp_అమరిక();

    // Create bios tables
    ఒకవేళ (Max_లెక్క_CPUs <= 255) {
        pirtable_అమరిక();
        mptable_అమరిక();
    }
    smbios_అమరిక();

    ఒకవేళ (CONFIG_FW_ROMFILE_ఎక్కించు) {
        పూర్ణాంకం loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = కనుగొను_acpi_rsdp();

        ఒకవేళ (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_అమరిక_acpi();
            తిరిగి;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        ఒకవేళ (!loader_err)
            warn_internalerror();
    }

    acpi_అమరిక();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#నిర్వచించు QEMU_CFG_SIGNATURE              0x00
#నిర్వచించు QEMU_CFG_ID                     0x01
#నిర్వచించు QEMU_CFG_UUID                   0x02
#నిర్వచించు QEMU_CFG_NOGRAPHIC              0x04
#నిర్వచించు QEMU_CFG_NUMA                   0x0d
#నిర్వచించు QEMU_CFG_BOOT_MENU              0x0e
#నిర్వచించు QEMU_CFG_NB_CPUS                0x05
#నిర్వచించు QEMU_CFG_MAX_CPUS               0x0f
#నిర్వచించు QEMU_CFG_FILE_DIR               0x19
#నిర్వచించు QEMU_CFG_ARCH_LOCAL             0x8000
#నిర్వచించు QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#నిర్వచించు QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#నిర్వచించు QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#నిర్వచించు QEMU_CFG_E820_పట్టిక             (QEMU_CFG_ARCH_LOCAL + 3)

స్థిర శూన్య
qemu_cfg_select(u16 f)
{
    outw(f, పోర్టు_QEMU_CFG_CTL);
}

స్థిర శూన్య
qemu_cfg_dma_transfer(శూన్య *చిరునామా, u32 నిడివి, u32 control)
{
    QemuCfgDmaAccess access;

    access.చిరునామా = cpu_గమ్యం_be64((u64)(u32)చిరునామా);
    access.నిడివి = cpu_గమ్యం_be32(నిడివి);
    access.control = cpu_గమ్యం_be32(control);

    barrier();

    outl(cpu_గమ్యం_be32((u32)&access), పోర్టు_QEMU_CFG_DMA_ADDR_దిగువ);

    అయితే(be32_గమ్యం_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

స్థిర శూన్య
qemu_cfg_read(శూన్య *buf, పూర్ణాంకం len)
{
    ఒకవేళ (len == 0) {
        తిరిగి;
    }

    ఒకవేళ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } లేకపోతే {
        insb(పోర్టు_QEMU_CFG_దత్తాంశం, buf, len);
    }
}

స్థిర శూన్య
qemu_cfg_write(శూన్య *buf, పూర్ణాంకం len)
{
    ఒకవేళ (len == 0) {
        తిరిగి;
    }

    ఒకవేళ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } లేకపోతే {
        warn_internalerror();
    }
}

స్థిర శూన్య
qemu_cfg_skip(పూర్ణాంకం len)
{
    ఒకవేళ (len == 0) {
        తిరిగి;
    }

    ఒకవేళ (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } లేకపోతే {
        అయితే (len--)
            inb(పోర్టు_QEMU_CFG_దత్తాంశం);
    }
}

స్థిర శూన్య
qemu_cfg_read_నమోదు(శూన్య *buf, పూర్ణాంకం e, పూర్ణాంకం len)
{
    ఒకవేళ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } లేకపోతే {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

స్థిర శూన్య
qemu_cfg_write_నమోదు(శూన్య *buf, పూర్ణాంకం e, పూర్ణాంకం len)
{
    ఒకవేళ (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } లేకపోతే {
        warn_internalerror();
    }
}

నిర్మాణం qemu_romfile_s {
    నిర్మాణం romfile_s file;
    పూర్ణాంకం select, skip;
};

స్థిర పూర్ణాంకం
qemu_cfg_read_file(నిర్మాణం romfile_s *file, శూన్య *dst, u32 maxlen)
{
    ఒకవేళ (file->కొలత > maxlen)
        తిరిగి -1;
    నిర్మాణం qemu_romfile_s *qfile;
    qfile = container_యొక్క(file, నిర్మాణం qemu_romfile_s, file);
    ఒకవేళ (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_నమోదు(dst, qfile->select, file->కొలత);
    } లేకపోతే {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->కొలత);
    }
    తిరిగి file->కొలత;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
పూర్ణాంకం
qemu_cfg_write_file_simple(శూన్య *src, u16 కీలకం, u32 స్థానభ్రంశం_2, u32 len)
{
    ఒకవేళ (స్థానభ్రంశం_2 == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_నమోదు(src, కీలకం, len);
    } లేకపోతే {
        qemu_cfg_select(కీలకం);
        qemu_cfg_skip(స్థానభ్రంశం_2);
        qemu_cfg_write(src, len);
    }
    తిరిగి len;
}

పూర్ణాంకం
qemu_cfg_write_file(శూన్య *src, నిర్మాణం romfile_s *file, u32 స్థానభ్రంశం_2, u32 len)
{
    ఒకవేళ ((స్థానభ్రంశం_2 + len) > file->కొలత)
        తిరిగి -1;

    ఒకవేళ (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        తిరిగి -1;
    }
    తిరిగి qemu_cfg_write_file_simple(src, qemu_get_romfile_కీలకం(file),
                                      స్థానభ్రంశం_2, len);
}

స్థిర శూన్య
qemu_romfile_కూడు(అక్షరం *name, పూర్ణాంకం select, పూర్ణాంకం skip, పూర్ణాంకం కొలత)
{
    నిర్మాణం qemu_romfile_s *qfile = malloc_tmp(పరిమాణం(*qfile));
    ఒకవేళ (!qfile) {
        warn_noalloc();
        తిరిగి;
    }
    memset(qfile, 0, పరిమాణం(*qfile));
    strtcpy(qfile->file.name, name, పరిమాణం(qfile->file.name));
    qfile->file.కొలత = కొలత;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_కూడు(&qfile->file);
}

u16
qemu_get_romfile_కీలకం(నిర్మాణం romfile_s *file)
{
    నిర్మాణం qemu_romfile_s *qfile;
    ఒకవేళ (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        తిరిగి 0;
    }
    qfile = container_యొక్క(file, నిర్మాణం qemu_romfile_s, file);
    తిరిగి qfile->select;
}

స్థిర పూర్ణాంకం rtc_present(శూన్య)
{
    తిరిగి rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_లెక్క(శూన్య)
{
    u16 smp_లెక్క = 0;
    ఒకవేళ (qemu_cfg_enabled()) {
        qemu_cfg_read_నమోదు(&smp_లెక్క, QEMU_CFG_NB_CPUS, పరిమాణం(smp_లెక్క));
    }
    ఒకవేళ (rtc_present()) {
        u16 cmos_cpu_లెక్క = rtc_read(CMOS_BIOS_SMP_లెక్క) + 1;
        ఒకవేళ (smp_లెక్క < cmos_cpu_లెక్క) {
            smp_లెక్క = cmos_cpu_లెక్క;
        }
    }
    తిరిగి smp_లెక్క;
}

నిర్మాణం e820_reservation {
    u64 చిరునామా;
    u64 నిడివి;
    u32 type;
};

#నిర్వచించు SMBIOS_క్షేత్రం_నమోదు 0
#నిర్వచించు SMBIOS_పట్టిక_నమోదు 1

నిర్మాణం qemu_smbios_శీర్షిక {
    u16 నిడివి;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

స్థిర శూన్య
qemu_cfg_e820(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    ఒకవేళ (romfile_కనుగొను("etc/e820")) {
        // qemu_early_e820() has handled everything
        తిరిగి;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_నమోదు(&count32, QEMU_CFG_E820_పట్టిక, పరిమాణం(count32));
    ఒకవేళ (count32) {
        నిర్మాణం e820_reservation నమోదు_2;
        పూర్ణాంకం i;
        కొరకు (i = 0; i < count32; i++) {
            qemu_cfg_read(&నమోదు_2, పరిమాణం(నమోదు_2));
            e820_కూడు(నమోదు_2.చిరునామా, నమోదు_2.నిడివి, నమోదు_2.type);
        }
    } లేకపోతే ఒకవేళ (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_కూడు(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 ఎగువ = ((rtc_read(CMOS_MEM_HIGHMEM_దిగువ) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_ఎగువ) << 32));
    Ram_కొలత_Over4_G = ఎగువ;
    e820_కూడు(0x100000000ull, ఎగువ, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_కొలత_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
స్థిర శూన్య
qemu_cfg_legacy(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    // Misc config items.
    qemu_romfile_కూడు("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_కూడు("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_కూడు("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_నమోదు(&numacount, QEMU_CFG_NUMA, పరిమాణం(numacount));
    పూర్ణాంకం max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_కూడు("etc/numa-cpu-map", QEMU_CFG_NUMA, పరిమాణం(numacount)
                     , max_cpu*పరిమాణం(u64));
    qemu_romfile_కూడు("etc/numa-nodes", QEMU_CFG_NUMA
                     , పరిమాణం(numacount) + max_cpu*పరిమాణం(u64)
                     , numacount*పరిమాణం(u64));

    // ACPI tables
    అక్షరం name[128];
    u16 cnt;
    qemu_cfg_read_నమోదు(&cnt, QEMU_CFG_ACPI_TABLES, పరిమాణం(cnt));
    పూర్ణాంకం i, స్థానభ్రంశం_2 = పరిమాణం(cnt);
    కొరకు (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, పరిమాణం(len));
        స్థానభ్రంశం_2 += పరిమాణం(len);
        snprintf(name, పరిమాణం(name), "acpi/table%d", i);
        qemu_romfile_కూడు(name, QEMU_CFG_ACPI_TABLES, స్థానభ్రంశం_2, len);
        qemu_cfg_skip(len);
        స్థానభ్రంశం_2 += len;
    }

    // SMBIOS info
    qemu_cfg_read_నమోదు(&cnt, QEMU_CFG_SMBIOS_ENTRIES, పరిమాణం(cnt));
    స్థానభ్రంశం_2 = పరిమాణం(cnt);
    కొరకు (i = 0; i < cnt; i++) {
        నిర్మాణం qemu_smbios_శీర్షిక శీర్షిక;
        qemu_cfg_read(&శీర్షిక, పరిమాణం(శీర్షిక));
        ఒకవేళ (శీర్షిక.headertype == SMBIOS_క్షేత్రం_నమోదు) {
            snprintf(name, పరిమాణం(name), "smbios/field%d-%d"
                     , శీర్షిక.tabletype, శీర్షిక.fieldoffset);
            qemu_romfile_కూడు(name, QEMU_CFG_SMBIOS_ENTRIES
                             , స్థానభ్రంశం_2 + పరిమాణం(శీర్షిక)
                             , శీర్షిక.నిడివి - పరిమాణం(శీర్షిక));
        } లేకపోతే {
            snprintf(name, పరిమాణం(name), "smbios/table%d-%d"
                     , శీర్షిక.tabletype, i);
            qemu_romfile_కూడు(name, QEMU_CFG_SMBIOS_ENTRIES
                             , స్థానభ్రంశం_2 + 3, శీర్షిక.నిడివి - 3);
        }
        qemu_cfg_skip(శీర్షిక.నిడివి - పరిమాణం(శీర్షిక));
        స్థానభ్రంశం_2 += శీర్షిక.నిడివి;
    }
}

నిర్మాణం QemuCfgFile {
    u32  కొలత;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    అక్షరం name[56];
};

స్థిర పూర్ణాంకం qemu_cfg_detect(శూన్య)
{
    ఒకవేళ (cfg_enabled)
        తిరిగి 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    అక్షరం *sig = "QEMU";
    పూర్ణాంకం i;
    కొరకు (i = 0; i < 4; i++)
        ఒకవేళ (inb(పోర్టు_QEMU_CFG_దత్తాంశం) != sig[i])
            తిరిగి 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_నమోదు(&id, QEMU_CFG_ID, పరిమాణం(id));

    ఒకవేళ (id & QEMU_CFG_సంస్కరణ_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    తిరిగి 1;
}

శూన్య qemu_cfg_init(శూన్య)
{
    ఒకవేళ (!runningOnQEMU())
        తిరిగి;

    ఒకవేళ (!qemu_cfg_detect())
        తిరిగి;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 లెక్క;
    qemu_cfg_read_నమోదు(&లెక్క, QEMU_CFG_FILE_DIR, పరిమాణం(లెక్క));
    లెక్క = be32_గమ్యం_cpu(లెక్క);
    u32 e;
    కొరకు (e = 0; e < లెక్క; e++) {
        నిర్మాణం QemuCfgFile qfile;
        qemu_cfg_read(&qfile, పరిమాణం(qfile));
        qemu_romfile_కూడు(qfile.name, be16_గమ్యం_cpu(qfile.select)
                         , 0, be32_గమ్యం_cpu(qfile.కొలత));
    }

    qemu_cfg_e820();

    ఒకవేళ (romfile_కనుగొను("etc/table-loader")) {
        acpi_pm_పునాది = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_పునాది);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_నమోదు(&nogfx, QEMU_CFG_NOGRAPHIC, పరిమాణం(nogfx));
    ఒకవేళ (nogfx && !romfile_కనుగొను("etc/sercon-port")
        && !romfile_కనుగొను("vgaroms/sgabios.bin"))
        const_romfile_కూడు_int("etc/sercon-port", పోర్టు_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
స్థిర పూర్ణాంకం qemu_early_e820(శూన్య)
{
    నిర్మాణం e820_reservation పట్టిక;
    నిర్మాణం QemuCfgFile qfile;
    u32 select = 0, కొలత = 0;
    u32 లెక్క, i;

    ఒకవేళ (!qemu_cfg_detect())
        తిరిగి 0;

    // find e820 table
    qemu_cfg_read_నమోదు(&లెక్క, QEMU_CFG_FILE_DIR, పరిమాణం(లెక్క));
    లెక్క = be32_గమ్యం_cpu(లెక్క);
    కొరకు (i = 0; i < లెక్క; i++) {
        qemu_cfg_read(&qfile, పరిమాణం(qfile));
        ఒకవేళ (memcmp(qfile.name, "etc/e820", 9) != 0)
            కొనసాగు;
        select = be16_గమ్యం_cpu(qfile.select);
        కొలత = be32_గమ్యం_cpu(qfile.కొలత);
        ఆపు;
    }
    ఒకవేళ (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        తిరిగి 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    లెక్క = కొలత/పరిమాణం(పట్టిక);
    కొరకు (i = 0, select = 0; i < లెక్క; i++) {
        qemu_cfg_read(&పట్టిక, పరిమాణం(పట్టిక));
        ఎంపిక (పట్టిక.type) {
        స్థితి E820_RESERVED:
            e820_కూడు(పట్టిక.చిరునామా, పట్టిక.నిడివి, పట్టిక.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    పట్టిక.చిరునామా, పట్టిక.నిడివి);
            ఆపు;
        స్థితి E820_SOFT_RESERVED:
            e820_కూడు(పట్టిక.చిరునామా, పట్టిక.నిడివి, పట్టిక.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    పట్టిక.చిరునామా, పట్టిక.నిడివి);
            ఆపు;
        స్థితి E820_RAM:
            e820_కూడు(పట్టిక.చిరునామా, పట్టిక.నిడివి, పట్టిక.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    పట్టిక.చిరునామా, పట్టిక.నిడివి);
            ఒకవేళ (పట్టిక.చిరునామా < 0x100000000LL) {
                // below 4g
                ఒకవేళ (Ram_కొలత < పట్టిక.చిరునామా + పట్టిక.నిడివి)
                    Ram_కొలత = పట్టిక.చిరునామా + పట్టిక.నిడివి;
            } లేకపోతే {
                // above 4g
                ఒకవేళ (Ram_కొలత_Over4_G < పట్టిక.చిరునామా + పట్టిక.నిడివి - 0x100000000LL)
                    Ram_కొలత_Over4_G = పట్టిక.చిరునామా + పట్టిక.నిడివి - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_కొలత);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_కొలత_Over4_G);
    తిరిగి 1;
}
