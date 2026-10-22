// PCI BIOS (int 1a/b1) calls
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "bregs.h" // struct bregs
#شامل "hw/pci.h" // pci_config_readl
#شامل "hw/pcidevice.h" // MaxPCIBus
#شامل "hw/pci_regs.h" // PCI_VENDOR_ID
#شامل "output.h" // dprintf
#شامل "std/pirtable.h" // struct pir_header
#شامل "string.h" // checksum
#شامل "util.h" // handle_1ab1

// romlayout.S
خارجی خالی اندراج_bios32(خالی);
خارجی خالی اندراج_pcibios32(خالی);

#تعریف_کرو RET_FUNC_NOT_SUPPORTED 0x81
#تعریف_کرو RET_BAD_VENDOR_ID      0x83
#تعریف_کرو RET_DEVICE_NOT_ملا   0x86
#تعریف_کرو RET_بفر_TOO_SMALL   0x89

// installation check
ساکن خالی
handle_1ab101(ساخت bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)اندراج_pcibios32 + BUILD_BIOS_ADDR;
    سیٹ_رمز_success(regs);
}

// find pci device
ساکن خالی
handle_1ab102(ساخت bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    صحیح گنتی = regs->si;
    صحیح bus = -1;
    جب (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        صحیح bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            اگر (v != id)
                جاری;
            اگر (گنتی--)
                جاری;
            regs->bx = bdf;
            سیٹ_رمز_success(regs);
            واپس;
        }
    }
    سیٹ_رمز_invalid(regs, RET_DEVICE_NOT_ملا);
}

// find class code
ساکن خالی
handle_1ab103(ساخت bregs *regs)
{
    صحیح گنتی = regs->si;
    u32 classprog = regs->ecx;
    صحیح bus = -1;
    جب (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        صحیح bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            اگر ((v>>8) != classprog)
                جاری;
            اگر (گنتی--)
                جاری;
            regs->bx = bdf;
            سیٹ_رمز_success(regs);
            واپس;
        }
    }
    سیٹ_رمز_invalid(regs, RET_DEVICE_NOT_ملا);
}

// read configuration byte
ساکن خالی
handle_1ab108(ساخت bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    سیٹ_رمز_success(regs);
}

// read configuration word
ساکن خالی
handle_1ab109(ساخت bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    سیٹ_رمز_success(regs);
}

// read configuration dword
ساکن خالی
handle_1ab10a(ساخت bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    سیٹ_رمز_success(regs);
}

// write configuration byte
ساکن خالی
handle_1ab10b(ساخت bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    سیٹ_رمز_success(regs);
}

// write configuration word
ساکن خالی
handle_1ab10c(ساخت bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    سیٹ_رمز_success(regs);
}

// write configuration dword
ساکن خالی
handle_1ab10d(ساخت bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    سیٹ_رمز_success(regs);
}

// get irq routing options
ساکن خالی
handle_1ab10e(ساخت bregs *regs)
{
    ساخت pir_سرخی *pirtable_gf = GET_GLOBAL(PirAddr);
    اگر (! pirtable_gf) {
        سیٹ_رمز_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        واپس;
    }
    ساخت pir_سرخی *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    ساخت param_s {
        u16 سائز;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (خالی*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->سائز);
    u16 pirsize = GET_GLOBAL(pirtable_g->سائز) - حجم(ساخت pir_سرخی);
    سیٹ_FARVAR(regs->es, param_far->سائز, pirsize);
    اگر (bufsize < pirsize) {
        سیٹ_رمز_invalid(regs, RET_بفر_TOO_SMALL);
        واپس;
    }

    // Get dest buffer.
    خالی *buf_far = (خالی*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (خالی*)(pirtable_g->slots) + get_global_انحراف()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    سیٹ_رمز_success(regs);
}

ساکن خالی
handle_1ab1XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

خالی
handle_1ab1(ساخت bregs *regs)
{
    //debug_stub(regs);

    اگر (! CONFIG_PCIBIOS) {
        سیٹ_invalid(regs);
        واپس;
    }

    انتخاب (regs->al) {
    حالت 0x01: handle_1ab101(regs); توڑو;
    حالت 0x02: handle_1ab102(regs); توڑو;
    حالت 0x03: handle_1ab103(regs); توڑو;
    حالت 0x08: handle_1ab108(regs); توڑو;
    حالت 0x09: handle_1ab109(regs); توڑو;
    حالت 0x0a: handle_1ab10a(regs); توڑو;
    حالت 0x0b: handle_1ab10b(regs); توڑو;
    حالت 0x0c: handle_1ab10c(regs); توڑو;
    حالت 0x0d: handle_1ab10d(regs); توڑو;
    حالت 0x0e: handle_1ab10e(regs); توڑو;
    طےشدہ:   handle_1ab1XX(regs); توڑو;
    }
}

// Entry point for pci bios functions.
خالی VISIBLE16 VISIBLE32SEG
handle_pcibios(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

ساخت bios32_s {
    u32 signature;
    u32 اندراج_2;
    u8 نسخہ_2;
    u8 لمبائی;
    u8 checksum;
    u8 reserved[5];
} PACKED;

ساخت bios32_s BIOS32_سرخی __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .لمبائی = حجم(BIOS32_سرخی) / 16,
};

خالی
bios32_init(خالی)
{
    dprintf(3, "init bios32\n");

    BIOS32_سرخی.اندراج_2 = (u32)اندراج_bios32;
    BIOS32_سرخی.checksum -= checksum(&BIOS32_سرخی, حجم(BIOS32_سرخی));
}
