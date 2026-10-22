// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "hw/pci.h" // pci_config_readl
#অন্তর্ভাব "hw/pcidevice.h" // MaxPCIBus
#অন্তর্ভাব "hw/pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "std/pirtable.h" // struct pir_header
#অন্তর্ভাব "string.h" // checksum
#অন্তর্ভাব "util.h" // handle_1ab1

// romlayout.S
বাহ্যিক শূন্য entry_bios32(শূন্য);
বাহ্যিক শূন্য entry_pcibios32(শূন্য);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_বাফার_TOO_SMALL   0x89

// installation check
স্থির শূন্য
handle_1ab101(কাঠামো bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
স্থির শূন্য
handle_1ab102(কাঠামো bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    পূর্ণসংখ্যা গুনতি = regs->si;
    পূর্ণসংখ্যা bus = -1;
    যখন (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        পূর্ণসংখ্যা bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            যদি (v != id)
                চালু;
            যদি (গুনতি--)
                চালু;
            regs->bx = bdf;
            set_code_success(regs);
            ফেরত;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
স্থির শূন্য
handle_1ab103(কাঠামো bregs *regs)
{
    পূর্ণসংখ্যা গুনতি = regs->si;
    u32 classprog = regs->ecx;
    পূর্ণসংখ্যা bus = -1;
    যখন (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        পূর্ণসংখ্যা bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            যদি ((v>>8) != classprog)
                চালু;
            যদি (গুনতি--)
                চালু;
            regs->bx = bdf;
            set_code_success(regs);
            ফেরত;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
স্থির শূন্য
handle_1ab108(কাঠামো bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
স্থির শূন্য
handle_1ab109(কাঠামো bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
স্থির শূন্য
handle_1ab10a(কাঠামো bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
স্থির শূন্য
handle_1ab10b(কাঠামো bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
স্থির শূন্য
handle_1ab10c(কাঠামো bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
স্থির শূন্য
handle_1ab10d(কাঠামো bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
স্থির শূন্য
handle_1ab10e(কাঠামো bregs *regs)
{
    কাঠামো pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    যদি (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        ফেরত;
    }
    কাঠামো pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    কাঠামো param_s {
        u16 আকার_2;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (শূন্য*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->আকার_2);
    u16 pirsize = GET_GLOBAL(pirtable_g->আকার_2) - আকার(কাঠামো pir_header);
    SET_FARVAR(regs->es, param_far->আকার_2, pirsize);
    যদি (bufsize < pirsize) {
        set_code_invalid(regs, RET_বাফার_TOO_SMALL);
        ফেরত;
    }

    // Get dest buffer.
    শূন্য *buf_far = (শূন্য*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (শূন্য*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

স্থির শূন্য
handle_1ab1XX(কাঠামো bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

শূন্য
handle_1ab1(কাঠামো bregs *regs)
{
    //debug_stub(regs);

    যদি (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        ফেরত;
    }

    নির্বাচন (regs->al) {
    ক্ষেত্র 0x01: handle_1ab101(regs); বিরতি;
    ক্ষেত্র 0x02: handle_1ab102(regs); বিরতি;
    ক্ষেত্র 0x03: handle_1ab103(regs); বিরতি;
    ক্ষেত্র 0x08: handle_1ab108(regs); বিরতি;
    ক্ষেত্র 0x09: handle_1ab109(regs); বিরতি;
    ক্ষেত্র 0x0a: handle_1ab10a(regs); বিরতি;
    ক্ষেত্র 0x0b: handle_1ab10b(regs); বিরতি;
    ক্ষেত্র 0x0c: handle_1ab10c(regs); বিরতি;
    ক্ষেত্র 0x0d: handle_1ab10d(regs); বিরতি;
    ক্ষেত্র 0x0e: handle_1ab10e(regs); বিরতি;
    পূর্বনির্ধারিত:   handle_1ab1XX(regs); বিরতি;
    }
}

// Entry point for pci bios functions.
শূন্য VISIBLE16 VISIBLE32SEG
handle_pcibios(কাঠামো bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

কাঠামো bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 দৈর্ঘ্য;
    u8 checksum;
    u8 reserved[5];
} PACKED;

কাঠামো bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .দৈর্ঘ্য = আকার(BIOS32HEADER) / 16,
};

শূন্য
bios32_init(শূন্য)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, আকার(BIOS32HEADER));
}
