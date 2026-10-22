// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "hw/pci.h" // pci_config_readl
#अन्तर्भाव "hw/pcidevice.h" // MaxPCIBus
#अन्तर्भाव "hw/pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/pirtable.h" // struct pir_header
#अन्तर्भाव "string.h" // checksum
#अन्तर्भाव "util.h" // handle_1ab1

// romlayout.S
बाह्य शून्य entry_bios32(शून्य);
बाह्य शून्य entry_pcibios32(शून्य);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_BUFFER_TOO_SMALL   0x89

// installation check
स्थिर शून्य
handle_1ab101(संरचना bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
स्थिर शून्य
handle_1ab102(संरचना bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    पूर्णाङ्क गन्ती = regs->si;
    पूर्णाङ्क bus = -1;
    जबसम्म (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        पूर्णाङ्क bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            यदि (v != id)
                जारी;
            यदि (गन्ती--)
                जारी;
            regs->bx = bdf;
            set_code_success(regs);
            फर्काऊ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
स्थिर शून्य
handle_1ab103(संरचना bregs *regs)
{
    पूर्णाङ्क गन्ती = regs->si;
    u32 classprog = regs->ecx;
    पूर्णाङ्क bus = -1;
    जबसम्म (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        पूर्णाङ्क bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            यदि ((v>>8) != classprog)
                जारी;
            यदि (गन्ती--)
                जारी;
            regs->bx = bdf;
            set_code_success(regs);
            फर्काऊ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
स्थिर शून्य
handle_1ab108(संरचना bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
स्थिर शून्य
handle_1ab109(संरचना bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
स्थिर शून्य
handle_1ab10a(संरचना bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
स्थिर शून्य
handle_1ab10b(संरचना bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
स्थिर शून्य
handle_1ab10c(संरचना bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
स्थिर शून्य
handle_1ab10d(संरचना bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
स्थिर शून्य
handle_1ab10e(संरचना bregs *regs)
{
    संरचना pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    यदि (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        फर्काऊ;
    }
    संरचना pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    संरचना param_s {
        u16 size;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (शून्य*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->size);
    u16 pirsize = GET_GLOBAL(pirtable_g->size) - परिमाण(संरचना pir_header);
    SET_FARVAR(regs->es, param_far->size, pirsize);
    यदि (bufsize < pirsize) {
        set_code_invalid(regs, RET_BUFFER_TOO_SMALL);
        फर्काऊ;
    }

    // Get dest buffer.
    शून्य *buf_far = (शून्य*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (शून्य*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

स्थिर शून्य
handle_1ab1XX(संरचना bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

शून्य
handle_1ab1(संरचना bregs *regs)
{
    //debug_stub(regs);

    यदि (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        फर्काऊ;
    }

    विकल्प (regs->al) {
    पक्ष 0x01: handle_1ab101(regs); रोक;
    पक्ष 0x02: handle_1ab102(regs); रोक;
    पक्ष 0x03: handle_1ab103(regs); रोक;
    पक्ष 0x08: handle_1ab108(regs); रोक;
    पक्ष 0x09: handle_1ab109(regs); रोक;
    पक्ष 0x0a: handle_1ab10a(regs); रोक;
    पक्ष 0x0b: handle_1ab10b(regs); रोक;
    पक्ष 0x0c: handle_1ab10c(regs); रोक;
    पक्ष 0x0d: handle_1ab10d(regs); रोक;
    पक्ष 0x0e: handle_1ab10e(regs); रोक;
    स्वभाव:   handle_1ab1XX(regs); रोक;
    }
}

// Entry point for pci bios functions.
शून्य VISIBLE16 VISIBLE32SEG
handle_pcibios(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

संरचना bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 length;
    u8 checksum;
    u8 reserved[5];
} PACKED;

संरचना bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .length = परिमाण(BIOS32HEADER) / 16,
};

शून्य
bios32_init(शून्य)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, परिमाण(BIOS32HEADER));
}
