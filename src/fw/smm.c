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
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

ساختار smm_state {
    اجتماع {
        ساختار {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        ساختار {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

ساختار smm_layout {
    ساختار smm_state backup1;
    ساختار smm_state backup2;
    u32 backup_a20;
    u8 پشته[0x8000 - اندازه(ساختار smm_state)*2 - اندازه(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    ساختار smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
ایستا تهی
memcpy_nowarn(تهی *d, ثابت تهی *s, size_t len)
{
    asm("" : "+r"(d), "+r"(s));
    memcpy(d, s, len);
}

تهی VISIBLE32FLAT
handle_smi(u16 cs)
{
    اگر (!CONFIG_USE_SMM)
        بازگشت;
    u8 cmd = inb(PORT_SMI_CMD);
    ساختار smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    اگر (smm == (تهی*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        اگر (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } وگرنه اگر (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } وگرنه {
            warn_internalerror();
            بازگشت;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        اگر (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            ساختار smm_layout *newsmm = (تهی*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, اندازه(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, اندازه(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        بازگشت;
    }

    اگر (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        اگر (rev == SMM_REV_I32) {
            u32 regs[8];
            memcpy(regs, &smm->cpu.i32.eax, اندازه(regs));
            اگر (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->backup2, &smm->cpu, اندازه(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, اندازه(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, اندازه(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } وگرنه اگر (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->cpu, &smm->backup2, اندازه(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, اندازه(regs));
                اگر (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } وگرنه اگر (rev == SMM_REV_I64) {
            u64 regs[8];
            memcpy(regs, &smm->cpu.i64.rdi, اندازه(regs));
            اگر ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                memcpy(&smm->backup2, &smm->cpu, اندازه(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, اندازه(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, اندازه(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } وگرنه اگر ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                memcpy(&smm->cpu, &smm->backup2, اندازه(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, اندازه(regs));
                اگر (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

بیرونی تهی entry_smi(تهی);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

ایستا تهی
smm_save_and_copy(تهی)
{
    // save original memory content
    ساختار smm_layout *initsmm = (تهی*)BUILD_SMM_INIT_ADDR;
    ساختار smm_layout *smm = (تهی*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, اندازه(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, اندازه(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

ایستا تهی
smm_relocate_and_restore(تهی)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    وقتی (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    ساختار smm_layout *initsmm = (تهی*)BUILD_SMM_INIT_ADDR;
    ساختار smm_layout *smm = (تهی*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, اندازه(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , اندازه(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
ایستا تهی piix4_apmc_smm_setup(صحیح isabdf, صحیح i440_bdf)
{
    /* check if SMM init is already done */
    u32 مقدار = pci_config_readl(isabdf, PIIX_DEVACTB);
    اگر (مقدار & PIIX_DEVACTB_APMC_EN)
        بازگشت;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, مقدار | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    مقدار = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(مقدار | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
تهی ich9_lpc_apmc_smm_setup(صحیح isabdf, صحیح mch_bdf)
{
    /* check if SMM init is already done */
    u32 مقدار = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    اگر (مقدار & ICH9_PMIO_SMI_EN_APMC_EN)
        بازگشت;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(مقدار | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    مقدار = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      مقدار | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

ایستا صحیح SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

تهی
smm_device_setup(تهی)
{
    اگر (!CONFIG_USE_SMM)
        بازگشت;

    ساختار pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    اگر (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        بازگشت;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    اگر (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

تهی
smm_setup(تهی)
{
    اگر (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        بازگشت;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    اگر (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    وگرنه
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
