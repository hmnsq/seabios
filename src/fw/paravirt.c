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
u32 Ram_اندازه;
// Amount of continuous ram >4Gig
u64 Ram_اندازه_Over4_G;
// physical address space bits
u8 CPUPhys_بټونه;
// 64bit processor
u8 CPULong_ډېرپېښ;
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
#تعریف_کړه KVM_CPUID_SIGNATURE     0x40000000
ساكن بینښه صحيح kvm_cpuid_بنسټ = 0;

ساكن فراغ kvm_detect(فراغ)
{
    بینښه صحيح i, max = 0;
    بینښه صحيح eax, ebx, ecx, edx;
    حرف signature[13];

    لكل (i = KVM_CPUID_SIGNATURE;; i += 0x100) {
        eax = 0;
        cpuid(i, &eax, &ebx, &ecx, &edx);
        إذا (eax < i)
            اكسر;
        حافظه_نقل(signature + 0, &ebx, 4);
        حافظه_نقل(signature + 4, &ecx, 4);
        حافظه_نقل(signature + 8, &edx, 4);
        signature[12] = 0;
        dprintf(1, "cpuid 0x%x: eax %x, signature '%s'\n", i, eax, signature);
        إذا (strcmp(signature, "KVMKVMKVM") == 0) {
            kvm_cpuid_بنسټ = i;
            max = eax;
        }
    }

    إذا (kvm_cpuid_بنسټ) {
        dprintf(1, "Running on KVM\n");
        PlatformRunningOn |= PF_KVM;
        إذا (max >= kvm_cpuid_بنسټ + 0x10) {
            cpuid(kvm_cpuid_بنسټ + 0x10, &eax, &ebx, &ecx, &edx);
            dprintf(1, "kvm: have invtsc, freq %u kHz\n", eax);
            tsctimer_setfreq(eax, "invtsc");
        }
    }
}

#تعریف_کړه KVM_FEATURE_CLOCKSOURCE           0
#تعریف_کړه KVM_FEATURE_CLOCKSOURCE2          3

#تعریف_کړه MSR_KVM_SYSTEM_وخت            0x12
#تعریف_کړه MSR_KVM_SYSTEM_وخت_NEW  0x4b564d01

#تعریف_کړه PVCLOCK_TSC_STABLE_بټ     (1 << 0)

بنية pvclock_vcpu_وخت_info *kvmclock;

ساكن فراغ kvmclock_init(فراغ)
{
    بینښه صحيح eax, ebx, ecx, edx, msr;

    إذا (!runningOnKVM())
        أرجع;

    cpuid(kvm_cpuid_بنسټ + 0x01, &eax, &ebx, &ecx, &edx);
    إذا (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE2))
        msr = MSR_KVM_SYSTEM_وخت_NEW;
    وإلا إذا (eax & (1 <<  KVM_FEATURE_CLOCKSOURCE))
        msr = MSR_KVM_SYSTEM_وخت;
    وإلا
        أرجع;

    kvmclock = memalign_ټیټ(حجم(*kvmclock), 32);
    memset(kvmclock, 0, حجم(*kvmclock));
    u32 ارزښت = (u32)(kvmclock);
    dprintf(1, "kvmclock: at 0x%x (msr 0x%x)\n", ارزښت, msr);
    wrmsr(msr, ارزښت | 0x01);

    إذا (!(kvmclock->flags & PVCLOCK_TSC_STABLE_بټ))
        أرجع;
    u32 MHz = (1000 << 16) / (kvmclock->tsc_منزل_system_mul >> 16);
    إذا (kvmclock->tsc_ښويدنه < 0)
        MHz <<= -kvmclock->tsc_ښويدنه;
    وإلا
        MHz >>= kvmclock->tsc_ښويدنه;
    dprintf(1, "kvmclock: stable tsc, %d MHz\n", MHz);
    tsctimer_setfreq(MHz * 1000, "kvmclock");
}

ساكن فراغ physbits(صحيح qemu_quirk)
{
    بینښه صحيح max, eax, ebx, ecx, edx;
    بینښه صحيح physbits;
    حرف signature[13];
    صحيح pae = 0, valid = 0;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    حافظه_نقل(signature + 0, &ebx, 4);
    حافظه_نقل(signature + 4, &edx, 4);
    حافظه_نقل(signature + 8, &ecx, 4);
    signature[12] = 0;
    إذا (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        pae = (edx & (1 << 6));
    }

    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max = eax;

    إذا (max >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        CPULong_ډېرپېښ = !!(edx & (1 << 29));
    }

    إذا (pae && CPULong_ډېرپېښ && max >= 0x80000008) {
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
            __func__, signature, pae ? "yes" : "no", CPULong_ډېرپېښ ? "yes" : "no",
            physbits, valid ? "yes" : "no");

    إذا (valid)
        CPUPhys_بټونه = physbits;
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
        u32 rs = ((rtc_read(CMOS_MEM_EXTMEM2_ټیټ) << 16)
                  | (rtc_read(CMOS_MEM_EXTMEM2_لوړ) << 24));
        إذا (rs)
            rs += 16 * 1024 * 1024;
        وإلا
            rs = (((rtc_read(CMOS_MEM_EXTMEM_ټیټ) << 10)
                   | (rtc_read(CMOS_MEM_EXTMEM_لوړ) << 18))
                  + 1 * 1024 * 1024);
        Ram_اندازه = rs;
        e820_جمع_کړه(0, rs, E820_RAM);
        dprintf(1, "RamSize: 0x%08x [cmos]\n", Ram_اندازه);
    }

    /* reserve 256KB BIOS area at the end of 4 GB */
    e820_جمع_کړه(0xfffc0000, 256*1024, E820_RESERVED);
}

#تعریف_کړه MSR_IA32_FEATURE_CONTROL 0x0000003a

ساكن فراغ msr_feature_control_تیاری(فراغ)
{
    u64 feature_control_بټونه = romfile_loadint("etc/msr_feature_control", 0);
    إذا (feature_control_بټونه)
        wrmsr_smp(MSR_IA32_FEATURE_CONTROL, feature_control_بټونه);
}

فراغ
qemu_platform_تیاری(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    إذا (runningOnXen()) {
        pci_probe_devices();
        xen_hypercall_تیاری();
        xen_biostable_تیاری();
        أرجع;
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
    إذا (Max_شمېر_CPUs <= 255) {
        pirtable_تیاری();
        mptable_تیاری();
    }
    smbios_تیاری();

    إذا (CONFIG_FW_ROMFILE_بارول) {
        صحيح loader_err;

        dprintf(3, "load ACPI tables\n");

        loader_err = romfile_loader_execute("etc/table-loader");

        RsdpAddr = پیدا_کړه_acpi_rsdp();

        إذا (RsdpAddr) {
            acpi_dsdt_parse();
            virtio_mmio_تیاری_acpi();
            أرجع;
        }
        /* If present, loader should have installed an RSDP.
         * Not installed? We might still be able to continue
         * using the builtin RSDP.
         */
        إذا (!loader_err)
            warn_internalerror();
    }

    acpi_تیاری();
}


/****************************************************************
 * QEMU firmware config (fw_cfg) interface
 ****************************************************************/

// List of QEMU fw_cfg entries.  DO NOT ADD MORE.  (All new content
// should be passed via the fw_cfg "file" interface.)
#تعریف_کړه QEMU_CFG_SIGNATURE              0x00
#تعریف_کړه QEMU_CFG_ID                     0x01
#تعریف_کړه QEMU_CFG_UUID                   0x02
#تعریف_کړه QEMU_CFG_NOGRAPHIC              0x04
#تعریف_کړه QEMU_CFG_NUMA                   0x0d
#تعریف_کړه QEMU_CFG_BOOT_MENU              0x0e
#تعریف_کړه QEMU_CFG_NB_CPUS                0x05
#تعریف_کړه QEMU_CFG_MAX_CPUS               0x0f
#تعریف_کړه QEMU_CFG_FILE_DIR               0x19
#تعریف_کړه QEMU_CFG_ARCH_LOCAL             0x8000
#تعریف_کړه QEMU_CFG_ACPI_TABLES            (QEMU_CFG_ARCH_LOCAL + 0)
#تعریف_کړه QEMU_CFG_SMBIOS_ENTRIES         (QEMU_CFG_ARCH_LOCAL + 1)
#تعریف_کړه QEMU_CFG_IRQ0_OVERRIDE          (QEMU_CFG_ARCH_LOCAL + 2)
#تعریف_کړه QEMU_CFG_E820_جدول             (QEMU_CFG_ARCH_LOCAL + 3)

ساكن فراغ
qemu_cfg_select(u16 f)
{
    outw(f, پورټ_QEMU_CFG_CTL);
}

ساكن فراغ
qemu_cfg_dma_transfer(فراغ *پته, u32 اوږدتیا, u32 control)
{
    QemuCfgDmaAccess access;

    access.پته = cpu_منزل_be64((u64)(u32)پته);
    access.اوږدتیا = cpu_منزل_be32(اوږدتیا);
    access.control = cpu_منزل_be32(control);

    barrier();

    outl(cpu_منزل_be32((u32)&access), پورټ_QEMU_CFG_DMA_ADDR_ټیټ);

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
        insb(پورټ_QEMU_CFG_معلومات, buf, len);
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
            inb(پورټ_QEMU_CFG_معلومات);
    }
}

ساكن فراغ
qemu_cfg_read_ثبت(فراغ *buf, صحيح e, صحيح len)
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
qemu_cfg_write_ثبت(فراغ *buf, صحيح e, صحيح len)
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
    إذا (file->اندازه > maxlen)
        أرجع -1;
    بنية qemu_romfile_s *qfile;
    qfile = container_د(file, بنية qemu_romfile_s, file);
    إذا (qfile->skip == 0) {
        /* Do it in one transfer */
        qemu_cfg_read_ثبت(dst, qfile->select, file->اندازه);
    } وإلا {
        qemu_cfg_select(qfile->select);
        qemu_cfg_skip(qfile->skip);
        qemu_cfg_read(dst, file->اندازه);
    }
    أرجع file->اندازه;
}

// Bare-bones function for writing a file knowing only its unique
// identifying key (select)
صحيح
qemu_cfg_write_file_simple(فراغ *src, u16 کلی, u32 دننه_واټن_2, u32 len)
{
    إذا (دننه_واټن_2 == 0) {
        /* Do it in one transfer */
        qemu_cfg_write_ثبت(src, کلی, len);
    } وإلا {
        qemu_cfg_select(کلی);
        qemu_cfg_skip(دننه_واټن_2);
        qemu_cfg_write(src, len);
    }
    أرجع len;
}

صحيح
qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 دننه_واټن_2, u32 len)
{
    إذا ((دننه_واټن_2 + len) > file->اندازه)
        أرجع -1;

    إذا (!qemu_cfg_dma_enabled() || (file->copy != qemu_cfg_read_file)) {
        warn_internalerror();
        أرجع -1;
    }
    أرجع qemu_cfg_write_file_simple(src, qemu_get_romfile_کلی(file),
                                      دننه_واټن_2, len);
}

ساكن فراغ
qemu_romfile_جمع_کړه(حرف *name, صحيح select, صحيح skip, صحيح اندازه)
{
    بنية qemu_romfile_s *qfile = malloc_tmp(حجم(*qfile));
    إذا (!qfile) {
        warn_noalloc();
        أرجع;
    }
    memset(qfile, 0, حجم(*qfile));
    strtcpy(qfile->file.name, name, حجم(qfile->file.name));
    qfile->file.اندازه = اندازه;
    qfile->select = select;
    qfile->skip = skip;
    qfile->file.copy = qemu_cfg_read_file;
    romfile_جمع_کړه(&qfile->file);
}

u16
qemu_get_romfile_کلی(بنية romfile_s *file)
{
    بنية qemu_romfile_s *qfile;
    إذا (file->copy != qemu_cfg_read_file) {
        warn_internalerror();
        أرجع 0;
    }
    qfile = container_د(file, بنية qemu_romfile_s, file);
    أرجع qfile->select;
}

ساكن صحيح rtc_present(فراغ)
{
    أرجع rtc_read(CMOS_RTC_MONTH) != 0xff;
}

u16
qemu_get_present_cpus_شمېر(فراغ)
{
    u16 smp_شمېر = 0;
    إذا (qemu_cfg_enabled()) {
        qemu_cfg_read_ثبت(&smp_شمېر, QEMU_CFG_NB_CPUS, حجم(smp_شمېر));
    }
    إذا (rtc_present()) {
        u16 cmos_cpu_شمېر = rtc_read(CMOS_BIOS_SMP_شمېر) + 1;
        إذا (smp_شمېر < cmos_cpu_شمېر) {
            smp_شمېر = cmos_cpu_شمېر;
        }
    }
    أرجع smp_شمېر;
}

بنية e820_reservation {
    u64 پته;
    u64 اوږدتیا;
    u32 type;
};

#تعریف_کړه SMBIOS_ډګر_ثبت 0
#تعریف_کړه SMBIOS_جدول_ثبت 1

بنية qemu_smbios_سرلیک {
    u16 اوږدتیا;
    u8 headertype;
    u8 tabletype;
    u16 fieldoffset;
} PACKED;

ساكن فراغ
qemu_cfg_e820(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    إذا (romfile_پیدا_کړه("etc/e820")) {
        // qemu_early_e820() has handled everything
        أرجع;
    }

    // QEMU_CFG_E820_TABLE has reservations only
    u32 count32;
    qemu_cfg_read_ثبت(&count32, QEMU_CFG_E820_جدول, حجم(count32));
    إذا (count32) {
        بنية e820_reservation ثبت;
        صحيح i;
        لكل (i = 0; i < count32; i++) {
            qemu_cfg_read(&ثبت, حجم(ثبت));
            e820_جمع_کړه(ثبت.پته, ثبت.اوږدتیا, ثبت.type);
        }
    } وإلا إذا (runningOnKVM()) {
        // Backwards compatibility - provide hard coded range.
        // 4 pages before the bios, 3 pages for vmx tss pages, the
        // other page for EPT real mode pagetable
        e820_جمع_کړه(0xfffbc000, 4*4096, E820_RESERVED);
    }

    // Check for memory over 4Gig in cmos
    u64 لوړ = ((rtc_read(CMOS_MEM_HIGHMEM_ټیټ) << 16)
                | ((u32)rtc_read(CMOS_MEM_HIGHMEM_MID) << 24)
                | ((u64)rtc_read(CMOS_MEM_HIGHMEM_لوړ) << 32));
    Ram_اندازه_Over4_G = لوړ;
    e820_جمع_کړه(0x100000000ull, لوړ, E820_RAM);
    dprintf(1, "RamSizeOver4G: 0x%016llx [cmos]\n", Ram_اندازه_Over4_G);
}

// Populate romfile entries for legacy fw_cfg ports (that predate the
// "file" interface).
ساكن فراغ
qemu_cfg_legacy(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    // Misc config items.
    qemu_romfile_جمع_کړه("etc/show-boot-menu", QEMU_CFG_BOOT_MENU, 0, 2);
    qemu_romfile_جمع_کړه("etc/irq0-override", QEMU_CFG_IRQ0_OVERRIDE, 0, 1);
    qemu_romfile_جمع_کړه("etc/max-cpus", QEMU_CFG_MAX_CPUS, 0, 2);

    // NUMA data
    u64 numacount;
    qemu_cfg_read_ثبت(&numacount, QEMU_CFG_NUMA, حجم(numacount));
    صحيح max_cpu = romfile_loadint("etc/max-cpus", 0);
    qemu_romfile_جمع_کړه("etc/numa-cpu-map", QEMU_CFG_NUMA, حجم(numacount)
                     , max_cpu*حجم(u64));
    qemu_romfile_جمع_کړه("etc/numa-nodes", QEMU_CFG_NUMA
                     , حجم(numacount) + max_cpu*حجم(u64)
                     , numacount*حجم(u64));

    // ACPI tables
    حرف name[128];
    u16 cnt;
    qemu_cfg_read_ثبت(&cnt, QEMU_CFG_ACPI_TABLES, حجم(cnt));
    صحيح i, دننه_واټن_2 = حجم(cnt);
    لكل (i = 0; i < cnt; i++) {
        u16 len;
        qemu_cfg_read(&len, حجم(len));
        دننه_واټن_2 += حجم(len);
        snprintf(name, حجم(name), "acpi/table%d", i);
        qemu_romfile_جمع_کړه(name, QEMU_CFG_ACPI_TABLES, دننه_واټن_2, len);
        qemu_cfg_skip(len);
        دننه_واټن_2 += len;
    }

    // SMBIOS info
    qemu_cfg_read_ثبت(&cnt, QEMU_CFG_SMBIOS_ENTRIES, حجم(cnt));
    دننه_واټن_2 = حجم(cnt);
    لكل (i = 0; i < cnt; i++) {
        بنية qemu_smbios_سرلیک سرلیک;
        qemu_cfg_read(&سرلیک, حجم(سرلیک));
        إذا (سرلیک.headertype == SMBIOS_ډګر_ثبت) {
            snprintf(name, حجم(name), "smbios/field%d-%d"
                     , سرلیک.tabletype, سرلیک.fieldoffset);
            qemu_romfile_جمع_کړه(name, QEMU_CFG_SMBIOS_ENTRIES
                             , دننه_واټن_2 + حجم(سرلیک)
                             , سرلیک.اوږدتیا - حجم(سرلیک));
        } وإلا {
            snprintf(name, حجم(name), "smbios/table%d-%d"
                     , سرلیک.tabletype, i);
            qemu_romfile_جمع_کړه(name, QEMU_CFG_SMBIOS_ENTRIES
                             , دننه_واټن_2 + 3, سرلیک.اوږدتیا - 3);
        }
        qemu_cfg_skip(سرلیک.اوږدتیا - حجم(سرلیک));
        دننه_واټن_2 += سرلیک.اوږدتیا;
    }
}

بنية QemuCfgFile {
    u32  اندازه;        /* file size */
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
        إذا (inb(پورټ_QEMU_CFG_معلومات) != sig[i])
            أرجع 0;

    dprintf(1, "Found QEMU fw_cfg\n");
    cfg_enabled = 1;

    // Detect DMA interface.
    u32 id;
    qemu_cfg_read_ثبت(&id, QEMU_CFG_ID, حجم(id));

    إذا (id & QEMU_CFG_نسخه_DMA) {
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
    u32 شمېر;
    qemu_cfg_read_ثبت(&شمېر, QEMU_CFG_FILE_DIR, حجم(شمېر));
    شمېر = be32_منزل_cpu(شمېر);
    u32 e;
    لكل (e = 0; e < شمېر; e++) {
        بنية QemuCfgFile qfile;
        qemu_cfg_read(&qfile, حجم(qfile));
        qemu_romfile_جمع_کړه(qfile.name, be16_منزل_cpu(qfile.select)
                         , 0, be32_منزل_cpu(qfile.اندازه));
    }

    qemu_cfg_e820();

    إذا (romfile_پیدا_کړه("etc/table-loader")) {
        acpi_pm_بنسټ = 0x0600;
        dprintf(1, "Moving pm_base to 0x%x\n", acpi_pm_بنسټ);
    }

    // serial console
    u16 nogfx = 0;
    qemu_cfg_read_ثبت(&nogfx, QEMU_CFG_NOGRAPHIC, حجم(nogfx));
    إذا (nogfx && !romfile_پیدا_کړه("etc/sercon-port")
        && !romfile_پیدا_کړه("vgaroms/sgabios.bin"))
        const_romfile_جمع_کړه_int("etc/sercon-port", پورټ_SERIAL1);
}

/*
 * This runs before malloc and romfile are ready, so we have to work
 * with stack allocations and read from fw_cfg in chunks.
 */
ساكن صحيح qemu_early_e820(فراغ)
{
    بنية e820_reservation جدول;
    بنية QemuCfgFile qfile;
    u32 select = 0, اندازه = 0;
    u32 شمېر, i;

    إذا (!qemu_cfg_detect())
        أرجع 0;

    // find e820 table
    qemu_cfg_read_ثبت(&شمېر, QEMU_CFG_FILE_DIR, حجم(شمېر));
    شمېر = be32_منزل_cpu(شمېر);
    لكل (i = 0; i < شمېر; i++) {
        qemu_cfg_read(&qfile, حجم(qfile));
        إذا (memcmp(qfile.name, "etc/e820", 9) != 0)
            استمر;
        select = be16_منزل_cpu(qfile.select);
        اندازه = be32_منزل_cpu(qfile.اندازه);
        اكسر;
    }
    إذا (select == 0) {
        // may happen on old qemu
        dprintf(1, "qemu/e820: fw_cfg file etc/e820 not found\n");
        أرجع 0;
    }

    // walk e820 table
    qemu_cfg_select(select);
    شمېر = اندازه/حجم(جدول);
    لكل (i = 0, select = 0; i < شمېر; i++) {
        qemu_cfg_read(&جدول, حجم(جدول));
        بدل (جدول.type) {
        حالة E820_RESERVED:
            e820_جمع_کړه(جدول.پته, جدول.اوږدتیا, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [reserved]\n",
                    جدول.پته, جدول.اوږدتیا);
            اكسر;
        حالة E820_SOFT_RESERVED:
            e820_جمع_کړه(جدول.پته, جدول.اوږدتیا, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [soft-reserved]\n",
                    جدول.پته, جدول.اوږدتیا);
            اكسر;
        حالة E820_RAM:
            e820_جمع_کړه(جدول.پته, جدول.اوږدتیا, جدول.type);
            dprintf(1, "qemu/e820: addr 0x%016llx len 0x%016llx [RAM]\n",
                    جدول.پته, جدول.اوږدتیا);
            إذا (جدول.پته < 0x100000000LL) {
                // below 4g
                إذا (Ram_اندازه < جدول.پته + جدول.اوږدتیا)
                    Ram_اندازه = جدول.پته + جدول.اوږدتیا;
            } وإلا {
                // above 4g
                إذا (Ram_اندازه_Over4_G < جدول.پته + جدول.اوږدتیا - 0x100000000LL)
                    Ram_اندازه_Over4_G = جدول.پته + جدول.اوږدتیا - 0x100000000LL;
            }
        }
    }

    dprintf(3, "qemu/e820: RamSize: 0x%08x\n", Ram_اندازه);
    dprintf(3, "qemu/e820: RamSizeOver4G: 0x%016llx\n", Ram_اندازه_Over4_G);
    أرجع 1;
}
