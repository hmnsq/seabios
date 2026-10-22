// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "bregs.h" // struct bregs
#समावेश "hw/pci.h" // pci_config_readl
#समावेश "hw/pcidevice.h" // MaxPCIBus
#समावेश "hw/pci_regs.h" // PCI_VENDOR_ID
#समावेश "output.h" // dprintf
#समावेश "std/pirtable.h" // struct pir_header
#समावेश "string.h" // checksum
#समावेश "util.h" // handle_1ab1

// romlayout.S
बाह्य व्योम entry_bios32(व्योम);
बाह्य व्योम entry_pcibios32(व्योम);

#घोषणा RET_FUNC_NOT_SUPPORTED 0x81
#घोषणा RET_BAD_VENDOR_ID      0x83
#घोषणा RET_DEVICE_NOT_FOUND   0x86
#घोषणा RET_बफर_TOO_SMALL   0x89

// installation check
अटल व्योम
handle_1ab101(काष्ठा bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
अटल व्योम
handle_1ab102(काष्ठा bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    पूर्णांक गिनती = regs->si;
    पूर्णांक bus = -1;
    जबतक (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        पूर्णांक bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            अगर (v != id)
                जारी;
            अगर (गिनती--)
                जारी;
            regs->bx = bdf;
            set_code_success(regs);
            वापस;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
अटल व्योम
handle_1ab103(काष्ठा bregs *regs)
{
    पूर्णांक गिनती = regs->si;
    u32 classprog = regs->ecx;
    पूर्णांक bus = -1;
    जबतक (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        पूर्णांक bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            अगर ((v>>8) != classprog)
                जारी;
            अगर (गिनती--)
                जारी;
            regs->bx = bdf;
            set_code_success(regs);
            वापस;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
अटल व्योम
handle_1ab108(काष्ठा bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
अटल व्योम
handle_1ab109(काष्ठा bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
अटल व्योम
handle_1ab10a(काष्ठा bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
अटल व्योम
handle_1ab10b(काष्ठा bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
अटल व्योम
handle_1ab10c(काष्ठा bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
अटल व्योम
handle_1ab10d(काष्ठा bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
अटल व्योम
handle_1ab10e(काष्ठा bregs *regs)
{
    काष्ठा pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    अगर (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        वापस;
    }
    काष्ठा pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    काष्ठा param_s {
        u16 आकार;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (व्योम*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->आकार);
    u16 pirsize = GET_GLOBAL(pirtable_g->आकार) - माप(काष्ठा pir_header);
    SET_FARVAR(regs->es, param_far->आकार, pirsize);
    अगर (bufsize < pirsize) {
        set_code_invalid(regs, RET_बफर_TOO_SMALL);
        वापस;
    }

    // Get dest buffer.
    व्योम *buf_far = (व्योम*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (व्योम*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

अटल व्योम
handle_1ab1XX(काष्ठा bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

व्योम
handle_1ab1(काष्ठा bregs *regs)
{
    //debug_stub(regs);

    अगर (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        वापस;
    }

    चयन (regs->al) {
    हाल 0x01: handle_1ab101(regs); अवरोध;
    हाल 0x02: handle_1ab102(regs); अवरोध;
    हाल 0x03: handle_1ab103(regs); अवरोध;
    हाल 0x08: handle_1ab108(regs); अवरोध;
    हाल 0x09: handle_1ab109(regs); अवरोध;
    हाल 0x0a: handle_1ab10a(regs); अवरोध;
    हाल 0x0b: handle_1ab10b(regs); अवरोध;
    हाल 0x0c: handle_1ab10c(regs); अवरोध;
    हाल 0x0d: handle_1ab10d(regs); अवरोध;
    हाल 0x0e: handle_1ab10e(regs); अवरोध;
    शेष:   handle_1ab1XX(regs); अवरोध;
    }
}

// Entry point for pci bios functions.
व्योम VISIBLE16 VISIBLE32SEG
handle_pcibios(काष्ठा bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

काष्ठा bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 लंबाई;
    u8 checksum;
    u8 reserved[5];
} PACKED;

काष्ठा bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .लंबाई = माप(BIOS32HEADER) / 16,
};

व्योम
bios32_init(व्योम)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, माप(BIOS32HEADER));
}
