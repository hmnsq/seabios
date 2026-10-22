// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_LOWFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SIZE
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-ehci.h" // ehci_wait_controllers
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-ohci.h" // struct ohci_hcca
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // msleep
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // readl

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FIT                     (1 << 31)

ਬਣਤਰ usb_ohci_s {
    ਬਣਤਰ usb_s usb;
    ਬਣਤਰ ohci_regs *regs;
};

ਬਣਤਰ ohci_pipe {
    ਬਣਤਰ ohci_ed ed;
    ਬਣਤਰ usb_pipe pipe;
    ਬਣਤਰ ohci_regs *regs;
    ਖਾਲੀ *ਅੰਕੜੇ;
    ਪੂਰਨ ਗਿਣਤੀ_2;
    ਬਣਤਰ ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
ਸਥਿਰ ਪੂਰਨ
ohci_hub_detect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_ohci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[ਪੋਰਟ]);
    ਵਾਪਸ (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
ਸਥਿਰ ਖਾਲੀ
ohci_hub_disconnect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_ohci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[ਪੋਰਟ], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
ਸਥਿਰ ਪੂਰਨ
ohci_hub_reset(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_ohci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[ਪੋਰਟ], RH_PS_PRS);
    u32 sts;
    u32 ਸਿਰਾ = timer_calc(USB_ਸਮਾਂ_DRSTR * 2);
    ਲਈ (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[ਪੋਰਟ]);
        ਜੇ (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, ਪੋਰਟ);
            ਵਾਪਸ -1;
        }
        yield();
    }

    ਜੇ ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        ਵਾਪਸ -1;

    ਵਾਪਸ !!(sts & RH_PS_LSDA);
}

ਸਥਿਰ ਬਣਤਰ usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
ਸਥਿਰ ਪੂਰਨ
check_ohci_ports(ਬਣਤਰ usb_ohci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();
    // Turn on power for all devices on roothub.
    u32 rha = readl(&cntl->regs->roothub_a);
    rha &= ~(RH_A_PSM | RH_A_OCPM);
    writel(&cntl->regs->roothub_ਸਥਿਤੀ, RH_HS_LPSC);
    writel(&cntl->regs->roothub_b, RH_B_PPCM);
    msleep((rha >> 24) * 2);
    // XXX - need to sleep for USB_TIME_SIGATT if just powered up?

    ਬਣਤਰ usbhub_s hub;
    memset(&hub, 0, ਆਕਾਰ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    ਵਾਪਸ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ਸਥਿਰ ਖਾਲੀ
ohci_waittick(ਬਣਤਰ ohci_regs *regs)
{
    barrier();
    ਬਣਤਰ ohci_hcca *hcca = (ਖਾਲੀ*)regs->hcca;
    u32 startframe = hcca->ਚੌਖਟਾ_no;
    u32 ਸਿਰਾ = timer_calc(1000 * 5);
    ਲਈ (;;) {
        ਜੇ (hcca->ਚੌਖਟਾ_no != startframe)
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ;
        }
        yield();
    }
}

ਸਥਿਰ ਖਾਲੀ
ohci_free_pipes(ਬਣਤਰ usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    ਜੇ (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    ਲਈ (;;) {
        ਬਣਤਰ ohci_ed *ਅਗਲਾ = (ਖਾਲੀ*)*pos;
        ਜੇ (!ਅਗਲਾ)
            ਰੋਕੋ;
        ਬਣਤਰ ohci_pipe *pipe = container_ਦਾ(ਅਗਲਾ, ਬਣਤਰ ohci_pipe, ed);
        ਜੇ (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = ਅਗਲਾ->hw_ਅਗਲਾ_ED;
            ਮੁਕਤ(pipe);
        } ਨਹੀਂਤਾਂ {
            pos = &ਅਗਲਾ->hw_ਅਗਲਾ_ED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ
ਸ਼ੁਰੂ_ohci(ਬਣਤਰ usb_ohci_s *cntl, ਬਣਤਰ ohci_hcca *hcca)
{
    u32 oldfminterval = readl(&cntl->regs->fminterval);
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;

    // XXX - check if already running?

    // Do reset
    writel(&cntl->regs->control, OHCI_USB_RESET | oldrwc);
    readl(&cntl->regs->control); // flush writes
    msleep(USB_ਸਮਾਂ_DRSTR);

    // Do software init (min 10us, max 2ms)
    u32 ਸਿਰਾ = timer_calc_usec(10);
    writel(&cntl->regs->cmdstatus, OHCI_HCR);
    ਲਈ (;;) {
        u32 ਸਥਿਤੀ = readl(&cntl->regs->cmdstatus);
        ਜੇ (!(ਸਥਿਤੀ & OHCI_HCR))
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
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

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ
stop_ohci(ਬਣਤਰ usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

ਸਥਿਰ ਖਾਲੀ
configure_ohci(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ usb_ohci_s *cntl = ਅੰਕੜੇ;

    // Allocate memory
    ਬਣਤਰ ohci_hcca *hcca = memalign_ਉੱਚਾ(256, ਆਕਾਰ(*hcca));
    ਬਣਤਰ ohci_ed *intr_ed = malloc_ਉੱਚਾ(ਆਕਾਰ(*intr_ed));
    ਜੇ (!hcca || !intr_ed) {
        warn_noalloc();
        ਜਾਓ ਮੁਕਤ;
    }
    memset(hcca, 0, ਆਕਾਰ(*hcca));
    memset(intr_ed, 0, ਆਕਾਰ(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ARRAY_ਮਾਪ(hcca->int_ਸਾਰਣੀ); i++)
        hcca->int_ਸਾਰਣੀ[i] = (u32)intr_ed;

    ਪੂਰਨ ret = ਸ਼ੁਰੂ_ohci(cntl, hcca);
    ਜੇ (ret)
        ਜਾਓ err;

    ਪੂਰਨ ਗਿਣਤੀ_2 = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    ਜੇ (! ਗਿਣਤੀ_2)
        ਜਾਓ err;
    ਵਾਪਸ;

err:
    stop_ohci(cntl);
ਮੁਕਤ:
    ਮੁਕਤ(hcca);
    ਮੁਕਤ(intr_ed);
}

ਸਥਿਰ ਖਾਲੀ
ohci_controller_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    ਬਣਤਰ ohci_regs *regs = pci_enable_membar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!regs)
        ਵਾਪਸ;

    ਬਣਤਰ usb_ohci_s *cntl = malloc_tmphigh(ਆਕਾਰ(*cntl));
    ਜੇ (!cntl) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(cntl, 0, ਆਕਾਰ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_OHCI;
    cntl->regs = regs;

    dprintf(1, "OHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    // Disable interrupts
    writel(&cntl->regs->intrdisable, ~0);
    writel(&cntl->regs->intrstatus, ~0);

    ਲੜੀਵਾਰ_thread(configure_ohci, cntl);
}

ਖਾਲੀ
ohci_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_OHCI)
        ਵਾਪਸ;
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_ਤਿਆਰੀ(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
ਸਥਿਰ ਖਾਲੀ
ohci_desc2pipe(ਬਣਤਰ ohci_pipe *pipe, ਬਣਤਰ usbdevice_s *usbdev
               , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->w_Max_Packet_ਮਾਪ << 16)
                       | (usbdev->ਰਫਤਾਰ ? ED_LOWSPEED : 0));
    ਬਣਤਰ usb_ohci_s *cntl = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

ਸਥਿਰ ਬਣਤਰ usb_pipe *
ohci_alloc_intr_pipe(ਬਣਤਰ usbdevice_s *usbdev
                     , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਬਣਤਰ usb_ohci_s *cntl = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_ohci_s, usb);
    ਪੂਰਨ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ਜੇ (frameexp > 5)
        frameexp = 5;
    ਪੂਰਨ maxpacket = epdesc->w_Max_Packet_ਮਾਪ;
    // Determine number of entries needed for 2 timer ticks.
    ਪੂਰਨ ms = 1<<frameexp;
    ਪੂਰਨ ਗਿਣਤੀ_2 = DIV_ਗੇੜਾ_ਉੱਪਰ(ticks_ਮੰਜ਼ਿਲ_ms(2), ms) + 1;
    ਬਣਤਰ ohci_pipe *pipe = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*pipe));
    ਬਣਤਰ ohci_td *tds = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*tds) * ਗਿਣਤੀ_2);
    ਖਾਲੀ *ਅੰਕੜੇ = malloc_ਨੀਵਾਂ(maxpacket * ਗਿਣਤੀ_2);
    ਜੇ (!pipe || !tds || !ਅੰਕੜੇ)
        ਜਾਓ err;
    memset(pipe, 0, ਆਕਾਰ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->ਅੰਕੜੇ = ਅੰਕੜੇ;
    pipe->ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2;
    pipe->tds = tds;

    ਬਣਤਰ ohci_ed *ed = &pipe->ed;
    ed->hw_ਸਿਰ_P = (u32)&tds[0];
    ed->hw_ਪੂਛ_P = (u32)&tds[ਗਿਣਤੀ_2-1];

    ਪੂਰਨ i;
    ਲਈ (i=0; i<ਗਿਣਤੀ_2-1; i++) {
        tds[i].hwINFO = TD_DP_ਵਿੱਚ | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)ਅੰਕੜੇ + maxpacket * i;
        tds[i].hw_ਅਗਲਾ_TD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    ਬਣਤਰ ohci_hcca *hcca = (ਖਾਲੀ*)cntl->regs->hcca;
    ਜੇ (frameexp == 0) {
        // Add to existing interrupt entry.
        ਬਣਤਰ ohci_ed *intr_ed = (ਖਾਲੀ*)hcca->int_ਸਾਰਣੀ[0];
        ed->hw_ਅਗਲਾ_ED = intr_ed->hw_ਅਗਲਾ_ED;
        barrier();
        intr_ed->hw_ਅਗਲਾ_ED = (u32)ed;
    } ਨਹੀਂਤਾਂ {
        ਪੂਰਨ startpos = 1<<(frameexp-1);
        ed->hw_ਅਗਲਾ_ED = hcca->int_ਸਾਰਣੀ[startpos];
        barrier();
        ਲਈ (i=startpos; i<ARRAY_ਮਾਪ(hcca->int_ਸਾਰਣੀ); i+=ms)
            hcca->int_ਸਾਰਣੀ[i] = (u32)ed;
    }

    ਵਾਪਸ &pipe->pipe;

err:
    ਮੁਕਤ(pipe);
    ਮੁਕਤ(tds);
    ਮੁਕਤ(ਅੰਕੜੇ);
    ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ usb_pipe *
ohci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev, ਬਣਤਰ usb_pipe *upipe
                  , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਜੇ (! CONFIG_USB_OHCI)
        ਵਾਪਸ ਨਿਰਾ;
    usb_ਜੋੜੋ_freelist(upipe);
    ਜੇ (!epdesc)
        ਵਾਪਸ ਨਿਰਾ;
    u8 eptype = epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ;
    ਜੇ (eptype == USB_ENDPOINT_XFER_INT)
        ਵਾਪਸ ohci_alloc_intr_pipe(usbdev, epdesc);
    ਬਣਤਰ usb_ohci_s *cntl = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    ਬਣਤਰ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ਜੇ (usbpipe) {
        // Use previously allocated pipe.
        ਬਣਤਰ ohci_pipe *pipe = container_ਦਾ(usbpipe, ਬਣਤਰ ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        ਵਾਪਸ usbpipe;
    }

    // Allocate a new queue head.
    ਬਣਤਰ ohci_pipe *pipe;
    ਜੇ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ਆਕਾਰ(*pipe));
    ਨਹੀਂਤਾਂ
        pipe = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*pipe));
    ਜੇ (!pipe) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(pipe, 0, ਆਕਾਰ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *ਸਿਰ = &cntl->regs->ed_controlhead;
    ਜੇ (eptype != USB_ENDPOINT_XFER_CONTROL)
        ਸਿਰ = &cntl->regs->ed_bulkhead;
    pipe->ed.hw_ਅਗਲਾ_ED = *ਸਿਰ;
    barrier();
    *ਸਿਰ = (u32)&pipe->ed;
    ਵਾਪਸ &pipe->pipe;
}

ਸਥਿਰ ਪੂਰਨ
wait_ed(ਬਣਤਰ ohci_ed *ed, ਪੂਰਨ timeout)
{
    u32 ਸਿਰਾ = timer_calc(timeout);
    ਲਈ (;;) {
        ਜੇ ((ed->hw_ਸਿਰ_P & ~(ED_C|ED_H)) == ed->hw_ਪੂਛ_P)
            ਵਾਪਸ 0;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hw_ਪੂਛ_P, ed->hw_ਸਿਰ_P, ed->hw_ਅਗਲਾ_ED);
            ਵਾਪਸ -1;
        }
        yield();
    }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STACKOTDS 18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OHCI_TD_ALIGN 16

ਪੂਰਨ
ohci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
               , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_USB_OHCI)
        ਵਾਪਸ -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    ਬਣਤਰ ohci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ਆਕਾਰ(ਬਣਤਰ ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    ਬਣਤਰ ohci_td *tds = (ਖਾਲੀ*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, ਆਕਾਰ(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    ਜੇ (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_ਤਿਆਰੀ | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hw_ਅਗਲਾ_TD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_ਤਿਆਰੀ_ਮਾਪ - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)ਅੰਕੜੇ, dataend = dest + datasize;
    ਜਦੋਂ (dest < dataend) {
        // Send data pids
        ਜੇ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ਵਾਪਸ -1;
        }
        ਪੂਰਨ maxtransfer = 2*ਪੰਨਾ_ਮਾਪ - (dest & (ਪੰਨਾ_ਮਾਪ-1));
        ਪੂਰਨ transfer = dataend - dest;
        ਜੇ (transfer > maxtransfer)
            transfer = ALIGN_ਥੱਲੇ(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_ਵਿੱਚ : TD_DP_ਬਾਹਰਲਾ) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hw_ਅਗਲਾ_TD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    ਜੇ (cmd) {
        // Send status pid on control transfers
        ਜੇ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ਵਾਪਸ -1;
        }
        td->hwINFO = (dir ? TD_DP_ਬਾਹਰਲਾ : TD_DP_ਵਿੱਚ) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hw_ਅਗਲਾ_TD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hw_ਸਿਰ_P = (u32)tds | (pipe->ed.hw_ਸਿਰ_P & ED_C);
    pipe->ed.hw_ਪੂਛ_P = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    ਪੂਰਨ ret = wait_ed(&pipe->ed, usb_xfer_ਸਮਾਂ(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    ਜੇ (ret)
        ohci_waittick(pipe->regs);
    ਵਾਪਸ ret;
}

ਪੂਰਨ
ohci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ)
{
    ASSERT16();
    ਜੇ (! CONFIG_USB_OHCI)
        ਵਾਪਸ -1;

    ਬਣਤਰ ohci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ ohci_pipe, pipe);
    ਬਣਤਰ ohci_td *tds = GET_LOWFLAT(pipe->tds);
    ਬਣਤਰ ohci_td *ਸਿਰ = (ਖਾਲੀ*)(GET_LOWFLAT(pipe->ed.hw_ਸਿਰ_P) & ~(ED_C|ED_H));
    ਬਣਤਰ ohci_td *ਪੂਛ = (ਖਾਲੀ*)GET_LOWFLAT(pipe->ed.hw_ਪੂਛ_P);
    ਪੂਰਨ ਗਿਣਤੀ_2 = GET_LOWFLAT(pipe->ਗਿਣਤੀ_2);
    ਪੂਰਨ pos = (ਪੂਛ - tds + 1) % ਗਿਣਤੀ_2;
    ਬਣਤਰ ohci_td *ਅਗਲਾ = &tds[pos];
    ਜੇ (ਸਿਰ == ਅਗਲਾ)
        // No intrs found.
        ਵਾਪਸ -1;
    // XXX - check for errors.

    // Copy data.
    ਪੂਰਨ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ਖਾਲੀ *pipedata = GET_LOWFLAT((pipe->ਅੰਕੜੇ));
    ਖਾਲੀ *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), ਅੰਕੜੇ, SEG_ਨੀਵਾਂ, LOWFLAT2_ਨੀਵਾਂ(intrdata), maxpacket);

    // Reenable this td.
    ਸੈੱਟ_LOWFLAT(ਪੂਛ->hwINFO, TD_DP_ਵਿੱਚ | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (ਪੂਛ-tds);
    ਸੈੱਟ_LOWFLAT(ਪੂਛ->hwCBP, (u32)intrdata);
    ਸੈੱਟ_LOWFLAT(ਪੂਛ->hw_ਅਗਲਾ_TD, (u32)ਅਗਲਾ);
    ਸੈੱਟ_LOWFLAT(ਪੂਛ->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    ਸੈੱਟ_LOWFLAT(pipe->ed.hw_ਪੂਛ_P, (u32)ਅਗਲਾ);

    ਵਾਪਸ 0;
}
