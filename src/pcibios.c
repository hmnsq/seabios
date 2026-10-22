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
বাহ্য শূন্য entry_bios32(শূন্য);
বাহ্য শূন্য entry_pcibios32(শূন্য);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_BUFFER_TOO_SMALL   0x89

// installation check
স্থির শূন্য
handle_1ab101(সংরচনা bregs *regs)
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
handle_1ab102(সংরচনা bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    পূর্ণাঙ্ক গুনতি = regs->si;
    পূর্ণাঙ্ক bus = -1;
    যাবত্ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        পূর্ণাঙ্ক bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            যদি (v != id)
                অনুবর্ত;
            যদি (গুনতি--)
                অনুবর্ত;
            regs->bx = bdf;
            set_code_success(regs);
            প্রত্যাগম;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
স্থির শূন্য
handle_1ab103(সংরচনা bregs *regs)
{
    পূর্ণাঙ্ক গুনতি = regs->si;
    u32 classprog = regs->ecx;
    পূর্ণাঙ্ক bus = -1;
    যাবত্ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        পূর্ণাঙ্ক bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            যদি ((v>>8) != classprog)
                অনুবর্ত;
            যদি (গুনতি--)
                অনুবর্ত;
            regs->bx = bdf;
            set_code_success(regs);
            প্রত্যাগম;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
স্থির শূন্য
handle_1ab108(সংরচনা bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
স্থির শূন্য
handle_1ab109(সংরচনা bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
স্থির শূন্য
handle_1ab10a(সংরচনা bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
স্থির শূন্য
handle_1ab10b(সংরচনা bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
স্থির শূন্য
handle_1ab10c(সংরচনা bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
স্থির শূন্য
handle_1ab10d(সংরচনা bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
স্থির শূন্য
handle_1ab10e(সংরচনা bregs *regs)
{
    সংরচনা pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    যদি (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        প্রত্যাগম;
    }
    সংরচনা pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    সংরচনা param_s {
        u16 size;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (শূন্য*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->size);
    u16 pirsize = GET_GLOBAL(pirtable_g->size) - পরিমাণ(সংরচনা pir_header);
    SET_FARVAR(regs->es, param_far->size, pirsize);
    যদি (bufsize < pirsize) {
        set_code_invalid(regs, RET_BUFFER_TOO_SMALL);
        প্রত্যাগম;
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
handle_1ab1XX(সংরচনা bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

শূন্য
handle_1ab1(সংরচনা bregs *regs)
{
    //debug_stub(regs);

    যদি (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        প্রত্যাগম;
    }

    বিকল্প (regs->al) {
    পক্ষ 0x01: handle_1ab101(regs); ভঙ্গ;
    পক্ষ 0x02: handle_1ab102(regs); ভঙ্গ;
    পক্ষ 0x03: handle_1ab103(regs); ভঙ্গ;
    পক্ষ 0x08: handle_1ab108(regs); ভঙ্গ;
    পক্ষ 0x09: handle_1ab109(regs); ভঙ্গ;
    পক্ষ 0x0a: handle_1ab10a(regs); ভঙ্গ;
    পক্ষ 0x0b: handle_1ab10b(regs); ভঙ্গ;
    পক্ষ 0x0c: handle_1ab10c(regs); ভঙ্গ;
    পক্ষ 0x0d: handle_1ab10d(regs); ভঙ্গ;
    পক্ষ 0x0e: handle_1ab10e(regs); ভঙ্গ;
    স্বভাব:   handle_1ab1XX(regs); ভঙ্গ;
    }
}

// Entry point for pci bios functions.
শূন্য VISIBLE16 VISIBLE32SEG
handle_pcibios(সংরচনা bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

সংরচনা bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 length;
    u8 checksum;
    u8 reserved[5];
} PACKED;

সংরচনা bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .length = পরিমাণ(BIOS32HEADER) / 16,
};

শূন্য
bios32_init(শূন্য)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, পরিমাণ(BIOS32HEADER));
}
