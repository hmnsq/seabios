// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "dev-q35.h"
#ضمن "dev-piix.h"
#ضمن "hw/pci.h" // pci_config_writel
#ضمن "hw/pcidevice.h" // pci_find_device
#ضمن "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ضمن "hw/pci_regs.h" // PCI_DEVICE_ID
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // PORT_SMI_STATUS
#ضمن "stacks.h" // HaveSmmCall32
#ضمن "string.h" // memcpy
#ضمن "util.h" // smm_setup
#ضمن "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#تعریف_کرو SMM_REV_نقاب 0x0002ffff

#تعریف_کرو SMM_REV_I32  0x00020000
#تعریف_کرو SMM_REV_I64  0x00020064

بنية smm_state {
    اتحاد {
        بنية {
            u8 pad_000[0xf8];
            u32 smm_ادھار;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        بنية {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_ادھار;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

بنية smm_layout {
    بنية smm_state backup1;
    بنية smm_state backup2;
    u32 backup_a20;
    u8 ڈھیر[0x8000 - حجم(بنية smm_state)*2 - حجم(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    بنية smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
ساكن فراغ
memcpy_nowarn(فراغ *d, ثابت فراغ *s, ناپ_t len)
{
    asm("" : "+r"(d), "+r"(s));
    حافظہ_نقل(d, s, len);
}

فراغ VISIBLE32FLAT
handle_smi(u16 cs)
{
    إذا (!CONFIG_USE_SMM)
        أرجع;
    u8 cmd = inb(پورٹ_SMI_CMD);
    بنية smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_نقاب;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    إذا (smm == (فراغ*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        إذا (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_ادھار = BUILD_SMM_ADDR;
        } وإلا إذا (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_ادھار = BUILD_SMM_ADDR;
        } وإلا {
            warn_internalerror();
            أرجع;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, پورٹ_SMI_حالت);

        إذا (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            بنية smm_layout *newsmm = (فراغ*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, حجم(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, حجم(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        أرجع;
    }

    إذا (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        إذا (rev == SMM_REV_I32) {
            u32 regs[8];
            حافظہ_نقل(regs, &smm->cpu.i32.eax, حجم(regs));
            إذا (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                حافظہ_نقل(&smm->backup2, &smm->cpu, حجم(smm->backup2));
                حافظہ_نقل(&smm->cpu, &smm->backup1, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i32.eax, regs, حجم(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = سیٹ_a20(1);
            } وإلا إذا (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                حافظہ_نقل(&smm->cpu, &smm->backup2, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i32.eax, regs, حجم(regs));
                إذا (!smm->backup_a20)
                    سیٹ_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } وإلا إذا (rev == SMM_REV_I64) {
            u64 regs[8];
            حافظہ_نقل(regs, &smm->cpu.i64.rdi, حجم(regs));
            إذا ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                حافظہ_نقل(&smm->backup2, &smm->cpu, حجم(smm->backup2));
                حافظہ_نقل(&smm->cpu, &smm->backup1, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i64.rdi, regs, حجم(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = سیٹ_a20(1);
            } وإلا إذا ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                حافظہ_نقل(&smm->cpu, &smm->backup2, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i64.rdi, regs, حجم(regs));
                إذا (!smm->backup_a20)
                    سیٹ_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

خارجي فراغ اندراج_smi(فراغ);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#تعریف_کرو SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)اندراج_smi - BUILD_BIOS_ADDR) << 24))

ساكن فراغ
smm_save_and_copy(فراغ)
{
    // save original memory content
    بنية smm_layout *initsmm = (فراغ*)BUILD_SMM_INIT_ADDR;
    بنية smm_layout *smm = (فراغ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, حجم(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, حجم(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

ساكن فراغ
smm_relocate_and_restore(فراغ)
{
    /* init APM status port */
    outb(0x01, پورٹ_SMI_حالت);

    /* raise an SMI interrupt */
    outb(0x00, پورٹ_SMI_CMD);

    /* wait until SMM code executed */
    طالما (inb(پورٹ_SMI_حالت) != 0x00)
        ;

    /* restore original memory content */
    بنية smm_layout *initsmm = (فراغ*)BUILD_SMM_INIT_ADDR;
    بنية smm_layout *smm = (فراغ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, حجم(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , حجم(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
ساكن فراغ piix4_apmc_smm_تیاری(صحيح isabdf, صحيح i440_bdf)
{
    /* check if SMM init is already done */
    u32 قدر = pci_config_readl(isabdf, PIIX_DEVACTB);
    إذا (قدر & PIIX_DEVACTB_APMC_EN)
        أرجع;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, قدر | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    قدر = inl(acpi_pm_ادھار + PIIX_PMIO_GLBCTL);
    outl(قدر | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_ادھار + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
فراغ ich9_lpc_apmc_smm_تیاری(صحيح isabdf, صحيح mch_bdf)
{
    /* check if SMM init is already done */
    u32 قدر = inl(acpi_pm_ادھار + ICH9_PMIO_SMI_EN);
    إذا (قدر & ICH9_PMIO_SMI_EN_APMC_EN)
        أرجع;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(قدر | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_ادھار + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    قدر = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      قدر | ICH9_LPC_GEN_PMCON_1_SMI_جندرا);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

ساكن صحيح SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

فراغ
smm_device_تیاری(فراغ)
{
    إذا (!CONFIG_USE_SMM)
        أرجع;

    بنية pci_device *isapci, *pmpci;
    isapci = pci_لبھو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_لبھو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    إذا (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        أرجع;
    }
    isapci = pci_لبھو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_لبھو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    إذا (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

فراغ
smm_تیاری(فراغ)
{
    إذا (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        أرجع;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    إذا (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_تیاری(SMMISADeviceBDF, SMMPMDeviceBDF);
    وإلا
        ich9_lpc_apmc_smm_تیاری(SMMISADeviceBDF, SMMPMDeviceBDF);
}
