// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOWFLAT
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-ehci.h" // ehci_wait_controllers
#ಅನ್ತರ್ಭಾವ "usb-ohci.h" // struct ohci_hcca
#ಅನ್ತರ್ಭಾವ "util.h" // msleep
#ಅನ್ತರ್ಭಾವ "x86.h" // readl

#ವ್ಯಾಖ್ಯಾನಿಸು FIT                     (1 << 31)

ರಚನೆ usb_ohci_s {
    ರಚನೆ usb_s usb;
    ರಚನೆ ohci_regs *regs;
};

ರಚನೆ ohci_pipe {
    ರಚನೆ ohci_ed ed;
    ರಚನೆ usb_pipe pipe;
    ರಚನೆ ohci_regs *regs;
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
    ರಚನೆ ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ohci_hub_detect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_ohci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[ಪೋರ್ಟ್]);
    ಹಿಂತಿರುಗಿ (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
ohci_hub_disconnect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_ohci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[ಪೋರ್ಟ್], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ohci_hub_reset(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_ohci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[ಪೋರ್ಟ್], RH_PS_PRS);
    u32 sts;
    u32 ಕೊನೆ = timer_calc(USB_ಸಮಯ_DRSTR * 2);
    ಗಾಗಿ (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[ಪೋರ್ಟ್]);
        ಆದರೆ (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, ಪೋರ್ಟ್);
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }

    ಆದರೆ ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        ಹಿಂತಿರುಗಿ -1;

    ಹಿಂತಿರುಗಿ !!(sts & RH_PS_LSDA);
}

ಸ್ಥಿರ ರಚನೆ usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
check_ohci_ports(ರಚನೆ usb_ohci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();
    // Turn on power for all devices on roothub.
    u32 rha = readl(&cntl->regs->roothub_a);
    rha &= ~(RH_A_PSM | RH_A_OCPM);
    writel(&cntl->regs->roothub_ಸ್ಥಿತಿ, RH_HS_LPSC);
    writel(&cntl->regs->roothub_b, RH_B_PPCM);
    msleep((rha >> 24) * 2);
    // XXX - need to sleep for USB_TIME_SIGATT if just powered up?

    ರಚನೆ usbhub_s hub;
    memset(&hub, 0, ಗಾತ್ರ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    ಹಿಂತಿರುಗಿ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ಸ್ಥಿರ ಶೂನ್ಯ
ohci_waittick(ರಚನೆ ohci_regs *regs)
{
    barrier();
    ರಚನೆ ohci_hcca *hcca = (ಶೂನ್ಯ*)regs->hcca;
    u32 startframe = hcca->ಚೌಕಟ್ಟು_no;
    u32 ಕೊನೆ = timer_calc(1000 * 5);
    ಗಾಗಿ (;;) {
        ಆದರೆ (hcca->ಚೌಕಟ್ಟು_no != startframe)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ;
        }
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ohci_free_pipes(ರಚನೆ usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    ಆದರೆ (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    ಗಾಗಿ (;;) {
        ರಚನೆ ohci_ed *ಮುಂದಿನ = (ಶೂನ್ಯ*)*pos;
        ಆದರೆ (!ಮುಂದಿನ)
            ನಿಲ್ಲಿಸು;
        ರಚನೆ ohci_pipe *pipe = container_ದ(ಮುಂದಿನ, ರಚನೆ ohci_pipe, ed);
        ಆದರೆ (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = ಮುಂದಿನ->hw_ಮುಂದಿನ_ED;
            ಬಿಡುಗಡೆ(pipe);
        } ಇಲ್ಲದಿದ್ದರೆ {
            pos = &ಮುಂದಿನ->hw_ಮುಂದಿನ_ED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಆರಂಭ_ohci(ರಚನೆ usb_ohci_s *cntl, ರಚನೆ ohci_hcca *hcca)
{
    u32 oldfminterval = readl(&cntl->regs->fminterval);
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;

    // XXX - check if already running?

    // Do reset
    writel(&cntl->regs->control, OHCI_USB_RESET | oldrwc);
    readl(&cntl->regs->control); // flush writes
    msleep(USB_ಸಮಯ_DRSTR);

    // Do software init (min 10us, max 2ms)
    u32 ಕೊನೆ = timer_calc_usec(10);
    writel(&cntl->regs->cmdstatus, OHCI_HCR);
    ಗಾಗಿ (;;) {
        u32 ಸ್ಥಿತಿ_2 = readl(&cntl->regs->cmdstatus);
        ಆದರೆ (!(ಸ್ಥಿತಿ_2 & OHCI_HCR))
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
    }

    // Init memory
    writel(&cntl->regs->ed_controlhead, 0);
    writel(&cntl->regs->ed_bulkhead, 0);
    writel(&cntl->regs->hcca, (u32)hcca);

    // Init fminterval
    u32 fi = oldfminterval & 0x3fff;
    writel(&cntl->regs->fminterval
           , (((oldfminterval & FIT) ^ FIT)
              | fi | (((6 * (fi - 210)) / 7) << 16)));
    writel(&cntl->regs->periodicstart, ((9 * fi) / 10) & 0x3fff);
    readl(&cntl->regs->control); // flush writes

    // XXX - verify that fminterval was setup correctly.

    // Go into operational state
    writel(&cntl->regs->control
           , (OHCI_CTRL_CBSR | OHCI_CTRL_CLE | OHCI_CTRL_BLE | OHCI_CTRL_PLE
              | OHCI_USB_OPER | oldrwc));
    readl(&cntl->regs->control); // flush writes

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
stop_ohci(ರಚನೆ usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

ಸ್ಥಿರ ಶೂನ್ಯ
configure_ohci(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ usb_ohci_s *cntl = ದತ್ತಾಂಶ;

    // Allocate memory
    ರಚನೆ ohci_hcca *hcca = memalign_ಮೇಲಿನ(256, ಗಾತ್ರ(*hcca));
    ರಚನೆ ohci_ed *intr_ed = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*intr_ed));
    ಆದರೆ (!hcca || !intr_ed) {
        warn_noalloc();
        ಹೋಗು ಬಿಡುಗಡೆ;
    }
    memset(hcca, 0, ಗಾತ್ರ(*hcca));
    memset(intr_ed, 0, ಗಾತ್ರ(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(hcca->int_ಕೋಷ್ಟಕ); i++)
        hcca->int_ಕೋಷ್ಟಕ[i] = (u32)intr_ed;

    ಪೂರ್ಣಾಂಕ ret = ಆರಂಭ_ohci(cntl, hcca);
    ಆದರೆ (ret)
        ಹೋಗು err;

    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    ಆದರೆ (! ಎಣಿಕೆ_2)
        ಹೋಗು err;
    ಹಿಂತಿರುಗಿ;

err:
    stop_ohci(cntl);
ಬಿಡುಗಡೆ:
    ಬಿಡುಗಡೆ(hcca);
    ಬಿಡುಗಡೆ(intr_ed);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ohci_controller_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    ರಚನೆ ohci_regs *regs = pci_enable_membar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!regs)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ usb_ohci_s *cntl = malloc_tmphigh(ಗಾತ್ರ(*cntl));
    ಆದರೆ (!cntl) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(cntl, 0, ಗಾತ್ರ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_OHCI;
    cntl->regs = regs;

    dprintf(1, "OHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    // Disable interrupts
    writel(&cntl->regs->intrdisable, ~0);
    writel(&cntl->regs->intrstatus, ~0);

    ಸತತ_thread(configure_ohci, cntl);
}

ಶೂನ್ಯ
ohci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_OHCI)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_ಸಿದ್ಧತೆ(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
ಸ್ಥಿರ ಶೂನ್ಯ
ohci_desc2pipe(ರಚನೆ ohci_pipe *pipe, ರಚನೆ usbdevice_s *usbdev
               , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->w_Max_Packet_ಗಾತ್ರ << 16)
                       | (usbdev->ವೇಗ ? ED_LOWSPEED : 0));
    ರಚನೆ usb_ohci_s *cntl = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

ಸ್ಥಿರ ರಚನೆ usb_pipe *
ohci_alloc_intr_pipe(ರಚನೆ usbdevice_s *usbdev
                     , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ರಚನೆ usb_ohci_s *cntl = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_ohci_s, usb);
    ಪೂರ್ಣಾಂಕ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ಆದರೆ (frameexp > 5)
        frameexp = 5;
    ಪೂರ್ಣಾಂಕ maxpacket = epdesc->w_Max_Packet_ಗಾತ್ರ;
    // Determine number of entries needed for 2 timer ticks.
    ಪೂರ್ಣಾಂಕ ms = 1<<frameexp;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = DIV_ಸುತ್ತು_ಮೇಲೆ(ticks_ಗಮ್ಯ_ms(2), ms) + 1;
    ರಚನೆ ohci_pipe *pipe = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*pipe));
    ರಚನೆ ohci_td *tds = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*tds) * ಎಣಿಕೆ_2);
    ಶೂನ್ಯ *ದತ್ತಾಂಶ = malloc_ಕೆಳಗಿನ(maxpacket * ಎಣಿಕೆ_2);
    ಆದರೆ (!pipe || !tds || !ದತ್ತಾಂಶ)
        ಹೋಗು err;
    memset(pipe, 0, ಗಾತ್ರ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
    pipe->ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2;
    pipe->tds = tds;

    ರಚನೆ ohci_ed *ed = &pipe->ed;
    ed->hw_ತಲೆ_P = (u32)&tds[0];
    ed->hw_ಬಾಲ_P = (u32)&tds[ಎಣಿಕೆ_2-1];

    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ಎಣಿಕೆ_2-1; i++) {
        tds[i].hwINFO = TD_DP_ಒಳ | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)ದತ್ತಾಂಶ + maxpacket * i;
        tds[i].hw_ಮುಂದಿನ_TD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    ರಚನೆ ohci_hcca *hcca = (ಶೂನ್ಯ*)cntl->regs->hcca;
    ಆದರೆ (frameexp == 0) {
        // Add to existing interrupt entry.
        ರಚನೆ ohci_ed *intr_ed = (ಶೂನ್ಯ*)hcca->int_ಕೋಷ್ಟಕ[0];
        ed->hw_ಮುಂದಿನ_ED = intr_ed->hw_ಮುಂದಿನ_ED;
        barrier();
        intr_ed->hw_ಮುಂದಿನ_ED = (u32)ed;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಪೂರ್ಣಾಂಕ startpos = 1<<(frameexp-1);
        ed->hw_ಮುಂದಿನ_ED = hcca->int_ಕೋಷ್ಟಕ[startpos];
        barrier();
        ಗಾಗಿ (i=startpos; i<ARRAY_ಗಾತ್ರ(hcca->int_ಕೋಷ್ಟಕ); i+=ms)
            hcca->int_ಕೋಷ್ಟಕ[i] = (u32)ed;
    }

    ಹಿಂತಿರುಗಿ &pipe->pipe;

err:
    ಬಿಡುಗಡೆ(pipe);
    ಬಿಡುಗಡೆ(tds);
    ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ usb_pipe *
ohci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev, ರಚನೆ usb_pipe *upipe
                  , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಆದರೆ (! CONFIG_USB_OHCI)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    usb_ಕೂಡಿಸು_freelist(upipe);
    ಆದರೆ (!epdesc)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    u8 eptype = epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_INT)
        ಹಿಂತಿರುಗಿ ohci_alloc_intr_pipe(usbdev, epdesc);
    ರಚನೆ usb_ohci_s *cntl = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    ರಚನೆ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ಆದರೆ (usbpipe) {
        // Use previously allocated pipe.
        ರಚನೆ ohci_pipe *pipe = container_ದ(usbpipe, ರಚನೆ ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        ಹಿಂತಿರುಗಿ usbpipe;
    }

    // Allocate a new queue head.
    ರಚನೆ ohci_pipe *pipe;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ಗಾತ್ರ(*pipe));
    ಇಲ್ಲದಿದ್ದರೆ
        pipe = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*pipe));
    ಆದರೆ (!pipe) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(pipe, 0, ಗಾತ್ರ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *ತಲೆ = &cntl->regs->ed_controlhead;
    ಆದರೆ (eptype != USB_ENDPOINT_XFER_CONTROL)
        ತಲೆ = &cntl->regs->ed_bulkhead;
    pipe->ed.hw_ಮುಂದಿನ_ED = *ತಲೆ;
    barrier();
    *ತಲೆ = (u32)&pipe->ed;
    ಹಿಂತಿರುಗಿ &pipe->pipe;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
wait_ed(ರಚನೆ ohci_ed *ed, ಪೂರ್ಣಾಂಕ timeout)
{
    u32 ಕೊನೆ = timer_calc(timeout);
    ಗಾಗಿ (;;) {
        ಆದರೆ ((ed->hw_ತಲೆ_P & ~(ED_C|ED_H)) == ed->hw_ಬಾಲ_P)
            ಹಿಂತಿರುಗಿ 0;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hw_ಬಾಲ_P, ed->hw_ತಲೆ_P, ed->hw_ಮುಂದಿನ_ED);
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
}

#ವ್ಯಾಖ್ಯಾನಿಸು STACKOTDS 18
#ವ್ಯಾಖ್ಯಾನಿಸು OHCI_TD_ALIGN 16

ಪೂರ್ಣಾಂಕ
ohci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_USB_OHCI)
        ಹಿಂತಿರುಗಿ -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    ರಚನೆ ohci_pipe *pipe = container_ದ(p, ರಚನೆ ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ಗಾತ್ರ(ರಚನೆ ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    ರಚನೆ ohci_td *tds = (ಶೂನ್ಯ*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, ಗಾತ್ರ(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    ಆದರೆ (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_ಸಿದ್ಧತೆ | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hw_ಮುಂದಿನ_TD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_ಸಿದ್ಧತೆ_ಗಾತ್ರ - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)ದತ್ತಾಂಶ, dataend = dest + datasize;
    ಆಗ (dest < dataend) {
        // Send data pids
        ಆದರೆ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ -1;
        }
        ಪೂರ್ಣಾಂಕ maxtransfer = 2*ಪುಟ_ಗಾತ್ರ - (dest & (ಪುಟ_ಗಾತ್ರ-1));
        ಪೂರ್ಣಾಂಕ transfer = dataend - dest;
        ಆದರೆ (transfer > maxtransfer)
            transfer = ALIGN_ಕೆಳಕ್ಕೆ(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_ಒಳ : TD_DP_ಹೊರಹೋಗು) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hw_ಮುಂದಿನ_TD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    ಆದರೆ (cmd) {
        // Send status pid on control transfers
        ಆದರೆ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ -1;
        }
        td->hwINFO = (dir ? TD_DP_ಹೊರಹೋಗು : TD_DP_ಒಳ) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hw_ಮುಂದಿನ_TD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hw_ತಲೆ_P = (u32)tds | (pipe->ed.hw_ತಲೆ_P & ED_C);
    pipe->ed.hw_ಬಾಲ_P = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    ಪೂರ್ಣಾಂಕ ret = wait_ed(&pipe->ed, usb_xfer_ಸಮಯ(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    ಆದರೆ (ret)
        ohci_waittick(pipe->regs);
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ
ohci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ASSERT16();
    ಆದರೆ (! CONFIG_USB_OHCI)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ ohci_pipe *pipe = container_ದ(p, ರಚನೆ ohci_pipe, pipe);
    ರಚನೆ ohci_td *tds = GET_LOWFLAT(pipe->tds);
    ರಚನೆ ohci_td *ತಲೆ = (ಶೂನ್ಯ*)(GET_LOWFLAT(pipe->ed.hw_ತಲೆ_P) & ~(ED_C|ED_H));
    ರಚನೆ ohci_td *ಬಾಲ = (ಶೂನ್ಯ*)GET_LOWFLAT(pipe->ed.hw_ಬಾಲ_P);
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = GET_LOWFLAT(pipe->ಎಣಿಕೆ_2);
    ಪೂರ್ಣಾಂಕ pos = (ಬಾಲ - tds + 1) % ಎಣಿಕೆ_2;
    ರಚನೆ ohci_td *ಮುಂದಿನ = &tds[pos];
    ಆದರೆ (ತಲೆ == ಮುಂದಿನ)
        // No intrs found.
        ಹಿಂತಿರುಗಿ -1;
    // XXX - check for errors.

    // Copy data.
    ಪೂರ್ಣಾಂಕ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ಶೂನ್ಯ *pipedata = GET_LOWFLAT((pipe->ದತ್ತಾಂಶ));
    ಶೂನ್ಯ *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), ದತ್ತಾಂಶ, SEG_ಕೆಳಗಿನ, LOWFLAT2_ಕೆಳಗಿನ(intrdata), maxpacket);

    // Reenable this td.
    ಗಣ_LOWFLAT(ಬಾಲ->hwINFO, TD_DP_ಒಳ | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (ಬಾಲ-tds);
    ಗಣ_LOWFLAT(ಬಾಲ->hwCBP, (u32)intrdata);
    ಗಣ_LOWFLAT(ಬಾಲ->hw_ಮುಂದಿನ_TD, (u32)ಮುಂದಿನ);
    ಗಣ_LOWFLAT(ಬಾಲ->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    ಗಣ_LOWFLAT(pipe->ed.hw_ಬಾಲ_P, (u32)ಮುಂದಿನ);

    ಹಿಂತಿರುಗಿ 0;
}
