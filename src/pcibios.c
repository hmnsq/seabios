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
ಬಾಹ್ಯ ಶೂನ್ಯ entry_bios32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ entry_pcibios32(ಶೂನ್ಯ);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_BUFFER_TOO_SMALL   0x89

// installation check
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab101(ಸಂರಚನಾ bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab102(ಸಂರಚನಾ bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = regs->si;
    ಪೂರ್ಣಾಙ್ಕ bus = -1;
    ಯಾವತ್ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ಪೂರ್ಣಾಙ್ಕ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            ಯದಿ (v != id)
                ಅನುವರ್ತ;
            ಯದಿ (ಎಣಿಕೆ--)
                ಅನುವರ್ತ;
            regs->bx = bdf;
            set_code_success(regs);
            ಪ್ರತ್ಯಾಗಮ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab103(ಸಂರಚನಾ bregs *regs)
{
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = regs->si;
    u32 classprog = regs->ecx;
    ಪೂರ್ಣಾಙ್ಕ bus = -1;
    ಯಾವತ್ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ಪೂರ್ಣಾಙ್ಕ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            ಯದಿ ((v>>8) != classprog)
                ಅನುವರ್ತ;
            ಯದಿ (ಎಣಿಕೆ--)
                ಅನುವರ್ತ;
            regs->bx = bdf;
            set_code_success(regs);
            ಪ್ರತ್ಯಾಗಮ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab108(ಸಂರಚನಾ bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab109(ಸಂರಚನಾ bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10a(ಸಂರಚನಾ bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10b(ಸಂರಚನಾ bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10c(ಸಂರಚನಾ bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10d(ಸಂರಚನಾ bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab10e(ಸಂರಚನಾ bregs *regs)
{
    ಸಂರಚನಾ pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    ಯದಿ (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಸಂರಚನಾ pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    ಸಂರಚನಾ param_s {
        u16 size;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (ಶೂನ್ಯ*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->size);
    u16 pirsize = GET_GLOBAL(pirtable_g->size) - ಪರಿಮಾಣ(ಸಂರಚನಾ pir_header);
    SET_FARVAR(regs->es, param_far->size, pirsize);
    ಯದಿ (bufsize < pirsize) {
        set_code_invalid(regs, RET_BUFFER_TOO_SMALL);
        ಪ್ರತ್ಯಾಗಮ;
    }

    // Get dest buffer.
    ಶೂನ್ಯ *buf_far = (ಶೂನ್ಯ*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (ಶೂನ್ಯ*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1ab1XX(ಸಂರಚನಾ bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

ಶೂನ್ಯ
handle_1ab1(ಸಂರಚನಾ bregs *regs)
{
    //debug_stub(regs);

    ಯದಿ (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x01: handle_1ab101(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: handle_1ab102(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x03: handle_1ab103(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x08: handle_1ab108(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x09: handle_1ab109(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0a: handle_1ab10a(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0b: handle_1ab10b(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0c: handle_1ab10c(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0d: handle_1ab10d(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0e: handle_1ab10e(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   handle_1ab1XX(regs); ಭಙ್ಗ;
    }
}

// Entry point for pci bios functions.
ಶೂನ್ಯ VISIBLE16 VISIBLE32SEG
handle_pcibios(ಸಂರಚನಾ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

ಸಂರಚನಾ bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 length;
    u8 checksum;
    u8 reserved[5];
} PACKED;

ಸಂರಚನಾ bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .length = ಪರಿಮಾಣ(BIOS32HEADER) / 16,
};

ಶೂನ್ಯ
bios32_init(ಶೂನ್ಯ)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, ಪರಿಮಾಣ(BIOS32HEADER));
}
