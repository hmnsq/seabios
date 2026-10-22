// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "bregs.h" // set_code_invalid
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "hw/pci.h" // pci_config_readb
#অন্তর্ভাব "hw/pcidevice.h" // pci_find_device
#অন্তর্ভাব "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#অন্তর্ভাব "hw/pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "string.h" // strcmp
#অন্তর্ভাব "util.h" // handle_155f, handle_157f

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

পূর্ণসংখ্যা VGAHookHandlerType VARFSEG;

স্থির শূন্য
handle_155fXX(কাঠামো bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

স্থির শূন্য
handle_157fXX(কাঠামো bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

পূর্ণসংখ্যা ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

স্থির শূন্য
via_155f01(কাঠামো bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

স্থির শূন্য
via_155f02(কাঠামো bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

স্থির শূন্য
via_155f18(কাঠামো bregs *regs)
{
    পূর্ণসংখ্যা fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    যদি (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ফেরত;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

স্থির শূন্য
via_155f19(কাঠামো bregs *regs)
{
    set_invalid_silent(regs);
}

স্থির শূন্য
via_155f(কাঠামো bregs *regs)
{
    নির্বাচন (regs->al) {
    ক্ষেত্র 0x01: via_155f01(regs); বিরতি;
    ক্ষেত্র 0x02: via_155f02(regs); বিরতি;
    ক্ষেত্র 0x18: via_155f18(regs); বিরতি;
    ক্ষেত্র 0x19: via_155f19(regs); বিরতি;
    পূর্বনির্ধারিত:   handle_155fXX(regs); বিরতি;
    }
}

স্থির পূর্ণসংখ্যা
getFBSize(কাঠামো pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    যদি (!(reg & 0x80))
        ফেরত -1;

    স্থির u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    ফেরত mem_power[(reg >> 4) & 0x7];
}

স্থির পূর্ণসংখ্যা
getViaRamSpeed(কাঠামো pci_device *pci)
{
    ফেরত (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

স্থির পূর্ণসংখ্যা
getAMDRamSpeed(শূন্য)
{
    কাঠামো pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    যদি (!pci)
        ফেরত -1;

    /* mem clk 0 = DDR2 400 */
    ফেরত (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

স্থির শূন্য
via_setup(কাঠামো pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    কাঠামো pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    যদি (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        ফেরত;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    যদি (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        ফেরত;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

স্থির শূন্য
intel_155f35(কাঠামো bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

স্থির শূন্য
intel_155f40(কাঠামো bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

স্থির শূন্য
intel_155f50(কাঠামো bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

স্থির শূন্য
intel_155f(কাঠামো bregs *regs)
{
    নির্বাচন (regs->al) {
    ক্ষেত্র 0x35: intel_155f35(regs); বিরতি;
    ক্ষেত্র 0x40: intel_155f40(regs); বিরতি;
    ক্ষেত্র 0x50: intel_155f50(regs); বিরতি;
    পূর্বনির্ধারিত:   handle_155fXX(regs); বিরতি;
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

স্থির শূন্য
intel_setup(কাঠামো pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

স্থির শূন্য
roda_setup(কাঠামো pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

স্থির শূন্য
kontron_setup(কাঠামো pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

স্থির শূন্য
getac_setup(কাঠামো pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

স্থির শূন্য
smi_157f02(কাঠামো bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

স্থির শূন্য
smi_157f14(কাঠামো bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

স্থির শূন্য
smi_157f(কাঠামো bregs *regs)
{
    নির্বাচন (regs->al) {
    ক্ষেত্র 0x02: smi_157f02(regs); বিরতি;
    ক্ষেত্র 0x14: smi_157f14(regs); বিরতি;
    পূর্বনির্ধারিত:   handle_157fXX(regs); বিরতি;
    }
}

স্থির শূন্য
winent_mb6047_setup(কাঠামো pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
শূন্য
handle_155f(কাঠামো bregs *regs)
{
    যদি (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        ফেরত;
    }

    পূর্ণসংখ্যা htype = GET_GLOBAL(VGAHookHandlerType);
    নির্বাচন (htype) {
    ক্ষেত্র VH_VIA:   via_155f(regs); বিরতি;
    ক্ষেত্র VH_INTEL: intel_155f(regs); বিরতি;
    পূর্বনির্ধারিত:       handle_155fXX(regs); বিরতি;
    }
}

// Main 16bit entry point
শূন্য
handle_157f(কাঠামো bregs *regs)
{
    যদি (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        ফেরত;
    }

    পূর্ণসংখ্যা htype = GET_GLOBAL(VGAHookHandlerType);
    নির্বাচন (htype) {
    ক্ষেত্র VH_SMI:   smi_157f(regs); বিরতি;
    পূর্বনির্ধারিত:       handle_157fXX(regs); বিরতি;
    }
}

// Setup
শূন্য
vgahook_setup(কাঠামো pci_device *pci)
{
    যদি (!CONFIG_VGAHOOKS)
        ফেরত;

    যদি (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    অন্যথা যদি (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    অন্যথা যদি (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    অন্যথা যদি (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    অন্যথা যদি (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    অন্যথা যদি (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
