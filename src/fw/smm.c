// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "dev-q35.h"
#ଅନ୍ତର୍ଭାଵ "dev-piix.h"
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_config_writel
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // pci_find_device
#ଅନ୍ତର୍ଭାଵ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ଅନ୍ତର୍ଭାଵ "hw/pci_regs.h" // PCI_DEVICE_ID
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "paravirt.h" // PORT_SMI_STATUS
#ଅନ୍ତର୍ଭାଵ "stacks.h" // HaveSmmCall32
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "util.h" // smm_setup
#ଅନ୍ତର୍ଭାଵ "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

ସଂରଚନା smm_state {
    ସଂଘ {
        ସଂରଚନା {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        ସଂରଚନା {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

ସଂରଚନା smm_layout {
    ସଂରଚନା smm_state backup1;
    ସଂରଚନା smm_state backup2;
    u32 backup_a20;
    u8 ଗଦା[0x8000 - ଆକାର(ସଂରଚନା smm_state)*2 - ଆକାର(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    ସଂରଚନା smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
ସ୍ଥିର ଶୂନ୍ୟ
memcpy_nowarn(ଶୂନ୍ୟ *d, ଅଚଳ ଶୂନ୍ୟ *s, size_t len)
{
    asm("" : "+r"(d), "+r"(s));
    memcpy(d, s, len);
}

ଶୂନ୍ୟ VISIBLE32FLAT
handle_smi(u16 cs)
{
    ଯଦି (!CONFIG_USE_SMM)
        ଫେରନ୍ତୁ;
    u8 cmd = inb(PORT_SMI_CMD);
    ସଂରଚନା smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    ଯଦି (smm == (ଶୂନ୍ୟ*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        ଯଦି (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } ଅନ୍ୟଥା ଯଦି (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } ଅନ୍ୟଥା {
            warn_internalerror();
            ଫେରନ୍ତୁ;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        ଯଦି (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            ସଂରଚନା smm_layout *newsmm = (ଶୂନ୍ୟ*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, ଆକାର(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, ଆକାର(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        ଫେରନ୍ତୁ;
    }

    ଯଦି (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        ଯଦି (rev == SMM_REV_I32) {
            u32 regs[8];
            memcpy(regs, &smm->cpu.i32.eax, ଆକାର(regs));
            ଯଦି (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->backup2, &smm->cpu, ଆକାର(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, ଆକାର(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, ଆକାର(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } ଅନ୍ୟଥା ଯଦି (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->cpu, &smm->backup2, ଆକାର(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, ଆକାର(regs));
                ଯଦି (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } ଅନ୍ୟଥା ଯଦି (rev == SMM_REV_I64) {
            u64 regs[8];
            memcpy(regs, &smm->cpu.i64.rdi, ଆକାର(regs));
            ଯଦି ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                memcpy(&smm->backup2, &smm->cpu, ଆକାର(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, ଆକାର(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, ଆକାର(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } ଅନ୍ୟଥା ଯଦି ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                memcpy(&smm->cpu, &smm->backup2, ଆକାର(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, ଆକାର(regs));
                ଯଦି (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

ବାହ୍ୟ ଶୂନ୍ୟ entry_smi(ଶୂନ୍ୟ);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

ସ୍ଥିର ଶୂନ୍ୟ
smm_save_and_copy(ଶୂନ୍ୟ)
{
    // save original memory content
    ସଂରଚନା smm_layout *initsmm = (ଶୂନ୍ୟ*)BUILD_SMM_INIT_ADDR;
    ସଂରଚନା smm_layout *smm = (ଶୂନ୍ୟ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, ଆକାର(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, ଆକାର(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

ସ୍ଥିର ଶୂନ୍ୟ
smm_relocate_and_restore(ଶୂନ୍ୟ)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    ଯେତେବେଳେ (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    ସଂରଚନା smm_layout *initsmm = (ଶୂନ୍ୟ*)BUILD_SMM_INIT_ADDR;
    ସଂରଚନା smm_layout *smm = (ଶୂନ୍ୟ*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, ଆକାର(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , ଆକାର(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
ସ୍ଥିର ଶୂନ୍ୟ piix4_apmc_smm_setup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା isabdf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i440_bdf)
{
    /* check if SMM init is already done */
    u32 ମୂଲ୍ୟ = pci_config_readl(isabdf, PIIX_DEVACTB);
    ଯଦି (ମୂଲ୍ୟ & PIIX_DEVACTB_APMC_EN)
        ଫେରନ୍ତୁ;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, ମୂଲ୍ୟ | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    ମୂଲ୍ୟ = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(ମୂଲ୍ୟ | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ଶୂନ୍ୟ ich9_lpc_apmc_smm_setup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା isabdf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mch_bdf)
{
    /* check if SMM init is already done */
    u32 ମୂଲ୍ୟ = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    ଯଦି (ମୂଲ୍ୟ & ICH9_PMIO_SMI_EN_APMC_EN)
        ଫେରନ୍ତୁ;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(ମୂଲ୍ୟ | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    ମୂଲ୍ୟ = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      ମୂଲ୍ୟ | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

ଶୂନ୍ୟ
smm_device_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_USE_SMM)
        ଫେରନ୍ତୁ;

    ସଂରଚନା pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    ଯଦି (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        ଫେରନ୍ତୁ;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    ଯଦି (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

ଶୂନ୍ୟ
smm_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        ଫେରନ୍ତୁ;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    ଯଦି (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    ଅନ୍ୟଥା
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
