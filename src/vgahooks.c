// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "bregs.h" // set_code_invalid
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_readb
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_find_device
#ಅನ್ತರ್ಭಾವ "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "string.h" // strcmp
#ಅನ್ತರ್ಭಾವ "util.h" // handle_155f, handle_157f

#ವ್ಯಾಖ್ಯಾನಿಸು VH_ಮೂಲಕ 1
#ವ್ಯಾಖ್ಯಾನಿಸು VH_INTEL 2
#ವ್ಯಾಖ್ಯಾನಿಸು VH_SMI 3

ಪೂರ್ಣಾಂಕ VGAHookHandlerType VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ
handle_155fXX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EUNSUPPORTED);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_157fXX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

ಪೂರ್ಣಾಂಕ ಮೂಲಕ_FBsize VARFSEG, ಮೂಲಕ_Ram_ವೇಗ VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ
ಮೂಲಕ_155f01(ರಚನೆ bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    ಗಣ_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಮೂಲಕ_155f02(ರಚನೆ bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    ಗಣ_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಮೂಲಕ_155f18(ರಚನೆ bregs *regs)
{
    ಪೂರ್ಣಾಂಕ fbsize = GET_GLOBAL(ಮೂಲಕ_FBsize), ramspeed = GET_GLOBAL(ಮೂಲಕ_Ram_ವೇಗ);
    ಆದರೆ (fbsize < 0 || ramspeed < 0) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EUNSUPPORTED);
        ಹಿಂತಿರುಗಿ;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಮೂಲಕ_155f19(ರಚನೆ bregs *regs)
{
    ಗಣ_invalid_silent(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಮೂಲಕ_155f(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x01: ಮೂಲಕ_155f01(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: ಮೂಲಕ_155f02(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x18: ಮೂಲಕ_155f18(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x19: ಮೂಲಕ_155f19(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_155fXX(regs); ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
getFBSize(ರಚನೆ pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    ಆದರೆ (!(reg & 0x80))
        ಹಿಂತಿರುಗಿ -1;

    ಸ್ಥಿರ u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    ಹಿಂತಿರುಗಿ mem_power[(reg >> 4) & 0x7];
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_ಮೂಲಕ_Ram_ವೇಗ(ರಚನೆ pci_device *pci)
{
    ಹಿಂತಿರುಗಿ (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_AMDRam_ವೇಗ(ಶೂನ್ಯ)
{
    ರಚನೆ pci_device *pci = pci_ಕಂಡುಹಿಡಿ_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    ಆದರೆ (!pci)
        ಹಿಂತಿರುಗಿ -1;

    /* mem clk 0 = DDR2 400 */
    ಹಿಂತಿರುಗಿ (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_ID_ಮೂಲಕ_K8_M890_CE_3    0x3336
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_ID_ಮೂಲಕ_VX855_MEMCTRL 0x3409

ಸ್ಥಿರ ಶೂನ್ಯ
ಮೂಲಕ_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    VGAHookHandlerType = VH_ಮೂಲಕ;

    ರಚನೆ pci_device *d = pci_ಕಂಡುಹಿಡಿ_device(PCI_VENDOR_ID_ಮೂಲಕ
                                           , PCI_DEVICE_ID_ಮೂಲಕ_K8_M890_CE_3);
    ಆದರೆ (d) {
        ಮೂಲಕ_FBsize = getFBSize(d);
        ಮೂಲಕ_Ram_ವೇಗ = get_AMDRam_ವೇಗ();
        ಹಿಂತಿರುಗಿ;
    }
    d = pci_ಕಂಡುಹಿಡಿ_device(PCI_VENDOR_ID_ಮೂಲಕ, PCI_DEVICE_ID_ಮೂಲಕ_VX855_MEMCTRL);
    ಆದರೆ (d) {
        ಮೂಲಕ_FBsize = getFBSize(d);
        ಮೂಲಕ_Ram_ವೇಗ = get_ಮೂಲಕ_Ram_ವೇಗ(d);
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ಮೂಲಕ_FBsize = 5; // 32M frame buffer
    ಮೂಲಕ_Ram_ವೇಗ = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f35(ರಚನೆ bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f40(ರಚನೆ bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f50(ರಚನೆ bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x35: intel_155f35(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x40: intel_155f40(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x50: intel_155f50(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_155fXX(regs); ನಿಲ್ಲಿಸು;
    }
}

#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_DEFAULT    (0)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_CRT        (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_TV         (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_EFP        (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_LCD        (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_CRT2       (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_TV2        (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_EFP2       (1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DISPLAY_LCD2       (1 << 7)

ಸ್ಥಿರ ಶೂನ್ಯ
intel_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

ಸ್ಥಿರ ಶೂನ್ಯ
roda_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

ಸ್ಥಿರ ಶೂನ್ಯ
kontron_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

ಸ್ಥಿರ ಶೂನ್ಯ
getac_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

ಸ್ಥಿರ ಶೂನ್ಯ
smi_157f02(ರಚನೆ bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
smi_157f14(ರಚನೆ bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
smi_157f(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x02: smi_157f02(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x14: smi_157f14(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_157fXX(regs); ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
winent_mb6047_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
ಶೂನ್ಯ
handle_155f(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        ಹಿಂತಿರುಗಿ;
    }

    ಪೂರ್ಣಾಂಕ htype = GET_GLOBAL(VGAHookHandlerType);
    ಆಯ್ಕೆ (htype) {
    ಸ್ಥಿತಿ VH_ಮೂಲಕ:   ಮೂಲಕ_155f(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ VH_INTEL: intel_155f(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:       handle_155fXX(regs); ನಿಲ್ಲಿಸು;
    }
}

// Main 16bit entry point
ಶೂನ್ಯ
handle_157f(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        ಹಿಂತಿರುಗಿ;
    }

    ಪೂರ್ಣಾಂಕ htype = GET_GLOBAL(VGAHookHandlerType);
    ಆಯ್ಕೆ (htype) {
    ಸ್ಥಿತಿ VH_SMI:   smi_157f(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:       handle_157fXX(regs); ನಿಲ್ಲಿಸು;
    }
}

//ಸಿದ್ಧತೆ
ಶೂನ್ಯ
vgahook_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    ಆದರೆ (!CONFIG_VGAHOOKS)
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_ಸಿದ್ಧತೆ(pci);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_ಸಿದ್ಧತೆ(pci);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_ಸಿದ್ಧತೆ(pci);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_ಸಿದ್ಧತೆ(pci);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pci->vendor == PCI_VENDOR_ID_ಮೂಲಕ)
        ಮೂಲಕ_ಸಿದ್ಧತೆ(pci);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_ಸಿದ್ಧತೆ(pci);
}
