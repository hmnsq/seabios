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
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

ಸಂರಚನಾ smm_state {
    ಸಙ್ಘ {
        ಸಂರಚನಾ {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        ಸಂರಚನಾ {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

ಸಂರಚನಾ smm_layout {
    ಸಂರಚನಾ smm_state backup1;
    ಸಂರಚನಾ smm_state backup2;
    u32 backup_a20;
    u8 ರಾಶಿ[0x8000 - ಪರಿಮಾಣ(ಸಂರಚನಾ smm_state)*2 - ಪರಿಮಾಣ(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    ಸಂರಚನಾ smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
ಸ್ಥಿರ ಶೂನ್ಯ
memcpy_nowarn(ಶೂನ್ಯ *d, ಧ್ರುವ ಶೂನ್ಯ *s, size_t len)
{
    asm("" : "+r"(d), "+r"(s));
    memcpy(d, s, len);
}

ಶೂನ್ಯ VISIBLE32FLAT
handle_smi(u16 cs)
{
    ಯದಿ (!CONFIG_USE_SMM)
        ಪ್ರತ್ಯಾಗಮ;
    u8 cmd = inb(PORT_SMI_CMD);
    ಸಂರಚನಾ smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    ಯದಿ (smm == (ಶೂನ್ಯ*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        ಯದಿ (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } ಅನ್ಯಥಾ ಯದಿ (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } ಅನ್ಯಥಾ {
            warn_internalerror();
            ಪ್ರತ್ಯಾಗಮ;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        ಯದಿ (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            ಸಂರಚನಾ smm_layout *newsmm = (ಶೂನ್ಯ*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, ಪರಿಮಾಣ(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, ಪರಿಮಾಣ(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        ಯದಿ (rev == SMM_REV_I32) {
            u32 regs[8];
            memcpy(regs, &smm->cpu.i32.eax, ಪರಿಮಾಣ(regs));
            ಯದಿ (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->backup2, &smm->cpu, ಪರಿಮಾಣ(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, ಪರಿಮಾಣ(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, ಪರಿಮಾಣ(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } ಅನ್ಯಥಾ ಯದಿ (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->cpu, &smm->backup2, ಪರಿಮಾಣ(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, ಪರಿಮಾಣ(regs));
                ಯದಿ (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } ಅನ್ಯಥಾ ಯದಿ (rev == SMM_REV_I64) {
            u64 regs[8];
            memcpy(regs, &smm->cpu.i64.rdi, ಪರಿಮಾಣ(regs));
            ಯದಿ ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                memcpy(&smm->backup2, &smm->cpu, ಪರಿಮಾಣ(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, ಪರಿಮಾಣ(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, ಪರಿಮಾಣ(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } ಅನ್ಯಥಾ ಯದಿ ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                memcpy(&smm->cpu, &smm->backup2, ಪರಿಮಾಣ(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, ಪರಿಮಾಣ(regs));
                ಯದಿ (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

ಬಾಹ್ಯ ಶೂನ್ಯ entry_smi(ಶೂನ್ಯ);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

ಸ್ಥಿರ ಶೂನ್ಯ
smm_save_and_copy(ಶೂನ್ಯ)
{
    // save original memory content
    ಸಂರಚನಾ smm_layout *initsmm = (ಶೂನ್ಯ*)BUILD_SMM_INIT_ADDR;
    ಸಂರಚನಾ smm_layout *smm = (ಶೂನ್ಯ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, ಪರಿಮಾಣ(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, ಪರಿಮಾಣ(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

ಸ್ಥಿರ ಶೂನ್ಯ
smm_relocate_and_restore(ಶೂನ್ಯ)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    ಯಾವತ್ (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    ಸಂರಚನಾ smm_layout *initsmm = (ಶೂನ್ಯ*)BUILD_SMM_INIT_ADDR;
    ಸಂರಚನಾ smm_layout *smm = (ಶೂನ್ಯ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, ಪರಿಮಾಣ(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , ಪರಿಮಾಣ(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
ಸ್ಥಿರ ಶೂನ್ಯ piix4_apmc_smm_setup(ಪೂರ್ಣಾಙ್ಕ isabdf, ಪೂರ್ಣಾಙ್ಕ i440_bdf)
{
    /* check if SMM init is already done */
    u32 ಮೌಲ್ಯ = pci_config_readl(isabdf, PIIX_DEVACTB);
    ಯದಿ (ಮೌಲ್ಯ & PIIX_DEVACTB_APMC_EN)
        ಪ್ರತ್ಯಾಗಮ;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, ಮೌಲ್ಯ | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    ಮೌಲ್ಯ = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(ಮೌಲ್ಯ | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ಶೂನ್ಯ ich9_lpc_apmc_smm_setup(ಪೂರ್ಣಾಙ್ಕ isabdf, ಪೂರ್ಣಾಙ್ಕ mch_bdf)
{
    /* check if SMM init is already done */
    u32 ಮೌಲ್ಯ = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    ಯದಿ (ಮೌಲ್ಯ & ICH9_PMIO_SMI_EN_APMC_EN)
        ಪ್ರತ್ಯಾಗಮ;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(ಮೌಲ್ಯ | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    ಮೌಲ್ಯ = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      ಮೌಲ್ಯ | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

ಶೂನ್ಯ
smm_device_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_USE_SMM)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    ಯದಿ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        ಪ್ರತ್ಯಾಗಮ;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    ಯದಿ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

ಶೂನ್ಯ
smm_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    ಯದಿ (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    ಅನ್ಯಥಾ
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
