// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // CONFIG_*
#include "dev-q35.h"
#include "dev-piix.h"
#include "hw/pci.h" // pci_config_writel
#include "hw/pcidevice.h" // pci_find_device
#include "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#include "hw/pci_regs.h" // PCI_DEVICE_ID
#include "output.h" // dprintf
#include "paravirt.h" // PORT_SMI_STATUS
#include "stacks.h" // HaveSmmCall32
#include "string.h" // memcpy
#include "util.h" // smm_setup
#include "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

sa_nracanaa smm_state {
    sa_ngha {
        sa_nracanaa {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        sa_nracanaa {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

sa_nracanaa smm_layout {
    sa_nracanaa smm_state backup1;
    sa_nracanaa smm_state backup2;
    u32 backup_a20;
    u8 stack[0x8000 - parimaa_na(sa_nracanaa smm_state)*2 - parimaa_na(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    sa_nracanaa smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
static void
memcpy_nowarn(void *d, const void *s, size_t len)
{
    asm("" : "+r"(d), "+r"(s));
    memcpy(d, s, len);
}

void VISIBLE32FLAT
handle_smi(u16 cs)
{
    ပေႃး (!CONFIG_USE_SMM)
        ပွၵ်ႈ;
    u8 cmd = inb(PORT_SMI_CMD);
    sa_nracanaa smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    ပေႃး (smm == (void*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        ပေႃး (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } ပေႃးဢမ်ႇ ပေႃး (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } ပေႃးဢမ်ႇ {
            warn_internalerror();
            ပွၵ်ႈ;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        ပေႃး (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            sa_nracanaa smm_layout *newsmm = (void*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, parimaa_na(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, parimaa_na(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        ပွၵ်ႈ;
    }

    ပေႃး (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        ပေႃး (rev == SMM_REV_I32) {
            u32 regs[8];
            memcpy(regs, &smm->cpu.i32.eax, parimaa_na(regs));
            ပေႃး (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->backup2, &smm->cpu, parimaa_na(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, parimaa_na(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, parimaa_na(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } ပေႃးဢမ်ႇ ပေႃး (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->cpu, &smm->backup2, parimaa_na(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, parimaa_na(regs));
                ပေႃး (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } ပေႃးဢမ်ႇ ပေႃး (rev == SMM_REV_I64) {
            u64 regs[8];
            memcpy(regs, &smm->cpu.i64.rdi, parimaa_na(regs));
            ပေႃး ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                memcpy(&smm->backup2, &smm->cpu, parimaa_na(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, parimaa_na(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, parimaa_na(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } ပေႃးဢမ်ႇ ပေႃး ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                memcpy(&smm->cpu, &smm->backup2, parimaa_na(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, parimaa_na(regs));
                ပေႃး (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

extern void entry_smi(void);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

static void
smm_save_and_copy(void)
{
    // save original memory content
    sa_nracanaa smm_layout *initsmm = (void*)BUILD_SMM_INIT_ADDR;
    sa_nracanaa smm_layout *smm = (void*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, parimaa_na(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, parimaa_na(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

static void
smm_relocate_and_restore(void)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    ၶၢဝ်း (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    sa_nracanaa smm_layout *initsmm = (void*)BUILD_SMM_INIT_ADDR;
    sa_nracanaa smm_layout *smm = (void*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, parimaa_na(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , parimaa_na(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
static void piix4_apmc_smm_setup(puur_naa_nka isabdf, puur_naa_nka i440_bdf)
{
    /* check if SMM init is already done */
    u32 value = pci_config_readl(isabdf, PIIX_DEVACTB);
    ပေႃး (value & PIIX_DEVACTB_APMC_EN)
        ပွၵ်ႈ;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, value | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    value = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(value | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
void ich9_lpc_apmc_smm_setup(puur_naa_nka isabdf, puur_naa_nka mch_bdf)
{
    /* check if SMM init is already done */
    u32 value = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    ပေႃး (value & ICH9_PMIO_SMI_EN_APMC_EN)
        ပွၵ်ႈ;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(value | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    value = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      value | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

static puur_naa_nka SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

void
smm_device_setup(void)
{
    ပေႃး (!CONFIG_USE_SMM)
        ပွၵ်ႈ;

    sa_nracanaa pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    ပေႃး (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        ပွၵ်ႈ;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    ပေႃး (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

void
smm_setup(void)
{
    ပေႃး (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        ပွၵ်ႈ;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    ပေႃး (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    ပေႃးဢမ်ႇ
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
