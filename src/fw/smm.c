// System Management Mode support (on emulators)
//
// Copyright (C) 2008-2014  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "dev-q35.h"
#அந்தர்பாவ "dev-piix.h"
#அந்தர்பாவ "hw/pci.h" // pci_config_writel
#அந்தர்பாவ "hw/pcidevice.h" // pci_find_device
#அந்தர்பாவ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#அந்தர்பாவ "hw/pci_regs.h" // PCI_DEVICE_ID
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "paravirt.h" // PORT_SMI_STATUS
#அந்தர்பாவ "stacks.h" // HaveSmmCall32
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "util.h" // smm_setup
#அந்தர்பாவ "x86.h" // wbinvd

/*
 * Check SMM state save area format (bits 0-15) and require support
 * for SMBASE relocation.
 */
#define SMM_REV_MASK 0x0002ffff

#define SMM_REV_I32  0x00020000
#define SMM_REV_I64  0x00020064

கட்டமைப்பு smm_state {
    ஒன்றிப்பு {
        கட்டமைப்பு {
            u8 pad_000[0xf8];
            u32 smm_base;
            u32 smm_rev;
            u8 pad_100[0xd0];
            u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
            u8 pad_1f8[0x08];
        } i32;
        கட்டமைப்பு {
            u8 pad_000[0xfc];
            u32 smm_rev;
            u32 smm_base;
            u8 pad_104[0x6c];
            u64 rflags, rip, r15, r14, r13, r12, r11, r10, r9, r8;
            u64 rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
        } i64;
    };
};

கட்டமைப்பு smm_layout {
    கட்டமைப்பு smm_state backup1;
    கட்டமைப்பு smm_state backup2;
    u32 backup_a20;
    u8 அடுக்கு[0x8000 - அளவு(கட்டமைப்பு smm_state)*2 - அளவு(u32)];
    u64 codeentry;
    u8 pad_8008[0x7df8];
    கட்டமைப்பு smm_state cpu;
};

// Hack to supress some gcc array-bounds warnings
நிலையான வெறுமை
memcpy_nowarn(வெறுமை *d, மாறிலி வெறுமை *s, size_t len)
{
    asm("" : "+r"(d), "+r"(s));
    memcpy(d, s, len);
}

வெறுமை VISIBLE32FLAT
handle_smi(u16 cs)
{
    எனில் (!CONFIG_USE_SMM)
        திரும்பு;
    u8 cmd = inb(PORT_SMI_CMD);
    கட்டமைப்பு smm_layout *smm = MAKE_FLATPTR(cs, 0);
    u32 rev = smm->cpu.i32.smm_rev & SMM_REV_MASK;
    dprintf(DEBUG_HDL_smi, "handle_smi cmd=%x smbase=%p\n", cmd, smm);

    எனில் (smm == (வெறுமை*)BUILD_SMM_INIT_ADDR) {
        // relocate SMBASE to 0xa0000
        எனில் (rev == SMM_REV_I32) {
            smm->cpu.i32.smm_base = BUILD_SMM_ADDR;
        } இல்லையெனில் எனில் (rev == SMM_REV_I64) {
            smm->cpu.i64.smm_base = BUILD_SMM_ADDR;
        } இல்லையெனில் {
            warn_internalerror();
            திரும்பு;
        }
        // indicate to smm_relocate_and_restore() that the SMM code was executed
        outb(0x00, PORT_SMI_STATUS);

        எனில் (CONFIG_CALL32_SMM) {
            // Backup current cpu state for SMM trampolining
            கட்டமைப்பு smm_layout *newsmm = (வெறுமை*)BUILD_SMM_ADDR;
            memcpy_nowarn(&newsmm->backup1, &smm->cpu, அளவு(newsmm->backup1));
            memcpy_nowarn(&newsmm->backup2, &smm->cpu, அளவு(newsmm->backup2));
            HaveSmmCall32 = 1;
        }

        திரும்பு;
    }

    எனில் (CONFIG_CALL32_SMM && cmd == CALL32SMM_CMDID) {
        எனில் (rev == SMM_REV_I32) {
            u32 regs[8];
            memcpy(regs, &smm->cpu.i32.eax, அளவு(regs));
            எனில் (smm->cpu.i32.ecx == CALL32SMM_ENTERID) {
                dprintf(9, "smm cpu call pc=%x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->backup2, &smm->cpu, அளவு(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, அளவு(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, அளவு(regs));
                smm->cpu.i32.eip = regs[3];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } இல்லையெனில் எனில் (smm->cpu.i32.ecx == CALL32SMM_RETURNID) {
                dprintf(9, "smm cpu ret %x esp=%x\n", regs[3], regs[4]);
                memcpy(&smm->cpu, &smm->backup2, அளவு(smm->cpu));
                memcpy(&smm->cpu.i32.eax, regs, அளவு(regs));
                எனில் (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i32.eip = regs[3];
            }
        } இல்லையெனில் எனில் (rev == SMM_REV_I64) {
            u64 regs[8];
            memcpy(regs, &smm->cpu.i64.rdi, அளவு(regs));
            எனில் ((u32)smm->cpu.i64.rcx == CALL32SMM_ENTERID) {
                memcpy(&smm->backup2, &smm->cpu, அளவு(smm->backup2));
                memcpy(&smm->cpu, &smm->backup1, அளவு(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, அளவு(regs));
                smm->cpu.i64.rip = (u32)regs[4];
                // Enable a20 and backup its previous state
                smm->backup_a20 = set_a20(1);
            } இல்லையெனில் எனில் ((u32)smm->cpu.i64.rcx == CALL32SMM_RETURNID) {
                memcpy(&smm->cpu, &smm->backup2, அளவு(smm->cpu));
                memcpy(&smm->cpu.i64.rdi, regs, அளவு(regs));
                எனில் (!smm->backup_a20)
                    set_a20(0);
                smm->cpu.i64.rip = (u32)regs[4];
            }
        }
    }
}

வெளி வெறுமை entry_smi(வெறுமை);
// movw %cs, %ax; ljmpw $SEG_BIOS, $(entry_smi - BUILD_BIOS_ADDR)
#define SMI_INSN (0xeac88c | ((u64)SEG_BIOS<<40) \
                  | ((u64)((u32)entry_smi - BUILD_BIOS_ADDR) << 24))

நிலையான வெறுமை
smm_save_and_copy(வெறுமை)
{
    // save original memory content
    கட்டமைப்பு smm_layout *initsmm = (வெறுமை*)BUILD_SMM_INIT_ADDR;
    கட்டமைப்பு smm_layout *smm = (வெறுமை*)BUILD_SMM_ADDR;
    memcpy_nowarn(&smm->cpu, &initsmm->cpu, அளவு(smm->cpu));
    memcpy_nowarn(&smm->codeentry, &initsmm->codeentry, அளவு(smm->codeentry));

    // Setup code entry point.
    initsmm->codeentry = SMI_INSN;
}

நிலையான வெறுமை
smm_relocate_and_restore(வெறுமை)
{
    /* init APM status port */
    outb(0x01, PORT_SMI_STATUS);

    /* raise an SMI interrupt */
    outb(0x00, PORT_SMI_CMD);

    /* wait until SMM code executed */
    போது (inb(PORT_SMI_STATUS) != 0x00)
        ;

    /* restore original memory content */
    கட்டமைப்பு smm_layout *initsmm = (வெறுமை*)BUILD_SMM_INIT_ADDR;
    கட்டமைப்பு smm_layout *smm = (வெறுமை*)BUILD_SMM_ADDR;
    memcpy_nowarn(&initsmm->cpu, &smm->cpu, அளவு(initsmm->cpu));
    memcpy_nowarn(&initsmm->codeentry, &smm->codeentry
                  , அளவு(initsmm->codeentry));

    // Setup code entry point.
    smm->codeentry = SMI_INSN;
    wbinvd();
}

// This code is hardcoded for PIIX4 Power Management device.
நிலையான வெறுமை piix4_apmc_smm_setup(முழு isabdf, முழு i440_bdf)
{
    /* check if SMM init is already done */
    u32 மதிப்பு = pci_config_readl(isabdf, PIIX_DEVACTB);
    எனில் (மதிப்பு & PIIX_DEVACTB_APMC_EN)
        திரும்பு;

    /* enable the SMM memory window */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    pci_config_writel(isabdf, PIIX_DEVACTB, மதிப்பு | PIIX_DEVACTB_APMC_EN);

    /* enable SMI generation */
    மதிப்பு = inl(acpi_pm_base + PIIX_PMIO_GLBCTL);
    outl(மதிப்பு | PIIX_PMIO_GLBCTL_SMI_EN, acpi_pm_base + PIIX_PMIO_GLBCTL);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(i440_bdf, I440FX_SMRAM, 0x02 | 0x08);
}

/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
வெறுமை ich9_lpc_apmc_smm_setup(முழு isabdf, முழு mch_bdf)
{
    /* check if SMM init is already done */
    u32 மதிப்பு = inl(acpi_pm_base + ICH9_PMIO_SMI_EN);
    எனில் (மதிப்பு & ICH9_PMIO_SMI_EN_APMC_EN)
        திரும்பு;

    /* enable the SMM memory window */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x48);

    smm_save_and_copy();

    /* enable SMI generation when writing to the APMC register */
    outl(மதிப்பு | ICH9_PMIO_SMI_EN_APMC_EN | ICH9_PMIO_SMI_EN_GLB_SMI_EN,
         acpi_pm_base + ICH9_PMIO_SMI_EN);

    /* lock SMI generation */
    மதிப்பு = pci_config_readw(isabdf, ICH9_LPC_GEN_PMCON_1);
    pci_config_writel(isabdf, ICH9_LPC_GEN_PMCON_1,
                      மதிப்பு | ICH9_LPC_GEN_PMCON_1_SMI_LOCK);

    smm_relocate_and_restore();

    /* close the SMM memory window and enable normal SMM */
    pci_config_writeb(mch_bdf, Q35_HOST_BRIDGE_SMRAM, 0x02 | 0x08);
}

நிலையான முழு SMMISADeviceBDF = -1, SMMPMDeviceBDF = -1;

வெறுமை
smm_device_setup(வெறுமை)
{
    எனில் (!CONFIG_USE_SMM)
        திரும்பு;

    கட்டமைப்பு pci_device *isapci, *pmpci;
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441);
    எனில் (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
        திரும்பு;
    }
    isapci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC);
    pmpci = pci_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH);
    எனில் (isapci && pmpci) {
        SMMISADeviceBDF = isapci->bdf;
        SMMPMDeviceBDF = pmpci->bdf;
    }
}

வெறுமை
smm_setup(வெறுமை)
{
    எனில் (!CONFIG_USE_SMM || SMMISADeviceBDF < 0)
        திரும்பு;

    dprintf(3, "init smm\n");
    u16 device = pci_config_readw(SMMISADeviceBDF, PCI_DEVICE_ID);
    எனில் (device == PCI_DEVICE_ID_INTEL_82371AB_3)
        piix4_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
    இல்லையெனில்
        ich9_lpc_apmc_smm_setup(SMMISADeviceBDF, SMMPMDeviceBDF);
}
