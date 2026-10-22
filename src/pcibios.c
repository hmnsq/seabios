// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "hw/pci.h" // pci_config_readl
#അന്തര്ഭാവ "hw/pcidevice.h" // MaxPCIBus
#അന്തര്ഭാവ "hw/pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "std/pirtable.h" // struct pir_header
#അന്തര്ഭാവ "string.h" // checksum
#അന്തര്ഭാവ "util.h" // handle_1ab1

// romlayout.S
ബാഹ്യ ശൂന്യം entry_bios32(ശൂന്യം);
ബാഹ്യ ശൂന്യം entry_pcibios32(ശൂന്യം);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_BUFFER_TOO_SMALL   0x89

// installation check
സ്ഥിതം ശൂന്യം
handle_1ab101(ഘടന bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
സ്ഥിതം ശൂന്യം
handle_1ab102(ഘടന bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    പൂർണ്ണസംഖ്യ എണ്ണം = regs->si;
    പൂർണ്ണസംഖ്യ bus = -1;
    ആകുമ്പോൾ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        പൂർണ്ണസംഖ്യ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            എങ്കിൽ (v != id)
                തുടരുക;
            എങ്കിൽ (എണ്ണം--)
                തുടരുക;
            regs->bx = bdf;
            set_code_success(regs);
            തിരികെ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
സ്ഥിതം ശൂന്യം
handle_1ab103(ഘടന bregs *regs)
{
    പൂർണ്ണസംഖ്യ എണ്ണം = regs->si;
    u32 classprog = regs->ecx;
    പൂർണ്ണസംഖ്യ bus = -1;
    ആകുമ്പോൾ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        പൂർണ്ണസംഖ്യ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            എങ്കിൽ ((v>>8) != classprog)
                തുടരുക;
            എങ്കിൽ (എണ്ണം--)
                തുടരുക;
            regs->bx = bdf;
            set_code_success(regs);
            തിരികെ;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
സ്ഥിതം ശൂന്യം
handle_1ab108(ഘടന bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
സ്ഥിതം ശൂന്യം
handle_1ab109(ഘടന bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
സ്ഥിതം ശൂന്യം
handle_1ab10a(ഘടന bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
സ്ഥിതം ശൂന്യം
handle_1ab10b(ഘടന bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
സ്ഥിതം ശൂന്യം
handle_1ab10c(ഘടന bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
സ്ഥിതം ശൂന്യം
handle_1ab10d(ഘടന bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
സ്ഥിതം ശൂന്യം
handle_1ab10e(ഘടന bregs *regs)
{
    ഘടന pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    എങ്കിൽ (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        തിരികെ;
    }
    ഘടന pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    ഘടന param_s {
        u16 size;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (ശൂന്യം*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->size);
    u16 pirsize = GET_GLOBAL(pirtable_g->size) - വലുപ്പം(ഘടന pir_header);
    SET_FARVAR(regs->es, param_far->size, pirsize);
    എങ്കിൽ (bufsize < pirsize) {
        set_code_invalid(regs, RET_BUFFER_TOO_SMALL);
        തിരികെ;
    }

    // Get dest buffer.
    ശൂന്യം *buf_far = (ശൂന്യം*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (ശൂന്യം*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_1ab1XX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

ശൂന്യം
handle_1ab1(ഘടന bregs *regs)
{
    //debug_stub(regs);

    എങ്കിൽ (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        തിരികെ;
    }

    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x01: handle_1ab101(regs); നിർത്തുക;
    അവസ്ഥ 0x02: handle_1ab102(regs); നിർത്തുക;
    അവസ്ഥ 0x03: handle_1ab103(regs); നിർത്തുക;
    അവസ്ഥ 0x08: handle_1ab108(regs); നിർത്തുക;
    അവസ്ഥ 0x09: handle_1ab109(regs); നിർത്തുക;
    അവസ്ഥ 0x0a: handle_1ab10a(regs); നിർത്തുക;
    അവസ്ഥ 0x0b: handle_1ab10b(regs); നിർത്തുക;
    അവസ്ഥ 0x0c: handle_1ab10c(regs); നിർത്തുക;
    അവസ്ഥ 0x0d: handle_1ab10d(regs); നിർത്തുക;
    അവസ്ഥ 0x0e: handle_1ab10e(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_1ab1XX(regs); നിർത്തുക;
    }
}

// Entry point for pci bios functions.
ശൂന്യം VISIBLE16 VISIBLE32SEG
handle_pcibios(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

ഘടന bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 length;
    u8 checksum;
    u8 reserved[5];
} PACKED;

ഘടന bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .length = വലുപ്പം(BIOS32HEADER) / 16,
};

ശൂന്യം
bios32_init(ശൂന്യം)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, വലുപ്പം(BIOS32HEADER));
}
