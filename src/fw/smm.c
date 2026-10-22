// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "dev-q35.h"
#समावेश "dev-piix.h"
#समावेश "hw/pci.h" // pci_config_writel
#समावेश "hw/pcidevice.h" // pci_find_device
#समावेश "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#समावेश "hw/pci_regs.h" // PCI_DEVICE_ID
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // PORT_SMI_STATUS
#समावेश "stacks.h" // HaveSmmCall32
#समावेश "string.h" // memcpy
#समावेश "util.h" // smm_setup
#समावेश "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#घोषणा SMM_REV_MASK 0x0002ffff

#घोषणा SMM_REV_I32  0x00020000
#घोषणा SMM_REV_I64  0x00020064

काष्ठा smm_state {
    जोड़ {
        काष्ठा {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        काष्ठा {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

काष्ठा smm_layout {
    काष्ठा smm_state backup1;
    काष्ठा smm_state backup2;
    u32 backup_a20;
    u8 ढेर_2[0x8000 - माप(काष्ठा smm_state)*2 - माप(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    काष्ठा smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
अटल व्योम
memcpy_nowarn(व्योम *d, स्थिर व्योम *s, माप_प्रकार len)
{
    यंत्र("" : "+r"(d), "+r"(s));
    स_नकल(d, s, len);
}

व्योम VISIBLE32FLAT
handle_smi(u16 cs)
{
    अगर (!CONFIG_USE_SMM)
        वापस;
    u8 cmd = inb(PORT_SMI_CMD);
    काष्ठा smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    अगर (smm == (व्योम*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        अगर (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } अन्यथा अगर (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } अन्यथा {
            warn_internalerror();
            वापस;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        अगर (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            काष्ठा smm_layout *newsmm = (व्योम*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, माप(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, माप(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        वापस;
    }

    अगर (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        अगर (rev == SMM_REV_I32) {
            u32 regs[8];
            स_नकल(regs, &smm->cpu.i32.eax, माप(regs));
            अगर (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                स_नकल(&smm->backup2, &smm->cpu, माप(smm->backup2));
                स_नकल(&smm->cpu, &smm->backup1, माप(smm->cpu));
                स_नकल(&smm->cpu.i32.eax, regs, माप(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } अन्यथा अगर (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                स_नकल(&smm->cpu, &smm->backup2, माप(smm->cpu));
                स_नकल(&smm->cpu.i32.eax, regs, माप(regs));
                अगर (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } अन्यथा अगर (rev == SMM_REV_I64) {
            u64 regs[8];
            स_नकल(regs, &smm->cpu.i64.rdi, माप(regs));
            अगर ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                स_नकल(&smm->backup2, &smm->cpu, माप(smm->backup2));
                स_नकल(&smm->cpu, &smm->backup1, माप(smm->cpu));
                स_नकल(&smm->cpu.i64.rdi, regs, माप(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } अन्यथा अगर ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                स_नकल(&smm->cpu, &smm->backup2, माप(smm->cpu));
                स_नकल(&smm->cpu.i64.rdi, regs, माप(regs));
                अगर (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

बाह्य व्योम entry_smi(व्योम);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#घोषणा SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

अटल व्योम
smm_save_and_copy(व्योम)
{
    // save original memory content
    काष्ठा smm_layout *initsmm = (व्योम*)BUILD_SMM_INIT_ADDR;
    काष्ठा smm_layout *smm = (व्योम*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, माप(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, माप(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

अटल व्योम
smm_relocate_and_restore(व्योम)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    जबतक (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    काष्ठा smm_layout *initsmm = (व्योम*)BUILD_SMM_INIT_ADDR;
    काष्ठा smm_layout *smm = (व्योम*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, माप(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , माप(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
अटल व्योम piix4_apmc_smm_setup(पूर्णांक isabdf, पूर्णांक i440_bdf)
{
    /* check if SMM init is already done */
    u32 मूल्य = pci_config_readl(isabdf, PIIX_DEVACTB);
    अगर (मूल्य & PIIX_DEVACTB_APMC_EN)
        वापस;

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
व्योम ich9_lpc_apmc_smm_setup(पूर्णांक isabdf, पूर्णांक mch_bdf)
{
    /* check if SMM init is already done */
    u32 मूल्य = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    अगर (मूल्य & ICH9_PMIO_SMI_EN_APMC_EN)
        वापस;

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

अटल पूर्णांक SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

व्योम
smm_device_setup(व्योम)
{
    अगर (!CONFIG_USE_SMM)
        वापस;

    काष्ठा pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    अगर (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        वापस;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    अगर (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

व्योम
smm_setup(व्योम)
{
    अगर (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        वापस;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    अगर (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    अन्यथा
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
