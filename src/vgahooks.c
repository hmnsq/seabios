// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "bregs.h" // set_code_invalid
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "hw/pci.h" // pci_config_readb
#അന്തര്ഭാവ "hw/pcidevice.h" // pci_find_device
#അന്തര്ഭാവ "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#അന്തര്ഭാവ "hw/pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "string.h" // strcmp
#അന്തര്ഭാവ "util.h" // handle_155f, handle_157f

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

പൂർണ്ണസംഖ്യ VGAHookHandlerType VARFSEG;

സ്ഥിതം ശൂന്യം
handle_155fXX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

സ്ഥിതം ശൂന്യം
handle_157fXX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

പൂർണ്ണസംഖ്യ ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

സ്ഥിതം ശൂന്യം
via_155f01(ഘടന bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

സ്ഥിതം ശൂന്യം
via_155f02(ഘടന bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

സ്ഥിതം ശൂന്യം
via_155f18(ഘടന bregs *regs)
{
    പൂർണ്ണസംഖ്യ fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    എങ്കിൽ (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        തിരികെ;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
via_155f19(ഘടന bregs *regs)
{
    set_invalid_silent(regs);
}

സ്ഥിതം ശൂന്യം
via_155f(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x01: via_155f01(regs); നിർത്തുക;
    അവസ്ഥ 0x02: via_155f02(regs); നിർത്തുക;
    അവസ്ഥ 0x18: via_155f18(regs); നിർത്തുക;
    അവസ്ഥ 0x19: via_155f19(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_155fXX(regs); നിർത്തുക;
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
getFBSize(ഘടന pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    എങ്കിൽ (!(reg & 0x80))
        തിരികെ -1;

    സ്ഥിതം u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    തിരികെ mem_power[(reg >> 4) & 0x7];
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
getViaRamSpeed(ഘടന pci_device *pci)
{
    തിരികെ (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
getAMDRamSpeed(ശൂന്യം)
{
    ഘടന pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    എങ്കിൽ (!pci)
        തിരികെ -1;

    /* mem clk 0 = DDR2 400 */
    തിരികെ (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

#define PCI_DEVICE_ID_VIA_K8M890CE_3    0x3336
#define PCI_DEVICE_ID_VIA_VX855_MEMCTRL 0x3409

സ്ഥിതം ശൂന്യം
via_setup(ഘടന pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    ഘടന pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    എങ്കിൽ (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        തിരികെ;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    എങ്കിൽ (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        തിരികെ;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

സ്ഥിതം ശൂന്യം
intel_155f35(ഘടന bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
intel_155f40(ഘടന bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
intel_155f50(ഘടന bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
intel_155f(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x35: intel_155f35(regs); നിർത്തുക;
    അവസ്ഥ 0x40: intel_155f40(regs); നിർത്തുക;
    അവസ്ഥ 0x50: intel_155f50(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_155fXX(regs); നിർത്തുക;
    }
}

#define BOOT_DISPLAY_DEFAULT    (0)
#define BOOT_DISPLAY_CRT        (1 << 0)
#define BOOT_DISPLAY_TV         (1 << 1)
#define BOOT_DISPLAY_EFP        (1 << 2)
#define BOOT_DISPLAY_LCD        (1 << 3)
#define BOOT_DISPLAY_CRT2       (1 << 4)
#define BOOT_DISPLAY_TV2        (1 << 5)
#define BOOT_DISPLAY_EFP2       (1 << 6)
#define BOOT_DISPLAY_LCD2       (1 << 7)

സ്ഥിതം ശൂന്യം
intel_setup(ഘടന pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

സ്ഥിതം ശൂന്യം
roda_setup(ഘടന pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

സ്ഥിതം ശൂന്യം
kontron_setup(ഘടന pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

സ്ഥിതം ശൂന്യം
getac_setup(ഘടന pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

സ്ഥിതം ശൂന്യം
smi_157f02(ഘടന bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
smi_157f14(ഘടന bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
smi_157f(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x02: smi_157f02(regs); നിർത്തുക;
    അവസ്ഥ 0x14: smi_157f14(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_157fXX(regs); നിർത്തുക;
    }
}

സ്ഥിതം ശൂന്യം
winent_mb6047_setup(ഘടന pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
ശൂന്യം
handle_155f(ഘടന bregs *regs)
{
    എങ്കിൽ (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        തിരികെ;
    }

    പൂർണ്ണസംഖ്യ htype = GET_GLOBAL(VGAHookHandlerType);
    തിരഞ്ഞെടുപ്പ് (htype) {
    അവസ്ഥ VH_VIA:   via_155f(regs); നിർത്തുക;
    അവസ്ഥ VH_INTEL: intel_155f(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:       handle_155fXX(regs); നിർത്തുക;
    }
}

// Main 16bit entry point
ശൂന്യം
handle_157f(ഘടന bregs *regs)
{
    എങ്കിൽ (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        തിരികെ;
    }

    പൂർണ്ണസംഖ്യ htype = GET_GLOBAL(VGAHookHandlerType);
    തിരഞ്ഞെടുപ്പ് (htype) {
    അവസ്ഥ VH_SMI:   smi_157f(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:       handle_157fXX(regs); നിർത്തുക;
    }
}

// Setup
ശൂന്യം
vgahook_setup(ഘടന pci_device *pci)
{
    എങ്കിൽ (!CONFIG_VGAHOOKS)
        തിരികെ;

    എങ്കിൽ (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    അല്ലെങ്കിൽ എങ്കിൽ (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    അല്ലെങ്കിൽ എങ്കിൽ (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
