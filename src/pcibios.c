// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "bregs.h" // struct bregs
#ضمن "hw/pci.h" // pci_config_readl
#ضمن "hw/pcidevice.h" // MaxPCIBus
#ضمن "hw/pci_regs.h" // PCI_VENDOR_ID
#ضمن "output.h" // dprintf
#ضمن "std/pirtable.h" // struct pir_header
#ضمن "string.h" // checksum
#ضمن "util.h" // handle_1ab1

// romlayout.S
خارجي فراغ مدخل_bios32(فراغ);
خارجي فراغ مدخل_pcibios32(فراغ);

#تعریف_کن RET_FUNC_NOT_SUPPORTED 0x81
#تعریف_کن RET_BAD_VENDOR_ID      0x83
#تعریف_کن RET_DEVICE_NOT_یافت   0x86
#تعریف_کن RET_میانگیر_TOO_SMALL   0x89

// installation check
ساكن فراغ
handle_1ab101(بنية bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)مدخل_pcibios32 + BUILD_BIOS_ADDR;
    مجموعه_کد_success(regs);
}

// find pci device
ساكن فراغ
handle_1ab102(بنية bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    صحيح شمار = regs->si;
    صحيح bus = -1;
    طالما (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        صحيح bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            إذا (v != id)
                استمر;
            إذا (شمار--)
                استمر;
            regs->bx = bdf;
            مجموعه_کد_success(regs);
            أرجع;
        }
    }
    مجموعه_کد_invalid(regs, RET_DEVICE_NOT_یافت);
}

// find class code
ساكن فراغ
handle_1ab103(بنية bregs *regs)
{
    صحيح شمار = regs->si;
    u32 classprog = regs->ecx;
    صحيح bus = -1;
    طالما (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        صحيح bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            إذا ((v>>8) != classprog)
                استمر;
            إذا (شمار--)
                استمر;
            regs->bx = bdf;
            مجموعه_کد_success(regs);
            أرجع;
        }
    }
    مجموعه_کد_invalid(regs, RET_DEVICE_NOT_یافت);
}

// read configuration byte
ساكن فراغ
handle_1ab108(بنية bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    مجموعه_کد_success(regs);
}

// read configuration word
ساكن فراغ
handle_1ab109(بنية bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    مجموعه_کد_success(regs);
}

// read configuration dword
ساكن فراغ
handle_1ab10a(بنية bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    مجموعه_کد_success(regs);
}

// write configuration byte
ساكن فراغ
handle_1ab10b(بنية bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    مجموعه_کد_success(regs);
}

// write configuration word
ساكن فراغ
handle_1ab10c(بنية bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    مجموعه_کد_success(regs);
}

// write configuration dword
ساكن فراغ
handle_1ab10d(بنية bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    مجموعه_کد_success(regs);
}

// get irq routing options
ساكن فراغ
handle_1ab10e(بنية bregs *regs)
{
    بنية pir_سرآیند *pirtable_gf = GET_GLOBAL(PirAddr);
    إذا (! pirtable_gf) {
        مجموعه_کد_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        أرجع;
    }
    بنية pir_سرآیند *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    بنية param_s {
        u16 اندازه;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (فراغ*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->اندازه);
    u16 pirsize = GET_GLOBAL(pirtable_g->اندازه) - حجم(بنية pir_سرآیند);
    مجموعه_FARVAR(regs->es, param_far->اندازه, pirsize);
    إذا (bufsize < pirsize) {
        مجموعه_کد_invalid(regs, RET_میانگیر_TOO_SMALL);
        أرجع;
    }

    // Get dest buffer.
    فراغ *buf_far = (فراغ*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (فراغ*)(pirtable_g->slots) + get_global_جابه_جایی()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    مجموعه_کد_success(regs);
}

ساكن فراغ
handle_1ab1XX(بنية bregs *regs)
{
    مجموعه_کد_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

فراغ
handle_1ab1(بنية bregs *regs)
{
    //debug_stub(regs);

    إذا (! CONFIG_PCIBIOS) {
        مجموعه_invalid(regs);
        أرجع;
    }

    بدل (regs->al) {
    حالة 0x01: handle_1ab101(regs); اكسر;
    حالة 0x02: handle_1ab102(regs); اكسر;
    حالة 0x03: handle_1ab103(regs); اكسر;
    حالة 0x08: handle_1ab108(regs); اكسر;
    حالة 0x09: handle_1ab109(regs); اكسر;
    حالة 0x0a: handle_1ab10a(regs); اكسر;
    حالة 0x0b: handle_1ab10b(regs); اكسر;
    حالة 0x0c: handle_1ab10c(regs); اكسر;
    حالة 0x0d: handle_1ab10d(regs); اكسر;
    حالة 0x0e: handle_1ab10e(regs); اكسر;
    افتراضي:   handle_1ab1XX(regs); اكسر;
    }
}

// Entry point for pci bios functions.
فراغ VISIBLE16 VISIBLE32SEG
handle_pcibios(بنية bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

بنية bios32_s {
    u32 signature;
    u32 مدخل;
    u8 نسخه_2;
    u8 درازی;
    u8 checksum;
    u8 reserved[5];
} PACKED;

بنية bios32_s BIOS32_سرآیند __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .درازی = حجم(BIOS32_سرآیند) / 16,
};

فراغ
bios32_init(فراغ)
{
    dprintf(3, "init bios32\n");

    BIOS32_سرآیند.مدخل = (u32)مدخل_bios32;
    BIOS32_سرآیند.checksum -= checksum(&BIOS32_سرآیند, حجم(BIOS32_سرآیند));
}
