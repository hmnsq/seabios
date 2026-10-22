// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "bregs.h" // set_code_invalid
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "hw/pci.h" // pci_config_readb
#अन्तर्भाव "hw/pcidevice.h" // pci_find_device
#अन्तर्भाव "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#अन्तर्भाव "hw/pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "string.h" // strcmp
#अन्तर्भाव "util.h" // handle_155f, handle_157f

#परिभाषय VH_द्वारा 1
#परिभाषय VH_INTEL 2
#परिभाषय VH_SMI 3

पूर्णाङ्क VGAHookHandlerType VARFSEG;

स्थिर शून्य
handle_155fXX(संरचना bregs *regs)
{
    समुच्चयः_संहिता_unimplemented(regs, RET_EUNSUPPORTED);
}

स्थिर शून्य
handle_157fXX(संरचना bregs *regs)
{
    समुच्चयः_संहिता_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

पूर्णाङ्क द्वारा_FBsize VARFSEG, द्वारा_Ram_जवः VARFSEG;

स्थिर शून्य
द्वारा_155f01(संरचना bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    समुच्चयः_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

स्थिर शून्य
द्वारा_155f02(संरचना bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    समुच्चयः_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

स्थिर शून्य
द्वारा_155f18(संरचना bregs *regs)
{
    पूर्णाङ्क fbsize = GET_GLOBAL(द्वारा_FBsize), ramspeed = GET_GLOBAL(द्वारा_Ram_जवः);
    यदि (fbsize < 0 || ramspeed < 0) {
        समुच्चयः_संहिता_invalid(regs, RET_EUNSUPPORTED);
        प्रत्यागम;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    समुच्चयः_success(regs);
}

स्थिर शून्य
द्वारा_155f19(संरचना bregs *regs)
{
    समुच्चयः_invalid_silent(regs);
}

स्थिर शून्य
द्वारा_155f(संरचना bregs *regs)
{
    विकल्प (regs->al) {
    पक्ष 0x01: द्वारा_155f01(regs); भङ्ग;
    पक्ष 0x02: द्वारा_155f02(regs); भङ्ग;
    पक्ष 0x18: द्वारा_155f18(regs); भङ्ग;
    पक्ष 0x19: द्वारा_155f19(regs); भङ्ग;
    स्वभाव:   handle_155fXX(regs); भङ्ग;
    }
}

स्थिर पूर्णाङ्क
getFBSize(संरचना pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    यदि (!(reg & 0x80))
        प्रत्यागम -1;

    स्थिर u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    प्रत्यागम mem_power[(reg >> 4) & 0x7];
}

स्थिर पूर्णाङ्क
get_द्वारा_Ram_जवः(संरचना pci_device *pci)
{
    प्रत्यागम (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

स्थिर पूर्णाङ्क
get_AMDRam_जवः(शून्य)
{
    संरचना pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    यदि (!pci)
        प्रत्यागम -1;

    /* mem clk 0 = DDR2 400 */
    प्रत्यागम (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

#परिभाषय PCI_DEVICE_ID_द्वारा_K8_M890_CE_3    0x3336
#परिभाषय PCI_DEVICE_ID_द्वारा_VX855_MEMCTRL 0x3409

स्थिर शून्य
द्वारा_सज्जता(संरचना pci_device *pci)
{
    VGAHookHandlerType = VH_द्वारा;

    संरचना pci_device *d = pci_find_device(PCI_VENDOR_ID_द्वारा
                                           , PCI_DEVICE_ID_द्वारा_K8_M890_CE_3);
    यदि (d) {
        द्वारा_FBsize = getFBSize(d);
        द्वारा_Ram_जवः = get_AMDRam_जवः();
        प्रत्यागम;
    }
    d = pci_find_device(PCI_VENDOR_ID_द्वारा, PCI_DEVICE_ID_द्वारा_VX855_MEMCTRL);
    यदि (d) {
        द्वारा_FBsize = getFBSize(d);
        द्वारा_Ram_जवः = get_द्वारा_Ram_जवः(d);
        प्रत्यागम;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    द्वारा_FBsize = 5; // 32M frame buffer
    द्वारा_Ram_जवः = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

स्थिर शून्य
intel_155f35(संरचना bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    समुच्चयः_success(regs);
}

स्थिर शून्य
intel_155f40(संरचना bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    समुच्चयः_success(regs);
}

स्थिर शून्य
intel_155f50(संरचना bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    समुच्चयः_success(regs);
}

स्थिर शून्य
intel_155f(संरचना bregs *regs)
{
    विकल्प (regs->al) {
    पक्ष 0x35: intel_155f35(regs); भङ्ग;
    पक्ष 0x40: intel_155f40(regs); भङ्ग;
    पक्ष 0x50: intel_155f50(regs); भङ्ग;
    स्वभाव:   handle_155fXX(regs); भङ्ग;
    }
}

#परिभाषय BOOT_DISPLAY_DEFAULT    (0)
#परिभाषय BOOT_DISPLAY_CRT        (1 << 0)
#परिभाषय BOOT_DISPLAY_TV         (1 << 1)
#परिभाषय BOOT_DISPLAY_EFP        (1 << 2)
#परिभाषय BOOT_DISPLAY_LCD        (1 << 3)
#परिभाषय BOOT_DISPLAY_CRT2       (1 << 4)
#परिभाषय BOOT_DISPLAY_TV2        (1 << 5)
#परिभाषय BOOT_DISPLAY_EFP2       (1 << 6)
#परिभाषय BOOT_DISPLAY_LCD2       (1 << 7)

स्थिर शून्य
intel_सज्जता(संरचना pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

स्थिर शून्य
roda_सज्जता(संरचना pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

स्थिर शून्य
kontron_सज्जता(संरचना pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

स्थिर शून्य
getac_सज्जता(संरचना pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

स्थिर शून्य
smi_157f02(संरचना bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    समुच्चयः_success(regs);
}

स्थिर शून्य
smi_157f14(संरचना bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    समुच्चयः_success(regs);
}

स्थिर शून्य
smi_157f(संरचना bregs *regs)
{
    विकल्प (regs->al) {
    पक्ष 0x02: smi_157f02(regs); भङ्ग;
    पक्ष 0x14: smi_157f14(regs); भङ्ग;
    स्वभाव:   handle_157fXX(regs); भङ्ग;
    }
}

स्थिर शून्य
winent_mb6047_सज्जता(संरचना pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
शून्य
handle_155f(संरचना bregs *regs)
{
    यदि (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        प्रत्यागम;
    }

    पूर्णाङ्क htype = GET_GLOBAL(VGAHookHandlerType);
    विकल्प (htype) {
    पक्ष VH_द्वारा:   द्वारा_155f(regs); भङ्ग;
    पक्ष VH_INTEL: intel_155f(regs); भङ्ग;
    स्वभाव:       handle_155fXX(regs); भङ्ग;
    }
}

// Main 16bit entry point
शून्य
handle_157f(संरचना bregs *regs)
{
    यदि (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        प्रत्यागम;
    }

    पूर्णाङ्क htype = GET_GLOBAL(VGAHookHandlerType);
    विकल्प (htype) {
    पक्ष VH_SMI:   smi_157f(regs); भङ्ग;
    स्वभाव:       handle_157fXX(regs); भङ्ग;
    }
}

//सज्जता
शून्य
vgahook_सज्जता(संरचना pci_device *pci)
{
    यदि (!CONFIG_VGAHOOKS)
        प्रत्यागम;

    यदि (वाक्यं_तुलय(CBvendor, "KONTRON") == 0 && वाक्यं_तुलय(CBpart, "986LCD-M") == 0)
        kontron_सज्जता(pci);
    अन्यथा यदि (वाक्यं_तुलय(CBvendor, "GETAC") == 0 && वाक्यं_तुलय(CBpart, "P470") == 0)
        getac_सज्जता(pci);
    अन्यथा यदि (वाक्यं_तुलय(CBvendor, "RODA") == 0 && वाक्यं_तुलय(CBpart, "RK886EX") == 0)
        roda_सज्जता(pci);
    अन्यथा यदि (वाक्यं_तुलय(CBvendor, "Win Enterprise") == 0 && वाक्यं_तुलय(CBpart, "MB6047") == 0)
        winent_mb6047_सज्जता(pci);
    अन्यथा यदि (pci->vendor == PCI_VENDOR_ID_द्वारा)
        द्वारा_सज्जता(pci);
    अन्यथा यदि (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_सज्जता(pci);
}
