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
u32 Ram_سائز;
// Amount of continuous ram >4Gig
u64 Ram_سائز_Over4_G;
// physical address space bits
u8 CPUPhys_بٹ;
// 64bit processor
u8 CPULong_منوال;
// Type of emulator platform.
صحیح PlatformRunningOn VARFSEG;
// cfg enabled
صحیح cfg_enabled = 0;
// cfg_dma enabled
صحیح cfg_dma_enabled = 0;

اندرونی صحیح qemu_cfg_enabled(خالی)
{
    واپس cfg_enabled;
}

اندرونی صحیح qemu_cfg_dma_enabled(خالی)
{
    واپس cfg_dma_enabled;
}

/* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
 * should be used to determine that a VM is running under KVM.
 */
#تعریف_کرو KVM_CPUID_SIGNATURE     0x40000000
ساکن غیرعلامتی صحیح kvm_cpuid_بنیاد = 0;

ساکن خالی kvm_detect(خالی)
{
    غیرعلامتی صحیح i, max = 0;
    غیرعلامتی صحیح eax, ebx, ecx, edx;
    حرف signature[13];

    برائے (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        اگر (eax < i)
            توڑو;
        حافظہ_نقل(signature + 0, &ebx, 4);
        حافظہ_نقل(signature + 4, &ecx, 4);
        حافظہ_نقل(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        اگر (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_بنیاد = i;
            max = eax;
        }
    }

    اگر (kvm_cpuid_بنیاد) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        اگر (max >= kvm_cpuid_بنیاد + 0x10) {
            cpuid(kvm_cpuid_بنیاد + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#تعریف_کرو KVM_FEATURE_CLOCKSOURCE           0
#تعریف_کرو KVM_FEATURE_CLOCKSOURCE2          3

#تعریف_کرو MSR_KVM_SYSTEM_وقت            0x12
#تعریف_کرو MSR_KVM_SYSTEM_وقت_NEW  0x4b564d01

#تعریف_کرو PVCLOCK_TSC_STABLE_ثنائی_ہندسہ     (1 << 0)

ساخت pvclock_vcpu_وقت_info *kvmclock;

ساکن خالی kvmclock_init(خالی)
{
    غیرعلامتی صحیح eax, ebx, ecx, edx, msr;

    اگر (!runningOnKVM())
        واپس;

    cpuid(kvm_cpuid_بنیاد + 0x01, &eax, &ebx, &ecx, &edx);
    اگر (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_وقت_NEW;
    ورنہ اگر (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_وقت;
    ورنہ
        واپس;

    kvmclock = memalign_کم(حجم(*kvmclock), 32);
    memset(kvmclock, 0, حجم(*kvmclock));
    u32 قدر = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", قدر, msr);
    wrmsr(msr, قدر | 0x01);

    اگر (!(kvmclock->flags & PVCLOCK_TSC_STABLE_ثنائی_ہندسہ))
        واپس;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_مقصد_system_mul >> 16);
    اگر (kvmclock->tsc_سرکاؤ < 0)
        MHz <<= -kvmclock->tsc_سرکاؤ;
    ورنہ
        MHz >>= kvmclock->tsc_سرکاؤ;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ساکن خالی physbits(صحیح qemu_quirk)
{
    غیرعلامتی صحیح max, eax, ebx, ecx, edx;
    غیرعلامتی صحیح physbits;
    حرف signature[13];
    صحیح pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    حافظہ_نقل(signature + 0, &ebx, 4);
    حافظہ_نقل(signature + 4, &edx, 4);
    حافظہ_نقل(signature + 8, &ecx, 4);
    signature[12] = 0;
    اگر (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    اگر (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULong_منوال = !!(edx & (1 << 29));
    }

    اگر (pae && CPULong_منوال && max >= 0x80000008) {
        cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        physbits = (u8)eax;
        اگر (!qemu_quirk) {
            valid = 1;
        } ورنہ اگر (physbits >= 41) {
            valid = 1;
        } ورنہ اگر (strcmp(signature, "GenuineIntel") == 0) {
            اگر ((physbits == 36) || (physbits == 39))
                valid = 1;
        } ورنہ اگر (strcmp(signature, "AuthenticAMD") == 0) {
            اگر (physbits == 40)
                valid = 1;
        }
    } ورنہ {
        physbits = pae ? 36 : 32;
        valid = 1;
    }

    dprintf(1, "%s: signature=\"%s\", pae=%s, lm=%s, phys-bits=%d, valid=%s\n",
            __func__, signature, pae ? "yes" : "no", CPULong_منوال ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    اگر (valid)
        CPUPhys_بٹ = physbits;
}

ساکن خالی qemu_detect(خالی)
{
    اگر (!CONFIG_QEMU_HARDWARE)
        واپس;

    // Setup QEMU debug output port
    qemu_debug_preinit();

    // check northbridge @ 00:00.0
    u16 v = pci_config_readw(0, PCI_VENDOR_ID);
    اگر (v == 0x0000 || v == 0xffff)
        واپس;
    u16 d = pci_config_readw(0, PCI_DEVICE_ID);
    u16 sv = pci_config_readw(0, PCI_SUBSYSTEM_VENDOR_ID);
    u16 sd = pci_config_readw(0, PCI_SUBSYSTEM_ID);

    اگر (sv != 0x1af4 || /* Red Hat, Inc */
        sd != 0x1100)   /* Qemu virtual machine */
        واپس;

    PlatformRunningOn |= PF_QEMU;
    انتخاب (d) {
    حالت 0x1237:
        dprintf(1, "Running on QEMU (i440fx)\n");
        توڑو;
    حالت 0x29c0:
        dprintf(1, "Running on QEMU (q35)\n");
        توڑو;
    طےشدہ:
        dprintf(1, "Running on QEMU (unknown nb: %04x:%04x)\n", v, d);
        توڑو;
    }
    physbits(1);
}

ساکن صحیح qemu_early_e820(خالی);

خالی
qemu_preinit(خالی)
{
    qemu_detect();
    kvm_detect();

    اگر (!CONFIG_QEMU)
        واپس;

    اگر (runningOnXen()) {
        xen_ramsize_preinit();
        واپس;
    }

    // try read e820 table first
    اگر (!qemu_early_e820()) {
        // when it fails get memory size from nvram.
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_کم) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_بلند) << 24));
        اگر (rs)
            rs += 16 * 1024 * 1024;
        ورنہ
            rs = (((rtc_read(CMOS_MEM_EXTMEM_کم) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_بلند) << 18))
                  + 1 * 1024 * 1024);
        Ram_سائز = rs;
        e820_جوڑو(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_سائز);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_جوڑو(0xfffc0000, 256*1024, E820_RESERVED);
}

#تعریف_کرو MSR_IA32_FEATURE_CONTROL 0x0000003a

ساکن خالی msr_feature_control_تیاری(خالی)
{
    u64 feature_control_بٹ = romfile_loadint("etc/msr_feature_control", 0);
    اگر (feature_control_بٹ)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_بٹ);
}

خالی
qemu_platform_تیاری(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    اگر (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_تیاری();
        xen_biostable_تیاری();
        واپس;
    }

    kvmclock_init();

    // Initialize pci
    pci_تیاری();
    smm_device_تیاری();
    smm_تیاری();

    // Initialize mtrr, msr_feature_control and smp
    mtrr_تیاری();
    msr_feature_control_تیاری();
    smp_تیاری();

    // Create bios tables
    اگر (Max_گنتی_CPUs <= 255) {
        pirtable_تیاری();
        mptable_تیاری();
    }
    smbios_تیاری();

    اگر (CONFIG_FW_ROMFILE_بھرو) {
        صحیح loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = ڈھونڈو_acpi_rsdp();

        اگر (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_تیاری_acpi();
            واپس;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        اگر (!loader_err)
            warn_internalerror();
    }

    acpi_تیاری();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#تعریف_کرو QEMU_CFG_SIGNATURE              0x00
#تعریف_کرو QEMU_CFG_ID                     0x01
#تعریف_کرو QEMU_CFG_UUID                   0x02
#تعریف_کرو QEMU_CFG_NOGRAPHIC              0x04
#تعریف_کرو QEMU_CFG_NUMA                   0x0d
#تعریف_کرو QEMU_CFG_BOOT_MENU              0x0e
#تعریف_کرو QEMU_CFG_NB_CPUS                0x05
#تعریف_کرو QEMU_CFG_MAX_CPUS               0x0f
#تعریف_کرو QEMU_CFG_FILE_DIR               0x19
#تعریف_کرو QEMU_CFG_ARCH_LOCAL             0x8000
#تعریف_کرو QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#تعریف_کرو QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#تعریف_کرو QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#تعریف_کرو QEMU_CFG_E820_جدول             (QEMU_CFG_ARCH_LOCAL + 3)

ساکن خالی
qemu_cfg_select(u16 f)
{
    outw(f, پورٹ_QEMU_CFG_CTL);
}

ساکن خالی
qemu_cfg_dma_transfer(خالی *پتہ, u32 لمبائی, u32 control)
{
    QemuCfgDmaAccess access;

    access.پتہ = cpu_مقصد_be64((u64)(u32)پتہ);
    access.لمبائی = cpu_مقصد_be32(لمبائی);
    access.control = cpu_مقصد_be32(control);

    barrier();

    outl(cpu_مقصد_be32((u32)&access), پورٹ_QEMU_CFG_DMA_ADDR_کم);

    جب(be32_مقصد_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
        yield();
    }
}

ساکن خالی
qemu_cfg_read(خالی *buf, صحیح len)
{
    اگر (len == 0) {
        واپس;
    }

    اگر (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_READ);
    } ورنہ {
        insb(پورٹ_QEMU_CFG_ڈیٹا, buf, len);
    }
}

ساکن خالی
qemu_cfg_write(خالی *buf, صحیح len)
{
    اگر (len == 0) {
        واپس;
    }

    اگر (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(buf, len, QEMU_CFG_DMA_CTL_WRITE);
    } ورنہ {
        warn_internalerror();
    }
}

ساکن خالی
qemu_cfg_skip(صحیح len)
{
    اگر (len == 0) {
        واپس;
    }

    اگر (qemu_cfg_dma_enabled()) {
        qemu_cfg_dma_transfer(0, len, QEMU_CFG_DMA_CTL_SKIP);
    } ورنہ {
        جب (len--)
            inb(پورٹ_QEMU_CFG_ڈیٹا);
    }
}

ساکن خالی
qemu_cfg_read_اندراج(خالی *buf, صحیح e, صحیح len)
{
    اگر (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_READ;
        qemu_cfg_dma_transfer(buf, len, control);
    } ورنہ {
        qemu_cfg_select(e);
        qemu_cfg_read(buf, len);
    }
}

ساکن خالی
qemu_cfg_write_اندراج(خالی *buf, صحیح e, صحیح len)
{
    اگر (qemu_cfg_dma_enabled()) {
        u32 control = (e << 16) | QEMU_CFG_DMA_CTL_SELECT
                        | QEMU_CFG_DMA_CTL_WRITE;
        qemu_cfg_dma_transfer(buf, len, control);
    } ورنہ {
        warn_internalerror();
    }
}

ساخت qemu_romfile_s {
    ساخت romfile_s file;
    صحیح select, skip;
};

ساکن صحیح
qemu_cfg_read_file(ساخت romfile_s *file, خالی *dst, u32 maxlen)
{
    اگر (file->سائز > maxlen)
        واپس -1;
    ساخت qemu_romfile_s *qfile;
    qfile = container_کا(file, ساخت qemu_romfile_s, file);
    اگر (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_اندراج(dst, qfile->select, file->سائز);
    } ورنہ {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->سائز);
    }
    واپس file->سائز;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
صحیح
qemu_cfg_write_file_simple(خالی *src, u16 کلید, u32 انحراف_2, u32 len)
{
    اگر (انحراف_2 == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_اندراج(src, کلید, len);
    } ورنہ {
        qemu_cfg_select(کلید);
        qemu_cfg_skip(انحراف_2);
        qemu_cfg_write(src, len);
    }
    واپس len;
}

صحیح
qemu_cfg_write_file(خالی *src, ساخت romfile_s *file, u32 انحراف_2, u32 len)
{
    اگر ((انحراف_2 + len) > file->سائز)
        واپس -1;

    اگر (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        واپس -1;
    }
    واپس qemu_cfg_write_file_simple(src, qemu_get_romfile_کلید(file),
                                      انحراف_2, len);
}

ساکن خالی
qemu_romfile_جوڑو(حرف *name, صحیح select, صحیح skip, صحیح سائز)
{
    ساخت qemu_romfile_s *qfile = malloc_tmp(حجم(*qfile));
    اگر (!qfile) {
        warn_noalloc();
        واپس;
    }
    memset(qfile, 0, حجم(*qfile));
    strtcpy(qfile->file.name, name, حجم(qfile->file.name));
    qfile->file.سائز = سائز;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_جوڑو(&qfile->file);
}

u16
qemu_get_romfile_کلید(ساخت romfile_s *file)
{
    ساخت qemu_romfile_s *qfile;
    اگر (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        واپس 0;
    }
    qfile = container_کا(file, ساخت qemu_romfile_s, file);
    واپس qfile->select;
}

ساکن صحیح rtc_present(خالی)
{
    واپس rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_گنتی(خالی)
{
    u16 smp_گنتی = 0;
    اگر (qemu_cfg_enabled()) {
        qemu_cfg_read_اندراج(&smp_گنتی, QEMU_CFG_NB_CPUS, حجم(smp_گنتی));
    }
    اگر (rtc_present()) {
        u16 cmos_cpu_گنتی = rtc_read(CMOS_BIOS_SMP_گنتی) + 1;
        اگر (smp_گنتی < cmos_cpu_گنتی) {
            smp_گنتی = cmos_cpu_گنتی;
        }
    }
    واپس smp_گنتی;
}

ساخت e820_reservation {
    u64 پتہ;
    u64 لمبائی;
    u32 type;
};

#تعریف_کرو SMBIOS_میدان_اندراج 0
#تعریف_کرو SMBIOS_جدول_اندراج 1

ساخت qemu_smbios_سرخی {
    u16 لمبائی;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ساکن خالی
qemu_cfg_e820(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    اگر (romfile_ڈھونڈو("etc/e820")) {
        // qemu_early_e820() has handled everything
        واپس;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_اندراج(&count32, QEMU_CFG_E820_جدول, حجم(count32));
    اگر (count32) {
        ساخت e820_reservation اندراج_2;
        صحیح i;
        برائے (i = 0; i < count32; i++) {
            qemu_cfg_read(&اندراج_2, حجم(اندراج_2));
            e820_جوڑو(اندراج_2.پتہ, اندراج_2.لمبائی, اندراج_2.type);
        }
    } ورنہ اگر (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_جوڑو(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 بلند = ((rtc_read(CMOS_MEM_HIGHMEM_کم) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_بلند) << 32));
    Ram_سائز_Over4_G = بلند;
    e820_جوڑو(0x100000000ull, بلند, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_سائز_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ساکن خالی
qemu_cfg_legacy(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    // Misc config items.
    qemu_romfile_جوڑو("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_جوڑو("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_جوڑو("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_اندراج(&numacount, QEMU_CFG_NUMA, حجم(numacount));
    صحیح max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_جوڑو("etc/numa-cpu-map", QEMU_CFG_NUMA, حجم(numacount)
                     , max_cpu*حجم(u64));
    qemu_romfile_جوڑو("etc/numa-nodes", QEMU_CFG_NUMA
                     , حجم(numacount) + max_cpu*حجم(u64)
                     , numacount*حجم(u64));

    // ACPI tables
    حرف name[128];
    u16 cnt;
    qemu_cfg_read_اندراج(&cnt, QEMU_CFG_ACPI_TABLES, حجم(cnt));
    صحیح i, انحراف_2 = حجم(cnt);
    برائے (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, حجم(len));
        انحراف_2 += حجم(len);
        snprintf(name, حجم(name), "acpi/table%d", i);
        qemu_romfile_جوڑو(name, QEMU_CFG_ACPI_TABLES, انحراف_2, len);
        qemu_cfg_skip(len);
        انحراف_2 += len;
    }

    // SMBIOS info
    qemu_cfg_read_اندراج(&cnt, QEMU_CFG_SMBIOS_ENTRIES, حجم(cnt));
    انحراف_2 = حجم(cnt);
    برائے (i = 0; i < cnt; i++) {
        ساخت qemu_smbios_سرخی سرخی;
        qemu_cfg_read(&سرخی, حجم(سرخی));
        اگر (سرخی.headertype == SMBIOS_میدان_اندراج) {
            snprintf(name, حجم(name), "smbios/field%d-%d"
                     , سرخی.tabletype, سرخی.fieldoffset);
            qemu_romfile_جوڑو(name, QEMU_CFG_SMBIOS_ENTRIES
                             , انحراف_2 + حجم(سرخی)
                             , سرخی.لمبائی - حجم(سرخی));
        } ورنہ {
            snprintf(name, حجم(name), "smbios/table%d-%d"
                     , سرخی.tabletype, i);
            qemu_romfile_جوڑو(name, QEMU_CFG_SMBIOS_ENTRIES
                             , انحراف_2 + 3, سرخی.لمبائی - 3);
        }
        qemu_cfg_skip(سرخی.لمبائی - حجم(سرخی));
        انحراف_2 += سرخی.لمبائی;
    }
}

ساخت QemuCfgFile {
    u32  سائز;        /* file size */
    u16  select;      /* write this to 0x510 to read it */
    u16  reserved;
    حرف name[56];
};

ساکن صحیح qemu_cfg_detect(خالی)
{
    اگر (cfg_enabled)
        واپس 1;

    // Detect fw_cfg interface.
    qemu_cfg_select(QEMU_CFG_SIGNATURE);
    حرف *sig = "QEMU";
    صحیح i;
    برائے (i = 0; i < 4; i++)
        اگر (inb(پورٹ_QEMU_CFG_ڈیٹا) != sig[i])
            واپس 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_اندراج(&id, QEMU_CFG_ID, حجم(id));

    اگر (id & QEMU_CFG_نسخہ_DMA) {
        dprintf(1, "QEMU fw_cfg DMA interface supported\n");
        cfg_dma_enabled = 1;
    }
    واپس 1;
}

خالی qemu_cfg_init(خالی)
{
    اگر (!runningOnQEMU())
        واپس;

    اگر (!qemu_cfg_detect())
        واپس;

    // Populate romfiles for legacy fw_cfg entries
    qemu_cfg_legacy();

    // Load files found in the fw_cfg file directory
    u32 گنتی;
    qemu_cfg_read_اندراج(&گنتی, QEMU_CFG_FILE_DIR, حجم(گنتی));
    گنتی = be32_مقصد_cpu(گنتی);
    u32 e;
    برائے (e = 0; e < گنتی; e++) {
        ساخت QemuCfgFile qfile;
        qemu_cfg_read(&qfile, حجم(qfile));
        qemu_romfile_جوڑو(qfile.name, be16_مقصد_cpu(qfile.select)
                         , 0, be32_مقصد_cpu(qfile.سائز));
    }

    qemu_cfg_e820();

    اگر (romfile_ڈھونڈو("etc/table-loader")) {
        acpi_pm_بنیاد = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_بنیاد);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_اندراج(&nogfx, QEMU_CFG_NOGRAPHIC, حجم(nogfx));
    اگر (nogfx && !romfile_ڈھونڈو("etc/sercon-port")
        && !romfile_ڈھونڈو("vgaroms/sgabios.bin"))
        const_romfile_جوڑو_int("etc/sercon-port", پورٹ_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ساکن صحیح qemu_early_e820(خالی)
{
    ساخت e820_reservation جدول;
    ساخت QemuCfgFile qfile;
    u32 select = 0, سائز = 0;
    u32 گنتی, i;

    اگر (!qemu_cfg_detect())
        واپس 0;

    // find e820 table
    qemu_cfg_read_اندراج(&گنتی, QEMU_CFG_FILE_DIR, حجم(گنتی));
    گنتی = be32_مقصد_cpu(گنتی);
    برائے (i = 0; i < گنتی; i++) {
        qemu_cfg_read(&qfile, حجم(qfile));
        اگر (memcmp(qfile.name, "etc/e820", 9) != 0)
            جاری;
        select = be16_مقصد_cpu(qfile.select);
        سائز = be32_مقصد_cpu(qfile.سائز);
        توڑو;
    }
    اگر (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        واپس 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    گنتی = سائز/حجم(جدول);
    برائے (i = 0, select = 0; i < گنتی; i++) {
        qemu_cfg_read(&جدول, حجم(جدول));
        انتخاب (جدول.type) {
        حالت E820_RESERVED:
            e820_جوڑو(جدول.پتہ, جدول.لمبائی, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    جدول.پتہ, جدول.لمبائی);
            توڑو;
        حالت E820_SOFT_RESERVED:
            e820_جوڑو(جدول.پتہ, جدول.لمبائی, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    جدول.پتہ, جدول.لمبائی);
            توڑو;
        حالت E820_RAM:
            e820_جوڑو(جدول.پتہ, جدول.لمبائی, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    جدول.پتہ, جدول.لمبائی);
            اگر (جدول.پتہ < 0x100000000LL) {
                // below 4g
                اگر (Ram_سائز < جدول.پتہ + جدول.لمبائی)
                    Ram_سائز = جدول.پتہ + جدول.لمبائی;
            } ورنہ {
                // above 4g
                اگر (Ram_سائز_Over4_G < جدول.پتہ + جدول.لمبائی - 0x100000000LL)
                    Ram_سائز_Over4_G = جدول.پتہ + جدول.لمبائی - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_سائز);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_سائز_Over4_G);
    واپس 1;
}
