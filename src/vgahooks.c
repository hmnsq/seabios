// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "bregs.h" // set_code_invalid
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "hw/pci.h" // pci_config_readb
#અન્તર્ભાવ "hw/pcidevice.h" // pci_find_device
#અન્તર્ભાવ "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#અન્તર્ભાવ "hw/pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "string.h" // strcmp
#અન્તર્ભાવ "util.h" // handle_155f, handle_157f

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

પૂર્ણાંક VGAHookHandlerType VARFSEG;

સ્થિર શૂન્ય
handle_155fXX(સંરચના bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

સ્થિર શૂન્ય
handle_157fXX(સંરચના bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

પૂર્ણાંક ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

સ્થિર શૂન્ય
via_155f01(સંરચના bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

સ્થિર શૂન્ય
via_155f02(સંરચના bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

સ્થિર શૂન્ય
via_155f18(સંરચના bregs *regs)
{
    પૂર્ણાંક fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    જો (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        પાછા;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

સ્થિર શૂન્ય
via_155f19(સંરચના bregs *regs)
{
    set_invalid_silent(regs);
}

સ્થિર શૂન્ય
via_155f(સંરચના bregs *regs)
{
    પસંદગી (regs->al) {
    સ્થિતિ 0x01: via_155f01(regs); તોડો;
    સ્થિતિ 0x02: via_155f02(regs); તોડો;
    સ્થિતિ 0x18: via_155f18(regs); તોડો;
    સ્થિતિ 0x19: via_155f19(regs); તોડો;
    મૂળભૂત:   handle_155fXX(regs); તોડો;
    }
}

સ્થિર પૂર્ણાંક
getFBSize(સંરચના pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    જો (!(reg & 0x80))
        પાછા -1;

    સ્થિર u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    પાછા mem_power[(reg >> 4) & 0x7];
}

સ્થિર પૂર્ણાંક
getViaRamSpeed(સંરચના pci_device *pci)
{
    પાછા (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

સ્થિર પૂર્ણાંક
getAMDRamSpeed(શૂન્ય)
{
    સંરચના pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    જો (!pci)
        પાછા -1;

    /* mem clk 0 = DDR2 400 */
    પાછા (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

સ્થિર શૂન્ય
via_setup(સંરચના pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    સંરચના pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    જો (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        પાછા;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    જો (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        પાછા;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

સ્થિર શૂન્ય
intel_155f35(સંરચના bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

સ્થિર શૂન્ય
intel_155f40(સંરચના bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

સ્થિર શૂન્ય
intel_155f50(સંરચના bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

સ્થિર શૂન્ય
intel_155f(સંરચના bregs *regs)
{
    પસંદગી (regs->al) {
    સ્થિતિ 0x35: intel_155f35(regs); તોડો;
    સ્થિતિ 0x40: intel_155f40(regs); તોડો;
    સ્થિતિ 0x50: intel_155f50(regs); તોડો;
    મૂળભૂત:   handle_155fXX(regs); તોડો;
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

સ્થિર શૂન્ય
intel_setup(સંરચના pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

સ્થિર શૂન્ય
roda_setup(સંરચના pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

સ્થિર શૂન્ય
kontron_setup(સંરચના pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

સ્થિર શૂન્ય
getac_setup(સંરચના pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

સ્થિર શૂન્ય
smi_157f02(સંરચના bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

સ્થિર શૂન્ય
smi_157f14(સંરચના bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

સ્થિર શૂન્ય
smi_157f(સંરચના bregs *regs)
{
    પસંદગી (regs->al) {
    સ્થિતિ 0x02: smi_157f02(regs); તોડો;
    સ્થિતિ 0x14: smi_157f14(regs); તોડો;
    મૂળભૂત:   handle_157fXX(regs); તોડો;
    }
}

સ્થિર શૂન્ય
winent_mb6047_setup(સંરચના pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
શૂન્ય
handle_155f(સંરચના bregs *regs)
{
    જો (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        પાછા;
    }

    પૂર્ણાંક htype = GET_GLOBAL(VGAHookHandlerType);
    પસંદગી (htype) {
    સ્થિતિ VH_VIA:   via_155f(regs); તોડો;
    સ્થિતિ VH_INTEL: intel_155f(regs); તોડો;
    મૂળભૂત:       handle_155fXX(regs); તોડો;
    }
}

// Main 16bit entry point
શૂન્ય
handle_157f(સંરચના bregs *regs)
{
    જો (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        પાછા;
    }

    પૂર્ણાંક htype = GET_GLOBAL(VGAHookHandlerType);
    પસંદગી (htype) {
    સ્થિતિ VH_SMI:   smi_157f(regs); તોડો;
    મૂળભૂત:       handle_157fXX(regs); તોડો;
    }
}

// Setup
શૂન્ય
vgahook_setup(સંરચના pci_device *pci)
{
    જો (!CONFIG_VGAHOOKS)
        પાછા;

    જો (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    નહીંતર જો (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    નહીંતર જો (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    નહીંતર જો (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    નહીંતર જો (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    નહીંતર જો (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
