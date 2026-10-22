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
बाह्य शून्य प्रविष्टिः_bios32(शून्य);
बाह्य शून्य प्रविष्टिः_pcibios32(शून्य);

#परिभाषय RET_FUNC_NOT_SUPPORTED 0x81
#परिभाषय RET_BAD_VENDOR_ID      0x83
#परिभाषय RET_DEVICE_NOT_प्राप्तम्   0x86
#परिभाषय RET_बफर_TOO_SMALL   0x89

// installation check
स्थिर शून्य
handle_1ab101(संरचना bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)प्रविष्टिः_pcibios32 + BUILD_BIOS_ADDR;
    समुच्चयः_संहिता_success(regs);
}

// find pci device
स्थिर शून्य
handle_1ab102(संरचना bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    पूर्णाङ्क गणनम् = regs->si;
    पूर्णाङ्क bus = -1;
    यावत् (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        पूर्णाङ्क bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            यदि (v != id)
                अनुवर्त;
            यदि (गणनम्--)
                अनुवर्त;
            regs->bx = bdf;
            समुच्चयः_संहिता_success(regs);
            प्रत्यागम;
        }
    }
    समुच्चयः_संहिता_invalid(regs, RET_DEVICE_NOT_प्राप्तम्);
}

// find class code
स्थिर शून्य
handle_1ab103(संरचना bregs *regs)
{
    पूर्णाङ्क गणनम् = regs->si;
    u32 classprog = regs->ecx;
    पूर्णाङ्क bus = -1;
    यावत् (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        पूर्णाङ्क bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            यदि ((v>>8) != classprog)
                अनुवर्त;
            यदि (गणनम्--)
                अनुवर्त;
            regs->bx = bdf;
            समुच्चयः_संहिता_success(regs);
            प्रत्यागम;
        }
    }
    समुच्चयः_संहिता_invalid(regs, RET_DEVICE_NOT_प्राप्तम्);
}

// read configuration byte
स्थिर शून्य
handle_1ab108(संरचना bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    समुच्चयः_संहिता_success(regs);
}

// read configuration word
स्थिर शून्य
handle_1ab109(संरचना bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    समुच्चयः_संहिता_success(regs);
}

// read configuration dword
स्थिर शून्य
handle_1ab10a(संरचना bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    समुच्चयः_संहिता_success(regs);
}

// write configuration byte
स्थिर शून्य
handle_1ab10b(संरचना bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    समुच्चयः_संहिता_success(regs);
}

// write configuration word
स्थिर शून्य
handle_1ab10c(संरचना bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    समुच्चयः_संहिता_success(regs);
}

// write configuration dword
स्थिर शून्य
handle_1ab10d(संरचना bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    समुच्चयः_संहिता_success(regs);
}

// get irq routing options
स्थिर शून्य
handle_1ab10e(संरचना bregs *regs)
{
    संरचना pir_शीर्षकम् *pirtable_gf = GET_GLOBAL(PirAddr);
    यदि (! pirtable_gf) {
        समुच्चयः_संहिता_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        प्रत्यागम;
    }
    संरचना pir_शीर्षकम् *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    संरचना param_s {
        u16 आकार;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (शून्य*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->आकार);
    u16 pirsize = GET_GLOBAL(pirtable_g->आकार) - परिमाण(संरचना pir_शीर्षकम्);
    समुच्चयः_FARVAR(regs->es, param_far->आकार, pirsize);
    यदि (bufsize < pirsize) {
        समुच्चयः_संहिता_invalid(regs, RET_बफर_TOO_SMALL);
        प्रत्यागम;
    }

    // Get dest buffer.
    शून्य *buf_far = (शून्य*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (शून्य*)(pirtable_g->slots) + get_global_विस्थापनम्()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    समुच्चयः_संहिता_success(regs);
}

स्थिर शून्य
handle_1ab1XX(संरचना bregs *regs)
{
    समुच्चयः_संहिता_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

शून्य
handle_1ab1(संरचना bregs *regs)
{
    //debug_stub(regs);

    यदि (! CONFIG_PCIBIOS) {
        समुच्चयः_invalid(regs);
        प्रत्यागम;
    }

    विकल्प (regs->al) {
    पक्ष 0x01: handle_1ab101(regs); भङ्ग;
    पक्ष 0x02: handle_1ab102(regs); भङ्ग;
    पक्ष 0x03: handle_1ab103(regs); भङ्ग;
    पक्ष 0x08: handle_1ab108(regs); भङ्ग;
    पक्ष 0x09: handle_1ab109(regs); भङ्ग;
    पक्ष 0x0a: handle_1ab10a(regs); भङ्ग;
    पक्ष 0x0b: handle_1ab10b(regs); भङ्ग;
    पक्ष 0x0c: handle_1ab10c(regs); भङ्ग;
    पक्ष 0x0d: handle_1ab10d(regs); भङ्ग;
    पक्ष 0x0e: handle_1ab10e(regs); भङ्ग;
    स्वभाव:   handle_1ab1XX(regs); भङ्ग;
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
    u32 प्रविष्टिः;
    u8 संस्करणम्_2;
    u8 दीर्घता;
    u8 checksum;
    u8 reserved[5];
} PACKED;

संरचना bios32_s BIOS32_शीर्षकम् __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .दीर्घता = परिमाण(BIOS32_शीर्षकम्) / 16,
};

शून्य
bios32_init(शून्य)
{
    dprintf(3, "init bios32\n");

    BIOS32_शीर्षकम्.प्रविष्टिः = (u32)प्रविष्टिः_bios32;
    BIOS32_शीर्षकम्.checksum -= checksum(&BIOS32_शीर्षकम्, परिमाण(BIOS32_शीर्षकम्));
}
