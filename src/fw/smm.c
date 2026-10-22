// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "dev-q35.h"
#अन्तर्भाव "dev-piix.h"
#अन्तर्भाव "hw/pci.h" // pci_config_writel
#अन्तर्भाव "hw/pcidevice.h" // pci_find_device
#अन्तर्भाव "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#अन्तर्भाव "hw/pci_regs.h" // PCI_DEVICE_ID
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // PORT_SMI_STATUS
#अन्तर्भाव "stacks.h" // HaveSmmCall32
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "util.h" // smm_setup
#अन्तर्भाव "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

संरचना smm_state {
    सङ्घ {
        संरचना {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        संरचना {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

संरचना smm_layout {
    संरचना smm_state backup1;
    संरचना smm_state backup2;
    u32 backup_a20;
    u8 ढेर[0x8000 - परिमाण(संरचना smm_state)*2 - परिमाण(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    संरचना smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
स्थिर शून्य
memcpy_nowarn(शून्य *d, ध्रुव शून्य *s, माप_प्रकार len)
{
    यंत्र("" : "+r"(d), "+r"(s));
    स_नकल(d, s, len);
}

शून्य VISIBLE32FLAT
handle_smi(u16 cs)
{
    यदि (!CONFIG_USE_SMM)
        प्रत्यागम;
    u8 cmd = inb(PORT_SMI_CMD);
    संरचना smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    यदि (smm == (शून्य*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        यदि (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } अन्यथा यदि (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } अन्यथा {
            warn_internalerror();
            प्रत्यागम;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        यदि (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            संरचना smm_layout *newsmm = (शून्य*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, परिमाण(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, परिमाण(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        प्रत्यागम;
    }

    यदि (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        यदि (rev == SMM_REV_I32) {
            u32 regs[8];
            स_नकल(regs, &smm->cpu.i32.eax, परिमाण(regs));
            यदि (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                स_नकल(&smm->backup2, &smm->cpu, परिमाण(smm->backup2));
                स_नकल(&smm->cpu, &smm->backup1, परिमाण(smm->cpu));
                स_नकल(&smm->cpu.i32.eax, regs, परिमाण(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } अन्यथा यदि (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                स_नकल(&smm->cpu, &smm->backup2, परिमाण(smm->cpu));
                स_नकल(&smm->cpu.i32.eax, regs, परिमाण(regs));
                यदि (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } अन्यथा यदि (rev == SMM_REV_I64) {
            u64 regs[8];
            स_नकल(regs, &smm->cpu.i64.rdi, परिमाण(regs));
            यदि ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                स_नकल(&smm->backup2, &smm->cpu, परिमाण(smm->backup2));
                स_नकल(&smm->cpu, &smm->backup1, परिमाण(smm->cpu));
                स_नकल(&smm->cpu.i64.rdi, regs, परिमाण(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } अन्यथा यदि ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                स_नकल(&smm->cpu, &smm->backup2, परिमाण(smm->cpu));
                स_नकल(&smm->cpu.i64.rdi, regs, परिमाण(regs));
                यदि (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

बाह्य शून्य entry_smi(शून्य);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

स्थिर शून्य
smm_save_and_copy(शून्य)
{
    // save original memory content
    संरचना smm_layout *initsmm = (शून्य*)BUILD_SMM_INIT_ADDR;
    संरचना smm_layout *smm = (शून्य*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, परिमाण(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, परिमाण(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

स्थिर शून्य
smm_relocate_and_restore(शून्य)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    यावत् (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    संरचना smm_layout *initsmm = (शून्य*)BUILD_SMM_INIT_ADDR;
    संरचना smm_layout *smm = (शून्य*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, परिमाण(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , परिमाण(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
स्थिर शून्य piix4_apmc_smm_setup(पूर्णाङ्क isabdf, पूर्णाङ्क i440_bdf)
{
    /* check if SMM init is already done */
    u32 मूल्य = pci_config_readl(isabdf, PIIX_DEVACTB);
    यदि (मूल्य & PIIX_DEVACTB_APMC_EN)
        प्रत्यागम;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, मूल्य | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    मूल्य = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(मूल्य | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
शून्य ich9_lpc_apmc_smm_setup(पूर्णाङ्क isabdf, पूर्णाङ्क mch_bdf)
{
    /* check if SMM init is already done */
    u32 मूल्य = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    यदि (मूल्य & ICH9_PMIO_SMI_EN_APMC_EN)
        प्रत्यागम;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(मूल्य | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    मूल्य = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      मूल्य | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

स्थिर पूर्णाङ्क SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

शून्य
smm_device_setup(शून्य)
{
    यदि (!CONFIG_USE_SMM)
        प्रत्यागम;

    संरचना pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    यदि (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        प्रत्यागम;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    यदि (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

शून्य
smm_setup(शून्य)
{
    यदि (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        प्रत्यागम;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    यदि (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    अन्यथा
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
