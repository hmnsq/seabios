// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "bregs.h" // set_code_invalid
#شامل "config.h" // CONFIG_*
#شامل "hw/pci.h" // pci_config_readb
#شامل "hw/pcidevice.h" // pci_find_device
#شامل "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#شامل "hw/pci_regs.h" // PCI_VENDOR_ID
#شامل "output.h" // dprintf
#شامل "string.h" // strcmp
#شامل "util.h" // handle_155f, handle_157f

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

صحیح VGAHookHandlerType VARFSEG;

ایستا تهی
handle_155fXX(ساختار bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

ایستا تهی
handle_157fXX(ساختار bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

صحیح ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

ایستا تهی
via_155f01(ساختار bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

ایستا تهی
via_155f02(ساختار bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

ایستا تهی
via_155f18(ساختار bregs *regs)
{
    صحیح fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    اگر (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        بازگشت;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

ایستا تهی
via_155f19(ساختار bregs *regs)
{
    set_invalid_silent(regs);
}

ایستا تهی
via_155f(ساختار bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x01: via_155f01(regs); بشکن;
    حالت 0x02: via_155f02(regs); بشکن;
    حالت 0x18: via_155f18(regs); بشکن;
    حالت 0x19: via_155f19(regs); بشکن;
    پیشفرض:   handle_155fXX(regs); بشکن;
    }
}

ایستا صحیح
getFBSize(ساختار pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    اگر (!(reg & 0x80))
        بازگشت -1;

    ایستا u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    بازگشت mem_power[(reg >> 4) & 0x7];
}

ایستا صحیح
getViaRamSpeed(ساختار pci_device *pci)
{
    بازگشت (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

ایستا صحیح
getAMDRamSpeed(تهی)
{
    ساختار pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    اگر (!pci)
        بازگشت -1;

    /* mem clk 0 = DDR2 400 */
    بازگشت (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

ایستا تهی
via_setup(ساختار pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    ساختار pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    اگر (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        بازگشت;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    اگر (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        بازگشت;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

ایستا تهی
intel_155f35(ساختار bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

ایستا تهی
intel_155f40(ساختار bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

ایستا تهی
intel_155f50(ساختار bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

ایستا تهی
intel_155f(ساختار bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x35: intel_155f35(regs); بشکن;
    حالت 0x40: intel_155f40(regs); بشکن;
    حالت 0x50: intel_155f50(regs); بشکن;
    پیشفرض:   handle_155fXX(regs); بشکن;
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

ایستا تهی
intel_setup(ساختار pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

ایستا تهی
roda_setup(ساختار pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

ایستا تهی
kontron_setup(ساختار pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

ایستا تهی
getac_setup(ساختار pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

ایستا تهی
smi_157f02(ساختار bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

ایستا تهی
smi_157f14(ساختار bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

ایستا تهی
smi_157f(ساختار bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x02: smi_157f02(regs); بشکن;
    حالت 0x14: smi_157f14(regs); بشکن;
    پیشفرض:   handle_157fXX(regs); بشکن;
    }
}

ایستا تهی
winent_mb6047_setup(ساختار pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
تهی
handle_155f(ساختار bregs *regs)
{
    اگر (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        بازگشت;
    }

    صحیح htype = GET_GLOBAL(VGAHookHandlerType);
    انتخاب (htype) {
    حالت VH_VIA:   via_155f(regs); بشکن;
    حالت VH_INTEL: intel_155f(regs); بشکن;
    پیشفرض:       handle_155fXX(regs); بشکن;
    }
}

// Main 16bit entry point
تهی
handle_157f(ساختار bregs *regs)
{
    اگر (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        بازگشت;
    }

    صحیح htype = GET_GLOBAL(VGAHookHandlerType);
    انتخاب (htype) {
    حالت VH_SMI:   smi_157f(regs); بشکن;
    پیشفرض:       handle_157fXX(regs); بشکن;
    }
}

// Setup
تهی
vgahook_setup(ساختار pci_device *pci)
{
    اگر (!CONFIG_VGAHOOKS)
        بازگشت;

    اگر (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    وگرنه اگر (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    وگرنه اگر (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    وگرنه اگر (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    وگرنه اگر (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    وگرنه اگر (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
