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

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

ಪೂರ್ಣಾಙ್ಕ VGAHookHandlerType VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ
handle_155fXX(ಸಂರಚನಾ bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_157fXX(ಸಂರಚನಾ bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

ಪೂರ್ಣಾಙ್ಕ ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ
via_155f01(ಸಂರಚನಾ bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ
via_155f02(ಸಂರಚನಾ bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ
via_155f18(ಸಂರಚನಾ bregs *regs)
{
    ಪೂರ್ಣಾಙ್ಕ fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    ಯದಿ (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ಪ್ರತ್ಯಾಗಮ;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
via_155f19(ಸಂರಚನಾ bregs *regs)
{
    set_invalid_silent(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
via_155f(ಸಂರಚನಾ bregs *regs)
{
    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x01: via_155f01(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: via_155f02(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x18: via_155f18(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x19: via_155f19(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   handle_155fXX(regs); ಭಙ್ಗ;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
getFBSize(ಸಂರಚನಾ pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    ಯದಿ (!(reg & 0x80))
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸ್ಥಿರ u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    ಪ್ರತ್ಯಾಗಮ mem_power[(reg >> 4) & 0x7];
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
getViaRamSpeed(ಸಂರಚನಾ pci_device *pci)
{
    ಪ್ರತ್ಯಾಗಮ (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
getAMDRamSpeed(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    ಯದಿ (!pci)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* mem clk 0 = DDR2 400 */
    ಪ್ರತ್ಯಾಗಮ (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

ಸ್ಥಿರ ಶೂನ್ಯ
via_setup(ಸಂರಚನಾ pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    ಸಂರಚನಾ pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    ಯದಿ (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        ಪ್ರತ್ಯಾಗಮ;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    ಯದಿ (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f35(ಸಂರಚನಾ bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f40(ಸಂರಚನಾ bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f50(ಸಂರಚನಾ bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
intel_155f(ಸಂರಚನಾ bregs *regs)
{
    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x35: intel_155f35(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x40: intel_155f40(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x50: intel_155f50(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   handle_155fXX(regs); ಭಙ್ಗ;
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

ಸ್ಥಿರ ಶೂನ್ಯ
intel_setup(ಸಂರಚನಾ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

ಸ್ಥಿರ ಶೂನ್ಯ
roda_setup(ಸಂರಚನಾ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

ಸ್ಥಿರ ಶೂನ್ಯ
kontron_setup(ಸಂರಚನಾ pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

ಸ್ಥಿರ ಶೂನ್ಯ
getac_setup(ಸಂರಚನಾ pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

ಸ್ಥಿರ ಶೂನ್ಯ
smi_157f02(ಸಂರಚನಾ bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
smi_157f14(ಸಂರಚನಾ bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
smi_157f(ಸಂರಚನಾ bregs *regs)
{
    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x02: smi_157f02(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x14: smi_157f14(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   handle_157fXX(regs); ಭಙ್ಗ;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
winent_mb6047_setup(ಸಂರಚನಾ pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
ಶೂನ್ಯ
handle_155f(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಪೂರ್ಣಾಙ್ಕ htype = GET_GLOBAL(VGAHookHandlerType);
    ವಿಕಲ್ಪ (htype) {
    ಪಕ್ಷ VH_VIA:   via_155f(regs); ಭಙ್ಗ;
    ಪಕ್ಷ VH_INTEL: intel_155f(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:       handle_155fXX(regs); ಭಙ್ಗ;
    }
}

// Main 16bit entry point
ಶೂನ್ಯ
handle_157f(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಪೂರ್ಣಾಙ್ಕ htype = GET_GLOBAL(VGAHookHandlerType);
    ವಿಕಲ್ಪ (htype) {
    ಪಕ್ಷ VH_SMI:   smi_157f(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:       handle_157fXX(regs); ಭಙ್ಗ;
    }
}

// Setup
ಶೂನ್ಯ
vgahook_setup(ಸಂರಚನಾ pci_device *pci)
{
    ಯದಿ (!CONFIG_VGAHOOKS)
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    ಅನ್ಯಥಾ ಯದಿ (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    ಅನ್ಯಥಾ ಯದಿ (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    ಅನ್ಯಥಾ ಯದಿ (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    ಅನ್ಯಥಾ ಯದಿ (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    ಅನ್ಯಥಾ ಯದಿ (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
