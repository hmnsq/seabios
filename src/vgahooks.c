// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "bregs.h" // set_code_invalid
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "hw/pci.h" // pci_config_readb
#అన్తర్భావ "hw/pcidevice.h" // pci_find_device
#అన్తర్భావ "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#అన్తర్భావ "hw/pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "string.h" // strcmp
#అన్తర్భావ "util.h" // handle_155f, handle_157f

#నిర్వచించు VH_ద్వారా 1
#నిర్వచించు VH_INTEL 2
#నిర్వచించు VH_SMI 3

పూర్ణాంకం VGAHookHandlerType VARFSEG;

స్థిర శూన్య
handle_155fXX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EUNSUPPORTED);
}

స్థిర శూన్య
handle_157fXX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

పూర్ణాంకం ద్వారా_FBsize VARFSEG, ద్వారా_Ram_వేగం VARFSEG;

స్థిర శూన్య
ద్వారా_155f01(నిర్మాణం bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    సమితి_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

స్థిర శూన్య
ద్వారా_155f02(నిర్మాణం bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    సమితి_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

స్థిర శూన్య
ద్వారా_155f18(నిర్మాణం bregs *regs)
{
    పూర్ణాంకం fbsize = GET_GLOBAL(ద్వారా_FBsize), ramspeed = GET_GLOBAL(ద్వారా_Ram_వేగం);
    ఒకవేళ (fbsize < 0 || ramspeed < 0) {
        సమితి_సంకేతం_invalid(regs, RET_EUNSUPPORTED);
        తిరిగి;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    సమితి_success(regs);
}

స్థిర శూన్య
ద్వారా_155f19(నిర్మాణం bregs *regs)
{
    సమితి_invalid_silent(regs);
}

స్థిర శూన్య
ద్వారా_155f(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->al) {
    స్థితి 0x01: ద్వారా_155f01(regs); ఆపు;
    స్థితి 0x02: ద్వారా_155f02(regs); ఆపు;
    స్థితి 0x18: ద్వారా_155f18(regs); ఆపు;
    స్థితి 0x19: ద్వారా_155f19(regs); ఆపు;
    అప్రమేయ:   handle_155fXX(regs); ఆపు;
    }
}

స్థిర పూర్ణాంకం
getFBSize(నిర్మాణం pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    ఒకవేళ (!(reg & 0x80))
        తిరిగి -1;

    స్థిర u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    తిరిగి mem_power[(reg >> 4) & 0x7];
}

స్థిర పూర్ణాంకం
get_ద్వారా_Ram_వేగం(నిర్మాణం pci_device *pci)
{
    తిరిగి (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

స్థిర పూర్ణాంకం
get_AMDRam_వేగం(శూన్య)
{
    నిర్మాణం pci_device *pci = pci_కనుగొను_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    ఒకవేళ (!pci)
        తిరిగి -1;

    /* mem clk 0 = DDR2 400 */
    తిరిగి (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

#నిర్వచించు PCI_DEVICE_ID_ద్వారా_K8_M890_CE_3    0x3336
#నిర్వచించు PCI_DEVICE_ID_ద్వారా_VX855_MEMCTRL 0x3409

స్థిర శూన్య
ద్వారా_అమరిక(నిర్మాణం pci_device *pci)
{
    VGAHookHandlerType = VH_ద్వారా;

    నిర్మాణం pci_device *d = pci_కనుగొను_device(PCI_VENDOR_ID_ద్వారా
                                           , PCI_DEVICE_ID_ద్వారా_K8_M890_CE_3);
    ఒకవేళ (d) {
        ద్వారా_FBsize = getFBSize(d);
        ద్వారా_Ram_వేగం = get_AMDRam_వేగం();
        తిరిగి;
    }
    d = pci_కనుగొను_device(PCI_VENDOR_ID_ద్వారా, PCI_DEVICE_ID_ద్వారా_VX855_MEMCTRL);
    ఒకవేళ (d) {
        ద్వారా_FBsize = getFBSize(d);
        ద్వారా_Ram_వేగం = get_ద్వారా_Ram_వేగం(d);
        తిరిగి;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ద్వారా_FBsize = 5; // 32M frame buffer
    ద్వారా_Ram_వేగం = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

స్థిర శూన్య
intel_155f35(నిర్మాణం bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    సమితి_success(regs);
}

స్థిర శూన్య
intel_155f40(నిర్మాణం bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    సమితి_success(regs);
}

స్థిర శూన్య
intel_155f50(నిర్మాణం bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    సమితి_success(regs);
}

స్థిర శూన్య
intel_155f(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->al) {
    స్థితి 0x35: intel_155f35(regs); ఆపు;
    స్థితి 0x40: intel_155f40(regs); ఆపు;
    స్థితి 0x50: intel_155f50(regs); ఆపు;
    అప్రమేయ:   handle_155fXX(regs); ఆపు;
    }
}

#నిర్వచించు BOOT_DISPLAY_DEFAULT    (0)
#నిర్వచించు BOOT_DISPLAY_CRT        (1 << 0)
#నిర్వచించు BOOT_DISPLAY_TV         (1 << 1)
#నిర్వచించు BOOT_DISPLAY_EFP        (1 << 2)
#నిర్వచించు BOOT_DISPLAY_LCD        (1 << 3)
#నిర్వచించు BOOT_DISPLAY_CRT2       (1 << 4)
#నిర్వచించు BOOT_DISPLAY_TV2        (1 << 5)
#నిర్వచించు BOOT_DISPLAY_EFP2       (1 << 6)
#నిర్వచించు BOOT_DISPLAY_LCD2       (1 << 7)

స్థిర శూన్య
intel_అమరిక(నిర్మాణం pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

స్థిర శూన్య
roda_అమరిక(నిర్మాణం pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

స్థిర శూన్య
kontron_అమరిక(నిర్మాణం pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

స్థిర శూన్య
getac_అమరిక(నిర్మాణం pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

స్థిర శూన్య
smi_157f02(నిర్మాణం bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    సమితి_success(regs);
}

స్థిర శూన్య
smi_157f14(నిర్మాణం bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    సమితి_success(regs);
}

స్థిర శూన్య
smi_157f(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->al) {
    స్థితి 0x02: smi_157f02(regs); ఆపు;
    స్థితి 0x14: smi_157f14(regs); ఆపు;
    అప్రమేయ:   handle_157fXX(regs); ఆపు;
    }
}

స్థిర శూన్య
winent_mb6047_అమరిక(నిర్మాణం pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
శూన్య
handle_155f(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        తిరిగి;
    }

    పూర్ణాంకం htype = GET_GLOBAL(VGAHookHandlerType);
    ఎంపిక (htype) {
    స్థితి VH_ద్వారా:   ద్వారా_155f(regs); ఆపు;
    స్థితి VH_INTEL: intel_155f(regs); ఆపు;
    అప్రమేయ:       handle_155fXX(regs); ఆపు;
    }
}

// Main 16bit entry point
శూన్య
handle_157f(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        తిరిగి;
    }

    పూర్ణాంకం htype = GET_GLOBAL(VGAHookHandlerType);
    ఎంపిక (htype) {
    స్థితి VH_SMI:   smi_157f(regs); ఆపు;
    అప్రమేయ:       handle_157fXX(regs); ఆపు;
    }
}

//అమరిక
శూన్య
vgahook_అమరిక(నిర్మాణం pci_device *pci)
{
    ఒకవేళ (!CONFIG_VGAHOOKS)
        తిరిగి;

    ఒకవేళ (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_అమరిక(pci);
    లేకపోతే ఒకవేళ (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_అమరిక(pci);
    లేకపోతే ఒకవేళ (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_అమరిక(pci);
    లేకపోతే ఒకవేళ (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_అమరిక(pci);
    లేకపోతే ఒకవేళ (pci->vendor == PCI_VENDOR_ID_ద్వారా)
        ద్వారా_అమరిక(pci);
    లేకపోతే ఒకవేళ (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_అమరిక(pci);
}
