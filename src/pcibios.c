// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_readl
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // MaxPCIBus
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "std/pirtable.h" // struct pir_header
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // checksum
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // handle_1ab1

// romlayout.S
ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_bios32(ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_pcibios32(ਖਾਲੀ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_FUNC_NOT_SUPPORTED 0x81
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_BAD_VENDOR_ID      0x83
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_DEVICE_NOT_ਮਿਲਿਆ   0x86
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_ਬਫਰ_TOO_SMALL   0x89

// installation check
ਸਥਿਰ ਖਾਲੀ
handle_1ab101(ਬਣਤਰ bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)ਇੰਦਰਾਜ_pcibios32 + BUILD_BIOS_ADDR;
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// find pci device
ਸਥਿਰ ਖਾਲੀ
handle_1ab102(ਬਣਤਰ bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    ਪੂਰਨ ਗਿਣਤੀ_2 = regs->si;
    ਪੂਰਨ bus = -1;
    ਜਦੋਂ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ਪੂਰਨ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            ਜੇ (v != id)
                ਜਾਰੀ;
            ਜੇ (ਗਿਣਤੀ_2--)
                ਜਾਰੀ;
            regs->bx = bdf;
            ਸੈੱਟ_ਕੋਡ_success(regs);
            ਵਾਪਸ;
        }
    }
    ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_DEVICE_NOT_ਮਿਲਿਆ);
}

// find class code
ਸਥਿਰ ਖਾਲੀ
handle_1ab103(ਬਣਤਰ bregs *regs)
{
    ਪੂਰਨ ਗਿਣਤੀ_2 = regs->si;
    u32 classprog = regs->ecx;
    ਪੂਰਨ bus = -1;
    ਜਦੋਂ (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        ਪੂਰਨ bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            ਜੇ ((v>>8) != classprog)
                ਜਾਰੀ;
            ਜੇ (ਗਿਣਤੀ_2--)
                ਜਾਰੀ;
            regs->bx = bdf;
            ਸੈੱਟ_ਕੋਡ_success(regs);
            ਵਾਪਸ;
        }
    }
    ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_DEVICE_NOT_ਮਿਲਿਆ);
}

// read configuration byte
ਸਥਿਰ ਖਾਲੀ
handle_1ab108(ਬਣਤਰ bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// read configuration word
ਸਥਿਰ ਖਾਲੀ
handle_1ab109(ਬਣਤਰ bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// read configuration dword
ਸਥਿਰ ਖਾਲੀ
handle_1ab10a(ਬਣਤਰ bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// write configuration byte
ਸਥਿਰ ਖਾਲੀ
handle_1ab10b(ਬਣਤਰ bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// write configuration word
ਸਥਿਰ ਖਾਲੀ
handle_1ab10c(ਬਣਤਰ bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// write configuration dword
ਸਥਿਰ ਖਾਲੀ
handle_1ab10d(ਬਣਤਰ bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// get irq routing options
ਸਥਿਰ ਖਾਲੀ
handle_1ab10e(ਬਣਤਰ bregs *regs)
{
    ਬਣਤਰ pir_ਸਿਰਲੇਖ *pirtable_gf = GET_GLOBAL(PirAddr);
    ਜੇ (! pirtable_gf) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        ਵਾਪਸ;
    }
    ਬਣਤਰ pir_ਸਿਰਲੇਖ *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    ਬਣਤਰ param_s {
        u16 ਮਾਪ;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (ਖਾਲੀ*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->ਮਾਪ);
    u16 pirsize = GET_GLOBAL(pirtable_g->ਮਾਪ) - ਆਕਾਰ(ਬਣਤਰ pir_ਸਿਰਲੇਖ);
    ਸੈੱਟ_FARVAR(regs->es, param_far->ਮਾਪ, pirsize);
    ਜੇ (bufsize < pirsize) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ਬਫਰ_TOO_SMALL);
        ਵਾਪਸ;
    }

    // Get dest buffer.
    ਖਾਲੀ *buf_far = (ਖਾਲੀ*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (ਖਾਲੀ*)(pirtable_g->slots) + get_global_ਵਿੱਥ()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_1ab1XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

ਖਾਲੀ
handle_1ab1(ਬਣਤਰ bregs *regs)
{
    //debug_stub(regs);

    ਜੇ (! CONFIG_PCIBIOS) {
        ਸੈੱਟ_invalid(regs);
        ਵਾਪਸ;
    }

    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x01: handle_1ab101(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: handle_1ab102(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x03: handle_1ab103(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x08: handle_1ab108(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x09: handle_1ab109(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0a: handle_1ab10a(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0b: handle_1ab10b(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0c: handle_1ab10c(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0d: handle_1ab10d(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0e: handle_1ab10e(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_1ab1XX(regs); ਰੋਕੋ;
    }
}

// Entry point for pci bios functions.
ਖਾਲੀ VISIBLE16 VISIBLE32SEG
handle_pcibios(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

ਬਣਤਰ bios32_s {
    u32 signature;
    u32 ਇੰਦਰਾਜ;
    u8 ਸੰਸਕਰਣ_2;
    u8 ਲੰਮਾਈ;
    u8 checksum;
    u8 reserved[5];
} PACKED;

ਬਣਤਰ bios32_s BIOS32_ਸਿਰਲੇਖ __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .ਲੰਮਾਈ = ਆਕਾਰ(BIOS32_ਸਿਰਲੇਖ) / 16,
};

ਖਾਲੀ
bios32_init(ਖਾਲੀ)
{
    dprintf(3, "init bios32\n");

    BIOS32_ਸਿਰਲੇਖ.ਇੰਦਰਾਜ = (u32)ਇੰਦਰਾਜ_bios32;
    BIOS32_ਸਿਰਲੇਖ.checksum -= checksum(&BIOS32_ਸਿਰਲੇਖ, ਆਕਾਰ(BIOS32_ਸਿਰਲੇਖ));
}
