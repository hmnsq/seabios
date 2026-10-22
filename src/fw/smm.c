// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "dev-q35.h"
#অন্তর্ভাৱ "dev-piix.h"
#অন্তর্ভাৱ "hw/pci.h" // pci_config_writel
#অন্তর্ভাৱ "hw/pcidevice.h" // pci_find_device
#অন্তর্ভাৱ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#অন্তর্ভাৱ "hw/pci_regs.h" // PCI_DEVICE_ID
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "paravirt.h" // PORT_SMI_STATUS
#অন্তর্ভাৱ "stacks.h" // HaveSmmCall32
#অন্তর্ভাৱ "string.h" // memcpy
#অন্তর্ভাৱ "util.h" // smm_setup
#অন্তর্ভাৱ "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

সংরচনা smm_state {
    সঙ্ঘ {
        সংরচনা {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        সংরচনা {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

সংরচনা smm_layout {
    সংরচনা smm_state backup1;
    সংরচনা smm_state backup2;
    u32 backup_a20;
    u8 স্তূপ[0x8000 - পরিমাণ(সংরচনা smm_state)*2 - পরিমাণ(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    সংরচনা smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
স্থির শূন্য
memcpy_nowarn(শূন্য *d, ধ্রুৱ শূন্য *s, size_t len)
{
    asm("" : "+r"(d), "+r"(s));
    memcpy(d, s, len);
}

শূন্য VISIBLE32FLAT
handle_smi(u16 cs)
{
    যদি (!CONFIG_USE_SMM)
        ঘূৰাই;
    u8 cmd = inb(PORT_SMI_CMD);
    সংরচনা smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    যদি (smm == (শূন্য*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        যদি (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } অন্যথা যদি (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } অন্যথা {
            warn_internalerror();
            ঘূৰাই;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        যদি (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            সংরচনা smm_layout *newsmm = (শূন্য*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, পরিমাণ(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, পরিমাণ(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        ঘূৰাই;
    }

    যদি (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        যদি (rev == SMM_REV_I32) {
            u32 regs[8];
            memcpy(regs, &smm->cpu.i32.eax, পরিমাণ(regs));
            যদি (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->backup2, &smm->cpu, পরিমাণ(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, পরিমাণ(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, পরিমাণ(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } অন্যথা যদি (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->cpu, &smm->backup2, পরিমাণ(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, পরিমাণ(regs));
                যদি (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } অন্যথা যদি (rev == SMM_REV_I64) {
            u64 regs[8];
            memcpy(regs, &smm->cpu.i64.rdi, পরিমাণ(regs));
            যদি ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                memcpy(&smm->backup2, &smm->cpu, পরিমাণ(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, পরিমাণ(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, পরিমাণ(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } অন্যথা যদি ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                memcpy(&smm->cpu, &smm->backup2, পরিমাণ(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, পরিমাণ(regs));
                যদি (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

বাহ্য শূন্য entry_smi(শূন্য);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

স্থির শূন্য
smm_save_and_copy(শূন্য)
{
    // save original memory content
    সংরচনা smm_layout *initsmm = (শূন্য*)BUILD_SMM_INIT_ADDR;
    সংরচনা smm_layout *smm = (শূন্য*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, পরিমাণ(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, পরিমাণ(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

স্থির শূন্য
smm_relocate_and_restore(শূন্য)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    যেতিয়ালৈ (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    সংরচনা smm_layout *initsmm = (শূন্য*)BUILD_SMM_INIT_ADDR;
    সংরচনা smm_layout *smm = (শূন্য*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, পরিমাণ(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , পরিমাণ(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
স্থির শূন্য piix4_apmc_smm_setup(পূর্ণাঙ্ক isabdf, পূর্ণাঙ্ক i440_bdf)
{
    /* check if SMM init is already done */
    u32 মান = pci_config_readl(isabdf, PIIX_DEVACTB);
    যদি (মান & PIIX_DEVACTB_APMC_EN)
        ঘূৰাই;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, মান | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    মান = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(মান | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
শূন্য ich9_lpc_apmc_smm_setup(পূর্ণাঙ্ক isabdf, পূর্ণাঙ্ক mch_bdf)
{
    /* check if SMM init is already done */
    u32 মান = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    যদি (মান & ICH9_PMIO_SMI_EN_APMC_EN)
        ঘূৰাই;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(মান | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    মান = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      মান | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

স্থির পূর্ণাঙ্ক SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

শূন্য
smm_device_setup(শূন্য)
{
    যদি (!CONFIG_USE_SMM)
        ঘূৰাই;

    সংরচনা pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    যদি (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        ঘূৰাই;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    যদি (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

শূন্য
smm_setup(শূন্য)
{
    যদি (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        ঘূৰাই;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    যদি (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    অন্যথা
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
