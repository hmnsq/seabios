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
u32 Ram_سائيز;
// Amount of continuous ram >4Gig
u64 Ram_سائيز_Over4_G;
// physical address space bits
u8 CPUPhys_بٽ;
// 64bit processor
u8 CPULong_گهڻائي;
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
#وصف_ڪريو KVM_CPUID_SIGNATURE     0x40000000
ساكن بنانشان صحيح kvm_cpuid_بنياد = 0;

ساكن فراغ kvm_detect(فراغ)
{
    بنانشان صحيح i, max = 0;
    بنانشان صحيح eax, ebx, ecx, edx;
    حرف signature[13];

    لكل (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        إذا (eax < i)
            اكسر;
        يادگيري_نقل(signature + 0, &ebx, 4);
        يادگيري_نقل(signature + 4, &ecx, 4);
        يادگيري_نقل(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        إذا (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_بنياد = i;
            max = eax;
        }
    }

    إذا (kvm_cpuid_بنياد) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        إذا (max >= kvm_cpuid_بنياد + 0x10) {
            cpuid(kvm_cpuid_بنياد + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#وصف_ڪريو KVM_FEATURE_CLOCKSOURCE           0
#وصف_ڪريو KVM_FEATURE_CLOCKSOURCE2          3

#وصف_ڪريو MSR_KVM_SYSTEM_وقت            0x12
#وصف_ڪريو MSR_KVM_SYSTEM_وقت_NEW  0x4b564d01

#وصف_ڪريو PVCLOCK_TSC_STABLE_ٻٽو_انگ     (1 << 0)

بنية pvclock_vcpu_وقت_info *kvmclock;

ساكن فراغ kvmclock_init(فراغ)
{
    بنانشان صحيح eax, ebx, ecx, edx, msr;

    إذا (!runningOnKVM())
        أرجع;

    cpuid(kvm_cpuid_بنياد + 0x01, &eax, &ebx, &ecx, &edx);
    إذا (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_وقت_NEW;
    وإلا إذا (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_وقت;
    وإلا
        أرجع;

    kvmclock = memalign_هيٺيون(حجم(*kvmclock), 32);
    memset(kvmclock, 0, حجم(*kvmclock));
    u32 قدر = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", قدر, msr);
    wrmsr(msr, قدر | 0x01);

    إذا (!(kvmclock->flags & PVCLOCK_TSC_STABLE_ٻٽو_انگ))
        أرجع;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_منزل_system_mul >> 16);
    إذا (kvmclock->tsc_سرڪايو < 0)
        MHz <<= -kvmclock->tsc_سرڪايو;
    وإلا
        MHz >>= kvmclock->tsc_سرڪايو;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ساكن فراغ physbits(صحيح qemu_quirk)
{
    بنانشان صحيح max, eax, ebx, ecx, edx;
    بنانشان صحيح physbits;
    حرف signature[13];
    صحيح pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    يادگيري_نقل(signature + 0, &ebx, 4);
    يادگيري_نقل(signature + 4, &edx, 4);
    يادگيري_نقل(signature + 8, &ecx, 4);
    signature[12] = 0;
    إذا (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    إذا (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULong_گهڻائي = !!(edx & (1 << 29));
    }

    إذا (pae && CPULong_گهڻائي && max >= 0x80000008) {
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
            __func__, signature, pae ? "yes" : "no", CPULong_گهڻائي ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    إذا (valid)
        CPUPhys_بٽ = physbits;
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
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_هيٺيون) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_مٿاهون) << 24));
        إذا (rs)
            rs += 16 * 1024 * 1024;
        وإلا
            rs = (((rtc_read(CMOS_MEM_EXTMEM_هيٺيون) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_مٿاهون) << 18))
                  + 1 * 1024 * 1024);
        Ram_سائيز = rs;
        e820_جوڙيو(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_سائيز);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_جوڙيو(0xfffc0000, 256*1024, E820_RESERVED);
}

#وصف_ڪريو MSR_IA32_FEATURE_CONTROL 0x0000003a

ساكن فراغ msr_feature_control_تياري(فراغ)
{
    u64 feature_control_بٽ = romfile_loadint("etc/msr_feature_control", 0);
    إذا (feature_control_بٽ)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_بٽ);
}

فراغ
qemu_platform_تياري(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    إذا (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_تياري();
        xen_biostable_تياري();
        أرجع;
    }

    kvmclock_init();

    // Initialize pci
    pci_تياري();
    smm_device_تياري();
    smm_تياري();

    // Initialize mtrr, msr_feature_control and smp
    mtrr_تياري();
    msr_feature_control_تياري();
    smp_تياري();

    // Create bios tables
    إذا (Max_ڳڻپ_CPUs <= 255) {
        pirtable_تياري();
        mptable_تياري();
    }
    smbios_تياري();

    إذا (CONFIG_FW_ROMFILE_ڀريو) {
        صحيح loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = ڳوليو_acpi_rsdp();

        إذا (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_تياري_acpi();
            أرجع;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        إذا (!loader_err)
            warn_internalerror();
    }

    acpi_تياري();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#وصف_ڪريو QEMU_CFG_SIGNATURE              0x00
#وصف_ڪريو QEMU_CFG_ID                     0x01
#وصف_ڪريو QEMU_CFG_UUID                   0x02
#وصف_ڪريو QEMU_CFG_NOGRAPHIC              0x04
#وصف_ڪريو QEMU_CFG_NUMA                   0x0d
#وصف_ڪريو QEMU_CFG_BOOT_MENU              0x0e
#وصف_ڪريو QEMU_CFG_NB_CPUS                0x05
#وصف_ڪريو QEMU_CFG_MAX_CPUS               0x0f
#وصف_ڪريو QEMU_CFG_FILE_DIR               0x19
#وصف_ڪريو QEMU_CFG_ARCH_LOCAL             0x8000
#وصف_ڪريو QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#وصف_ڪريو QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#وصف_ڪريو QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#وصف_ڪريو QEMU_CFG_E820_جدول             (QEMU_CFG_ARCH_LOCAL + 3)

ساكن فراغ
qemu_cfg_select(u16 f)
{
    outw(f, پورٽ_QEMU_CFG_CTL);
}

ساكن فراغ
qemu_cfg_dma_transfer(فراغ *پتو, u32 ڊيگهائي, u32 control)
{
    QemuCfgDmaAccess access;

    access.پتو = cpu_منزل_be64((u64)(u32)پتو);
    access.ڊيگهائي = cpu_منزل_be32(ڊيگهائي);
    access.control = cpu_منزل_be32(control);

    barrier();

    outl(cpu_منزل_be32((u32)&access), پورٽ_QEMU_CFG_DMA_ADDR_هيٺيون);

    طالما(be32_منزل_cpu(access.control) & ~QEMU_CFG_DMA_CTL_ERROR) {
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
        insb(پورٽ_QEMU_CFG_معلومات, buf, len);
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
            inb(پورٽ_QEMU_CFG_معلومات);
    }
}

ساكن فراغ
qemu_cfg_read_داخلا(فراغ *buf, صحيح e, صحيح len)
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
qemu_cfg_write_داخلا(فراغ *buf, صحيح e, صحيح len)
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
    إذا (file->سائيز > maxlen)
        أرجع -1;
    بنية qemu_romfile_s *qfile;
    qfile = container_جو(file, بنية qemu_romfile_s, file);
    إذا (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_داخلا(dst, qfile->select, file->سائيز);
    } وإلا {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->سائيز);
    }
    أرجع file->سائيز;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
صحيح
qemu_cfg_write_file_simple(فراغ *src, u16 ڪنجي, u32 وٿي_2, u32 len)
{
    إذا (وٿي_2 == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_داخلا(src, ڪنجي, len);
    } وإلا {
        qemu_cfg_select(ڪنجي);
        qemu_cfg_skip(وٿي_2);
        qemu_cfg_write(src, len);
    }
    أرجع len;
}

صحيح
qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 وٿي_2, u32 len)
{
    إذا ((وٿي_2 + len) > file->سائيز)
        أرجع -1;

    إذا (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        أرجع -1;
    }
    أرجع qemu_cfg_write_file_simple(src, qemu_get_romfile_ڪنجي(file),
                                      وٿي_2, len);
}

ساكن فراغ
qemu_romfile_جوڙيو(حرف *name, صحيح select, صحيح skip, صحيح سائيز)
{
    بنية qemu_romfile_s *qfile = malloc_tmp(حجم(*qfile));
    إذا (!qfile) {
        warn_noalloc();
        أرجع;
    }
    memset(qfile, 0, حجم(*qfile));
    strtcpy(qfile->file.name, name, حجم(qfile->file.name));
    qfile->file.سائيز = سائيز;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_جوڙيو(&qfile->file);
}

u16
qemu_get_romfile_ڪنجي(بنية romfile_s *file)
{
    بنية qemu_romfile_s *qfile;
    إذا (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        أرجع 0;
    }
    qfile = container_جو(file, بنية qemu_romfile_s, file);
    أرجع qfile->select;
}

ساكن صحيح rtc_present(فراغ)
{
    أرجع rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_ڳڻپ(فراغ)
{
    u16 smp_ڳڻپ = 0;
    إذا (qemu_cfg_enabled()) {
        qemu_cfg_read_داخلا(&smp_ڳڻپ, QEMU_CFG_NB_CPUS, حجم(smp_ڳڻپ));
    }
    إذا (rtc_present()) {
        u16 cmos_cpu_ڳڻپ = rtc_read(CMOS_BIOS_SMP_ڳڻپ) + 1;
        إذا (smp_ڳڻپ < cmos_cpu_ڳڻپ) {
            smp_ڳڻپ = cmos_cpu_ڳڻپ;
        }
    }
    أرجع smp_ڳڻپ;
}

بنية e820_reservation {
    u64 پتو;
    u64 ڊيگهائي;
    u32 type;
};

#وصف_ڪريو SMBIOS_ميدان_داخلا 0
#وصف_ڪريو SMBIOS_جدول_داخلا 1

بنية qemu_smbios_سرو {
    u16 ڊيگهائي;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ساكن فراغ
qemu_cfg_e820(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    إذا (romfile_ڳوليو("etc/e820")) {
        // qemu_early_e820() has handled everything
        أرجع;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_داخلا(&count32, QEMU_CFG_E820_جدول, حجم(count32));
    إذا (count32) {
        بنية e820_reservation داخلا;
        صحيح i;
        لكل (i = 0; i < count32; i++) {
            qemu_cfg_read(&داخلا, حجم(داخلا));
            e820_جوڙيو(داخلا.پتو, داخلا.ڊيگهائي, داخلا.type);
        }
    } وإلا إذا (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_جوڙيو(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 مٿاهون = ((rtc_read(CMOS_MEM_HIGHMEM_هيٺيون) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_مٿاهون) << 32));
    Ram_سائيز_Over4_G = مٿاهون;
    e820_جوڙيو(0x100000000ull, مٿاهون, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_سائيز_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ساكن فراغ
qemu_cfg_legacy(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    // Misc config items.
    qemu_romfile_جوڙيو("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_جوڙيو("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_جوڙيو("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_داخلا(&numacount, QEMU_CFG_NUMA, حجم(numacount));
    صحيح max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_جوڙيو("etc/numa-cpu-map", QEMU_CFG_NUMA, حجم(numacount)
                     , max_cpu*حجم(u64));
    qemu_romfile_جوڙيو("etc/numa-nodes", QEMU_CFG_NUMA
                     , حجم(numacount) + max_cpu*حجم(u64)
                     , numacount*حجم(u64));

    // ACPI tables
    حرف name[128];
    u16 cnt;
    qemu_cfg_read_داخلا(&cnt, QEMU_CFG_ACPI_TABLES, حجم(cnt));
    صحيح i, وٿي_2 = حجم(cnt);
    لكل (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, حجم(len));
        وٿي_2 += حجم(len);
        snprintf(name, حجم(name), "acpi/table%d", i);
        qemu_romfile_جوڙيو(name, QEMU_CFG_ACPI_TABLES, وٿي_2, len);
        qemu_cfg_skip(len);
        وٿي_2 += len;
    }

    // SMBIOS info
    qemu_cfg_read_داخلا(&cnt, QEMU_CFG_SMBIOS_ENTRIES, حجم(cnt));
    وٿي_2 = حجم(cnt);
    لكل (i = 0; i < cnt; i++) {
        بنية qemu_smbios_سرو سرو;
        qemu_cfg_read(&سرو, حجم(سرو));
        إذا (سرو.headertype == SMBIOS_ميدان_داخلا) {
            snprintf(name, حجم(name), "smbios/field%d-%d"
                     , سرو.tabletype, سرو.fieldoffset);
            qemu_romfile_جوڙيو(name, QEMU_CFG_SMBIOS_ENTRIES
                             , وٿي_2 + حجم(سرو)
                             , سرو.ڊيگهائي - حجم(سرو));
        } وإلا {
            snprintf(name, حجم(name), "smbios/table%d-%d"
                     , سرو.tabletype, i);
            qemu_romfile_جوڙيو(name, QEMU_CFG_SMBIOS_ENTRIES
                             , وٿي_2 + 3, سرو.ڊيگهائي - 3);
        }
        qemu_cfg_skip(سرو.ڊيگهائي - حجم(سرو));
        وٿي_2 += سرو.ڊيگهائي;
    }
}

بنية QemuCfgFile {
    u32  سائيز;        /* file size */
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
        إذا (inb(پورٽ_QEMU_CFG_معلومات) != sig[i])
            أرجع 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_داخلا(&id, QEMU_CFG_ID, حجم(id));

    إذا (id & QEMU_CFG_نسخو_DMA) {
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
    u32 ڳڻپ;
    qemu_cfg_read_داخلا(&ڳڻپ, QEMU_CFG_FILE_DIR, حجم(ڳڻپ));
    ڳڻپ = be32_منزل_cpu(ڳڻپ);
    u32 e;
    لكل (e = 0; e < ڳڻپ; e++) {
        بنية QemuCfgFile qfile;
        qemu_cfg_read(&qfile, حجم(qfile));
        qemu_romfile_جوڙيو(qfile.name, be16_منزل_cpu(qfile.select)
                         , 0, be32_منزل_cpu(qfile.سائيز));
    }

    qemu_cfg_e820();

    إذا (romfile_ڳوليو("etc/table-loader")) {
        acpi_pm_بنياد = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_بنياد);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_داخلا(&nogfx, QEMU_CFG_NOGRAPHIC, حجم(nogfx));
    إذا (nogfx && !romfile_ڳوليو("etc/sercon-port")
        && !romfile_ڳوليو("vgaroms/sgabios.bin"))
        const_romfile_جوڙيو_int("etc/sercon-port", پورٽ_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ساكن صحيح qemu_early_e820(فراغ)
{
    بنية e820_reservation جدول;
    بنية QemuCfgFile qfile;
    u32 select = 0, سائيز = 0;
    u32 ڳڻپ, i;

    إذا (!qemu_cfg_detect())
        أرجع 0;

    // find e820 table
    qemu_cfg_read_داخلا(&ڳڻپ, QEMU_CFG_FILE_DIR, حجم(ڳڻپ));
    ڳڻپ = be32_منزل_cpu(ڳڻپ);
    لكل (i = 0; i < ڳڻپ; i++) {
        qemu_cfg_read(&qfile, حجم(qfile));
        إذا (memcmp(qfile.name, "etc/e820", 9) != 0)
            استمر;
        select = be16_منزل_cpu(qfile.select);
        سائيز = be32_منزل_cpu(qfile.سائيز);
        اكسر;
    }
    إذا (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        أرجع 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    ڳڻپ = سائيز/حجم(جدول);
    لكل (i = 0, select = 0; i < ڳڻپ; i++) {
        qemu_cfg_read(&جدول, حجم(جدول));
        بدل (جدول.type) {
        حالة E820_RESERVED:
            e820_جوڙيو(جدول.پتو, جدول.ڊيگهائي, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    جدول.پتو, جدول.ڊيگهائي);
            اكسر;
        حالة E820_SOFT_RESERVED:
            e820_جوڙيو(جدول.پتو, جدول.ڊيگهائي, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    جدول.پتو, جدول.ڊيگهائي);
            اكسر;
        حالة E820_RAM:
            e820_جوڙيو(جدول.پتو, جدول.ڊيگهائي, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    جدول.پتو, جدول.ڊيگهائي);
            إذا (جدول.پتو < 0x100000000LL) {
                // below 4g
                إذا (Ram_سائيز < جدول.پتو + جدول.ڊيگهائي)
                    Ram_سائيز = جدول.پتو + جدول.ڊيگهائي;
            } وإلا {
                // above 4g
                إذا (Ram_سائيز_Over4_G < جدول.پتو + جدول.ڊيگهائي - 0x100000000LL)
                    Ram_سائيز_Over4_G = جدول.پتو + جدول.ڊيگهائي - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_سائيز);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_سائيز_Over4_G);
    أرجع 1;
}
