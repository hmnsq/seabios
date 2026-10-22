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
بیرونی تهی entry_bios32(تهی);
بیرونی تهی entry_pcibios32(تهی);

#define RET_FUNC_NOT_SUPPORTED 0x81
#define RET_BAD_VENDOR_ID      0x83
#define RET_DEVICE_NOT_FOUND   0x86
#define RET_BUFFER_TOO_SMALL   0x89

// installation check
ایستا تهی
handle_1ab101(ساختار bregs *regs)
{
    regs->al = 0x01; // Flags - "Config Mechanism #1" supported.
    regs->bx = 0x0210; // PCI version 2.10
    regs->cl = GET_GLOBAL(MaxPCIBus);
    regs->edx = 0x20494350; // "PCI "
    regs->edi = (u32)entry_pcibios32 + BUILD_BIOS_ADDR;
    set_code_success(regs);
}

// find pci device
ایستا تهی
handle_1ab102(ساختار bregs *regs)
{
    u32 id = (regs->cx << 16) | regs->dx;
    صحیح شمار = regs->si;
    صحیح bus = -1;
    وقتی (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        صحیح bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_VENDOR_ID);
            اگر (v != id)
                ادامه;
            اگر (شمار--)
                ادامه;
            regs->bx = bdf;
            set_code_success(regs);
            بازگشت;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// find class code
ایستا تهی
handle_1ab103(ساختار bregs *regs)
{
    صحیح شمار = regs->si;
    u32 classprog = regs->ecx;
    صحیح bus = -1;
    وقتی (bus < GET_GLOBAL(MaxPCIBus)) {
        bus++;
        صحیح bdf;
        foreachbdf(bdf, bus) {
            u32 v = pci_config_readl(bdf, PCI_CLASS_REVISION);
            اگر ((v>>8) != classprog)
                ادامه;
            اگر (شمار--)
                ادامه;
            regs->bx = bdf;
            set_code_success(regs);
            بازگشت;
        }
    }
    set_code_invalid(regs, RET_DEVICE_NOT_FOUND);
}

// read configuration byte
ایستا تهی
handle_1ab108(ساختار bregs *regs)
{
    regs->cl = pci_config_readb(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration word
ایستا تهی
handle_1ab109(ساختار bregs *regs)
{
    regs->cx = pci_config_readw(regs->bx, regs->di);
    set_code_success(regs);
}

// read configuration dword
ایستا تهی
handle_1ab10a(ساختار bregs *regs)
{
    regs->ecx = pci_config_readl(regs->bx, regs->di);
    set_code_success(regs);
}

// write configuration byte
ایستا تهی
handle_1ab10b(ساختار bregs *regs)
{
    pci_config_writeb(regs->bx, regs->di, regs->cl);
    set_code_success(regs);
}

// write configuration word
ایستا تهی
handle_1ab10c(ساختار bregs *regs)
{
    pci_config_writew(regs->bx, regs->di, regs->cx);
    set_code_success(regs);
}

// write configuration dword
ایستا تهی
handle_1ab10d(ساختار bregs *regs)
{
    pci_config_writel(regs->bx, regs->di, regs->ecx);
    set_code_success(regs);
}

// get irq routing options
ایستا تهی
handle_1ab10e(ساختار bregs *regs)
{
    ساختار pir_header *pirtable_gf = GET_GLOBAL(PirAddr);
    اگر (! pirtable_gf) {
        set_code_invalid(regs, RET_FUNC_NOT_SUPPORTED);
        بازگشت;
    }
    ساختار pir_header *pirtable_g = GLOBALFLAT2GLOBAL(pirtable_gf);

    ساختار param_s {
        u16 size;
        u16 buf_off;
        u16 buf_seg;
    } *param_far = (تهی*)(regs->di+0);

    // Validate and update size.
    u16 bufsize = GET_FARVAR(regs->es, param_far->size);
    u16 pirsize = GET_GLOBAL(pirtable_g->size) - اندازه(ساختار pir_header);
    SET_FARVAR(regs->es, param_far->size, pirsize);
    اگر (bufsize < pirsize) {
        set_code_invalid(regs, RET_BUFFER_TOO_SMALL);
        بازگشت;
    }

    // Get dest buffer.
    تهی *buf_far = (تهی*)(GET_FARVAR(regs->es, param_far->buf_off)+0);
    u16 buf_seg = GET_FARVAR(regs->es, param_far->buf_seg);

    // Memcpy pir table slots to dest buffer.
    memcpy_far(buf_seg, buf_far
               , get_global_seg()
               , (تهی*)(pirtable_g->slots) + get_global_offset()
               , pirsize);

    // XXX - bochs bios sets bx to (1 << 9) | (1 << 11)
    regs->bx = GET_GLOBAL(pirtable_g->exclusive_irqs);
    set_code_success(regs);
}

ایستا تهی
handle_1ab1XX(ساختار bregs *regs)
{
    set_code_unimplemented(regs, RET_FUNC_NOT_SUPPORTED);
}

تهی
handle_1ab1(ساختار bregs *regs)
{
    //debug_stub(regs);

    اگر (! CONFIG_PCIBIOS) {
        set_invalid(regs);
        بازگشت;
    }

    انتخاب (regs->al) {
    حالت 0x01: handle_1ab101(regs); بشکن;
    حالت 0x02: handle_1ab102(regs); بشکن;
    حالت 0x03: handle_1ab103(regs); بشکن;
    حالت 0x08: handle_1ab108(regs); بشکن;
    حالت 0x09: handle_1ab109(regs); بشکن;
    حالت 0x0a: handle_1ab10a(regs); بشکن;
    حالت 0x0b: handle_1ab10b(regs); بشکن;
    حالت 0x0c: handle_1ab10c(regs); بشکن;
    حالت 0x0d: handle_1ab10d(regs); بشکن;
    حالت 0x0e: handle_1ab10e(regs); بشکن;
    پیشفرض:   handle_1ab1XX(regs); بشکن;
    }
}

// Entry point for pci bios functions.
تهی VISIBLE16 VISIBLE32SEG
handle_pcibios(ساختار bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_pcibios);
    handle_1ab1(regs);
}


/****************************************************************
 * 32bit interface
 ****************************************************************/

ساختار bios32_s {
    u32 signature;
    u32 entry;
    u8 version;
    u8 length;
    u8 checksum;
    u8 reserved[5];
} PACKED;

ساختار bios32_s BIOS32HEADER __aligned(16) VARFSEG = {
    .signature = 0x5f32335f, // _32_
    .length = اندازه(BIOS32HEADER) / 16,
};

تهی
bios32_init(تهی)
{
    dprintf(3, "init bios32\n");

    BIOS32HEADER.entry = (u32)entry_bios32;
    BIOS32HEADER.checksum -= checksum(&BIOS32HEADER, اندازه(BIOS32HEADER));
}
