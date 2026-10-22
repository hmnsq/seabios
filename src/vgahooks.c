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

#تعریف_کرو VH_بذریعہ 1
#تعریف_کرو VH_INTEL 2
#تعریف_کرو VH_SMI 3

صحیح VGAHookHandlerType VARFSEG;

ساکن خالی
handle_155fXX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EUNSUPPORTED);
}

ساکن خالی
handle_157fXX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

صحیح بذریعہ_FBsize VARFSEG, بذریعہ_Ram_رفتار VARFSEG;

ساکن خالی
بذریعہ_155f01(ساخت bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    سیٹ_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

ساکن خالی
بذریعہ_155f02(ساخت bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    سیٹ_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

ساکن خالی
بذریعہ_155f18(ساخت bregs *regs)
{
    صحیح fbsize = GET_GLOBAL(بذریعہ_FBsize), ramspeed = GET_GLOBAL(بذریعہ_Ram_رفتار);
    اگر (fbsize < 0 || ramspeed < 0) {
        سیٹ_رمز_invalid(regs, RET_EUNSUPPORTED);
        واپس;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    سیٹ_success(regs);
}

ساکن خالی
بذریعہ_155f19(ساخت bregs *regs)
{
    سیٹ_invalid_silent(regs);
}

ساکن خالی
بذریعہ_155f(ساخت bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x01: بذریعہ_155f01(regs); توڑو;
    حالت 0x02: بذریعہ_155f02(regs); توڑو;
    حالت 0x18: بذریعہ_155f18(regs); توڑو;
    حالت 0x19: بذریعہ_155f19(regs); توڑو;
    طےشدہ:   handle_155fXX(regs); توڑو;
    }
}

ساکن صحیح
getFBSize(ساخت pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    اگر (!(reg & 0x80))
        واپس -1;

    ساکن u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    واپس mem_power[(reg >> 4) & 0x7];
}

ساکن صحیح
get_بذریعہ_Ram_رفتار(ساخت pci_device *pci)
{
    واپس (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

ساکن صحیح
get_AMDRam_رفتار(خالی)
{
    ساخت pci_device *pci = pci_ڈھونڈو_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    اگر (!pci)
        واپس -1;

    /* mem clk 0 = DDR2 400 */
    واپس (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

#تعریف_کرو PCI_DEVICE_ID_بذریعہ_K8_M890_CE_3    0x3336
#تعریف_کرو PCI_DEVICE_ID_بذریعہ_VX855_MEMCTRL 0x3409

ساکن خالی
بذریعہ_تیاری(ساخت pci_device *pci)
{
    VGAHookHandlerType = VH_بذریعہ;

    ساخت pci_device *d = pci_ڈھونڈو_device(PCI_VENDOR_ID_بذریعہ
                                           , PCI_DEVICE_ID_بذریعہ_K8_M890_CE_3);
    اگر (d) {
        بذریعہ_FBsize = getFBSize(d);
        بذریعہ_Ram_رفتار = get_AMDRam_رفتار();
        واپس;
    }
    d = pci_ڈھونڈو_device(PCI_VENDOR_ID_بذریعہ, PCI_DEVICE_ID_بذریعہ_VX855_MEMCTRL);
    اگر (d) {
        بذریعہ_FBsize = getFBSize(d);
        بذریعہ_Ram_رفتار = get_بذریعہ_Ram_رفتار(d);
        واپس;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    بذریعہ_FBsize = 5; // 32M frame buffer
    بذریعہ_Ram_رفتار = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

ساکن خالی
intel_155f35(ساخت bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    سیٹ_success(regs);
}

ساکن خالی
intel_155f40(ساخت bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    سیٹ_success(regs);
}

ساکن خالی
intel_155f50(ساخت bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    سیٹ_success(regs);
}

ساکن خالی
intel_155f(ساخت bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x35: intel_155f35(regs); توڑو;
    حالت 0x40: intel_155f40(regs); توڑو;
    حالت 0x50: intel_155f50(regs); توڑو;
    طےشدہ:   handle_155fXX(regs); توڑو;
    }
}

#تعریف_کرو BOOT_DISPLAY_DEFAULT    (0)
#تعریف_کرو BOOT_DISPLAY_CRT        (1 << 0)
#تعریف_کرو BOOT_DISPLAY_TV         (1 << 1)
#تعریف_کرو BOOT_DISPLAY_EFP        (1 << 2)
#تعریف_کرو BOOT_DISPLAY_LCD        (1 << 3)
#تعریف_کرو BOOT_DISPLAY_CRT2       (1 << 4)
#تعریف_کرو BOOT_DISPLAY_TV2        (1 << 5)
#تعریف_کرو BOOT_DISPLAY_EFP2       (1 << 6)
#تعریف_کرو BOOT_DISPLAY_LCD2       (1 << 7)

ساکن خالی
intel_تیاری(ساخت pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

ساکن خالی
roda_تیاری(ساخت pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

ساکن خالی
kontron_تیاری(ساخت pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

ساکن خالی
getac_تیاری(ساخت pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

ساکن خالی
smi_157f02(ساخت bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    سیٹ_success(regs);
}

ساکن خالی
smi_157f14(ساخت bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    سیٹ_success(regs);
}

ساکن خالی
smi_157f(ساخت bregs *regs)
{
    انتخاب (regs->al) {
    حالت 0x02: smi_157f02(regs); توڑو;
    حالت 0x14: smi_157f14(regs); توڑو;
    طےشدہ:   handle_157fXX(regs); توڑو;
    }
}

ساکن خالی
winent_mb6047_تیاری(ساخت pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
خالی
handle_155f(ساخت bregs *regs)
{
    اگر (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        واپس;
    }

    صحیح htype = GET_GLOBAL(VGAHookHandlerType);
    انتخاب (htype) {
    حالت VH_بذریعہ:   بذریعہ_155f(regs); توڑو;
    حالت VH_INTEL: intel_155f(regs); توڑو;
    طےشدہ:       handle_155fXX(regs); توڑو;
    }
}

// Main 16bit entry point
خالی
handle_157f(ساخت bregs *regs)
{
    اگر (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        واپس;
    }

    صحیح htype = GET_GLOBAL(VGAHookHandlerType);
    انتخاب (htype) {
    حالت VH_SMI:   smi_157f(regs); توڑو;
    طےشدہ:       handle_157fXX(regs); توڑو;
    }
}

//تیاری
خالی
vgahook_تیاری(ساخت pci_device *pci)
{
    اگر (!CONFIG_VGAHOOKS)
        واپس;

    اگر (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_تیاری(pci);
    ورنہ اگر (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_تیاری(pci);
    ورنہ اگر (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_تیاری(pci);
    ورنہ اگر (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_تیاری(pci);
    ورنہ اگر (pci->vendor == PCI_VENDOR_ID_بذریعہ)
        بذریعہ_تیاری(pci);
    ورنہ اگر (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_تیاری(pci);
}
