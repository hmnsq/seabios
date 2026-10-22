// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "dev-q35.h"
#ಅನ್ತರ್ಭಾವ "dev-piix.h"
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_writel
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_find_device
#ಅನ್ತರ್ಭಾವ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_DEVICE_ID
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // PORT_SMI_STATUS
#ಅನ್ತರ್ಭಾವ "stacks.h" // HaveSmmCall32
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "util.h" // smm_setup
#ಅನ್ತರ್ಭಾವ "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು SMM_REV_ಮುಖವಾಡ 0x0002ffff

#ವ್ಯಾಖ್ಯಾನಿಸು SMM_REV_I32  0x00020000
#ವ್ಯಾಖ್ಯಾನಿಸು SMM_REV_I64  0x00020064

ರಚನೆ smm_state {
    ಒಕ್ಕೂಟ {
        ರಚನೆ {
            u8 pad_000[0xf8];
            u32 smm_ತಳ;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        ರಚನೆ {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_ತಳ;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

ರಚನೆ smm_layout {
    ರಚನೆ smm_state backup1;
    ರಚನೆ smm_state backup2;
    u32 backup_a20;
    u8 ರಾಶಿ[0x8000 - ಗಾತ್ರ(ರಚನೆ smm_state)*2 - ಗಾತ್ರ(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    ರಚನೆ smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
ಸ್ಥಿರ ಶೂನ್ಯ
memcpy_nowarn(ಶೂನ್ಯ *d, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s, ಗಾತ್ರ_t len)
{
    asm("" : "+r"(d), "+r"(s));
    ಸ್ಮರಣೆ_ನಕಲು(d, s, len);
}

ಶೂನ್ಯ VISIBLE32FLAT
handle_smi(u16 cs)
{
    ಆದರೆ (!CONFIG_USE_SMM)
        ಹಿಂತಿರುಗಿ;
    u8 cmd = inb(ಪೋರ್ಟ್_SMI_CMD);
    ರಚನೆ smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_ಮುಖವಾಡ;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    ಆದರೆ (smm == (ಶೂನ್ಯ*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        ಆದರೆ (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_ತಳ = BUILD_SMM_ADDR;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_ತಳ = BUILD_SMM_ADDR;
        } ಇಲ್ಲದಿದ್ದರೆ {
            warn_internalerror();
            ಹಿಂತಿರುಗಿ;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, ಪೋರ್ಟ್_SMI_ಸ್ಥಿತಿ);

        ಆದರೆ (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            ರಚನೆ smm_layout *newsmm = (ಶೂನ್ಯ*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, ಗಾತ್ರ(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, ಗಾತ್ರ(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        ಆದರೆ (rev == SMM_REV_I32) {
            u32 regs[8];
            ಸ್ಮರಣೆ_ನಕಲು(regs, &smm->cpu.i32.eax, ಗಾತ್ರ(regs));
            ಆದರೆ (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                ಸ್ಮರಣೆ_ನಕಲು(&smm->backup2, &smm->cpu, ಗಾತ್ರ(smm->backup2));
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu, &smm->backup1, ಗಾತ್ರ(smm->cpu));
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu.i32.eax, regs, ಗಾತ್ರ(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = ಗಣ_a20(1);
            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu, &smm->backup2, ಗಾತ್ರ(smm->cpu));
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu.i32.eax, regs, ಗಾತ್ರ(regs));
                ಆದರೆ (!smm->backup_a20)
                    ಗಣ_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rev == SMM_REV_I64) {
            u64 regs[8];
            ಸ್ಮರಣೆ_ನಕಲು(regs, &smm->cpu.i64.rdi, ಗಾತ್ರ(regs));
            ಆದರೆ ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                ಸ್ಮರಣೆ_ನಕಲು(&smm->backup2, &smm->cpu, ಗಾತ್ರ(smm->backup2));
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu, &smm->backup1, ಗಾತ್ರ(smm->cpu));
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu.i64.rdi, regs, ಗಾತ್ರ(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = ಗಣ_a20(1);
            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu, &smm->backup2, ಗಾತ್ರ(smm->cpu));
                ಸ್ಮರಣೆ_ನಕಲು(&smm->cpu.i64.rdi, regs, ಗಾತ್ರ(regs));
                ಆದರೆ (!smm->backup_a20)
                    ಗಣ_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_smi(ಶೂನ್ಯ);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#ವ್ಯಾಖ್ಯಾನಿಸು SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)ನಮೂದು_smi - BUILD_BIOS_ADDR) << 24))

ಸ್ಥಿರ ಶೂನ್ಯ
smm_save_and_copy(ಶೂನ್ಯ)
{
    // save original memory content
    ರಚನೆ smm_layout *initsmm = (ಶೂನ್ಯ*)BUILD_SMM_INIT_ADDR;
    ರಚನೆ smm_layout *smm = (ಶೂನ್ಯ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, ಗಾತ್ರ(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, ಗಾತ್ರ(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

ಸ್ಥಿರ ಶೂನ್ಯ
smm_relocate_and_restore(ಶೂನ್ಯ)
{
    /* init APM status port */
    outb(0x01, ಪೋರ್ಟ್_SMI_ಸ್ಥಿತಿ);

    /* raise an SMI interrupt */
    outb(0x00, ಪೋರ್ಟ್_SMI_CMD);

    /* wait until SMM code executed */
    ಆಗ (inb(ಪೋರ್ಟ್_SMI_ಸ್ಥಿತಿ) != 0x00)
        ;

    /* restore original memory content */
    ರಚನೆ smm_layout *initsmm = (ಶೂನ್ಯ*)BUILD_SMM_INIT_ADDR;
    ರಚನೆ smm_layout *smm = (ಶೂನ್ಯ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, ಗಾತ್ರ(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , ಗಾತ್ರ(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
ಸ್ಥಿರ ಶೂನ್ಯ piix4_apmc_smm_ಸಿದ್ಧತೆ(ಪೂರ್ಣಾಂಕ isabdf, ಪೂರ್ಣಾಂಕ i440_bdf)
{
    /* check if SMM init is already done */
    u32 ಮೌಲ್ಯ = pci_config_readl(isabdf, PIIX_DEVACTB);
    ಆದರೆ (ಮೌಲ್ಯ & PIIX_DEVACTB_APMC_EN)
        ಹಿಂತಿರುಗಿ;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, ಮೌಲ್ಯ | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    ಮೌಲ್ಯ = inl(acpi_pm_ತಳ + PIIX_PMIO_GLBCTL);
    outl(ಮೌಲ್ಯ | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_ತಳ + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ಶೂನ್ಯ ich9_lpc_apmc_smm_ಸಿದ್ಧತೆ(ಪೂರ್ಣಾಂಕ isabdf, ಪೂರ್ಣಾಂಕ mch_bdf)
{
    /* check if SMM init is already done */
    u32 ಮೌಲ್ಯ = inl(acpi_pm_ತಳ + ICH9_PMIO_SMI_EN);
    ಆದರೆ (ಮೌಲ್ಯ & ICH9_PMIO_SMI_EN_APMC_EN)
        ಹಿಂತಿರುಗಿ;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(ಮೌಲ್ಯ | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_ತಳ + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    ಮೌಲ್ಯ = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      ಮೌಲ್ಯ | ICH9_LPC_GEN_PMCON_1_SMI_ಬೀಗ);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

ಶೂನ್ಯ
smm_device_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_USE_SMM)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ pci_device *isapci, *pmpci;
    isapci = pci_ಕಂಡುಹಿಡಿ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_ಕಂಡುಹಿಡಿ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    ಆದರೆ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        ಹಿಂತಿರುಗಿ;
    }
    isapci = pci_ಕಂಡುಹಿಡಿ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_ಕಂಡುಹಿಡಿ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    ಆದರೆ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

ಶೂನ್ಯ
smm_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    ಆದರೆ (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_ಸಿದ್ಧತೆ(SMMISADeviceBDF, SMMPMDeviceBDF);
    ಇಲ್ಲದಿದ್ದರೆ
        ich9_lpc_apmc_smm_ಸಿದ್ಧತೆ(SMMISADeviceBDF, SMMPMDeviceBDF);
}
