// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "dev-q35.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "dev-piix.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_writel
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // pci_find_device
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_DEVICE_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // PORT_SMI_STATUS
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // HaveSmmCall32
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // smm_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMM_REV_ਨਕਾਬ 0x0002ffff

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMM_REV_I32  0x00020000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMM_REV_I64  0x00020064

ਬਣਤਰ smm_state {
    ਸੰਘ {
        ਬਣਤਰ {
            u8 pad_000[0xf8];
            u32 smm_ਅਧਾਰ;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        ਬਣਤਰ {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_ਅਧਾਰ;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

ਬਣਤਰ smm_layout {
    ਬਣਤਰ smm_state backup1;
    ਬਣਤਰ smm_state backup2;
    u32 backup_a20;
    u8 ਢੇਰ[0x8000 - ਆਕਾਰ(ਬਣਤਰ smm_state)*2 - ਆਕਾਰ(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    ਬਣਤਰ smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
ਸਥਿਰ ਖਾਲੀ
memcpy_nowarn(ਖਾਲੀ *d, ਅਚਲ ਖਾਲੀ *s, ਮਾਪ_t len)
{
    asm("" : "+r"(d), "+r"(s));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(d, s, len);
}

ਖਾਲੀ VISIBLE32FLAT
handle_smi(u16 cs)
{
    ਜੇ (!CONFIG_USE_SMM)
        ਵਾਪਸ;
    u8 cmd = inb(ਪੋਰਟ_SMI_CMD);
    ਬਣਤਰ smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_ਨਕਾਬ;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    ਜੇ (smm == (ਖਾਲੀ*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        ਜੇ (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_ਅਧਾਰ = BUILD_SMM_ADDR;
        } ਨਹੀਂਤਾਂ ਜੇ (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_ਅਧਾਰ = BUILD_SMM_ADDR;
        } ਨਹੀਂਤਾਂ {
            warn_internalerror();
            ਵਾਪਸ;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, ਪੋਰਟ_SMI_ਸਥਿਤੀ);

        ਜੇ (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            ਬਣਤਰ smm_layout *newsmm = (ਖਾਲੀ*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, ਆਕਾਰ(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, ਆਕਾਰ(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        ਵਾਪਸ;
    }

    ਜੇ (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        ਜੇ (rev == SMM_REV_I32) {
            u32 regs[8];
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(regs, &smm->cpu.i32.eax, ਆਕਾਰ(regs));
            ਜੇ (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->backup2, &smm->cpu, ਆਕਾਰ(smm->backup2));
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu, &smm->backup1, ਆਕਾਰ(smm->cpu));
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu.i32.eax, regs, ਆਕਾਰ(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = ਸੈੱਟ_a20(1);
            } ਨਹੀਂਤਾਂ ਜੇ (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu, &smm->backup2, ਆਕਾਰ(smm->cpu));
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu.i32.eax, regs, ਆਕਾਰ(regs));
                ਜੇ (!smm->backup_a20)
                    ਸੈੱਟ_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } ਨਹੀਂਤਾਂ ਜੇ (rev == SMM_REV_I64) {
            u64 regs[8];
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(regs, &smm->cpu.i64.rdi, ਆਕਾਰ(regs));
            ਜੇ ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->backup2, &smm->cpu, ਆਕਾਰ(smm->backup2));
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu, &smm->backup1, ਆਕਾਰ(smm->cpu));
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu.i64.rdi, regs, ਆਕਾਰ(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = ਸੈੱਟ_a20(1);
            } ਨਹੀਂਤਾਂ ਜੇ ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu, &smm->backup2, ਆਕਾਰ(smm->cpu));
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(&smm->cpu.i64.rdi, regs, ਆਕਾਰ(regs));
                ਜੇ (!smm->backup_a20)
                    ਸੈੱਟ_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_smi(ਖਾਲੀ);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)ਇੰਦਰਾਜ_smi - BUILD_BIOS_ADDR) << 24))

ਸਥਿਰ ਖਾਲੀ
smm_save_and_copy(ਖਾਲੀ)
{
    // save original memory content
    ਬਣਤਰ smm_layout *initsmm = (ਖਾਲੀ*)BUILD_SMM_INIT_ADDR;
    ਬਣਤਰ smm_layout *smm = (ਖਾਲੀ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, ਆਕਾਰ(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, ਆਕਾਰ(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

ਸਥਿਰ ਖਾਲੀ
smm_relocate_and_restore(ਖਾਲੀ)
{
    /* init APM status port */
    outb(0x01, ਪੋਰਟ_SMI_ਸਥਿਤੀ);

    /* raise an SMI interrupt */
    outb(0x00, ਪੋਰਟ_SMI_CMD);

    /* wait until SMM code executed */
    ਜਦੋਂ (inb(ਪੋਰਟ_SMI_ਸਥਿਤੀ) != 0x00)
        ;

    /* restore original memory content */
    ਬਣਤਰ smm_layout *initsmm = (ਖਾਲੀ*)BUILD_SMM_INIT_ADDR;
    ਬਣਤਰ smm_layout *smm = (ਖਾਲੀ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, ਆਕਾਰ(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , ਆਕਾਰ(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
ਸਥਿਰ ਖਾਲੀ piix4_apmc_smm_ਤਿਆਰੀ(ਪੂਰਨ isabdf, ਪੂਰਨ i440_bdf)
{
    /* check if SMM init is already done */
    u32 ਮੁੱਲ = pci_config_readl(isabdf, PIIX_DEVACTB);
    ਜੇ (ਮੁੱਲ & PIIX_DEVACTB_APMC_EN)
        ਵਾਪਸ;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, ਮੁੱਲ | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    ਮੁੱਲ = inl(acpi_pm_ਅਧਾਰ + PIIX_PMIO_GLBCTL);
    outl(ਮੁੱਲ | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_ਅਧਾਰ + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ਖਾਲੀ ich9_lpc_apmc_smm_ਤਿਆਰੀ(ਪੂਰਨ isabdf, ਪੂਰਨ mch_bdf)
{
    /* check if SMM init is already done */
    u32 ਮੁੱਲ = inl(acpi_pm_ਅਧਾਰ + ICH9_PMIO_SMI_EN);
    ਜੇ (ਮੁੱਲ & ICH9_PMIO_SMI_EN_APMC_EN)
        ਵਾਪਸ;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(ਮੁੱਲ | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_ਅਧਾਰ + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    ਮੁੱਲ = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      ਮੁੱਲ | ICH9_LPC_GEN_PMCON_1_SMI_ਤਾਲਾ);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

ਸਥਿਰ ਪੂਰਨ SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

ਖਾਲੀ
smm_device_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_USE_SMM)
        ਵਾਪਸ;

    ਬਣਤਰ pci_device *isapci, *pmpci;
    isapci = pci_ਲੱਭੋ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_ਲੱਭੋ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    ਜੇ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        ਵਾਪਸ;
    }
    isapci = pci_ਲੱਭੋ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_ਲੱਭੋ_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    ਜੇ (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

ਖਾਲੀ
smm_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        ਵਾਪਸ;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    ਜੇ (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_ਤਿਆਰੀ(SMMISADeviceBDF, SMMPMDeviceBDF);
    ਨਹੀਂਤਾਂ
        ich9_lpc_apmc_smm_ਤਿਆਰੀ(SMMISADeviceBDF, SMMPMDeviceBDF);
}
