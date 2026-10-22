// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // set_code_invalid
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_readb
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // pci_find_device
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // strcmp
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // handle_155f, handle_157f

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VH_ਰਾਹੀਂ 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VH_INTEL 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VH_SMI 3

ਪੂਰਨ VGAHookHandlerType VARFSEG;

ਸਥਿਰ ਖਾਲੀ
handle_155fXX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EUNSUPPORTED);
}

ਸਥਿਰ ਖਾਲੀ
handle_157fXX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

ਪੂਰਨ ਰਾਹੀਂ_FBsize VARFSEG, ਰਾਹੀਂ_Ram_ਰਫਤਾਰ VARFSEG;

ਸਥਿਰ ਖਾਲੀ
ਰਾਹੀਂ_155f01(ਬਣਤਰ bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    ਸੈੱਟ_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

ਸਥਿਰ ਖਾਲੀ
ਰਾਹੀਂ_155f02(ਬਣਤਰ bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    ਸੈੱਟ_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

ਸਥਿਰ ਖਾਲੀ
ਰਾਹੀਂ_155f18(ਬਣਤਰ bregs *regs)
{
    ਪੂਰਨ fbsize = GET_GLOBAL(ਰਾਹੀਂ_FBsize), ramspeed = GET_GLOBAL(ਰਾਹੀਂ_Ram_ਰਫਤਾਰ);
    ਜੇ (fbsize < 0 || ramspeed < 0) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EUNSUPPORTED);
        ਵਾਪਸ;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
ਰਾਹੀਂ_155f19(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_invalid_silent(regs);
}

ਸਥਿਰ ਖਾਲੀ
ਰਾਹੀਂ_155f(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x01: ਰਾਹੀਂ_155f01(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: ਰਾਹੀਂ_155f02(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x18: ਰਾਹੀਂ_155f18(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x19: ਰਾਹੀਂ_155f19(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_155fXX(regs); ਰੋਕੋ;
    }
}

ਸਥਿਰ ਪੂਰਨ
getFBSize(ਬਣਤਰ pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    ਜੇ (!(reg & 0x80))
        ਵਾਪਸ -1;

    ਸਥਿਰ u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    ਵਾਪਸ mem_power[(reg >> 4) & 0x7];
}

ਸਥਿਰ ਪੂਰਨ
get_ਰਾਹੀਂ_Ram_ਰਫਤਾਰ(ਬਣਤਰ pci_device *pci)
{
    ਵਾਪਸ (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

ਸਥਿਰ ਪੂਰਨ
get_AMDRam_ਰਫਤਾਰ(ਖਾਲੀ)
{
    ਬਣਤਰ pci_device *pci = pci_ਲੱਭੋ_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    ਜੇ (!pci)
        ਵਾਪਸ -1;

    /* mem clk 0 = DDR2 400 */
    ਵਾਪਸ (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
}

/* int 0x15 - 5f18

   ECX = unknown/don't care
   EBX[3..0] Frame Buffer Size 2^N MiB
   EBX[7..4] Memory speed:
       0: SDR  66Mhz
       1: SDR 100Mhz
       2: SDR 133Mhz
       3: DDR 100Mhz (PC1600 or DDR200)
       4: DDR 133Mhz (PC2100 or DDR266)
       5: DDR 166Mhz (PC2700 or DDR333)
       6: DDR 200Mhz (PC3200 or DDR400)
       7: DDR2 133Mhz (DDR2 533)
       8: DDR2 166Mhz (DDR2 667)
       9: DDR2 200Mhz (DDR2 800)
       A: DDR2 233Mhz (DDR2 1066)
       B: and above: Unknown
   EBX[?..8] Total memory size?
   EAX = 0x5f for success
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_ID_ਰਾਹੀਂ_K8_M890_CE_3    0x3336
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_ID_ਰਾਹੀਂ_VX855_MEMCTRL 0x3409

ਸਥਿਰ ਖਾਲੀ
ਰਾਹੀਂ_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    VGAHookHandlerType = VH_ਰਾਹੀਂ;

    ਬਣਤਰ pci_device *d = pci_ਲੱਭੋ_device(PCI_VENDOR_ID_ਰਾਹੀਂ
                                           , PCI_DEVICE_ID_ਰਾਹੀਂ_K8_M890_CE_3);
    ਜੇ (d) {
        ਰਾਹੀਂ_FBsize = getFBSize(d);
        ਰਾਹੀਂ_Ram_ਰਫਤਾਰ = get_AMDRam_ਰਫਤਾਰ();
        ਵਾਪਸ;
    }
    d = pci_ਲੱਭੋ_device(PCI_VENDOR_ID_ਰਾਹੀਂ, PCI_DEVICE_ID_ਰਾਹੀਂ_VX855_MEMCTRL);
    ਜੇ (d) {
        ਰਾਹੀਂ_FBsize = getFBSize(d);
        ਰਾਹੀਂ_Ram_ਰਫਤਾਰ = get_ਰਾਹੀਂ_Ram_ਰਫਤਾਰ(d);
        ਵਾਪਸ;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ਰਾਹੀਂ_FBsize = 5; // 32M frame buffer
    ਰਾਹੀਂ_Ram_ਰਫਤਾਰ = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

ਸਥਿਰ ਖਾਲੀ
intel_155f35(ਬਣਤਰ bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
intel_155f40(ਬਣਤਰ bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
intel_155f50(ਬਣਤਰ bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
intel_155f(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x35: intel_155f35(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x40: intel_155f40(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x50: intel_155f50(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_155fXX(regs); ਰੋਕੋ;
    }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_DEFAULT    (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_CRT        (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_TV         (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_EFP        (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_LCD        (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_CRT2       (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_TV2        (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_EFP2       (1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DISPLAY_LCD2       (1 << 7)

ਸਥਿਰ ਖਾਲੀ
intel_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

ਸਥਿਰ ਖਾਲੀ
roda_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

ਸਥਿਰ ਖਾਲੀ
kontron_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

ਸਥਿਰ ਖਾਲੀ
getac_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

ਸਥਿਰ ਖਾਲੀ
smi_157f02(ਬਣਤਰ bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
smi_157f14(ਬਣਤਰ bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
smi_157f(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x02: smi_157f02(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x14: smi_157f14(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_157fXX(regs); ਰੋਕੋ;
    }
}

ਸਥਿਰ ਖਾਲੀ
winent_mb6047_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
ਖਾਲੀ
handle_155f(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        ਵਾਪਸ;
    }

    ਪੂਰਨ htype = GET_GLOBAL(VGAHookHandlerType);
    ਚੋਣ (htype) {
    ਹਾਲਤ VH_ਰਾਹੀਂ:   ਰਾਹੀਂ_155f(regs); ਰੋਕੋ;
    ਹਾਲਤ VH_INTEL: intel_155f(regs); ਰੋਕੋ;
    ਮੂਲ:       handle_155fXX(regs); ਰੋਕੋ;
    }
}

// Main 16bit entry point
ਖਾਲੀ
handle_157f(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        ਵਾਪਸ;
    }

    ਪੂਰਨ htype = GET_GLOBAL(VGAHookHandlerType);
    ਚੋਣ (htype) {
    ਹਾਲਤ VH_SMI:   smi_157f(regs); ਰੋਕੋ;
    ਮੂਲ:       handle_157fXX(regs); ਰੋਕੋ;
    }
}

//ਤਿਆਰੀ
ਖਾਲੀ
vgahook_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    ਜੇ (!CONFIG_VGAHOOKS)
        ਵਾਪਸ;

    ਜੇ (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_ਤਿਆਰੀ(pci);
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_ਤਿਆਰੀ(pci);
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_ਤਿਆਰੀ(pci);
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_ਤਿਆਰੀ(pci);
    ਨਹੀਂਤਾਂ ਜੇ (pci->vendor == PCI_VENDOR_ID_ਰਾਹੀਂ)
        ਰਾਹੀਂ_ਤਿਆਰੀ(pci);
    ਨਹੀਂਤਾਂ ਜੇ (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_ਤਿਆਰੀ(pci);
}
