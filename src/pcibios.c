// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "hw/pci.h" // pci_config_readl
#అన్తర్భావ "hw/pcidevice.h" // MaxPCIBus
#అన్తర్భావ "hw/pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "std/pirtable.h" // struct pir_header
#అన్తర్భావ "string.h" // checksum
#అన్తర్భావ "util.h" // handle_1ab1

// romlayout.S
బాహ్య శూన్య నమోదు_bios32(శూన్య);
బాహ్య శూన్య నమోదు_pcibios32(శూన్య);

#నిర్వచించు RET_FUNC_NOT_SUPPORTED 0x81
#నిర్వచించు RET_BAD_VENDOR_ID      0x83
#నిర్వచించు RET_DEVICE_NOT_దొరికింది   0x86
#నిర్వచించు RET_బఫరు_TOO_SMALL   0x89

// installation check
స్థిర శూన్య
handle_1ab101(నిర్మాణం bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)నమోదు_pcibios32 + BUILD_BIOS_ADDR;
    సమితి_సంకేతం_success(regs);
}

// find pci device
స్థిర శూన్య
handle_1ab102(నిర్మాణం bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    పూర్ణాంకం లెక్క = regs->si;
    పూర్ణాంకం bus = -1;
    అయితే (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        పూర్ణాంకం bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            ఒకవేళ (v != id)
                కొనసాగు;
            ఒకవేళ (లెక్క--)
                కొనసాగు;
            regs->bx = bdf;
            సమితి_సంకేతం_success(regs);
            తిరిగి;
        }
    }
    సమితి_సంకేతం_invalid(regs, RET_DEVICE_NOT_దొరికింది);
}

// find class code
స్థిర శూన్య
handle_1ab103(నిర్మాణం bregs *regs)
{
    పూర్ణాంకం లెక్క = regs->si;
    u32 classprog = regs->ecx;
    పూర్ణాంకం bus = -1;
    అయితే (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        పూర్ణాంకం bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            ఒకవేళ ((v>>8) != classprog)
                కొనసాగు;
            ఒకవేళ (లెక్క--)
                కొనసాగు;
            regs->bx = bdf;
            సమితి_సంకేతం_success(regs);
            తిరిగి;
        }
    }
    సమితి_సంకేతం_invalid(regs, RET_DEVICE_NOT_దొరికింది);
}

// read configuration byte
స్థిర శూన్య
handle_1ab108(నిర్మాణం bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    సమితి_సంకేతం_success(regs);
}

// read configuration word
స్థిర శూన్య
handle_1ab109(నిర్మాణం bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    సమితి_సంకేతం_success(regs);
}

// read configuration dword
స్థిర శూన్య
handle_1ab10a(నిర్మాణం bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    సమితి_సంకేతం_success(regs);
}

// write configuration byte
స్థిర శూన్య
handle_1ab10b(నిర్మాణం bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    సమితి_సంకేతం_success(regs);
}

// write configuration word
స్థిర శూన్య
handle_1ab10c(నిర్మాణం bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    సమితి_సంకేతం_success(regs);
}

// write configuration dword
స్థిర శూన్య
handle_1ab10d(నిర్మాణం bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    సమితి_సంకేతం_success(regs);
}

// get irq routing options
స్థిర శూన్య
handle_1ab10e(నిర్మాణం bregs *regs)
{
    నిర్మాణం pir_శీర్షిక *pirtable_gf = GET_GLOBAL(PirAddr);
    ఒకవేళ (! pirtable_gf) {
        సమితి_సంకేతం_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        తిరిగి;
    }
    నిర్మాణం pir_శీర్షిక *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    నిర్మాణం param_s {
        u16 కొలత;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (శూన్య*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->కొలత);
    u16 pirsize = GET_GLOBAL(pirtable_g->కొలత) - పరిమాణం(నిర్మాణం pir_శీర్షిక);
    సమితి_FARVAR(regs->es, param_far->కొలత, pirsize);
    ఒకవేళ (bufsize < pirsize) {
        సమితి_సంకేతం_invalid(regs, RET_బఫరు_TOO_SMALL);
        తిరిగి;
    }

    // Get dest buffer.
    శూన్య *buf_far = (శూన్య*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (శూన్య*)(pirtable_g->slots) + get_global_స్థానభ్రంశం()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
handle_1ab1XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

శూన్య
handle_1ab1(నిర్మాణం bregs *regs)
{
    //debug_stub(regs);

    ఒకవేళ (! CONFIG_PCIBIOS) {
        సమితి_invalid(regs);
        తిరిగి;
    }

    ఎంపిక (regs->al) {
    స్థితి 0x01: handle_1ab101(regs); ఆపు;
    స్థితి 0x02: handle_1ab102(regs); ఆపు;
    స్థితి 0x03: handle_1ab103(regs); ఆపు;
    స్థితి 0x08: handle_1ab108(regs); ఆపు;
    స్థితి 0x09: handle_1ab109(regs); ఆపు;
    స్థితి 0x0a: handle_1ab10a(regs); ఆపు;
    స్థితి 0x0b: handle_1ab10b(regs); ఆపు;
    స్థితి 0x0c: handle_1ab10c(regs); ఆపు;
    స్థితి 0x0d: handle_1ab10d(regs); ఆపు;
    స్థితి 0x0e: handle_1ab10e(regs); ఆపు;
    అప్రమేయ:   handle_1ab1XX(regs); ఆపు;
    }
}

// Entry point for pci bios functions.
శూన్య VISIBLE16 VISIBLE32SEG
handle_pcibios(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

నిర్మాణం bios32_s {
    u32 signature;
    u32 నమోదు_2;
    u8 సంస్కరణ_2;
    u8 నిడివి;
    u8 checksum;
    u8 reserved[5];
} PACKED;

నిర్మాణం bios32_s BIOS32_శీర్షిక __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .నిడివి = పరిమాణం(BIOS32_శీర్షిక) / 16,
};

శూన్య
bios32_init(శూన్య)
{
    dprintf(3, "init bios32\n");

    BIOS32_శీర్షిక.నమోదు_2 = (u32)నమోదు_bios32;
    BIOS32_శీర్షిక.checksum -= checksum(&BIOS32_శీర్షిక, పరిమాణం(BIOS32_శీర్షిక));
}
