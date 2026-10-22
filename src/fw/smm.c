// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "dev-q35.h"
#شامل "dev-piix.h"
#شامل "hw/pci.h" // pci_config_writel
#شامل "hw/pcidevice.h" // pci_find_device
#شامل "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#شامل "hw/pci_regs.h" // PCI_DEVICE_ID
#شامل "output.h" // dprintf
#شامل "paravirt.h" // PORT_SMI_STATUS
#شامل "stacks.h" // HaveSmmCall32
#شامل "string.h" // memcpy
#شامل "util.h" // smm_setup
#شامل "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#تعریف_کرو SMM_REV_نقاب 0x0002ffff

#تعریف_کرو SMM_REV_I32  0x00020000
#تعریف_کرو SMM_REV_I64  0x00020064

ساخت smm_state {
    اتحاد {
        ساخت {
            u8 pad_000[0xf8];
            u32 smm_بنیاد;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        ساخت {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_بنیاد;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

ساخت smm_layout {
    ساخت smm_state backup1;
    ساخت smm_state backup2;
    u32 backup_a20;
    u8 ڈھیر[0x8000 - حجم(ساخت smm_state)*2 - حجم(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    ساخت smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
ساکن خالی
memcpy_nowarn(خالی *d, ثابت خالی *s, سائز_t len)
{
    asm("" : "+r"(d), "+r"(s));
    حافظہ_نقل(d, s, len);
}

خالی VISIBLE32FLAT
handle_smi(u16 cs)
{
    اگر (!CONFIG_USE_SMM)
        واپس;
    u8 cmd = inb(پورٹ_SMI_CMD);
    ساخت smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_نقاب;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    اگر (smm == (خالی*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        اگر (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_بنیاد = BUILD_SMM_ADDR;
        } ورنہ اگر (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_بنیاد = BUILD_SMM_ADDR;
        } ورنہ {
            warn_internalerror();
            واپس;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, پورٹ_SMI_حالت);

        اگر (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            ساخت smm_layout *newsmm = (خالی*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, حجم(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, حجم(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        واپس;
    }

    اگر (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        اگر (rev == SMM_REV_I32) {
            u32 regs[8];
            حافظہ_نقل(regs, &smm->cpu.i32.eax, حجم(regs));
            اگر (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                حافظہ_نقل(&smm->backup2, &smm->cpu, حجم(smm->backup2));
                حافظہ_نقل(&smm->cpu, &smm->backup1, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i32.eax, regs, حجم(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = سیٹ_a20(1);
            } ورنہ اگر (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                حافظہ_نقل(&smm->cpu, &smm->backup2, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i32.eax, regs, حجم(regs));
                اگر (!smm->backup_a20)
                    سیٹ_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } ورنہ اگر (rev == SMM_REV_I64) {
            u64 regs[8];
            حافظہ_نقل(regs, &smm->cpu.i64.rdi, حجم(regs));
            اگر ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                حافظہ_نقل(&smm->backup2, &smm->cpu, حجم(smm->backup2));
                حافظہ_نقل(&smm->cpu, &smm->backup1, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i64.rdi, regs, حجم(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = سیٹ_a20(1);
            } ورنہ اگر ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                حافظہ_نقل(&smm->cpu, &smm->backup2, حجم(smm->cpu));
                حافظہ_نقل(&smm->cpu.i64.rdi, regs, حجم(regs));
                اگر (!smm->backup_a20)
                    سیٹ_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

خارجی خالی اندراج_smi(خالی);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#تعریف_کرو SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)اندراج_smi - BUILD_BIOS_ADDR) << 24))

ساکن خالی
smm_save_and_copy(خالی)
{
    // save original memory content
    ساخت smm_layout *initsmm = (خالی*)BUILD_SMM_INIT_ADDR;
    ساخت smm_layout *smm = (خالی*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, حجم(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, حجم(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

ساکن خالی
smm_relocate_and_restore(خالی)
{
    /* init APM status port */
    outb(0x01, پورٹ_SMI_حالت);

    /* raise an SMI interrupt */
    outb(0x00, پورٹ_SMI_CMD);

    /* wait until SMM code executed */
    جب (inb(پورٹ_SMI_حالت) != 0x00)
        ;

    /* restore original memory content */
    ساخت smm_layout *initsmm = (خالی*)BUILD_SMM_INIT_ADDR;
    ساخت smm_layout *smm = (خالی*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, حجم(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , حجم(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
ساکن خالی piix4_apmc_smm_تیاری(صحیح isabdf, صحیح i440_bdf)
{
    /* check if SMM init is already done */
    u32 قدر = pci_config_readl(isabdf, PIIX_DEVACTB);
    اگر (قدر & PIIX_DEVACTB_APMC_EN)
        واپس;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, قدر | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    قدر = inl(acpi_pm_بنیاد + PIIX_PMIO_GLBCTL);
    outl(قدر | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_بنیاد + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
خالی ich9_lpc_apmc_smm_تیاری(صحیح isabdf, صحیح mch_bdf)
{
    /* check if SMM init is already done */
    u32 قدر = inl(acpi_pm_بنیاد + ICH9_PMIO_SMI_EN);
    اگر (قدر & ICH9_PMIO_SMI_EN_APMC_EN)
        واپس;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(قدر | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_بنیاد + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    قدر = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      قدر | ICH9_LPC_GEN_PMCON_1_SMI_تالا);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

ساکن صحیح SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

خالی
smm_device_تیاری(خالی)
{
    اگر (!CONFIG_USE_SMM)
        واپس;

    ساخت pci_device *isapci, *pmpci;
    isapci = pci_ڈھونڈو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_ڈھونڈو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    اگر (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        واپس;
    }
    isapci = pci_ڈھونڈو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_ڈھونڈو_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    اگر (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

خالی
smm_تیاری(خالی)
{
    اگر (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        واپس;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    اگر (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_تیاری(SMMISADeviceBDF, SMMPMDeviceBDF);
    ورنہ
        ich9_lpc_apmc_smm_تیاری(SMMISADeviceBDF, SMMPMDeviceBDF);
}
