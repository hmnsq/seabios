// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "dev-q35.h"
#അന്തര്ഭാവ "dev-piix.h"
#അന്തര്ഭാവ "hw/pci.h" // pci_config_writel
#അന്തര്ഭാവ "hw/pcidevice.h" // pci_find_device
#അന്തര്ഭാവ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#അന്തര്ഭാവ "hw/pci_regs.h" // PCI_DEVICE_ID
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "paravirt.h" // PORT_SMI_STATUS
#അന്തര്ഭാവ "stacks.h" // HaveSmmCall32
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "util.h" // smm_setup
#അന്തര്ഭാവ "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

ഘടന smm_state {
    സംഘം {
        ഘടന {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        ഘടന {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

ഘടന smm_layout {
    ഘടന smm_state backup1;
    ഘടന smm_state backup2;
    u32 backup_a20;
    u8 അട്ടി[0x8000 - വലുപ്പം(ഘടന smm_state)*2 - വലുപ്പം(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    ഘടന smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
സ്ഥിതം ശൂന്യം
memcpy_nowarn(ശൂന്യം *d, സ്ഥിരം ശൂന്യം *s, size_t len)
{
    asm("" : "+r"(d), "+r"(s));
    memcpy(d, s, len);
}

ശൂന്യം VISIBLE32FLAT
handle_smi(u16 cs)
{
    എങ്കിൽ (!CONFIG_USE_SMM)
        തിരികെ;
    u8 cmd = inb(PORT_SMI_CMD);
    ഘടന smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    എങ്കിൽ (smm == (ശൂന്യം*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        എങ്കിൽ (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } അല്ലെങ്കിൽ എങ്കിൽ (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } അല്ലെങ്കിൽ {
            warn_internalerror();
            തിരികെ;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        എങ്കിൽ (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            ഘടന smm_layout *newsmm = (ശൂന്യം*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, വലുപ്പം(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, വലുപ്പം(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        തിരികെ;
    }

    എങ്കിൽ (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        എങ്കിൽ (rev == SMM_REV_I32) {
            u32 regs[8];
            memcpy(regs, &smm->cpu.i32.eax, വലുപ്പം(regs));
            എങ്കിൽ (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->backup2, &smm->cpu, വലുപ്പം(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, വലുപ്പം(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, വലുപ്പം(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } അല്ലെങ്കിൽ എങ്കിൽ (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->cpu, &smm->backup2, വലുപ്പം(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, വലുപ്പം(regs));
                എങ്കിൽ (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } അല്ലെങ്കിൽ എങ്കിൽ (rev == SMM_REV_I64) {
            u64 regs[8];
            memcpy(regs, &smm->cpu.i64.rdi, വലുപ്പം(regs));
            എങ്കിൽ ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                memcpy(&smm->backup2, &smm->cpu, വലുപ്പം(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, വലുപ്പം(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, വലുപ്പം(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } അല്ലെങ്കിൽ എങ്കിൽ ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                memcpy(&smm->cpu, &smm->backup2, വലുപ്പം(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, വലുപ്പം(regs));
                എങ്കിൽ (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

ബാഹ്യ ശൂന്യം entry_smi(ശൂന്യം);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

സ്ഥിതം ശൂന്യം
smm_save_and_copy(ശൂന്യം)
{
    // save original memory content
    ഘടന smm_layout *initsmm = (ശൂന്യം*)BUILD_SMM_INIT_ADDR;
    ഘടന smm_layout *smm = (ശൂന്യം*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, വലുപ്പം(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, വലുപ്പം(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

സ്ഥിതം ശൂന്യം
smm_relocate_and_restore(ശൂന്യം)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    ആകുമ്പോൾ (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    ഘടന smm_layout *initsmm = (ശൂന്യം*)BUILD_SMM_INIT_ADDR;
    ഘടന smm_layout *smm = (ശൂന്യം*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, വലുപ്പം(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , വലുപ്പം(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
സ്ഥിതം ശൂന്യം piix4_apmc_smm_setup(പൂർണ്ണസംഖ്യ isabdf, പൂർണ്ണസംഖ്യ i440_bdf)
{
    /* check if SMM init is already done */
    u32 മൂല്യം = pci_config_readl(isabdf, PIIX_DEVACTB);
    എങ്കിൽ (മൂല്യം & PIIX_DEVACTB_APMC_EN)
        തിരികെ;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, മൂല്യം | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    മൂല്യം = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(മൂല്യം | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ശൂന്യം ich9_lpc_apmc_smm_setup(പൂർണ്ണസംഖ്യ isabdf, പൂർണ്ണസംഖ്യ mch_bdf)
{
    /* check if SMM init is already done */
    u32 മൂല്യം = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    എങ്കിൽ (മൂല്യം & ICH9_PMIO_SMI_EN_APMC_EN)
        തിരികെ;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(മൂല്യം | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    മൂല്യം = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      മൂല്യം | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

ശൂന്യം
smm_device_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_USE_SMM)
        തിരികെ;

    ഘടന pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    എങ്കിൽ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        തിരികെ;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    എങ്കിൽ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

ശൂന്യം
smm_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        തിരികെ;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    എങ്കിൽ (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    അല്ലെങ്കിൽ
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
