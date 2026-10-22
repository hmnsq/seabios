// Hooks for via vgabios calls into main bios.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBAL
#include "bregs.h" // set_code_invalid
#include "config.h" // CONFIG_*
#include "hw/pci.h" // pci_config_readb
#include "hw/pcidevice.h" // pci_find_device
#include "hw/pci_ids.h" // PCI_VENDOR_ID_VIA
#include "hw/pci_regs.h" // PCI_VENDOR_ID
#include "output.h" // dprintf
#include "string.h" // strcmp
#include "util.h" // handle_155f, handle_157f

#define VH_VIA 1
#define VH_INTEL 2
#define VH_SMI 3

puur_naa_nka VGAHookHandlerType VARFSEG;

static void
handle_155fXX(sa_nracanaa bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

static void
handle_157fXX(sa_nracanaa bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
}

/****************************************************************
 * Via hooks
 ****************************************************************/

puur_naa_nka ViaFBsize VARFSEG, ViaRamSpeed VARFSEG;

static void
via_155f01(sa_nracanaa bregs *regs)
{
    regs->eax = 0x5f;
    regs->cl = 2; // panel type =  2 = 1024 * 768
    set_success(regs);
    dprintf(1, "Warning: VGA panel type is hardcoded\n");
}

static void
via_155f02(sa_nracanaa bregs *regs)
{
    regs->eax = 0x5f;
    regs->bx = 2;
    regs->cx = 0x401;  // PAL + crt only
    regs->dx = 0;  // TV Layout - default
    set_success(regs);
    dprintf(1, "Warning: VGA TV/CRT output type is hardcoded\n");
}

static void
via_155f18(sa_nracanaa bregs *regs)
{
    puur_naa_nka fbsize = GET_GLOBAL(ViaFBsize), ramspeed = GET_GLOBAL(ViaRamSpeed);
    ᨀᨘ (fbsize < 0 || ramspeed < 0) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ᨀᨘᨒ;
    }
    regs->eax = 0x5f;
    regs->ebx = 0x500 | (ramspeed << 4) | fbsize;
    regs->ecx = 0x060;
    set_success(regs);
}

static void
via_155f19(sa_nracanaa bregs *regs)
{
    set_invalid_silent(regs);
}

static void
via_155f(sa_nracanaa bregs *regs)
{
    switch (regs->al) {
    pak_sa 0x01: via_155f01(regs); bha_nga;
    pak_sa 0x02: via_155f02(regs); bha_nga;
    pak_sa 0x18: via_155f18(regs); bha_nga;
    pak_sa 0x19: via_155f19(regs); bha_nga;
    default:   handle_155fXX(regs); bha_nga;
    }
}

static puur_naa_nka
getFBSize(sa_nracanaa pci_device *pci)
{
    /* FB config */
    u8 reg = pci_config_readb(pci->bdf, 0xa1);

    /* GFX disabled ? */
    ᨀᨘ (!(reg & 0x80))
        ᨀᨘᨒ -1;

    static u8 mem_power[] = {0, 3, 4, 5, 6, 7, 8, 9};
    ᨀᨘᨒ mem_power[(reg >> 4) & 0x7];
}

static puur_naa_nka
getViaRamSpeed(sa_nracanaa pci_device *pci)
{
    ᨀᨘᨒ (pci_config_readb(pci->bdf, 0x90) & 0x07) + 3;
}

static puur_naa_nka
getAMDRamSpeed(void)
{
    sa_nracanaa pci_device *pci = pci_find_device(PCI_VENDOR_ID_AMD
                                             , PCI_DEVICE_ID_AMD_K8_NB_MEMCTL);
    ᨀᨘ (!pci)
        ᨀᨘᨒ -1;

    /* mem clk 0 = DDR2 400 */
    ᨀᨘᨒ (pci_config_readb(pci->bdf, 0x94) & 0x7) + 6;
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

static void
via_setup(sa_nracanaa pci_device *pci)
{
    VGAHookHandlerType = VH_VIA;

    sa_nracanaa pci_device *d = pci_find_device(PCI_VENDOR_ID_VIA
                                           , PCI_DEVICE_ID_VIA_K8M890CE_3);
    ᨀᨘ (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getAMDRamSpeed();
        ᨀᨘᨒ;
    }
    d = pci_find_device(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_VX855_MEMCTRL);
    ᨀᨘ (d) {
        ViaFBsize = getFBSize(d);
        ViaRamSpeed = getViaRamSpeed(d);
        ᨀᨘᨒ;
    }

    dprintf(1, "Warning: VGA memory size and speed is hardcoded\n");
    ViaFBsize = 5; // 32M frame buffer
    ViaRamSpeed = 4; // MCLK = DDR266
}


/****************************************************************
 * Intel VGA hooks
 ****************************************************************/

u8 IntelDisplayType VARFSEG, IntelDisplayId VARFSEG;

static void
intel_155f35(sa_nracanaa bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayType);
    set_success(regs);
}

static void
intel_155f40(sa_nracanaa bregs *regs)
{
    regs->ax = 0x005f;
    regs->cl = GET_GLOBAL(IntelDisplayId);
    set_success(regs);
}

static void
intel_155f50(sa_nracanaa bregs *regs)
{
    /* Mandatory hook on some Dell laptops */
    regs->ax = 0x005f;
    set_success(regs);
}

static void
intel_155f(sa_nracanaa bregs *regs)
{
    switch (regs->al) {
    pak_sa 0x35: intel_155f35(regs); bha_nga;
    pak_sa 0x40: intel_155f40(regs); bha_nga;
    pak_sa 0x50: intel_155f50(regs); bha_nga;
    default:   handle_155fXX(regs); bha_nga;
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

static void
intel_setup(sa_nracanaa pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;

    IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayId = 3;
}

static void
roda_setup(sa_nracanaa pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    // IntelDisplayType = BOOT_DISPLAY_DEFAULT;
    IntelDisplayType = BOOT_DISPLAY_LCD;
    // IntelDisplayId = inb(0x60f) & 0x0f; // Correct according to Crete
    IntelDisplayId = 3; // Correct according to empirical studies
}

static void
kontron_setup(sa_nracanaa pci_device *pci)
{
    VGAHookHandlerType = VH_INTEL;
    IntelDisplayType = BOOT_DISPLAY_CRT;
    IntelDisplayId = 3;
}

static void
getac_setup(sa_nracanaa pci_device *pci)
{
}

/****************************************************************
 * Silicon Motion hooks
 ****************************************************************/

u8 SmiBootDisplay VARFSEG; // 1: LCD, 2: CRT, 3: Both */

static void
smi_157f02(sa_nracanaa bregs *regs)
{
    /* Boot Display Device Override */
    regs->ax = 0x007f;
    regs->bl = GET_GLOBAL(SmiBootDisplay);
    set_success(regs);
}

static void
smi_157f14(sa_nracanaa bregs *regs)
{
    /* ReduceOn support default status */
    regs->ax = 0x007f;
    regs->bl = 0x00;
    set_success(regs);
}

static void
smi_157f(sa_nracanaa bregs *regs)
{
    switch (regs->al) {
    pak_sa 0x02: smi_157f02(regs); bha_nga;
    pak_sa 0x14: smi_157f14(regs); bha_nga;
    default:   handle_157fXX(regs); bha_nga;
    }
}

static void
winent_mb6047_setup(sa_nracanaa pci_device *pci)
{
    VGAHookHandlerType = VH_SMI;
    SmiBootDisplay = 0x02;
}

/****************************************************************
 * Entry and setup
 ****************************************************************/

// Main 16bit entry point
void
handle_155f(sa_nracanaa bregs *regs)
{
    ᨀᨘ (!CONFIG_VGAHOOKS) {
        handle_155fXX(regs);
        ᨀᨘᨒ;
    }

    puur_naa_nka htype = GET_GLOBAL(VGAHookHandlerType);
    switch (htype) {
    pak_sa VH_VIA:   via_155f(regs); bha_nga;
    pak_sa VH_INTEL: intel_155f(regs); bha_nga;
    default:       handle_155fXX(regs); bha_nga;
    }
}

// Main 16bit entry point
void
handle_157f(sa_nracanaa bregs *regs)
{
    ᨀᨘ (!CONFIG_VGAHOOKS) {
        handle_157fXX(regs);
        ᨀᨘᨒ;
    }

    puur_naa_nka htype = GET_GLOBAL(VGAHookHandlerType);
    switch (htype) {
    pak_sa VH_SMI:   smi_157f(regs); bha_nga;
    default:       handle_157fXX(regs); bha_nga;
    }
}

// Setup
void
vgahook_setup(sa_nracanaa pci_device *pci)
{
    ᨀᨘ (!CONFIG_VGAHOOKS)
        ᨀᨘᨒ;

    ᨀᨘ (strcmp(CBvendor, "KONTRON") == 0 && strcmp(CBpart, "986LCD-M") == 0)
        kontron_setup(pci);
    ᨀᨘᨁ ᨀᨘ (strcmp(CBvendor, "GETAC") == 0 && strcmp(CBpart, "P470") == 0)
        getac_setup(pci);
    ᨀᨘᨁ ᨀᨘ (strcmp(CBvendor, "RODA") == 0 && strcmp(CBpart, "RK886EX") == 0)
        roda_setup(pci);
    ᨀᨘᨁ ᨀᨘ (strcmp(CBvendor, "Win Enterprise") == 0 && strcmp(CBpart, "MB6047") == 0)
        winent_mb6047_setup(pci);
    ᨀᨘᨁ ᨀᨘ (pci->vendor == PCI_VENDOR_ID_VIA)
        via_setup(pci);
    ᨀᨘᨁ ᨀᨘ (pci->vendor == PCI_VENDOR_ID_INTEL)
        intel_setup(pci);
}
