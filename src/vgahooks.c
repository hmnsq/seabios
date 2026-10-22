// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "bregs.h" // set_code_invalid
#समावेश "config.h" // CONFIG_*
#समावेश "hw/pci.h" // pci_config_readb
#समावेश "hw/pcidevice.h" // pci_find_device
#समावेश "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#समावेश "hw/pci_regs.h" // PCI_VENDOR_ID
#समावेश "output.h" // dprintf
#समावेश "string.h" // strcmp
#समावेश "util.h" // handle_155f, handle_157f

#घोषणा VH_VIA 1
#घोषणा VH_INTEL 2
#घोषणा VH_SMI 3

पूर्णांक VGAHookHandlerType VARFSEG;

अटल व्योम
handle_155fXX(काष्ठा bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

अटल व्योम
handle_157fXX(काष्ठा bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

पूर्णांक ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

अटल व्योम
via_155f01(काष्ठा bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

अटल व्योम
via_155f02(काष्ठा bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

अटल व्योम
via_155f18(काष्ठा bregs *regs)
{
    पूर्णांक fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    अगर (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        वापस;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

अटल व्योम
via_155f19(काष्ठा bregs *regs)
{
    set_invalid_silent(regs);
}

अटल व्योम
via_155f(काष्ठा bregs *regs)
{
    चयन (regs->al) {
    हाल 0x01: via_155f01(regs); अवरोध;
    हाल 0x02: via_155f02(regs); अवरोध;
    हाल 0x18: via_155f18(regs); अवरोध;
    हाल 0x19: via_155f19(regs); अवरोध;
    शेष:   handle_155fXX(regs); अवरोध;
    }
}

अटल पूर्णांक
getFBSize(काष्ठा pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    अगर (!(reg & 0x80))
        वापस -1;

    अटल u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    वापस mem_power[(reg >> 4) & 0x7];
}

अटल पूर्णांक
getViaRamSpeed(काष्ठा pci_device *pci)
{
    वापस (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

अटल पूर्णांक
getAMDRamSpeed(व्योम)
{
    काष्ठा pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    अगर (!pci)
        वापस -1;

    /* mem clk 0 = DDR2 400 */
    वापस (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

#घोषणा PCI_DEVICE_ID_VIA_K8M890CE_3    0x3336
#घोषणा PCI_DEVICE_ID_VIA_VX855_MEMCTRL 0x3409

अटल व्योम
via_setup(काष्ठा pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    काष्ठा pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    अगर (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        वापस;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    अगर (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        वापस;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

अटल व्योम
intel_155f35(काष्ठा bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

अटल व्योम
intel_155f40(काष्ठा bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

अटल व्योम
intel_155f50(काष्ठा bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

अटल व्योम
intel_155f(काष्ठा bregs *regs)
{
    चयन (regs->al) {
    हाल 0x35: intel_155f35(regs); अवरोध;
    हाल 0x40: intel_155f40(regs); अवरोध;
    हाल 0x50: intel_155f50(regs); अवरोध;
    शेष:   handle_155fXX(regs); अवरोध;
    }
}

#घोषणा BOOT_DISPLAY_DEFAULT    (0)
#घोषणा BOOT_DISPLAY_CRT        (1 << 0)
#घोषणा BOOT_DISPLAY_TV         (1 << 1)
#घोषणा BOOT_DISPLAY_EFP        (1 << 2)
#घोषणा BOOT_DISPLAY_LCD        (1 << 3)
#घोषणा BOOT_DISPLAY_CRT2       (1 << 4)
#घोषणा BOOT_DISPLAY_TV2        (1 << 5)
#घोषणा BOOT_DISPLAY_EFP2       (1 << 6)
#घोषणा BOOT_DISPLAY_LCD2       (1 << 7)

अटल व्योम
intel_setup(काष्ठा pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

अटल व्योम
roda_setup(काष्ठा pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

अटल व्योम
kontron_setup(काष्ठा pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

अटल व्योम
getac_setup(काष्ठा pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

अटल व्योम
smi_157f02(काष्ठा bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

अटल व्योम
smi_157f14(काष्ठा bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

अटल व्योम
smi_157f(काष्ठा bregs *regs)
{
    चयन (regs->al) {
    हाल 0x02: smi_157f02(regs); अवरोध;
    हाल 0x14: smi_157f14(regs); अवरोध;
    शेष:   handle_157fXX(regs); अवरोध;
    }
}

अटल व्योम
winent_mb6047_setup(काष्ठा pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
व्योम
handle_155f(काष्ठा bregs *regs)
{
    अगर (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        वापस;
    }

    पूर्णांक htype = GET_GLOBAL(VGAHookHandlerType);
    चयन (htype) {
    हाल VH_VIA:   via_155f(regs); अवरोध;
    हाल VH_INTEL: intel_155f(regs); अवरोध;
    शेष:       handle_155fXX(regs); अवरोध;
    }
}

// Main 16bit entry point
व्योम
handle_157f(काष्ठा bregs *regs)
{
    अगर (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        वापस;
    }

    पूर्णांक htype = GET_GLOBAL(VGAHookHandlerType);
    चयन (htype) {
    हाल VH_SMI:   smi_157f(regs); अवरोध;
    शेष:       handle_157fXX(regs); अवरोध;
    }
}

// Setup
व्योम
vgahook_setup(काष्ठा pci_device *pci)
{
    अगर (!CONFIG_VGAHOOKS)
        वापस;

    अगर (म_भेद(CBvendor, "KONTRON") == 0 && म_भेद(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    अन्यथा अगर (म_भेद(CBvendor, "GETAC") == 0 && म_भेद(CBpart, "P470") == 0)
        getac_setup(pci);
    अन्यथा अगर (म_भेद(CBvendor, "RODA") == 0 && म_भेद(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    अन्यथा अगर (म_भेद(CBvendor, "Win Enterprise") == 0 && म_भेद(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    अन्यथा अगर (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    अन्यथा अगर (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
