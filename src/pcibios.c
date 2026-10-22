// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_readl
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // MaxPCIBus
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/pirtable.h" // struct pir_header
#ಅನ್ತರ್ಭಾವ "string.h" // checksum
#ಅನ್ತರ್ಭಾವ "util.h" // handle_1ab1

// romlayout.S
ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_bios32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_pcibios32(ಶೂನ್ಯ);

#ವ್ಯಾಖ್ಯಾನಿಸು RET_FUNC_NOT_SUPPORTED 0x81
#ವ್ಯಾಖ್ಯಾನಿಸು RET_BAD_VENDOR_ID      0x83
#ವ್ಯಾಖ್ಯಾನಿಸು RET_DEVICE_NOT_ಸಿಕ್ಕಿತು   0x86
#ವ್ಯಾಖ್ಯಾನಿಸು RET_ಬಫರ್_TOO_SMALL   0x89

// installation check
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab101(ರಚನೆ bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)ನಮೂದು_pcibios32 + BUILD_BIOS_ADDR;
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// find pci device
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab102(ರಚನೆ bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = regs->si;
    ಪೂರ್ಣಾಂಕ bus = -1;
    ಆಗ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ಪೂರ್ಣಾಂಕ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            ಆದರೆ (v != id)
                ಮುಂದುವರಿ;
            ಆದರೆ (ಎಣಿಕೆ_2--)
                ಮುಂದುವರಿ;
            regs->bx = bdf;
            ಗಣ_ಸಂಕೇತ_success(regs);
            ಹಿಂತಿರುಗಿ;
        }
    }
    ಗಣ_ಸಂಕೇತ_invalid(regs, RET_DEVICE_NOT_ಸಿಕ್ಕಿತು);
}

// find class code
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab103(ರಚನೆ bregs *regs)
{
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = regs->si;
    u32 classprog = regs->ecx;
    ಪೂರ್ಣಾಂಕ bus = -1;
    ಆಗ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ಪೂರ್ಣಾಂಕ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            ಆದರೆ ((v>>8) != classprog)
                ಮುಂದುವರಿ;
            ಆದರೆ (ಎಣಿಕೆ_2--)
                ಮುಂದುವರಿ;
            regs->bx = bdf;
            ಗಣ_ಸಂಕೇತ_success(regs);
            ಹಿಂತಿರುಗಿ;
        }
    }
    ಗಣ_ಸಂಕೇತ_invalid(regs, RET_DEVICE_NOT_ಸಿಕ್ಕಿತು);
}

// read configuration byte
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab108(ರಚನೆ bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// read configuration word
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab109(ರಚನೆ bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// read configuration dword
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10a(ರಚನೆ bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// write configuration byte
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10b(ರಚನೆ bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// write configuration word
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10c(ರಚನೆ bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// write configuration dword
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10d(ರಚನೆ bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// get irq routing options
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10e(ರಚನೆ bregs *regs)
{
    ರಚನೆ pir_ಶೀರ್ಷಿಕೆ *pirtable_gf = GET_GLOBAL(PirAddr);
    ಆದರೆ (! pirtable_gf) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        ಹಿಂತಿರುಗಿ;
    }
    ರಚನೆ pir_ಶೀರ್ಷಿಕೆ *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    ರಚನೆ param_s {
        u16 ಗಾತ್ರ_2;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (ಶೂನ್ಯ*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->ಗಾತ್ರ_2);
    u16 pirsize = GET_GLOBAL(pirtable_g->ಗಾತ್ರ_2) - ಗಾತ್ರ(ರಚನೆ pir_ಶೀರ್ಷಿಕೆ);
    ಗಣ_FARVAR(regs->es, param_far->ಗಾತ್ರ_2, pirsize);
    ಆದರೆ (bufsize < pirsize) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ಬಫರ್_TOO_SMALL);
        ಹಿಂತಿರುಗಿ;
    }

    // Get dest buffer.
    ಶೂನ್ಯ *buf_far = (ಶೂನ್ಯ*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (ಶೂನ್ಯ*)(pirtable_g->slots) + get_global_ಸ್ಥಾನಾಂತರ()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab1XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

ಶೂನ್ಯ
handle_1ab1(ರಚನೆ bregs *regs)
{
    //debug_stub(regs);

    ಆದರೆ (! CONFIG_PCIBIOS) {
        ಗಣ_invalid(regs);
        ಹಿಂತಿರುಗಿ;
    }

    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x01: handle_1ab101(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: handle_1ab102(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: handle_1ab103(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x08: handle_1ab108(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x09: handle_1ab109(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0a: handle_1ab10a(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0b: handle_1ab10b(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0c: handle_1ab10c(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0d: handle_1ab10d(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0e: handle_1ab10e(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_1ab1XX(regs); ನಿಲ್ಲಿಸು;
    }
}

// Entry point for pci bios functions.
ಶೂನ್ಯ VISIBLE16 VISIBLE32SEG
handle_pcibios(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

ರಚನೆ bios32_s {
    u32 signature;
    u32 ನಮೂದು;
    u8 ಆವೃತ್ತಿ_2;
    u8 ಉದ್ದಳತೆ;
    u8 checksum;
    u8 reserved[5];
} PACKED;

ರಚನೆ bios32_s BIOS32_ಶೀರ್ಷಿಕೆ __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .ಉದ್ದಳತೆ = ಗಾತ್ರ(BIOS32_ಶೀರ್ಷಿಕೆ) / 16,
};

ಶೂನ್ಯ
bios32_init(ಶೂನ್ಯ)
{
    dprintf(3, "init bios32\n");

    BIOS32_ಶೀರ್ಷಿಕೆ.ನಮೂದು = (u32)ನಮೂದು_bios32;
    BIOS32_ಶೀರ್ಷಿಕೆ.checksum -= checksum(&BIOS32_ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ(BIOS32_ಶೀರ್ಷಿಕೆ));
}
