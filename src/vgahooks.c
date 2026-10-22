// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "bregs.h" // set_code_invalid
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_config_readb
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // pci_find_device
#ଅନ୍ତର୍ଭାଵ "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#ଅନ୍ତର୍ଭାଵ "hw/pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "string.h" // strcmp
#ଅନ୍ତର୍ଭାଵ "util.h" // handle_155f, handle_157f

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

ପୂର୍ଣ୍ଣସଂଖ୍ୟା VGAHookHandlerType VARFSEG;

ସ୍ଥିର ଶୂନ୍ୟ
handle_155fXX(ସଂରଚନା bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_157fXX(ସଂରଚନା bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

ସ୍ଥିର ଶୂନ୍ୟ
via_155f01(ସଂରଚନା bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

ସ୍ଥିର ଶୂନ୍ୟ
via_155f02(ସଂରଚନା bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

ସ୍ଥିର ଶୂନ୍ୟ
via_155f18(ସଂରଚନା bregs *regs)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    ଯଦି (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ଫେରନ୍ତୁ;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
via_155f19(ସଂରଚନା bregs *regs)
{
    set_invalid_silent(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
via_155f(ସଂରଚନା bregs *regs)
{
    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x01: via_155f01(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: via_155f02(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x18: via_155f18(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x19: via_155f19(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_155fXX(regs); ଭାଙ୍ଗ;
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
getFBSize(ସଂରଚନା pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    ଯଦି (!(reg & 0x80))
        ଫେରନ୍ତୁ -1;

    ସ୍ଥିର u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    ଫେରନ୍ତୁ mem_power[(reg >> 4) & 0x7];
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
getViaRamSpeed(ସଂରଚନା pci_device *pci)
{
    ଫେରନ୍ତୁ (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
getAMDRamSpeed(ଶୂନ୍ୟ)
{
    ସଂରଚନା pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    ଯଦି (!pci)
        ଫେରନ୍ତୁ -1;

    /* mem clk 0 = DDR2 400 */
    ଫେରନ୍ତୁ (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

ସ୍ଥିର ଶୂନ୍ୟ
via_setup(ସଂରଚନା pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    ସଂରଚନା pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    ଯଦି (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        ଫେରନ୍ତୁ;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    ଯଦି (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        ଫେରନ୍ତୁ;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

ସ୍ଥିର ଶୂନ୍ୟ
intel_155f35(ସଂରଚନା bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
intel_155f40(ସଂରଚନା bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
intel_155f50(ସଂରଚନା bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
intel_155f(ସଂରଚନା bregs *regs)
{
    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x35: intel_155f35(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x40: intel_155f40(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x50: intel_155f50(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_155fXX(regs); ଭାଙ୍ଗ;
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

ସ୍ଥିର ଶୂନ୍ୟ
intel_setup(ସଂରଚନା pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

ସ୍ଥିର ଶୂନ୍ୟ
roda_setup(ସଂରଚନା pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

ସ୍ଥିର ଶୂନ୍ୟ
kontron_setup(ସଂରଚନା pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

ସ୍ଥିର ଶୂନ୍ୟ
getac_setup(ସଂରଚନା pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

ସ୍ଥିର ଶୂନ୍ୟ
smi_157f02(ସଂରଚନା bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
smi_157f14(ସଂରଚନା bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
smi_157f(ସଂରଚନା bregs *regs)
{
    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x02: smi_157f02(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x14: smi_157f14(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_157fXX(regs); ଭାଙ୍ଗ;
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
winent_mb6047_setup(ସଂରଚନା pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
ଶୂନ୍ୟ
handle_155f(ସଂରଚନା bregs *regs)
{
    ଯଦି (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        ଫେରନ୍ତୁ;
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା htype = GET_GLOBAL(VGAHookHandlerType);
    ବାଛ (htype) {
    ସ୍ଥିତି VH_VIA:   via_155f(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି VH_INTEL: intel_155f(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:       handle_155fXX(regs); ଭାଙ୍ଗ;
    }
}

// Main 16bit entry point
ଶୂନ୍ୟ
handle_157f(ସଂରଚନା bregs *regs)
{
    ଯଦି (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        ଫେରନ୍ତୁ;
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା htype = GET_GLOBAL(VGAHookHandlerType);
    ବାଛ (htype) {
    ସ୍ଥିତି VH_SMI:   smi_157f(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:       handle_157fXX(regs); ଭାଙ୍ଗ;
    }
}

// Setup
ଶୂନ୍ୟ
vgahook_setup(ସଂରଚନା pci_device *pci)
{
    ଯଦି (!CONFIG_VGAHOOKS)
        ଫେରନ୍ତୁ;

    ଯଦି (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    ଅନ୍ୟଥା ଯଦି (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    ଅନ୍ୟଥା ଯଦି (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    ଅନ୍ୟଥା ଯଦି (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    ଅନ୍ୟଥା ଯଦି (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    ଅନ୍ୟଥା ଯଦି (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
