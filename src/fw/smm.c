// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "dev-q35.h"
#అన్తర్భావ "dev-piix.h"
#అన్తర్భావ "hw/pci.h" // pci_config_writel
#అన్తర్భావ "hw/pcidevice.h" // pci_find_device
#అన్తర్భావ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#అన్తర్భావ "hw/pci_regs.h" // PCI_DEVICE_ID
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // PORT_SMI_STATUS
#అన్తర్భావ "stacks.h" // HaveSmmCall32
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "util.h" // smm_setup
#అన్తర్భావ "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#నిర్వచించు SMM_REV_ముసుగు 0x0002ffff

#నిర్వచించు SMM_REV_I32  0x00020000
#నిర్వచించు SMM_REV_I64  0x00020064

నిర్మాణం smm_state {
    సంఘం {
        నిర్మాణం {
            u8 pad_000[0xf8];
            u32 smm_పునాది;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        నిర్మాణం {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_పునాది;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

నిర్మాణం smm_layout {
    నిర్మాణం smm_state backup1;
    నిర్మాణం smm_state backup2;
    u32 backup_a20;
    u8 దొంతర[0x8000 - పరిమాణం(నిర్మాణం smm_state)*2 - పరిమాణం(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    నిర్మాణం smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
స్థిర శూన్య
memcpy_nowarn(శూన్య *d, స్థిరరాశి శూన్య *s, కొలత_t len)
{
    asm("" : "+r"(d), "+r"(s));
    జ్ఞాపకం_నకలు(d, s, len);
}

శూన్య VISIBLE32FLAT
handle_smi(u16 cs)
{
    ఒకవేళ (!CONFIG_USE_SMM)
        తిరిగి;
    u8 cmd = inb(పోర్టు_SMI_CMD);
    నిర్మాణం smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_ముసుగు;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    ఒకవేళ (smm == (శూన్య*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        ఒకవేళ (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_పునాది = BUILD_SMM_ADDR;
        } లేకపోతే ఒకవేళ (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_పునాది = BUILD_SMM_ADDR;
        } లేకపోతే {
            warn_internalerror();
            తిరిగి;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, పోర్టు_SMI_స్థితి);

        ఒకవేళ (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            నిర్మాణం smm_layout *newsmm = (శూన్య*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, పరిమాణం(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, పరిమాణం(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        తిరిగి;
    }

    ఒకవేళ (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        ఒకవేళ (rev == SMM_REV_I32) {
            u32 regs[8];
            జ్ఞాపకం_నకలు(regs, &smm->cpu.i32.eax, పరిమాణం(regs));
            ఒకవేళ (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                జ్ఞాపకం_నకలు(&smm->backup2, &smm->cpu, పరిమాణం(smm->backup2));
                జ్ఞాపకం_నకలు(&smm->cpu, &smm->backup1, పరిమాణం(smm->cpu));
                జ్ఞాపకం_నకలు(&smm->cpu.i32.eax, regs, పరిమాణం(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = సమితి_a20(1);
            } లేకపోతే ఒకవేళ (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                జ్ఞాపకం_నకలు(&smm->cpu, &smm->backup2, పరిమాణం(smm->cpu));
                జ్ఞాపకం_నకలు(&smm->cpu.i32.eax, regs, పరిమాణం(regs));
                ఒకవేళ (!smm->backup_a20)
                    సమితి_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } లేకపోతే ఒకవేళ (rev == SMM_REV_I64) {
            u64 regs[8];
            జ్ఞాపకం_నకలు(regs, &smm->cpu.i64.rdi, పరిమాణం(regs));
            ఒకవేళ ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                జ్ఞాపకం_నకలు(&smm->backup2, &smm->cpu, పరిమాణం(smm->backup2));
                జ్ఞాపకం_నకలు(&smm->cpu, &smm->backup1, పరిమాణం(smm->cpu));
                జ్ఞాపకం_నకలు(&smm->cpu.i64.rdi, regs, పరిమాణం(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = సమితి_a20(1);
            } లేకపోతే ఒకవేళ ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                జ్ఞాపకం_నకలు(&smm->cpu, &smm->backup2, పరిమాణం(smm->cpu));
                జ్ఞాపకం_నకలు(&smm->cpu.i64.rdi, regs, పరిమాణం(regs));
                ఒకవేళ (!smm->backup_a20)
                    సమితి_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

బాహ్య శూన్య నమోదు_smi(శూన్య);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#నిర్వచించు SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)నమోదు_smi - BUILD_BIOS_ADDR) << 24))

స్థిర శూన్య
smm_save_and_copy(శూన్య)
{
    // save original memory content
    నిర్మాణం smm_layout *initsmm = (శూన్య*)BUILD_SMM_INIT_ADDR;
    నిర్మాణం smm_layout *smm = (శూన్య*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, పరిమాణం(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, పరిమాణం(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

స్థిర శూన్య
smm_relocate_and_restore(శూన్య)
{
    /* init APM status port */
    outb(0x01, పోర్టు_SMI_స్థితి);

    /* raise an SMI interrupt */
    outb(0x00, పోర్టు_SMI_CMD);

    /* wait until SMM code executed */
    అయితే (inb(పోర్టు_SMI_స్థితి) != 0x00)
        ;

    /* restore original memory content */
    నిర్మాణం smm_layout *initsmm = (శూన్య*)BUILD_SMM_INIT_ADDR;
    నిర్మాణం smm_layout *smm = (శూన్య*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, పరిమాణం(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , పరిమాణం(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
స్థిర శూన్య piix4_apmc_smm_అమరిక(పూర్ణాంకం isabdf, పూర్ణాంకం i440_bdf)
{
    /* check if SMM init is already done */
    u32 విలువ = pci_config_readl(isabdf, PIIX_DEVACTB);
    ఒకవేళ (విలువ & PIIX_DEVACTB_APMC_EN)
        తిరిగి;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, విలువ | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    విలువ = inl(acpi_pm_పునాది + PIIX_PMIO_GLBCTL);
    outl(విలువ | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_పునాది + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
శూన్య ich9_lpc_apmc_smm_అమరిక(పూర్ణాంకం isabdf, పూర్ణాంకం mch_bdf)
{
    /* check if SMM init is already done */
    u32 విలువ = inl(acpi_pm_పునాది + ICH9_PMIO_SMI_EN);
    ఒకవేళ (విలువ & ICH9_PMIO_SMI_EN_APMC_EN)
        తిరిగి;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(విలువ | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_పునాది + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    విలువ = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      విలువ | ICH9_LPC_GEN_PMCON_1_SMI_తాళం);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

స్థిర పూర్ణాంకం SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

శూన్య
smm_device_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_USE_SMM)
        తిరిగి;

    నిర్మాణం pci_device *isapci, *pmpci;
    isapci = pci_కనుగొను_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_కనుగొను_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    ఒకవేళ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        తిరిగి;
    }
    isapci = pci_కనుగొను_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_కనుగొను_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    ఒకవేళ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

శూన్య
smm_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        తిరిగి;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    ఒకవేళ (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_అమరిక(SMMISADeviceBDF, SMMPMDeviceBDF);
    లేకపోతే
        ich9_lpc_apmc_smm_అమరిక(SMMISADeviceBDF, SMMPMDeviceBDF);
}
