// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "bregs.h" // set_code_invalid
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "hw/pci.h" // pci_config_readb
#அந்தர்பாவ "hw/pcidevice.h" // pci_find_device
#அந்தர்பாவ "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#அந்தர்பாவ "hw/pci_regs.h" // PCI_VENDOR_ID
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "string.h" // strcmp
#அந்தர்பாவ "util.h" // handle_155f, handle_157f

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

முழு VGAHookHandlerType VARFSEG;

நிலையான வெறுமை
handle_155fXX(கட்டமைப்பு bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

நிலையான வெறுமை
handle_157fXX(கட்டமைப்பு bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

முழு ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

நிலையான வெறுமை
via_155f01(கட்டமைப்பு bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

நிலையான வெறுமை
via_155f02(கட்டமைப்பு bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

நிலையான வெறுமை
via_155f18(கட்டமைப்பு bregs *regs)
{
    முழு fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    எனில் (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        திரும்பு;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

நிலையான வெறுமை
via_155f19(கட்டமைப்பு bregs *regs)
{
    set_invalid_silent(regs);
}

நிலையான வெறுமை
via_155f(கட்டமைப்பு bregs *regs)
{
    தேர்வு (regs->al) {
    நிலை 0x01: via_155f01(regs); நிறுத்து;
    நிலை 0x02: via_155f02(regs); நிறுத்து;
    நிலை 0x18: via_155f18(regs); நிறுத்து;
    நிலை 0x19: via_155f19(regs); நிறுத்து;
    இயல்பு:   handle_155fXX(regs); நிறுத்து;
    }
}

நிலையான முழு
getFBSize(கட்டமைப்பு pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    எனில் (!(reg & 0x80))
        திரும்பு -1;

    நிலையான u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    திரும்பு mem_power[(reg >> 4) & 0x7];
}

நிலையான முழு
getViaRamSpeed(கட்டமைப்பு pci_device *pci)
{
    திரும்பு (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

நிலையான முழு
getAMDRamSpeed(வெறுமை)
{
    கட்டமைப்பு pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    எனில் (!pci)
        திரும்பு -1;

    /* mem clk 0 = DDR2 400 */
    திரும்பு (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

நிலையான வெறுமை
via_setup(கட்டமைப்பு pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    கட்டமைப்பு pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    எனில் (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        திரும்பு;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    எனில் (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        திரும்பு;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

நிலையான வெறுமை
intel_155f35(கட்டமைப்பு bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

நிலையான வெறுமை
intel_155f40(கட்டமைப்பு bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

நிலையான வெறுமை
intel_155f50(கட்டமைப்பு bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

நிலையான வெறுமை
intel_155f(கட்டமைப்பு bregs *regs)
{
    தேர்வு (regs->al) {
    நிலை 0x35: intel_155f35(regs); நிறுத்து;
    நிலை 0x40: intel_155f40(regs); நிறுத்து;
    நிலை 0x50: intel_155f50(regs); நிறுத்து;
    இயல்பு:   handle_155fXX(regs); நிறுத்து;
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

நிலையான வெறுமை
intel_setup(கட்டமைப்பு pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

நிலையான வெறுமை
roda_setup(கட்டமைப்பு pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

நிலையான வெறுமை
kontron_setup(கட்டமைப்பு pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

நிலையான வெறுமை
getac_setup(கட்டமைப்பு pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

நிலையான வெறுமை
smi_157f02(கட்டமைப்பு bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

நிலையான வெறுமை
smi_157f14(கட்டமைப்பு bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

நிலையான வெறுமை
smi_157f(கட்டமைப்பு bregs *regs)
{
    தேர்வு (regs->al) {
    நிலை 0x02: smi_157f02(regs); நிறுத்து;
    நிலை 0x14: smi_157f14(regs); நிறுத்து;
    இயல்பு:   handle_157fXX(regs); நிறுத்து;
    }
}

நிலையான வெறுமை
winent_mb6047_setup(கட்டமைப்பு pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
வெறுமை
handle_155f(கட்டமைப்பு bregs *regs)
{
    எனில் (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        திரும்பு;
    }

    முழு htype = GET_GLOBAL(VGAHookHandlerType);
    தேர்வு (htype) {
    நிலை VH_VIA:   via_155f(regs); நிறுத்து;
    நிலை VH_INTEL: intel_155f(regs); நிறுத்து;
    இயல்பு:       handle_155fXX(regs); நிறுத்து;
    }
}

// Main 16bit entry point
வெறுமை
handle_157f(கட்டமைப்பு bregs *regs)
{
    எனில் (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        திரும்பு;
    }

    முழு htype = GET_GLOBAL(VGAHookHandlerType);
    தேர்வு (htype) {
    நிலை VH_SMI:   smi_157f(regs); நிறுத்து;
    இயல்பு:       handle_157fXX(regs); நிறுத்து;
    }
}

// Setup
வெறுமை
vgahook_setup(கட்டமைப்பு pci_device *pci)
{
    எனில் (!CONFIG_VGAHOOKS)
        திரும்பு;

    எனில் (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    இல்லையெனில் எனில் (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    இல்லையெனில் எனில் (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    இல்லையெனில் எனில் (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    இல்லையெனில் எனில் (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    இல்லையெனில் எனில் (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
