// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "hw/pci.h" // pci_config_readl
#અન્તર્ભાવ "hw/pcidevice.h" // MaxPCIBus
#અન્તર્ભાવ "hw/pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "std/pirtable.h" // struct pir_header
#અન્તર્ભાવ "string.h" // checksum
#અન્તર્ભાવ "util.h" // handle_1ab1

// romlayout.S
બાહ્ય શૂન્ય entry_bios32(શૂન્ય);
બાહ્ય શૂન્ય entry_pcibios32(શૂન્ય);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_BUFFER_TOO_SMALL   0x89

// installation check
સ્થિર શૂન્ય
handle_1ab101(સંરચના bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
સ્થિર શૂન્ય
handle_1ab102(સંરચના bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    પૂર્ણાંક ગણતરી = regs->si;
    પૂર્ણાંક bus = -1;
    જ્યારે (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        પૂર્ણાંક bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            જો (v != id)
                ચાલુ;
            જો (ગણતરી--)
                ચાલુ;
            regs->bx = bdf;
            set_code_success(regs);
            પાછા;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
સ્થિર શૂન્ય
handle_1ab103(સંરચના bregs *regs)
{
    પૂર્ણાંક ગણતરી = regs->si;
    u32 classprog = regs->ecx;
    પૂર્ણાંક bus = -1;
    જ્યારે (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        પૂર્ણાંક bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            જો ((v>>8) != classprog)
                ચાલુ;
            જો (ગણતરી--)
                ચાલુ;
            regs->bx = bdf;
            set_code_success(regs);
            પાછા;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
સ્થિર શૂન્ય
handle_1ab108(સંરચના bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
સ્થિર શૂન્ય
handle_1ab109(સંરચના bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
સ્થિર શૂન્ય
handle_1ab10a(સંરચના bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
સ્થિર શૂન્ય
handle_1ab10b(સંરચના bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
સ્થિર શૂન્ય
handle_1ab10c(સંરચના bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
સ્થિર શૂન્ય
handle_1ab10d(સંરચના bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
સ્થિર શૂન્ય
handle_1ab10e(સંરચના bregs *regs)
{
    સંરચના pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    જો (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        પાછા;
    }
    સંરચના pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    સંરચના param_s {
        u16 size;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (શૂન્ય*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->size);
    u16 pirsize = GET_GLOBAL(pirtable_g->size) - કદ(સંરચના pir_header);
    SET_FARVAR(regs->es, param_far->size, pirsize);
    જો (bufsize < pirsize) {
        set_code_invalid(regs, RET_BUFFER_TOO_SMALL);
        પાછા;
    }

    // Get dest buffer.
    શૂન્ય *buf_far = (શૂન્ય*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (શૂન્ય*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

સ્થિર શૂન્ય
handle_1ab1XX(સંરચના bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

શૂન્ય
handle_1ab1(સંરચના bregs *regs)
{
    //debug_stub(regs);

    જો (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        પાછા;
    }

    પસંદગી (regs->al) {
    સ્થિતિ 0x01: handle_1ab101(regs); તોડો;
    સ્થિતિ 0x02: handle_1ab102(regs); તોડો;
    સ્થિતિ 0x03: handle_1ab103(regs); તોડો;
    સ્થિતિ 0x08: handle_1ab108(regs); તોડો;
    સ્થિતિ 0x09: handle_1ab109(regs); તોડો;
    સ્થિતિ 0x0a: handle_1ab10a(regs); તોડો;
    સ્થિતિ 0x0b: handle_1ab10b(regs); તોડો;
    સ્થિતિ 0x0c: handle_1ab10c(regs); તોડો;
    સ્થિતિ 0x0d: handle_1ab10d(regs); તોડો;
    સ્થિતિ 0x0e: handle_1ab10e(regs); તોડો;
    મૂળભૂત:   handle_1ab1XX(regs); તોડો;
    }
}

// Entry point for pci bios functions.
શૂન્ય VISIBLE16 VISIBLE32SEG
handle_pcibios(સંરચના bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

સંરચના bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 length;
    u8 checksum;
    u8 reserved[5];
} PACKED;

સંરચના bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .length = કદ(BIOS32HEADER) / 16,
};

શૂન્ય
bios32_init(શૂન્ય)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, કદ(BIOS32HEADER));
}
