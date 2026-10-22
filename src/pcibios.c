// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_config_readl
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // MaxPCIBus
#ଅନ୍ତର୍ଭାଵ "hw/pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "std/pirtable.h" // struct pir_header
#ଅନ୍ତର୍ଭାଵ "string.h" // checksum
#ଅନ୍ତର୍ଭାଵ "util.h" // handle_1ab1

// romlayout.S
ବାହ୍ୟ ଶୂନ୍ୟ entry_bios32(ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ entry_pcibios32(ଶୂନ୍ୟ);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_BUFFER_TOO_SMALL   0x89

// installation check
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab101(ସଂରଚନା bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab102(ସଂରଚନା bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = regs->si;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus = -1;
    ଯେତେବେଳେ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            ଯଦି (v != id)
                ଜାରି;
            ଯଦି (ଗଣନା_2--)
                ଜାରି;
            regs->bx = bdf;
            set_code_success(regs);
            ଫେରନ୍ତୁ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab103(ସଂରଚନା bregs *regs)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = regs->si;
    u32 classprog = regs->ecx;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus = -1;
    ଯେତେବେଳେ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            ଯଦି ((v>>8) != classprog)
                ଜାରି;
            ଯଦି (ଗଣନା_2--)
                ଜାରି;
            regs->bx = bdf;
            set_code_success(regs);
            ଫେରନ୍ତୁ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab108(ସଂରଚନା bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab109(ସଂରଚନା bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab10a(ସଂରଚନା bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab10b(ସଂରଚନା bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab10c(ସଂରଚନା bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab10d(ସଂରଚନା bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab10e(ସଂରଚନା bregs *regs)
{
    ସଂରଚନା pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    ଯଦି (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        ଫେରନ୍ତୁ;
    }
    ସଂରଚନା pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    ସଂରଚନା param_s {
        u16 size;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (ଶୂନ୍ୟ*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->size);
    u16 pirsize = GET_GLOBAL(pirtable_g->size) - ଆକାର(ସଂରଚନା pir_header);
    SET_FARVAR(regs->es, param_far->size, pirsize);
    ଯଦି (bufsize < pirsize) {
        set_code_invalid(regs, RET_BUFFER_TOO_SMALL);
        ଫେରନ୍ତୁ;
    }

    // Get dest buffer.
    ଶୂନ୍ୟ *buf_far = (ଶୂନ୍ୟ*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (ଶୂନ୍ୟ*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_1ab1XX(ସଂରଚନା bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

ଶୂନ୍ୟ
handle_1ab1(ସଂରଚନା bregs *regs)
{
    //debug_stub(regs);

    ଯଦି (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        ଫେରନ୍ତୁ;
    }

    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x01: handle_1ab101(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: handle_1ab102(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: handle_1ab103(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x08: handle_1ab108(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x09: handle_1ab109(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0a: handle_1ab10a(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0b: handle_1ab10b(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0c: handle_1ab10c(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0d: handle_1ab10d(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0e: handle_1ab10e(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_1ab1XX(regs); ଭାଙ୍ଗ;
    }
}

// Entry point for pci bios functions.
ଶୂନ୍ୟ VISIBLE16 VISIBLE32SEG
handle_pcibios(ସଂରଚନା bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

ସଂରଚନା bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 length;
    u8 checksum;
    u8 reserved[5];
} PACKED;

ସଂରଚନା bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .length = ଆକାର(BIOS32HEADER) / 16,
};

ଶୂନ୍ୟ
bios32_init(ଶୂନ୍ୟ)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, ଆକାର(BIOS32HEADER));
}
